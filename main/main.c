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
#include "webserver.h"
// #include "wifi_manager.h"
// #include "http_app.h"

TaskHandle_t TaskSub;
SemaphoreHandle_t baton;

char GATE_STATE = OPENED;

/** @brief tag used for ESP serial console messages */
static const char TAG[] = "MAIN";

/**
 * @brief auto disconnect_handler for stopping webserver in wifi service
 */
static void disconnect_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver...");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver...");
        *server = start_webserver();
    }
}

/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code! This is an example on how you can integrate your code with wifi-manager
 */
void monitoring_task(void *arg)
{
	for(;;){
		ESP_LOGI(TAG, "free heap: %d\n",esp_get_free_heap_size());
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}

void app_main(void)
{
    vSemaphoreCreateBinary(baton); //initialize binary semaphore //baton = xSemaphoreCreateBinary(); //this works too but not as good as the current use
    xSemaphoreTake(baton, portMAX_DELAY); // ( TickType_t ) and portTICK_PERIOD_MS is also available , view: http://esp32.info/docs/esp_idf/html/d1/d19/group__xSemaphoreTake.html 

    // --------Initialize NVS - must have -------
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    //-------------- Motor task ------------------
    xTaskCreate(
        &motor_task,                          /* Task Function */
        "motor_task",                        /* Name of Task */
        STACK,                               /* Stack size of Task */
        NULL,                                /* Parameter of the task */
        3,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
        &TaskSub);                            /* Task handle to keep track of created task */

    xSemaphoreGive(baton); //give out the baton for other thread to catch

    //------------ Webserver task -----------------
    //wifi and webserver initialization and configuration
    wifiSTA_init();

    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               WIFI_EVENT_STA_DISCONNECTED,
                                               &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();

    
    //------------ monitoring task -----------------
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
