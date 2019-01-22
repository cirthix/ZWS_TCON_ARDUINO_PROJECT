// Moves all of the read/write/update functions to eeprom storage into one place
// Most of this code is simple stubs, avoid cluttering the other files.

uint8_t UserConfiguration_LoadShutdown(){return EEPROM.read(ADDRESS_POWER_STATE);}
void UserConfiguration_SaveShutdown(uint8_t myValue){ EEPROM.update(ADDRESS_POWER_STATE, myValue); }
void UserConfiguration_SaveDefaultShutdown(){EEPROM.update(ADDRESS_POWER_STATE, FACTORY_DEFAULT_POWERSTATE); }

uint8_t UserConfiguration_LoadEDID(){return EEPROM.read(ADDRESS_SELECTED_EDID);}
void UserConfiguration_SaveEDID(uint8_t myValue){ EEPROM.update(ADDRESS_SELECTED_EDID, myValue); }
void UserConfiguration_SaveDefaultEDID(){EEPROM.update(ADDRESS_SELECTED_EDID, FACTORY_DEFAULT_SELECTED_EDID); }

uint8_t UserConfiguration_LoadBrightness(){return EEPROM.read(ADDRESS_BACKLIGHT_LEVEL);}
void UserConfiguration_SaveBrightness(uint8_t myValue){ EEPROM.update(ADDRESS_BACKLIGHT_LEVEL, myValue); }
void UserConfiguration_SaveDefaultBrightness(){EEPROM.update(ADDRESS_BACKLIGHT_LEVEL, FACTORY_DEFAULT_BACKLIGHT_BRIGHTNESS); }

uint8_t UserConfiguration_LoadOSD(){return EEPROM.read(ADDRESS_OSD_ENABLED);}
void UserConfiguration_SaveOSD(uint8_t myValue){ EEPROM.update(ADDRESS_OSD_ENABLED, myValue); }
void UserConfiguration_SaveDefaultOSD(){EEPROM.update(ADDRESS_OSD_ENABLED, FACTORY_DEFAULT_USE_OSD); }

uint8_t UserConfiguration_LoadCrosshair(){return EEPROM.read(ADDRESS_CROSSHAIR_ENABLED);}
void UserConfiguration_SaveCrosshair(uint8_t myValue){ EEPROM.update(ADDRESS_CROSSHAIR_ENABLED, myValue); }
void UserConfiguration_SaveDefaultCrosshair(){EEPROM.update(ADDRESS_CROSSHAIR_ENABLED, FACTORY_DEFAULT_USE_CROSSHAIR); }

uint8_t UserConfiguration_LoadMagicByte(){return EEPROM.read(ADDRESS_MAGIC_BYTE);}
void UserConfiguration_SaveDefaultMagicByte(){EEPROM.update(ADDRESS_MAGIC_BYTE, MagicByte()); }

uint8_t DetermineIfFactoryProgrammed(){ return (UserConfiguration_LoadMagicByte() == (MagicByte())); }

// This function is entirely compiled away into a single byte value known at compile-time
uint8_t MagicByte(){
const uint8_t array_size=24;
const uint8_t compile_date_time[array_size] = "" __DATE__ " " __TIME__ ""; // Example string: "Apr  3 2016 01:44:37"
uint8_t hashed_value= 
compile_date_time[0]+
compile_date_time[1]+
compile_date_time[2]+
compile_date_time[3]+
compile_date_time[4]+
compile_date_time[5]+
compile_date_time[6]+
compile_date_time[7]+
compile_date_time[8]+
compile_date_time[9]+
compile_date_time[10]+
compile_date_time[11]+
compile_date_time[12]+
compile_date_time[13]+
compile_date_time[14]+
compile_date_time[15]+
compile_date_time[16]+
compile_date_time[17]+
compile_date_time[18]+
compile_date_time[19];
  
  if(hashed_value== 0xFF || hashed_value==0x00) {return 0xa9;}
return hashed_value;
}
