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
//#include <linux/time.h>


struct port* openPort(int lineNumber, char* debugName, bool inputOutput) {
    struct port* newPort = (struct port*) malloc(sizeof(struct port));
    if (newPort == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Open GPIO chip by number (e.g., 0 for /dev/gpiochip0)
    newPort->chip = gpiod_chip_open(GPIO_CHIP);
    if (!newPort->chip) {
        perror("Open GPIO chip failed");
        free(newPort);
        return NULL;
    }

    // Get GPIO line
    newPort->line = gpiod_chip_get_line(newPort->chip, (unsigned)lineNumber);
    if (!newPort->line) {
        perror("Get GPIO line failed");
        gpiod_chip_close(newPort->chip);
        free(newPort);
        return NULL;
    }

    // Request line based on inputOutput flag
    int lineRequestReturn;
    if (inputOutput) {
        lineRequestReturn = gpiod_line_request_input(newPort->line, debugName);
    } else {
        lineRequestReturn = gpiod_line_request_output(newPort->line, debugName, 0);
    }

    if (lineRequestReturn < 0) {
        perror("Request line failed");
        gpiod_chip_close(newPort->chip);
        free(newPort);
        return NULL;
    }

    return newPort;
}


void preciseSleep(double seconds) {
    struct timespec req, rem;

    // Break down the seconds into whole seconds and nanoseconds
    req.tv_sec = (time_t)seconds; // Get the whole seconds part
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
        fprintf(stderr, "Failed to open port.\n");
        return NULL;
    }

    int stableCount = 0;
    const int debounceThreshold = 3; // Require 3 stable reads
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
            pthread_mutex_lock(&buttonMutex); // Lock the mutex before modifying the shared variable
            buttonPressed = 1; // Set buttonPressed to 1 (pressed)
            pthread_mutex_unlock(&buttonMutex); // Unlock the mutex
            break; // Exit the loop when the button is pressed
        } else {
            //printf("Button is not pressed.\n");
        }

        preciseSleep(0.05); // 50 ms delay
    }

    ClosePort(openedPort);

    return NULL;
}

void ClosePort(struct port* openedPort)
{
    gpiod_line_set_value(openedPort->line, 0);
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);
}

struct port* ShowReady(void)
{  
    struct port *openedPort = openPort(GPIO_READY_LED, "GPIO PIN 23", false);

    if (openedPort == NULL) {
        return;
    }
    // Display LED
    gpiod_line_set_value(openedPort->line, 1);

    return openedPort; 
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
        oledWriteText(i2cHandle, 0, 2, "Shutting Down");
        printf("Error with chronyc, shutting down\n");
        //system ("sudo shutdown -h now");
        return 1;
    }

    // Parse the output line by line
    while (fgets(buffer, sizeof(buffer), fp) != NULL) 
    {
        // Look for the line that contains "System time" to get the offset
        if (strstr(buffer, "System time") != NULL) 
        {
             // Extract the offset value (it will be the second value in the line)
            if (sscanf(buffer, "System time     : %lf seconds", &systemOffset) != 1) {
                fprintf(stderr, "Failed to parse system offset.\n");
                break;
            }
            break;
        }
    }

    // Close the pipe
    if (pclose(fp) == -1) {
        perror("pclose failed");
        return 1;
    }

    // Output the system offset to the user
    sprintf(numberStr, "%.7f", systemOffset);
    snprintf(message, sizeof(message), "Offset: %7s", numberStr);
    
    oledClear(i2cHandle);
    // Display a message on the OLED
    oledWriteText(i2cHandle, 0, 0, message);

    // Check synchronization status
    // piiriks 0.1 ms
    if (systemOffset < 0.0001 && systemOffset > -0.0001) 
    {
        oledWriteText(i2cHandle, 0, 2, "Clock is in sync");
        return 0;
    } 
    else 
    {
        oledWriteText(i2cHandle, 0, 2, "Clock is not in sync");
    }

    return 1;
}

void printDelaysToFile(const char *filename, double *data, int count, double averageDelay)
{
    FILE *file = fopen(filename, "w"); // Open file for writing

    if (file == NULL) {
        perror("Error opening file"); // Handle file opening error
        return;
    }

    // Print each delay value to the file
    fprintf(file, "Delays:\n");
    for (int i = 0; i < count; i++) {
        fprintf(file, "Delay %d: %.2f\n", i + 1, data[i]);
    }

    // Print the average delay to the file
    fprintf(file, "\nAverage Delay: %.2f\n", averageDelay); // Print average to file

    fclose(file); // Close the file
}

const char* checkButtonState(struct port* port1, struct port* port2) {
    
    int state1 = gpiod_line_get_value(port1->line);
    int state2 = gpiod_line_get_value(port2->line);

    if (state1 < 0 || state2 < 0) {
        perror("Failed to read GPIO line value");
        return "error";
    }

    // Determine the button state
    if (state1 == 1 && state2 == 0) {
        return "saatja";  // Button pressed for "saatja"
    } else if (state1 == 0 && state2 == 1) {
        return "vastuvotja";  // Button pressed for "vastuvotja"
    } else {
        return "undefined";  // Undefined state
    }
}

const char* waitForButtonState() {
    
    struct port1* port1 = openPort(0, "Port 1", true);  // Pin for saatja
    struct port2* port2 = openPort(1, "Port 2", true);  // Pin for vastuvotja
    
    const char* state = checkButtonState(port1, port2);
    printf("Button state: %s\n", state);
    usleep(500000);  // Delay for readability (500 ms)

    return state;
}

int ChronySync(int i2cHandle)
{
    // message string
    char message[100] = "";  
    char numberStr[20] = "";

    // teeb 60 sekundilist checki kui hea kell on
    // 10min vähemalt
    int minutes = 0;
    while(1)
    {
        preciseSleep(5);
    
        if (CheckSync(i2cHandle) == 0)
        {
            break;
        }
        else 
        {
            // kirjuta ekraanile et oodatud 5 sek
            sprintf(numberStr, "%d", minutes+5);
            snprintf(message, sizeof(message), "seconds waited : %s", numberStr);
            printf("%s\n", message);
            oledWriteText(i2cHandle, 0, 0, message);
        }
        
        minutes+=5;

        // kui ei ole syncis 10 mintaga siis error
        if (minutes == 120)
        {
            oledClear(i2cHandle);
            // Display a message on the OLED
            oledWriteText(i2cHandle, 0, 0, "NOT SYNCED");
            oledWriteText(i2cHandle, 2, 0, "ERROR BAD RECEPTION");
            oledWriteText(i2cHandle, 4, 0, "Shutting Down");
        
            if (system ("sudo shutdown -h now") != 0) {
                perror("Failed to shutdown");
                oledClear(i2cHandle);
                oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
                // Handle the error or exit
            }
            return 1;
        }
        
    }
    
    printf("Syncronized\n");    


    return 0;
}