#ifndef SUPPORTEDBOARDS_H
#define SUPPORTEDBOARDS_H

#if BOARD_VERSION==BOARD_IS_DUAL_EP369_TCON
inline void board_print_name(){ Serial.println(F("ZisWorks DualDP TCON v1.3"));}
#define BUTTON_POWER  A0
#define BUTTON_A_ANALOG  A1
#define BUTTON_B_ANALOG  A2
#define BLPIN_BLON  A3
#define THIS_PIN_IS_NOT_USED  A4
#define THIS_PIN_IS_NOT_USED  A5 
#define LED_RGB  1 // Shared with the serial output of UART
#define THIS_PIN_IS_NOT_USED  2
#define PANEL_GPIO0  3
#define SDA_PIN_SEC 4
#define SCL_PIN_SEC 5
#define SDA_PIN_PRI  6
#define SCL_PIN_PRI  7
#define CONTROL_VREG_VPANEL  8
  #define CONTROL_VREG_VPANEL_POLARITY ACTIVE_HIGH
#define LOW_POWER_MODE_PRI 9
#define RESET_OTHER_CHIPS_PRI 10
#define BLPIN_PWM  11
#define ACTIVE_VIDEO_PRI  12
#define ACTIVE_VIDEO_SEC  13
#define LOW_POWER_MODE_SEC 14
#define RESET_OTHER_CHIPS_SEC  15

#elif BOARD_VERSION==BOARD_IS_EP369_REV2017
inline void board_print_name(){ Serial.println(F("ZisWorks dp2lvds v2017"));}
//Note: unused pins may be connected to active circuits on the board.  Do not drive them!
#define BUTTON_POWER  A0
#define BUTTON_A_ANALOG  A1
#define BUTTON_B_ANALOG  A2
// Note: no boards were ever manufactured with dcdc populated
#define INPUT_VOLTAGE_MONITORING_PIN  A3
const float MICROCONTROLLER_VOLTAGE=3.3;
const uint16_t ADC_RESOLUTION = 1024;
const uint16_t PULLUP_RESISTOR = 4700;
const uint16_t PULLDOWN_RESISTOR = 1000;
#define SDA_PIN_PRI A4
#define SCL_PIN_PRI A5
#define SDA_PIN_SEC A4
#define SCL_PIN_SEC A5
#define DATA_ENABLE  2
#define THIS_PIN_IS_NOT_USED  3
#define CONTROL_VREG_VPANEL  4
  #define CONTROL_DCDC_VPANEL_POLARITY ACTIVE_HIGH
#define THIS_PIN_IS_NOT_USED 5
#define BLPIN_BLON  6
#define THIS_PIN_IS_NOT_USED  7
#define THIS_PIN_IS_NOT_USED  8
#define RESET_OTHER_CHIPS_PRI  9
#define ACTIVE_VIDEO_PRI 10
#define BLPIN_PWM  11
#define PANEL_GPIO0  12
#define PANEL_GPIO1  13
#define LOW_POWER_MODE_PRI 14
#define THIS_PIN_IS_NOT_USED  15

#else
  #error "Unsupported BOARD_VERSION"
#endif



#endif


