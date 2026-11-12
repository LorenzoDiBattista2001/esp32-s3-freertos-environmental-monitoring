#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define MQTT_CONNECTED_BIT BIT1

esp_err_t mqtt_publisher_init(EventGroupHandle_t net_event_group);

esp_err_t mqtt_publish_data(void *data);

#endif