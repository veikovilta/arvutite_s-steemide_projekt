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
#include "State.h"
#include "Files.h"
#include "Main.h"


int main(void)
{
//##########################################################################

    InstanceState = STARTING;
    char *buffer = NULL;

    TimeStampToBuffer(&buffer, "Start: ");

//##########################################################################

    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
    }

//##########################################################################


    int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
    if (i2cHandle < 0) return -1;
    
    char message[100] = "";  
    //char numberStr[20] = "";

    oledInit(i2cHandle);
    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program started");
    printf("Program started\n");
    preciseSleep(1);

//##########################################################################
    
    pthread_mutex_init(&buttonLock, NULL);

    pthread_t buttonThread;

    if (CreateButtonThread(i2cHandle, &buttonThread))
    {
        printf("Button thread created\n");
        append_to_buffer(&buffer, "Button thread created\n");
    }
    

//##########################################################################

    InstanceState = PICKING_CONFIG; 

    const char* saatjaOrVastuvotja = WaitForButtonAndSelectConfig(i2cHandle);
    printf("You have chosen: %s\n", saatjaOrVastuvotja);
    snprintf(message, sizeof(message), "Picked configuration: %s\n", saatjaOrVastuvotja);
    append_to_buffer(&buffer, message); 


//##########################################################################


    InstanceState = SYNCHRONIZING; 

    struct port* syncLedOpenedPort = NULL;

    int synced = ChronySync(i2cHandle);

    if(!synced){
        printf("Syncronized\n");
        append_to_buffer(&buffer, "Syncronized\n");
        syncLedOpenedPort = ShowReady();
    }


//##########################################################################


    if(!strcmp(saatjaOrVastuvotja, (const char*)"saatja"))
    {
        Saatja_Vastuvotja_State = SAATJA; 

        printf("Starting: %s\n", "SAATJA"); 
        fflush(stdout);

        struct args_port ledBlinkPort; 

        ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK; 
        ledBlinkPort.debugName = "ledBlink";
        ledBlinkPort.inputOutput = false;

        struct timespec firstblink = ledBlinkOnce(&ledBlinkPort, &buffer);

        // Print the time
        printf("LED blink time: %ld seconds and %ld nanoseconds\n", 
            (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
        fflush(stdout);

        
        InstanceState = WAITING_NEXT_MINUTE_LED; 

        WaitForNextMinuteBlinker(firstblink); 
        
        InstanceState = LED_BLINKING;

        printf("Start blinking\n");
        fflush(stdout);
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "BLINKING");
        
        ledBlinking20(&ledBlinkPort, &buffer);

        InstanceState = BLINKING_FINISHED;

        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "FINISHED");
        
        preciseSleep(1); 

        printf("Blinking finished\n"); 
        TimeStampToBuffer(&buffer, "Blinking finished: "); 
    }
    else if(!strcmp(saatjaOrVastuvotja, (const char*)"vastuvotja"))
    {
        Saatja_Vastuvotja_State = VASTUVOTJA;

        printf("Starting: %s\n", "VASTUVOTJA"); 

        /*
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
        */
    }


//##########################################################################


    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished");
    oledWriteText(i2cHandle, 0, 2, "Shutting Down");

    write_log_to_file(buffer);
    free(buffer);

    if (syncLedOpenedPort)
    {
        ClosePort(syncLedOpenedPort);
    }

    pthread_join(buttonThread, NULL);
    pthread_mutex_destroy(&buttonLock);

    if (i2cHandle){
        close(i2cHandle);
    }

    /*if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/


//##########################################################################


    return 0;
}
