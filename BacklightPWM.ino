
void InitialConfigureBacklightPWM(){
  TIMSK2=0x00; // Disable interrupts here
  TCCR2A=0x00;
  TCCR2B=0x00;
  //  Set fastPWM 8bit mode for each timer
  TCCR2A=0x00|(1<<WGM21)|(1<<WGM20);
  TCCR2B=DetermineCorrectTimer2Divider(8); // Set to 4KHz
  BacklightDisable();  
}

uint8_t DetermineCorrectTimer2Divider(uint16_t myTargetDivision){
  switch (myTargetDivision) {
  case     1 : return 0x01;
  case     8 : return 0x02;
  case    32 : return 0x03;
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
  pinMode(BLPIN_BLON, OUTPUT); digitalWrite(BLPIN_BLON, BacklightInfo.BACKLIGHT_ENABLE_POLARITY);
  BacklightSetBrightness(UserConfiguration_LoadBrightness());
  BacklightIsOn = true;
}

void BacklightSetBrightness(uint8_t TargetBrightness){
  if(CONNECTED_BACKLIGHT != CONNECTED_BACKLIGHT_IS_GENERIC) {return;}
  uint8_t PinBrightness = TargetBrightness;
  if(BacklightInfo.BACKLIGHT_PWM_POLARITY == LOW) { PinBrightness = 255 - TargetBrightness; }
  pinMode(BLPIN_PWM, OUTPUT); analogWrite(BLPIN_PWM, PinBrightness);  
}

