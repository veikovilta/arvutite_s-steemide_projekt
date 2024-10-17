#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "HelperFunctions.h"
#include <gpiod.h>
#include <string.h>
#include "display.h"


struct port* openPort(int portPin, char* debugName, bool inputOutput) {
    struct port* newPort = (struct port*) malloc(sizeof(struct port));
    if (newPort == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Open GPIO chip
    newPort->chip = gpiod_chip_open_by_number(portPin);
    if (!newPort->chip) {
        perror("Open GPIO chip failed");
        free(newPort);
        return NULL;
    }

    // Get GPIO line
    newPort->line = gpiod_chip_get_line(newPort->chip, portPin);
    if (!newPort->line) {
        perror("Get GPIO line failed");
        gpiod_chip_close(newPort->chip);
        free(newPort);
        return NULL;
    }

    // Request line as output
    if (inputOutput)
	{
		int lineRequestReturn = gpiod_line_request_input(newPort->line, debugName);
		if (lineRequestReturn < 0) {
			perror("Request line as output failed");
			gpiod_chip_close(newPort->chip);
			free(newPort);
			return NULL;
		}
	}
	else
	{
		int lineRequestReturn = gpiod_line_request_output(newPort->line, debugName, 0);
		if (lineRequestReturn < 0) {
			perror("Request line as output failed");
			gpiod_chip_close(newPort->chip);
			free(newPort);
			return NULL;
		}
	}

    return newPort;
}


void preciseSleep(double seconds) {
    struct timespec req, rem;

    // Break down the seconds into whole seconds and nanoseconds
    req.tv_sec = (time_t)seconds;                     // Get the whole seconds part
    req.tv_nsec = (long)((seconds - req.tv_sec) * 1e9); // Convert the fractional part to nanoseconds

    int ret = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);

    if (ret != 0) {
        if (ret == EINTR) {
            // Interrupted by a signal handler, display remaining time
            printf("Sleep interrupted. Remaining: %ld seconds and %ld nanoseconds\n", rem.tv_sec, rem.tv_nsec);
        } else {
            // Handle other potential errors
            perror("clock_nanosleep failed");
        }
    }
}

//fix it add debouncing and say when to use
void* readButtonState_thread(void* arg) {
    struct args_port* args = (struct args_port*) arg;
    struct port *openedPort = openPort(args->portPin, args->debugName, args->inputOutput);

    if (openedPort == NULL) {
        return NULL;
    }

    while (1) {
        int value = gpiod_line_get_value(openedPort->line);
        if (value < 0) {
            perror("Failed to read button state");
            break; // Exit the loop on error
        }

        if (value == 1) {
            printf("Button is pressed!\n");
        } else {
            printf("Button is not pressed.\n");
        }

        usleep(200000);
    }

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return NULL;
}

int debounceButtonState(struct args_port* args) {
    
    struct port *openedPort = openPort(args->portPin, args->debugName, args->inputOutput);

    if (openedPort == NULL) {
        fprintf(stderr, "Failed to open port.\n");
        return -1;
    }

    int stableCount = 0;
    const int debounceThreshold = 3; // Require 3 stable reads
    const int delayMs = 50; // 50 ms delay between reads
    int value = 0;

    while(1) {	
        value = gpiod_line_get_value(openedPort->line);
        if (value < 0) {
            perror("Failed to read button state");
            break; // Exit the loop on error
        }

        if (value == 1) {
            stableCount++;
        } else {
            stableCount = 0;
        }

        if (stableCount >= debounceThreshold) {
            printf("Button is pressed!\n");
            break;
        } else {
            printf("Button is not pressed.\n");
        }

        usleep(delayMs * 1000); // 50 ms delay
    }

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return value;
}


void ShowReady(void)
{
    struct args_port* args = (struct args_port*) args;
    struct port *openedPort = openPort(GPIO_READY_LED, "GPIO PIN 23", true);

    //display-ime 1 minut
    gpiod_line_set_value(openedPort->line, 1);
    preciseSleep(60);
}

int CheckSync(int i2cHandle)
{
    FILE *fp;
    double systemOffset = 0.0;
    char buffer[200];
    char numberStr[20] = "";
    char message[100] = ""; 

    // Run the "chronyc tracking" command and open a pipe to read the output
    fp = popen("chronyc tracking", "r");
    if (fp == NULL) 
    {
        oledClear(i2cHandle); // Clear the display
        oledWriteText(i2cHandle, 0, 0, "Failed to run chronyc command.");
        oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        return 1;
    }

    // Parse the output line by line
    while (fgets(buffer, sizeof(buffer), fp) != NULL) 
    {
        // Look for the line that contains "System time" to get the offset
        if (strstr(buffer, "System time") != NULL) 
        {
            // Extract the offset value (it will be the second value in the line)
            sscanf(buffer, "System time     : %lf seconds", &systemOffset);
            break;
        }
    }

    // Close the pipe
    pclose(fp);

    // Output the system offset to the user
    sprintf(numberStr, "%.9f", systemOffset);
    snprintf(message, sizeof(message), "System time offset: %9s", numberStr);


    
    oledClear(i2cHandle);
    // Display a message on the OLED
    oledWriteText(i2cHandle, 0, 0, message);

    //printf("System time offset: %.9f \n", systemOffset);

    // Check synchronization status
    // piiriks 0.1 ms
    if (systemOffset < 0.0001 && systemOffset > -0.0001) 
    {
        oledWriteText(i2cHandle, 2, 0, "System clock is synchronized");
        return 0;
    } 
    else 
    {
        oledWriteText(i2cHandle, 2, 0, "System clock is not synchronized");
    }

    return 1;
}
