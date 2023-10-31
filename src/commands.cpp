//===================================================================
// commands.cpp
//
// Functions specific to an OCP project - these are the command
// functions called by the CLI.  Also contains pin definitions that
// are used by the pins, read and write commands. See also main.hpp
// for the actual Arduino pin numbering that aligns with variants.cpp
// README.md in platformio folder of repo has details about this file.
//===================================================================
#include "main.hpp"
#include "eeprom.hpp"
#include "commands.hpp"
#include <math.h>

extern char                 *tokens[];
extern EEPROM_data_t        EEPROMData;
extern volatile uint32_t    scanClockPulseCounter;
extern volatile bool        enableScanClk;
extern volatile uint32_t    scanShiftRegister_0;

// pin defs used for 1) pin init and 2) copied into volatile status structure
// to maintain state of inputs pins that get written 3) pin names (nice, right?) ;-)
// NOTE: Any I/O that is connected to the DIP switches HAS to be an input because those
// switches can be strapped to ground.  Thus, if the pin was an output and a 1 was
// written, there would be a dead short on that pin (no resistors).
// NOTE: The order of the entries in this table is the order they are displayed by the
// 'pins' command. There is no other signficance to the order.  The first entry in a
// pair is the left column while the second entry is the right column.
 const pin_mgt_t     staticPins[] = {
  {               TEMP_WARN, INPUT,     ACT_HI, "TEMP_WARN"},
  {         OCP_MAIN_PWR_EN, OUTPUT,    ACT_HI, "MAIN_EN"},

  {               TEMP_CRIT, INPUT,     ACT_HI, "TEMP_CRIT"},
  {          OCP_AUX_PWR_EN, OUTPUT,    ACT_HI, "AUX_EN"},

  {              FAN_ON_AUX, INPUT,     ACT_HI, "FAN_ON_AUX"},
  {              ATX_PWR_OK, INPUT,     ACT_LO, "ATX_PWR_OK"},

  {           OCP_PRSNTB0_N, INPUT,     ACT_LO, "PRSNTB0_N"},
  {              SCAN_VER_0, INPUT,     ACT_HI, "SCAN_VER_0"},

  {           OCP_PRSNTB1_N, INPUT,     ACT_LO, "PRSNTB1_N"},
  {              SCAN_VER_1, INPUT,     ACT_HI, "SCAN_VER_1"},

  {           OCP_PRSNTB2_N, INPUT,     ACT_LO, "PRSNTB2_N"},
  {              P1_LINKA_N, INPUT,     ACT_LO, "P1_LINKA_N"},

  {           OCP_PRSNTB3_N, INPUT,     ACT_LO, "PRSNTB3_N"},
  {            P1_LED_ACT_N, INPUT,     ACT_LO, "P1_LED_ACT_N"},

  {              OCP_WAKE_N, INPUT,     ACT_LO, "WAKE_N"},
  {            OCP_PWRBRK_N, INPUT,     ACT_LO, "PWRBRK_N"},

  {              P3_LINKA_N, INPUT,     ACT_LO, "P3_LINKA_N"},
  {            P3_LED_ACT_N, INPUT,     ACT_LO, "P3_LED_ACT_N"},

  {              BOARD_ID_0, INPUT_PULLDOWN, ACT_HI, "BOARD_ID_0"},
  {              NCSI_RST_N, OUTPUT,    ACT_LO, "NCSI_RST_N"},

  {              BOARD_ID_1, INPUT_PULLDOWN, ACT_HI, "BOARD_ID_1"},
  {       OCP_HEARTBEAT_LED, OUTPUT,    ACT_LO, "HEARTBEAT_LED"},     // HACK: temporary LED between UART pins 1 & 3

  {              BOARD_ID_2, INPUT_PULLDOWN, ACT_HI, "BOARD_ID_2"},
  {        NIC_PWR_GOOD_JMP, INPUT,     ACT_HI, "NIC_PWR_GOOD"},      // HACK: PWR_GOOD_LED to UART connector pin 2

  {           OCP_SCAN_LD_N, OUTPUT,    ACT_LO, "SCAN_LD_N"},
  {        OCP_SCAN_DATA_IN, INPUT,     ACT_HI, "SCAN_DATA_IN"},      // "in" from NIC 3.0 card (baseboard perspective)

  {            OCP_SCAN_CLK, OUTPUT,    ACT_LO, "SCAN_CLK"},
  {       OCP_SCAN_DATA_OUT, OUTPUT,    ACT_HI, "SCAN_DATA_OUT"},     // "out" to NIC 3.0 card
};

