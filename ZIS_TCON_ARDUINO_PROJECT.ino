/*
 * THIS CODE WILL FAIL TO COMPILE IN AN UNMODIFIED ARDUINO ENVIRONMENT! 
 * License for use is granted for use with ZisWorks hardware only.  Use of this code for any other purpose is prohibited.
 * TO AVOID DAMAGING YOUR BOARD AND/OR PANEL, CORRECTLY SET THE VALUES UNDER " CHANGE SYSTEM CONFIGURATION PARAMETERS HERE " IN "CONSTANTS.H"
 * This code is intended for use with the ZisWorks "4k120 tcon v1.3" board. 
 * Timing in this application is absolutely critical, so some core files had to be modified to avoid the use of interrupts.  
 * Notably, serial buffer transfer and timer0 overflow handling.  These tasks have been pushed into the control loop.
 * 
 * This version is a work-in-progress, though it is in a fairly stable state and is perfectly usable.  
 * This version has not been tested with non-zws backlights, so it will probalby not work with them yet.
 * 
 * MODIFICATIONS AND LIBRARIES NECESSARY
 * ... a bunch of them.  See the firmware update guide for more info 
 * 
 * Please forgive the coding.
 * 
 * There was too much legacy code to keep track of.  This version is for the DualDP TCON only
 */

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push

#include <EEPROM.h>
#include <avr/wdt.h>
#include "constants.h"
//#include "BACKLIGHT.h"
#include "edid.h"
#include <SoftIIC.h>
#include <WS2812.h>
#include <SendOnlySoftwareSerial.h>
#include "INPUT_HANDLING.h"

SendOnlySoftwareSerial SerialToPanel(PANEL_GPIO0);
SendOnlySoftwareSerial SerialToBldriver(BLPIN_PWM);

InputHandling Inputs;

#if BOARD_VERSION==BOARD_IS_EP369_REV2017    
  EDID myEDID = EDID(); 
#else
  EDID myEDID_PRI = EDID(); 
  EDID myEDID_SEC = EDID(); 
#endif

SoftIIC my_SoftIIC_EDID_PRI=SoftIIC(SCL_PIN_PRI, SDA_PIN_PRI, IIC_SPEED, true, false, true);
SoftIIC my_SoftIIC_EDID_SEC=SoftIIC(SCL_PIN_SEC, SDA_PIN_SEC, IIC_SPEED, true, false, true);

#ifdef LED_RGB
  #define LEDCount 1   // Number of LEDs in RGB string
  WS2812 LED(LEDCount); 
  cRGB value;
  cRGB valuePrev;
#endif

uint32_t currentMillis=0;
uint32_t Task1ms_previousMillis=0;
uint32_t Task10ms_previousMillis=0;
uint32_t Task100ms_previousMillis=0;
uint32_t Task1000ms_previousMillis=0;
uint32_t Task10000ms_previousMillis=0; 
uint32_t FinishedConfigurationTime=0;
const uint32_t MillisToSpamConfigUART=200;
const uint32_t MillisBetweenForwardedUARTAndSerialStatusSending=10000;
const uint32_t MillisToEnterSelfTest=5000;
const uint32_t MillisToEnterEDIDUpdate=250;
uint32_t BlockPanelUARTSelfUpdate=0;
uint32_t ButtonHoldTime=0;

void power_down_receivers();
void regular_operation_transmitters();
uint8_t DetermineIfFactoryProgrammed();
void do_factory_configuration();
void SetStaticPins();
void configure_watchdog_timer();
void power_down_fpga();
void Task1ms();
void Task10ms();
void Task100ms();
void Task1000ms();
void Task10000ms();
void handle_button_state();
void HandleSystemState();
void SendSerialStateToBldriver();

uint8_t UserConfiguration_LoadShutdown();
void UserConfiguration_SaveShutdown(uint8_t myValue);
void UserConfiguration_SaveDefaultShutdown();

uint8_t UserConfiguration_LoadEDID();
void UserConfiguration_SaveEDID(uint8_t myValue);
void UserConfiguration_SaveDefaultEDID();

uint8_t UserConfiguration_LoadBacklightMode();
void UserConfiguration_SaveBacklightMode(uint8_t myValue);
void UserConfiguration_SaveDefaultBacklightMode();

uint8_t UserConfiguration_LoadBrightness();
void UserConfiguration_SaveBrightness(uint8_t myValue);
void UserConfiguration_SaveDefaultBrightness();

uint8_t UserConfiguration_LoadFrequencyPWM();
void UserConfiguration_SaveFrequencyPWM(uint8_t myValue);
void UserConfiguration_SaveDefaultFrequencyPWM();

uint8_t UserConfiguration_LoadOSD();
void UserConfiguration_SaveOSD(uint8_t myValue);
void UserConfiguration_SaveDefaultOSD();

uint8_t UserConfiguration_LoadCrosshair();
void UserConfiguration_SaveCrosshair(uint8_t myValue);
void UserConfiguration_SaveDefaultCrosshair();

uint8_t UserConfiguration_LoadMagicByte();
void UserConfiguration_SaveDefaultMagicByte();

uint8_t DetermineIfFactoryProgrammed();  
void write_config_eeproms();

uint8_t SystemState = SystemState_Init;
uint8_t StatusLEDState = SystemState_PowerOff;
uint8_t ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_INVALID;
uint8_t CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_GENERIC;
const uint8_t PowerStateInvalid = 0;
const uint8_t PowerStateOn = 1;
const uint8_t PowerStateOff = 2;
uint8_t PowerStateFPGA = PowerStateInvalid;

uint8_t I_AM_A_SECONDARY=false;

uint8_t ACTIVE_VIDEO_MODE_FORCED_ON = false;

void setup(){
  wdt_reset();
  wdt_disable();
  #ifdef LED_RGB
    LED.setOutput(LED_RGB);
    LED.setColorOrderGRB();
  #endif
  UpdateStatusLEDBuffer();
  
  Serial.begin(SERIAL_BAUD);
  
  SerialDebugln("\n" __DATE__ " " __TIME__ "");  
  SerialDebugln("PCB: "); board_print_name(); 
  SerialDebugln("CFG: "); panel_print_name(); 

  if ((!DetermineIfFactoryProgrammed())) {   do_factory_configuration();  }  // This is very early because we don't want anything to interfere with factory programming.  It should run standalone.
  SerialDebugln("\nSTART");
  power_down_receivers();
  regular_operation_transmitters();
  power_down_fpga();  
  SerialDebugln("\nINIT");
  /*SerialFlush();Disabled*/
  // Note: atmega chips power up with all pins in input mode (high impedance).  When first using a pin as an output, it must be declared as such.
 
// ENABLE THE WATCHDOG TIMER
//  SerialDebugln("\nINIT/WDT"); 
//  /*SerialFlush();Disabled*/ // Flush serial output in case somethinge goes horribly wrong, that way we guarantee seeing the last line of output.
//    configure_watchdog_timer();     
//  SerialDebugln("\t->OK");

  SerialDebugln("\nINIT/MISC\n\n"); 
  I_AM_A_SECONDARY=UserConfiguration_LoadWasSecondary();
    
  SerialDebugln("\nINIT/UARTs");   
  SerialToPanel.begin(PANEL_UART_SPEED);
  SerialToBldriver.begin(BACKLIGHT_UART_SPEED);
  SerialDebugln("\t->OK");   

  SerialDebugln("\nINIT/Backlight");   
  InitialConfigureBacklightPWM();
  SerialDebugln("\t->OK");   
  
  SerialDebugln("\nINIT/DONE\n\n"); 
  /*SerialFlush();Disabled*/
  FinishedConfigurationTime=millis();
  ButtonHoldTime = FinishedConfigurationTime;
}

