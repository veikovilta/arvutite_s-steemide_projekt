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
#include "Files.h"
#include <signal.h>
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

void signalHandler(int signum) {
    printf("\nReceived signal %d, shutting down...\n", signum);
    programRunning = 0;  // Set flag to stop the thread loop
    pthread_join(buttonThread, NULL);  // Wait for the thread to finish
	pthread_mutex_destroy(&buttonLock);
	ShowReady(0);
	
	//pthread_join(oledThread, NULL);	
    //pthread_mutex_destroy(&oledLock);
	
    // Additional cleanup if needed
    printf("Program terminated cleanly.\n");
    exit(0);
}

void* readButtonState_thread(void* arg) {
    struct args_port* args = (struct args_port*) arg;
    struct port *openedPort = openPort(args->portPin, args->debugName, args->inputOutput);
    
    double buttonPressedTime = 0.0;
    const double shutdownHoldTime = 3.0;
    
    //pthread_cleanup_push((void(*)(void*))ClosePort, openedPort);
	
    if (openedPort == NULL) {
        fprintf(stderr, "Failed to open port.\n");
        return NULL;
    }

    int debounceCount = 0;
    const int debounceThreshold = 3; // Require 3 stable reads
    int buttonState = 0;
    int prevButtonState = 0;
	int lastReportedState = 0;
	char *new_argv[] = { "sudo", "./projekt", NULL };

	
    while(programRunning) {
        buttonState = gpiod_line_get_value(openedPort->line);
        if (buttonState < 0) {
            perror("Failed to read button state");
            break; // Exit the loop on error
        }

        if (buttonState == prevButtonState) {
			if (buttonState){
				debounceCount++;
		    }
        }

        if (debounceCount >= debounceThreshold && buttonState != lastReportedState) {

            if (buttonState == 1) {
                pthread_mutex_lock(&buttonLock);
			    buttonPressed = 1;
                pthread_mutex_unlock(&buttonLock);
            }
          
            lastReportedState = buttonState;
            debounceCount = 0;
        }
        
        // === 3-second hold causes shutdown ===
        if (buttonState == 1) 
        {
            buttonPressedTime += 0.01; // 10 ms per loop
            if (buttonPressedTime >= shutdownHoldTime) 
            {
                //printf("Restarting...\n");
				SetOledMessage("Restarting...", 0, 0, true);
				preciseSleep(2); 
				SetOledMessage(" ", 0, 0, true);
				preciseSleep(1);
                if (system("sudo systemctl restart mooteseade.service") != 0) {
                    perror("Failed to restart");
                }
                programRunning = 0;
                break;
            }
        }
        else 
        {
            buttonPressedTime = 0.0; // Reset if button released
        }    


        prevButtonState = buttonState;
        preciseSleep(0.01); // 10 ms delay
    }

    ClosePort(openedPort);	
    return NULL;
}

void ClosePort(struct port* openedPort)
{
    //gpiod_line_set_value(openedPort->line, 0);
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);
}

void ShowReady(int outputValue)
{  
    struct port *openedPort = openPort(GPIO_READY_LED, "GPIO PIN 23", false);

    if (openedPort == NULL) {
        return;
    }
    // Display LED
    gpiod_line_set_value(openedPort->line, outputValue);

    ClosePort(openedPort);
}

void AddSystemOffsetToBuffer(char** buffer)
{
	FILE *fp;
    double systemOffset = 0.0;
    char bufferStr[200];
    char numberStr[20] = "";
    char message[100] = ""; 

	// Run the "chronyc tracking" command and open a pipe to read the output
	fp = popen("chronyc tracking", "r");
	if (fp == NULL) 
	{
	    //oledClear(i2cHandle); // Clear the display
        SetOledMessage("Failed to run chronyc command.", 0, 0, true); 
        SetOledMessage("Shutting Down", 0, 0, false); 
        //oledWriteText(i2cHandle, 0, 0, "Failed to run chronyc command.");
	    //oledWriteText(i2cHandle, 0, 2, "Shutting Down");
	    printf("Error with chronyc, shutting down\n");
	    //system ("sudo shutdown -h now");
	   	return;
	}

	// Parse the output line by line
    while (fgets(bufferStr, sizeof(bufferStr), fp) != NULL) 
    {
        // Look for the line that contains "System time" to get the offset
        if (strstr(bufferStr, "System time") != NULL) 
        {
             // Extract the offset value (it will be the second value in the line)
            if (sscanf(bufferStr, "System time     : %lf seconds", &systemOffset) != 1) {
                fprintf(stderr, "Failed to parse system offset.\n");
                break;
            }
            break;
        }
    }

    // Close the pipe
    if (pclose(fp) == -1) {
        perror("pclose failed");
        return;
    }

    // Output the system offset to the user
    sprintf(numberStr, "%.7f", systemOffset);
    snprintf(message, sizeof(message), "Clock offset: %7s\n", numberStr);

	append_to_buffer(buffer, message);
}

