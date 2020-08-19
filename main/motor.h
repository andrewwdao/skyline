/*------------------------------------------------------------*-
  Brushed DC motor controller - header file
  ESP32 CORE0 - COMMUNICATION CORE
  (c) Minh-An Dao - Nhu-Phung Tran-Thi 2020
  version 1.00 - 15/08/2020
---------------------------------------------------------------
 * Use MCPWM module to control brushed dc motor.
 * 
 *  ** CAUTION:
 *  This core is responsible for RF radio, Wifi stack and protocols (watchdog,...) 
 *  Therefore, you have to be EXTREMELY CAREFUL when create task for this core,
 *  in order to not intefere these default functions (watchdog has the piority of 1 among 0 to N, higher number means higer piority)
 *
 --------------------------------------------------------------*/
#ifndef __MOTOR_H
#define __MOTOR_H
#include "config.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "freertos/queue.h"
#include "driver/gpio.h"


// ------ Public constants ------------------------------------
/**************** pinout define *********************/
#define PWM_PIN 27 //Set GPIO as PWM0A
#define INA_PIN 25
#define INB_PIN 26
#define EN_PIN  33
#define MOTOR_PIN_SEL  ((1ULL<<EN_PIN)|(1ULL<<INA_PIN)|(1ULL<<INB_PIN))
  
// isr pins
#define OPEN_LIMIT_PIN  22
#define CLOSE_LIMIT_PIN 23
#define OPEN_DOOR_PIN 18
#define CLOSE_DOOR_PIN 19
#define STOP_DOOR_PIN 21
#define ISR_PIN_SEL  ((1ULL<<OPEN_LIMIT_PIN)|(1ULL<<CLOSE_LIMIT_PIN)|(1ULL<<OPEN_DOOR_PIN)|(1ULL<<CLOSE_DOOR_PIN)|(1ULL<<STOP_DOOR_PIN))

/**************** parameters define *********************/
// setting PWM properties
#define FREQ    5000
#define CHANNEL 0
#define RES     8

// ------ Public function prototypes --------------------------
void motor_task(void *);

// ------ Public variable -------------------------------------
extern SemaphoreHandle_t baton;
extern volatile char GATE_STATE;

#endif