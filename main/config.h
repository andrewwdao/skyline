/*  config file
 *  (c) Minh-An Dao 2020 
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_log.h"

// #include "WiFi.h"
// #include <WebServer.h>

// //OTA LIBRARY
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

#ifdef DEBUG // Define actual debug output functions when necessary.
  // #ifndef DEBUG_BEGIN()
  //   #define DEBUG_BEGIN()      {Serial.begin(115200);}
  // #endif
  #define D_PRINT(...)       {printf(__VA_ARGS__);}
  // #define D_PRINTLN(...)     {Serial.println(__VA_ARGS__);}
  // #define D_PRINTF(...)      {Serial.printf(__VA_ARGS__);}
#else
  // #ifndef DEBUG_BEGIN()
  //   #define DEBUG_BEGIN()  {}
  // #endif
  #define D_PRINT(...)   {}
  // #define D_PRINTLN(...) {}
  // #define D_PRINTF(...)  {}
#endif


/////////////////////////////////////////////////////////////////////////////////////////
#define HIGH 1
#define LOW  0

#define DELAY 200

#define CLOSED  '0'
#define OPENED  '1'
#define CLOSING '2'
#define OPENING '3'
/******** Core 0 declaration ***********************/
#define STACK   4096 // Stack size of CORE0 Task


//https://art-hanoi.com/vncode/

#endif //__CONFIG_H