uint16_t      static_pin_count = sizeof(staticPins) / sizeof(pin_mgt_t);

typedef struct {
    uint8_t     bitNo;
    char        bitName[20];
} scan_data_t;

// NOTE: This table is processed aligning with bits 31..0
// The bitNo entry is NOT used
const scan_data_t     scanBitNames[] = {
    // Byte 0
    {7, "0.7 FAN_ON_AUX"},
    {6, "0.6 TEMP_CRIT_N"},
    {5, "0.5 TEMP_WARN_N"},
    {4, "0.4 WAKE_N"},
    {3, "0.3 PRSNTB[3]_P#"},
    {2, "0.2 PRSNTB[2]_P#"},
    {1, "0.1 PRSNTB[1]_P#"},
    {0, "0.0 PRSNTB[0]_P#"},

    // Byte 1
    {15, "1.7 LINK_SPDB_P2#"},
    {14, "1.6 LINK_SPDA_P2#"},
    {13, "1.5 ACT_P1#"},
    {12, "1.4 LINK_SPDB_P1#"},
    {11, "1.3 LINK_SPDA_P1#"},
    {10, "1.2 ACT_PO#"},
    {9, "1.1 LINK_SPDB_PO#"},
    {8, "1.0 LINK_SPDA_PO#"},

    // Byte 2
    {23, "2.7 LINK_SPDA_P5#"},
    {22, "2.6 ACT_P4#"},
    {21, "2.5 LINK_SPDB_P4#"},
    {20, "2.4 LINK_SPDA_P4#"},
    {19, "2.3 ACT_P3#"},
    {18, "2.2 LINK_SPDB_P3#"},
    {17, "2.1 LINK_SPDA_P3#"},
    {16, "2.0 ACT_P2#"},

    // Byte 3
    {31, "3.7 ACT_P7#"},
    {30, "3.6 LINK_SPDB_P7#"},
    {29, "3.5 LINK_SPDA_P7#"},
    {28, "3.4 ACT_P6#"},
    {27, "3.3 LINK_SPDB_P6#"},
    {26, "3.2 LINK_SPDA_P6#"},
    {25, "3.1 ACT_P5#"},
    {24, "3.0 LINK_SPDB_P5#"},
};

static char             outBfr[OUTBFR_SIZE];
uint8_t                 pinStates[PINS_COUNT] = {0};

// Prototypes
void timers_scanChainCapture(void);
void writePin(uint8_t pinNo, uint8_t value);
void readAllPins(void);

/**
  * @name   configureIOPins
  * @brief  configure all I/O pins
  * @param  None
  * @retval None
  */
void configureIOPins(void)
{
  pin_size_t        pinNo;

  for ( int i = 0; i < static_pin_count; i++ )
  {
      pinNo = staticPins[i].pinNo;
      pinMode(pinNo, staticPins[i].pinFunc);

      if ( staticPins[i].pinFunc == OUTPUT )
      {
          // increase drive strength on output pins
          // see ttf/variants.cpp for the data in g_APinDescription[]
          // NOTE: this will source 7mA, sink 10mA
          PORT->Group[g_APinDescription[pinNo].ulPort].PINCFG[g_APinDescription[pinNo].ulPin].bit.DRVSTR = 1;

          // deassert pin
          writePin(pinNo, (staticPins[i].activeState == ACT_LO) ? 1 : 0);
      }
  }
}

//===================================================================
//                    READ, WRITE COMMANDS
//===================================================================

/**
  * @name   readPin
  * @brief  wrapper to digitalRead via pinStates[]
  * @param  pinNo   Arduino pin # to read
  * @retval bool    pin state
  */
