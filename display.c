#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <gpiod.h>
#include "display.h"

// Function to initialize I2C communication
int i2cInit(const char *device, int addr) {
    int file = open(device, O_RDWR);
    if (file < 0) {
        perror("Failed to open I2C device");
        return -1;
    }

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        perror("Failed to set I2C address");
        close(file);
        return -1;
    }

    return file;
}

// Write command to OLED
void oledWriteCommand(int i2cHandle, uint8_t command) {
    uint8_t buffer[2] = {OLED_COMMAND, command};
    write(i2cHandle, buffer, 2);
}

// Write data to OLED
void oledWriteData(int i2cHandle, uint8_t data) {
    uint8_t buffer[2] = {OLED_DATA, data};
    write(i2cHandle, buffer, 2);
}

// Initialize OLED display
void oledInit(int i2cHandle) {
    oledWriteCommand(i2cHandle, 0xAE); // Display OFF
    oledWriteCommand(i2cHandle, 0xD5); // Set display clock
    oledWriteCommand(i2cHandle, 0x80);
    oledWriteCommand(i2cHandle, 0xA8); // Set multiplex
    oledWriteCommand(i2cHandle, 0x3F);
    oledWriteCommand(i2cHandle, 0xD3); // Display offset
    oledWriteCommand(i2cHandle, 0x00);
    oledWriteCommand(i2cHandle, 0x40); // Set start line
    oledWriteCommand(i2cHandle, 0x8D); // Charge pump
    oledWriteCommand(i2cHandle, 0x14);
    oledWriteCommand(i2cHandle, 0xA1); // Segment re-map
    oledWriteCommand(i2cHandle, 0xC8); // COM scan direction
    oledWriteCommand(i2cHandle, 0xDA); // COM hardware
    oledWriteCommand(i2cHandle, 0x12);
    oledWriteCommand(i2cHandle, 0x81); // Contrast
    oledWriteCommand(i2cHandle, 0xCF);
    oledWriteCommand(i2cHandle, 0xD9); // Pre-charge period
    oledWriteCommand(i2cHandle, 0xF1);
    oledWriteCommand(i2cHandle, 0xDB); // VCOMH
    oledWriteCommand(i2cHandle, 0x40);
    oledWriteCommand(i2cHandle, 0xA4); // Disable display on
    oledWriteCommand(i2cHandle, 0xA6); // Normal display
    oledWriteCommand(i2cHandle, 0xAF); // Display ON
}

// Set OLED cursor
void oledSetCursor(int i2cHandle, int x, int y) {
    oledWriteCommand(i2cHandle, 0xB0 + y); // Set page start address
    oledWriteCommand(i2cHandle, 0x00 + (x & 0x0F)); // Lower column address
    oledWriteCommand(i2cHandle, 0x10 + ((x >> 4) & 0x0F)); // Higher column address
}

// Clear OLED display
void oledClear(int i2cHandle) {
    for (int page = 0; page < 8; page++) {
        oledSetCursor(i2cHandle, 0, page);
        for (int col = 0; col < OLED_WIDTH; col++) {
            oledWriteData(i2cHandle, 0x00); // Clear
        }
    }
}

// Write a character to the OLED
void oledWriteChar(int i2cHandle, char ch) {
    if (ch < 32 || ch > 127) ch = ' '; // Replace non-printable characters with space
    const unsigned char *bitmap = font5x7[ch - 32]; // Get character bitmap

    for (int i = 0; i < 5; i++) {
        oledWriteData(i2cHandle, bitmap[i]); // Write each column of the character
    }
    oledWriteData(i2cHandle, 0x00); // Space between characters
}

// Write a string to the OLED
void oledWriteText(int i2cHandle, int x, int y, const char *text) {
    oledSetCursor(i2cHandle, x, y); // Set start position

    while (*text) {
        oledWriteChar(i2cHandle, *text++); // Write characters
    }
}

