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

#include <stdbool.h>

#define OLED_BUFFER_SIZE 100
// Basic font (5x7)
extern const unsigned char font5x7[][5];

static pthread_t oledThread;
extern pthread_mutex_t global_mutex;

// for storing data which will be displayed on the OLED
struct oled {
    char oledBuffer[OLED_BUFFER_SIZE];
    int x;
    int y; 
    bool clean;
};

extern struct oled oled;

extern char systemState[OLED_BUFFER_SIZE];

// Function prototypes
/**
 * @brief Initializes I2C communication with the OLED display.
 *
 * Opens the I2C device and sets the slave address.
 *
 * @param device The I2C device file path.
 * @param addr The I2C address of the OLED display.
 * @return File descriptor for the I2C device, or -1 on failure.
 */
int i2cInit(const char *device, int addr);

/**
 * @brief Sends a command byte to the OLED display.
 *
 * @param i2cHandle The I2C file descriptor.
 * @param command The command byte to send.
 */
void oledWriteCommand(int i2cHandle, uint8_t command);

/**
 * @brief Sends a data byte to the OLED display.
 *
 * @param i2cHandle The I2C file descriptor.
 * @param data The data byte to send.
 */
void oledWriteData(int i2cHandle, uint8_t data);

/**
 * @brief Initializes the OLED display with required settings.
 *
 * Sends a sequence of commands to configure the OLED display.
 *
 * @param i2cHandle The I2C file descriptor.
 */
void oledInit(int i2cHandle);

/**
 * @brief Sets the cursor position on the OLED display.
 *
 * @param i2cHandle The I2C file descriptor.
 * @param x The x-coordinate (column).
 * @param y The y-coordinate (page).
 */
void oledSetCursor(int i2cHandle, int x, int y);

/**
 * @brief Clears the entire OLED display.
 *
 * Writes zeros to all display memory to clear the screen.
 *
 * @param i2cHandle The I2C file descriptor.
 */
void oledClear(int i2cHandle);
void oledWriteChar(int i2cHandle, char ch);
void oledWriteText(int i2cHandle, int x, int y, const char *text);

/**
 * @brief Thread function to handle OLED display updates.
 *
 * Initializes the OLED display and continuously checks for new messages to display.
 * Handles clearing, writing text, and closing the display when needed.
 *
 * @param arg Unused.
 * @return Always returns NULL or -1 on error.
 */
void* oled_thread(void* arg); 

/**
 * @brief Sets the message to be displayed on the OLED screen.
 *
 * Copies the given message to the OLED buffer and sets the display coordinates and clean flag.
 * Uses a mutex to ensure thread safety.
 *
 * @param message The message to display.
 * @param x The x-coordinate for the message.
 * @param y The y-coordinate for the message.
 * @param clean If true, the display will be cleared before showing the message.
 */
void SetOledMessage(const char* message, int x, int y, bool clean);

/**
 * @brief Sets the system state message for the OLED display.
 *
 * Copies the new state string to the systemState buffer.
 * Uses a mutex to ensure thread safety.
 *
 * @param newState The new system state message.
 */
void SetSystemState(const char* newState);

#endif
