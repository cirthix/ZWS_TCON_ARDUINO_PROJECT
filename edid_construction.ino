#include "edid.h"
#include "supported_panels.h"

// "Minimal" modelines use extremely small blankings to minimize panel clock, but can potentially have compatibilitiy issues or gpu power state implications (eg: GPU not dropping to idle clocks)
// "Safe" modelines use small but not minimal blankings in order to maximize compatibility.  Problems from these timings should be rare.
// "Extreme" modes are likely to cause problems with many setups, but may work for some.
#define ModeLine_Null { 0.0, 0, 0, 0, 0, 0, 0, 0, 0 }
#define ModeLine_4K120_StripeMinimal { 520.32, 1920, 16, 32, 32, 2160, 1, 3, 4 }
#define ModeLine_4K120_StripeSafe { 522.72, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_4K115_StripeMinimal { 498.64, 1920, 16, 32, 32, 2160, 1, 3, 6 }
#define ModeLine_1440p180_StripeExtreme { 558.000, 1920, 16, 32, 32, 1440, 3, 5, 102 }
#define ModeLine_1440p165_StripeSafe { 528.000, 1920, 16, 32, 32, 1440, 3, 5, 152 }
#define ModeLine_4K60_StripeMinimal { 261.36, 1920, 24, 16, 20, 2160, 3, 5, 32 }
#define ModeLine_4K72_Extreme { 621.72, 3840, 16, 32, 37, 2160, 3, 5, 32 }
#define ModeLine_4K60_Safe { 528.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_4K30_Safe { 264.00, 3840, 48, 32, 80, 2160, 3, 5, 32 }
#define ModeLine_1080p240_Minimal { 523.200, 1920, 16, 32, 32, 1080, 1, 3, 6 }
#define ModeLine_1080p240_Safe { 529.490, 1920, 16, 32, 16, 1080, 3, 5, 24 }
#define ModeLine_1080p230_Minimal { 501.40, 1920, 16, 32, 32, 1080, 1, 3, 6 }
#define ModeLine_1080p120_Safe { 270.000, 1920, 16, 32, 32, 1080, 3, 5, 37 }
#define ModeLine_1080p60_Safe { 140.400, 1920, 48, 32, 80, 1080, 3, 5, 37 }
#define ModeLine_720p240_Safe { 268.800, 1280, 48, 32, 40, 720, 3, 5, 72 }
#define ModeLine_720p360_Minimal  { 401.730, 1280, 48, 32, 140, 720, 3, 5, 16 }
#define ModeLine_720p360_RectMinimal  { 535.680, 1920, 16, 32, 32, 720, 3, 5, 16 }
#define ModeLine_540p240_Safe { 151.800, 960, 48, 32, 60, 540, 3, 5, 27 }
#define ModeLine_540p480_Minimal { 333.600, 960, 48, 32, 210, 540, 3, 5, 8 }
#define ModeLine_540p480_RectMinimal  { 533.760, 1920, 16, 32, 32, 540, 3, 5, 8 }

// The modes below this line are for zisworks internal testing only, don't use them on x28/x39
#define ModeLine_ROGPhoneII  { 342.72, 1080, 40, 40, 40, 2340, 2, 6, 32 }
#define ModeLine_TripleHeadArray  { 180.00, 3840, 16, 32, 32, 800, 3, 5, 27 }


struct TiledMetaConfig {
uint8_t PixelScalingH;
uint8_t PixelScalingV;
ModeLine TiledPreferredMode;
ModeLine TiledFallbackMode;
// Note: DiD tile defines a resolution, so all tiled modes must keep the same resolution.  Only use fallback for reduced refresh rate for legacy host devices
};

