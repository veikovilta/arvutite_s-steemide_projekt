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

void CountBlinks() {
    SetSystemState("CALIBRATE-->RECEIVER");
    
    int end = 0;
    struct port *openedPort = openPort(GPIO_PIN_LED, "GPIO PIN 22", true);
    char numberStr[20] = "";
    SetOledMessage("Press button to end", 0, 0, true);
    preciseSleep(0.5);
    sprintf(numberStr, "Blink %d detected\n", 0);
    SetOledMessage(numberStr, 0, 2, false);
    for (int i = 0; i < BLINK_COUNT_CALIBRATION; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) {
            preciseSleep(0.001);
            if (IsButtonPressed())
            {
                SetOledMessage("Calibration stopped", 0, 0, true);
                end = -1;
                break;
            }
        }

        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) == 1) {
            preciseSleep(0.3);
        }

        if (end == -1) {
            break;
        }

        sprintf(numberStr, "Blink %d detected\n", i + 1);
        SetOledMessage(numberStr, 0, 2, false);
    }

    ClosePort(openedPort);
    SetSystemState("CALIBRATE-->MODE MENU");
    return;
}

double* RegisterBlinks(char **buffer, int *count) {
    char numberStr[20] = "";
    struct port *openedPort = openPort(GPIO_PIN_LED, "GPIO PIN 22", true);
    double *delaysCalculated = (double *)malloc(BLINK_COUNT * sizeof(double));

    struct timespec timestamps[BLINK_COUNT];
    struct timespec senderStartTime;
    struct timespec currentTime;

    int breakCounter = 0;
    int i;

    // Wait for the first blink
    while (gpiod_line_get_value(openedPort->line) == 0) {
        preciseSleep(0.1);
    }

    TimeStampToBuffer(buffer, "Got first blink: ");
    // Get the current time with high precision
    clock_gettime(CLOCK_REALTIME, &currentTime);

    WaitForNextMinute(currentTime);
    
    clock_gettime(CLOCK_REALTIME, &senderStartTime);

    SetOledMessage("Waiting blinks ...", 0, 0, true);

    // Read the LED blinks and record timestamps
    for (i = 0; i < BLINK_COUNT; i++) {
        // Wait for the GPIO pin to go HIGH
        while (gpiod_line_get_value(openedPort->line) == 0) {
            preciseSleep(0.001);
            breakCounter++;

            // Stop if no blinking is detected for 4 seconds
            if (breakCounter > 4000) {
                breakCounter = -1;
                SetOledMessage("Stopped, 4sec", 0, 0, true);
                TimeStampToBuffer(buffer, "Sensor stopped due to no blinking after 4sec: ");
                preciseSleep(1);
                break;
            }
        }

        // Get current time with high precision
        clock_gettime(CLOCK_REALTIME, &timestamps[i]);

        // Wait for the GPIO pin to go LOW
        while (gpiod_line_get_value(openedPort->line) == 1) {
            preciseSleep(0.3);
        }

        if (breakCounter == -1) {
            break;
        }

        // Calculate delay for the current blink
        delaysCalculated[i] = CalculateDelaySingle(timestamps[i], senderStartTime, i);
        breakCounter = 0;

        // Log the timestamp and delay
        sprintf(numberStr, "Delay: %.5f ms\n", delaysCalculated[i]);
        SetOledMessage(numberStr, 0, 0, true);

        printf("Got %d\n", i + 1);
    }

    printf("Got all data\n");
    *count = i;

    TimeStampToBufferWithTime(buffer, "Sender start time: ", senderStartTime);

    for (int j = 0; j < i; j++)
    {
        TimeStampToBufferWithTime(buffer, "Seen at: ", timestamps[j]);
        sprintf(numberStr, "Delay: %.5f ms\n", delaysCalculated[j]);
        append_to_buffer(buffer, numberStr);
    }
    
    // Clean up resources
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return delaysCalculated;
}

double CalculateDelaySingle(struct timespec timestamp, struct timespec senderStartTime, int numOfBlink) {
    double TimeFix = 0.0; // Adjustment for synchronization issues

    double sensorSawTimeSec =
        (double)timestamp.tv_sec +
        ((double)timestamp.tv_nsec / 1e9);

    double blinkStartTimeSec =
        (double)senderStartTime.tv_sec +
        ((double)senderStartTime.tv_nsec / 1e9) +
        (numOfBlink * BLINK_INTERVAL) + TimeFix;

    double result = -1.0;
    int i = 0;

    // Adjust for synchronization issues
    while (sensorSawTimeSec < blinkStartTimeSec) {
        blinkStartTimeSec -= BLINK_INTERVAL;

        if (i > BLINK_INTERVAL) {
            return result;
        }
    }

    if (sensorSawTimeSec > blinkStartTimeSec) {
        result = sensorSawTimeSec - blinkStartTimeSec;
        result = result * 1000; // Convert to milliseconds
    }

    // Handle large delays
    while (result >= 2000) {
        result -= 2000;
    }

    return result - (numOfBlink * 0.19);
}

void setArrayToZero(double *array) {
    for (int i = 0; i < BLINK_COUNT; i++) {
        array[i] = 0.0;
    }
}

double calculateAverage(double *data, int *count) {
    double sum = 0.0;

    for (int i = 0; i < *count; i++) {
        if (data[i] != 0.0) {
            sum += data[i];
        }
    }

    // Check for division by zero
    if (*count == 0) {
        return 0.0;
    }

    return sum / *count;
}
