uint8_t BacklightBrightness = 0;
uint8_t BacklightIsOn = false;

void InitialConfigureBacklightPWM(){
  TIMSK2=0x00; // Disable interrupts here
  TCCR2A=0x00;
  TCCR2B=0x00;
  //  Set fastPWM 8bit mode for each timer
  TCCR2A=0x00|(1<<WGM21)|(1<<WGM20);
  TCCR2B=DetermineCorrectTimer2Divider(BacklightInfo.PWM_DIVIDER);
  BacklightDisable();  
  BacklightBrightness=UserConfiguration_LoadBrightness();
}

uint8_t DetermineCorrectTimer2Divider(uint16_t myTargetDivision){
  switch (myTargetDivision) {
  case     1 : return 0x01; // 31.25KHz
  case     8 : return 0x02; // 3.9KHz
  case    32 : return 0x03; // 1KHz
  case    64 : return 0x04;
  case   128 : return 0x05;
  case   256 : return 0x06;
  case  1024 : return 0x07;
  default    : return 0x00;
  };
}


void BacklightDisable(){  
  if(CONNECTED_BACKLIGHT != CONNECTED_BACKLIGHT_IS_GENERIC) {return;}
  SerialDebugln(F("BL-"));
  pinMode(BLPIN_BLON, OUTPUT); digitalWrite(BLPIN_BLON, not BacklightInfo.BACKLIGHT_ENABLE_POLARITY);
  pinMode(BLPIN_PWM, OUTPUT); digitalWrite(BLPIN_PWM, not BacklightInfo.BACKLIGHT_PWM_POLARITY);
  BacklightIsOn = false;
}

void BacklightEnable(){
  if(CONNECTED_BACKLIGHT != CONNECTED_BACKLIGHT_IS_GENERIC) {return;}
  SerialDebugln(F("BL+"));
  BacklightIsOn = true;
  pinMode(BLPIN_BLON, OUTPUT); digitalWrite(BLPIN_BLON, BacklightInfo.BACKLIGHT_ENABLE_POLARITY);
  BacklightSetBrightness();
}

uint8_t BacklightGetState(){
  return BacklightIsOn;
}

uint8_t BacklightGetBrightness(){
  return BacklightBrightness;
}

void BacklightIncrementBrightness(){
  if(BacklightIsOn == false) {return;}
  if(BacklightBrightness<BacklightInfo.PWM_MAX_DUTYCYCLE){
    BacklightBrightness=BacklightBrightness+1;
  }
  BacklightSetBrightness();
}

void BacklightDecrementBrightness(){
  if(BacklightIsOn == false) {return;}
  if(BacklightBrightness>BacklightInfo.PWM_MIN_DUTYCYCLE){
    BacklightBrightness=BacklightBrightness-1;
  }
  BacklightSetBrightness();
}

void BacklightSetBrightness(){
  if(BacklightIsOn == false) {return;}
  if(CONNECTED_BACKLIGHT != CONNECTED_BACKLIGHT_IS_GENERIC) {return;}
  uint8_t PinBrightness = BacklightBrightness;
  if(BacklightInfo.BACKLIGHT_PWM_POLARITY == LOW) { PinBrightness = 255 - BacklightBrightness; }
  pinMode(BLPIN_PWM, OUTPUT); analogWrite(BLPIN_PWM, PinBrightness);  
}

void BacklightSaveBrightness(){
  UserConfiguration_SaveBrightness(BacklightBrightness);  
}
