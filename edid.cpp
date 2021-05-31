#include "edid.h"
#include "constants.h"
 
void PrintHexByte(uint8_t hexbyte){   
  uint8_t mychar=hexbyte>>4;
  if(mychar<0x0A) { Serial.write('0' + mychar);} else  { Serial.write('7' + mychar);}
  mychar=hexbyte&0x0F;
  if(mychar<0x0A) { Serial.write('0' + mychar);} else  { Serial.write('7' + mychar);}
}

EDID::EDID(){EDID::Reset();}

void EDID::Reset(){
  memset(RawBytes,0,sizeof(RawBytes));
  NumberOfFilledDescriptorBlocks = 0;
  NumberOfFilledCEADescriptorBlocks = 0;
}  

uint8_t EDID::GetByte(uint8_t address){
  return RawBytes[address];
}  


void EDID::SetByte(uint8_t address, uint8_t value){
  RawBytes[address]= value;
}
  
void EDID::SetHeader(){
  EDID::SetByte(0x00, 0x00);
  EDID::SetByte(0x01, 0xff);
  EDID::SetByte(0x02, 0xff);
  EDID::SetByte(0x03, 0xff);
  EDID::SetByte(0x04, 0xff);
  EDID::SetByte(0x05, 0xff);
  EDID::SetByte(0x06, 0xff);
  EDID::SetByte(0x07, 0x00);
  EDID::SetByte(0x12, 0x01); // Only support EDID 1.4
  EDID::SetByte(0x13, 0x04);
  EDID::SetExtensionBlockCount(0);
}

void EDID::SetExtensionBlockCount(uint8_t value){
  EDID::SetByte(0x7E, value);
}

uint8_t ConvertAsciiTo5BitLetters(uint8_t myChar){
  return (myChar - (uint8_t('A')-1))&0x1F;
}
  
void EDID::SetManufacturerID(uint8_t ManufacturerID[3]){
  // Input standard 8-bit ascii values
  uint16_t outvalue;
  uint8_t Letter1 = ConvertAsciiTo5BitLetters(ManufacturerID[0]);
  uint8_t Letter2 = ConvertAsciiTo5BitLetters(ManufacturerID[1]);
  uint8_t Letter3 = ConvertAsciiTo5BitLetters(ManufacturerID[2]);
  outvalue = 0x0000;
  outvalue = outvalue | uint16_t( Letter1 << 10);
  outvalue = outvalue | uint16_t( Letter2 << 5);
  outvalue = outvalue | uint16_t( Letter3 << 0);
  uint8_t HighByte = uint8_t((outvalue >>8) & 0x00FF);
  uint8_t LowByte = uint8_t(outvalue & 0x00FF);  
  EDID::SetByte(0x08, HighByte);
  EDID::SetByte(0x09, LowByte);
}
  
void EDID::SetManufacturerProductCode(uint16_t value){
  uint8_t HighByte = uint8_t((value >>8) & 0x00FF);
  uint8_t LowByte = uint8_t(value & 0x00FF);  
  EDID::SetByte(0x0B, HighByte);
  EDID::SetByte(0x0A, LowByte);
}

void EDID::SetManufacturerSerialNumber(uint32_t value){
  uint32_t TempValue = value;
  EDID::SetByte(0x0C, uint8_t(TempValue&0x000000FF));
  TempValue = TempValue << 8 ;
  EDID::SetByte(0x0D, uint8_t(TempValue&0x000000FF));
  TempValue = TempValue << 8 ;
  EDID::SetByte(0x0E, uint8_t(TempValue&0x000000FF));
  TempValue = TempValue << 8 ;
  EDID::SetByte(0x0F, uint8_t(TempValue&0x000000FF));
}

void EDID::SetManufactureWeek(uint8_t value){
  EDID::SetByte(0x10, value);
}

void EDID::SetManufactureYear(uint16_t value){
  uint8_t EncodedValue = uint8_t(value - 1990);
  EDID::SetByte(0x11, EncodedValue);
}

void EDID::SetDisplayPort10Bit(){
  EDID::SetByte(0x14, 0xB5);
}

void EDID::SetDisplayPort8Bit(){
  EDID::SetByte(0x14, 0xA5);
}

