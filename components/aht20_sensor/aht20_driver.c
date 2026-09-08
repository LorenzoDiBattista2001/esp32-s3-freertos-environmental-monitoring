#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "aht20_driver.h"

static const char *TAG = "AHT20_DRIVER";

#define I2C_TIMEOUT_MS 1000

esp_err_t aht20_init(i2c_master_bus_handle_t bus_handle, aht20_dev_handle_t *aht20_ret_handle) {

    ESP_RETURN_ON_FALSE(bus_handle != NULL && aht20_ret_handle != NULL, ESP_ERR_INVALID_ARG, 
                        TAG, "Invalid Arguments");

    // wait 100 ms after power-on
    vTaskDelay(pdMS_TO_TICKS(100));

    // add device to I2C bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AHT20_SENSOR_ADDR,
        .scl_speed_hz = 100000,
    };

    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus_handle, &dev_config, aht20_ret_handle), 
                        TAG, "Failed to add sensor to I2C bus");

    // get status word
    uint8_t status_word;
    uint8_t status_cmd = AHT20_CMD_GET_STATUS;
    ESP_RETURN_ON_ERROR(i2c_master_transmit(*aht20_ret_handle, &status_cmd, sizeof(status_cmd), I2C_TIMEOUT_MS), 
                        TAG, "Failed to send status command");
    ESP_RETURN_ON_ERROR(i2c_master_receive(*aht20_ret_handle, &status_word, sizeof(status_word), I2C_TIMEOUT_MS), 
                        TAG, "Failed to read status word");

    // send calibration command
    
    if(!((status_word & AHT20_STATUS_CALIBRATED_BIT_MASK) == AHT20_STATUS_CALIBRATED_BIT_MASK)) {
        uint8_t cal_cmd[3] = {
            AHT20_CMD_INITIALIZE, 
            AHT20_INIT_PARAM_0, 
            AHT20_INIT_PARAM_1
        };
        ESP_RETURN_ON_ERROR(i2c_master_transmit(*aht20_ret_handle, cal_cmd, sizeof(cal_cmd), I2C_TIMEOUT_MS), 
                            TAG, "Failed to send calibration command");

        // wait 10 ms after sending calibration command
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "AHT20 sensor successfully initialized");
    return ESP_OK;
}

esp_err_t aht20_read_data(aht20_dev_handle_t aht20_handle, aht20_sensor_data_t *data) {

    ESP_RETURN_ON_FALSE(aht20_handle != NULL && data != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid Arguments");

    // send trigger measurement command
    uint8_t trig_cmd[3] = {
        AHT20_CMD_TRIGGER_MEASUREMENT, 
        AHT20_TRIGGER_PARAM_0, 
        AHT20_TRIGGER_PARAM_1
    };
    ESP_RETURN_ON_ERROR(i2c_master_transmit(aht20_handle, trig_cmd, sizeof(trig_cmd), I2C_TIMEOUT_MS), 
                        TAG, "Failed to trigger measurement");

    // wait for the measurement to be completed
    vTaskDelay(pdMS_TO_TICKS(80));

    // read status word + six bytes
    uint8_t measurement[7];
    ESP_RETURN_ON_ERROR(i2c_master_receive(aht20_handle, measurement, sizeof(measurement), I2C_TIMEOUT_MS), 
                        TAG, "Failed to read data");

    // check busy bit
    uint8_t status_word = measurement[0];
    ESP_RETURN_ON_FALSE((status_word & AHT20_STATUS_BUSY_BIT_MASK) == 0U, ESP_ERR_INVALID_STATE, 
                        TAG, "Sensor is still busy");

    // extract measurement data
    uint32_t humidity_bits = ((uint32_t)measurement[1] << 12) | 
                             ((uint32_t)measurement[2] << 4)  | 
                             ((uint32_t)measurement[3] >> 4);

    uint32_t temperature_bits = ((uint32_t)(measurement[3] & 0x0F) << 16) |
                                ((uint32_t)measurement[4] << 8) |
                                ((uint32_t)measurement[5]);

    // convert raw data
    data->humidity = ((float)humidity_bits / 1048576.0f) * 100.0f;
    data->temperature = (((float)temperature_bits / 1048576.0f) * 200.0f) - 50.0f;

    return ESP_OK;

}

esp_err_t aht20_del(aht20_dev_handle_t aht20_handle) {
    ESP_RETURN_ON_FALSE(aht20_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Invalid Argument");
    return i2c_master_bus_rm_device(aht20_handle);
}