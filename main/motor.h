/*------------------------------------------------------------*-
  Brushed DC motor controller - header file
  (c) Minh-An Dao - Nhu-Phung Tran-Thi 2020
  version 1.10 - 20/08/2020
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

// ------ Public constants ------------------------------------
// ------ Public function prototypes --------------------------
void motor_task(void *);

// ------ Public variable -------------------------------------
// extern SemaphoreHandle_t baton;
extern char GATE_STATE;
extern bool STOP_FLAG;

#endif