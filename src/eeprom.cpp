//===================================================================
// eeprom.cpp 
// Drivers for simulated EEPROM (in FLASH) and FRU EEPROM.
//===================================================================
#include <Arduino.h>
#include "main.hpp"
#include <Wire.h>
#include "FlashAsEEPROM_SAMD.h"
#include <time.h>
#include "eeprom.hpp"
#include "cli.hpp"
#include "commands.hpp"

// uncomment line below to enable hex dumps of EEPROM regions
//#define EEPROM_DEBUG 1

extern const uint16_t   static_pin_count;
extern char             *tokens[];
static char             outBfr[OUTBFR_SIZE];
const uint32_t          EEPROM_signature = 0xDE110C03;
uint8_t                 eepromAddresses[4] = {0x50, 0x52, 0x54, 0x56};      // NOTE: these DO NOT match Table 67
const uint32_t          jan1996 = 820454400;                                // epoch time (secs) of 1/1/1996 00:00

// FLASH/EEPROM Data buffer
EEPROM_data_t           EEPROMData;

// FRU EEPROM stuff
common_hdr_t            commonHeader;
board_hdr_t             boardHeader;
prod_hdr_t              prodHeader;
eeprom_desc_t           EEPROMDescriptor;
const char              sixBitASCII[64] = " !\"$%&'()*+,-./123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

//===================================================================
//                      EEPROM/NVM Stuff
//===================================================================

#define MAX_I2C_WRITE     16        // safe size, could be larger?
#define EEPROM_MAX_LEN    256

// temporary read buffer for FRU EEPROM
byte              EEPROMBuffer[EEPROM_MAX_LEN];

/**
  * @name   readEEPROM
  * @brief  read FRU EEPROM
  * @param  i2cAddr 
  * @param  eeaddress 
  * @param  dest pointer to write data to
  * @param  length in bytes to read
  * @retval None
  */
void readEEPROM(uint8_t i2cAddr, uint32_t eeaddress, uint8_t *dest, uint16_t length)
{
  if ( length > EEPROM_MAX_LEN )
    length = EEPROM_MAX_LEN;

  Wire.beginTransmission(i2cAddr);

  Wire.write((int)(eeaddress >> 8));      // MSB
  Wire.write((int)(eeaddress & 0xFF));    // LSB
  Wire.endTransmission();

  Wire.requestFrom(i2cAddr, length);

  while ( Wire.available() && length-- > 0 ) 
  {
      *dest++  = Wire.read();
  }
}

// --------------------------------------------
// writeEEPROMPage() - write a 'page' to the
// FRU EEPROM. NOTE: Not in use, here in case.
// NOTE: Not tested! Should work, hahah.
// --------------------------------------------
/**
  * @name   writeEEPROMPage
  * @brief  write page of data to FRU EEPROM
  * @param  None
  * @retval None
  * @note   NOT TESTED!
  */
void writeEEPROMPage(uint8_t i2cAddr, long eeAddress, byte *buffer)
{

  Wire.beginTransmission(i2cAddr);

  Wire.write((int)(eeAddress >> 8));        // MSB
  Wire.write((int)(eeAddress & 0xFF));      // LSB

  // Write bytes to EEPROM
  for (byte x = 0 ; x < MAX_I2C_WRITE ; x++)
    Wire.write(buffer[x]);                //Write the data

  Wire.endTransmission();                 //Send stop condition
}

// --------------------------------------------
// unpack6bitASCII()
// --------------------------------------------
/**
  * @name   unpack6bitASCII
  * @brief  unpack into 8-bit ASCII
  * @param  s pointer to write unpacked  data to
  * @param  field_length
  * @param  bytes pointer to packed data
  * @retval None
  */
void unpack6bitASCII(char *s, uint8_t field_length, uint8_t *bytes)
{
    uint8_t         temp;
    uint16_t        field_offset = 0;

    while ( field_length > 0 )
    {
        temp = bytes[field_offset] & 0x3F;
        *s++ = sixBitASCII[temp];

        temp = bytes[field_offset + 1] << 2 | (bytes[field_offset] >> 6);
        *s++ = sixBitASCII[temp];

        temp = (bytes[field_offset + 1] >> 4) | (bytes[field_offset + 2] & 3);
        *s++ = sixBitASCII[temp];

        *s++ = sixBitASCII[bytes[field_offset + 2] >> 2];

        field_length -= 3;
        field_offset += 3;
    }

    *s = 0;
}