#define TiledMetaConfig_Invalid       { 1, 1, ModeLine_Null, ModeLine_Null}
#define TiledMetaConfig_Profile0      { 1, 1, ModeLine_4K120_StripeSafe, ModeLine_4K60_StripeMinimal }
#define TiledMetaConfig_Profile0x     { 1, 1, ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal }
#define TiledMetaConfig_Profile0z     { 1, 1, ModeLine_4K120_StripeMinimal, ModeLine_Null }
#define TiledMetaConfig_Profile1      { 1, 1, ModeLine_1440p165_StripeSafe, ModeLine_Null }
#define TiledMetaConfig_Profile1x     { 1, 1, ModeLine_1440p180_StripeExtreme, ModeLine_Null }
#define TiledMetaConfig_Profile2Rect  { 1, 2, ModeLine_1080p240_Safe, ModeLine_Null }
#define TiledMetaConfig_Profile2Rectx { 1, 2, ModeLine_1080p240_Minimal, ModeLine_Null}
#define TiledMetaConfig_Profile3Rectx { 1, 3, ModeLine_720p360_RectMinimal, ModeLine_Null}
#define TiledMetaConfig_Profile4Rectx { 1, 4, ModeLine_540p480_RectMinimal, ModeLine_Null}

#define EDID_BASE_SLOTS 4
struct BaseMetaConfig {
uint8_t PixelScalingH[EDID_BASE_SLOTS];
uint8_t PixelScalingV[EDID_BASE_SLOTS];
ModeLine myModeLine[EDID_BASE_SLOTS]; // Note: using the fourth slot will remove the monitor name string from the EDID
};

#define BaseMetaConfig_Invalid       { {1,1,1,1}, {1,1,1,1}, {ModeLine_Null, ModeLine_Null, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_FullThree     { {2,3,4,1}, {2,3,4,1}, {ModeLine_1080p240_Safe, ModeLine_720p360_Minimal, ModeLine_540p480_Minimal, ModeLine_Null}}
#define BaseMetaConfig_FullThreex    { {2,3,4,1}, {2,3,4,1}, {ModeLine_1080p240_Minimal, ModeLine_720p360_Minimal, ModeLine_540p480_Minimal, ModeLine_Null}}
#define BaseMetaConfig_FullCombo     { {1,2,3,4}, {1,2,3,4}, {ModeLine_4K60_Safe, ModeLine_1080p240_Safe, ModeLine_720p360_Minimal, ModeLine_540p480_Minimal}}
#define BaseMetaConfig_FullCombox    { {1,2,3,4}, {1,2,3,4}, {ModeLine_4K60_Safe, ModeLine_1080p240_Minimal, ModeLine_720p360_Minimal, ModeLine_540p480_Minimal}}
#define BaseMetaConfig_Stripe        { {1,1,1,1}, {1,1,1,1}, {ModeLine_4K120_StripeMinimal, ModeLine_4K60_StripeMinimal, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Stripex       { {1,1,1,1}, {1,1,1,1}, {ModeLine_4K120_StripeSafe, ModeLine_4K60_StripeMinimal, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile0      { {1,1,1,1}, {1,1,1,1}, {ModeLine_4K60_Safe, ModeLine_4K30_Safe, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile1      { {1,1,1,1}, {1,1,1,1}, {ModeLine_1440p165_StripeSafe, ModeLine_Null, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile1x     { {1,1,1,1}, {1,1,1,1}, {ModeLine_1440p180_StripeExtreme, ModeLine_Null, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile2      { {2,2,1,1}, {2,2,1,1}, {ModeLine_1080p240_Safe, ModeLine_1080p120_Safe, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile2x     { {2,2,1,1}, {2,2,1,1}, {ModeLine_1080p240_Minimal, ModeLine_1080p120_Safe, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile3x     { {3,3,1,1}, {3,3,1,1}, {ModeLine_720p360_Minimal, ModeLine_720p240_Safe, ModeLine_Null, ModeLine_Null}}
#define BaseMetaConfig_Profile4x     { {4,4,1,1}, {4,4,1,1}, {ModeLine_540p480_Minimal, ModeLine_540p240_Safe, ModeLine_Null, ModeLine_Null}}

