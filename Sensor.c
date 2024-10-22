#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <gpiod.h>
#include <math.h> 
#include "Sensor.h"
#include "HelperFunctions.h"
#include "display.h"


double* RegisterBlinks(int i2cHandle)
{
    fflush(stdout);
    struct port *openedPort = openPort(GPIO_PIN_LED, "GPIO PIN 22", true);
    // Wait for the first blink
    
    while (gpiod_line_get_value(openedPort->line) == 0) 
    {
        preciseSleep(0.1);  
    }

    printf("Got the first blink\n");
    fflush(stdout);
    struct timespec currentTime;
    // Get the current time with high precision

    clock_gettime(CLOCK_REALTIME, &currentTime);

    // Extract the seconds part of the current time
    long currentSeconds = currentTime.tv_sec % 60;
    long currentNanoseconds = currentTime.tv_nsec;
    long nanosecondsToWait = 0;
    long secondsToWait= 60 - currentSeconds;
    double totalSecondsToWait = 0;
    // If the seconds are less than 10, wait until the next full minute
    if (secondsToWait < WAIT_TIME_BEFORE_NEXT_MINUTE) 
    {
        secondsToWait += 60;
        nanosecondsToWait = 1000000000 - currentNanoseconds;
        totalSecondsToWait = (double)secondsToWait + ((double)nanosecondsToWait / 1e9);
    }
    else 
    {
        nanosecondsToWait = 1000000000 - currentNanoseconds;
        totalSecondsToWait = (double)secondsToWait + ((double)nanosecondsToWait / 1e9);
    }
    char numberStr[20] = "";
    char message[100] = ""; 
    sprintf(numberStr, "%.5f", totalSecondsToWait);
    snprintf(message, sizeof(message), "Sleeping for: %5s", numberStr);
    
    oledWriteText(i2cHandle, 0, 2, message);
    printf("waiting for %.4f\n", totalSecondsToWait);
    fflush(stdout);
    preciseSleep(totalSecondsToWait);
    
    struct timespec senderStartTime; 
    
    clock_gettime(CLOCK_REALTIME, &senderStartTime);
        
    struct timespec timestamps[BLINK_COUNT]; 
    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 2, "Got: ");
    // Read the LED blinks and record timestamps
    for (int i = 0; i < BLINK_COUNT; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) 
        {
            preciseSleep(0.001);  //vb checkimis sleep ajad ära võtma?
                                  // sealt mingi kadu ju
        }
        sprintf(numberStr, "%5d ", i);
        printf("Read timestamp : %5d\n", i);
        oledWriteText(i2cHandle, 0, 4, numberStr);
        // Get current time with high precision
        clock_gettime(CLOCK_REALTIME, &timestamps[i]);

        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) == 1) 
        {
            preciseSleep(0.001);
        }
    }
    printf("Got all data\n");
	double *delaysCalculated = calculateDelays(timestamps, senderStartTime);
    printf("Calculated delays\n");
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);
	
	return delaysCalculated; 
}

//error handling to-do if isnt some readings are wrong 
double* calculateDelays(const struct timespec *timestamps,
	const struct timespec senderStartTime) 
{
    double TimeFix = 0.0; // kui läheb syncist välja siis kasutan
    
    double* delaysCalculated = (double*)malloc(BLINK_COUNT * sizeof(double));
    
    // Check if malloc was successful
    if (delaysCalculated == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed for delaysCalculated\n");
        return NULL;
    }
    //double delaysCalculated[BLINK_COUNT];
	
	setArrayToZero(delaysCalculated);
	
    for (int i = 0; i < BLINK_COUNT; i++) {
    
        double sensorSawTimeSec = 
            (double)timestamps[i].tv_sec + 
            ((double)timestamps[i].tv_nsec / 1e9);

        double blinkStartTimeSec = 
            (double)senderStartTime.tv_sec + 
            ((double)senderStartTime.tv_nsec / 1e9) +
            i * BLINK_INTERVAL + TimeFix;
		
        // Check if the time difference is greater than 2 seconds
        if (fabs(sensorSawTimeSec - blinkStartTimeSec) > BLINK_INTERVAL ) 
        {
           if (i + 1 < BLINK_COUNT) { // Ensure there's a next blink to check
                double nextSensorSawTimeSec = 
                    (double)timestamps[i + 1].tv_sec + 
                    ((double)timestamps[i + 1].tv_nsec / 1e9);

                double nextBlinkStartTimeSec = 
                    (double)senderStartTime.tv_sec + 
                    ((double)senderStartTime.tv_nsec / 1e9) +
                    (i + 1) * BLINK_INTERVAL + TimeFix;

                // If the next blink is within the correct range
                if (fabs(nextSensorSawTimeSec - nextBlinkStartTimeSec) <= BLINK_INTERVAL) 
                {
                    continue;  // Skip the current blink
                }
                if (fabs(sensorSawTimeSec - nextBlinkStartTimeSec) <= BLINK_INTERVAL) 
                {
                    TimeFix += BLINK_INTERVAL;
                    delaysCalculated[i] = sensorSawTimeSec - BLINK_INTERVAL + blinkStartTimeSec;
                    continue;
                }
                else
                {
                    continue;  // Skip the current blink if both are out of sync
                }
            }
            else 
            {
                continue;  // No next blink to check, so skip this one
            }
        }
		if (sensorSawTimeSec > blinkStartTimeSec)
		{
			delaysCalculated[i] = sensorSawTimeSec - blinkStartTimeSec;
		}
        
	}

    return delaysCalculated;
}

void setArrayToZero(double *array)
{
    
    for (int i = 0; i < BLINK_COUNT; i++)
    {
        array[i] = 0.0;
    }
}

double calculateAverage(double *data, int *count)
{
    double sum = 0.0;

	for (int i = 0; i < BLINK_COUNT; i++)
	{
		if (data[i] != 0.0)
		{
			sum += data[i]; // Add the value to the sum
			(*count)++; // Increment the count of values
		}
	}

    // Check for division by zero
    if (*count == 0) {
        return 0.0; // or handle it as an error
    }

    return sum / *count; // Return the average
}