void loop() {
  currentMillis = millis();
  HandleMillisInTimer0Overflow();
  wdt_reset();
  TaskFastest();
  if ((currentMillis - Task1ms_previousMillis) >= OneMillisecond) {
    Task1ms();
    Task1ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task10ms_previousMillis) >= TenMilliseconds) {
    Task10ms();
    Task10ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task100ms_previousMillis) >= HundredMilliseconds) {
    Task100ms();
    Task100ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task1000ms_previousMillis) >= OneSecond) {
    Task1000ms();
    Task1000ms_previousMillis = currentMillis;
  }

  if ((currentMillis - Task10000ms_previousMillis) >= TenSeconds) {
    Task10000ms();
    Task10000ms_previousMillis = currentMillis;
  }
}

void TaskFastest() {
}


void Task1ms(){ 
  handle_serial_commands();
  MaybeUpdateStatusLED();  
}

void Task10ms(){  
  Inputs.ReadPhysicalInputs();
  Inputs.RefilterInputState();   
  handle_button_state();
  HandleSystemState();
}

void Task100ms(){
    SendSerialStateToBldriver();
    DeterminePrimarySecondary();
}

void Task1000ms(){
  if(CheckVideoActive()==false) {  SerialDebug("\n VIDEO fail\n"); /*SerialFlush();Disabled*/ }
  Inputs.PrintState();
  SendSerialStateToPanel();
}

void Task10000ms(){
  PrintSystemState();
}

const uint32_t SystemStateHandlerCallRateMilliseconds = 10;
uint32_t SystemStateCounter = 0;
// Note : To keep timing of state transitions proper, call this function consistently.
void HandleSystemState(){
  // This function handles all 'forward' transitions.  Transitions to 'lower' states are handled elsewhere because these situations are caused by external events.
  if(SystemStateCounter <= SystemStateHandlerCallRateMilliseconds ) {
    SystemStateCounter = 0;
  } else {
    SystemStateCounter = SystemStateCounter - SystemStateHandlerCallRateMilliseconds;
  }

boolean PrintStateChanges = true;

    uint8_t myUserConfiguration_LoadShutdown = UserConfiguration_LoadShutdown();
    uint8_t myVideoActive = CheckVideoActive();
    switch (SystemState) {
      case SystemState_Init:     
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Init0"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_receivers();
          regular_operation_transmitters();
          SystemStateCounter=0;
          SystemState=SystemState_PowerOff;
        break;
      case SystemState_PowerOff:     
        if( myUserConfiguration_LoadShutdown!=TargetPowerSaveSHUTDOWN ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Off0"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          SystemStateCounter=SystemStateDelay_OffToRx;
          SystemState=SystemState_Rx;
        }
        break;
      case SystemState_Rx:
        if( (SystemStateCounter==0) && (myUserConfiguration_LoadShutdown==TargetPowerSaveFULLY_ON) ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Rx0"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_up_receivers();    
          SystemStateCounter=SystemStateDelay_RxToTx;
          SystemState=SystemState_Tx;
          break;
        }
        if( (myUserConfiguration_LoadShutdown==TargetPowerSaveSHUTDOWN) ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Rx1"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          SystemStateCounter=0;
          SystemState=SystemState_PowerOff;
          break;
        }    
        break;
      case SystemState_Tx:       
        if( (myUserConfiguration_LoadShutdown==TargetPowerSaveSHUTDOWN) ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Tx0"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_receivers();
          SystemState=SystemState_PowerOff;
          break;      
        }
        if( myUserConfiguration_LoadShutdown==TargetPowerSaveLOWPOWER ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Tx1"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_receivers();
          SystemState=SystemState_Rx;
          break;      
        }
        if(SystemStateCounter==0) {
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : Tx2"); /*SerialFlush();Disabled*/}
          BacklightDisable();
          regular_operation_transmitters();
          SystemStateCounter=SystemStateDelay_TxToPanel;
          SystemState=SystemState_Panel;
          break;    
        }  
        break;
      case SystemState_Panel:            
        if( (myUserConfiguration_LoadShutdown==TargetPowerSaveSHUTDOWN) ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : P0");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          force_sleep_transmitters();
          power_down_receivers();
          SystemStateCounter=0;
          SystemState=SystemState_PowerOff;
          break;
        }
        if( myUserConfiguration_LoadShutdown==TargetPowerSaveLOWPOWER ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : P1");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          force_sleep_transmitters();
          SystemStateCounter=SystemStateDelay_OffToRx;
          SystemState=SystemState_Rx;
          break;
        }
      
        if ( (SystemStateCounter==0) && (myVideoActive == true ) ) {
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : P2");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_up_fpga();
          SystemStateCounter=VIDEO_SIGNAL_TO_BACKLIGHT_ON_DELAY;
          SystemState=SystemState_Backlight;
          break;
        }  
      break;      
      case SystemState_Backlight: 
        if( (myUserConfiguration_LoadShutdown==TargetPowerSaveSHUTDOWN) ){
      if(PrintStateChanges) {SerialDebugln("SysStateTransition : B0");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          force_sleep_transmitters();
          SystemStateCounter=0;
          SystemState=SystemState_PowerOff;
          break;
        }
        if( myUserConfiguration_LoadShutdown==TargetPowerSaveLOWPOWER ){
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : B1");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          force_sleep_transmitters();
          SystemStateCounter=SystemStateDelay_OffToRx;
          SystemState=SystemState_Rx;
          break;
        }
        if ( myVideoActive == false ) {
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : B2");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          SystemStateCounter=0;
          SystemState=SystemState_Panel;
          break;
        }          
        if ( SystemStateCounter==0 ) {
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : B3");  /*SerialFlush();Disabled*/}
          BacklightEnable();
          SystemStateCounter=0;
          SystemState=SystemState_On;
          break;    
        }  
        break;
      case SystemState_On: 
        if( (myUserConfiguration_LoadShutdown==TargetPowerSaveSHUTDOWN) ){
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : ON0");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          force_sleep_transmitters();
          power_down_receivers();
          SystemStateCounter=0;
          SystemState=SystemState_PowerOff;
          break;
        }
        if( myUserConfiguration_LoadShutdown==TargetPowerSaveLOWPOWER ){
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : ON1");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          force_sleep_transmitters();
          SystemStateCounter=SystemStateDelay_OffToRx;
          SystemState=SystemState_Rx;
          break;
        }
        if ( myVideoActive == false ) {
          if(PrintStateChanges) {SerialDebugln("SysStateTransition : ON2");  /*SerialFlush();Disabled*/}
          BacklightDisable();
          power_down_fpga();
          SystemStateCounter=0;
          SystemState=SystemState_Panel;
          break;
        }          
        break;
      default:                
        if(PrintStateChanges) {SerialDebug("SysStateTransition : DEFAULTED :"); SerialDebuglnD(SystemState ); /*SerialFlush();Disabled*/}
        SystemState = SystemState_PowerOff;
        SystemStateCounter = 0;      
    }
 
}

void SendSerialStateToPanel(){  
  if(PowerStateFPGA == PowerStateOn){
  if(millis() < BlockPanelUARTSelfUpdate ) { return; }
  SerialToPanel.println(PanelInfoArray[PANEL_VERSION].Name);
  SerialToPanel.println(F("  OSD_OFF")); // Only needed for boards shipped in first batch in Jan2019, though it does not hurt to include in others
  // if(UserConfiguration_LoadOSD()){ SerialToPanel.println(F("OSD_ON"));} else {SerialToPanel.println(F("OSD_OFF"));}
  if(UserConfiguration_LoadCrosshair()){ SerialToPanel.println(F("XHAIR_EN"));} else {SerialToPanel.println(F("XHAIR_NO"));}
  }
}

void SendSerialStateToBldriver() {
  // The purpose of sending an on/off character command instead of relying on FSYNC to control the backlight driver is to avoid the situation where the bldriver might find itself in soft-off mode.
  if(SystemState == SystemState_On){
    SerialToBldriver.write(ASCII_CODE_FOR_POWER_ON);
  } else {    
    SerialToBldriver.write(ASCII_CODE_FOR_POWER_OFF);
  }
// SerialToBldriver.print(F("."); SerialToBldriver.println(PanelInfoArray[PANEL_VERSION].Name);  This functionality could result in undesired edge-cases.  Disable it.
}



void PrintSystemState(){  
  
SerialDebug("SystemState : "); 
  switch (SystemState) {
  case SystemState_PowerOff:     SerialDebugln("OFF");   break;
  case SystemState_Rx    :       SerialDebugln("RX");    break;
  case SystemState_Tx    :       SerialDebugln("TX");    break;
  case SystemState_Panel  :      SerialDebugln("Panel"); break;
  case SystemState_Backlight:    SerialDebugln("BL");    break;
  case SystemState_On:           SerialDebugln("ON");    break;
  default:                       SerialDebug("???");  SerialDebuglnD(SystemState);
  }
  
uint8_t myUserConfiguration_LoadShutdown=UserConfiguration_LoadShutdown();
SerialDebug("TargetState : "); 
  switch (UserConfiguration_LoadShutdown()) {
  case TargetPowerSaveSHUTDOWN:  SerialDebugln("OFF");  break;
  case TargetPowerSaveLOWPOWER:  SerialDebugln("LOW");  break;
  case TargetPowerSaveFULLY_ON:  SerialDebugln("ON");   break;
  default:                       SerialDebug("???");   SerialDebuglnD(myUserConfiguration_LoadShutdown);
  }

SerialDebug("BacklightState : "); 
  if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_ZWS){
    switch (ZWS_BACKLIGHT_MODE) {
    case ZWS_BACKLIGHT_MODE_PWM:    SerialDebugln("ZWS_PWM");  break;
    case ZWS_BACKLIGHT_MODE_NOPWM:  SerialDebugln("ZWS_PWMFREE");  break;
    case ZWS_BACKLIGHT_MODE_STROBE: SerialDebugln("ZWS_STROBE");   break;
    case ZWS_BACKLIGHT_MODE_SCAN:   SerialDebugln("ZWS_SCAN");   break;
    default:                        SerialDebugln("ZWS_INVALID");
    };
  } else {
    if(BacklightGetState() == true) {
      SerialDebugln("InternalPWM");
      SerialDebug("Brightness="); SerialDebuglnD(BacklightGetBrightness());
    } else {
      SerialDebugln("OFF");
    }
  }
}


