#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <display.h>

void * displayInfo(void* arg)
{
    displayToScreen();

    return NULL;
}

void displayToScreen(void)
{
	printf("Hello"); 
}

int Display() {
    int fd;

    // Initialize WiringPi I2C interface
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed.\n");
        return -1;
    }

    // Connect to the OLED display via I2C
    fd = wiringPiI2CSetup(OLED_ADDR);
    if (fd == -1) {
        printf("Failed to initialize I2C communication.\n");
        return -1;
    }

    // Initialize the OLED display
    oled_init(fd);

    // Clear the display
    oled_clear(fd);

    // Display text
    oled_print_text(fd, "Hello, Raspberry Pi!");

    // Keep the display on for a while
    sleep(5);

    // Clear display before exit
    oled_clear(fd);

    return 0;
}

void oled_init(int fd) 
{
    oled_send_command(fd, OLED_DISPLAYOFF);
    oled_send_command(fd, OLED_SETDISPLAYCLOCKDIV);
    oled_send_command(fd, 0x80);
    oled_send_command(fd, OLED_SETMULTIPLEX);
    oled_send_command(fd, 0x3F);
    oled_send_command(fd, OLED_SETDISPLAYOFFSET);
    oled_send_command(fd, 0x00);
    oled_send_command(fd, OLED_SETSTARTLINE | 0x00);
    oled_send_command(fd, OLED_CHARGEPUMP);
    oled_send_command(fd, 0x14);
    oled_send_command(fd, OLED_MEMORYMODE);
    oled_send_command(fd, 0x00);
    oled_send_command(fd, OLED_SEGREMAP | 0x1);
    oled_send_command(fd, OLED_COMSCANDEC);
    oled_send_command(fd, OLED_SETCOMPINS);
    oled_send_command(fd, 0x12);
    oled_send_command(fd, OLED_SETCONTRAST);
    oled_send_command(fd, 0xCF);
    oled_send_command(fd, OLED_SETPRECHARGE);
    oled_send_command(fd, 0xF1);
    oled_send_command(fd, OLED_SETVCOMDETECT);
    oled_send_command(fd, 0x40);
    oled_send_command(fd, OLED_DISPLAYALLON_RESUME);
    oled_send_command(fd, OLED_NORMALDISPLAY);
    oled_send_command(fd, OLED_DISPLAYON);
}

void oled_clear(int fd) {
    for (int i = 0; i < (OLED_WIDTH * OLED_HEIGHT / 8); i++) 
    {
        oled_send_data(fd, 0x00);
    }
}

void oled_send_command(int fd, uint8_t command) 
{
    wiringPiI2CWriteReg8(fd, OLED_CMD, command);
}

void oled_send_data(int fd, uint8_t data) 
{
    wiringPiI2CWriteReg8(fd, OLED_DATA, data);
}

void oled_print_text(int fd, const char* text) 
{
    while (*text) 
    {
        oled_send_data(fd, *text++);
    }
}

