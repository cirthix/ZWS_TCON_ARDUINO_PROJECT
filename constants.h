#ifndef CONSTANTS_h
#define CONSTANTS_h

#include <Arduino.h>

const uint32_t SERIAL_BAUD = 38400; // Set to 0 to disable serial port and all debug output.
const uint32_t PANEL_UART_SPEED = 115200;
const uint32_t BACKLIGHT_UART_SPEED = 9600;

const uint8_t SK6812_RESET_TIME = 150; // UNITS=microseconds
const uint8_t SK6812_POST_IDLE_TIME = 10; // UNITS=microseconds
const uint8_t SK6812_WRITE_TIME = 2; // UNITS=microseconds
const uint8_t SK6812_TOTAL_TIME = SK6812_RESET_TIME+SK6812_POST_IDLE_TIME+SK6812_WRITE_TIME; // UNITS=microseconds

#define IIC_SPEED 50 // Speed in KHz
#define EDID_IIC_ADDRESS 0x50
#define I2C_TIMEOUT 10
#define MY_WATCHDOG_TIMEOUT WDTO_60MS

#define ACTIVE_LOW  LOW
#define ACTIVE_HIGH HIGH

#ifndef ENABLED
#define ENABLED 1 
#endif

#ifndef DISABLED
#define DISABLED 0
#endif

#define SERIAL_DEBUGGING_OUTPUT DISABLED
// conditional debugging
#if (SERIAL_DEBUGGING_OUTPUT == ENABLED)
#define SerialDebug(x)      Serial.print(x);   Serial.flush();
#define SerialDebugln(x)    Serial.println(x); Serial.flush();
#define SerialWrite(x)      Serial.write(x);   Serial.flush();
#define SerialFlush()       Serial.flush()
#else
#define SerialDebug(x)      ((void) 0)
#define SerialDebugln(x)    ((void) 0)
#define SerialWrite(x)      ((void) 0)
#define SerialFlush()       ((void) 0)
#endif

// These are indexes in PanelInfoArray
#define PANEL_IS_M280GJC1ZIS  0
#define PANEL_IS_M280GJC3ZIS  1
#define PANEL_IS_M280GJQDZIS  2
#define PANEL_IS_V390DKZIS    3
#define PANEL_IS_V400DKZIS    4
#define PANEL_IS_V420DKZIS    5 
#define PANEL_IS_V500DKZIS    6
#define PANEL_IS_V580DKZIS    7

#define BUTTONBOARD_IS_ZISWORKS   100
#define BUTTONBOARD_IS_SAMSUNG    101
#define BUTTONBOARD_IS_SAMSUNG_WITH_RGBLED    102


#define BOARD_IS_DUAL_EP369_TCON        200
#define BOARD_IS_EP369_REV2017          201

//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////
#define PANEL_VERSION PANEL_IS_M280GJQDZIS
#define BUTTONBOARD_VERSION BUTTONBOARD_IS_SAMSUNG
#define BOARD_VERSION BOARD_IS_DUAL_EP369_TCON
// Note that you also have the option to change EDID configurations in edid_construction file
// In particular, #define EDIDMetaConfigs EDIDMetaConfig_safe_configuration may be useful to change
//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////

// Note: use FIRMWARE_UNIQUE_ID_OVERRIDE with old boards (R1 kits) to keep the serial numbers in EDIDs matched.
#if BOARD_VERSION == BOARD_IS_EP369_REV2017
  #define FIRMWARE_UNIQUE_ID_OVERRIDE 123
#endif


#include "SUPPORTED_BOARDS.h"
#include "SUPPORTED_PANELS.h"
#include "SUPPORTED_BLDRIVERS.h"
#include "SUPPORTED_BUTTONBOARDS.h"

const uint16_t ADDRESS_MAGIC_BYTE               = 16;
const uint16_t ADDRESS_POWER_STATE              = 17;
const uint16_t ADDRESS_SELECTED_EDID            = 18;
const uint16_t ADDRESS_BACKLIGHT_LEVEL          = 19;
const uint16_t ADDRESS_OSD_ENABLED              = 20;
const uint16_t ADDRESS_CROSSHAIR_ENABLED        = 21;
const uint16_t ADDRESS_WAS_SECONDARY                = 22;

const uint8_t BUTTON_SENSE_TIME = 5;   // 1 TICK IS 1 MICROCSECOND

const uint8_t TargetPowerSaveSHUTDOWN = 0; // Shutdown disables the dp recievers
const uint8_t TargetPowerSaveLOWPOWER = 1; // Lowpower mode disables the fpga, and by extension, the backlight
const uint8_t TargetPowerSaveFULLY_ON = 2; // System fully operational

const uint32_t SystemStateDelay_OffToRx = 10;
const uint32_t SystemStateDelay_RxToTx = 10;
const uint32_t SystemStateDelay_TxToPanel = 10;