void DeterminePrimarySecondary(){  
  #if BOARD_VERSION==BOARD_IS_EP369_REV2017
    uint8_t myPrimarySecondary=determine_if_Secondary();
    if(I_AM_A_SECONDARY != myPrimarySecondary){
      // We have changed roles, handle it appropriately
      I_AM_A_SECONDARY = myPrimarySecondary;
      UserConfiguration_SaveWasSecondary(I_AM_A_SECONDARY);
      PrimarySecondaryChangedStates();
    }
   #endif
}

void PrimarySecondaryChangedStates(){  
    SerialDebugln("Primary/Secondary detection changed");
    set_selected_edid(UserConfiguration_LoadEDID());
}

uint8_t determine_if_Secondary(){  
uint8_t retval=false;  
  #if BOARD_VERSION==BOARD_IS_EP369_REV2017
    pinMode ( PANEL_GPIO0, INPUT_PULLUP );    
    // The ZWS tcon uses a 1K pulldown on this signal.  This will easily overcome the on-chip pullup resistor and cause a 0 to be read.  The Secondary board will instead read a 1 because of the on-chip pullup.
    zdelay(10); // Let the line settle
    if(digitalRead(PANEL_GPIO0)==HIGH) { retval= true;}
    pinMode ( PANEL_GPIO0, OUTPUT );  
    #endif
  return retval;
}


uint8_t CheckVideoActive(){
  if(ACTIVE_VIDEO_MODE_FORCED_ON == true) {return LOW | HIGH; } //? ugly but it works
    uint8_t result = LOW;
    result |= digitalRead(ACTIVE_VIDEO_PRI);
    //SerialDebug("VideoActivePRI="); SerialDebuglnD(result);
#ifdef ACTIVE_VIDEO_SEC
    result |= digitalRead(ACTIVE_VIDEO_SEC);
    //SerialDebug("VideoActiveSEC="); SerialDebuglnD(result);
#endif
    return result;
}


// Replacement for 'delay' which doesn't totally lock up the system
void zdelay(uint32_t delayvalue){ 
  uint32_t remdel=delayvalue;
  while(remdel>0) {
    remdel=remdel-1; 
    handle_serial_commands();  
    wdt_reset();
    delay(1);
  }
}


void get_and_handle_buttons_nowait(){  
      Inputs.ReadPhysicalInputs();
      Inputs.RefilterInputState();      
      handle_button_state();
}


void get_and_handle_buttons(){  
      zdelay(BUTTON_SENSE_TIME);  // specifically do not touch watchdog here.  avoid zdelay
      get_and_handle_buttons_nowait();
}





void configure_watchdog_timer(){
  wdt_enable(MY_WATCHDOG_TIMEOUT); 
  wdt_reset();
}


void configure_watchdog_timer_slow(){
wdt_enable(WDTO_4S); 
  wdt_reset();
}



void power_down_fpga() {
  if(PowerStateFPGA != PowerStateOff){
    SerialDebugln("F-");
    pinMode(CONTROL_VREG_VPANEL,OUTPUT);  digitalWrite(CONTROL_VREG_VPANEL, LOW);
    PowerStateFPGA = PowerStateOff;
  }
}

