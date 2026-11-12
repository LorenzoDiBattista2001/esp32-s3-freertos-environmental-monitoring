#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_publisher.h"
#include "esp_tls.h"

#define BROKER_URL CONFIG_BROKER_URL
#define CLIENT_USERNAME CONFIG_CLIENT_USERNAME
#define CLIENT_PASSWORD CONFIG_CLIENT_PASSWORD
#define TOPIC_STRING CONFIG_TOPIC_STRING

static const char *TAG = "MQTT_PUBLISHER";

static EventGroupHandle_t s_network_event_group = NULL;

extern const uint8_t adafruit_ca_pem_start[] asm("_binary_adafruit_ca_pem_start");

extern const uint8_t adafruit_ca_pem_end[] asm("_binary_adafruit_ca_pem_end");

static esp_mqtt_client_handle_t mqtt_client;

static void mqtt_event_handler(void *arg, esp_event_base_t event_base, 
                                    int32_t event_id, void *event_data) 
{
    switch((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Successfully connected to MQTT broker");
            if (s_network_event_group != NULL) {
                xEventGroupSetBits(s_network_event_group, MQTT_CONNECTED_BIT);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            if (s_network_event_group != NULL) {
                xEventGroupClearBits(s_network_event_group, MQTT_CONNECTED_BIT);
            }
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Published message was received by MQTT Broker");
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT client initialized");
            break;
        default:
            ESP_LOGE(TAG, "Unhandled MQTT event");
            break;
    }
}

esp_err_t mqtt_publisher_init(EventGroupHandle_t net_event_group) {
    
    ESP_RETURN_ON_FALSE(net_event_group != NULL, ESP_ERR_INVALID_ARG, TAG, "Event Group Handle is NULL");
    s_network_event_group = net_event_group;

    esp_mqtt_client_config_t mqtt_config = {
        .broker = {
            .address.uri = BROKER_URL,
            .verification.certificate = (const char *)adafruit_ca_pem_start
        },
        .credentials = { 
            .username = CLIENT_USERNAME,
            .authentication.password = CLIENT_PASSWORD
        }
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    ESP_RETURN_ON_ERROR(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL), 
                            TAG, "Failed to register MQTT client event handler");
    ESP_RETURN_ON_ERROR(esp_mqtt_client_start(mqtt_client), TAG, "Failed to start MQTT client");

    return ESP_OK;
}

esp_err_t mqtt_publish_data(void *data) {

    ESP_RETURN_ON_FALSE(mqtt_client != NULL, ESP_ERR_INVALID_ARG, TAG, "MQTT client handle not initialized");

    int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_STRING, (const char *) data, 0, 1, 0);

    if (msg_id < 0) {
        ESP_LOGW(TAG, "Failed to publish data to MQTT broker");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Successfully published data to MQTT broker");
    return ESP_OK;
}








