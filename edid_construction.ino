#include "edid.h"
#include "supported_panels.h"

// "Minimal" modelines use extremely small blankings to minimize panel clock, but can potentially have compatibilitiy issues or gpu power state implications (eg: GPU not dropping to idle clocks)
// "Safe" modelines use small but not minimal blankings in order to maximize compatibility.  Problems from these timings should be rare.
// "Extreme" modes are likely to cause problems with many setups, but may work for some.
#define ModeLine_4K120_StripeMinimal { 520.32, 1920, 16, 32, 32, 2160, 1, 3, 4 }
#define ModeLine_4K120_StripeSafe { 522.72, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_1440p180_StripeExtreme { 558.000, 1920, 16, 32, 32, 1440, 3, 5, 102 }
#define ModeLine_1440p165_StripeSafe { 528.000, 1920, 16, 32, 32, 1440, 3, 5, 152 }
#define ModeLine_4K60_StripeMinimal { 261.36, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_4K72_Extreme { 621.72, 3840, 16, 32, 37, 2160, 3, 5, 32 }
#define ModeLine_4K60_Safe { 528.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_4K30_Safe { 264.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_1080p240_Minimal { 523.200, 1920, 16, 32, 32, 1080, 1, 3, 6 }
#define ModeLine_1080p240_Safe { 529.490, 1920, 16, 32, 16, 1080, 3, 5, 24 }
#define ModeLine_1080p120_Safe { 270.000, 1920, 16, 32, 32, 1080, 3, 5, 37 }
#define ModeLine_1080p60_Safe { 140.400, 1920, 48, 32, 80, 1080, 3, 5, 37 }
#define ModeLine_720p360_Minimal  { 401.730, 1280, 48, 32, 140, 720, 3, 5, 16 }
#define ModeLine_720p240_Safe { 268.800, 1280, 48, 32, 40, 720, 3, 5, 72 }
#define ModeLine_540p480_Minimal { 333.600, 960, 48, 32, 210, 540, 3, 5, 8 }
#define ModeLine_540p240_Safe { 151.800, 960, 48, 32, 60, 540, 3, 5, 27 }
#define ModeLine_720p360_RectMinimal  { 535.680, 1920, 16, 32, 32, 720, 3, 5, 16 }
#define ModeLine_720p360_RectSafe  { 576.00, 1920, 16, 32, 32, 720, 3, 5, 72 }
#define ModeLine_540p480_RectMinimal  { 533.760, 1920, 16, 32, 32, 540, 3, 5, 8 }
#define ModeLine_540p480_RectSafe  { 549.12, 1920, 16, 32, 32, 540, 3, 5, 27 }

struct EDIDMetaConfig {
char NameSuffix[5];
uint8_t PixelScalingH;
uint8_t PixelScalingV;
ModeLine PreferredMode;
ModeLine FallbackMode;
uint8_t IncludesDiDTileBlock; // If present, we assume that the two tiles left/right in arrangement.  If not present, the secondary DP interface will disconnect from host.
ModeLine TiledPreferredMode;
ModeLine TiledFallbackMode;
};

// Note: Allcaps modes are using ultra-low vertical blanking.  This helps the user identify which config is selected
#define EDIDMetaConfig_Profile0xTileFix { "4K120", 1, 1, ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal, true, ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile0TileFix { "4k120", 1, 1, ModeLine_4K120_StripeSafe, ModeLine_4K60_StripeMinimal, true, ModeLine_4K120_StripeSafe, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile0x { "4K120", 1, 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, true, ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile0 { "4k120", 1, 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, true, ModeLine_4K120_StripeSafe, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile1Alternative { "180Hz", 1, 1, ModeLine_1440p180_StripeTesting, ModeLine_1440p165_StripeSafe, true, ModeLine_1440p180_StripeTesting, ModeLine_1440p165_StripeSafe}
#define EDIDMetaConfig_Profile1 { "4k60 ", 1, 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, false, ModeLine_4K60_Safe, ModeLine_4K30_Safe}
#define EDIDMetaConfig_Profile2x { "240HZ", 2, 2, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe, false, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe}
#define EDIDMetaConfig_Profile2 { "240Hz", 2, 2, ModeLine_1080p240_Safe, ModeLine_1080p120_Safe, false, ModeLine_1080p240_Safe, ModeLine_1080p120_Safe}
#define EDIDMetaConfig_Profile3 { "360Hz", 3, 3, ModeLine_720p360_Minimal, ModeLine_720p240_Safe, false, ModeLine_720p360_Minimal, ModeLine_720p240_Safe}
#define EDIDMetaConfig_Profile4 { "480Hz", 4, 4, ModeLine_540p480_Minimal, ModeLine_540p240_Safe, false, ModeLine_540p480_Minimal, ModeLine_540p240_Safe}
#define EDIDMetaConfig_Profile2Rect { "Rp240", 1, 2, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe, true, ModeLine_1080p240_Safe, ModeLine_1080p240_Safe}
#define EDIDMetaConfig_Profile3Rect { "Rp360", 1, 3, ModeLine_720p360_Minimal, ModeLine_720p240_Safe, true, ModeLine_720p360_RectSafe, ModeLine_720p360_RectSafe}
#define EDIDMetaConfig_Profile4Rect { "Rp480", 1, 4, ModeLine_540p480_Minimal, ModeLine_540p240_Safe, true, ModeLine_540p480_RectSafe, ModeLine_540p480_RectSafe}
#define EDIDMetaConfig_Profile2xRect { "RP240", 1, 2, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe, true, ModeLine_1080p240_Minimal, ModeLine_1080p240_Minimal}
#define EDIDMetaConfig_Profile3xRect { "RP360", 1, 3, ModeLine_720p360_Minimal, ModeLine_720p240_Safe, true, ModeLine_720p360_RectMinimal, ModeLine_720p360_RectMinimal}
#define EDIDMetaConfig_Profile4xRect { "RP480", 1, 4, ModeLine_540p480_Minimal, ModeLine_540p240_Safe, true, ModeLine_540p480_RectMinimal, ModeLine_540p480_RectMinimal}

