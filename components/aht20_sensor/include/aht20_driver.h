#ifndef AHT20_DRIVER_H
#define AHT20_DRIVER_H

#include "esp_err.h"
#include "driver/i2c_master.h"

typedef i2c_master_dev_handle_t aht20_dev_handle_t;

typedef struct {
    float temperature;
    float humidity;
} aht20_sensor_data_t;

esp_err_t aht20_init(i2c_master_bus_handle_t bus_handle, aht20_dev_handle_t *aht20_ret_handle);

esp_err_t aht20_read_data(aht20_dev_handle_t aht20_handle, aht20_sensor_data_t *data);

esp_err_t aht20_del(aht20_dev_handle_t aht20_handle);

#endif