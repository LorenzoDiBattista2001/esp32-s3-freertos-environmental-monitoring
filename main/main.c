#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "aht20_driver.h"
#include "ssd1306_driver.h"

static const char *TAG = "MAIN_APP";

#define I2C_MASTER_SCL_GPIO 4
#define I2C_MASTER_SDA_GPIO 5

#define SENSOR_POLL_PERIOD_MS 5000

#define SENSOR_TASK_STACK_SIZE 3072
#define DISPLAY_TASK_STACK_SIZE 4096

#define SENSOR_TASK_PRIORITY 5
#define DISPLAY_TASK_PRIORITY 2

static QueueHandle_t s_display_queue = NULL;

void sensor_task(void *pvParameters) {

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(SENSOR_POLL_PERIOD_MS);

    aht20_dev_handle_t aht20_handle = (aht20_dev_handle_t) pvParameters;
    aht20_sensor_data_t data;
    esp_err_t ret;

    while(1) {

        ret = aht20_read_data(aht20_handle, &data);

        if (ret == ESP_OK) {
            xQueueOverwrite(s_display_queue, &data);
        } else {
            ESP_LOGW(TAG, "Failed to read data from AHT20 sensor: %s", esp_err_to_name(ret));
        }

        vTaskDelayUntil(&last_wake_time, frequency);
    }

    vTaskDelete(NULL);
}

void display_task(void *pvParameters) {

    ssd1306_dev_handle_t ssd1306_handle = (ssd1306_dev_handle_t) pvParameters;
    aht20_sensor_data_t data;
    char buffer[100];

    while (1) {

        if(xQueueReceive(s_display_queue, &data, portMAX_DELAY) == pdPASS) {
            snprintf(buffer, sizeof(buffer), "Temperature:\n%.2f C", data.temperature);

            ssd1306_clear();
            ssd1306_draw_string(buffer);
            ssd1306_update(ssd1306_handle);
        }
    }

    vTaskDelete(NULL);
}

static esp_err_t i2c_master_init(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_GPIO,
        .scl_io_num = I2C_MASTER_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    return i2c_new_master_bus(&bus_config, bus_handle);
}

void app_main(void)
{
    // Single-item queue so as to overwrite past state
    s_display_queue = xQueueCreate(1, sizeof(aht20_sensor_data_t));

    if (s_display_queue == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for display queue");
        return;
    }

    // I2C Master Bus Initialization
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_master_init(&bus_handle));

    // I2C Devices Initialization
    aht20_dev_handle_t aht20_handle;
    ESP_ERROR_CHECK(aht20_init(bus_handle, &aht20_handle));

    ssd1306_dev_handle_t ssd1306_handle;
    ESP_ERROR_CHECK(ssd1306_init(bus_handle, &ssd1306_handle));

    // Tasks Creation
    xTaskCreatePinnedToCore(sensor_task, "AHT20_Task", SENSOR_TASK_STACK_SIZE, 
                            (void *)aht20_handle, SENSOR_TASK_PRIORITY, NULL, 0);
    xTaskCreatePinnedToCore(display_task, "SSD1306_Task", DISPLAY_TASK_STACK_SIZE, 
                            (void *) ssd1306_handle, DISPLAY_TASK_PRIORITY, NULL, 0);
}
