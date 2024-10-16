#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <gpiod.h>
#include "Sensor.h"
#include "HelperFunctions.h"
#include "display.h"


    struct timespec timestamps[BLINK_COUNT];
    struct timespec startAndEndStamp[2];

void RegisterBlinks()
{
    struct args_port* args = (struct args_port*) args;
    struct port *openedPort = openPort(GPIO_PIN, "GPIO PIN 22", false);

    // Wait for the first blink
    while (gpiod_line_get_value(openedPort->line) == 0) 
    {
        preciseSleep(0.1); 
    }

    // Record the time of the first blink
    clock_gettime(CLOCK_REALTIME, &startAndEndStamp[0]);
    
    struct timespec currentTime;
    // Get the current time with high precision

    clock_gettime(CLOCK_REALTIME, &currentTime);

    // Extract the seconds part of the current time
    long currentSeconds = currentTime.tv_sec % 60;
    long currentNanoseconds = currentTime.tv_nsec;
    long nanosecondsToWait = 0;
    long secondsToWait= 0;
    double totalSecondsToWait = 0;
    // If the seconds are less than 10, wait until the next full minute
    if (currentSeconds < WAIT_TIME_BEFORE_NEXT_MINUTE) 
    {
        // Calculate how many seconds are left until the next full minute
        secondsToWait = 60 + currentSeconds;
        nanosecondsToWait = 1000000000 - currentNanoseconds;
        
        // Adjust the seconds to wait if we are already in the current second
        if (nanosecondsToWait < 1000000000) 
        {
            secondsToWait += 1; // Add a second since we have to wait for the full second
        }
        
        totalSecondsToWait = (double)secondsToWait + ((double)nanosecondsToWait / 1e9);
        
    }
    else 
    {
        nanosecondsToWait = 1000000000 - currentNanoseconds;
        // Adjust the seconds to wait if we are already in the current second
        if (nanosecondsToWait < 1000000000) 
        {
            secondsToWait += 1; // Add a second since we have to wait for the full second
        }   
        totalSecondsToWait = (double)secondsToWait + ((double)nanosecondsToWait / 1e9);
        
    }
    preciseSleep(totalSecondsToWait);
    
    // Read the LED blinks and record timestamps
    for (int i = 0; i < BLINK_COUNT; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) 
        {
             preciseSleep(0.001);
        }

        // Get current time with high precision
        clock_gettime(CLOCK_REALTIME, &timestamps[i]);
        if (i == (BLINK_COUNT-1) )
        {
            clock_gettime(CLOCK_REALTIME, &startAndEndStamp[1]);
        }
        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) ==1) 
        {
            preciseSleep(0.001);
        }
    }

    //Write to the file
    //
    // !! TODO !!!
    //
    // Write timestamps to a file

    //for (int i = 0; i < BLINK_COUNT; i++) {
    //    fprintf(file, "%ld.%09ld\n", timestamps[i].tv_sec, timestamps[i].tv_nsec);
    //}
    //initial start and ending time, for debugging how long the program takes
    //fprintf(file, "Start Time: %ld.%09ld\n", startAndEndStamp[0].tv_sec, startAndEndStamp[0].tv_nsec);
    //fprintf(file, "End Time: %ld.%09ld\n", startAndEndStamp[1].tv_sec, startAndEndStamp[1].tv_nsec);
    //fclose(file);
    

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

}
