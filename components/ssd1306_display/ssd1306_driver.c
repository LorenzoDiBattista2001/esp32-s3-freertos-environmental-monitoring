#include "esp_check.h"
#include "ssd1306_driver.h"

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
