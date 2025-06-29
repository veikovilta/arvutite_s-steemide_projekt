#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <gpiod.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "display.h"
#include "HelperFunctions.h"

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
struct oled oled = {
    .oledBuffer = "",
    .x = 0,
    .y = 0,
    .clean = false
};

char systemState[OLED_BUFFER_SIZE] = "";

void SetOledMessage(const char* message, int x, int y, bool clean)
{
    pthread_mutex_lock(&global_mutex);
    strncpy(oled.oledBuffer, message, OLED_BUFFER_SIZE);
    oled.x = x;
    oled.y = y;
    oled.clean = clean; 
    pthread_mutex_unlock(&global_mutex);
}

void SetSystemState(const char* newState)
{
    pthread_mutex_lock(&global_mutex);
    strncpy(systemState, newState, OLED_BUFFER_SIZE);
    pthread_mutex_unlock(&global_mutex);
}

void* oled_thread(void* arg)
{
    int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
	if (i2cHandle < 0) return -1;
	
	oledInit(i2cHandle);

	while (programRunning)
	{	
		pthread_mutex_lock(&global_mutex);
		//printf(":%s\n", oledBuffer);
		if(strcmp("", oled.oledBuffer) != 0)
		{
			if(oled.clean)
            {
                oledClear(i2cHandle);
            }
			
			if(strcmp(" ", oled.oledBuffer) == 0)
			{
				if (i2cHandle){
					close(i2cHandle);
                    systemState[0] = '\0';
				}
                break;
			}
			
            oledWriteText(i2cHandle, oled.x, oled.y, oled.oledBuffer);
            oledWriteText(i2cHandle, 0, 7, systemState);  
            printf("%s\n", oled.oledBuffer);
			oled.oledBuffer[0] = '\0'; 
		}
		pthread_mutex_unlock(&global_mutex);
		preciseSleep(0.1);
	}
}

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
    oledWriteCommand(i2cHandle, 0xB0 + (uint8_t)y); // Set page start address
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
void oledWriteText(int i2cHandle, int x, int y, const char *text) 
{
    oledSetCursor(i2cHandle, x, y); // Set start position

    while (*text) 
    {
        oledWriteChar(i2cHandle, *text++); // Write characters
    }
}


const unsigned char font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // (space)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // (backslash)
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x03, 0x05, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x08, 0x14, 0x54, 0x54, 0x3C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x40, 0x44, 0x3D}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x08, 0x08, 0x2A, 0x1C, 0x08}, // ->
    {0x08, 0x1C, 0x2A, 0x08, 0x08}, // <-
};