void power_up_fpga() {
  if(PowerStateFPGA != PowerStateOn){
    SerialDebugln("F+");
    pinMode(CONTROL_VREG_VPANEL,OUTPUT);  digitalWrite(CONTROL_VREG_VPANEL, HIGH);
    PowerStateFPGA = PowerStateOn;
    uint32_t WhenToStopSpammingUART = millis() + MillisToSpamConfigUART;
    while(millis()< WhenToStopSpammingUART) {
        BlockPanelUARTSelfUpdate=0;
        SendSerialStateToPanel();
    }
  }
}

uint8_t PowerStateTx = PowerStateInvalid;
void force_sleep_transmitters() {
  if(PowerStateTx != PowerStateOff){
    SerialDebugln("Tx-");
    pinMode(LOW_POWER_MODE_PRI,OUTPUT); digitalWrite(LOW_POWER_MODE_PRI, HIGH);
    #ifdef LOW_POWER_MODE_SEC
    pinMode(LOW_POWER_MODE_SEC,OUTPUT); digitalWrite(LOW_POWER_MODE_SEC, HIGH);
    #endif
    PowerStateTx = PowerStateOff;
  }
}

void regular_operation_transmitters() {
  if(PowerStateTx != PowerStateOn){
    SerialDebugln("Tx+");
    pinMode(LOW_POWER_MODE_PRI,OUTPUT); digitalWrite(LOW_POWER_MODE_PRI, LOW);
#ifdef LOW_POWER_MODE_SEC
    pinMode(LOW_POWER_MODE_SEC,OUTPUT); digitalWrite(LOW_POWER_MODE_SEC, LOW);
#endif
    PowerStateTx = PowerStateOn;
  }
}

const uint32_t millis_between_dp_connections = 500;

uint8_t PowerStateRx = PowerStateInvalid;
void power_up_receivers() {
  if(PowerStateRx != PowerStateOn){
    SerialDebugln("Rx+");
    pinMode(RESET_OTHER_CHIPS_PRI,OUTPUT); digitalWrite(RESET_OTHER_CHIPS_PRI, HIGH); 
    #ifdef RESET_OTHER_CHIPS_SEC
    delay( millis_between_dp_connections );
    pinMode(RESET_OTHER_CHIPS_SEC,OUTPUT); digitalWrite(RESET_OTHER_CHIPS_SEC, HIGH);  
    #endif
    PowerStateRx = PowerStateOn;
  }
}

void power_down_receivers() {
  if(PowerStateRx != PowerStateOff){
    SerialDebugln("Rx-");
    #ifdef RESET_OTHER_CHIPS_SEC
    pinMode(RESET_OTHER_CHIPS_SEC,OUTPUT); digitalWrite(RESET_OTHER_CHIPS_SEC, LOW);
    delay( millis_between_dp_connections );  
    #endif    
    pinMode(RESET_OTHER_CHIPS_PRI,OUTPUT); digitalWrite(RESET_OTHER_CHIPS_PRI, LOW);
    PowerStateRx = PowerStateOff;
  }
}

void do_factory_configuration() {
  SerialDebugln("Fac");  
  power_down_fpga();
 // power_up_board();   while(1){;} // This line is useful when trying to prgram the internal ep369s mcu sometimes  
  /*SerialFlush();Disabled*/
  // Program the internal eeprom with safe default values
  UserConfiguration_SaveDefaultShutdown();
  UserConfiguration_SaveDefaultEDID();
  UserConfiguration_SaveDefaultBrightness();
  UserConfiguration_SaveDefaultOSD();
  UserConfiguration_SaveDefaultCrosshair();
  SerialDebugln("FacEEPROMs");  
  write_config_eeproms();
  SerialDebugln("FinishedDone_almost");   
  UserConfiguration_SaveDefaultMagicByte();
  SerialDebugln("FinishedFac");  
  softReset();
}


void softReset() {  
  SerialDebugln("\nRst!");  /*SerialFlush();Disabled*/
  noInterrupts();
  power_down_fpga();
  zdelay(500); // Ensure that the board is powered down sufficiently long to cause a full reset. 
  Serial.end();  
wdt_enable(WDTO_15MS); 
 while(1){}
//  asm volatile ("  jmp 0"); // Note: the jmp 0 does not load the bootloader, so we might not get the startup state that we expect.  Using watchdog is cleaner.
}


struct ParsedSerialCommand {
bool Valid;
bool XHAIR;
uint8_t EDID;
uint8_t PowerSave;
};

void handle_serial_commands() {
  while (Serial.available() > 0) {
    int incomingByte = Serial.read();      
    if (incomingByte > 0) { 
      uint8_t myChar = incomingByte & 0xff ;
      if(myChar>0x7f) { // All bytes with values above 127 are considered special zws serial commands        
        #if BOARD_VERSION==BOARD_IS_EP369_REV2017    
          struct ParsedSerialCommand myParsedSerialCommand=SerialCommandParser(myChar);
          PrintParsedSerialCommand(myChar);
          if(myParsedSerialCommand.Valid==false ) {return;}
          if(myParsedSerialCommand.XHAIR==true) {CrosshairToggleON();}  
          if(myParsedSerialCommand.XHAIR==false){CrosshairToggleOFF();}  
          if(myParsedSerialCommand.EDID!=UserConfiguration_LoadEDID()) { set_selected_edid(myParsedSerialCommand.EDID);}
          // Don't support off/low/on powerstates yet.  just power on
           if(myParsedSerialCommand.PowerSave==TargetPowerSaveFULLY_ON) {set_on_power_state(); }
           else {set_off_power_state(); }
        #endif
        return;
      } 
      switch (myChar) {         
        case ASCII_CODE_FOR_SIMPLE_DEBUG_COMMAND : ACTIVE_VIDEO_MODE_FORCED_ON = true; return;   
        case ASCII_CODE_FOR_BL_MODE_IS_PWM     : if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_GENERIC) { BacklightDisable();} CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_ZWS; ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_PWM; return; 
        case ASCII_CODE_FOR_BL_MODE_IS_NOPWM   : if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_GENERIC) { BacklightDisable();} CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_ZWS; ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_NOPWM; return;
        case ASCII_CODE_FOR_BL_MODE_IS_STROBE  : if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_GENERIC) { BacklightDisable();} CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_ZWS; ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_STROBE; return;
        case ASCII_CODE_FOR_BL_MODE_IS_SCAN    : if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_GENERIC) { BacklightDisable();} CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_ZWS; ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_SCAN; return;
        case ASCII_CODE_FOR_BL_MODE_IS_INVALID : if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_GENERIC) { BacklightDisable();} CONNECTED_BACKLIGHT = CONNECTED_BACKLIGHT_IS_ZWS; ZWS_BACKLIGHT_MODE = ZWS_BACKLIGHT_MODE_INVALID; return;
        default : MaybeSendUARTCharToPanel(myChar);
      }
    }
  }
}