// --------------------------------------------
// extractField() - extract next field in an
// EEPROM area
//
// Uses data in EEPROMBuffer at field_offset
// which is the type/length byte of the first
// field, then extracts/decodes the data into
// the char field pointed to by t.
// --------------------------------------------
uint16_t extractField(char *t, uint16_t field_offset)
{
    uint8_t             field_type = GET_TYPE(EEPROMBuffer[field_offset]);
    uint16_t            field_length = GET_LENGTH(EEPROMBuffer[field_offset]);
    uint8_t             temp;

    field_offset++;

    if ( field_type == 3 )
    {
        // 8-bit ASCII
        strncpy(t, (char *) &EEPROMBuffer[field_offset], field_length);
        t[field_length] = 0;
    }
    else if ( field_type == 2 )
    {
        // 6-bit ASCII
        unpack6bitASCII(t, field_length, &EEPROMBuffer[field_offset]);
    }
    else if ( field_type == 1 )
    {
        // BCD plus per 13.1 in platform mgt spec 
        // TODO - untested, haven't seen this type of data in test boards
        while ( field_length-- > 0 )
        {
            temp = EEPROMBuffer[field_offset] >> 4;
            if ( temp >= 0 && temp <= 9 )
                *t++ = temp | '0';
            else if ( temp == 0xA )
                *t++ = ' ';
            else if ( temp == 0xB )
                *t++ = '-';
            else if ( temp == 0xC )
                *t++ = '.';
            else
                *t++ = '?';
            temp = EEPROMBuffer[field_offset] & 0xF;
            if ( temp >= 0 && temp <= 9 )
                *t++ = temp | '0';
            else if ( temp == 0xA )
                *t++ = ' ';
            else if ( temp == 0xB )
                *t++ = '-';
            else if ( temp == 0xC )
                *t++ = '.';
            else
                *t++ = '?';

            field_offset++;
        }
    }
    else
    {
        // binary or unspecified
        strcpy(t, "Binary or unspecified format");
        while ( field_length-- > 0 )
        {
            sprintf(t++, "%02X", EEPROMBuffer[field_offset++]);
        }
    }

    // adjust field offset for caller past field just processed
    return(field_offset + field_length);
}

// --------------------------------------------
// eepromCmd() - 'eeprom' command works on FRU
// EEPROM only; simulated EEPROM is called 
// FLASH to user.
// --------------------------------------------
int eepromCmd(int arg)
{
    uint16_t          eepromAddr = 0;
    uint8_t           eepromI2CAddr = 0x52;
    char              tempStr[256];
    uint16_t          field_offset;
    uint8_t           slot;
    uint32_t          deltaTime;
    time_t            t;

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; cannot query FRU EEPROM");
        return(1);
    }

    // read the slot ID, which determines the FRU EEPROM I2C address
    // NOTE: Slot ID pins are tied to ground on TTF so zero here
    // TODO: Use slot ID as board rev?
    slot = 0;

    if ( slot >= 0 && slot <= 3 )
    {
        eepromI2CAddr = eepromAddresses[slot];
    }
    else
    {
        terminalOut((char *) "Invalid slot ID, cannot determine EEPROM I2C address");
        return(1);
    }

    if ( arg == 1 )
    {
        if ( strcmp(tokens[1], "show") != 0 )
        {
            terminalOut((char *) "Invalid subcommand, use 'show' to display EEPROM contents.");
            return(1);
        }
    }
    else if ( arg == 3 )
    {
        if ( strcmp(tokens[1], "dump") == 0 )
        {
            // 'eeprom dump <offset> <length>' command dumps FRU EEPROM at offset for length bytes
            uint16_t        offset = atoi(tokens[2]);
            uint16_t        length = atoi(tokens[3]);

            if ( offset > MAX_EEPROM_ADDR )
            {
                sprintf(outBfr, "offset of %d exceeds EEPROM capacity, use a smaller number", offset);
                SHOW();
                return(1);
            }

            if ( length > (16 * 20) )
            {
                sprintf(outBfr, "length of %d exceeds maximum, use a smaller number", length);
                SHOW();
                return(1);
            }

            readEEPROM(eepromI2CAddr, offset, EEPROMBuffer, length);
            dumpMem(EEPROMBuffer, length);
            return(0);
        }
        else
        {
            sprintf(outBfr, "Invalid eeprom subcommand '%s'", tokens[1]);
            SHOW();
            return(1);
        }
    }
    else
    {
        showCommandHelp(tokens[0]);
        return(1);
    }

    // the first byte in the EEPROM should be a 1 which is the format version
    // TODO: this may evolve over time and the code below need to be refactored
    readEEPROM(eepromI2CAddr, 0, EEPROMBuffer, 1);
    if ( EEPROMBuffer[0] != 1 )
    {
        sprintf(outBfr, "Unable to locate FRU EEPROM at expected SMB address 0x%02X", eepromI2CAddr);
        SHOW();
        return(0);
    }

    sprintf(outBfr, "FRU EEPROM found at SMB address 0x%02x", eepromI2CAddr);
    SHOW();

    // read common header
    readEEPROM(eepromI2CAddr, eepromAddr, (byte *) &commonHeader, sizeof(common_hdr_t));

