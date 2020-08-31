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
#include "esp_event.h"
#include "esp_pm.h"
#include "nvs_flash.h"
#include <sys/param.h>
#include <esp_http_server.h>

// ------ Public constants ------------------------------------
// ------ Public function prototypes --------------------------
void           wifiSTA_init(void);
httpd_handle_t start_webserver(void);
void           stop_webserver(httpd_handle_t);
// ------ Public variable -------------------------------------
// extern SemaphoreHandle_t baton;
extern char GATE_STATE;
extern bool STOP_FLAG;
#endif