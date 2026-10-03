#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "ssd1306_driver.h"
#include "font_8x8_monospace.h"

static const char *TAG = "SSD1306_DRIVER";

#define I2C_TIMEOUT_MS 1000

#define SSD1306_DISPLAY_ADDR 0x3C // Extension bit SA0 = 0

// Control Bytes
#define SSD1306_CONTROL_BYTE_SINGLE_CMD 0x80 // [Co=1, D/C#=0, 0, 0, 0, 0, 0, 0]
#define SSD1306_CONTROL_BYTE_CMD_STREAM 0x00 // [Co=0, D/C#=0, 0, 0, 0, 0, 0, 0]
#define SSD1306_CONTROL_BYTE_SINGLE_DATA_BYTE 0xC0 // [Co=1, D/C#=1, 0, 0, 0, 0, 0, 0]
#define SSD1306_CONTROL_BYTE_DATA_STREAM 0x40 // [Co=0, D/C#=1, 0, 0, 0, 0, 0, 0]

// Charge Pump Setting
#define SSD1306_CMD_CHARGE_PUMP_SETTING 0x8D
#define SSD1306_PARAM_ENABLE_CHARGE_PUMP 0x14
#define SSD1306_PARAM_DISABLE_CHARGE_PUMP 0x10

// Fundamental Commands
#define SSD1306_CMD_SET_CONTRAST 0x81
#define SSD1306_CMD_DISPLAY_ON 0xAF
#define SSD1306_CMD_DISPLAY_OFF 0xAE
#define SSD1306_CMD_ENTIRE_DISPLAY_ON 0xA4 // Display resumes to RAM content
#define SSD1306_CMD_SET_NORMAL_DISPLAY 0xA6 // Pixel is on if bit is 1

// Addressing Setting Commands
#define SSD1306_CMD_SET_ADDRESS_MODE 0x20 // Sets memory addressing mode
#define SSD1306_PARAM_HORIZONTAL_MODE 0x00 // Horizontal addressing mode
#define SSD1306_CMD_SET_COLUMN_ADDRESS 0x21 // Sets start column and end column addresses
#define SSD1306_CMD_SET_PAGE_ADDRESS 0x22 // Sets first page and last page addresses

// Hardware Configuration Commands
#define SSD1306_CMD_SET_DISPLAY_START_LINE 0x40 // RAM display starts from line 0
#define SSD1306_CMD_SET_SEGMENT_REMAP 0xA1 // Column address 127 is mapped to SEG0
#define SSD1306_CMD_SET_MUX_RATIO 0xA8 // Sets multiplex ratio
#define SSD1306_CMD_SET_COM_PINS_CONFIG 0xDA // Sets COM pins hardware configuration
#define SSD1306_PARAM_ALTERNATIVE_COM_PINS 0x12 // Alternative COM pins configuration
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3 // Sets vertical shift
#define SSD1306_CMD_SET_COM_SCAN_DIRECTION 0xC8 // Sets COM output scan direction (remapped mode)

// Timing and Driving Scheme Commands
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIVIDE_RATIO 0xD5 // Sets the display clock (DCLK) divide ratio 
                                                        // and oscillator frequency. Followed by param A:
                                                        // A[3:0] Defines divide ratio of display clocks
                                                        // A[7:4] Sets oscillator frequency
                                                        // A = 0x80 recommended value for 80-100 Hz refresh rate
#define SSD1306_CMD_SET_PRECHARGE_PERIOD 0xD9 // Sets the precharge period. Followed by param A: 
                                              // A[3:0] Phase 1 Period (up to 15 DCLK clocks)
                                              // A[7:4] Phase 2 Period (up to 15 DCLK clocks)
                                              // A = 0xF1 recommended value for display's sharpness
#define SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL 0xDB // Sets deselect voltage level

static uint8_t s_framebuffer[1024] = {0};

// Display Initialization Functions

esp_err_t ssd1306_setup(ssd1306_dev_handle_t ssd1306_handle) {

    ESP_RETURN_ON_FALSE(ssd1306_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Null pointer passed as handle");

    uint8_t init_sequence[32];
    size_t cmd_index = 0;

    // add commands and parameters to the write buffer

    init_sequence[cmd_index++] = SSD1306_CONTROL_BYTE_CMD_STREAM; // Do not expect another Control Byte +
                                                                  // Every byte that follows is a command
    init_sequence[cmd_index++] = SSD1306_CMD_DISPLAY_OFF;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_DISPLAY_CLOCK_DIVIDE_RATIO;
    init_sequence[cmd_index++] = 0x80;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_MUX_RATIO;
    init_sequence[cmd_index++] = 0x3F;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_DISPLAY_OFFSET;
    init_sequence[cmd_index++] = 0x00;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_DISPLAY_START_LINE;
    init_sequence[cmd_index++] = SSD1306_CMD_CHARGE_PUMP_SETTING;
    init_sequence[cmd_index++] = SSD1306_PARAM_ENABLE_CHARGE_PUMP;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_ADDRESS_MODE;
    init_sequence[cmd_index++] = SSD1306_PARAM_HORIZONTAL_MODE;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_SEGMENT_REMAP;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_COM_SCAN_DIRECTION;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_COM_PINS_CONFIG;
    init_sequence[cmd_index++] = SSD1306_PARAM_ALTERNATIVE_COM_PINS;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_CONTRAST;
    init_sequence[cmd_index++] = 0x7F;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_PRECHARGE_PERIOD;
    init_sequence[cmd_index++] = 0xF1;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL;
    init_sequence[cmd_index++] = 0x40;
    init_sequence[cmd_index++] = SSD1306_CMD_ENTIRE_DISPLAY_ON;
    init_sequence[cmd_index++] = SSD1306_CMD_SET_NORMAL_DISPLAY;
    init_sequence[cmd_index++] = SSD1306_CMD_DISPLAY_ON;

    ESP_RETURN_ON_ERROR(i2c_master_transmit(ssd1306_handle, init_sequence, cmd_index, I2C_TIMEOUT_MS),
                        TAG, "Failed to send initialization sequence");
    
    return ESP_OK;
}

esp_err_t ssd1306_init(i2c_master_bus_handle_t bus_handle, ssd1306_dev_handle_t *ssd1306_ret_handle) {

    ESP_RETURN_ON_FALSE(bus_handle != NULL && ssd1306_ret_handle != NULL, ESP_ERR_INVALID_ARG, 
                        TAG, "Null pointers passed as handles");

    // wait 20 ms after power-on
    vTaskDelay(pdMS_TO_TICKS(20));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SSD1306_DISPLAY_ADDR,
        .scl_speed_hz = 100000,
    };

    // add device to I2C bus
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus_handle, &dev_config, ssd1306_ret_handle), 
                        TAG, "Failed to add display to I2C bus");

    // send initialization command sequence
    ESP_RETURN_ON_ERROR(ssd1306_setup(*ssd1306_ret_handle), TAG, "Failed to initialize SSD1306 display");

    return ESP_OK;
}