#ifdef EEPROM_DEBUG
    dumpMem((unsigned char *) &commonHeader, sizeof(common_hdr_t));
#endif
    terminalOut((char *) "--- COMMON HEADER DATA");
    sprintf(outBfr, "Format version:  %d", commonHeader.format_vers & 0xF);
    SHOW();

    // all area offsets in common area are x8 bytes
    EEPROMDescriptor.internal_area_offset_actual = commonHeader.internal_area_offset * 8;
    EEPROMDescriptor.chassis_area_offset_actual = commonHeader.chassis_area_offset * 8;
    EEPROMDescriptor.board_area_offset_actual = commonHeader.board_area_offset * 8;
    EEPROMDescriptor.product_area_offset_actual = commonHeader.product_area_offset * 8;
    EEPROMDescriptor.multirecord_area_offset_actual = commonHeader.multirecord_area_offset * 8;

    sprintf(outBfr, "Int Use Area:    %d",  EEPROMDescriptor.internal_area_offset_actual);
    SHOW();

    sprintf(outBfr, "Chassis Area:    %d", EEPROMDescriptor.chassis_area_offset_actual);
    SHOW();

    sprintf(outBfr, "Board Area:      %d", EEPROMDescriptor.board_area_offset_actual);
    SHOW();

    sprintf(outBfr, "Product Area:    %d (not supported)", EEPROMDescriptor.product_area_offset_actual);
    SHOW();

    sprintf(outBfr, "MRecord Area:    %d (not supported)", EEPROMDescriptor.multirecord_area_offset_actual);
    SHOW();

    // read first 7 bytes of board info area "header" to determine length
    eepromAddr = EEPROMDescriptor.board_area_offset_actual;
    readEEPROM(eepromI2CAddr, eepromAddr, (byte *) &boardHeader, sizeof(board_hdr_t));
#ifdef EEPROM_DEBUG
    dumpMem(EEPROMBuffer, sizeof(board_hdr_t));
#endif
    EEPROMDescriptor.board_area_length = boardHeader.board_area_length * 8;

    terminalOut((char *) "--- BOARD AREA DATA");
    sprintf(outBfr, "Language Code:   %02X", boardHeader.language);
    SHOW();

    // format manufacturing date/time
    deltaTime = boardHeader.mfg_time[2] << 16 | boardHeader.mfg_time[1] << 8 | boardHeader.mfg_time[0];
    deltaTime *= 60;            // time in EEPROM is in minutes, convert to seconds
    deltaTime += jan1996;       // convert to epoch since 1/1/1970
    t = deltaTime;
    strcpy(tempStr, asctime(gmtime(&t)));
    tempStr[strcspn(tempStr, "\n")] = 0;
    sprintf(outBfr, "Mfg Date/Time:   %s", tempStr);
    SHOW();

    sprintf(outBfr, "Bd Area Length:  %d", EEPROMDescriptor.board_area_length);
    SHOW();

    // read the entire board area
    eepromAddr += sizeof(board_hdr_t);
    readEEPROM(eepromI2CAddr, eepromAddr, (byte *) &EEPROMBuffer, EEPROMDescriptor.board_area_length);
