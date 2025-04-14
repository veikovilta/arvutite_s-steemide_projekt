#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <gpiod.h>

// I2C address for the OLED display
#define OLED_I2C_ADDR 0x3C

// OLED commands (SSD1306)
#define OLED_COMMAND 0x00
#define OLED_DATA 0x40

// Screen dimensions
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Screen dimensions
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// Basic font (5x7)
extern const unsigned char font5x7[][5];

static pthread_t oledThread;

// Function prototypes
int i2cInit(const char *device, int addr);
void oledWriteCommand(int i2cHandle, uint8_t command);
void oledWriteData(int i2cHandle, uint8_t data);
void oledInit(int i2cHandle);
void oledSetCursor(int i2cHandle, int x, int y);
void oledClear(int i2cHandle);
void oledWriteChar(int i2cHandle, char ch);
void oledWriteText(int i2cHandle, int x, int y, const char *text);
void* oled_thread(void* arg); 


#endif
