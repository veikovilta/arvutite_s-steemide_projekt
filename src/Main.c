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
    
    const char* saatjaOrVastuvotja = waitForButtonState();
    struct port* syncLedOpenedPort = NULL;

    int synced = ChronySync(i2cHandle); 

    if(!synced){
        syncLedOpenedPort = ShowReady();
    }

    if(!strcmp(saatjaOrVastuvotja, (const char*)"saatja"))
    {
        struct args_port ledBlinkPort; 

        ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK; 
        ledBlinkPort.debugName = "ledBlink";
        ledBlinkPort.inputOutput = false;

        struct timespec firstblink = ledBlinkOnce(&ledBlinkPort);

        // Print the time
        printf("LED blink time: %ld seconds and %ld nanoseconds\n", 
            (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
        fflush(stdout);
        
        printf("Checking if its less than 10sec to the next full minute\n");
        fflush(stdout);
        
        
        if (60 - (firstblink.tv_sec % 60) <= 10)
        {
            preciseSleep(11);
        }
        
        printf("Waiting next minute\n");
        fflush(stdout);
        
        struct timespec currentTime; 
        
        while (1)
        {
            clock_gettime(CLOCK_REALTIME, &currentTime);
            
            if ((currentTime.tv_sec % 60) == 0 && currentTime.tv_nsec < 1e6)
            {
                break;
            }
            
            preciseSleep(0.0001);
        }
        
        printf("Start blinking\n");
        fflush(stdout);
        
        ledBlinking20(&ledBlinkPort);
        close(i2cHandle);
    }
    else if(!strcmp(saatjaOrVastuvotja, (const char*)"vastuvotja"))
    {
        printf("Showed that im ready\n");
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
        pthread_join(buttonThread, NULL);
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "Program finished");
        oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        close(i2cHandle);

    }

    ClosePort(syncLedOpenedPort);

    /*if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/

    return 0;
}