void EDID::SetPhysicalWidthInCentimeters(uint8_t value){
  EDID::SetByte(0x15, value);
}
void EDID::SetPhysicalHeightInCentimeters(uint8_t value){
  EDID::SetByte(0x16, value);
}
void EDID::SetReportedGammaValueTimes100(uint16_t value){
  uint8_t EncodedValue = uint8_t(value - 100);
  EDID::SetByte(0x17, EncodedValue);
}

void EDID::SetRGB444WithNoDPMSWithNoNativeTimingsAndNoContinuiousFrequency(){
  EDID::SetByte(0x18, 0x04);
}

void EDID::SetRGB444WithNoDPMSWithNativeTimingsAndNoContinuiousFrequency(){
  EDID::SetByte(0x18, 0x06);
}
uint8_t ConvertChromaticityUint16ToLowByte(uint16_t value){
  uint8_t LowByte = uint8_t(value & 0x0003);  
  return LowByte;
}
uint8_t ConvertChromaticityUint16ToHighByte(uint16_t value){
  uint8_t HighByte = uint8_t((value >>2) & 0x00FF);
  return HighByte;
}
uint16_t ConvertChromaticityFloatToUint16(float value){
  //SerialDebug("Input = "); SerialDebug(value,3);
  uint16_t FixedPoint = round(value*1024);
  //uint8_t HighByte = ConvertChromaticityUint16ToHighByte(FixedPoint);
  //uint8_t LowByte = ConvertChromaticityUint16ToLowByte(FixedPoint);  
  //SerialDebug(" Output = "); PrintHexByte(HighByte); PrintHexByte(LowByte); SerialDebugln();
  return FixedPoint;
}
  
void EDID::SetReportedChromaticites(Chromaticities myChromaticities){
  
  EDID::SetByte(0x19, 
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.rx))<<6 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.ry))<<4 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.gx))<<2 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.gy))<<0 );

  EDID::SetByte(0x1A, 
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.bx))<<6 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.by))<<4 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.wx))<<2 |
  ConvertChromaticityUint16ToLowByte(ConvertChromaticityFloatToUint16(myChromaticities.wy))<<0 );

  EDID::SetByte(0x1B, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.rx)));
  EDID::SetByte(0x1C, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.ry)));
  EDID::SetByte(0x1D, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.gx)));
  EDID::SetByte(0x1E, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.gy)));
  EDID::SetByte(0x1F, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.bx)));
  EDID::SetByte(0x20, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.by)));
  EDID::SetByte(0x21, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.wx)));
  EDID::SetByte(0x22, ConvertChromaticityUint16ToHighByte(ConvertChromaticityFloatToUint16(myChromaticities.wy)));
  }
  
void EDID::SetNoLegacyStandardVideoModes(){
  EDID::SetByte(0x23, 0x00);
  EDID::SetByte(0x24, 0x00);
  EDID::SetByte(0x25, 0x00);  
}
  
void EDID::SetNoGenericVideoModes(){
  EDID::SetByte(0x26, 0x01);
  EDID::SetByte(0x27, 0x01);
  EDID::SetByte(0x28, 0x01);  
  EDID::SetByte(0x29, 0x01); 
  EDID::SetByte(0x2A, 0x01); 
  EDID::SetByte(0x2B, 0x01); 
  EDID::SetByte(0x2C, 0x01); 
  EDID::SetByte(0x2D, 0x01); 
  EDID::SetByte(0x2E, 0x01); 
  EDID::SetByte(0x2F, 0x01); 
  EDID::SetByte(0x30, 0x01);
  EDID::SetByte(0x31, 0x01);
  EDID::SetByte(0x32, 0x01);
  EDID::SetByte(0x33, 0x01); 
  EDID::SetByte(0x34, 0x01);
  EDID::SetByte(0x35, 0x01);
}


void EDID::AddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters){
  if(myModeLine.HActive==0) {
   // SerialDebug("SkippingDTD");
    return;
  }
  EDID::AddDetailedDescriptorTiming18BytesToOffset(EDID::GetDetailedDescriptorBlockOffset(), myModeLine, HSizeInMilliMeters, VSizeInMilliMeters);
  EDID::IncrementNumberOfFilledDescriptorBlocks();
}

