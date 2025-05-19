#ifndef SENSOR_H
#define SENSOR_H

#define GPIO_PIN_LED 22 // input from the LED
#define BLINK_COUNT 10 // Number of LED blinks to read
#define WAIT_TIME_BEFORE_NEXT_MINUTE 10
#define BLINK_INTERVAL 2.0 // Blink interval in seconds

/**
 * @brief Counts LED blinks for calibration and displays progress on the OLED.
 *
 * Waits for a specified number of blinks on a GPIO pin, updating the OLED display with the count.
 * Allows the user to stop the process early by pressing a button.
 */
void CountBlinks();

/**
 * @brief Registers LED blinks, records timestamps, and calculates delays.
 *
 * Waits for a series of blinks on a GPIO pin, records the timestamp of each blink,
 * calculates the delay from the sender's start time, and logs the results.
 * Stops if no blink is detected for a set period.
 *
 * @param buffer Pointer to the buffer pointer for logging.
 * @param count Pointer to an integer to store the number of blinks detected.
 * @return Pointer to an array of calculated delays for each blink.
 */
double* RegisterBlinks(char** buffer, int *count); 

/**
 * @brief Calculates the delay between the sender's blink and the sensor's detection.
 *
 * Computes the time difference in milliseconds between when the sender started the blink
 * and when the sensor detected it, adjusting for synchronization and blink intervals.
 *
 * @param timestamp The timespec when the blink was detected by the sensor.
 * @param senderStartTime The timespec when the sender started blinking.
 * @param numOfBlink The index of the current blink.
 * @return The calculated delay in milliseconds.
 */
double CalculateDelaySingle(struct timespec timestamp, struct timespec senderStartTime, int numOfBlink); 

/**
 * @brief Sets all elements of a double array to zero.
 *
 * Initializes the provided array to zero for all BLINK_COUNT elements.
 *
 * @param array Pointer to the array to zero out.
 */
void setArrayToZero(double *array);

/**
 * @brief Calculates the average of the data array.
 *
 * Computes the average of non-zero elements in the provided data array.
 *
 * @param data Pointer to the array of data.
 * @param count Pointer to the number of elements to consider.
 * @return The average value, or 0.0 if count is zero.
 */
double calculateAverage(double *data, int *count);

#endif
