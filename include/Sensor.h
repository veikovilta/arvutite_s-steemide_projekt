#ifndef SENSOR_H
#define SENSOR_H

#define GPIO_PIN_LED 22 // input from the LED
#define BLINK_COUNT 20 // Number of LED blinks to read
#define WAIT_TIME_BEFORE_NEXT_MINUTE 10
#define BLINK_INTERVAL 2.0 // Blink interval in seconds

double* RegisterBlinks(int i2cHandle);
double* calculateDelays(const struct timespec *timestamps,
const struct timespec senderStartTime); 
void setArrayToZero(double *array);
double calculateAverage(double *data, int *count);


#endif