void EDID::AddDetailedDescriptorRangeLimitsOnly(uint16_t MinVHz, uint16_t MaxVHz, uint16_t MinHKhz, uint16_t MaxHKhz, uint16_t MaxPixelClock){
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x00, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x01, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x02, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x03, 0xFD);
  uint8_t LimitOffsets = 0x00;
  uint8_t PostOffsetMinVHz;
  uint8_t PostOffsetMaxVHz;
  uint8_t PostOffsetMinHKhz;
  uint8_t PostOffsetMaxHKhz;
  if(MinVHz >255) {LimitOffsets = LimitOffsets | 0x01; PostOffsetMinVHz  = MinVHz  - 255; } else {PostOffsetMinVHz  = MinVHz  ;}
  if(MaxVHz >255) {LimitOffsets = LimitOffsets | 0x02; PostOffsetMaxVHz  = MaxVHz  - 255; } else {PostOffsetMaxVHz  = MaxVHz  ;}
  if(MinHKhz>255) {LimitOffsets = LimitOffsets | 0x04; PostOffsetMinHKhz = MinHKhz - 255; } else {PostOffsetMinHKhz = MinHKhz ;}
  if(MaxHKhz>255) {LimitOffsets = LimitOffsets | 0x08; PostOffsetMaxHKhz = MaxHKhz - 255; } else {PostOffsetMaxHKhz = MaxHKhz ;}

  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x04, LimitOffsets);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x05, PostOffsetMinVHz);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x06, PostOffsetMaxVHz);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x07, PostOffsetMinHKhz);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x08, PostOffsetMaxHKhz);

  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x09, ceil(MaxPixelClock*0.1));
  // Also set no timing information - limits only
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0A, 0x01);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0B, 0x0A);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0C, 0x20);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0D, 0x20);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0E, 0x20);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x0F, 0x20);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x10, 0x20);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x11, 0x20);
  EDID::IncrementNumberOfFilledDescriptorBlocks();
}

void EDID::AddDetailedDescriptorName(uint8_t NameLength, uint8_t* NamePointer){  
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x00, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x01, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x02, 0x00);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x03, 0xFC);
  EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x04, 0x00);
  for(uint8_t i=0; i<13; i++){
    if(i <  NameLength) {EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x05 + i, NamePointer[i]);}
    if(i == NameLength) {EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x05 + i, 0x0A);}
    if(i >  NameLength) {EDID::SetByte(EDID::GetDetailedDescriptorBlockOffset() + 0x05 + i, ' ');}
  }
  EDID::IncrementNumberOfFilledDescriptorBlocks();
}

uint8_t EDID::GetDetailedDescriptorBlockOffset(){
  return 0x36 + NumberOfFilledDescriptorBlocks*18;
}

void EDID::IncrementNumberOfFilledDescriptorBlocks(){
  if(NumberOfFilledDescriptorBlocks < MaxNumberOfFilledDescriptorBlocks) {
    NumberOfFilledDescriptorBlocks = NumberOfFilledDescriptorBlocks +1;
  }
}

boolean EDID::QueryIfAllDescriptorBlocksAreUsed(){
  if(NumberOfFilledDescriptorBlocks < MaxNumberOfFilledDescriptorBlocks) {
    return false;
  } else { 
    return true;
  }
}

uint8_t EDID::CalculateSumBlock(uint8_t block){
  uint16_t AccumulatedValue = 0;
  for(uint16_t i=EDID_BLOCK_SIZE*block; i<(EDID_BLOCK_SIZE*(block+1))-1; i++){
    AccumulatedValue += EDID::GetByte(i);
  }
  return uint8_t(AccumulatedValue&0x00FF);
}

void EDID::FixChecksumBaseBlock(){
  uint8_t CheckSum = 0x00 - CalculateSumBlock(0);
  EDID::SetByte(0x7F, CheckSum);  
}

void EDID::FixChecksumExtensionBlock(){
  uint8_t CheckSum = 0x00 - CalculateSumBlock(1);
  EDID::SetByte(0xFF, CheckSum);  
}

void EDID::CEACreateBlock(){
  EDID::SetExtensionBlockCount(1);
  //Note: already memset to 0 during class initilization
  EDID::SetByte(CEABlockOffset+0, 0x02);  // CEA extension ID
  EDID::SetByte(CEABlockOffset+1, 0x03);  // CEA version    
  EDID::SetByte(CEABlockOffset+2, 0x04);  // No extra blocks (DTD offset = 0x04)
  EDID::SetByte(CEABlockOffset+3, 0x00);  // No features described
}

