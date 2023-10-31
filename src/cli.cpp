//===================================================================
// cli.cpp 
// Contains Command Line Interpreter (CLI) and related terminal
// functions.
//===================================================================
#include <Arduino.h>
#include "main.hpp"
#include "cli.hpp"
#include "commands.hpp"

extern uint8_t  boardIDReal;

// Constant Data
const char      cliPrompt[] = "ttf> ";
const int       promptLen = sizeof(cliPrompt);
const char      hello[] = "OCP NIC 3.0 Thermal Test Fixture V";

// CLI token stack and formatting buffer
char            *tokens[MAX_TOKENS];
static char     outBfr[OUTBFR_SIZE];

// CLI Command Table structure
typedef struct {
    char        cmd[CMD_NAME_MAX];
    int         (*func) (int x);
    int         argCount;
    char        help1[MAX_LINE_SZ];
    char        help2[MAX_LINE_SZ];

} cli_entry;

// command functions
int curCmd(int);
int writeCmd(int arg);
int readCmd(int arg);
int setCmd(int arg);
int pinCmd(int arg);
int debug(int arg);
int statusCmd(int arg);
int eepromCmd(int arg);
int pwrCmd(int arg);
int versCmd(int arg);
int scanCmd(int arg);

// CLI command table
// CLI_COMMAND_CNT is defined in cli.hpp
// format is "command", function, required arg count, "help line 1", "help line 2" 
// NOTE: -1 as arg count means "don't check number of arguments, cmd function will"
// NOTE: " " (space) on 2nd line of help doesn't display anything (for short helps)
// NOTE: These are in alphabetical order for presentation (except help) FYI...
cli_entry     cmdTable[CLI_COMMAND_CNT] = {
    {"eeprom", eepromCmd,  -1, "'eeprom show' displays FRU EEPROM info areas.",  "'eeprom dump <addr> <length>' dumps <length> bytes @ <addr>"},
    {"pins",      pinCmd,   0, "Displays pin names and numbers.",                "TTF uses Arduino-style pin numbering shown in this display."},
    {"power",     pwrCmd,  -1, "Control power to NIC 3.0 card.",                 "'power <up|down> <main|aux|card>' or 'power status' "},
    {"read",     readCmd,   1, "Read input pin (Arduino numbering).",            "'read <pin_number>'"},
    {"set",       setCmd,  -1, "Set FLASH parameter to a value.",                "'set <param> <value>' sets value; or 'set' with no args for help."},
    {"scan",     scanCmd,   0, "Scan chain query of NIC 3.0 card.",              " "},
    {"status", statusCmd,   0, "Displays status of I/O pins etc.",               " "},
    {"vers",     versCmd,   0, "Shows firmware version information.",            " "},
    {"write",   writeCmd,   2, "Write output pin (Arduino numbering).",          "'write <pin_number> <0|1>'"},
    {"xdebug",     debug,  -1, "Debug functions mostly for developer use.",      "Enter 'xdebug' with no arguments for more info."},
    {"help",        help,   0, "NOTE: THIS DOES NOT DISPLAY ON PURPOSE",         " "},    
};

/**
  * @name   CURSOR
  * @brief  set terminal cursor
  * @param  r row
  * @param  c column
  * @retval None
  * @note   no error checking for out of bounds params
  */
void CURSOR(uint8_t r,uint8_t c)                 
{
    char          bfr[12];

    sprintf(bfr, "\x1b[%d;%df", r, c);
    SerialUSB.write(bfr);
    SerialUSB.flush();
    delay(5);
}

/**
  * @name   terminalOut
  * @brief  wrapper to SerialUSB.print[ln]
  * @param  msg to output
  * @retval None
  * @note   needed to address missing chars
  */
void terminalOut(char *msg)
{
    SerialUSB.println(msg);
    SerialUSB.flush();
    delay(50);
}

/**
  * @name   displayLine
  * @brief  wrapper to SerialUSB.write
  * @param  None
  * @retval None
  */
void displayLine(char *m)
{
    SerialUSB.write(m);
    SerialUSB.flush();
    delay(10);
}

/**
  * @name   doPrompt
  * @brief  output firmware prompt to terminal
  * @param  None
  * @retval None
  */
void doPrompt(void)
{
    SerialUSB.write(0x0a);
    SerialUSB.write(0x0d);
    SerialUSB.flush();
    SerialUSB.print(cliPrompt);
    SerialUSB.flush();
}

/**
  * @name   doHello
  * @brief  Output welcome message to terminal
  * @param  None
  * @retval None
  */
void doHello(void)
{
    sprintf(outBfr, "\r\n%s%s Board Rev X%02d", hello, VERSION_ID, boardIDReal);
    terminalOut(outBfr);
}