// Data Writing Functions

esp_err_t ssd1306_draw_pixel(uint8_t x, uint8_t y, bool color) {

    ESP_RETURN_ON_FALSE((x < 128) && (y < 64), ESP_ERR_INVALID_ARG, TAG, "Coordinates out of bounds");

	uint8_t page = y >> 3;
	uint16_t byte_index = (128 * page) + x;
	uint8_t bit_mask = (1U << (y & 0x07));
	if(color) {
		s_framebuffer[byte_index] |= bit_mask;
	} else {
		s_framebuffer[byte_index] &= ~bit_mask;
	}

    return ESP_OK;
}

esp_err_t ssd1306_draw_character(uint8_t x_start, uint8_t y_start, char character) {

    if (character < 32 || character > 127) {
        character = ' '; // replace invalid characters with a space
    }

	const uint8_t *p = font_bitmap[character - 32];
	for (uint8_t col = 0; col < 8; col++) {
		uint8_t byte = *(p+col);
		for(uint8_t row = 0; row < 8; row++) {
			uint8_t bit = (byte & (1U << row)) >> row;
			ssd1306_draw_pixel(x_start + col, y_start + row, bit);
		}
	}

    return ESP_OK;
}

esp_err_t ssd1306_draw_string(const char *text) {
    ESP_RETURN_ON_FALSE(text != NULL, ESP_ERR_INVALID_ARG, TAG, "No text provided");
	
    // get the number of lines
	uint8_t lf_count = 0;
    const char *ptr = text;
    while(*ptr != '\0') {
        if (*ptr == '\n') lf_count++;
        ptr++;
    }

    uint8_t line_count = lf_count + 1;

    ESP_RETURN_ON_FALSE(line_count <= 8, ESP_ERR_INVALID_ARG, TAG, "Cannot display more than eight lines");

    // center the lines and align them with physical pages
    uint8_t start_page = (8 - line_count) / 2;
    uint8_t y_start = start_page * 8;

    // display lines one by one
	const char *str_pos = text;

	for(uint8_t j = 0; j < line_count; j++) {

        // get the number of characters in the current line
		uint8_t char_count = 0;
        const char *line_pos = str_pos;
		while (*line_pos != '\0' && *line_pos != '\n') {
			line_pos++;
			char_count++;
			ESP_RETURN_ON_FALSE(char_count <= 16, ESP_ERR_INVALID_ARG, TAG, "A line cannot contain more than 16 characters");
		}

        uint8_t line_width = char_count * 8;
        uint8_t x_start = (128 - line_width) / 2;

        // write single characters one by one
        for (uint8_t i = 0; i < char_count; i++) {
            ssd1306_draw_character(x_start + (i * 8), y_start, str_pos[i]);
        }

        // move onto the next line
        y_start += 8;
		if (*line_pos == '\n') {
            str_pos = line_pos + 1;
        } else {
            break;
        }
	}

    return ESP_OK;
}

esp_err_t ssd1306_update(ssd1306_dev_handle_t ssd1306_handle) {
    ESP_RETURN_ON_FALSE(ssd1306_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Null pointer passed as handle");

    uint8_t address_cmd[] = {
        SSD1306_CONTROL_BYTE_CMD_STREAM,
        SSD1306_CMD_SET_COLUMN_ADDRESS,
        0x00,
        0x7F,
        SSD1306_CMD_SET_PAGE_ADDRESS,
        0x00,
        0x07
    };

    ESP_RETURN_ON_ERROR(i2c_master_transmit(ssd1306_handle, address_cmd, sizeof(address_cmd), I2C_TIMEOUT_MS), 
                        TAG, "Failed to set column and page start and end addresses");

    uint8_t data_buffer[1025] = {SSD1306_CONTROL_BYTE_DATA_STREAM};
    memcpy(&data_buffer[1], s_framebuffer, sizeof(s_framebuffer));
    ESP_RETURN_ON_ERROR(i2c_master_transmit(ssd1306_handle, data_buffer, sizeof(data_buffer), I2C_TIMEOUT_MS), 
                        TAG, "Failed to update framebuffer");

    return ESP_OK;
}

esp_err_t ssd1306_clear(void) {
    memset(s_framebuffer, 0x00, sizeof(s_framebuffer));
    return ESP_OK;
}