int CheckSync(char** buffer)
{
    FILE *fp;
    double systemOffset = 0.0;
    char bufferStr[200];
    char numberStr[20] = "";
    char message[100] = ""; 

    // Run the "chronyc tracking" command and open a pipe to read the output
    fp = popen("chronyc tracking", "r");
    if (fp == NULL) 
    {
        //oledClear(i2cHandle); // Clear the display
        //oledWriteText(i2cHandle, 0, 0, "Failed to run chronyc command.");
        //oledWriteText(i2cHandle, 0, 2, "Shutting Down");
        SetOledMessage("Failed to run chronyc command.", 0, 0, true); 
        SetOledMessage("Shutting Down", 0, 0, false); 
        printf("Error with chronyc, shutting down\n");
        if (system("sudo systemctl status mooteseade.service") != 0) {
            perror("Failed to restart");
        }
        return 1;
    }

    // Parse the output line by line
    while (fgets(bufferStr, sizeof(bufferStr), fp) != NULL) 
    {
        // Look for the line that contains "System time" to get the offset
        if (strstr(bufferStr, "System time") != NULL) 
        {
             // Extract the offset value (it will be the second value in the line)
            if (sscanf(bufferStr, "System time     : %lf seconds", &systemOffset) != 1) {
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
    snprintf(message, sizeof(message), "Offset: %7s\n", numberStr);
    append_to_buffer(buffer, message);
    //oledClear(i2cHandle);
    // Display a message on the OLED
    
    //oledWriteText(i2cHandle, 0, 0, "Syncronized");
    //SetOledMessage("Syncronized", 0, 0, true); 
    //oledWriteText(i2cHandle, 0, 2, message);
    //preciseSleep(0.5);
    //SetOledMessage(message, 0, 2, false); 
    // Check synchronization status
    // piiriks 0.1 ms
    if (systemOffset < 0.0001 && systemOffset > -0.0001) 
    {
      	 return 0;
    }

    return 1;
}

const char* waitForButtonState(int port1, int port2, const char* state1Value, const char* state2Value) 
{
    
    struct port* openedPort1 = openPort(port1, "Port 1", true);  // Pin for saatja
	struct port* openedPort2 = openPort(port2, "Port 2", true);  // Pin for saatja
	    
	int state1 = gpiod_line_get_value(openedPort1->line);
	int state2 = gpiod_line_get_value(openedPort2->line);

	ClosePort(openedPort1);
	ClosePort(openedPort2);
	
    if (state1 < 0 || state2 < 0) {
        perror("Failed to read GPIO line value");
        return "error";
    }
	
    // Determine the button state
    if (state1 == 1 && state2 == 0)
    {
        return state1Value;  // Button pressed for "saatja"
    } 
    else if (state1 == 0 && state2 == 1) 
    {
        return state2Value;  // Button pressed for "vastuvotja"
    } 
    else
    //printf("Button state: %s\n", state);
	preciseSleep(0.5);
	    
    return "undefined";  // Undefined state
}


int ChronySync(char** buffer)
{
    // message string
    char message[100] = "";  
    char numberStr[20] = "";

	//oledWriteText(i2cHandle, 0, 0, "Syncronizing");    
	SetOledMessage("Syncronizing", 0, 0, true); 
    // teeb 60 sekundilist checki kui hea kell on
    // 10min vähemalt
    int minutes = 0;
    while(1)
    {
        preciseSleep(5);
   		 
    
        if (CheckSync(buffer) == 0)
        {
            break;
        }
        else 
        {
            // kirjuta ekraanile et oodatud 5 sek
            sprintf(numberStr, "%d", minutes+5);
            snprintf(message, sizeof(message), "seconds waited : %s", numberStr);
            printf("%s\n", message);
            
            //oledWriteText(i2cHandle, 0, 0, "Syncronizing");
            SetOledMessage("Syncronizing", 0, 0, true); 
            preciseSleep(0.5);
            SetOledMessage(message, 0, 2, false); 
            //oledWriteText(i2cHandle, 0, 2, message);
        }
        
        minutes+=5;

        // kui ei ole syncis 10 mintaga siis error
        if (minutes == 120)
        {
            //oledClear(i2cHandle);
            // Display a message on the OLED
            //oledWriteText(i2cHandle, 0, 0, "NOT SYNCED");
            //oledWriteText(i2cHandle, 2, 0, "ERROR BAD RECEPTION");
            //oledWriteText(i2cHandle, 4, 0, "Shutting Down");
            SetOledMessage("NOT SYNCED", 0, 0, true); 
            SetOledMessage("ERROR BAD RECEPTION", 0, 2, false);
            SetOledMessage("Shutting Down", 0, 4, false); 
            /*
            if (system ("sudo shutdown -h now") != 0) {
                perror("Failed to shutdown");
                //oledClear(i2cHandle);
                //oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
                SetOledMessage("ERROR BAD RECEPTION", 2, 0, true);
                // Handle the error or exit
            }
            */
            return 1;
        }
        
    }

    return 0;
}

void WaitForNextMinute(struct timespec firstblink) {
    
    time_t rounded_time = firstblink.tv_sec;

    int seconds_past_minute = rounded_time % 60;

    if (seconds_past_minute >= 50) {
        // If within last 10 seconds of the minute → skip next minute
        rounded_time += (60 - seconds_past_minute) + 60;
    } else {
        // Just go to the next minute
        rounded_time += (60 - seconds_past_minute);
    }

    struct tm *rounded_tm = localtime(&rounded_time);

    char awakeBuffer[100];
    strftime(awakeBuffer, sizeof(awakeBuffer), "START AT %H:%M", rounded_tm);
    struct timespec currentTime;
    int prevSecondsLeft = -1;
    int bufferSize = 16;
    char buffer[bufferSize];
    
    SetOledMessage(awakeBuffer, 0, 0, true);
    preciseSleep(0.5);

    // Print checking status
    //printf("Checking if it's less than 10 sec to the next full minute\n");
    //fflush(stdout);

    // Calculate if within 10 seconds of the next full minute
    if (60 - (firstblink.tv_sec % 60) <= 10) {
        //preciseSleep(11); // Sleep for 11 seconds
        while (1) {
            clock_gettime(CLOCK_REALTIME, &currentTime);
    
            int secondsLeft = 60 - (currentTime.tv_sec % 60);
    
            if (secondsLeft == 60 && currentTime.tv_nsec < 1e6) {
                preciseSleep(0.01); // Sleep for 0.5 seconds
                break;
            }
    
            // Only update the buffer when seconds left changes
            if (secondsLeft != prevSecondsLeft) {
                // Format: "Sec left: <seconds>"
                // Assumes bufferSize >= 16
                snprintf(buffer, bufferSize, "Sec left: %2ds", secondsLeft + 60);
                prevSecondsLeft = secondsLeft;
                SetOledMessage(buffer, 0, 2, false);
            }
    
            preciseSleep(0.0001);  // Sleep 100 microseconds
        }
    }

    //printf("Waiting for the next minute\n");
    //fflush(stdout);

    // Loop until the next full minute

    while (1) {
        clock_gettime(CLOCK_REALTIME, &currentTime);

        int secondsLeft = 60 - (currentTime.tv_sec % 60);

        if (secondsLeft == 60 && currentTime.tv_nsec < 1e6) {
            break;
        }

        // Only update the buffer when seconds left changes
        if (secondsLeft != prevSecondsLeft) {
            // Format: "Sec left: <seconds>"
            // Assumes bufferSize >= 16
            snprintf(buffer, bufferSize, "Sec left: %2ds", secondsLeft);
            prevSecondsLeft = secondsLeft;
            SetOledMessage(buffer, 0, 2, false);
        }

        preciseSleep(0.0001);  // Sleep 100 microseconds
    }

    return;
}

const char* WaitForButtonAndSelectConfig(const char* state1Value,
											 const char* state2Value, const char* state3Value) 
{
    char* value = "\0";
    char message[100] = "";
    char lastPicked[100] = "";
    // Lock and reset the buttonPressed flag

    pthread_mutex_unlock(&buttonLock);
    pthread_mutex_lock(&buttonLock);
    buttonPressed = 0;
    pthread_mutex_unlock(&buttonLock);

    while (1) {

        // Wait for button state and get the selected config
        value = waitForButtonState(24, 25, state1Value, state2Value);

        if(strcmp(value, "undefined") == 0){
			value = state3Value; 
		}

        sprintf(message, "Selected:%s\n", value);
		
        if (lastPicked[0] == '\0') {
            SetOledMessage("PRESS BUTTON TO PICK", 0, 0, true);
            preciseSleep(0.5);
            SetOledMessage(message, 1, 2, false);
            printf("%s\n", message); 
        }

        if(strcmp(value, lastPicked) != 0 && strcmp("", lastPicked) != 0){
            SetOledMessage("PRESS BUTTON TO PICK", 0, 0, true);
            preciseSleep(0.5);
            SetOledMessage(message, 1, 2, false);
            printf("%s\n", message); 
        }
	    strcpy(lastPicked, value);
        preciseSleep(0.5);
        // Check if button was pressed, exit loop if true
        pthread_mutex_lock(&buttonLock);
        //printf("%d\n", buttonPressed);
        if (buttonPressed) {
            buttonPressed = 0; 
			pthread_mutex_unlock(&buttonLock); 
            break;
        }
        pthread_mutex_unlock(&buttonLock); 
    }

    return value;
}

int IsButtonPressed(void)
{
	pthread_mutex_lock(&buttonLock);
	int pressed = 0;

	if (buttonPressed) {
		buttonPressed = 0;
		pressed = 1;
	}

	pthread_mutex_unlock(&buttonLock);
	return pressed;
}

