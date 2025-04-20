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

    // Wait for user to select configuration
    do {
        startState = WaitForButtonAndSelectConfig("SHUTDOWN", "CALIBRATE", "START");

        if (strcmp(startState, "SHUTDOWN") == 0) {
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
            
            SetOledMessage("STARTING...", 0, 0, true);
            preciseSleep(1.5);

        } else if (strcmp(startState, "CALIBRATE") == 0) {
            
            preciseSleep(1);
            Calibrate();
        }

    } while (strcmp("START", startState) != 0);

    //##########################################################################

    // Start chrony service for synchronization
    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
        printf("Error with chrony, restarting program\n");
        if (system("sudo systemctl restart mooteseade.service") != 0) {
            perror("Failed to restart");
        }
    }

    //##########################################################################

    char message[50];
    const char *saatjaOrVastuvotja = WaitForButtonAndSelectConfig("saatja", "vastuvotja", "None");
    printf("You have chosen: %s\n", saatjaOrVastuvotja);
    snprintf(message, sizeof(message), "Picked configuration: %s\n", saatjaOrVastuvotja);
    append_to_buffer(&buffer, message);

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

        int synced = ChronySync(&buffer);
        if (!synced) {
            printf("Synchronized\n");
            append_to_buffer(&buffer, "Synchronized\n");
            ShowReady(1);
        }

        //##########################################################################

        if (!strcmp(saatjaOrVastuvotja, "saatja")) {

            printf("Starting: SAATJA\n");
            SetOledMessage("Starting Saatja", 0, 0, true);

            TimeStampToBuffer(&buffer, "Blinking program start: ");

            struct args_port ledBlinkPort;
            ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK;
            ledBlinkPort.debugName = "ledBlink";
            ledBlinkPort.inputOutput = false;

            struct timespec firstblink = ledBlinkOnce(&ledBlinkPort, &buffer);

    
            SetOledMessage("Next min blink", 0, 0, true);
            preciseSleep(0.5);
            SetOledMessage("sleeping...", 0, 2, false);

            printf("LED blink time: %ld seconds and %ld nanoseconds\n",
                   (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
            fflush(stdout);

            WaitForNextMinuteBlinker(firstblink);

            SetOledMessage("Blinking...", 0, 0, true);
            ledBlinking20(&ledBlinkPort, &buffer);

            SetOledMessage("FINISHED", 0, 2, true);
            preciseSleep(1);

            printf("Blinking finished\n");
            TimeStampToBuffer(&buffer, "Blinking finished: ");

            pthread_mutex_lock(&buttonLock);
            buttonPressed = 0;
            pthread_mutex_unlock(&buttonLock);

            SetOledMessage("PRESS BTN TO END", 0, 0, false);

            while (1) {
                if (IsButtonPressed()) {
                    printf("ENDED pressed\n");
                    break;
                }
                preciseSleep(0.1);
            }
        
        } else if (!strcmp(saatjaOrVastuvotja, "vastuvotja")) {

            TimeStampToBuffer(&buffer, "Sensor program start: ");
            SetOledMessage("Starting VASTUVOTJA", 0, 0, true);

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
            TimeStampToBuffer(&buffer, "Sensor finished: ");
        
            pthread_mutex_lock(&buttonLock);
            buttonPressed = 0;
            pthread_mutex_unlock(&buttonLock);

            AddSystemOffsetToBuffer(&buffer);
            TimeStampToBuffer(&buffer, "End: ");

            SetOledMessage("PRESS BTN TO END", 0, 0, false);

            while (1) {
                if (IsButtonPressed()) {
                    printf("ENDED pressed\n");
                    break;
                }
                preciseSleep(0.1);
            }
        }

        //##########################################################################

        write_log_to_file(buffer);

        const char *endState = WaitForButtonAndSelectConfig("shutdown", "restart all", "run again");

        if (strcmp(endState, "shutdown") == 0) {
            runAgain = 0;
            free(buffer);
            break;
        } else if (strcmp(endState, "restart all") == 0) {
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
            runAgain = 1;
			// Free and reset the buffer in "run again" mode
			if (runAgain) {
				free(buffer);  // Free the old buffer
				buffer = NULL; // Set to NULL to reset static variables in append_to_buffer
			}
        } else {
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
