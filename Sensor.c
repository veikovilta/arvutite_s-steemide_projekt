#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>
#include <time.h>

#define GPIO_PIN 22 // GPIO pin number (BCM) for the input from the LED
#define BLINK_COUNT 20 // Number of LED blinks to read

int main(void) {
    // Initialize pigpio
    if (gpioInitialise() < 0) {
        printf("Failed to initialize pigpio\n");
        return 1;
    }

    // Set GPIO_PIN as input
    gpioSetMode(GPIO_PIN, PI_INPUT);
    struct timespec timestamps[BLINK_COUNT];
    int pin = 0;

    // Wait for the first blink
    while (gpioRead(GPIO_PIN) == 0) {
        usleep(100000); // Check every 100 milliseconds
    }

    // Record the time of the first blink
    clock_gettime(CLOCK_REALTIME, &startAndEndStamp[0]);
    pin = 1;

    // Wait until the next full minute
    time_t currentTime;
    struct tm *timeInfo;

    while (1) {
        // Get the current time
        time(&currentTime);
        timeInfo = localtime(&currentTime);

        // Check if seconds are 0, indicating the start of a new minute
        if (timeInfo->tm_sec == 0) {
            break; // Exit the loop when the next full minute starts
        }
        usleep(100000); // Check every 100 milliseconds
    }

    
    
    // Read the LED blinks and record timestamps
    for (int i = 0; i < BLINK_COUNT; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpioRead(GPIO_PIN) == 0) {
            usleep(100000); // Check every 100 milliseconds
        }

        // Get current time with high precision
        clock_gettime(CLOCK_REALTIME, &timestamps[i]);
        if (i == (BLINK_COUNT-1) )
        {
            clock_gettime(CLOCK_REALTIME, &startAndEndStamp[1]);
        }
        // Wait for the GPIO pin to go LOW
        while (gpioRead(GPIO_PIN) == 1) {
            usleep(100000); // Check every 100 milliseconds
        }
    }

    // Write timestamps to a file
    FILE *file = fopen("timestamps.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        gpioTerminate();
        return 1;
    }

    for (int i = 0; i < BLINK_COUNT; i++) {
        fprintf(file, "%ld.%09ld\n", timestamps[i].tv_sec, timestamps[i].tv_nsec);
    }
    
    //initial start and ending time, for debugging how long the program takes
    fprintf(file, "Start Time: %ld.%09ld\n", startAndEndStamp[0].tv_sec, startAndEndStamp[0].tv_nsec);
    fprintf(file, "End Time: %ld.%09ld\n", startAndEndStamp[1].tv_sec, startAndEndStamp[1].tv_nsec);

    fclose(file);
    
    // Cleanup
    gpioTerminate();
    return 0;
}
