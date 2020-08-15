/* brushed dc motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include "config.h"
#include "motor.h"
#include "wifi_manager.h"

TaskHandle_t TaskSub;
SemaphoreHandle_t baton;


uint8_t gate_state = OPENED;

/*
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */
void monitoring_task(void *arg)
{
	for(;;){
		printf("free heap: %d\n",esp_get_free_heap_size());
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}

/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */
void connection_ok(void *arg)
{
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)arg;
	/* transform IP to human readable string */
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);
	printf("Connected at IP %s", str_ip);

}

void app_main(void)
{
    wifi_manager_start();
    // xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);
    // xTaskCreatePinnedToCore(
    // &motor_task,                          /* Task Function */
    // "motor_task",                        /* Name of Task */
    // STACK,                               /* Stack size of Task */
    // NULL,                                /* Parameter of the task */
    // 0,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
    // &TaskSub,                            /* Task handle to keep track of created task */
    // 0);                                  /* CoreID */
 
    /* register a callback as an example to how you can integrate your code with the wifi manager */
	wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &connection_ok);

    //A task on core 2 that monitors free heap memory
    xTaskCreatePinnedToCore(
    &monitoring_task,                    /* Task Function */
    "monitoring_task",                 /* Name of Task */
    2048,                                /* Stack size of Task */
    NULL,                                /* Parameter of the task */
    1,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
    NULL,                                /* Task handle to keep track of created task */
    1);                                  /* CoreID */
  
}
