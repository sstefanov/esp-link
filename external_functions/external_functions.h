#ifndef EXTERNAL_FUNC
#define EXTERNAL_FUNC

#include "console.h"
//#pragma message ("External functions header")
#define EXTINDEXHTML
// process /index.html
int ajaxWeight(HttpdConnData *connData);
int ajaxStatus(HttpdConnData *connData);

#endif