const EDIDMetaConfig EDIDMetaConfig_safe_configurationTileFix[4] = {EDIDMetaConfig_Profile0TileFix, EDIDMetaConfig_Profile2, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_minimum_panel_clockTileFix[4] = {EDIDMetaConfig_Profile0xTileFix, EDIDMetaConfig_Profile2x, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_safe_configuration[4] = {EDIDMetaConfig_Profile0, EDIDMetaConfig_Profile2, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_minimum_panel_clock[4] = {EDIDMetaConfig_Profile0x, EDIDMetaConfig_Profile2x, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_safe_rectangular_pixels[4] = {EDIDMetaConfig_Profile0, EDIDMetaConfig_Profile2Rect, EDIDMetaConfig_Profile3Rect, EDIDMetaConfig_Profile4Rect};
const EDIDMetaConfig EDIDMetaConfig_minimum_rectangular_pixels[4] = {EDIDMetaConfig_Profile0x, EDIDMetaConfig_Profile2xRect, EDIDMetaConfig_Profile3xRect, EDIDMetaConfig_Profile4xRect};
const EDIDMetaConfig EDIDMetaConfig_single_inputs_only[4] = {EDIDMetaConfig_Profile1, EDIDMetaConfig_Profile2, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_all_rectangular_pixels[4] = {EDIDMetaConfig_Profile0x, EDIDMetaConfig_Profile2x, EDIDMetaConfig_Profile2xRect, EDIDMetaConfig_Profile4xRect};


//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////
#define EDIDMetaConfigs EDIDMetaConfig_minimum_panel_clockTileFix
#define TRY_TO_ADD_CEA_EXTENSION_BLOCK false
//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////

void SetU28H750EDID(){
  const uint8_t StockSamsungEDID[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4C, 0x2D, 0x4D, 0x0C, 0x4A, 0x53, 0x4D, 0x30, 0x10, 0x1C, 0x01, 0x04, 0xB5, 0x3D, 0x23, 0x78, 0x3B, 0x5F, 0xB1, 0xA2, 0x57, 0x4F, 0xA2, 0x28, 0x0F, 0x50, 0x54, 0x23, 0x08, 0x00, 0x81, 0x00, 0x81, 0xC0, 0x81, 0x80, 0xA9, 0xC0, 0xB3, 0x00, 0x95, 0x00, 0x01, 0x01, 0x01, 0x01, 0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20, 0x35, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x28, 0x3C, 0x87, 0x87, 0x3C, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x55, 0x32, 0x38, 0x45, 0x35, 0x39, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x48, 0x54, 0x50, 0x4B, 0x34, 0x30, 0x39, 0x36, 0x38, 0x34, 0x0A, 0x20, 0x20, 0x01, 0xD8, 0x02, 0x03, 0x0E, 0xF0, 0x41, 0x10, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1E, 0x56, 0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30, 0x20, 0x35, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF};
  myEDID.Reset();
  for(uint16_t index=0; index<=0xFF; index++){
    myEDID.SetByte(index, StockSamsungEDID[index]);     
  } 
}

void GenerateEDIDWithParameters(uint8_t AmPrimary, uint8_t PanelID, uint8_t EDIDMetaConfigID, uint32_t SerialNumber){
//  SetU28H750EDID(); return;  // Sometimes, it is useful to try with a known-good EDID
  myEDID.Reset();
  
  if(AmPrimary == false && (EDIDMetaConfigs[EDIDMetaConfigID].IncludesDiDTileBlock == false)) {return;}
  uint16_t BasePID = 20180;

  uint16_t TotalImageWidthInPixels = EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.HActive;

  if((EDIDMetaConfigs[EDIDMetaConfigID].IncludesDiDTileBlock == true) 
  && (EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.HActive < 2 * EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive)){
    TotalImageWidthInPixels = 2 * EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive;
  }

  uint16_t ImageWidthInMillimeters = (TotalImageWidthInPixels * EDIDMetaConfigs[EDIDMetaConfigID].PixelScalingH *1.0*25.4) / PanelInfoArray[PanelID].DotsPerInch;
  uint16_t ImageHeightInMillimeters = (EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.VActive * EDIDMetaConfigs[EDIDMetaConfigID].PixelScalingV *1.0*25.4) / PanelInfoArray[PanelID].DotsPerInch;
  
  uint16_t DiagonalInMillimeters = pow(pow(ImageHeightInMillimeters, 2) + pow(ImageWidthInMillimeters, 2), 0.5);
  uint8_t DiagonalInInches = round(DiagonalInMillimeters *1.0 / 25.4);
  uint8_t DiagonalInInchesTens = floor(DiagonalInInches/10);
  uint8_t DiagonalInInchesOnes = DiagonalInInches - 10*DiagonalInInchesTens;
  const uint8_t DetailedDescriptorNameLength = 13;
  char DetailedDescriptorName[DetailedDescriptorNameLength];
 
  DetailedDescriptorName[0] = 'Z';
  DetailedDescriptorName[1] = 'W';
  DetailedDescriptorName[2] = 'S';
  DetailedDescriptorName[3] = ' ';
  DetailedDescriptorName[4] = '0'+DiagonalInInchesTens;
  DetailedDescriptorName[5] = '0'+DiagonalInInchesOnes;
  DetailedDescriptorName[6] = '\"';
  DetailedDescriptorName[7] = ' ';
  DetailedDescriptorName[8] = EDIDMetaConfigs[EDIDMetaConfigID].NameSuffix[0];
  DetailedDescriptorName[9] = EDIDMetaConfigs[EDIDMetaConfigID].NameSuffix[1];
  DetailedDescriptorName[10] = EDIDMetaConfigs[EDIDMetaConfigID].NameSuffix[2];
  DetailedDescriptorName[11] = EDIDMetaConfigs[EDIDMetaConfigID].NameSuffix[3];
  DetailedDescriptorName[12] = EDIDMetaConfigs[EDIDMetaConfigID].NameSuffix[4];  
  
  myEDID.SetHeader();
  myEDID.SetManufacturerID("ZIS");
  myEDID.SetManufacturerProductCode(BasePID + EDIDMetaConfigID);
  myEDID.SetManufacturerSerialNumber(SerialNumber);
  myEDID.SetManufactureWeek(1);
  myEDID.SetManufactureYear(2018);
  myEDID.SetDisplayPort10Bit();
  myEDID.SetPhysicalWidthInCentimeters(round(ImageWidthInMillimeters/10));
  myEDID.SetPhysicalHeightInCentimeters(round(ImageHeightInMillimeters/10));
  myEDID.SetReportedGammaValueTimes100(2.2 * 100);
  myEDID.SetRGB444WithNoDPMSWithNativeTimingsAndNoContinuiousFrequency();
  myEDID.SetReportedChromaticites(PanelInfoArray[PanelID].Chromaticity);
  myEDID.SetNoLegacyStandardVideoModes();
  myEDID.SetNoGenericVideoModes();
  myEDID.AddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode, ImageHeightInMillimeters, ImageWidthInMillimeters);
  myEDID.AddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].FallbackMode, ImageHeightInMillimeters, ImageWidthInMillimeters);
 // myEDID.AddDetailedDescriptorRangeLimitsOnly(30, 120, 132, 264, 540);
  myEDID.AddDetailedDescriptorName(DetailedDescriptorNameLength, DetailedDescriptorName);
  // Note: HDMI block is not needed, no audio and no freesync 
  if(EDIDMetaConfigs[EDIDMetaConfigID].IncludesDiDTileBlock == true) {
    // Add a DiD block
      myEDID.DiDCreateBlock(); 
      if(AmPrimary == true){
      myEDID.DiDAddTiledDescriptor("ZIS", BasePID + EDIDMetaConfigID, SerialNumber, 2, 1, 0, 0, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.VActive);
      } else {
      myEDID.DiDAddTiledDescriptor("ZIS", BasePID + EDIDMetaConfigID, SerialNumber, 2, 1, 1, 0, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.VActive);
      }
      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode, ImageHeightInMillimeters, ImageWidthInMillimeters/2);
      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].TiledFallbackMode, ImageHeightInMillimeters, ImageWidthInMillimeters/2);
      myEDID.DiDSetChecksum(); 
      myEDID.FixChecksumExtensionBlock();
  } else {
    #if TRY_TO_ADD_CEA_EXTENSION_BLOCK == true
    // Add a CEA block
      myEDID.CEACreateBlock(); 
      myEDID.CEAAddHDMI();
      myEDID.CEAAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode, ImageHeightInMillimeters, ImageWidthInMillimeters);
      myEDID.CEAAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].FallbackMode, ImageHeightInMillimeters, ImageWidthInMillimeters);
      myEDID.FixChecksumExtensionBlock();
    #endif
  }
  myEDID.FixChecksumBaseBlock();
}


