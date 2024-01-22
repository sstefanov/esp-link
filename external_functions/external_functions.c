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
// round to 0.05
#define _ROUND 50

extern int *pconsole_wr;
extern int *pconsole_rd;
extern int *pconsole_pos;
extern char *pconsole_buf;


// remove spaces from string
void trimspaces (char* s) {
    char* p = s;
    do {
      if (*s <= ' ') continue;
      *p++=*s;
    }
    while (*s++);
    *p = 0; // null termination
}

double ICACHE_FLASH_ATTR  _atof(const char *s)
{
  // This function stolen from either Rolf Neugebauer or Andrew Tolmach. 
  // Probably Rolf.
  double a = 0.0;
  int e = 0;
  int c;
  while ((c = *s++) != '\0' && isdigit(c)) {
    a = a*10.0 + (c - '0');
  }
  if (c == '.') {
    while ((c = *s++) != '\0' && isdigit(c)) {
      a = a*10.0 + (c - '0');
      e = e-1;
    }
  }
  if (c == 'e' || c == 'E') {
    int sign = 1;
    int i = 0;
    c = *s++;
    if (c == '+')
      c = *s++;
    else if (c == '-') {
      c = *s++;
      sign = -1;
    }
    while (isdigit(c)) {
      i = i*10 + (c - '0');
      c = *s++;
    }
    e += i*sign;
  }
  while (e > 0) {
    a *= 10.0;
    e--;
  }
  while (e < 0) {
    a *= 0.1;
    e++;
  }
  return a;
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
// #define WDBG

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
#ifdef WDBG
  char prbuff[256];
  int prlen;
  os_printf("weight\n");
#endif
  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.
  //jsonHeader(connData, 200); // not needed, program is using plain text
  rdbuff[buff_pos] = 0;
  len = os_sprintf(buff, "{");
  found = false;
  buff_pos=0;
#ifdef WDBG
  prlen = os_sprintf(prbuff, "p_wr=%d, p_rd=%d\n", *pconsole_wr, *pconsole_rd);
  os_printf(prbuff,prlen);
#endif
  int rdcount = 0; // read 80 bytes max
  // while (rd > 0 && buff_pos<sizeof(rdbuff)) {
  while (rdcount < sizeof(rdbuff) * 2 + 2 && buff_pos < sizeof(rdbuff)) {
      c = pconsole_buf[rd];
#ifdef WDBG
      prlen = os_sprintf(prbuff, "rdcount=%d, rd=%d, p_rd=%d, p_wr=%d, buff_pos=%d, c=%c 0x%02x \n", rdcount, rd, *pconsole_rd, *pconsole_wr, buff_pos, c, c);
      os_printf(prbuff, prlen);
#endif
      rdcount++;
      rd--;
      if (rd < 0)
          rd = BUF_MAX - 1;
      if (!found && (c == '\r')) { // || c == '\n'
          found = true;
#ifdef WDBG
          os_printf("\nFound first!\n");
#endif

      } else {
        // between fisrt and second \n
        if (found) {
            if (c == '\r' || c == '\n') {
                rdbuff[buff_pos] = 0; // terminate string
                break;                // exit on second \n
            }
            rdbuff[buff_pos++] = c;
        }
    }
  }
#ifdef WDBG
  prlen = os_sprintf(prbuff, "rdbuff=%s\n", rdbuff);
  os_printf(prbuff, prlen);
#endif
  // if (buff_pos == 0) {
// none read
  // }
  // len += os_sprintf(buff + len, "\"rdbuff\":\"%s\", ", rdbuff);
  // parse string
  // ST,NT,     0.0 oz
  if (found) {
    strrev(rdbuff);
    memset(weight, 0, sizeof(weight));
    memset(units, 0, sizeof(units));
#ifdef WDBG
    prlen = os_sprintf(prbuff, "rdbuff 2 =%s\n", rdbuff);
    os_printf(prbuff, prlen);
#endif
    status = (rdbuff[0] == 'S' && rdbuff[1] == 'T');
    strncpy(weight, rdbuff + 6, 8);
#ifdef WDBG
    prlen = os_sprintf(prbuff, "weight=%s\n", weight);
    os_printf(prbuff, prlen);
#endif
    strncpy(units, rdbuff + 15, 2);
#ifdef WDBG
    prlen = os_sprintf(prbuff, "units=%s\n", units);
    os_printf(prbuff, prlen);
#endif
    trimspaces(weight);
    trimspaces(units);
#ifdef WDBG
    prlen = os_sprintf(prbuff, "status=%d, weight=%s, units=%s\n", status ? 1 : 0, weight, units);
    os_printf(prbuff, prlen);
#endif
    int sign = 1;
    if (weight[0] == '-') {
      sign = -1;
      weight[0] = '0';
    } 
    double f = _atof(weight);
    if (f < 0) f = -f;
    int intPart = f * 1000;
#ifdef WDBG
    prlen = os_sprintf(prbuff, "sign=%d intPart = %d\n", sign, intPart);
    os_printf(prbuff, prlen);
#endif

    if (os_strcmp(units, "kg") == 0) {
      power = 2;
#ifdef _ROUND
// round to 50 g
      if (intPart % _ROUND  >= _ROUND/2 )
        intPart = intPart + _ROUND - intPart % _ROUND;
      else
        intPart = intPart - intPart % _ROUND;
  //      len += os_sprintf(buff+len, "\"IntPart1\": %d", intPart);
#ifdef WDBG
      prlen = os_sprintf(prbuff, "intPart round = %d\n", intPart);
      os_printf(prbuff, prlen);
#endif
#endif
    } 
    else {
      if (os_strcmp(units,"g ") == 0) {
        power = -3;
      }
      else {
        power = 0;
      }
    }

//    len += os_sprintf(buff+len, "\"IntPart2\": %d, ", intPart);
    // f = intPart;
    // os_sprintf doesn't support %f
    int fracPart = intPart % 1000;
    intPart = sign * intPart / 1000;
#ifdef WDBG
    prlen = os_sprintf(prbuff, "1 intPart = %d, fracPart = %d\n", intPart, fracPart);
    os_printf(prbuff, prlen);
#endif
    fracPart = fracPart / 10; // use 2 digit precision
                              //    lent+art = %d, fracPar+len%d"\"I intPa3\":fracPart);
    //    len += os_sprintf(buff+len, "\"fracPart\": %d, ", fracPart);
    if( fracPart < 0 ) // for negative numbers
      fracPart = -fracPart;
//    os_sprintf(weight, "%d.%02d", intPart, fracPart);
#ifdef WDBG
        prlen = os_sprintf(prbuff, "2 intPart = %d, fracPart = %d\n", intPart, fracPart);
        os_printf(prbuff, prlen);
#endif

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

        len += os_sprintf(buff + len, "\"final_weight\": %d.%02d, \"weight\": %d.%02d, \"timestamp\" : \"%s\", \"power\": \"%d\", \"status\" : \"%d\", \"unit\" : \"%s\"",
                          intPart, fracPart, intPart, fracPart, rdbuff, power, status, units);
    }
    os_strcpy(buff + len, " }");
    len += 2;
    httpdSend(connData, buff, len);
    return HTTPD_CGI_DONE;
}

// check connection and scale status
int ICACHE_FLASH_ATTR
ajaxStatus(HttpdConnData *connData) {
  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  return HTTPD_CGI_DONE;
}