struct EDIDMetaConfig {
char NameSuffix[5];
BaseMetaConfig myBaseMetaConfig;
TiledMetaConfig myTiledMetaConfig; // If preferred mode is present, we assume that the two tiles left/right in arrangement.  If not present, the secondary DP interface will disconnect from host.
};
#define EDIDMetaConfig_ComboFull { "1MODE", BaseMetaConfig_FullThreex, TiledMetaConfig_Profile0z}
#define EDIDMetaConfig_ComboLegacy { "COMBO", BaseMetaConfig_FullCombo, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile0 { "4k120", BaseMetaConfig_Profile0, TiledMetaConfig_Profile0}
#define EDIDMetaConfig_Profile0x { "4K120", BaseMetaConfig_Profile0, TiledMetaConfig_Profile0x}
#define EDIDMetaConfig_Profile0IntelFix { "4k120", BaseMetaConfig_Stripe, TiledMetaConfig_Profile0}
#define EDIDMetaConfig_Profile0IntelFixx { "4K120", BaseMetaConfig_Stripex, TiledMetaConfig_Profile0x}
#define EDIDMetaConfig_Profile1 { "4k60 ", BaseMetaConfig_Profile0, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile1Alternative { "165Hz", BaseMetaConfig_Profile1, TiledMetaConfig_Profile1}
#define EDIDMetaConfig_Profile1Alternativex { "180Hz", BaseMetaConfig_Profile1x, TiledMetaConfig_Profile1x}
#define EDIDMetaConfig_Profile2 { "240Hz", BaseMetaConfig_Profile2, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile2x { "240HZ", BaseMetaConfig_Profile2x, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile3x { "360HZ", BaseMetaConfig_Profile3x, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile4x { "480HZ", BaseMetaConfig_Profile4x, TiledMetaConfig_Invalid}
#define EDIDMetaConfig_Profile2Rect { "Rp240", BaseMetaConfig_Profile2, TiledMetaConfig_Profile2Rect}
#define EDIDMetaConfig_Profile2Rectx { "RP240", BaseMetaConfig_Profile2x, TiledMetaConfig_Profile2Rectx}
#define EDIDMetaConfig_Profile3Rectx { "RP360", BaseMetaConfig_Profile3x, TiledMetaConfig_Profile3Rectx}
#define EDIDMetaConfig_Profile4Rectx { "RP480", BaseMetaConfig_Profile4x, TiledMetaConfig_Profile4Rectx}


const EDIDMetaConfig EDIDMetaConfig_SHIPPING[4] = {EDIDMetaConfig_Profile0IntelFixx, EDIDMetaConfig_Profile2x, EDIDMetaConfig_Profile3x, EDIDMetaConfig_Profile4x};
const EDIDMetaConfig EDIDMetaConfig_SIMPLE[4] = {EDIDMetaConfig_ComboFull, EDIDMetaConfig_ComboFull, EDIDMetaConfig_ComboFull, EDIDMetaConfig_ComboFull};
const EDIDMetaConfig EDIDMetaConfig_MOREBLANKING[4] = {EDIDMetaConfig_Profile0IntelFix, EDIDMetaConfig_Profile2, EDIDMetaConfig_Profile3x, EDIDMetaConfig_Profile4x};
const EDIDMetaConfig EDIDMetaConfig_RECTANGULAR[4] = {EDIDMetaConfig_Profile0IntelFixx, EDIDMetaConfig_Profile2Rectx, EDIDMetaConfig_Profile3Rectx, EDIDMetaConfig_Profile4Rectx};
const EDIDMetaConfig EDIDMetaConfig_GOODOPTIONS[4] = {EDIDMetaConfig_ComboFull, EDIDMetaConfig_ComboLegacy, EDIDMetaConfig_Profile1Alternativex, EDIDMetaConfig_Profile2Rectx};
const EDIDMetaConfig EDIDMetaConfig_TWEAKER[4] = {EDIDMetaConfig_ComboFull, EDIDMetaConfig_Profile1Alternativex, EDIDMetaConfig_Profile2Rectx, EDIDMetaConfig_Profile3Rectx};

struct VideoWallConfig_t {
uint8_t NumTilesPerDisplay; // Note: assumed horizontal left/right split within each display
uint8_t NumDisplaysH; // Note: counting starts from one
uint8_t NumDisplaysV; // Note: counting starts from one
uint8_t MyPositionH; // Note: counting starts from zero
uint8_t MyPositionV; // Note: counting starts from zero
};