/**
  * @name   waitAnyKey
  * @brief  wait for any keyboard hit 
  * @param  None
  * @retval None
  * @note   WARNING! Blocking call!
  */
int waitAnyKey(void)
{
    int             charIn;

    while ( SerialUSB.available() == 0 )
      ;

    charIn = SerialUSB.read();
    return(charIn);
}

/**
  * @name   cli
  * @brief  command line interpreter
  * @param  raw = raw input line from terminal
  * @retval None
  */
bool cli(char *raw)
{
    bool         rc = false;
    int         len;
    char        *token;
    const char  delim[] = " ";
    int         tokNdx = 0;
    char        input[80];
    int         error = CLI_ERR_CMD_NOT_FOUND;
    int         argCount;

    strcpy(input, (char *) raw);
    len = strlen(input);

    // initial call, should get and save the command as 0th token
    token = strtok(input, delim);
    if ( token == NULL )
    {
      doPrompt();
      return(true);
    }

    tokens[tokNdx++] = token;

    // subsequent calls should parse any space-separated args into tokens[]
    // note that what's stored in tokens[] are string pointers into the
    // input array not the actual string token itself
    while ( (token = (char *) strtok(NULL, delim)) != NULL && tokNdx < MAX_TOKENS )
    {
        tokens[tokNdx++] = token;
    }

    if ( tokNdx >= MAX_TOKENS )
    {
        terminalOut((char *) "Too many arguments in command line!");
        doPrompt();
        return(false);
    }

    // adjust arg count to not include the command itself (token[0]
    argCount = tokNdx - 1;

    for ( int i = 0; i < (int) CLI_COMMAND_CNT; i++ )
    {
        if ( strncmp(tokens[0], cmdTable[i].cmd, len) == 0 )
        {
            if ( (cmdTable[i].argCount == argCount) || (cmdTable[i].argCount == -1) )
            {
                // command funcs are passed arg count, tokens are global
                (cmdTable[i].func) (argCount);
                SerialUSB.flush();
                rc = true;
                error = CLI_ERR_NO_ERROR;
                break;
            }
            else if ( cmdTable[i].argCount > argCount )
            {
                error = CLI_ERR_TOO_FEW_ARGS;
                rc = false;
                break;
            }
            else
            {
                error = CLI_ERR_TOO_MANY_ARGS;
                rc = false;
                break;
            }
        }
    }

    if ( rc == false )
    {
        if ( error == CLI_ERR_CMD_NOT_FOUND )
         terminalOut((char *) "Invalid command");
        else if ( error == CLI_ERR_TOO_FEW_ARGS )
          terminalOut((char *) "Not enough arguments for this command, check help.");
        else if ( error == CLI_ERR_TOO_MANY_ARGS )
          terminalOut((char *) "Too many arguments for this command, check help.");
        else
          terminalOut((char *) "Unknown parser s/w error");
    }

    doPrompt();
    return(rc);

} // cli()

/**
  * @name   help
  * @brief  CLI help feature
  * @param  None
  * @retval None
  */
int help(int arg)
{
    doHello();
    terminalOut((char *) "Enter a command then press ENTER. Some commands require arguments, which must");
    terminalOut((char *) "be separated from the command and other arguments by a space.");
    terminalOut((char *) "Up arrow repeats the last command; backspace or delete erases the last");
    terminalOut((char *) "character entered. Commands available are:");
    terminalOut((char *) " ");

    for ( int i = 0; i < (int) CLI_COMMAND_CNT; i++ )
    {
      if ( strcmp(cmdTable[i].cmd, "help") == 0 )
        continue;

      sprintf(outBfr, "%s\t%s", cmdTable[i].cmd, cmdTable[i].help1);
      terminalOut(outBfr);

      if ( strcmp(cmdTable[i].help2, " ") != 0 )
      {
        sprintf(outBfr, "\t%s", cmdTable[i].help2);
        terminalOut(outBfr);
      }
    }

    return(0);
}

/**
  * @name   showCommandHelp
  * @brief  show help for given command
  * @param  cmd command to search for
  * @retval None
  */
void showCommandHelp(char *cmd)
{
    for ( int i = 0; i < (int) CLI_COMMAND_CNT; i++ )
    {
      if ( strcmp(cmdTable[i].cmd, "help") == 0 )
        continue;

      if ( strcmp(cmd, cmdTable[i].cmd) == 0 )
      {
        terminalOut(cmdTable[i].help1);

        if ( cmdTable[i].help2[0] != ' ' )
        {
          terminalOut(cmdTable[i].help2);
        }          
      }
    }
}
