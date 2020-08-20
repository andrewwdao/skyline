/*------------------------------------------------------------*-
  Webserver - header file
  (c) Minh-An Dao - Anh Khoi Tran 2020
  version 1.00 - 20/08/2020
---------------------------------------------------------------
 * Init Wifi and create local webserver
 * 
 --------------------------------------------------------------*/
#ifndef __WEBSERVER_H
#define __WEBSERVER_H
#include "config.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_pm.h"
#include "nvs_flash.h"
#include <esp_http_server.h>

// ------ Public constants ------------------------------------
/**
 * @note set the ssid and password via "idf.py menuconfig
 */
#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PWD  CONFIG_WIFI_PWD

#define LISTEN_INTERVAL CONFIG_LISTEN_INTERVAL

#if CONFIG_POWER_SAVE_MIN
#define PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_POWER_SAVE_MAX
#define PS_MODE WIFI_PS_MAX_MODEM
#elif CONFIG_POWER_SAVE_NONE
#define PS_MODE WIFI_PS_NONE
#else
#define PS_MODE WIFI_PS_NONE
#endif


// ------ Public function prototypes --------------------------
void           wifiSTA_init(void);
httpd_handle_t start_webserver(void);
void           stop_webserver(httpd_handle_t);
// ------ Public variable -------------------------------------
// extern SemaphoreHandle_t baton;
extern char GATE_STATE;
extern bool STOP_FLAG;
#endif