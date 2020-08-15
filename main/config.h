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

#define CLOSED  '0'
#define OPENED  '1'
#define CLOSING '2'
#define OPENING '3'
/******** Core 0 declaration ***********************/
#define STACK   5120 // Stack size of CORE0 Task

////////////////////////////////////////COMMUNICATION SETUP//////////////////////////////////

/*
// HTML & CSS contents which display on web server
char HTML[] = "<style> \
    button { \
        color: white; \
        width:33vw; \
        height: 20vh; \
        background-color: #33c930; \
        font-size: 200%; \
        position: absolute; \
        top: 50%; \
        left: 50%; \
        transform: translate(-50%,-50%); \
        border-radius: 10px; \
        border: 0; \
        outline: 0; \
        transition: .05s; \
        font-weight: bold; \
    } \
    button:active { \
        background-color: brown; \
        font-size: 180%; \
        width: 32vw; \
        height: 19vh; \
    } \
    button.close { \
        background-color: #db2323; \
    } \
    button.closing { \
        background-color: #9c0b0b; \
    } \
    button.opening { \
        background-color: #0c8c0a; \
    } \
</style> \
<script> \
    updateState(); \
    var interv = null; \
    function toggleState() { \
        if (interv) clearInterval(interv); \
        console.log('inv'); \
        interv = setInterval(updateState, 1000); \
        var xhttp = new XMLHttpRequest(); \
        xhttp.open('GET', 'rotate'); \
        xhttp.send(); \
    } \
    function updateState() { \
        var xhttp = new XMLHttpRequest(); \
        xhttp.onreadystatechange = function() { \
            if (this.readyState == 4 && this.status == 200) { \
                console.log(this.responseText); \
                var button = document.getElementById('btn'); \
                if (this.responseText === '0') { \
                  button.innerHTML = 'M&#7903; c&#7917;a'; \
                  button.className = 'Open'; \
                  clearInterval(interv); \
                  interv = null; \
                } else if (this.responseText === '1') { \
                  button.innerHTML = '&#272;&#243;ng c&#7917;a'; \
                  button.className = 'Close'; \
                  clearInterval(interv); \
                  interv = null; \
                } else if (this.responseText === '2') { \
                  button.innerHTML = 'C&#7917;a &#273;ang &#273;&#243;ng...'; \
                  button.className = 'Closing'; \
                } else if (this.responseText === '3') { \
                  button.innerHTML = 'C&#7917;a &#273;ang m&#7903;...'; \
                  button.className = 'Opening'; \
                } \
            } \
        }; \
        xhttp.open('GET', 'get_state'); \
        xhttp.send(); \
    } \
</script> \
<div>H&#7879; Th&#7889;ng C&#7917;a Tr&#7901;i T&#7921; &#272;&#7897;ng</div> \
<button id=\"btn\" onclick=\"toggleState()\">OPEN</button>";
*/
//https://art-hanoi.com/vncode/

#endif //__CONFIG_H
