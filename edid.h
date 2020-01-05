#ifndef edid_h
#define edid_h
#include <Arduino.h>

const uint16_t EDID_BLOCK_SIZE = 128;
const uint16_t EDID_SIZE = 2*EDID_BLOCK_SIZE;
 // Can have either DiD or CEA
const uint8_t DiDBlockOffset = EDID_BLOCK_SIZE;
const uint8_t CEABlockOffset = EDID_BLOCK_SIZE;

struct ModeLine{
  float PixelClock;
  uint16_t HActive;  
  uint16_t HFP;
  uint16_t HSW;
  uint16_t HBP;
  uint16_t VActive;
  uint16_t VFP;
  uint16_t VSW;
  uint16_t VBP;  
};

struct Chromaticities{float rx; float ry; float gx; float gy; float bx; float by; float wx; float wy;};

class EDID
{
public:
  EDID() ;
  void Reset();
  uint8_t GetByte(uint8_t address);
  void SetByte(uint8_t address, uint8_t value);
  void SetHeader();
  void SetManufacturerID(uint8_t ManufacturerID[3]);
 // void SetManufacturerID(uint8_t a, uint8_t b, uint8_t c);
  void SetManufacturerProductCode(uint16_t value);
  void SetManufacturerSerialNumber(uint32_t value);
  void SetManufactureWeek(uint8_t value);
  void SetManufactureYear(uint16_t value);
  void SetDisplayPort10Bit();
  void SetDisplayPort8Bit();
  void SetPhysicalWidthInCentimeters(uint8_t value);
  void SetPhysicalHeightInCentimeters(uint8_t value);
  void SetReportedGammaValueTimes100(uint16_t value);
  void SetRGB444WithNoDPMSWithNoNativeTimingsAndNoContinuiousFrequency();
  void SetRGB444WithNoDPMSWithNativeTimingsAndNoContinuiousFrequency();
  void SetReportedChromaticites(Chromaticities myChromaticities);
  void SetNoLegacyStandardVideoModes();
  void SetNoGenericVideoModes();
  void AddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters);
  void AddDetailedDescriptorRangeLimitsOnly(uint16_t MinVHz, uint16_t MaxVHz, uint16_t MinHKhz, uint16_t MaxHKhz, uint16_t MaxPixelClock);
  void AddDetailedDescriptorName(uint8_t NameLength, uint8_t* NamePointer);
  void FixChecksumBaseBlock();
  void FixChecksumExtensionBlock();
  void CEACreateBlock();
  void CEAAddHDMI();
  void CEAAddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters);
  void DiDCreateBlock();
  void DiDAddTiledDescriptor(uint8_t ManufacturerID[3], uint16_t ProductID, uint32_t SerialNumber, uint8_t HTiles, uint8_t VTiles, uint8_t HTileLocation, uint8_t VTileLocation, uint16_t HTileSize, uint16_t VTileSize);
  void DiDAddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters);
  void DiDSetChecksum();
  void PrintEDID();
private:
  const uint8_t MaxNumberOfFilledDescriptorBlocks = 4;
  uint8_t DiDPreferred = 0x80;
  uint8_t DiDByteCount;
  uint8_t DiDDetailedDescriptorByteCountAddress;
  uint8_t NumberOfFilledDescriptorBlocks = 0;
  uint8_t NumberOfFilledCEADescriptorBlocks = 0;
  uint8_t RawBytes[EDID_SIZE];
  void SetExtensionBlockCount(uint8_t value);
  uint8_t DiDGetDescriptorOffset();
  void DiDAddToByteCount(uint8_t value);
  uint8_t CalculateSumBlock(uint8_t block);
  void AddDetailedDescriptorTiming18BytesToOffset(uint8_t myOffset, ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters);
  void IncrementNumberOfFilledDescriptorBlocks();
  uint8_t GetDetailedDescriptorBlockOffset();
  void DiDAddDetailedDescriptorTimingDataOnly(uint8_t myOffset, ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters);
};

#endif
