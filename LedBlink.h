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

void ledBlinking20(struct args_port* args);
//void* ledBlinkOnce(void* arg);
struct timespec ledBlinkOnce(struct args_port *newPort);

#endif