void MaybeSendUARTCharToPanel(uint8_t myChar){
  if(PowerStateFPGA == PowerStateOn){
    BlockPanelUARTSelfUpdate = millis() + MillisBetweenForwardedUARTAndSerialStatusSending;
    SerialToPanel.write(myChar);
    //SerialDebug("Forwarded byte: ");  SerialWrite(myChar); SerialDebugln("");
  }
}

uint8_t ButtonHoldHandled = false;
uint8_t ButtonHoldMasking = false;
void handle_button_state() {  
// This optimization breaks on-release effects like saving the brightness and powering on/off the board, SO DON'T USE IT.
//  if (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_UNDEFINED || Inputs.GetCurrentFilteredInput() == BUTTONSTATE_NOTHING)               {                                          return; }  // exit early if there is nothing to do
  
    uint8_t  mypowerstate=UserConfiguration_LoadShutdown();
      // Handle 'soft' button presses via serial port   


    if(Inputs.GetCurrentFilteredInput() != Inputs.GetPreviousFilteredInput()){      
      ButtonHoldTime = millis();    
      ButtonHoldHandled = false;    
    }
    
    if((ButtonHoldHandled == false) && (Inputs.GetCurrentFilteredInput() == Inputs.GetPreviousFilteredInput())){
      uint32_t ButtonHeldTime = millis() - ButtonHoldTime;
      if ( (ButtonHeldTime > MillisToEnterSelfTest ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_POWER_BUTTON) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        EnterTestMode();
        return;
        }        
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_0) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(0);
        set_on_power_state();
        return;
        }       
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_1) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(1);
        set_on_power_state();
        return;
        }              
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_2) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(2);
        set_on_power_state();
        return;
        }            
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_3) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(3);
        set_on_power_state();
        return;
        }              
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_4) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(4);
        set_on_power_state();
        return;
        }             
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_5) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(5);
        return;
        }             
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_6) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(6);
        set_on_power_state();
        return;
        }             
      if ( (ButtonHeldTime > MillisToEnterEDIDUpdate ) && (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_EDID_7) ) {
        ButtonHoldHandled = true;
        ButtonHoldMasking = true;
        set_selected_edid(7);
        set_on_power_state();
        return;
        }       
    }
    
  if (
    ((Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_NOTHING) || (Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_UNDEFINED))
    &&    
    ((Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_NOTHING) || (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_UNDEFINED))
    )  { ButtonHoldMasking = false; }
  
  if(ButtonHoldMasking == true){ return; }
    
  // Handle regular button presses via button board    
  // Note: on button release to avoid cycling. 
  if (((Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_NOTHING) || (Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_UNDEFINED)))  {
    if (Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_POWER_BUTTON)  { toggle_power_state();               return; }
    if (Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_CROSSHAIR)     { CrosshairToggle();                  return; }
    if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_ZWS){
      if (Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_TOGGLE_STEREO_EYE)  { SerialToBldriver.write(ASCII_CODE_FOR_TOGGLE_STEREO_EYE); return;}
      if (Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_STROBE_ROTATE)  { SerialDebugln("SendRotateComamnd"); SerialToBldriver.write(ASCII_CODE_FOR_STROBE_ROTATE); return;}
    }
  }
  if(CONNECTED_BACKLIGHT == CONNECTED_BACKLIGHT_IS_ZWS){
    if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE ) { SerialToBldriver.write(ASCII_CODE_FOR_BRIGHTNESS_INCREASE); return; }
    if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE ) { SerialToBldriver.write(ASCII_CODE_FOR_BRIGHTNESS_DECREASE); return; } 
  } else {
  if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE )                                                                                      { BacklightIncrementBrightness();    return; }
  if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetCurrentFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE )                                                                                      { BacklightDecrementBrightness();    return; }
  if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_INCREASE && Inputs.GetCurrentFilteredInput() != COMMAND_CODE_FOR_BRIGHTNESS_INCREASE)          { BacklightIncrementBrightness(); BacklightSaveBrightness(); return; }
  if (mypowerstate==TargetPowerSaveFULLY_ON && Inputs.GetPreviousFilteredInput() == COMMAND_CODE_FOR_BRIGHTNESS_DECREASE && Inputs.GetCurrentFilteredInput() != COMMAND_CODE_FOR_BRIGHTNESS_DECREASE)          { BacklightDecrementBrightness(); BacklightSaveBrightness(); return; }
  }

}

void EnterTestMode(){
      SerialDebugln("ENTERING TEST MODE");
      ACTIVE_VIDEO_MODE_FORCED_ON = true;
      set_on_power_state();
      while (SystemState != SystemState_On) { HandleSystemState(); }
      zdelay(1000);
      SerialToPanel.println(" TESTPATTERNS"); 
      zdelay(1000);
      SerialToBldriver.write(ASCII_CODE_FOR_SIMPLE_DEBUG_COMMAND);
}

void toggle_power_state() {
//  SerialDebugln("PSTATE TOGGLE"); 
  if (UserConfiguration_LoadShutdown() != TargetPowerSaveFULLY_ON) {
    set_on_power_state();  
  }
  else {
    set_off_power_state();
  }
}

void set_off_power_state(){
  return set_low_power_state();
  // Do not support ultra-low-power state right now, because it screws with DP autodetection.  
}

void set_low_power_state(){
    UserConfiguration_SaveShutdown(TargetPowerSaveLOWPOWER);
}

void set_on_power_state(){
    UserConfiguration_SaveShutdown(TargetPowerSaveFULLY_ON);
}

void CrosshairToggle(){
  if(UserConfiguration_LoadCrosshair()==false) {CrosshairToggleON(); } else{ CrosshairToggleOFF();}
}

void CrosshairToggleON(){
  UserConfiguration_SaveCrosshair(true);
}

void CrosshairToggleOFF(){
  UserConfiguration_SaveCrosshair(false);
}

 void fastprinthexbyte(uint8_t hexbyte){   
    uint8_t mychar=hexbyte>>4;
    if(mychar<0x0A) { SerialWrite('0' + mychar);} else  { SerialWrite('7' + mychar);}
    mychar=hexbyte&0x0F;
    if(mychar<0x0A) { SerialWrite('0' + mychar);} else  { SerialWrite('7' + mychar);}
}

uint8_t which_edid_selected() {
  if (UserConfiguration_LoadEDID() >= EDIDMetaConfigCount ){
    return 0; // Default to EDID_0_SLOT
    }
  else {return UserConfiguration_LoadEDID();}
}

void rotate_selected_edid() {
  if (which_edid_selected() <( EDIDMetaConfigCount-1)) {
    UserConfiguration_SaveEDID(UserConfiguration_LoadEDID() + 1);
  }
  else {
    UserConfiguration_SaveEDID(0);
  }
  write_config_eeproms();
}

void set_selected_edid(uint8_t desired_edid) {  
  if(desired_edid<EDIDMetaConfigCount){ UserConfiguration_SaveEDID(desired_edid); }
  write_config_eeproms();
}


