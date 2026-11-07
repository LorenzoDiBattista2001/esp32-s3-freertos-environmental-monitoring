#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0

esp_err_t wifi_initialize_network(void);

esp_err_t wifi_configure_station(void);

esp_err_t wifi_start_station(void);

esp_err_t wifi_controller_init(EventGroupHandle_t net_event_group);

#endif