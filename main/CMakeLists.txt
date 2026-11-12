idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES aht20_sensor ssd1306_display wifi_controller mqtt_publisher nvs_flash)
