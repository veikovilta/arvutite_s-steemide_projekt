#ifndef DISPLAY_H
#define DISPLAY_H

#define OLED_ADDR 0x3C // I2C address of the SSD1306
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Command macros for the OLED display
#define OLED_CMD 0x00
#define OLED_DATA 0x40

// Some basic command defines
#define OLED_DISPLAYOFF 0xAE
#define OLED_DISPLAYON  0xAF
#define OLED_SETDISPLAYCLOCKDIV 0xD5
#define OLED_SETMULTIPLEX 0xA8
#define OLED_SETDISPLAYOFFSET 0xD3
#define OLED_SETSTARTLINE 0x40
#define OLED_CHARGEPUMP 0x8D
#define OLED_MEMORYMODE 0x20
#define OLED_SEGREMAP 0xA1
#define OLED_COMSCANDEC 0xC8
#define OLED_SETCOMPINS 0xDA
#define OLED_SETCONTRAST 0x81
#define OLED_SETPRECHARGE 0xD9
#define OLED_SETVCOMDETECT 0xDB
#define OLED_DISPLAYALLON_RESUME 0xA4
#define OLED_NORMALDISPLAY 0xA6

void * displayInfo(void* arg);
void displayToScreen(void);
// Function prototypes
int Display()
void oled_init(int fd);
void oled_clear(int fd);
void oled_send_command(int fd, uint8_t command);
void oled_send_data(int fd, uint8_t data);
void oled_print_text(int fd, const char* text);

#endif