const uint8_t SystemState_Init = 0 ;
const uint8_t SystemState_PowerOff = 1 ;
const uint8_t SystemState_Rx = 2 ;
const uint8_t SystemState_Tx = 3 ;
const uint8_t SystemState_Panel = 4 ;
const uint8_t SystemState_Backlight = 5 ;
const uint8_t SystemState_On = 6 ;

const uint8_t BACKLIGHT_MODE_OFF = 1;
const uint8_t BACKLIGHT_MODE_STABLE = 2;

// THIS MUST EXACTLY MATCH THE EP369S INTERNAL FIRMWARE VALUE
const uint8_t ZWSMOD_EP369S_ADDRESS_SPECIAL = 0x00;
const uint8_t ZWSMOD_EP369S_VALUE_SPECIAL = 0x01;
const uint8_t ZWSMOD_EP369S_ADDRESS_CONFIGURATION = 0x01;
const uint8_t CONFIGMASK_EPMI_DW0   = 0b00000001;
const uint8_t CONFIGMASK_EPMI_DW1   = 0b00000010;
const uint8_t CONFIGMASK_EPMI_MAP   = 0b00000100;
const uint8_t CONFIGMASK_EPMI_LR    = 0b00001000;
const uint8_t CONFIGMASK_EPMI_EO    = 0b00010000;
const uint8_t CONFIGMASK_EPMI_DMODE = 0b00100000;
const uint8_t CONFIGMASK_EPMI_TMODE = 0b01000000;
const uint8_t CONFIGMASK_EPMI_RS    = 0b10000000;

const uint8_t FACTORY_DEFAULT_BACKLIGHT_BRIGHTNESS = DEFAULT_BRIGHTNESS_LEVEL ; // This default level is set in SUPPORTED_BLDRIVERS.h
const uint8_t FACTORY_DEFAULT_POWERSTATE = TargetPowerSaveFULLY_ON ;
const uint8_t FACTORY_DEFAULT_SELECTED_EDID = 0;
const uint8_t FACTORY_DEFAULT_USE_CROSSHAIR = false;
const uint8_t FACTORY_DEFAULT_USE_OSD = false;

const uint16_t OneMillisecond        =1;
const uint16_t TenMilliseconds       =10;
const uint16_t HundredMilliseconds   =100;
const uint16_t OneSecond             =1000;
const uint16_t TenSeconds            =10000;

const uint8_t EDIDMetaConfigCount = 4;

const uint8_t ZWS_BACKLIGHT_MODE_INVALID = 0;
const uint8_t ZWS_BACKLIGHT_MODE_PWM = 1;
const uint8_t ZWS_BACKLIGHT_MODE_NOPWM = 2;
const uint8_t ZWS_BACKLIGHT_MODE_STROBE = 3;
const uint8_t ZWS_BACKLIGHT_MODE_SCAN = 4;

const uint8_t CONNECTED_BACKLIGHT_IS_GENERIC = 0;
const uint8_t CONNECTED_BACKLIGHT_IS_ZWS = 1;

const uint8_t LED_STATE_INDICATOR_MAX_PWM_VALUE  = 16;  // The LED is too bright for indicating status!
const uint8_t LED_STATE_INDICATOR_ERROR       = 0;
const uint8_t LED_STATE_INDICATOR_POWERSAVE   = 1;
const uint8_t LED_STATE_INDICATOR_NO_INPUT    = 2;
const uint8_t LED_STATE_INDICATOR_PWM         = 3;
const uint8_t LED_STATE_INDICATOR_PWMFREE     = 4;
const uint8_t LED_STATE_INDICATOR_STROBING    = 5;
const uint8_t LED_STATE_INDICATOR_SCANNING    = 6;
const uint8_t LED_STATE_INDICATOR_SOFTADJUST  = 7;

struct LED_STATE_INDICATOR_COLOR { uint8_t r;  uint8_t g;  uint8_t b; };

const LED_STATE_INDICATOR_COLOR LED_STATE_INDICATOR_COLORS[] = {
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // WHITE  // LED_STATE_INDICATOR_ERROR      = 0
  { .r =                0                 , .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b =                0                  }, // GREEN  // LED_STATE_INDICATOR_POWERSAVE  = 1
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g =                0                 , .b =                0                  }, // RED    // LED_STATE_INDICATOR_NO_INPUT   = 2
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b =                0                  }, // YELLOW // LED_STATE_INDICATOR_PWM        = 3
  { .r =                0                 , .g = LED_STATE_INDICATOR_MAX_PWM_VALUE, .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // CYAN   // LED_STATE_INDICATOR_PWMFREE    = 4
  { .r =                0                 , .g =                0                 , .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // BLUE   // LED_STATE_INDICATOR_STROBING   = 5
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g =                0                 , .b = LED_STATE_INDICATOR_MAX_PWM_VALUE }, // PURPLE // LED_STATE_INDICATOR_SCANNING   = 6
  { .r = LED_STATE_INDICATOR_MAX_PWM_VALUE, .g = LED_STATE_INDICATOR_MAX_PWM_VALUE * 165 / 255    , .b = 0}                   // ORANGE // LED_STATE_INDICATOR_SOFTADJUST = 7
};


#endif