const VideoWallConfig_t VideoWallConfigSingle = {2, 1, 1, 0, 0};
const VideoWallConfig_t VideoWallConfigDual   = {2, 2, 1, 0, 0};
const VideoWallConfig_t VideoWallConfigTriple = {2, 3, 1, 0, 0};
const VideoWallConfig_t VideoWallConfigQuad   = {2, 2, 2, 0, 0};

//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////
#define EDIDMetaConfigs EDIDMetaConfig_TWEAKER
// Note that if you are using video wall functionality, FIRMWARE_UNIQUE_ID_OVERRIDE must be forced to be the same for all tiles in constants file
#define VideoWallConfig VideoWallConfigSingle
#define TRY_TO_ADD_CEA_EXTENSION_BLOCK false
//////////////////////////////////////////////////////////////////////// CHANGE SYSTEM CONFIGURATION PARAMETERS HERE ////////////////////////////////////////////////////////////////////////

void SetU28H750EDID(){
  const uint8_t StockSamsungEDID[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4C, 0x2D, 0x4D, 0x0C, 0x4A, 0x53, 0x4D, 0x30, 0x10, 0x1C, 0x01, 0x04, 0xB5, 0x3D, 0x23, 0x78, 0x3B, 0x5F, 0xB1, 0xA2, 0x57, 0x4F, 0xA2, 0x28, 0x0F, 0x50, 0x54, 0x23, 0x08, 0x00, 0x81, 0x00, 0x81, 0xC0, 0x81, 0x80, 0xA9, 0xC0, 0xB3, 0x00, 0x95, 0x00, 0x01, 0x01, 0x01, 0x01, 0x4D, 0xD0, 0x00, 0xA0, 0xF0, 0x70, 0x3E, 0x80, 0x30, 0x20, 0x35, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x28, 0x3C, 0x87, 0x87, 0x3C, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x55, 0x32, 0x38, 0x45, 0x35, 0x39, 0x30, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x48, 0x54, 0x50, 0x4B, 0x34, 0x30, 0x39, 0x36, 0x38, 0x34, 0x0A, 0x20, 0x20, 0x01, 0xD8, 0x02, 0x03, 0x0E, 0xF0, 0x41, 0x10, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1E, 0x56, 0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29, 0x50, 0x30, 0x20, 0x35, 0x00, 0x5F, 0x59, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF};
  myEDID.Reset();
  for(uint16_t index=0; index<=0xFF; index++){
    myEDID.SetByte(index, StockSamsungEDID[index]);     
  } 
}


struct ImageDimensions_t {
uint16_t ImageWidthInMillimeters; 
uint16_t ImageHeightInMillimeters; 
};

struct ImageDimensions_t ImageSizeCalculator(float PanelDPI, uint8_t PixelScalingH, uint8_t PixelScalingV, uint16_t ResolutionH, uint16_t ResolutionV){
  ImageDimensions_t myImageDimensions;  
  myImageDimensions.ImageWidthInMillimeters = (ResolutionH * PixelScalingH *1.0*25.4) / PanelDPI;
  myImageDimensions.ImageHeightInMillimeters = (ResolutionV* PixelScalingV *1.0*25.4) / PanelDPI;
  return myImageDimensions;  
}


