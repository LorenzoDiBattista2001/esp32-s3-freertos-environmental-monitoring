#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "aht20_driver.h"
#include "ssd1306_driver.h"

static const char *TAG = "Temperature_Monitoring";

#define I2C_MASTER_SCL_GPIO 4
#define I2C_MASTER_SDA_GPIO 5

static void i2c_master_init(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_GPIO,
        .scl_io_num = I2C_MASTER_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void app_main(void)
{
    // I2C Master Bus Initialization
    i2c_master_bus_handle_t bus_handle;
    i2c_master_init(&bus_handle);

    // I2C Devices Initialization
    aht20_dev_handle_t aht20_handle;
    esp_err_t ret = aht20_init(bus_handle, &aht20_handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AHT20 Sensor initialization failed");
        return;
    }

    ssd1306_dev_handle_t ssd1306_handle;
    ret = ssd1306_init(bus_handle, &ssd1306_handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SSD1306 Display initialization failed");
        return;
    }

    // Tasks Creation
}
