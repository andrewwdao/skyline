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

#define DELAY_MS(...)     {vTaskDelay(__VA_ARGS__/portTICK_RATE_MS);}

/////////////////////////////////////////////////////////////////////////////////////////
#define HIGH 1
#define LOW  0

#define CLOSED  '0'
#define OPENED  '1'
#define CLOSING '2'
#define OPENING '3'

#endif //__CONFIG_H