bool readPin(uint8_t pinNo)
{
    uint8_t         index = getPinIndex(pinNo);

    // if requested pin is an input, read that pin; else the
    // latest value written will be in pinStates[]
    if ( staticPins[index].pinFunc == INPUT )
        pinStates[index] = digitalRead((pin_size_t) pinNo);

    return(pinStates[index]);
}

/**
  * @name   writePin
  * @brief  wrapper to digitalWrite via pinStates[]
  * @param  pinNo = Arduino pin #
  * @param  value = value to write 0 or 1
  * @retval None
  */
void writePin(uint8_t pinNo, uint8_t value)
{
    value = (value == 0) ? 0 : 1;           // force value to boolean
    digitalWrite(pinNo, value);
    pinStates[getPinIndex(pinNo)] = value;
}

/**
  * @name   readCmd
  * @brief  read an I/O pin
  * @param  arg not used
  * @param  tokens[1] = Arduino pin #
  * @retval 0=OK 1=pin # not found
  * @note   displays pin info
  */
int readCmd(int arg)
{
    uint8_t       pinNo = atoi(tokens[1]);
    uint8_t       index = getPinIndex(pinNo);

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; cannot read an I/O pin");
        return(1);
    }

    if ( pinNo > PINS_COUNT )
    {
        terminalOut((char *) "Invalid pin number; please use Arduino numbering");
        return(1);
    }

    (void) readPin(pinNo);
    sprintf(outBfr, "%s Pin %d (%s) = %d", (staticPins[index].pinFunc == INPUT) ? "Input" : "Output", 
            pinNo, getPinName(pinNo), pinStates[index]);
    terminalOut(outBfr);
    return(0);
}

/**
  * @name   writeCmd
  * @brief  write a pin with 0 or 1
  * @param  arg 1 Arduino pin #
  * @param  arg 2 value to write
  * @retval None
  */
int writeCmd(int argCnt)
{
    uint8_t     pinNo = atoi(tokens[1]);
    uint8_t     value = atoi(tokens[2]);
    uint8_t     index = getPinIndex(pinNo);

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; cannot write an I/O pin");
        return(1);
    }

    if ( pinNo > PINS_COUNT || index == -1 )
    {
        terminalOut((char *) "Invalid pin number; use 'pins' command for help.");
        return(1);
    }    

    if ( staticPins[index].pinFunc == INPUT )
    {
        terminalOut((char *) "Cannot write to an input pin! Use 'pins' command for help.");
        return(1);
    }  

    if ( value != 0 && value != 1 )
    {
        terminalOut((char *) "Invalid pin value; please enter either 0 or 1");
        return(1);
    }

    writePin(pinNo, value);

    sprintf(outBfr, "Wrote %d to pin # %d (%s)", value, pinNo, getPinName(pinNo));
    terminalOut(outBfr);
    return(0);
}

/**
  * @name   
  * @brief  
  * @param  None
  * @retval 0
  */
char getPinChar(int index)
{
    if ( staticPins[index].pinFunc == INPUT )
        return('<');
    else if ( staticPins[index].pinFunc == OUTPUT )
        return('>');
    else
        return('=');
}

/**
  * @name   pinCmd
  * @brief  display I/O pins
  * @param  argCnt = CLI arg count
  * @retval None
  */
int pinCmd(int arg)
{
    int         count = static_pin_count;
    int         index = 0;

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; cannot display I/O pins");
        return(1);
    }

    terminalOut((char *) " ");
    terminalOut((char *) " #           Pin Name   D/S              #        Pin Name      D/S ");
    terminalOut((char *) "-------------------------------------------------------------------- ");

	readAllPins();
	
    while ( count > 0 )
    {
      if ( count == 1 )
      {
          sprintf(outBfr, "%2d %20s %c %d ", staticPins[index].pinNo, staticPins[index].name,
                  getPinChar(index), readPin(staticPins[index].pinNo));
          terminalOut(outBfr);
          break;
      }
      else
      {
          sprintf(outBfr, "%2d %20s %c %d\t\t%2d %20s %c %d ", 
                  staticPins[index].pinNo, staticPins[index].name, 
                  getPinChar(index), readPin(staticPins[index].pinNo),
                  staticPins[index+1].pinNo, staticPins[index+1].name, 
                  getPinChar(index+1), readPin(staticPins[index+1].pinNo));
          terminalOut(outBfr);
          count -= 2;
          index += 2;
      }
    }

    terminalOut((char *) "D/S = Direction/State; < input, > output");
    return(0);
}

