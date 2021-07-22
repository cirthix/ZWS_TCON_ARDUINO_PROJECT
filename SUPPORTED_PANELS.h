#ifndef SUPPORTEDPANELS_H
#define SUPPORTEDPANELS_H

#include <Arduino.h>
#include "edid.h"


// Values from datasheets
#define Chromaticities_M280DGJ3 {0.630, 0.341, 0.310, 0.626, 0.156, 0.061, 0.313, 0.329}
#define Chromaticities_M280DGJ_QuantumDot {0.634, 0.341, 0.312, 0.636, 0.158, 0.062, 0.313, 0.329}
#define Chromaticities_MVA_with_innolux_leds {0.645, 0.335, 0.306, 0.617, 0.149, 0.058, 0.280, 0.290}
#define Chromaticities_MVA_with_bdm4065_leds {0.645, 0.335, 0.306, 0.617, 0.149, 0.058, 0.280, 0.290} // No real data
#define Chromaticities_Experimental {.6675, .3075, .1745, .7555, .1395, .0635, 0.308, 0.32}

struct PanelInfo {
char Name[9];
float DotsPerInch;
Chromaticities Chromaticity;
};

#define PanelInfo_0 { "M280DGJ1", 157.35, Chromaticities_M280DGJ3}
#define PanelInfo_1 { "M280DGJ3", 157.35, Chromaticities_M280DGJ3}
#define PanelInfo_2 { "M280DGJQ", 157.35, Chromaticities_M280DGJ_QuantumDot}
#define PanelInfo_3 { "  V390DK", 112.97, Chromaticities_MVA_with_innolux_leds}
#define PanelInfo_4 { "  V400DK", 110.15, Chromaticities_MVA_with_innolux_leds}
#define PanelInfo_5 { "  V420DK", 104.90, Chromaticities_MVA_with_innolux_leds}
#define PanelInfo_6 { "  V500DK",  88.12, Chromaticities_MVA_with_innolux_leds}
#define PanelInfo_7 { "  V580DK",  75.96, Chromaticities_MVA_with_innolux_leds}
#define PanelInfo_8 { "ZIS_TEST", 208.53, Chromaticities_Experimental}

const PanelInfo PanelInfoArray[9] = {PanelInfo_0, PanelInfo_1, PanelInfo_2, PanelInfo_3, PanelInfo_4, PanelInfo_5, PanelInfo_6, PanelInfo_7, PanelInfo_8};

inline void panel_print_name(){ SerialDebuglnD(PanelInfoArray[PANEL_VERSION].Name);}
const uint16_t VIDEO_SIGNAL_TO_BACKLIGHT_ON_DELAY = 600; // MILLISECONDS
const uint8_t ADDED_DELAY_AFTER_PANEL_POWERUP = 0;
const uint8_t DELAY_BETWEEN_BACKLIGHT_POWEROFF_AND_PANEL_POWEROFF = 100;

#define LVDS_MAPPING_VESA 0
#define LVDS_MAPPING_JEIDA 1

#define LVDS_SWING_LEVEL_LOW 0
#define LVDS_SWING_LEVEL_HIGH 1
 
#define PANEL_BIT_DEPTH     10
#define LVDS_CHANNEL_COUNT 4
#define LVDS_SWING_LEVEL LVDS_SWING_LEVEL_HIGH  
#define PIXEL_ORDERING PIXEL_ORDERING_SEQUENTIAL
#define LVDS_MAPPING LVDS_MAPPING_VESA

//Experimental panel
//#define PANEL_BIT_DEPTH     8
//#define LVDS_CHANNEL_COUNT 2
//#define LVDS_SWING_LEVEL LVDS_SWING_LEVEL_HIGH  
//#define PIXEL_ORDERING PIXEL_ORDERING_SEQUENTIAL
//#define LVDS_MAPPING LVDS_MAPPING_JEIDA

#ifndef PANEL_BIT_DEPTH
  #define PANEL_BIT_DEPTH 8 
  #warning "Bit depth not selected, defaulting to 8 bit mode"
#endif

#ifndef LVDS_CHANNEL_COUNT
  #define LVDS_CHANNEL_COUNT 4 
  #warning "LVDS channel count not selected, defaulting to quad channel mode"
#endif

#ifndef LVDS_SWING_LEVEL
  #define LVDS_SWING_LEVEL LVDS_SWING_LEVEL_LOW 
  #warning "LVDS swing level not selected, defaulting to low swing mode"
#endif

#ifndef PIXEL_ORDERING
  #define PIXEL_ORDERING PIXEL_ORDERING_SEQUENTIAL 
  #warning "Pixel ordering not selected, defaulting to sequential mode"
#endif

#ifndef LVDS_MAPPING
  #define LVDS_MAPPING LVDS_MAPPING_VESA
  #warning "LVDS mapping not selected, defaulting to VESA mode"
#endif


#endif
