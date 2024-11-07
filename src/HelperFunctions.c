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
#include "State.h"
#include "Main.h"


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

    pthread_cleanup_push((void(*)(void*))ClosePort, openedPort);


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
		//printf("PIP\n");
        } else {
            stableCount = 0;
        }

        if (stableCount >= debounceThreshold) {
            printf("Button is pressed!\n");
            pthread_mutex_lock(&buttonLock);
            buttonPressed = 1;
            pthread_mutex_unlock(&buttonLock); 
        } else {
            //printf("Button is not pressed.\n");
        }

        preciseSleep(0.05); // 50 ms delay
    }

    pthread_cleanup_pop(1);

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
        return NULL;
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
      	 return 0;
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

const char* waitForButtonState(int port1, int port2) {
    
    struct port* openedPort1 = openPort(port1, "Port 1", true);  // Pin for saatja
    struct port* openedPort2 = openPort(port2, "Port 2", true);  // Pin for vastuvotja
    
    const char* state = checkButtonState(openedPort1, openedPort2);
    //printf("Button state: %s\n", state);
    usleep(500000);  // Delay for readability (500 ms)

    ClosePort(openedPort1);
    ClosePort(openedPort2);

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

void WaitForNextMinuteBlinker(struct timespec firstblink) {
    struct timespec currentTime;

    // Print checking status
    printf("Checking if it's less than 10 sec to the next full minute\n");
    fflush(stdout);
    
    // Calculate if within 10 seconds of the next full minute
    if (60 - (firstblink.tv_sec % 60) <= 10) {
        preciseSleep(11); // Sleep for 11 seconds
    }

    printf("Waiting for the next minute\n");
    fflush(stdout);

    // Loop until the next full minute
    while (1) {
        clock_gettime(CLOCK_REALTIME, &currentTime);

        // Break if the seconds are exactly at the start of a minute
        if ((currentTime.tv_sec % 60) == 0 && currentTime.tv_nsec < 1e6) {
            break;
        }

        preciseSleep(0.0001); // Sleep a short time before rechecking
    }
}

const char* WaitForButtonAndSelectConfig(int i2cHandle) {
    char* saatjaOrVastuvotja;
    char message[100] = "";
    char lastPicked[30];
    // Lock and reset the buttonPressed flag
    pthread_mutex_lock(&buttonLock);
    buttonPressed = 0;
    pthread_mutex_unlock(&buttonLock);
	
    while (1) {

        // Wait for button state and get the selected config
        saatjaOrVastuvotja = waitForButtonState(23, 24);
        sprintf(message, "Selected:%s\n", saatjaOrVastuvotja);

        if (lastPicked[0] == '\0') {
            oledWriteText(i2cHandle, 0, 0, "PRESS BUTTON TO PICK");
            oledWriteText(i2cHandle, 1, 2, message);
        }
        
        if(strcmp(saatjaOrVastuvotja, lastPicked) != 0){
		oledClear(i2cHandle);
            oledWriteText(i2cHandle, 0, 0, "PRESS BUTTON TO PICK");
            oledWriteText(i2cHandle, 1, 2, message);
        }
        
	    strcpy(lastPicked, saatjaOrVastuvotja);
	
        preciseSleep(0.5);

        // Check if button was pressed, exit loop if true
        pthread_mutex_lock(&buttonLock);
        //printf("%d\n", buttonPressed);
        if (buttonPressed) {
            buttonPressed = 0; 
            break;
        }
        pthread_mutex_unlock(&buttonLock); 
    }

    return saatjaOrVastuvotja;
}

int CreateButtonThread(int i2cHandle, pthread_t* buttonThread) {

    struct args_port args;
    args.portPin = GPIO_BUTTON;
    args.debugName = "InputButton";
	args.inputOutput = true;

    if(pthread_create(buttonThread, NULL, readButtonState_thread, (void*)&args) < 0)
    {
        perror("Failed to create thread");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread");
        oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        
        if (system("sudo shutdown -h now") != 0) {
            perror("Failed to shutdown");
            oledClear(i2cHandle);
            oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        }
        return 1;
    }

    printf("Button thread created\n");

    return 0; 
}
