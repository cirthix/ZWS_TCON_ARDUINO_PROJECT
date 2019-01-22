#ifndef SUPPORTEDBLDRIVERS_H
#define SUPPORTEDBLDRIVERS_H

const uint8_t DEFAULT_BRIGHTNESS_LEVEL=150;

struct BacklightInfo_t {
  uint8_t BACKLIGHT_ENABLE_POLARITY;
  uint8_t BACKLIGHT_PWM_POLARITY;
  uint8_t PWM_MIN_DUTYCYCLE;
  uint8_t PWM_MAX_DUTYCYCLE;
  uint16_t PWM_FREQUENCY; // Note: ignored and set to 4KHz 
};

const BacklightInfo_t BacklightInfo = { HIGH, HIGH, 5, 250, 4000};

#endif
