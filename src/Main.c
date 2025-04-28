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
#include "Files.h"
#include "Main.h"
#include "Calibration.h"

int main(void) {
    //##########################################################################
    ShowReady(0);
    printf("Program started\n");	
    preciseSleep(1);

    // Initialize button lock mutex
    pthread_mutex_init(&buttonLock, NULL);

    // Configure button arguments
    struct args_port args_btn;
    args_btn.portPin = GPIO_BUTTON;
    args_btn.debugName = "InputButton";
    args_btn.inputOutput = true;

    // Set up signal handler
    signal(SIGINT, signalHandler);

    char *buffer = NULL;

    // Log the start timestamp
    TimeStampToBuffer(&buffer, "Start: ");

    // Create OLED thread
    if (pthread_create(&oledThread, NULL, oled_thread, NULL) < 0) {
        perror("Failed to create OLED thread");

        if (system("sudo shutdown -h now") != 0) {
            perror("Failed to shutdown");
        }
        return 1;
    }

    // Create button thread
    if (pthread_create(&buttonThread, NULL, readButtonState_thread, (void *)&args_btn) < 0) {
        perror("Failed to create button thread");
        SetOledMessage("ERROR Failed to create thread", 0, 0, true);
        SetOledMessage("Restarting program", 2, 0, false);

        printf("Error with thread, restarting program\n");
    
        SetOledMessage(" ", 0, 0, true);
        preciseSleep(0.5);
        pthread_cancel(oledThread);
        pthread_join(oledThread, NULL);
    

        if (system("sudo systemctl restart mooteseade.service") != 0) {
            perror("Failed to restart");
        }

        return 1;
    }

    printf("Button thread created\n");
    append_to_buffer(&buffer, "Button thread created\n");

    const char *startState;
    const char *blinkerOrReceiver;
    
    do
    {

        // Wait for user to select configuration
        do {
            SetSystemState("MAIN-->MAIN MENU");
            startState = WaitForButtonAndSelectConfig("SHUTDOWN", "CALIBRATE", "START");

            if (strcmp(startState, "SHUTDOWN") == 0) {
                SetSystemState("MAIN-->SHUTDOWN");
                free(buffer);
                SetOledMessage("Shutting down...", 0, 0, true);
                preciseSleep(1);
                programRunning = 0;
                
                pthread_cancel(buttonThread);
                pthread_join(buttonThread, NULL);
            
                SetOledMessage(" ", 0, 0, true);
                preciseSleep(0.5);
                pthread_cancel(oledThread);
                pthread_join(oledThread, NULL);        

                if (system("sudo shutdown -h now") != 0) {
                    perror("Failed to shutdown");
                }
                return 1;
            } else if (strcmp(startState, "START") == 0) {
                
                SetSystemState("MAIN");
                SetOledMessage("STARTING...", 0, 0, true);
                preciseSleep(1.5);

            } else if (strcmp(startState, "CALIBRATE") == 0) {
                
                SetSystemState("CALIBRATE-->MODE MENU");
                preciseSleep(1);
                Calibrate();
            }

        } while (strcmp("START", startState) != 0);

        //##########################################################################
        
        SetSystemState("MAIN-->MODE MENU");
        blinkerOrReceiver = WaitForButtonAndSelectConfig("Blinker", "Receiver", "Back");
        
    } while (strcmp("Blinker", blinkerOrReceiver) != 0 && strcmp("Receiver", blinkerOrReceiver) != 0);
    
    char message[50];
    printf("You have chosen: %s\n", blinkerOrReceiver);
    snprintf(message, sizeof(message), "Picked configuration: %s\n", blinkerOrReceiver);
    append_to_buffer(&buffer, message);

    // Start chrony service for synchronization
    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
        printf("Error with chrony, restarting program\n");
        if (system("sudo systemctl restart mooteseade.service") != 0) {
            perror("Failed to restart");
        }
    }
    
    int runAgain = 0;

    do {
        // Log for "run again" mode
        if (runAgain) {
            TimeStampToBuffer(&buffer, "Starting log in run again mode: \n");
            append_to_buffer(&buffer, message);
        }

        // Synchronize using chrony
        printf("Synchronizing\n");
        TimeStampToBuffer(&buffer, "Start synchronizing: ");

        SetSystemState("MAIN");
        int synced = ChronySync(&buffer);
        if (!synced) {
            printf("Synchronized\n");
            append_to_buffer(&buffer, "Synchronized\n");
            ShowReady(1);
        }

        //##########################################################################

        if (!strcmp(blinkerOrReceiver, "Blinker")) {
            
            SetSystemState("MAIN-->BLINKER");
            printf("Starting: BLINKER\n");
            SetOledMessage("Starting Blinker", 0, 0, true);
            preciseSleep(0.5);
            TimeStampToBuffer(&buffer, "Blinker program start: ");

            struct args_port ledBlinkPort;
            ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK;
            ledBlinkPort.debugName = "ledBlink";
            ledBlinkPort.inputOutput = false;

            struct timespec firstblink = ledBlinkOnce(&ledBlinkPort, &buffer);

            WaitForNextMinute(firstblink);

            SetOledMessage("Blinking...", 0, 0, true);
            ledBlinkingMain(&ledBlinkPort, &buffer);

            SetOledMessage("FINISHED", 0, 2, true);
            preciseSleep(1);

            printf("Blinking finished\n");
            TimeStampToBuffer(&buffer, "Blinking finished: ");

            pthread_mutex_lock(&buttonLock);
            buttonPressed = 0;
            pthread_mutex_unlock(&buttonLock);


        
        } else if (!strcmp(blinkerOrReceiver, "Receiver")) {

            SetSystemState("MAIN-->RECEIVER");

            TimeStampToBuffer(&buffer, "RECEIVER program start: ");
            SetOledMessage("Starting RECEIVER", 0, 0, true);
            preciseSleep(0.5);
            SetOledMessage("Waiting sync blink...", 0, 2, false);

            int numOfValidCalculations = 0;
            double *delaysCalculated = RegisterBlinks(&buffer, &numOfValidCalculations);

            printf("Calculating average delay\n");
            double averageDelay = calculateAverage(delaysCalculated, &numOfValidCalculations);

            char averageDelayStr[50];
            sprintf(averageDelayStr, "Average: %.3f %s\n", averageDelay, "ms");
            printf("%s\n", averageDelayStr);
            append_to_buffer(&buffer, averageDelayStr);

            free(delaysCalculated);

            SetOledMessage(averageDelayStr, 0, 2, true);
            preciseSleep(1);
            TimeStampToBuffer(&buffer, "Receiver finished: ");
        
            pthread_mutex_lock(&buttonLock);
            buttonPressed = 0;
            pthread_mutex_unlock(&buttonLock);

            AddSystemOffsetToBuffer(&buffer);
            TimeStampToBuffer(&buffer, "End: ");
        }

        //##########################################################################
        
        SetOledMessage("PRESS BUTTON TO END", 0, 0, false);

        while (1) {
            if (IsButtonPressed()) {
                printf("ENDED pressed\n");
                break;
            }
            preciseSleep(0.1);
        }
        
        write_log_to_file(buffer);
        SetSystemState("MAIN-->END MENU");

        const char *endState = WaitForButtonAndSelectConfig("shutdown", "restart all", "run again");

        if (strcmp(endState, "shutdown") == 0) {
            SetSystemState("MAIN-->SHUTDOWN");
            runAgain = 0;
            free(buffer);
            break;
        } else if (strcmp(endState, "restart all") == 0) {
            SetSystemState("MAIN-->RESTART");
            SetOledMessage("Restarting program", 0, 2, true);
            free(buffer);

            pthread_cancel(buttonThread);
            pthread_join(buttonThread, NULL);
        
            SetOledMessage(" ", 0, 0, true);
            preciseSleep(0.5);
            pthread_cancel(oledThread);
            pthread_join(oledThread, NULL);
        

            if (system("sudo systemctl restart mooteseade.service") != 0) {
                perror("Failed to restart");
            }
            runAgain = 0;
            
        } else if (strcmp(endState, "run again") == 0) {
            SetSystemState("MAIN");
            runAgain = 1;
			// Free and reset the buffer in "run again" mode
			if (runAgain) {
				free(buffer);  // Free the old buffer
				buffer = NULL; // Set to NULL to reset static variables in append_to_buffer
			}
            ShowReady(0);
        } else {
            SetSystemState("MAIN-->SHUTDOWN");
            free(buffer);
            runAgain = 0;
        }

    } while (runAgain == 1);

    //##########################################################################

    SetOledMessage("Program finished", 0, 0, true);
    preciseSleep(1);
    SetOledMessage("Shutting Down", 0, 0, true);
    printf("Program finished\n");

    ShowReady(0);

    programRunning = 0;
    printf("programRunning: %d\n", programRunning);
    fflush(stdout);

    pthread_cancel(buttonThread);
    pthread_join(buttonThread, NULL);

    SetOledMessage(" ", 0, 0, true);
    preciseSleep(0.5);
    pthread_cancel(oledThread);
    pthread_join(oledThread, NULL);

    pthread_mutex_destroy(&buttonLock);

    if (system("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
    }

    return 0;
}
