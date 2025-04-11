

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <gpiod.h> 
#include <signal.h> 
#include "stdbool.h"
#include "HelperFunctions.h"
#include "display.h"
#include <string.h>
#include "Sensor.h"
#include "LedBlink.h"
#include "State.h"
#include "Files.h"
#include "Main.h"
#include "Calibration.h"

int main(void)
{
//##########################################################################
	ShowReady(0);
	
	int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
	if (i2cHandle < 0) return -1;
	
	oledInit(i2cHandle);
	oledClear(i2cHandle);
	oledWriteText(i2cHandle, 0, 0, "Program started");
	printf("Program started\n");
	preciseSleep(1);
	
	//peab testima kas see ethernet test tootab v pean panema testima kuni on ja mingi ajaga valja lic
	if (!check_ethernet_connected()) {
	    printf("Ethernet not connected! Closing\n");
	    oledClear(i2cHandle);
	    oledWriteText(i2cHandle, 0, 0, "No Ethernet!");
	    preciseSleep(3);
	    return 1;
	}

    printf("Ethernet connected\n");
    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Ethernet connected!");
    preciseSleep(2);

    signal(SIGINT, signalHandler);

    InstanceState = STARTING;
    char *buffer = NULL;
	
    TimeStampToBuffer(&buffer, "Start: ");
	
//##########################################################################

    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
    }

//##########################################################################
    
    //char numberStr[20] = "";

		
	
	/*
    pthread_mutex_init(&oledLock, NULL);


	if(pthread_create(&oledThread, NULL, oled_thread, NULL) < 0)
	{
	    perror("Failed to create thread");
	    
	    if (system("sudo shutdown -h now") != 0) {
	        perror("Failed to shutdown");
	    }
	    return 1;
	}

	*/
	

//##########################################################################
    
    pthread_mutex_init(&buttonLock, NULL);

    struct args_port args;
    args.portPin = GPIO_BUTTON;
    args.debugName = "InputButton";
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
        }
        return 1;
    }

    printf("Button thread created\n");
	append_to_buffer(&buffer, "Button thread created\n");

//##########################################################################

    InstanceState = PICKING_CONFIG; 
    
    char message[50];
    const char* saatjaOrVastuvotja = WaitForButtonAndSelectConfig(i2cHandle, "saatja", "vastuvotja");
    printf("You have chosen: %s\n", saatjaOrVastuvotja);
    snprintf(message, sizeof(message), "Picked configuration: %s\n", saatjaOrVastuvotja);
    append_to_buffer(&buffer, message); 
	//const char* saatjaOrVastuvotja = "vastuvotja";

//##########################################################################


    oledClear(i2cHandle);
    InstanceState = SYNCHRONIZING; 
    printf("Syncronizing\n");
    TimeStampToBuffer(&buffer, "Start syncronizing: ");
    
    int synced = ChronySync(i2cHandle, &buffer);

    if(!synced){
        printf("Syncronized\n");
        append_to_buffer(&buffer, "Syncronized\n");
        ShowReady(1);
    }


//##########################################################################


    if(!strcmp(saatjaOrVastuvotja, (const char*)"saatja"))
    {
        Saatja_Vastuvotja_State = SAATJA; 

        printf("Starting: SAATJA\n");
	oledClear(i2cHandle);
	oledWriteText(i2cHandle, 0, 0, "Starting Saatja");
        
        TimeStampToBuffer(&buffer, "Blinking program start: "); 

        struct args_port ledBlinkPort; 

        ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK; 
        ledBlinkPort.debugName = "ledBlink";
        ledBlinkPort.inputOutput = false;

        struct timespec firstblink = ledBlinkOnce(&ledBlinkPort, &buffer);

        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "Waiting");
        oledWriteText(i2cHandle, 0, 2, "Next min blink");
        // Print the time
        printf("LED blink time: %ld seconds and %ld nanoseconds\n", 
            (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
        fflush(stdout);

        
        InstanceState = WAITING_NEXT_MINUTE_LED; 
	
        WaitForNextMinuteBlinker(firstblink); 
        
        //InstanceState = LED_BLINKING;

        printf("Start blinking\n");
        ledBlinking20(&ledBlinkPort, &buffer);

        InstanceState = BLINKING_FINISHED;

        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "FINISHED");
        
        preciseSleep(1); 

        printf("Blinking finished\n"); 
        TimeStampToBuffer(&buffer, "Blinking finished: ");
        
        pthread_mutex_lock(&buttonLock);
        buttonPressed = 0;
        pthread_mutex_unlock(&buttonLock);

        oledWriteText(i2cHandle, 0, 2, "PRESS BTN TO END");


		/*
        while (1)
        {
            if (IsButtonPressed())
            {
                printf("ENDED pressed\n");

				break;
            }

            preciseSleep(0.1);
        }
        */
        
    }
    else if(!strcmp(saatjaOrVastuvotja, (const char*)"vastuvotja"))
    {
		
    	
        Saatja_Vastuvotja_State = VASTUVOTJA;

	CalibrateVastuvotja(i2cHandle);
		
		
		
        TimeStampToBuffer(&buffer, "Sensor program start: "); 

        printf("Starting: VASTUVOTJA\n"); 
	oledClear(i2cHandle);
	oledWriteText(i2cHandle, 0, 0, "Starting VASTUVOTJA");
		        
        double *delaysCalculated = RegisterBlinks(i2cHandle, &buffer); 

        printf("Calculating average delay\n");
        int numOfValidCalculations = 0;
        double averageDelay = calculateAverage(delaysCalculated, &numOfValidCalculations); 
        char averageDelayStr[50]; 
    
        sprintf(averageDelayStr, "Average: %.7f\n", averageDelay); // Format average delay
        printf("%s\n",averageDelayStr);
        append_to_buffer(&buffer, averageDelayStr); 

        free(delaysCalculated);

        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 4, averageDelayStr);

        TimeStampToBuffer(&buffer, "Sensor finished: "); 
	
        pthread_mutex_lock(&buttonLock);
        buttonPressed = 0;
        pthread_mutex_unlock(&buttonLock);
		
        //oledWriteText(i2cHandle, 0, 2, "PRESS BTN TO END");
		/*
        while (1)
        {
            if (IsButtonPressed())
            {
                break;
            }

            preciseSleep(0.1);
        }
        */
	
    }


//##########################################################################

    //oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished");
    oledWriteText(i2cHandle, 0, 2, "Shutting Down");
    printf("Program finished\n"); 

	AddSystemOffsetToBuffer(&buffer, i2cHandle);	
    TimeStampToBuffer(&buffer, "End: "); 
    
    write_log_to_file(buffer);
    free(buffer);

	ShowReady(0);

	printf("Program finished before thread\n");

	programRunning = 0;
	printf("programmRunning: %d\n", programRunning);
	fflush(stdout); 

	pthread_cancel(buttonThread);
    pthread_join(buttonThread, NULL);

	printf("Program finished before BUTTONLOCK DESTROCTION\n");

    pthread_mutex_destroy(&buttonLock);

	//pthread_join(oledThread, NULL);	
    //pthread_mutex_destroy(&oledLock);

	printf("Program finished before i2c\n");

    oledClear(i2cHandle);
		
    if (i2cHandle){
        close(i2cHandle);
    }

	printf("Program finished\n");
	
    /*if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/


//##########################################################################


    return 0;
}
