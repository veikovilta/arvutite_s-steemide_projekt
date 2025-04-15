

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


	char *new_argv[] = { "sudo", "./projekt", NULL };

	int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
	if (i2cHandle < 0) return -1;
	
	oledInit(i2cHandle);
	oledClear(i2cHandle);
	//oledWriteText(i2cHandle, 0, 0, "Program started");
	printf("Program started\n");
	preciseSleep(1);

    pthread_mutex_init(&buttonLock, NULL);

    struct args_port args_btn;
    args_btn.portPin = GPIO_BUTTON;
    args_btn.debugName = "InputButton";
    args_btn.inputOutput = true;

	signal(SIGINT, signalHandler);

	InstanceState = STARTING;
	char *buffer = NULL;
	
	TimeStampToBuffer(&buffer, "Start: ");
  
    if(pthread_create(&buttonThread, NULL, readButtonState_thread, (void*)&args_btn) < 0)
    {
        perror("Failed to create thread");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread");
        oledWriteText(i2cHandle, 2, 0, "Restarting program");
	
        printf("Error with thread, restarting program\n");
        
        execvp("sudo", new_argv);

        perror("execvp failed");

        
        return 1;
    }

    printf("Button thread created\n");
	append_to_buffer(&buffer, "Button thread created\n");

	oledClear(i2cHandle);
	
	oledWriteText(i2cHandle, 0, 2, "PRESS BTN TO START");

	while (1)
	{
	    if (IsButtonPressed())
	    {
	        printf("START pressed\n");

				break;
	    }

	    preciseSleep(0.1);
	}	

	if (!check_ethernet_connected()) {
	    printf("Ethernet not connected! Closing\n");
	    oledClear(i2cHandle);
	    oledWriteText(i2cHandle, 0, 0, "No Ethernet!");
	    preciseSleep(3);

	    //system("shutdown -h now");
	    //return 1;
	}
	else{

	    printf("Ethernet connected\n");
	    oledClear(i2cHandle);
	    oledWriteText(i2cHandle, 0, 0, "Ethernet connected!");
	    preciseSleep(2);
			
	}

	
//##########################################################################


    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
	oledClear(i2cHandle);
	oledWriteText(i2cHandle, 0, 0, "Chrony failed!");
	
	printf("Error with chrony, restarting program\n");
        
        execvp("sudo", new_argv);

        perror("execvp failed");
    }

//##########################################################################
    
    //char numberStr[20] = "";

    //pthread_mutex_init(&oledLock, NULL);

	
	if(pthread_create(&oledThread, NULL, oled_thread, NULL) < 0)
	{
	    perror("Failed to create thread");
	    
	    if (system("sudo shutdown -h now") != 0) {
	        perror("Failed to shutdown");
	    }
	    return 1;
	}
	
	SetOledMessage("Hello OLED!");	
	SetOledMessage("midagi"); 
//##########################################################################
    

//##########################################################################

    InstanceState = PICKING_CONFIG; 
    
    char message[50];
    const char* saatjaOrVastuvotja = WaitForButtonAndSelectConfig(i2cHandle, "saatja", "vastuvotja", "Switch state");
    printf("You have chosen: %s\n", saatjaOrVastuvotja);
    snprintf(message, sizeof(message), "Picked configuration: %s\n", saatjaOrVastuvotja);
    append_to_buffer(&buffer, message); 
	//const char* saatjaOrVastuvotja = "vastuvotja";

//##########################################################################
	int runAgain = 1; 

	
	do { ////  FOR RUNNING AGAIN  //////

	if (runAgain)
	{
		TimeStampToBuffer(&buffer, "Starting log in run again mode: \n");		
		append_to_buffer(&buffer, message);		
	}
///////////////////////////////////////////////////////////////////////////

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

		
        while (1)
        {
            if (IsButtonPressed())
            {
                printf("ENDED pressed\n");

				break;
            }

            preciseSleep(0.1);
        }
         
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
		 
		AddSystemOffsetToBuffer(&buffer, i2cHandle);	
		TimeStampToBuffer(&buffer, "End: "); 

        oledWriteText(i2cHandle, 0, 2, "PRESS BTN TO END");

        while (1)
        {
            if (IsButtonPressed())
            {
                printf("ENDED pressed\n");

				break;
            }

            preciseSleep(0.1);
        }
        
    }
//##########################################################################

	write_log_to_file(buffer);

	oledClear(i2cHandle);	

	printf("HELLO2");
	
	const char* endState = WaitForButtonAndSelectConfig(i2cHandle, "shutdown", "restart all", "run again");
	
	
	printf("HELLO3"); 
		
		
	if(strcmp(endState, "shutdown") == 0)
	{
		runAgain = 0;
		free(buffer);
		break;
	}
	else if (strcmp(endState, "restart all") == 0)// voib ka lic else-iks panna
	{	
        oledClear(i2cHandle);
	
		oledWriteText(i2cHandle, 2, 0, "Restarting program");
		free(buffer);
		printf("restarting program\n");
	    
		execvp("sudo", new_argv);
		perror("execvp failed");
	    runAgain = 0; 
	}
	else if(strcmp(endState, "run again") == 0)
	{
		runAgain = 1;
		
		if (buffer != NULL) {
		      free(buffer);    // Free the allocated memory.
		      buffer = NULL;   // Set the pointer to NULL to avoid a dangling pointer.
		}  
	}
	else
	{
		free(buffer);
		runAgain = 0;	
	}
	printf("hello"); 
//////////////////////////////////////////////////////////
	} while (runAgain == 1); 	
//##########################################################################

    //oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished");
    oledWriteText(i2cHandle, 0, 2, "Shutting Down");
    printf("Program finished\n"); 



	ShowReady(0);


	programRunning = 0;
	printf("programmRunning: %d\n", programRunning);
	fflush(stdout); 

	pthread_cancel(buttonThread);
    pthread_join(buttonThread, NULL);
	

	pthread_cancel(oledThread);
    pthread_join(oledThread, NULL);


    pthread_mutex_destroy(&buttonLock);

	//pthread_join(oledThread, NULL);	
    //pthread_mutex_destroy(&oledLock);


    oledClear(i2cHandle);
		
    if (i2cHandle){
        close(i2cHandle);
    }

	
	/*    if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/


//##########################################################################


    return 0;
}
