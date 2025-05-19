#ifndef FILES_H
#define FILES_H

#define INITIAL_BUFFER_SIZE 1024 // Define an initial buffer size

/**
 * @brief Appends data to a dynamically allocated buffer, resizing as needed.
 *
 * Ensures the buffer is large enough to hold the new data. If not, reallocates with increased capacity.
 * Maintains static variables for current size and capacity.
 *
 * @param buffer Pointer to the buffer pointer to append data to.
 * @param data The string data to append.
 */
void append_to_buffer(char **buffer, const char *data);

/**
 * @brief Writes the contents of the buffer to a log file.
 *
 * Creates a dated directory if it does not exist, then writes the buffer to a file named with the current time.
 *
 * @param buffer The buffer containing the log data to write.
 */
void write_log_to_file(const char *buffer); 

/**
 * @brief Returns a formatted timestamp string with a prefix.
 *
 * Allocates and returns a string containing the prefix followed by the current date and time with nanosecond precision.
 * The caller is responsible for freeing the returned string.
 *
 * @param prefix The prefix to prepend to the timestamp.
 * @return Pointer to the allocated timestamp string.
 */
char* getCurrentTimestamp();

/**
 * @brief Appends a timestamp to the buffer with a given prefix.
 *
 * Calls getCurrentTimestamp to get the formatted timestamp and appends it to the buffer.
 *
 * @param buffer Pointer to the buffer pointer to append the timestamp to.
 * @param prefix The prefix to prepend to the timestamp.
 */
void TimeStampToBuffer(char **buffer, const char* prefix);

/**
 * @brief Appends a timestamp with a prefix and specific time to the buffer.
 *
 * Generates a timestamp string with the given prefix and specified time, then appends it to the buffer, followed by a newline.
 *
 * @param buffer Pointer to the buffer pointer to append the timestamp to.
 * @param prefix The prefix to prepend to the timestamp.
 * @param ts The timespec structure specifying the time.
 */
void TimeStampToBufferWithTime(char **buffer, const char* prefix, struct timespec ts);

/**
 * @brief Returns a formatted timestamp string with a prefix and a specific time.
 *
 * Allocates and returns a string containing the prefix followed by the specified date and time with nanosecond precision.
 * The caller is responsible for freeing the returned string.
 *
 * @param prefix The prefix to prepend to the timestamp.
 * @param ts The timespec structure specifying the time.
 * @return Pointer to the allocated timestamp string.
 */
char* getCurrentTimestampWithTime(const char *prefix, struct timespec ts);

#endif