//===================================================================
//                         Status Display Screen
//===================================================================

/**
  * @name   padBuffer
  * @brief  pad outBfr with spaces 
  * @param  pos = position to start padding at in 'outBfr'
  * @retval None
  */
 char *padBuffer(int pos)
 {
    int         leftLen = strlen(outBfr);
    int         padLen = pos - leftLen;
    char        *s;

    s = &outBfr[leftLen];
    memset((void *) s, ' ', padLen);
    s += padLen;
    return(s);
 }

/**
  * @name   readAllPins
  * @brief  read all I/O pins into pinStates[]
  * @param  None
  * @retval None
  */
void readAllPins(void)
{
    for ( int i = 0; i < static_pin_count; i++ )
    {
        (void) readPin(staticPins[i].pinNo);
    }
}

/**
  * @name   statusCmd
  * @brief  display status screen
  * @param  argCnt = number of CLI arguments
  * @retval None
  */
int statusCmd(int arg)
{
    uint16_t        count = EEPROMData.status_delay_secs;
    bool            oneShot = (count == 0) ? true : false;

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; cannot display status");
        return(1);
    }

    while ( 1 )
    {
        readAllPins();

        CLR_SCREEN();
        CURSOR(1, 29);
        displayLine((char *) "TTF Status Display");

        CURSOR(3,1);
        sprintf(outBfr, "TEMP WARN         %d", readPin(TEMP_WARN));
        displayLine(outBfr);

        CURSOR(3,57);
        sprintf(outBfr, "P1_LINK_A_N      %u", readPin(P1_LINKA_N));
        displayLine(outBfr);

        CURSOR(4,1);
        sprintf(outBfr, "TEMP CRIT         %u", readPin(TEMP_CRIT));
        displayLine(outBfr);

        CURSOR(4,56);
        sprintf(outBfr, "PRSNTB [3:0]   %u%u%u%u %s", readPin(OCP_PRSNTB3_N), readPin(OCP_PRSNTB2_N), 
                readPin(OCP_PRSNTB1_N), readPin(OCP_PRSNTB0_N), isCardPresent() ? "CARD" : "VOID");
        displayLine(outBfr);

        CURSOR(5,1);
        sprintf(outBfr, "FAN ON AUX        %u", readPin(FAN_ON_AUX));
        displayLine(outBfr);

        CURSOR(5,58);
        sprintf(outBfr, "ATX_PWR_OK      %u", readPin(ATX_PWR_OK));
        displayLine(outBfr);

        CURSOR(6,1);
        sprintf(outBfr, "SCAN_LD_N         %d", readPin(OCP_SCAN_LD_N));
        displayLine(outBfr);

        CURSOR(6,53);
        sprintf(outBfr, "SCAN VERS [1:0]     %u%u", readPin(SCAN_VER_1), readPin(SCAN_VER_0));
        displayLine(outBfr);

        CURSOR(7,1);
        sprintf(outBfr, "AUX_EN            %d", readPin(OCP_AUX_PWR_EN));
        displayLine(outBfr);      

        CURSOR(7,60);
        sprintf(outBfr, "PWRBRK_N      %d", readPin(OCP_PWRBRK_N));
        displayLine(outBfr);

        CURSOR(8,1);
        sprintf(outBfr, "MAIN_EN           %d", readPin(OCP_MAIN_PWR_EN));
        displayLine(outBfr);  

        CURSOR(8,62);
        sprintf(outBfr, "WAKE_N      %d", readPin(OCP_WAKE_N));
        displayLine(outBfr);

        CURSOR(9,1);
        sprintf(outBfr, "P3_LED_ACT_N      %d", readPin(P3_LED_ACT_N));
        displayLine(outBfr);  

        CURSOR(9,58);
        sprintf(outBfr, "P3_LINKA_N      %d", readPin(P3_LINKA_N));
        displayLine(outBfr);

        CURSOR(10,1);
        sprintf(outBfr, "P1_LED_ACT_N      %d", readPin(P1_LED_ACT_N));
        displayLine(outBfr);

        CURSOR(10, 58);
        sprintf(outBfr, "NCSI_RST_N      %d", readPin(NCSI_RST_N));
        displayLine(outBfr);

        if ( oneShot )
        {
            CURSOR(12,1);
            displayLine((char *) "Status delay 0, set sdelay to nonzero for this screen to loop.");
            return(0);
        }

        CURSOR(24, 22);
        displayLine((char *) "Hit any key to exit this display");

        while ( count-- > 0 )
        {
            if ( SerialUSB.available() )
            {
                // flush any user input and exit
                (void) SerialUSB.read();

                while ( SerialUSB.available() )
                {
                    (void) SerialUSB.read();
                }

                CLR_SCREEN();
                return(0);
            }

            delay(1000);
        }

        count = EEPROMData.status_delay_secs;
    }

    return(0);

} // statusCmd()


