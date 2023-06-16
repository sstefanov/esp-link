#ifndef CONSOLE_H
#define CONSOLE_H

#include "httpd.h"

// #define BUF_MAX (1024)
// static char console_buf[BUF_MAX];
// static int console_wr, console_rd;
// static int console_pos; // offset since reset of buffer

void consoleInit(void);
void ICACHE_FLASH_ATTR console_write_char(char c);
int ajaxConsole(HttpdConnData *connData);
int ajaxConsoleReset(HttpdConnData *connData);
int ajaxConsoleClear(HttpdConnData *connData);
int ajaxConsoleBaud(HttpdConnData *connData);
int ajaxConsoleFormat(HttpdConnData *connData);
int ajaxConsoleSend(HttpdConnData *connData);
int tplConsole(HttpdConnData *connData, char *token, void **arg);

#endif