void EDID::CEAAddHDMI(){
  uint8_t myByteOffset = CEABlockOffset + EDID::GetByte(CEABlockOffset+2);
  EDID::SetByte(CEABlockOffset+2, EDID::GetByte(CEABlockOffset+2)+0x08);
  // Add empty HDMI block, 340MHz max, no features
  EDID::SetByte(myByteOffset+0, 0x67);
  EDID::SetByte(myByteOffset+1, 0x03);
  EDID::SetByte(myByteOffset+2, 0x0C);
  EDID::SetByte(myByteOffset+3, 0x00);
  EDID::SetByte(myByteOffset+4, 0x00);
  EDID::SetByte(myByteOffset+5, 0x00);
  EDID::SetByte(myByteOffset+6, 0x00);
  EDID::SetByte(myByteOffset+7, 0x44);
}

void EDID::CEAAddHDMITwoPointOne(){
  uint8_t myByteOffset = CEABlockOffset + EDID::GetByte(CEABlockOffset+2);
  EDID::SetByte(CEABlockOffset+2, EDID::GetByte(CEABlockOffset+2)+0x0E);
  // 600MHz max, no features
  EDID::SetByte(myByteOffset+0, 0x6D);
  EDID::SetByte(myByteOffset+1, 0xD8);
  EDID::SetByte(myByteOffset+2, 0x5D);
  EDID::SetByte(myByteOffset+3, 0xC4);
  EDID::SetByte(myByteOffset+4, 0x01);
  EDID::SetByte(myByteOffset+5, 0x78);
  EDID::SetByte(myByteOffset+6, 0x00);
  EDID::SetByte(myByteOffset+7, 0x00);
  EDID::SetByte(myByteOffset+8, 0x00);
  EDID::SetByte(myByteOffset+9, 0x00);
  EDID::SetByte(myByteOffset+10, 0x00);
  EDID::SetByte(myByteOffset+11, 0x00);
  EDID::SetByte(myByteOffset+12, 0x00);
  EDID::SetByte(myByteOffset+13, 0x00);
}

void EDID::CEAAddSupportedStandardModes(){
  uint8_t myByteOffset = CEABlockOffset + EDID::GetByte(CEABlockOffset+2);
  EDID::SetByte(CEABlockOffset+2, EDID::GetByte(CEABlockOffset+2)+0x04);
  // Add 1080p60, 1080p120, 4K60 as "standard" modes for PS5 compatibility
  EDID::SetByte(myByteOffset+0, 0x43);
  EDID::SetByte(myByteOffset+1, 0x10);
  EDID::SetByte(myByteOffset+2, 0x3F);
  EDID::SetByte(myByteOffset+3, 0x61);
}

void EDID::CEAAddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters){
  const uint8_t CEA_DTD_SIZE = 18;
  uint8_t myByteOffset = CEABlockOffset + EDID::GetByte(CEABlockOffset+2) + NumberOfFilledCEADescriptorBlocks * CEA_DTD_SIZE;
  if(myModeLine.HActive==0) { 
  //  SerialDebug("SkippingDTD");
    return;
  }
  EDID::AddDetailedDescriptorTiming18BytesToOffset(myByteOffset, myModeLine, HSizeInMilliMeters, VSizeInMilliMeters);
  NumberOfFilledCEADescriptorBlocks = NumberOfFilledCEADescriptorBlocks +1;
  EDID::SetByte(CEABlockOffset+3, NumberOfFilledCEADescriptorBlocks);  // update number of native formats
}

