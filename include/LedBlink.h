#ifndef LEDBLINK_H
#define LEDBLINK_H

#include <gpiod.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>

#define GPIO_LINE_MAIN_BLINK 17 
#define BLINK_COUNT_CALIBRATION 100
#define BLINK_COUNT_MAIN_PROGRAM 10 

/**
 * @brief Runs the LED blinking calibration routine.
 *
 * Blinks the LED a specified number of times with fixed intervals.
 * Displays calibration messages on the OLED and allows the user to stop the process by pressing a button.
 *
 * @param blinkCount Number of times to blink the LED.
 */
void ledBlinkingCalibration(int blinkCount);

/**
 * @brief Runs the main LED blinking sequence and logs timestamps.
 *
 * Blinks the LED a fixed number of times, records the timestamp of each blink,
 * and appends the timestamps to the provided buffer.
 *
 * @param args Pointer to the args_port structure with port configuration.
 * @param buffer Pointer to the buffer pointer for logging timestamps.
 */
void ledBlinkingMain(struct args_port* args, char** buffer);

/**
 * @brief Blinks the LED once and logs the timestamp.
 *
 * Turns the LED on, records the current time, logs the timestamp to the buffer,
 * waits for 1 second, then turns the LED off and closes the port.
 *
 * @param newPort Pointer to the args_port structure with port configuration.
 * @param buffer Pointer to the buffer pointer for logging the timestamp.
 * @return The timespec structure containing the timestamp of the blink.
 */
struct timespec ledBlinkOnce(struct args_port *newPort, char** buffer);

#endif
