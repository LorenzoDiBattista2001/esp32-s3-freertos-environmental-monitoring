idf_component_register(SRCS "mqtt_publisher.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event mqtt
                    EMBED_TXTFILES "adafruit_ca.pem")