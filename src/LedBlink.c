#include <gpiod.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gpiod.h>
#include "Files.h"
#include <time.h>
#include "display.h"

void ledBlinkingCalibration(int blinkCount)
{
    SetSystemState("CALIBRATE-->BLINKER");
    struct port *openedPort = openPort(GPIO_LINE_MAIN_BLINK, "debug", false); // 'true' for output

    SetOledMessage("Calibration started", 0, 0, true);
    preciseSleep(0.5);
    SetOledMessage("Press button to stop", 0, 2, false);

    for (int i = 0; i < blinkCount; i++) {
        // Turn the LED on
        gpiod_line_set_value(openedPort->line, 1);
        
        // Sleep for 1 second
        preciseSleep(0.25); // Sleep for 1 second
        
        // Turn the LED off
        gpiod_line_set_value(openedPort->line, 0);
        
        // Sleep for 1 second
        preciseSleep(1.75); // Sleep for 1 second

        if (IsButtonPressed())
        {
            SetOledMessage("Calibration stopped", 0, 0, true);
            break;
        }
        
    }
    
    // Clean up
    ClosePort(openedPort);
    SetSystemState("CALIBRATE-->MODE MENU");
}

void ledBlinking20(struct args_port* args, char** buffer)
{
    struct port *openedPort = openPort(args->portPin, args->debugName, false); // 'true' for output

    // Array to store timestamps (local variable inside the loop)
    struct timespec blinkTimes[BLINK_COUNT_MAIN_PROGRAM];
    struct timespec ts;
    
    for (int i = 0; i < BLINK_COUNT_MAIN_PROGRAM; i++) {
        // Turn the LED on
        gpiod_line_set_value(openedPort->line, 1);
        
        // Get the current timestamp and store it in the array at index i
        clock_gettime(CLOCK_REALTIME, &ts);  // Get current time
        blinkTimes[i] = ts;
    
        // Sleep for 1 second
        preciseSleep(0.25); // Sleep for 1 second
        
        // Turn the LED off
        gpiod_line_set_value(openedPort->line, 0);
        
        // Sleep for 1 second
        preciseSleep(1.75); // Sleep for 1 second
    }
    
    for (int i = 0; i < BLINK_COUNT_MAIN_PROGRAM; i++)
    {
        TimeStampToBufferWithTime(buffer, "BSELECT link: ", blinkTimes[i]);
    }
    
    // Clean up
    ClosePort(openedPort);
}

struct timespec ledBlinkOnce(struct args_port *newPort, char** buffer) {
    // Open the port for controlling the LED
    struct port *openedPort = openPort(newPort->portPin, newPort->debugName, false); // 'FALSE' for output
    static struct timespec blinkTime; // Use static to return a pointer safely

    // Turn the LED on
    gpiod_line_set_value(openedPort->line, 1);
    clock_gettime(CLOCK_REALTIME, &blinkTime);
    TimeStampToBuffer(buffer, "First sync blink: ");
    preciseSleep(1);  // Sleep for 1 second

    // Turn the LED off
    gpiod_line_set_value(openedPort->line, 0);

    ClosePort(openedPort); 
    
    return blinkTime; // Return the address of blinkTime
}
