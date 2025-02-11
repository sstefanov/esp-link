/*
SSDPBoth renamed to ssdp
*/
/*
ESP8266 Simple Service Discovery STA and AP
Copyright (c) Patrick Briot, May 29th, 2019.
 
Original (ESP8266) version by Hristo Gochkov, 2015.

Original (Arduino) version by Filippo Sallemi, July 23, 2014.
Can be found at: https://github.com/nomadnt/uSSDP

License (MIT license):
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#ifndef SSDPBOTH_H
#define SSDPBOTH_H

#include <ESP8266WiFi.h>

//#define DEBUG_SERIAL                                     // uncomment for Serial debugging statements

#ifdef DEBUG_SERIAL
#define DEBUG_BEGIN(x)              Serial.begin(x)
#define DEBUG_PRINT(x)              Serial.print(x)
#define DEBUG_PRINTLN(x)            Serial.println(x)
#define DEBUG_PRINTF(fmt,...)       Serial.printf((PGM_P)PSTR(fmt), ## __VA_ARGS__)
#else
#define DEBUG_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt,...)
#endif

#define SSDP_UUID_SIZE              37
#define SSDP_SCHEMA_URL_SIZE        64
#define SSDP_DEVICE_TYPE_SIZE       64
#define SSDP_FRIENDLY_NAME_SIZE     64
#define SSDP_SERIAL_NUMBER_SIZE     37
#define SSDP_PRESENTATION_URL_SIZE  128
#define SSDP_MODEL_NAME_SIZE        64
#define SSDP_MODEL_URL_SIZE         128
#define SSDP_MODEL_VERSION_SIZE     32
#define SSDP_MANUFACTURER_SIZE      64
#define SSDP_MANUFACTURER_URL_SIZE  128

#define SSDP_DELTA_MS               100L

#define SN(b,s,...)                 s=1+snprintf_P(NULL,0,## __VA_ARGS__); b=(char*)malloc(s); snprintf_P(b,s,## __VA_ARGS__)  // 321696

typedef enum {
  NONE,
  SEARCH,
  NOTIFY
} ssdp_method_t;

typedef enum {
  STA_READ,
  STA_HIDE,
  STA_SHOW,
  STA_TOGGLE,
  AP_READ,
  AP_HIDE,
  AP_SHOW,
  AP_TOGGLE
} ssdp_manage_t;


class UdpContext;

class SSDPBoth{
  public:
    SSDPBoth();
    ~SSDPBoth();
    void end();
    bool begin();
    void update();
    void schema(WiFiClient client);
    int  manage(ssdp_manage_t manage);

    void setDeviceType(const String& deviceType) { setDeviceType(deviceType.c_str()); }
    void setDeviceType(const char *deviceType);
	
    /*To define a custom UUID, you must call the method before begin(). Otherwise an automatic UUID based on CHIPID will be generated.*/
    void setUUID(const String& uuid)	{ setUUID(uuid.c_str()); }
    void setUUID(const char *uuid);

    void setName(const String& name) { setName(name.c_str()); }
    void setName(const char *name);
    void setURL(const String& url) { setURL(url.c_str()); }
    void setURL(const char *url);
    void setSchemaURL(const String& url) { setSchemaURL(url.c_str()); }
    void setSchemaURL(const char *url);
    void setSerialNumber(const String& serialNumber) { setSerialNumber(serialNumber.c_str()); }
    void setSerialNumber(const char *serialNumber);
    void setSerialNumber(const uint32_t serialNumber);
    void setModelName(const String& name) { setModelName(name.c_str()); }
    void setModelName(const char *name);
    void setModelNumber(const String& num) { setModelNumber(num.c_str()); }
    void setModelNumber(const char *num);
    void setModelURL(const String& url) { setModelURL(url.c_str()); }
    void setModelURL(const char *url);
    void setManufacturer(const String& name) { setManufacturer(name.c_str()); }
    void setManufacturer(const char *name);
    void setManufacturerURL(const String& url) { setManufacturerURL(url.c_str()); }
    void setManufacturerURL(const char *url);
    void setHTTPPort(uint16_t port);
    void setTTL(uint8_t ttl);

  protected:
    void _send(ssdp_method_t method);
    void _update();

    UdpContext* _server;
    bool _pending;
    uint8_t _ttl;
    uint16_t _port;
    unsigned short _delay;
    unsigned long _millis;
    unsigned long _notify_time;
    unsigned long _process_time;
    uint16_t  _respondToPort;
    ip_addr_t _respondToAddr;
    struct ip_info _sta;
    struct ip_info _ap;
    int _hideSTA;
    int _hideAP;

    char _deviceType[SSDP_DEVICE_TYPE_SIZE];
    char _friendlyName[SSDP_FRIENDLY_NAME_SIZE];
    char _manufacturer[SSDP_MANUFACTURER_SIZE];
    char _manufacturerURL[SSDP_MANUFACTURER_URL_SIZE];
    char _modelName[SSDP_MODEL_NAME_SIZE];
    char _modelNumber[SSDP_MODEL_VERSION_SIZE];
    char _modelURL[SSDP_MODEL_URL_SIZE];
    char _presentationURL[SSDP_PRESENTATION_URL_SIZE];
    char _schemaURL[SSDP_SCHEMA_URL_SIZE];
    char _serialNumber[SSDP_SERIAL_NUMBER_SIZE];
    char _uuid[SSDP_UUID_SIZE];
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SSDP)
extern SSDPBoth SSDP;
#endif

#endif