void EDID::AddDetailedDescriptorTiming18BytesToOffset(uint8_t myOffset, ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters){
  uint16_t PixelClockInteger = round(100*myModeLine.PixelClock);
  uint16_t HBlank = myModeLine.HFP+myModeLine.HSW+myModeLine.HBP;
  uint16_t VBlank = myModeLine.VFP+myModeLine.VSW+myModeLine.VBP;
  EDID::SetByte(myOffset + 0x00, (PixelClockInteger)&0x00FF);
  EDID::SetByte(myOffset + 0x01, (PixelClockInteger>>8)&0x00FF);
  EDID::SetByte(myOffset + 0x02, (myModeLine.HActive)&0x00FF);
  EDID::SetByte(myOffset + 0x03, (HBlank)&0x00FF);
  EDID::SetByte(myOffset + 0x04, ((((myModeLine.HActive>>8)&0x000F)<<4) | ((HBlank>>8)&0x000F)));
  EDID::SetByte(myOffset + 0x05, (myModeLine.VActive)&0x00FF);
  EDID::SetByte(myOffset + 0x06, (VBlank)&0x00FF);
  EDID::SetByte(myOffset + 0x07, ((((myModeLine.VActive>>8)&0x000F)<<4) | ((VBlank>>8)&0x000F)));
  EDID::SetByte(myOffset + 0x08, (myModeLine.HFP)&0x00FF);
  EDID::SetByte(myOffset + 0x09, (myModeLine.HSW)&0x00FF);
  EDID::SetByte(myOffset + 0x0A, (((myModeLine.VFP&0x000F)<<4) | (myModeLine.VSW&0x000F)));
  EDID::SetByte(myOffset + 0x0B, 
  (((myModeLine.HFP>>8)&0x0003) <<6) |
  (((myModeLine.HSW>>8)&0x0003) <<4) |
  (((myModeLine.VFP>>4)&0x0003) <<2) |
  (((myModeLine.VSW>>4)&0x0003) <<0) );
  EDID::SetByte(myOffset + 0x0C, (HSizeInMilliMeters)&0x00FF);
  EDID::SetByte(myOffset + 0x0D, (VSizeInMilliMeters)&0x00FF);
  EDID::SetByte(myOffset + 0x0E, ((((HSizeInMilliMeters>>8)&0x000F)<<4) | ((VSizeInMilliMeters>>8)&0x000F)));
  // Assume zero-borders, 2d image,  and +HSYNC +VSYNC  
  EDID::SetByte(myOffset + 0x0F, 0x00);
  EDID::SetByte(myOffset + 0x10, 0x00);
  EDID::SetByte(myOffset + 0x11, 0x1E);
}

void EDID::DiDCreateBlock(){
  EDID::SetExtensionBlockCount(1);
  //Note: already memset to 0 during class initilization
  EDID::SetByte(DiDBlockOffset+0, 0x70);  // DiD extension ID
  EDID::SetByte(DiDBlockOffset+1, 0x12);  // DiD version    
  EDID::SetByte(DiDBlockOffset+2, 0x79);  // Number of bytes in DiD payload.  Rather than keep track, just claim the whole EDID extension block
  EDID::SetByte(DiDBlockOffset+3, 0x00);  // Generic Display
  EDID::SetByte(DiDBlockOffset+4, 0x00);  // DiD extension count  
  DiDPreferred = 0x80;
  DiDByteCount = 0;
  DiDDetailedDescriptorByteCountAddress = 0;
}

void EDID::DiDAddToByteCount(uint8_t value){
  DiDByteCount = DiDByteCount + value;
}

uint8_t EDID::DiDGetDescriptorOffset(){  
  uint8_t myOffset = DiDBlockOffset + 5 + DiDByteCount;
  return myOffset;
}