/**
  * @name   getPinName
  * @brief  get name of pin
  * @param  Arduino pin number
  * @retval pointer to name or 'unknown' if not found
  */
const char *getPinName(int pinNo)
{
    for ( int i = 0; i < static_pin_count; i++ )
    {
        if ( pinNo == staticPins[i].pinNo )
            return(staticPins[i].name);
    }

    return("Unknown");
}

/**
  * @name   getPinIndex
  * @brief  get index into static/dynamic pin arrays
  * @param  Arduino pin number
  * @retval None
  */
int8_t getPinIndex(uint8_t pinNo)
{
    for ( int i = 0; i < static_pin_count; i++ )
    {
        if ( staticPins[i].pinNo == pinNo )
            return(i);
    }

    return(-1);
}

/**
  * @name   set_help
  * @brief  help for set command
  * @param  None
  * @retval None
  */
void set_help(void)
{
    terminalOut((char *) "FLASH Parameters are:");
    sprintf(outBfr, "  sdelay <integer> - status display delay in seconds; current: %d", EEPROMData.status_delay_secs);
    terminalOut(outBfr);
    sprintf(outBfr, "  pdelay <integer> - power up sequence delay in milliseconds; current: %d", EEPROMData.pwr_seq_delay_msec);
    terminalOut(outBfr);
    terminalOut((char *) "'set <parameter> <value>' sets a parameter from list above to value");
    terminalOut((char *) "  value can be <integer>, <string> or <float> depending on the parameter");

    // TODO add more set command help here
}

/**
  * @name   setCmd
  * @brief  Set a parameter (seeing) in FLASH
  * @param  arg 1 = parameter name
  * @param  arg 2 = value to set
  * @retval 0
  * @note   no args shows help w/current values
  * @note   simulated EEPROM is called FLASH to the user
  */
int setCmd(int argCnt)
{
    char          *parameter = tokens[1];
    String        valueEntered = tokens[2];
//    float         fValue;
    int           iValue;
    bool          isDirty = false;

    if ( argCnt == 0 )
    {
        set_help();
        return(0);
    }
    else if ( argCnt != 2 )
    {
        set_help();
        return(0);
    }

    if ( strcmp(parameter, "sdelay") == 0 )
    {
        iValue = valueEntered.toInt();
        if (EEPROMData.status_delay_secs != iValue )
        {
          isDirty = true;
          EEPROMData.status_delay_secs = iValue;
        }
    }
    else if ( strcmp(parameter, "pdelay") == 0 )
    {
        iValue = valueEntered.toInt();
        if (EEPROMData.pwr_seq_delay_msec != iValue )
        {
          isDirty = true;
          EEPROMData.pwr_seq_delay_msec = iValue;
        }
    }
    else
    {
        terminalOut((char *) "Invalid parameter name");
        set_help();
        return(1);
    }

    if ( isDirty == true )
        EEPROM_Save();

    return(0);

} // setCmd()

/**
  * @name   queryScanChain
  * @brief  extract info from scan chain output
  * @param  displayResults  true to display results, else false
  * @retval uint32_t    32 bits of scan chain data received
  */
