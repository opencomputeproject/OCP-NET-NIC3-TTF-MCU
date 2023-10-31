#ifndef _CLI_H_
#define _CLI_H_

#include <Arduino.h>
#include "main.hpp"

// update CLI_COMMAND_CNT if adding new commands to table in cli.cpp
#define CLI_COMMAND_CNT           11

#define CMD_NAME_MAX              12

// possible CLI errors
#define CLI_ERR_NO_ERROR          0
#define CLI_ERR_CMD_NOT_FOUND     1
#define CLI_ERR_TOO_FEW_ARGS      2
#define CLI_ERR_TOO_MANY_ARGS     3
#define MAX_TOKENS                8

void CURSOR(uint8_t r,uint8_t c);
void terminalOut(char *msg);
void displayLine(char *m);
void doPrompt(void);
void doHello(void);
int waitAnyKey(void);
bool cli(char *raw);
int help(int);
void showCommandHelp(char *cmd);

#endif // _CLI_H_
