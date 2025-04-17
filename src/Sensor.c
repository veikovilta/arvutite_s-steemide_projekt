#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <gpiod.h>
#include <math.h> 
#include "Sensor.h"
#include "HelperFunctions.h"
#include "display.h"
#include "Files.h"
#include "LedBlink.h"

int CountBlinks()
{
    //(void)i2cHandle;
    int blinkCount = 0; 
    struct port *openedPort = openPort(GPIO_PIN_LED, "GPIO PIN 22", true);
    
    for (int i = 0; i < BLINK_COUNT_CALIBRATION; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) 
        {
            preciseSleep(0.001);  
        }

        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) == 1) 
        {
            preciseSleep(0.3);
        }

        blinkCount++;
    }

    ClosePort(openedPort);
	
	return blinkCount; 
}

double* RegisterBlinks(char** buffer, int *count)
{
    char numberStr[20] = "";
    struct args_port* args = (struct args_port*) args;
    struct port *openedPort = openPort(GPIO_PIN_LED, "GPIO PIN 22", true);
	double* delaysCalculated = (double*)malloc(BLINK_COUNT * sizeof(double)); 
    // Wait for the first blink
    while (gpiod_line_get_value(openedPort->line) == 0) 
    {
        preciseSleep(0.1); 
    }
    printf("Got first blink, sleeping until next full minute\n");
    //oledClear(i2cHandle);
    //oledWriteText(i2cHandle, 0, 0, "Got first blink");
    SetOledMessage("Got first blink", 0, 0, true);
    TimeStampToBuffer(buffer, "Got first blink: ");
	
    struct timespec currentTime;
    // Get the current time with high precision
    clock_gettime(CLOCK_REALTIME, &currentTime);

    // Extract the seconds part of the current time
    long currentSeconds = currentTime.tv_sec % 60;

    if (60 - (currentSeconds) < 10)
    {
        preciseSleep(11);
    }
    while (1)
    {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        
        if ((currentTime.tv_sec % 60) == 0 && (currentTime.tv_nsec < 1e6))
        {
            break;
        }
        
        preciseSleep(0.0001);
    }
    //oledClear(i2cHandle);
    //oledWriteText(i2cHandle, 0, 0, "Got :");
    
    struct timespec senderStartTime; 
    
    clock_gettime(CLOCK_REALTIME, &senderStartTime);
    TimeStampToBufferWithTime(buffer, "Sender start time: ", senderStartTime); 
        
    struct timespec timestamps[BLINK_COUNT]; 
    int i; 

    int breakCounter = 0; 
    // Read the LED blinks and record timestamps
    for (i = 0; i < BLINK_COUNT; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) 
        {
            preciseSleep(0.001);
            breakCounter++; 
            if(breakCounter > 4000)
            {
				breakCounter = -1; 
				SetOledMessage("Stopped, 4sec", 0, 0, true); 
				TimeStampToBuffer(buffer, "Sensor stopped due to no blinking after 4sec");				
				preciseSleep(1);
            	break;
            }  
        }

        // Get current time with high precision
        clock_gettime(CLOCK_REALTIME, &timestamps[i]);

        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) == 1) 
        {
            preciseSleep(0.3);
        }

		if(breakCounter == -1)
		{
			break; 
		}
		
        delaysCalculated[i] = CalculateDelaySingle(timestamps[i], senderStartTime, i);
		breakCounter = 0; 
        TimeStampToBufferWithTime(buffer, "Seen at: ", timestamps[i]);
        sprintf(numberStr, "Delay: %.5f ms\n", delaysCalculated[i]);
        SetOledMessage(numberStr, 0, 2, true);
        append_to_buffer(buffer, numberStr); 

        printf("Got %d\n", i + 1);
    }

    printf("Got all data\n");
	*count = i; 
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);
	
	return delaysCalculated; 
}

double CalculateDelaySingle(struct timespec timestamp, struct timespec senderStartTime, int numOfBlink)
{
    double TimeFix = 0.0; // kui läheb syncist välja siis kasutan
        
    double sensorSawTimeSec = 
        (double)timestamp.tv_sec + 
        ((double)timestamp.tv_nsec / 1e9);

    double blinkStartTimeSec = 
        (double)senderStartTime.tv_sec + 
        ((double)senderStartTime.tv_nsec / 1e9) +
        (numOfBlink * BLINK_INTERVAL) + TimeFix;
    
    double result = -1.0;  

	int i = 0; 

	while(sensorSawTimeSec < blinkStartTimeSec)
	{

		blinkStartTimeSec -= BLINK_INTERVAL;
		
		if(i > BLINK_INTERVAL)
		{
			return result;
		}
	} 
	
    if (sensorSawTimeSec > blinkStartTimeSec)
    {
        result = sensorSawTimeSec - blinkStartTimeSec;
	
		result = result * 1000;
    }

    while (result >= 2000)
    {
    	result -= 2000;
    }

    return result - (numOfBlink * 0.19); 
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

	for (int i = 0; i < *count; i++)
	{
		if (data[i] != 0.0)
		{
			sum += data[i]; // Add the value to the sum
		}
	}

    // Check for division by zero
    if (*count == 0) {
        return 0.0; // or handle it as an error
    }

    return sum / *count; // Return the average
}