uint32_t queryScanChain(bool displayResults)
{
    uint8_t             shift = 31;
    char                *s = outBfr;
    const char          fmt[] = "%-20s ... %d    ";
    unsigned            i = 0;

    timers_scanChainCapture();

    if ( displayResults == false )
        return(scanShiftRegister_0);

    sprintf(outBfr, "scan chain shift register 0: %08X", (unsigned int) scanShiftRegister_0);
    terminalOut(outBfr);

    // WARNING: This code below expects the entries in scanBitNames[] to be in order order 31..0
    // to align with incoming shifted left bits from SCAN_DATA_IN
    while ( i < 32 )
    {
        sprintf(s, fmt, scanBitNames[i++].bitName, (scanShiftRegister_0 & (1 << shift--)) ? 1 : 0);
        s += 30;
        sprintf(s, fmt, scanBitNames[i++].bitName, (scanShiftRegister_0 & (1 << shift--)) ? 1 : 0);
        s += 30;
        *s = 0;

        terminalOut(outBfr);
        s = outBfr;
    }

    return(scanShiftRegister_0);
    
} // queryScanChain()

/**
  * @name   pwrCmdHelp
  * @brief  display help for the power command
  * @param  None
  * @retval None
  */
static void pwrCmdHelp(void)
{
    terminalOut((char *) "Usage: power <up | down | status> <main | aux | card>");
    terminalOut((char *) "  'power status' requires no argument and shows the power status of NIC card");
    terminalOut((char *) "  main = MAIN_EN to NIC card; aux = AUX_EN to NIC card; ");
    terminalOut((char *) "  card = MAIN_EN=1 then pdelay msecs then AUX_EN=1; see 'set' command for pdelay");
}

/**
  * @name   pwrCmd
  * @brief  Control AUX and MAIN power to NIC 3.0 board
  * @param  argCnt  number of arguments
  * @param  tokens[1]  up, down or status
  * @param  tokens[2]   main, aux or card
  * @retval 0   OK
  * @retval 1   error
  * @note   Delay is changed with 'set pdelay <msec>'
  * @note   power <up|down|status> <main|aux|board> status has no 2nd arg
  */
