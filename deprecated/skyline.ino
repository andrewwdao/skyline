
#define DEBUG

#include "config.h"

/********* Global State  **********/
WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

/*************************************RAM EXECUTED INTERRUPT **********************************************/
void IRAM_ATTR open_limit_isr() {
  taskENTER_CRITICAL(&lock);
  LIMIT_FLAG = true;
  D_PRINTLN("Open Limit reached!");
  taskEXIT_CRITICAL(&lock);
}
void IRAM_ATTR close_limit_isr() {
  taskENTER_CRITICAL(&lock);
  LIMIT_FLAG = true;
  D_PRINTLN("Close Limit reached!");
  taskEXIT_CRITICAL(&lock);
}


void setup(){
  WiFi.mode(WIFI_STA);
  DEBUG_BEGIN();
  vSemaphoreCreateBinary(baton); //initialize binary semaphore //baton = xSemaphoreCreateBinary(); //this works too but not as good as the current use
  xSemaphoreTake(baton, portMAX_DELAY); // ( TickType_t ) and portTICK_PERIOD_MS is also available , view: http://esp32.info/docs/esp_idf/html/d1/d19/group__xSemaphoreTake.html 
  xTaskCreatePinnedToCore(
    Task_Motor,                          /* Task Function */
    "Motor_task",                        /* Name of Task */
    STACK,                               /* Stack size of Task */
    NULL,                                /* Parameter of the task */
    0,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
    &TaskSub,                            /* Task handle to keep track of created task */
    0);                                  /* CoreID */
  
  Wifi_begin();
  webserver_init();
  xSemaphoreGive(baton);
}

 
void loop(){
  server.handleClient();
}
