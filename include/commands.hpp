#ifndef _COMMANDS_H_
#define _COMMANDS_H_
//===================================================================
// commands.hpp 
// Definitions for project commands (see commands.cpp).
//===================================================================
#include <stdint-gcc.h>

void monitorsInit(void);
const char *getPinName(int pinNo);
int8_t getPinIndex(uint8_t pinNo);
int statusCmd(int arg);
char *padBuffer(int pos);
void configureIOPins(void);
void readAllPins(void);
bool readPin(uint8_t pinNo);
void writePin(uint8_t pinNo, uint8_t value);
bool isCardPresent(void);

#endif // _COMMANDS_H_