void EDID::DiDAddTiledDescriptor(uint8_t ManufacturerID[3], uint16_t ProductID, uint32_t SerialNumber, uint8_t HTiles, uint8_t VTiles, uint8_t HTileLocation, uint8_t VTileLocation, uint16_t HTileSize, uint16_t VTileSize){
  uint8_t myOffset = EDID::DiDGetDescriptorOffset();
  uint16_t OffsetHTileSize = HTileSize -1;
  uint16_t OffsetVTileSize = VTileSize -1;
  uint16_t OffsetHTiles = HTiles -1;
  uint16_t OffsetVTiles = VTiles -1;
  EDID::DiDAddToByteCount(0x19);  
  EDID::SetByte(myOffset + 0x00, 0x12);  // DiD Tiled block ID
  EDID::SetByte(myOffset + 0x01, 0x00);  // DiD Tiled block ID version
  EDID::SetByte(myOffset + 0x02, 0x16);  // DiD Tiled block ID payload size

  // Current shipping zws value is EDID::SetByte(myOffset + 0x03, 0b10001001);  // Partial driven behavior : tiles to their locations, no bezels, single enclosure

  if(HTileLocation + VTileLocation*HTiles == 0){    
  EDID::SetByte(myOffset + 0x03, 0x82); // Image scaled to fit
  } else {    
  EDID::SetByte(myOffset + 0x03, 0x80); // "Described elsewhere"
  }
  
  EDID::SetByte(myOffset + 0x04, (((OffsetHTiles&0x000F)<<4) | (OffsetVTiles&0x000F)));
  EDID::SetByte(myOffset + 0x05, ((((HTileLocation)&0x000F)<<4) | ((VTileLocation)&0x000F)));
  EDID::SetByte(myOffset + 0x06, ((((OffsetHTiles&0x0030)>>4)<<6) | (((OffsetVTiles&0x0030)>>4)<<4)| (((HTileLocation&0x0030)>>4)<<2)| (((VTileLocation&0x0030)>>4)<<0)));
  EDID::SetByte(myOffset + 0x07, OffsetHTileSize&0x00FF);
  EDID::SetByte(myOffset + 0x08, (OffsetHTileSize>>8)&0x00FF);
  EDID::SetByte(myOffset + 0x09, OffsetVTileSize&0x00FF);
  EDID::SetByte(myOffset + 0x0A, (OffsetVTileSize>>8)&0x00FF);
  EDID::SetByte(myOffset + 0x0B, 0x00);
  EDID::SetByte(myOffset + 0x0C, 0x00);
  EDID::SetByte(myOffset + 0x0D, 0x00);
  EDID::SetByte(myOffset + 0x0E, 0x00);
  EDID::SetByte(myOffset + 0x0F, 0x00);
  EDID::SetByte(myOffset + 0x10, ManufacturerID[0]);
  EDID::SetByte(myOffset + 0x11, ManufacturerID[1]);
  EDID::SetByte(myOffset + 0x12, ManufacturerID[2]);
  EDID::SetByte(myOffset + 0x13, ProductID&0x00FF);
  EDID::SetByte(myOffset + 0x14, (ProductID>>8)&0x00FF);
  EDID::SetByte(myOffset + 0x15, (SerialNumber>>0)&0x000000FF);
  EDID::SetByte(myOffset + 0x16, (SerialNumber>>8)&0x000000FF);
  EDID::SetByte(myOffset + 0x17, (SerialNumber>>16)&0x000000FF);
  EDID::SetByte(myOffset + 0x18, (SerialNumber>>24)&0x000000FF);
}



void EDID::DiDAddDetailedDescriptorTiming(ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters){
  uint8_t myOffset = EDID::DiDGetDescriptorOffset();
  if(myModeLine.HActive==0) { 
    //SerialDebug("SkippingDTD"); 
    return; 
    }
  if(DiDDetailedDescriptorByteCountAddress == 0x00){
    DiDDetailedDescriptorByteCountAddress = myOffset + 0x02;
    EDID::DiDAddToByteCount(0x17);  
    EDID::SetByte(myOffset + 0x00, 0x03);  // DiD detailed timing block ID
    EDID::SetByte(myOffset + 0x01, 0x00);  // DiD detailed timing block ID version
    EDID::SetByte(myOffset + 0x02, 0x14);  // DiD detailed timing block ID payload size   
    EDID::DiDAddDetailedDescriptorTimingDataOnly(myOffset+0x03, myModeLine, HSizeInMilliMeters, VSizeInMilliMeters);
  } else {
    EDID::SetByte(DiDDetailedDescriptorByteCountAddress, 0x14 + EDID::GetByte(DiDDetailedDescriptorByteCountAddress));   
    EDID::DiDAddDetailedDescriptorTimingDataOnly(myOffset+0x00, myModeLine, HSizeInMilliMeters, VSizeInMilliMeters);
  }
}

