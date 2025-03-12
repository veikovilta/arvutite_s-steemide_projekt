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
#define BLINK_COUNT_CALIBRATION 5
#define BLINK_COUNT_MAIN_PROGRAM 20 

void ledBlinking20(struct args_port* args, char** buffer);
//void* ledBlinkOnce(void* arg);
struct timespec ledBlinkOnce(struct args_port *newPort, char** buffer);
void ledBlinkingCalibration(int blinkCount);

#endif