int pwrCmd(int argCnt)
{
    int             rc = 0;
    bool            isPowered = false;
    uint8_t         mainPin = readPin(OCP_MAIN_PWR_EN);
    uint8_t         auxPin = readPin(OCP_AUX_PWR_EN);
    uint8_t         pwrGoodPin = readPin(NIC_PWR_GOOD_JMP);

    if ( argCnt == 0 )
    {
        pwrCmdHelp();
        return(1);
    }

    if ( isCardPresent() == false )
    {
        terminalOut((char *) "NIC card is not present; no power info available");
        return(1);
    }

    if (  mainPin == 1 &&  auxPin == 1 && pwrGoodPin )
        isPowered = true;

    if ( argCnt == 1 )
    {
        if ( strcmp(tokens[1], "status") == 0 )
        {
            sprintf(outBfr, "Status: NIC card is powered %s", (isPowered) ? "up" : "down");
            SHOW();
            return(rc);
        }
        else
        {
            terminalOut((char *) "Incorrect number of command arguments");
            pwrCmdHelp();
            return(1);
        }
    }
    else if ( argCnt != 2 )
    {
        terminalOut((char *) "Incorrect number of command arguments");
        pwrCmdHelp();
        return(1);
    }

    if ( strcmp(tokens[1], "up") == 0 )
    {
        if ( strcmp(tokens[2], "card") == 0 )
        {
            if ( isPowered == false )
            {
                sprintf(outBfr, "Starting NIC power up sequence, delay = %d msec", EEPROMData.pwr_seq_delay_msec);
                SHOW();
                writePin(OCP_MAIN_PWR_EN, 1);
                delay(EEPROMData.pwr_seq_delay_msec);
                writePin(OCP_AUX_PWR_EN, 1);

                // NIC card takes a bit of time to power up
                delay(50);
                if ( readPin(NIC_PWR_GOOD_JMP) )
                    terminalOut((char *) "Power up sequence complete");
                else
                {
                    terminalOut((char *) "Power up sequence failed; NIC_PWR_GOOD = 0");
                    return(1);
                }

                terminalOut((char *) "Waiting for scan chain data...");
                delay(2000);
                queryScanChain(false);
                queryScanChain(true);

            }
            else
            {
                terminalOut((char *) "Power is already up on NIC card");
            }
        }
        else if ( strcmp(tokens[2], "main") == 0 )
        {
            if ( mainPin == 1 )
            {
                terminalOut((char *) "MAIN_EN is already 1");
                return(0);
            }
            else
            {
                writePin(OCP_MAIN_PWR_EN, 1);
                terminalOut((char *) "Set MAIN_EN to 1");
                return(0);
            }
        }
        else if ( strcmp(tokens[2], "aux") == 0 )
        {
            if ( auxPin == 1 )
            {
                terminalOut((char *) "AUX_EN is already 1");
                return(0);
            }
            else
            {
                writePin(OCP_AUX_PWR_EN, 1);
                terminalOut((char *) "Set AUX_EN to 1");
                return(0);
            }
        }
        else
        {
            terminalOut((char *) "Invalid argument");
            pwrCmdHelp();
            return(1);
        }
    }
    else if ( strcmp(tokens[1], "down") == 0 )
    {
        if ( strcmp(tokens[2], "card") == 0 )
        {
            if ( isPowered == true )
            {
                writePin(OCP_MAIN_PWR_EN, 0);
                writePin(OCP_AUX_PWR_EN, 0);
                delay(100);
                if ( readPin(NIC_PWR_GOOD_JMP) == 0 )
                    terminalOut((char *) "Power down sequence complete");
                else
                    terminalOut((char *) "Power down failed; NIC_PWR_GOOD = 1");
            }
            else
            {
                terminalOut((char *) "Power is already down on NIC card");
            }
        }
        else if ( strcmp(tokens[2], "main") == 0 )
        {
            if ( mainPin == 0 )
            {
                terminalOut((char *) "MAIN_PWR_EN is already 0");
                return(0);
            }
            else
            {
                writePin(OCP_MAIN_PWR_EN, 0);
                terminalOut((char *) "Set MAIN_PWR_EN to 0");
                return(0);
            }
        }
        else if ( strcmp(tokens[2], "aux") == 0 )
        {
            if ( auxPin == 0 )
            {
                terminalOut((char *) "AUX_PWR_EN is already 0");
                return(0);
            }
            else
            {
                writePin(OCP_AUX_PWR_EN, 0);
                terminalOut((char *) "Set AUX_PWR_EN to 0");
                return(0);
            }
        }
        else
        {
            terminalOut((char *) "Invalid argument");
            pwrCmdHelp();
            return(1);
        }
    }
    else
    {
        terminalOut((char *) "Invalid subcommand: use 'up', 'down' or 'status'");
        rc = 1;
    }

    return(rc);
}

/**
  * @name   versCmd
  * @brief  Displays firmware version
  * @param  arg not used
  * @retval None
  */
int versCmd(int arg)
{
    sprintf(outBfr, "Firmware version %s built on %s at %s", VERSION_ID, BUILD_DATE, BUILD_TIME);
    terminalOut(outBfr);
    return(0);
}

/**
  * @name   isCardPresent
  * @brief  Determine if NIC card is present
  * @param  None
  * @retval true if card present, else false
  */
bool isCardPresent(void)
{
    uint8_t         present = readPin(OCP_PRSNTB0_N);

    present |= (readPin(OCP_PRSNTB1_N) << 1);
    present |= (readPin(OCP_PRSNTB2_N) << 2);
    present |= (readPin(OCP_PRSNTB3_N) << 3);

    if ( present == 0xF )
        return(false);
    
    return(true);
}


/**
  * @name   scanCmd
  * @brief  implement scan command
  * @param  argCnt  not used
  * @retval int 0=OK, 1=error
  */
int scanCmd(int argCnt)
{
    if ( isCardPresent() )
    {
        queryScanChain(true);
    }
    else
    {
        terminalOut((char *) "NIC card is not present; cannot query scan chain");
    }

    return(0);
}