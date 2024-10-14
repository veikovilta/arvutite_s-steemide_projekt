#ifndef SENSOR_H
#define SENSOR_H

#define GPIO_PIN 22 // GPIO pin number (BCM) for the input from the LED
#define BLINK_COUNT 20 // Number of LED blinks to read
#define WAIT_TIME_BEFORE_NEXT_MINUTE 10

void RegisterBlinks();

#endif