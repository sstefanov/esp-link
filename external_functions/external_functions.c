// Copy from console.c

#include <esp8266.h>
#include "uart.h"
#include "cgi.h"
#include "uart.h"
#include "serbridge.h"
#include "serled.h"
#include "config.h"
#include "console.h"
#include <string.h>  
#include <sntp.h>  
#include <time.h>

#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW()   REG_READ(0x3ff20c00)

#define BUF_MAX (1024)
extern int *pconsole_wr;
extern int *pconsole_rd;
extern int *pconsole_pos;
extern char *pconsole_buf;

// remove spaces from string
void trimspaces (char* s) {
    char* p = s;
    do {
        if (*s!=' ')
            *p++=*s;
    }
    while (*s++);
}

char ICACHE_FLASH_ATTR *strrev(char *str) {
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

int ICACHE_FLASH_ATTR
ajaxWeight(HttpdConnData *connData) {
  char weight[10];
  bool status;
  int power;
  char units[4];
  char buff[1024];
  char rdbuff[40];
  int len; // length of text in buff
  char c;
  int buff_pos = 0;
  int rd = *pconsole_wr;
  bool found;

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.
  //jsonHeader(connData, 200); // not needed, program is using plain text
  len = os_sprintf(buff, "{"); 
  found = false;
  buff_pos=0;
  while (buff_pos<sizeof(rdbuff)) {
    c = pconsole_buf[rd--];
    if (rd < 0) rd = BUF_MAX - 1;
    if (!found && (c == '\r')) {
      found = true;
    }
    else {
    // between fisrt and second \n
      if (found) {
        if (c == '\r' || c == '\n') {
          rdbuff[buff_pos]=0; // terminate string
          break;  // exit on second \n
        }
        rdbuff[buff_pos++]=c;
      }
    }
  }
 strrev(rdbuff);
// parse string
// ST,NT,     0.0 oz
  if (found) {
    status = (rdbuff[0] == 'S' && rdbuff[1] == 'T');
    strncpy(weight,rdbuff+6,8); 
    strncpy(units,rdbuff+15,2);
    if (units == "kg") {
      power = 2;
    } 
    else {
      if (units == "g ") {
        power = -3;
      }
      else {
        power = 0;
      }
    }
  // timestamp
    time_t now = NULL;
    struct tm *tp = NULL;
    // create timestamp: FULL-DATE "T" PARTIAL-TIME "Z": 'YYYY-mm-ddTHH:MM:SSZ '
    // as long as realtime_stamp is 0 we use tick div 10⁶ as date
    uint32_t realtime_stamp = sntp_get_current_timestamp();
    if (realtime_stamp == 0) 
       realtime_stamp = WDEV_NOW() / 1000000;
    // now = (realtime_stamp == 0) ? (se->tick / 1000000) : realtime_stamp;
    now = realtime_stamp;
    tp = gmtime(&now);

    os_sprintf(rdbuff, "%4d-%02d-%02dT%02d:%02d:%02d",
        tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
        tp->tm_hour, tp->tm_min, tp->tm_sec);

    trimspaces(weight);
    trimspaces(units);

    len += os_sprintf(buff+len, "\"final_weight\": %s, \"weight\": %s, \"timestamp\" : \"%s\", \"power\": \"%d\", \"status\" : \"%d\", \"unit\" : \"%s\"", 
      weight, weight, rdbuff, power, status, units);
  }
  os_strcpy(buff+len, " }"); len+=2;
  httpdSend(connData, buff, len);
  return HTTPD_CGI_DONE;
}

// check connection and scale status
int ICACHE_FLASH_ATTR
ajaxStatus(HttpdConnData *connData) {
  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  return HTTPD_CGI_DONE;
}
