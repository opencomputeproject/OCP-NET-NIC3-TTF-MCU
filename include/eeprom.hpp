#ifndef _EEPROM_H_
#define _EEPROM_H_

//===================================================================
// eeprom.hpp
// Defines for EEPROM drivers - see eeprom.cpp for code.
//===================================================================
#include <stdint-gcc.h>

#define MAX_EEPROM_ADDR       (8 * 1024 - 1)

// EEPROM data storage struct
typedef struct {
    uint32_t        sig;                  // unique EEPROMP signature (see #define)
    uint16_t        status_delay_secs;    // time in secs to delay updating status display
    uint16_t        pwr_seq_delay_msec;   // time between MAIN and AUX pwr enables
    
    // TODO add more data

} EEPROM_data_t;

// Table 8-1 COMMON HEADER
typedef struct {
  uint8_t         format_vers;
  uint8_t         internal_area_offset;
  uint8_t         chassis_area_offset;
  uint8_t         board_area_offset;
  uint8_t         product_area_offset;
  uint8_t         multirecord_area_offset;
  uint8_t         pad;
  uint8_t         cksum;
} common_hdr_t;

// Table 11-1 BOARD INFO AREA "header" (first 6 bytes before repeating type/length + data sets)
typedef struct {
  uint8_t         format_vers;
  uint8_t         board_area_length;
  uint8_t         language;
  uint8_t         mfg_time[3];           // mins since 0:00 1/1/1996 little endian
} board_hdr_t;

// Table 12-1 PRODUCT INFO AREA "header" (first 4 bytes before repeating type/length + data sets)
typedef struct {
    uint8_t     format_vers;
    uint8_t     prod_area_length;
    uint8_t     language;
    uint8_t     manuf_type_length;
} prod_hdr_t;

// internal structure used to keep track of FRU section offsets/lengths
typedef struct {
  uint16_t        board_area_offset_actual;
  uint16_t        board_area_length;
  uint16_t        product_area_offset_actual;
  uint16_t        internal_area_offset_actual;
  uint16_t        chassis_area_offset_actual;
  uint16_t        multirecord_area_offset_actual;
} eeprom_desc_t;

// see section 13 TYPE/LENGTH BYTE FORMAT
#define TYPE_LENGTH_MASK        0x3F
#define GET_TYPE(x)             (x >> 6)
#define GET_LENGTH(x)           (x & TYPE_LENGTH_MASK)

int eepromCmd(int arg);
void EEPROM_Save(void);
void EEPROM_Read(void);
void EEPROM_Defaults(void);
bool EEPROM_InitLocal(void);
void readEEPROM(uint8_t i2cAddr, uint32_t eeaddress, uint8_t *dest, uint16_t length);
void writeEEPROMPage(uint8_t i2cAddr, long eeAddress, uint8_t *buffer);

#endif // _EEPROM_H_