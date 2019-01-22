#include "edid.h"
#include "supported_panels.h"

#define ModeLine_4K120_StripeMinimal { 520.32, 1920, 16, 32, 32, 2160, 1, 3, 4 }
#define ModeLine_4K120_StripeRegular { 522.72, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_4K60_StripeMinimal { 261.36, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_1440p180_StripeTesting { 558.000, 1920, 16, 32, 32, 1440, 3, 5, 102 }
#define ModeLine_1440p165_StripeSafe { 528.000, 1920, 16, 32, 32, 1440, 3, 5, 152 }
#define ModeLine_4K72_Extreme { 621.72, 3840, 16, 32, 37, 2160, 3, 5, 32 }
#define ModeLine_4K60_Safe { 528.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_4K30_Safe { 264.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_1080p240_Minimal { 523.200, 1920, 16, 32, 32, 1080, 1, 3, 6 }
#define ModeLine_1080p240_Regular { 529.490, 1920, 16, 32, 16, 1080, 3, 5, 24 }
#define ModeLine_1080p120_Safe { 270.000, 1920, 16, 32, 32, 1080, 3, 5, 37 }
#define ModeLine_720p360_Minimal  { 401.730, 1280, 48, 32, 140, 720, 3, 5, 16 }
#define ModeLine_720p240_Safe { 268.800, 1280, 48, 32, 40, 720, 3, 5, 72 }
#define ModeLine_540p480_Minimal { 333.600, 960, 48, 32, 210, 540, 3, 5, 8 }
#define ModeLine_540p240_Safe { 151.800, 960, 48, 32, 60, 540, 3, 5, 27 }


struct EDIDMetaConfig {
char NameSuffix[5];
uint8_t PixelScaling;
ModeLine PreferredMode;
ModeLine FallbackMode;
uint8_t IncludesDiDTileBlock; // If present, we assume that the two tiles left/right in arrangement.  If not present, the secondary DP interface will disconnect from host.
ModeLine TiledPreferredMode;
ModeLine TiledFallbackMode;
};

// Note: Allcaps modes are using ultra-low vertical blanking.  This helps the user identify which config is selected
#define EDIDMetaConfig_Profile0x { "4K120", 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, true, ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile0 { "4k120", 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, true, ModeLine_4K120_StripeRegular, ModeLine_4K60_StripeMinimal}
#define EDIDMetaConfig_Profile1Alternative { "180Hz", 1, ModeLine_1440p180_StripeTesting, ModeLine_1440p165_StripeSafe, true, ModeLine_1440p180_StripeTesting, ModeLine_1440p165_StripeSafe}
#define EDIDMetaConfig_Profile1 { "4k60 ", 1, ModeLine_4K60_Safe, ModeLine_4K30_Safe, false, ModeLine_4K60_Safe, ModeLine_4K30_Safe}
#define EDIDMetaConfig_Profile2x { "240HZ", 2, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe, false, ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe}
#define EDIDMetaConfig_Profile2 { "240Hz", 2, ModeLine_1080p240_Regular, ModeLine_1080p120_Safe, false, ModeLine_1080p240_Regular, ModeLine_1080p120_Safe}
#define EDIDMetaConfig_Profile3 { "360Hz", 3, ModeLine_720p360_Minimal, ModeLine_720p240_Safe, false, ModeLine_720p360_Minimal, ModeLine_720p240_Safe}
#define EDIDMetaConfig_Profile4 { "480Hz", 4, ModeLine_540p480_Minimal, ModeLine_540p240_Safe, false, ModeLine_540p480_Minimal, ModeLine_540p240_Safe}

const EDIDMetaConfig EDIDMetaConfig_minimum_panel_clock[4] = {EDIDMetaConfig_Profile0x, EDIDMetaConfig_Profile2x, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};
const EDIDMetaConfig EDIDMetaConfig_safe_configuration[4] = {EDIDMetaConfig_Profile0, EDIDMetaConfig_Profile2, EDIDMetaConfig_Profile3, EDIDMetaConfig_Profile4};

#define EDIDMetaConfigs EDIDMetaConfig_safe_configuration


void GenerateEDIDWithParameters(uint8_t AmPrimary, uint8_t PanelID, uint8_t EDIDMetaConfigID, uint32_t SerialNumber){
  myEDID.Reset();
  
  if(AmPrimary == false && (EDIDMetaConfigs[EDIDMetaConfigID].IncludesDiDTileBlock == false)) {return;}
  uint16_t BasePID = 20180;

  uint16_t TotalImageWidthInPixels = EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.HActive;

  if((EDIDMetaConfigs[EDIDMetaConfigID].IncludesDiDTileBlock == true) 
  && (EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.HActive < 2 * EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive)){
    TotalImageWidthInPixels = 2 * EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive;
  }

  uint16_t ImageWidthInMillimeters = (TotalImageWidthInPixels * EDIDMetaConfigs[EDIDMetaConfigID].PixelScaling *1.0*25.4) / PanelInfoArray[PanelID].DotsPerInch;
  uint16_t ImageHeightInMillimeters = (EDIDMetaConfigs[EDIDMetaConfigID].PreferredMode.VActive * EDIDMetaConfigs[EDIDMetaConfigID].PixelScaling *1.0*25.4) / PanelInfoArray[PanelID].DotsPerInch;
  
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
      myEDID.DiDCreateBlock(); 
      if(AmPrimary == true){
      myEDID.DiDAddTiledDescriptor("ZIS", BasePID + EDIDMetaConfigID, SerialNumber, 2, 1, 0, 0, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.VActive);
      } else {
      myEDID.DiDAddTiledDescriptor("ZIS", BasePID + EDIDMetaConfigID, SerialNumber, 2, 1, 1, 0, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.HActive, EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode.VActive);
      }
      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].TiledPreferredMode, ImageHeightInMillimeters, ImageWidthInMillimeters/2);
      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].TiledFallbackMode, ImageHeightInMillimeters, ImageWidthInMillimeters/2);
      myEDID.DiDSetChecksum(); 
      myEDID.FixChecksumDiDBlock();
  }
  myEDID.FixChecksumBaseBlock();
}