void GenerateEDIDWithParameters(uint8_t AmPrimary, uint8_t AmCloned, uint8_t PanelID, uint8_t EDIDMetaConfigID, uint32_t SerialNumber){
//  SetU28H750EDID(); return;  // Sometimes, it is useful to try with a known-good EDID
  ImageDimensions_t myImageDimensions;
  myEDID.Reset();
  if(AmPrimary == false && (EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive == 0)) {
    if(AmCloned == true) {
      return GenerateEDIDWithParameters(true, AmCloned, PanelID, EDIDMetaConfigID, SerialNumber);
    } else {
      return;
    }
  }
  uint16_t BasePID = 20180;

  if(EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive == 0) {
  myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingH[0],    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingV[0],        EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[0].HActive,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[0].VActive );
  } else {
  myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingH,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingV,        EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive*2,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.VActive );    
  }

  
  uint16_t DiagonalInMillimeters = pow(pow(myImageDimensions.ImageHeightInMillimeters, 2) + pow(myImageDimensions.ImageWidthInMillimeters, 2), 0.5);
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
  myEDID.SetPhysicalWidthInCentimeters(round(myImageDimensions.ImageWidthInMillimeters/10));
  myEDID.SetPhysicalHeightInCentimeters(round(myImageDimensions.ImageHeightInMillimeters/10));
  myEDID.SetReportedGammaValueTimes100(2.2 * 100);
  myEDID.SetRGB444WithNoDPMSWithNativeTimingsAndNoContinuiousFrequency();
  myEDID.SetReportedChromaticites(PanelInfoArray[PanelID].Chromaticity);
  myEDID.SetNoLegacyStandardVideoModes();
  myEDID.SetNoGenericVideoModes();

  boolean SkipDTDs = false;
  //if( ((EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive != 0) && (AmPrimary == false))) { SkipDTDs = true; } // Experimental fix for intel graphics : remove preferred timing mode on secondary output base block

  if(SkipDTDs == false) {
    for(uint8_t j=0; j<EDID_BASE_SLOTS; j++){
      myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingH[j],    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingV[j],        EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j].HActive,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j].VActive );
      myEDID.AddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j], myImageDimensions.ImageWidthInMillimeters, myImageDimensions.ImageHeightInMillimeters);
    }
  } else {
    myEDID.SetRGB444WithNoDPMSWithNoNativeTimingsAndNoContinuiousFrequency();
  }
  
  // myEDID.AddDetailedDescriptorRangeLimitsOnly(30, 120, 132, 264, 540);
  if(EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[EDID_BASE_SLOTS-1].HActive == 0) {
   myEDID.AddDetailedDescriptorName(DetailedDescriptorNameLength, DetailedDescriptorName);
  }
  // Note: HDMI block is not needed, no audio and no freesync 
  if(EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive != 0) {
  // Add a DiD block
      myEDID.DiDCreateBlock(); 
      uint8_t myPositionH=VideoWallConfig.NumTilesPerDisplay*VideoWallConfig.MyPositionH;
      if(AmPrimary == false){myPositionH = myPositionH+1;}
      myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingH,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingV,        EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.VActive );
      myEDID.DiDAddTiledDescriptor("ZIS", BasePID + EDIDMetaConfigID, SerialNumber, VideoWallConfig.NumTilesPerDisplay*VideoWallConfig.NumDisplaysH, VideoWallConfig.NumDisplaysV, myPositionH, VideoWallConfig.MyPositionV, EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.HActive, EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode.VActive);
      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledPreferredMode, myImageDimensions.ImageWidthInMillimeters, myImageDimensions.ImageHeightInMillimeters);
      myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingH,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.PixelScalingV,        EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledFallbackMode.HActive,    EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledFallbackMode.VActive );

      myEDID.DiDAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].myTiledMetaConfig.TiledFallbackMode, myImageDimensions.ImageWidthInMillimeters, myImageDimensions.ImageHeightInMillimeters);
      myEDID.DiDSetChecksum(); 
      myEDID.FixChecksumExtensionBlock();
  } else {
    #if TRY_TO_ADD_CEA_EXTENSION_BLOCK == true
    // Add a CEA block
      myEDID.CEACreateBlock(); 
      myEDID.CEAAddHDMI();
      for(uint8_t j=0; j<EDID_BASE_SLOTS; j++){
        myImageDimensions = ImageSizeCalculator(    PanelInfoArray[PanelID].DotsPerInch,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingH[j],    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.PixelScalingV[j],        EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j].HActive,    EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j].VActive );
        myEDID.CEAAddDetailedDescriptorTiming(EDIDMetaConfigs[EDIDMetaConfigID].myBaseMetaConfig.myModeLine[j], myImageDimensions.ImageWidthInMillimeters, myImageDimensions.ImageHeightInMillimeters);
      }
      myEDID.FixChecksumExtensionBlock();
    #endif
  }
  myEDID.FixChecksumBaseBlock();
}