uint8_t ConfigGenerateEPMI(){
  uint8_t retval = 0x00;
   retval |= CONFIGMASK_EPMI_TMODE; // Set four LVDS channel output.
  // retval |= CONFIGMASK_EPMI_DMODE; // Split output into left/right tiles.
 //  retval |= CONFIGMASK_EPMI_RS; // Set high swing level output.
   retval |= CONFIGMASK_EPMI_LR; // Set left-right swap on LVDS channels.
  // retval |= CONFIGMASK_EPMI_MAP; // Set JEIDA color mapping mode on LVDS channels.

// DW1,DW0 WIDTH meaning: 00=10BIT, 10=8BIT, 01=6BIT, 11=POWERDOWN
    switch(PANEL_BIT_DEPTH) {
    case 6: retval=retval| CONFIGMASK_EPMI_DW1 ; break;// DW1,DW0=10
    case 8: retval=retval| CONFIGMASK_EPMI_DW0 ; break;// DW1,DW0=01
    case 10: retval=retval ; break; // DW1,DW0=00
    default: retval=retval | CONFIGMASK_EPMI_DW1| CONFIGMASK_EPMI_DW0; SerialDebugln("Bits??"); 
    } 

return retval;
}

void CheckEPMIConfig(){
  SerialDebug("EPMI_ADDR_SPECIAL="); fastprinthexbyte(ZWSMOD_EP369S_ADDRESS_SPECIAL);
  SerialDebug("ZWSMOD_EP369S_VALUE_SPECIAL="); fastprinthexbyte(ZWSMOD_EP369S_VALUE_SPECIAL);
  SerialDebug("ZWSMOD_EP369S_ADDRESS_CONFIGURATION="); fastprinthexbyte(ZWSMOD_EP369S_ADDRESS_CONFIGURATION);
  SerialDebug("ZWSMOD_EP369S_VALUE_CONFIGURATION="); fastprinthexbyte(ConfigGenerateEPMI());
}

  #if BOARD_VERSION==BOARD_IS_EP369_REV2017    
uint8_t GetByte(uint8_t address){
  return myEDID.GetByte(address);
}
#else
  
uint8_t GetBytePRI(uint8_t address){
  return myEDID_PRI.GetByte(address);
}  
uint8_t GetByteSEC(uint8_t address){
  return myEDID_SEC.GetByte(address);
}
#endif

void write_config_eeproms(){  
  #if DEBUG_OPTION_SKIP_EDID_PROGRAMMING == true
    return;
  #endif
  boolean doVerification = true;
    uint32_t dp_rx_shutdown_millis = millis();
    const uint32_t millis_disconnect_for_dp_edid_change = 1000;
    const uint32_t millis_disconnect_for_zeroed_edid = 1000;
    power_down_receivers();

    uint8_t TargetProfile = UserConfiguration_LoadEDID();
    uint8_t SerialNumber = MagicByte();
    uint16_t failureCount = 0;

  #if BOARD_VERSION==BOARD_IS_EP369_REV2017    
    myEDID.Reset();
    update_eeprom(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetByte); 
    power_up_receivers();  
    delay(millis_disconnect_for_zeroed_edid);
    power_down_receivers();
    GenerateEDIDWithParameters(not I_AM_A_SECONDARY, ENABLE_SECONDARY_INPUT_TO_BE_USED_DURING_SINGLE_INPUT_MODE, PANEL_VERSION, TargetProfile, SerialNumber, &myEDID);
    myEDID.PrintEDID();   
    myEDID.SetByte(ZWSMOD_EP369S_ADDRESS_SPECIAL, ZWSMOD_EP369S_VALUE_SPECIAL);
    myEDID.SetByte(ZWSMOD_EP369S_ADDRESS_CONFIGURATION, ConfigGenerateEPMI());  
    failureCount = update_eeprom(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetByte);     
    if(failureCount > 0) { SerialDebugln("\nEEPROM update failed!");}
    if((failureCount == 0) && (doVerification==true)){
      failureCount = verify_eeprom_byte_mode(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetByte);     
      if(failureCount > 0) { SerialDebugln("\nEEPROM verify failed!");}
    }
  #else

  
    power_down_receivers();
    myEDID_PRI.Reset();
    myEDID_SEC.Reset();
    update_both_eeproms_byte_mode_write_only_differences(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetBytePRI, &my_SoftIIC_EDID_SEC, EDID_IIC_ADDRESS, GetByteSEC);
    power_up_receivers();  
    delay(millis_disconnect_for_zeroed_edid);
    power_down_receivers();    
    GenerateEDIDWithParameters(true, ENABLE_SECONDARY_INPUT_TO_BE_USED_DURING_SINGLE_INPUT_MODE, PANEL_VERSION, TargetProfile, SerialNumber, &myEDID_PRI);
    GenerateEDIDWithParameters(false, ENABLE_SECONDARY_INPUT_TO_BE_USED_DURING_SINGLE_INPUT_MODE, PANEL_VERSION, TargetProfile, SerialNumber, &myEDID_SEC);
    SerialDebug("\nPrimaryEDID=");
    #if (SERIAL_DEBUGGING_OUTPUT == ENABLED)
      myEDID_PRI.PrintEDID();  
    #endif 
    SerialDebug("\nSecondaryEDID=");
    #if (SERIAL_DEBUGGING_OUTPUT == ENABLED)
      myEDID_SEC.PrintEDID();   
    #endif
    myEDID_PRI.SetByte(ZWSMOD_EP369S_ADDRESS_SPECIAL, ZWSMOD_EP369S_VALUE_SPECIAL);
    myEDID_PRI.SetByte(ZWSMOD_EP369S_ADDRESS_CONFIGURATION, ConfigGenerateEPMI());  
    myEDID_SEC.SetByte(ZWSMOD_EP369S_ADDRESS_SPECIAL, ZWSMOD_EP369S_VALUE_SPECIAL);
    myEDID_SEC.SetByte(ZWSMOD_EP369S_ADDRESS_CONFIGURATION, ConfigGenerateEPMI());  
    
    failureCount = update_both_eeproms_byte_mode_write_only_differences(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetBytePRI, &my_SoftIIC_EDID_SEC, EDID_IIC_ADDRESS, GetByteSEC);
    if((failureCount == 0) && (doVerification==true)){
      failureCount = verify_eeprom_byte_mode(&my_SoftIIC_EDID_PRI, EDID_IIC_ADDRESS, GetBytePRI);     
      if(failureCount > 0) { SerialDebugln("\nPRI EEPROM verify failed!");}  
      failureCount = verify_eeprom_byte_mode(&my_SoftIIC_EDID_SEC, EDID_IIC_ADDRESS, GetByteSEC);     
      if(failureCount > 0) { SerialDebugln("\nSEC EEPROM verify failed!");}
    }
  #endif
    while ((millis() - dp_rx_shutdown_millis) < millis_disconnect_for_dp_edid_change) {wdt_reset();}
    power_up_receivers();  
}

