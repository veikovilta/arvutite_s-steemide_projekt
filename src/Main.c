#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <gpiod.h> 
#include "stdbool.h"
#include "HelperFunctions.h"
#include "display.h"
#include <string.h>
#include "Sensor.h"
#include "LedBlink.h"
#include "Main.h"


int main(void)
{
    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
    }

    int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
    if (i2cHandle < 0) return -1; // Exit if failed
    // message string
    char message[100] = "";  
    char numberStr[20] = "";

    oledInit(i2cHandle); // Initialize the OLED
    oledClear(i2cHandle); // Clear the display
    oledWriteText(i2cHandle, 0, 0, "Program started");
    
    // eraldi thread enne käima mis checkib nuppu
    pthread_t buttonThread;
    struct args_port args;

    pthread_mutex_init(&buttonMutex, NULL);

    // Initialize thread arguments
    args.portPin = GPIO_BUTTON; // Set GPIO port number
    args.debugName = "InputButton";  // Set debug name
	args.inputOutput = true;

    if(pthread_create(&buttonThread, NULL, readButtonState_thread, (void*)&args) < 0)
    {
        perror("Failed to create thread");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread");
        oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        
        if (system("sudo shutdown -h now") != 0) {
            perror("Failed to shutdown");
            oledClear(i2cHandle);
            oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
            // Handle the error or exit
        }
        return 1;
    } 


    pthread_mutex_lock(&buttonMutex); // Lock the mutex before modifying the shared variable
    buttonPressed = 0; // Set buttonPressed to 1 (pressed)
    pthread_mutex_unlock(&buttonMutex); // Unlock the mutex
    
    const char* saatjaOrVastuvotja = waitForButtonState(27, 22);
    struct port* syncLedOpenedPort = NULL;

    int synced = ChronySync(i2cHandle); 

    if(!synced){
        syncLedOpenedPort = ShowReady();
    }

    printf("You have chosen: %s\n", saatjaOrVastuvotja); 

    if(!strcmp(saatjaOrVastuvotja, (const char*)"saatja"))
    {
        
        printf("Starting: %s\n", "SAATJA"); 
        fflush(stdout);

        struct args_port ledBlinkPort; 

        ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK; 
        ledBlinkPort.debugName = "ledBlink";
        ledBlinkPort.inputOutput = false;

        struct timespec firstblink = ledBlinkOnce(&ledBlinkPort);

        // Print the time
        printf("LED blink time: %ld seconds and %ld nanoseconds\n", 
            (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
        fflush(stdout);
        
        WaitForNextMinuteBlinker(firstblink); 
        
        printf("Start blinking\n");
        fflush(stdout);
        
        ledBlinking20(&ledBlinkPort);
    }
    else if(!strcmp(saatjaOrVastuvotja, (const char*)"vastuvotja"))
    {

        printf("Starting: %s\n", "VASTUVOTJA"); 

        double *delaysCalculated = RegisterBlinks(i2cHandle); 

        printf("Calculated delays and returned\n");
        int numOfValidCalculations = 0;
        double averageDelay = calculateAverage(delaysCalculated, &numOfValidCalculations); 
        char averageDelayStr[50]; 
    
        sprintf(averageDelayStr, "Average Delay: %.5f\n", averageDelay); // Format average delay
        printf("%s\n",averageDelayStr);
    
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, averageDelayStr);
        printDelaysToFile("delays.txt", delaysCalculated, numOfValidCalculations, averageDelay);
    }

    if (i2cHandle){
        close(i2cHandle);
    }

    if (syncLedOpenedPort)
    {
        ClosePort(syncLedOpenedPort);
    }

    pthread_join(buttonThread, NULL);

    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished");
    oledWriteText(i2cHandle, 2, 0, "Shutting Down");

    /*if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/

    return 0;
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024 // Define an initial buffer size

void append_to_buffer(char **buffer, int *current_size, const char *data) {
    // Check if there is enough space in the buffer
    if (*current_size + strlen(data) >= BUFFER_SIZE) {
        fprintf(stderr, "Buffer overflow. Consider increasing BUFFER_SIZE.\n");
        return;
    }
    
    // Append data to buffer
    strcat(*buffer, data);
    *current_size += strlen(data);
}

int main() {
    // Allocate buffer and initialize it to an empty string
    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Failed to allocate buffer");
        return 1;
    }
    buffer[0] = '\0'; // Start with an empty string

    int current_size = 0;

    // Example usage
    append_to_buffer(&buffer, &current_size, "This is the first line.\n");
    append_to_buffer(&buffer, &current_size, "This is the second line.\n");
    append_to_buffer(&buffer, &current_size, "This is the third line.\n");

    // Write buffer content to file
    FILE *file = fopen("output.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        free(buffer);
        return 1;
    }
    fputs(buffer, file); // Write the buffer to the file in one go
    fclose(file);

    // Cleanup
    free(buffer);
    printf("Data written to output.txt\n");
    return 0;
}
*/