void EDID::DiDAddDetailedDescriptorTimingDataOnly(uint8_t myOffset, ModeLine myModeLine, uint16_t HSizeInMilliMeters, uint16_t VSizeInMilliMeters){
  uint32_t PixelClockInteger = round(100*myModeLine.PixelClock);
  EDID::SetByte(myOffset + 0x00, (PixelClockInteger)&0x000000FF);
  EDID::SetByte(myOffset + 0x01, (PixelClockInteger>>8)&0x000000FF);
  EDID::SetByte(myOffset + 0x02, ((PixelClockInteger>>16)&0x000000FF));
  EDID::SetByte(myOffset + 0x03, (0x08 | DiDPreferred)); // Set 2d display
  DiDPreferred = 0x00; // Let's follow EDID tradition and list the first DTD as preferred.
  uint16_t OffsetHActive = myModeLine.HActive -1;
  uint16_t OffsetVActive = myModeLine.VActive -1;  
  uint16_t OffsetHFP = myModeLine.HFP -1;
  uint16_t OffsetVFP = myModeLine.VFP -1;
  uint16_t OffsetHSW = myModeLine.HSW -1;
  uint16_t OffsetVSW = myModeLine.VSW -1;
  uint16_t HBlank = myModeLine.HFP+myModeLine.HSW+myModeLine.HBP-1;
  uint16_t VBlank = myModeLine.VFP+myModeLine.VSW+myModeLine.VBP-1;    
  EDID::SetByte(myOffset + 0x04, (OffsetHActive)&0x00FF);
  EDID::SetByte(myOffset + 0x05, ((OffsetHActive>>8)&0x00FF));
  EDID::SetByte(myOffset + 0x06, (HBlank)&0x00FF);
  EDID::SetByte(myOffset + 0x07, ((HBlank>>8)&0x00FF));
  EDID::SetByte(myOffset + 0x08, (OffsetHFP)&0x00FF);
  EDID::SetByte(myOffset + 0x09, ((OffsetHFP>>8)&0x00FF) | 0x80); // Set +Hsync;
  EDID::SetByte(myOffset + 0x0A, (OffsetHSW)&0x00FF);
  EDID::SetByte(myOffset + 0x0B, ((OffsetHSW>>8)&0x00FF));
  EDID::SetByte(myOffset + 0x0C, (OffsetVActive)&0x00FF);
  EDID::SetByte(myOffset + 0x0D, ((OffsetVActive>>8)&0x00FF));
  EDID::SetByte(myOffset + 0x0E, (VBlank)&0x00FF);
  EDID::SetByte(myOffset + 0x0F, ((VBlank>>8)&0x00FF));
  EDID::SetByte(myOffset + 0x10, (OffsetVFP)&0x00FF);
  EDID::SetByte(myOffset + 0x11, ((OffsetVFP>>8)&0x00FF) | 0x80); // Set +Vsync;
  EDID::SetByte(myOffset + 0x12, (OffsetVSW)&0x00FF);
  EDID::SetByte(myOffset + 0x13, ((OffsetVSW>>8)&0x00FF));
}




void EDID::DiDSetChecksum(){  
  uint16_t AccumulatedValue = 0;
  uint16_t DidBlockStart = DiDBlockOffset+1;
  uint16_t DidBlockStop = DiDBlockOffset+5+EDID::GetByte(DiDBlockOffset+2);  
  for(uint16_t i=DidBlockStart; i<DidBlockStop; i++){
    AccumulatedValue += EDID::GetByte(i);
  }
  uint8_t DiDChecksum = uint8_t(AccumulatedValue&0x00FF);  
  uint8_t CheckSum = 0x00 - DiDChecksum;
  EDID::SetByte(DidBlockStop, CheckSum);  
}
  
void EDID::PrintEDID(){    
  SerialDebugln("EDID Buffer:");
  for(uint16_t i=0; i<EDID_BLOCK_SIZE; i++){
    SerialDebug(" ");
    //  SerialDebug("0x");
    PrintHexByte(EDID::GetByte(i));
    if(i%16 == 15) {SerialDebugln("");}
  }
  uint8_t print_the_extension_block=false;
  for(uint16_t i=128; i<EDID_SIZE; i++){
    if(EDID::GetByte(i)!=0x00) {print_the_extension_block=true;}
  }
  if(print_the_extension_block==true) {
    for(uint16_t i=128; i<EDID_SIZE; i++){
      SerialDebug(" ");
      PrintHexByte(EDID::GetByte(i));
      if(i%16 == 15) {SerialDebugln("");}
    }
  }
  SerialDebugln("");
}


