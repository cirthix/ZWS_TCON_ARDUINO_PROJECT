#ifndef SUPPORTEDBLDRIVERS_H
#define SUPPORTEDBLDRIVERS_H

const uint8_t DEFAULT_BRIGHTNESS_LEVEL=150;

struct BacklightInfo_t {
  uint8_t BACKLIGHT_ENABLE_POLARITY;
  uint8_t BACKLIGHT_PWM_POLARITY;
  uint8_t PWM_MIN_DUTYCYCLE;
  uint8_t PWM_MAX_DUTYCYCLE;
  uint8_t PWM_DIVIDER;
};

const BacklightInfo_t BacklightInfoGeneric = { HIGH, HIGH, 5, 250, 2};
const BacklightInfo_t BacklightInfoBDM4065 = { HIGH, HIGH, 35, 255, 1};

// Note: This is intentially not in constants.h to avoid confusing users.
// There have been no shipped kits which were configured for external backlights, so the option is hidden here.
const BacklightInfo_t BacklightInfo = BacklightInfoBDM4065 ;

#endif
