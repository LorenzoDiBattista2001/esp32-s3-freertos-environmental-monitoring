#ifndef AHT20_DRIVER_H
#define AHT20_DRIVER_H

#include "esp_err.h"
#include "driver/i2c_master.h"

#define AHT20_SENSOR_ADDR 0x38
#define AHT20_CMD_GET_STATUS 0x71

#define AHT20_CMD_INITIALIZE 0xBE
#define AHT20_INIT_PARAM_0 0x08
#define AHT20_INIT_PARAM_1 0x00

#define AHT20_CMD_TRIGGER_MEASUREMENT 0xAC
#define AHT20_TRIGGER_PARAM_0 0x33
#define AHT20_TRIGGER_PARAM_1 0x00

#define AHT20_STATUS_CALIBRATED_BIT_MASK 0x08
#define AHT20_STATUS_BUSY_BIT_MASK 0x80

typedef i2c_master_dev_handle_t aht20_dev_handle_t;

typedef struct {
    float temperature;
    float humidity;
} aht20_sensor_data_t;

esp_err_t aht20_init(i2c_master_bus_handle_t bus_handle, aht20_dev_handle_t *aht20_ret_handle);

esp_err_t aht20_read_data(aht20_dev_handle_t aht20_handle, aht20_sensor_data_t *data);

esp_err_t aht20_del(aht20_dev_handle_t aht20_handle);

#endif