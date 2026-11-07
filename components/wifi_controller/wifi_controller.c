#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi_controller.h"

#define WIFI_SSID CONFIG_WIFI_SSID
#define WIFI_PASS CONFIG_WIFI_PASSWORD
#define WIFI_MAXIMUM_RETRY CONFIG_MAXIMUM_RETRY

static const char *TAG = "WIFI_CONTROLLER";

static int s_retry_num = 0;
static EventGroupHandle_t s_network_event_group = NULL;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, 
                                    int32_t event_id, void *event_data) 
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                if (s_retry_num < WIFI_MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGW(TAG, "Retry to connect to the Access Point");
                } else {
                    if (s_network_event_group != NULL) {
                        xEventGroupClearBits(s_network_event_group, WIFI_CONNECTED_BIT);
                    }
                }
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Successfully connected to the Access Point");
                break;
            default:
                ESP_LOGE(TAG, "Unhandled Wi-Fi Event");
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Station got IP:" IPSTR, IP2STR(&event->ip_info.ip));

        s_retry_num = 0;

        if (s_network_event_group != NULL) {
            xEventGroupSetBits(s_network_event_group, WIFI_CONNECTED_BIT);
        }
    }
    else {
        ESP_LOGE(TAG, "Unexpected event");
    }
}

esp_err_t wifi_controller_init(EventGroupHandle_t net_event_group) {
    ESP_RETURN_ON_FALSE(net_event_group != NULL, ESP_ERR_INVALID_ARG, TAG, "Event Group Handle is NULL");
    s_network_event_group = net_event_group;

    ESP_RETURN_ON_ERROR(wifi_initialize_network(), TAG, "Failed to initialize network stack");
    ESP_RETURN_ON_ERROR(wifi_configure_station(), TAG, "Failed to configure Wi-Fi Station");
    ESP_RETURN_ON_ERROR(wifi_start_station(), TAG, "Failed to start Wi-Fi station");

    return ESP_OK;
}

esp_err_t wifi_initialize_network(void) {

    // Initialize network interface
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Failed to initialize TCP/IP Stack");

    // Create default event loop
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "Failed to create default event loop");

    // Bind the network interface to the Wi-Fi driver
    esp_netif_create_default_wifi_sta();
 
    // Initialize Wi-Fi driver with default configurations

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialize Wi-Fi driver");

    // Register event handlers

    esp_event_handler_instance_t wifi_instance;
    esp_event_handler_instance_t ip_instance;

    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(WIFI_EVENT, 
                                                            ESP_EVENT_ANY_ID, 
                                                            &wifi_event_handler, 
                                                            NULL, &wifi_instance), 
                                                            TAG, "Failed to register Wi-Fi handler");
    ESP_RETURN_ON_ERROR(esp_event_handler_instance_register(IP_EVENT, 
                                                            IP_EVENT_STA_GOT_IP, 
                                                            &wifi_event_handler, 
                                                            NULL, &ip_instance), 
                                                            TAG, "Failed to register IP handler");
    ESP_LOGI(TAG, "Successfully initialized network interface and Wi-Fi Driver");
    ESP_LOGI(TAG, "Correctly registered Wi-Fi event handler instances");
    return ESP_OK;
}

esp_err_t wifi_configure_station(void) {

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set Wi-Fi station mode");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "Failed to set Wi-Fi station configurations");

    ESP_LOGI(TAG, "Successfully set configurations for Wi-Fi station");
    return ESP_OK;
}

esp_err_t wifi_start_station(void) {

    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start Wi-Fi station");

    ESP_LOGI(TAG, "Wi-Fi station initialization completed");

    return ESP_OK;
}