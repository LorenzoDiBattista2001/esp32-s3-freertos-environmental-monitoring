# Temperature Monitoring IoT Project with ESP32-S3 and FreeRTOS

This is an extremely simple, work-in-progress IoT project based on the ESP32-S3 SoC and FreeRTOS Real-Time Operating System for monitoring environmental temperature. The system under development will constantly sample temperature values by means of the AHT20 temperature and humidity sensor, transmitting them to both an SSD1306 OLED display (via the I2C serial protocol) and an online dashboard leveraging the ESP32-S3 Wi-Fi capabilities.

### System Architecture

The firmware under development will comprise three independent FreeRTOS tasks dedicated to:
* Reading temperature values from the AHT20 sensor
* Printing data on the SSD1306 OLED display
* Sending telemetry to an online dashboard using the MQTT protocol and the ESP-IDF LwIP TCP/IP stack

These tasks will communicate and exchange data by means of two FreeRTOS queues: one dedicated to the SSD1306 display and the other to the custom MQTT client.

### Main Goals and Expected Outcomes

The primary objective of this project is to gain basic hands-on experience with microcontroller programming and the I2C serial communication protocol. Special attention will also be paid to common Inter-Task Communication problems and their solutions, focusing on the use of semaphores and mutexes.