#ifdef EEPROM_DEBUG
    dumpMem((unsigned char *) EEPROMBuffer, EEPROMDescriptor.board_area_length);
#endif
    field_offset = 0;

    // extract manufacturer name; type/lenth is last item in board header
    field_offset = extractField(tempStr, field_offset);
    sprintf(outBfr, "Manufacturer:    %s", tempStr);
    SHOW();

    // extract the next field: product name
    field_offset = extractField(tempStr, field_offset);
    sprintf(outBfr, "Product Name:    %s", tempStr);
    SHOW();

    // extract the next field: serial number
    field_offset = extractField(tempStr, field_offset);
    sprintf(outBfr, "Serial Number:   %s", tempStr);
    SHOW();

    // extract the next field: part #
    field_offset = extractField(tempStr, field_offset);
    sprintf(outBfr, "Part Number:     %s", tempStr);
    SHOW();

    // FRU File ID
    field_offset = extractField(tempStr, field_offset);
    sprintf(outBfr, "FRU File ID:     %s", tempStr);
    SHOW();

    // NOTE: Custom product info area fields are NOT processed
    // nor is the check for the 0xC1 terminator checked

#if 0
// See GitHub Issue #4 (Need a decision on FRU EEPROM 'other areas'
    // read the product area header
    eepromAddr = EEPROMDescriptor.product_area_offset_actual;
    readEEPROM(eepromI2CAddr, eepromAddr, (uint8_t *) &prodHeader, sizeof(prod_hdr_t));
    eepromAddr += sizeof(prod_hdr_t);
    dumpMem((uint8_t *) &prodHeader, sizeof(prod_hdr_t));

    terminalOut("--- PRODUCT AREA DATA");
    sprintf(outBfr, "Format version %d", prodHeader.format_vers & 0xF);
    SHOW();

    sprintf(outBfr, "Language Code: %02X", prodHeader.language);
    SHOW();

    uint16_t        length = prodHeader.prod_area_length * 8;
    sprintf(outBfr, "Prod Area Size:%d", length);
    SHOW();

    readEEPROM(eepromI2CAddr, eepromAddr, EEPROMBuffer, length);
    dumpMem(EEPROMBuffer, length);
#endif // 0

    return(0);
}

// --------------------------------------------
// EEPROM_Save() - write EEPROM structure to
// the simulated EEPROM
// --------------------------------------------
void EEPROM_Save(void)
{
    uint8_t         *p = (uint8_t *) &EEPROMData;
    uint16_t        eepromAddr = 0;

    for ( int i = 0; i < (int) sizeof(EEPROM_data_t); i++ )
    {
        EEPROM.write(eepromAddr++, *p++);
    }
    
    EEPROM.commit();
}

// --------------------------------------------
// EEPROM_Read() - Read struct from simulated
// EEPROM
// --------------------------------------------
void EEPROM_Read(void)
{
    uint8_t         *p = (uint8_t *) &EEPROMData;
    uint16_t        eepromAddr = 0;

    for ( int i = 0; i < (int) sizeof(EEPROM_data_t); i++ )
    {
        *p++ = EEPROM.read(eepromAddr++);
    }
}

// --------------------------------------------
// EEPROM_Defaults() - Set defaults in struct
// --------------------------------------------
void EEPROM_Defaults(void)
{
    EEPROMData.sig = EEPROM_signature;
    EEPROMData.status_delay_secs = 3;
    EEPROMData.pwr_seq_delay_msec = 250;

    // TODO add other fields
}

// --------------------------------------------
// EEPROM_InitLocal() - Initialize EEPROM & Data
//
// Returns false if no error, else true
// NOTE: For simulated EEPROM, not FRU EEPROM
// --------------------------------------------
bool EEPROM_InitLocal(void)
{
    bool          rc = false;

    EEPROM_Read();

    if ( EEPROMData.sig != EEPROM_signature )
    {
      // EEPROM failed: either never been used, or real failure
      // initialize the signature and settings
 
      EEPROM_Defaults();

      // save EEPROM data on the device
      EEPROM_Save();

      rc = true;
      terminalOut((char *) "FLASH storage validation FAILED, FLASH defaults loaded");
    }
    else
    {
      terminalOut((char *) "FLASH storage validated OK");
    }

    return(rc);

} // EEPROM_InitLocal()