uint16_t update_eeprom(SoftIIC* my_SoftIIC, uint8_t eeprom_address, uint8_t (*fp_virtualeeprom)(uint8_t address )){
    // Try page mode twice, then use byte mode as a fallback method
    uint16_t retval=0;
    //retval=update_eeprom_page_mode( my_SoftIIC, eeprom_address,fp_virtualeeprom); if(retval==0) {goto end_of_update_eeprom_function;}
    //retval=update_eeprom_page_mode( my_SoftIIC, eeprom_address,fp_virtualeeprom); if(retval==0) {goto end_of_update_eeprom_function;}
    //retval=update_eeprom_byte_mode( my_SoftIIC, eeprom_address,fp_virtualeeprom);
    retval=update_eeprom_byte_mode_write_only_differences( my_SoftIIC, eeprom_address,fp_virtualeeprom);

//    end_of_update_eeprom_function:
//    my_SoftIIC->MasterDumpRegisters(eeprom_address); // This should normally be commented out    
    return retval;
}


const uint8_t EEPROM_WRITE_TIME = 5+1;   // Time between i2c eeprom writes, needed for 24c02/24c08 to complete internal operations.  6ms works fine.  Slower to give status led more time to blink
const uint8_t BlinkDividerMax = 8; // Make flashing of status LED more visible
uint8_t update_eeprom_page_mode(SoftIIC* my_SoftIIC, uint8_t eeprom_address, uint8_t (*fp_virtualeeprom)(uint8_t address )){
    wdt_reset();  
    SerialDebug("Page8PROG: IIC 0x"); fastprinthexbyte(eeprom_address); SerialDebug("\t"); /*SerialFlush();Disabled*/
    const uint8_t pagesize=8;
    uint8_t bytearray[pagesize];
    uint8_t tretval=0xff;
    uint16_t failures = 0;
    uint16_t current_address = 0;
    current_address = 0;
    uint8_t BlinkDivider = BlinkDividerMax;
    while(current_address <= 0xff ){            
        /*SerialFlush();Disabled*/
        for(uint16_t addr=0; addr<pagesize; addr++){bytearray[addr]=fp_virtualeeprom(current_address+addr);} 
        tretval = my_SoftIIC->MasterWritePage(eeprom_address, current_address, pagesize, 1, bytearray ); 
        if ( tretval != 0) {        failures++;   SerialDebug("*");   } else {SerialDebug(".");}
        current_address=current_address+pagesize;
        zdelay(EEPROM_WRITE_TIME);  // Time between writes
        if(BlinkDivider==0){
          BlinkDivider = BlinkDividerMax;
          BlinkStateLED();
        } else { 
          BlinkDivider = BlinkDivider -1;
        }
    }
    SerialDebug(" IIC writes done, "); SerialDebugD(failures); SerialDebugln(" errors.");
    return failures;
}

uint8_t update_eeprom_byte_mode(SoftIIC* my_SoftIIC, uint8_t eeprom_address, uint8_t (*fp_virtualeeprom)(uint8_t address )){
    wdt_reset();
    SerialDebug("BytePROG: IIC 0x"); fastprinthexbyte(eeprom_address); SerialDebug("\t"); /*SerialFlush();Disabled*/
    uint8_t tretval=0xff;
    uint16_t failures = 1;
    uint16_t current_address = 0;
    uint8_t BlinkDivider = BlinkDividerMax;
    uint8_t retries_remaining = 3;
    while ((failures > 0) && (retries_remaining>0) ) {
        failures = 0;
        retries_remaining = retries_remaining -1;        
        for (current_address = 0; current_address <= 0xff; current_address++ ) {
            /*SerialFlush();Disabled*/
            tretval=my_SoftIIC->MasterWriteByte( eeprom_address,  current_address,  fp_virtualeeprom(current_address), 1 );
            if ( tretval != 0) {        failures++;   SerialDebug("*");   } else {SerialDebug(".");}     
            zdelay(EEPROM_WRITE_TIME);  // Time between writes
            if(BlinkDivider==0){
              BlinkDivider = BlinkDividerMax;
              BlinkStateLED();
            } else { 
              BlinkDivider = BlinkDivider -1;
            }
        }
        SerialDebug(" IIC writes done, "); SerialDebugD(failures); SerialDebugln(" errors.");

    }
    return failures;
}


uint16_t update_eeprom_byte_mode_write_only_differences(SoftIIC* my_SoftIIC, uint8_t eeprom_address, uint8_t (*fp_virtualeeprom)(uint8_t address )){
    wdt_reset();
    SerialDebug("ByteProg: IIC 0x"); fastprinthexbyte(eeprom_address); SerialDebug("\t"); /*SerialFlush();Disabled*/
    uint8_t tretval=0xff;
    uint16_t failures = 1;
    uint16_t current_address = 0;
    uint8_t BlinkDivider = BlinkDividerMax;
    uint8_t retries_remaining = 3;
    while ((failures > 0) && (retries_remaining>0) ) {
        failures = 0;
        retries_remaining = retries_remaining -1;        
        for (current_address = 0; current_address <= 0xff; current_address++ ) {
            /*SerialFlush();Disabled*/
            tretval=my_SoftIIC->MasterReadByte( eeprom_address,  current_address );
            if(tretval!=fp_virtualeeprom(current_address)){
              tretval=my_SoftIIC->MasterWriteByte( eeprom_address,  current_address,  fp_virtualeeprom(current_address), 1 );
              if ( tretval != 0) {        failures++;   SerialDebug("*");   } else {SerialDebug(".");}     
              zdelay(EEPROM_WRITE_TIME);  // Time between writes
            }
            if(BlinkDivider==0){
              BlinkDivider = BlinkDividerMax;
              BlinkStateLED();
            } else { 
              BlinkDivider = BlinkDivider -1;
            }
        }
        SerialDebug(" IIC writes done, "); SerialDebugD(failures); SerialDebugln(" errors.");
    }
    return failures;
}


