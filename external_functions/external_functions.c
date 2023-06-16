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

// Microcontroller console capturing the last 1024 characters received on the uart so
// they can be shown on a web page

// Buffer to hold concole contents.
// Invariants:
// - *pconsole_rd==*pconsole_wr <=> buffer empty
// - **pconsole_rd == next char to read
// - **pconsole_wr == next char to write
// - 0 <= console_xx < BUF_MAX
// - (*pconsole_wr+1)%BUF_MAX) == *pconsole_rd <=> buffer full
#define BUF_MAX (1024)
// static char *pconsole_buf[BUF_MAX];
// static int *pconsole_wr, *pconsole_rd;
// static int *pconsole_pos; // offset since reset of buffer
extern int *pconsole_wr;
extern int *pconsole_rd;
extern int *pconsole_pos;
extern char *pconsole_buf;

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

// Buffer to hold concole contents.
// Invariants:
// - *pconsole_rd==*pconsole_wr <=> buffer empty
// - **pconsole_rd == next char to read
// - **pconsole_wr == next char to write
// - 0 <= console_xx < BUF_MAX
// - (*pconsole_wr+1)%BUF_MAX) == *pconsole_rd <=> buffer full
//static int *pconsole_wr, *pconsole_rd;
//static int *pconsole_pos; // offset since reset of buffer
int ICACHE_FLASH_ATTR
ajaxIndexHTML(HttpdConnData *connData) {

  // static char oldweight[8];
  // static bool oldstatus = false;
  char weight[8];
  bool status;
  int power;
  char units[2];
  char buff[1024];
  char rdbuff[40];
//  char wstr[8];
  int len; // length of text in buff
  char c;
  int buff_pos = 0;
  int rd = *pconsole_wr;
  bool found;

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.
  jsonHeader(connData, 200);
  len = os_sprintf(buff, "{"); 
// get temporary data in rdbuff
  found = false;
  buff_pos=0;
  while (buff_pos<sizeof(rdbuff)) {
    // len += os_sprintf(buff+len, " rd: %d", rd);
    c = pconsole_buf[rd--];
    // len += os_sprintf(buff+len, " c: \\u%02x", c);
    if (rd < 0) rd = BUF_MAX - 1;
    if (!found && (c == '\r')) {
      found = true;
      // len += os_sprintf(buff+len, " found! ");
    }
    else {
    // between fisrt and second \n
      if (found) {
        if (c == '\r' || c == '\n') {
          rdbuff[buff_pos]=0; // terminate string
          break;  // exit on second \n
        }
        rdbuff[buff_pos++]=c;
        // len += os_sprintf(buff+len, " bufpos: %d, c: \\u%02x", buff_pos, c);
#if 0
        if (c == '\\' || c == '"') {
          buff[len++] = '\\';
          buff[len++] = c;
        } else if (c == '\r') {
          // this is crummy, but browsers display a newline for \r\n sequences
        } else if (c < ' ') {
          len += os_sprintf(buff+len, "\\u%02x", c);
        } else {
          buff[len++] = c;
        }
#endif
      }
    }
  }
// trim trailing \n
 strrev(rdbuff);
  // len += os_sprintf(buff+len, "\"rdbuff len 3\": %d, ", strlen(rdbuff));
  // len += os_sprintf(buff+len, "\"rdbuff\": \"%s\", \"bufpos\": %d, \"found\": %d, ", rdbuff, buff_pos, found);
// parse string
// ST,NT,     0.0 oz
  if (found) {
    status = (rdbuff[0] == 'S' && rdbuff[1] == 'T');
    strncpy(weight,rdbuff+7,7); 
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
    // now = (realtime_stamp == 0) ? (se->tick / 1000000) : realtime_stamp;
    now = realtime_stamp;
    tp = gmtime(&now);

    os_sprintf(rdbuff, "%4d-%02d-%02dT%02d:%02d:%02d",
        tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
        tp->tm_hour, tp->tm_min, tp->tm_sec);
    len += os_sprintf(buff+len, "\"final_weight\": %s, \"weight\": %s, \"timestamp\" : \"%s\", \"power\": \"%d\", \"status\" : \"%d\", \"unit\" : \"%s\"", 
      weight, weight, rdbuff, power, status, units);
  }
  os_strcpy(buff+len, " }"); len+=2;
  httpdSend(connData, buff, len);
  return HTTPD_CGI_DONE;
}

