#ifndef SENSOR_H
#define SENSOR_H

#define GPIO_PIN_LED 22 // input from the LED
#define BLINK_COUNT 10 // Number of LED blinks to read
#define WAIT_TIME_BEFORE_NEXT_MINUTE 10
#define BLINK_INTERVAL 2.0 // Blink interval in seconds

double* calculateDelays(const struct timespec *timestamps,
const struct timespec senderStartTime); 
void setArrayToZero(double *array);
double calculateAverage(double *data, int *count);
double CalculateDelaySingle(struct timespec timestamp, struct timespec senderStartTime, int numOfBlink); 
double* RegisterBlinks(char** buffer, int *count); 
void CountBlinks();

#endif
