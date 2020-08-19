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

volatile char GATE_STATE = OPENED;

/* @brief tag used for ESP serial console messages */
static const char TAG[] = "MAIN";


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

/*
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

/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */
// void connection_ok(void *arg)
// {
// 	ip_event_got_ip_t* param = (ip_event_got_ip_t*)arg;
// 	/* transform IP to human readable string */
// 	char str_ip[16];
// 	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);
// 	printf("Connected at IP %s", str_ip);

// }

// static esp_err_t webapp_handler(httpd_req_t *req){

// 	/* our custom page sits at /ctl */
// 	if(strcmp(req->uri, "/ctl") == 0){
// 		ESP_LOGI(TAG, "Serving page /ctl");
// 		httpd_resp_set_status(req, "200 OK");
// 		httpd_resp_set_type(req, "text/html");
// 		httpd_resp_send(req, html_response, strlen(html_response));
// 	} else if(strcmp(req->uri, "/get_state") == 0){
// 		ESP_LOGI(TAG, "Serving page /get_state");
// 		httpd_resp_set_status(req, "200 OK");
// 		httpd_resp_set_type(req, "application/json");
// 		httpd_resp_send(req, &GATE_STATE, 1);
// 	} else if(strcmp(req->uri, "/rotate") == 0){
// 		ESP_LOGI(TAG, "Serving page /rotate");
//         if (GATE_STATE == CLOSED) {
//             GATE_STATE = OPENING;
//         } else if (GATE_STATE == OPENED) {
//             GATE_STATE = CLOSING;
//         } else if (GATE_STATE == CLOSING) {
//             GATE_STATE = OPENED;
//         } else if (GATE_STATE == OPENING) {
//             GATE_STATE = CLOSED;
//         }
// 		httpd_resp_set_status(req, "200 OK");
// 		httpd_resp_set_type(req, "application/json");
// 		httpd_resp_send(req, &GATE_STATE, 1);
// 	} else
//     {
// 		/* send a 404 otherwise */
// 		httpd_resp_send_404(req);
// 	}
// 	return ESP_OK;
// }

void app_main(void)
{
    vSemaphoreCreateBinary(baton); //initialize binary semaphore //baton = xSemaphoreCreateBinary(); //this works too but not as good as the current use
    xSemaphoreTake(baton, portMAX_DELAY); // ( TickType_t ) and portTICK_PERIOD_MS is also available , view: http://esp32.info/docs/esp_idf/html/d1/d19/group__xSemaphoreTake.html 

    // Initialize NVS - must have
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    xTaskCreate(
        &motor_task,                          /* Task Function */
        "motor_task",                        /* Name of Task */
        STACK,                               /* Stack size of Task */
        NULL,                                /* Parameter of the task */
        3,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
        &TaskSub);                            /* Task handle to keep track of created task */

    // xTaskCreate(
    //     &webserver_task,                          /* Task Function */
    //     "webserver_task",                        /* Name of Task */
    //     STACK,                               /* Stack size of Task */
    //     NULL,                                /* Parameter of the task */
    //     4,                                   /* Priority of the task, vary from 0 to N, bigger means higher piority, need to be 0 to be lower than the watchdog*/
    //     &TaskSub);                            /* Task handle to keep track of created task */
 
    // wifi_manager_start();
    // /* register a callback as an example to how you can integrate your code with the wifi manager */
	// wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &connection_ok);
    // /* set custom handler for the http server
	//  * Now navigate to /controller to see the custom page
	//  */
	// http_app_set_handler_hook(HTTP_GET, &webapp_handler);

    xSemaphoreGive(baton); //give out the baton for other thread to catch

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
