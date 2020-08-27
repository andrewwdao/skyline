/** ------------------------------------------------------------*-
  Webserver - header file
  (c) Minh-An Dao - Anh Khoi Tran 2020
  version 1.00 - 20/08/2020
---------------------------------------------------------------
 * Init Wifi and create local webserver
 * 
 * @note html code and external files imported from cmake
 * @ref https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/api-guides/build-system.html#embedding-binary-data
 * @ref https://esp32.com/viewtopic.php?t=11219
 --------------------------------------------------------------*/
#ifndef __WEBSERVER_C
#define __WEBSERVER_C
#include "webserver.h"
// #include "my_html.h"
// ------ Private constants -----------------------------------
#define CLOSE_BUTTON 'c'
#define STOP_BUTTON  's'
#define OPEN_BUTTON  'o'
// ------ Private function prototypes -------------------------

// ------ Private variables -----------------------------------
/** @brief tag used for ESP serial console messages */
static const char *TAG = "webserver";

// ------ PUBLIC variable definitions -------------------------

//--------------------------------------------------------------
// FUNCTION DEFINITIONS
//--------------------------------------------------------------
/**
 * @brief handler for wifi connecting
 */
static void wifiSTA_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGW(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

/**
 * @brief handler for /ctl url
 */
static esp_err_t control_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serving /ctl");
    extern const char skyline_html_start[] asm("_binary_skyline_html_start");
    extern const char skyline_html_end[]   asm("_binary_skyline_html_end");
	httpd_resp_set_type(req, HTTPD_TYPE_TEXT); //"text/html"
    httpd_resp_send(req, skyline_html_start, skyline_html_end-skyline_html_start-1);
    return ESP_OK;
}

/**
 * @brief handler for /manifest.json - for the purpose of getting the webapp
 */
static esp_err_t manifest_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serving /manifest.json");
    extern const char manifest_json_start[] asm("_binary_manifest_json_start");
    extern const char manifest_json_end[]   asm("_binary_manifest_json_end");
	httpd_resp_set_type(req, HTTPD_TYPE_JSON);
    httpd_resp_send(req, manifest_json_start, manifest_json_end-manifest_json_start-1);
    return ESP_OK;
}

/**
 * @brief handler for /get_state url
 */
static esp_err_t getstate_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Serving /get_state");
	httpd_resp_set_type(req, HTTPD_TYPE_JSON); // "application/json"
    httpd_resp_send(req, &GATE_STATE, 1);
    return ESP_OK;
}

/**
 * @brief handler for /rotate url
 */
static esp_err_t rotate_get_handler(httpd_req_t *req)
{
	httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    int buf_len;
    char *buf;
    buf_len = httpd_req_get_hdr_value_len(req, "mode");
    if (buf_len > 0) {
        buf = malloc(++buf_len);
        if (!buf) {
            ESP_LOGE(TAG, "Failed to allocate memory of %d bytes!", buf_len);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
            return ESP_ERR_NO_MEM;
        }
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "mode", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "mode: %s", buf);
            if (*buf==CLOSE_BUTTON) {
                GATE_STATE = CLOSING;
            } else if (*buf==STOP_BUTTON) {
                STOP_FLAG = true;
                if (GATE_STATE == CLOSING) {
                    GATE_STATE = OPENED;
                } else if (GATE_STATE == OPENING) {
                    GATE_STATE = CLOSED;
                }
            } else if (*buf==OPEN_BUTTON) {
                GATE_STATE = OPENING;
            }
        } else {
            ESP_LOGE(TAG, "Error in getting value of Header1");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Error in getting value of Header1");
            free(buf);
            return ESP_FAIL;
        }

        free(buf);
    }
    httpd_resp_send(req, &GATE_STATE, 1);
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
static const httpd_uri_t basic_handlers[] = {
    { .uri      = "/ctl",
      .method   = HTTP_GET,
      .handler  = control_get_handler,
      .user_ctx = NULL,
    },
    { .uri      = "/manifest.json",
      .method   = HTTP_GET,
      .handler  = manifest_get_handler,
      .user_ctx = NULL,
    },
    { .uri      = "/get_state",
      .method   = HTTP_GET,
      .handler  = getstate_get_handler,
      .user_ctx = NULL,
    },
    { .uri      = "/rotate",
      .method   = HTTP_GET,
      .handler  = rotate_get_handler,
      .user_ctx = NULL,
    }
};

static const int basic_handlers_no = sizeof(basic_handlers)/sizeof(httpd_uri_t);
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief init wifi as sta and set power save mode
 */
void wifiSTA_init(void)
{

    ESP_LOGI(TAG, "Starting wifi...\n");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    //set static ip - https://esp32.com/viewtopic.php?f=2&t=14689
    esp_netif_dhcpc_stop(sta_netif);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 43, 174);
   	IP4_ADDR(&ip_info.gw, 192, 168, 43, 1);
   	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

    esp_netif_set_ip_info(sta_netif, &ip_info);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                        ESP_EVENT_ANY_ID, 
                                                        &wifiSTA_handler, 
                                                        NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, 
                                                        IP_EVENT_STA_GOT_IP, 
                                                        &wifiSTA_handler,
                                                        NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PWD,
            .listen_interval = LISTEN_INTERVAL,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "entering Power save mode...\n");
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
#if CONFIG_IDF_TARGET_ESP32
    esp_pm_config_esp32_t pm_config = {
#elif CONFIG_IDF_TARGET_ESP32S2
    esp_pm_config_esp32s2_t pm_config = {
#endif
            .max_freq_mhz = CONFIG_MAX_CPU_FREQ_MHZ,
            .min_freq_mhz = CONFIG_MIN_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
#endif // CONFIG_PM_ENABLE

    esp_wifi_set_ps(PS_MODE);
}

/**
 * @brief start the local Webserver and start the httpd service
 */
httpd_handle_t start_webserver(void)
{
    httpd_handle_t sv;
    httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
    /** @note Modify this setting to match the number of URI handlers */
    conf.max_uri_handlers  = 4;
    conf.server_port = 7497;

    /* This check should be a part of http_server */
    conf.max_open_sockets = (CONFIG_LWIP_MAX_SOCKETS - 3);

    if (httpd_start(&sv, &conf) == ESP_OK) {
        ESP_LOGI(TAG, "Started HTTP server on port: '%d'", conf.server_port);
        ESP_LOGI(TAG, "Max URI handlers: '%d'", conf.max_uri_handlers);
        ESP_LOGI(TAG, "Max Open Sessions: '%d'", conf.max_open_sockets);
        ESP_LOGI(TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
        ESP_LOGI(TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
        ESP_LOGI(TAG, "Max Stack Size: '%d'", conf.stack_size);
        
        // ------------------- register basic handlers ------------------
        ESP_LOGI(TAG, "Registering %d URI handlers...", basic_handlers_no);
        for (int i = 0; i < basic_handlers_no; i++) {
            if (httpd_register_uri_handler(sv, &basic_handlers[i]) != ESP_OK) {
                ESP_LOGW(TAG, "register failed for %d", i);
                return NULL;
            }
        }
        ESP_LOGI(TAG, "Done");
        //---------------------------------------------------------------
        return sv;
    }
    ESP_LOGW(TAG, "Error starting server!");
    return NULL;
}

/**
 * @brief stop the local Webserver by stopping the httpd service
 */
void stop_webserver(httpd_handle_t sv)
{
    ESP_LOGI(TAG, "Stopping httpd...");
    httpd_stop(sv);
    ESP_LOGI(TAG, "HTTPD Stopped: Current free memory: %d", esp_get_free_heap_size());
}

#endif