uint16_t update_both_eeproms_byte_mode_write_only_differences(SoftIIC* my_SoftIIC_PRI, uint8_t eeprom_address_PRI, uint8_t (*fp_virtualeeprom_PRI)(uint8_t address ), SoftIIC* my_SoftIIC_SEC, uint8_t eeprom_address_SEC, uint8_t (*fp_virtualeeprom_SEC)(uint8_t address )){
    wdt_reset();
    SerialDebug("ByteProgBoth"); SerialDebug("\t"); /*SerialFlush();Disabled*/
    uint8_t tretval_PRI=0xff;
    uint8_t tretval_SEC=0xff;
    uint16_t failures_PRI = 1;
    uint16_t failures_SEC = 1;
    uint16_t current_address = 0;
    uint8_t BlinkDivider = BlinkDividerMax;
    uint8_t retries_remaining = 3;
    boolean wrotePRI = false;
    boolean wroteSEC = false;
    while ((failures_PRI + failures_SEC > 0) && (retries_remaining>0) ) {
        failures_PRI = 0;
        failures_SEC = 0;
        retries_remaining = retries_remaining -1;   
        wrotePRI=false;     
        wroteSEC=false;     
        for (current_address = 0; current_address <= 0xff; current_address++ ) {
            /*SerialFlush();Disabled*/
            tretval_PRI=my_SoftIIC_PRI->MasterReadByte( eeprom_address_PRI,  current_address );
            if(tretval_PRI!=fp_virtualeeprom_PRI(current_address)){
              tretval_PRI=my_SoftIIC_PRI->MasterWriteByte( eeprom_address_PRI,  current_address,  fp_virtualeeprom_PRI(current_address), 1 );
              wrotePRI=true;
              if(tretval_PRI != 0){ failures_PRI++;}
            }
            tretval_SEC=my_SoftIIC_SEC->MasterReadByte( eeprom_address_SEC,  current_address );
            if(tretval_SEC!=fp_virtualeeprom_SEC(current_address)){
              tretval_SEC=my_SoftIIC_SEC->MasterWriteByte( eeprom_address_SEC,  current_address,  fp_virtualeeprom_SEC(current_address), 1 );
              wroteSEC=true;        
              if(tretval_SEC != 0){ failures_SEC++;}
            }

            // Decoder:
            // _ Wrote nothing
            // . Wrote PRI, no error
            // , Wrote SEC, no error
            // ~ Wrote both, no error
            // / Error writing PRI
            // \ Error writing SEC
            // # Error writing both

            uint8_t statusCharacter = '?';
            if((wrotePRI==false) && (wroteSEC==false)) {statusCharacter='_';}
            if((wrotePRI==true) && (tretval_PRI == 0) && (wroteSEC==false)) {statusCharacter='.';}
            if((wrotePRI==false)&& (wroteSEC==true) && (tretval_SEC == 0)) {statusCharacter=',';}
            if((wrotePRI==true) && (tretval_PRI == 0) && (wroteSEC==true) && (tretval_SEC == 0)) {statusCharacter='~';}            
            if((wrotePRI==true) && (tretval_PRI != 0)) {statusCharacter='/';}
            if((wroteSEC==true) && (tretval_SEC != 0)) {statusCharacter='\\';}
            if((wrotePRI==true) && (tretval_PRI != 0) && (wroteSEC==true) && (tretval_SEC != 0)) {statusCharacter='#';}            
            SerialWrite(statusCharacter);
            if((wrotePRI==true) || (wroteSEC==true)){ delay(EEPROM_WRITE_TIME);}
            if(BlinkDivider==0){
              BlinkDivider = BlinkDividerMax;
              BlinkStateLED();
            } else { 
              BlinkDivider = BlinkDivider -1;
            }
        }
        SerialDebug(" IIC writes done, "); SerialDebugD(failures_PRI);  SerialDebug(" : "); SerialDebugD(failures_SEC); SerialDebugln(" errors.");
    }
    return failures_PRI + failures_SEC;
}


uint16_t verify_eeprom_byte_mode(SoftIIC* my_SoftIIC, uint8_t eeprom_address, uint8_t (*fp_virtualeeprom)(uint8_t address )){
    wdt_reset();
    uint8_t tretval;
    uint16_t failures = 0;
    uint16_t current_address = 0;
    for (current_address = 0; current_address <= 0xff; current_address++ ) {
        tretval=my_SoftIIC->MasterReadByte( eeprom_address,  current_address );
        if(tretval!=fp_virtualeeprom(current_address)){failures = failures +1;}
    }
    return failures;
}


  
  
void UpdateStatusLEDBuffer(){
  #ifdef LED_STATUS
    if(SystemState==SystemState_On){ StatusLEDState = SystemState_On;}
    else { StatusLEDState = SystemState_PowerOff;}
  #endif
  #ifdef LED_RGB
  uint8_t LED_STATE_INDICATOR;
  switch (SystemState) {
    case SystemState_Init    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_ERROR; break;
    case SystemState_PowerOff    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_POWERSAVE; break;
    case SystemState_Rx    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_POWERSAVE; break;
    case SystemState_Tx    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_NO_INPUT; break;
    case SystemState_Panel    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_NO_INPUT; break;
    case SystemState_Backlight    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_ERROR; break;
    case SystemState_On    : 
      switch (ZWS_BACKLIGHT_MODE) {
        case ZWS_BACKLIGHT_MODE_PWM    : LED_STATE_INDICATOR = LED_STATE_INDICATOR_PWM; break;
        case ZWS_BACKLIGHT_MODE_NOPWM  : LED_STATE_INDICATOR = LED_STATE_INDICATOR_PWMFREE; break;
        case ZWS_BACKLIGHT_MODE_STROBE : LED_STATE_INDICATOR = LED_STATE_INDICATOR_STROBING; break;
        case ZWS_BACKLIGHT_MODE_SCAN   : LED_STATE_INDICATOR = LED_STATE_INDICATOR_SCANNING; break;
        default                        : LED_STATE_INDICATOR = LED_STATE_INDICATOR_SOFTADJUST ;      
      }
      break;
    default                        : LED_STATE_INDICATOR = LED_STATE_INDICATOR_ERROR ;   
  }
  UpdateStatusLEDBuffer(LED_STATE_INDICATOR);
  #endif
}

void UpdateStatusLEDBuffer(uint8_t myLEDState){
  #ifdef LED_RGB
  value.r=LED_STATE_INDICATOR_COLORS[myLEDState].r;
  value.g=LED_STATE_INDICATOR_COLORS[myLEDState].g;
  value.b=LED_STATE_INDICATOR_COLORS[myLEDState].b;
  LED.set_crgb_at(0, value); // Set value at LED found at index 0  
  #endif
}

uint8_t SkipSameLEDCount=0;
const uint8_t ResendLED = 250; // Reduce same-state retransmits by this ratio

void MaybeUpdateStatusLED(){
  UpdateStatusLEDBuffer();
  if((value.r==valuePrev.r) && (value.g==valuePrev.g) && (value.b==valuePrev.b) && (SkipSameLEDCount >0)) {
    SkipSameLEDCount=SkipSameLEDCount-1;  
  } else {
    valuePrev=value;
    SkipSameLEDCount=ResendLED;
    UpdateStateLED();
  }     
}



uint8_t IsSerialPortBusy(){
  return 0;
  return ((UCSR0A & (1<<TXC0))>0) ;  
}

void BlinkStateLED(){ // Used when changing EDIDs
  if(StatusLEDState == SystemState_On){
    UpdateStatusLEDBuffer(LED_STATE_INDICATOR_ERROR);
    StatusLEDState = SystemState_PowerOff;
  } else {
    UpdateStatusLEDBuffer(LED_STATE_INDICATOR_POWERSAVE); 
    StatusLEDState = SystemState_On;   
  }
    UpdateStateLED();
}

void UpdateStateLED(){ 
  #ifdef LED_STATUS
    pinMode(LED_STATUS, OUTPUT);
    if(StatusLEDState==SystemState_On){ digitalWrite(LED_STATUS, HIGH);}
    else digitalWrite(LED_STATUS, LOW);
  #endif
  
  #ifdef LED_RGB
    if(IsSerialPortBusy()==0){      
     UCSR0B = UCSR0B & ~(1<<TXEN0); 
     delayMicroseconds(SK6812_RESET_TIME); 
     LED.sync(); // Sends the data to the LEDs
     delayMicroseconds(SK6812_POST_IDLE_TIME);
     UCSR0B |= (1<<TXEN0);
   }
   #endif
}

