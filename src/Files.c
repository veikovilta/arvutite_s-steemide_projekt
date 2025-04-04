#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Files.h"

void append_to_buffer(char **buffer, const char *data) {
    static int current_size = 0;
    static int buffer_capacity = 0;

    int data_length = (int)strlen(data);

    // Initialize the buffer if it's the first call
    if (*buffer == NULL) {
        *buffer = malloc(INITIAL_BUFFER_SIZE);
        if (!*buffer) {
            fprintf(stderr, "Failed to allocate initial buffer.\n");
            exit(EXIT_FAILURE);
        }
        buffer_capacity = INITIAL_BUFFER_SIZE;
        (*buffer)[0] = '\0';  // Start with an empty string
    }

    // Resize buffer if needed
    if (current_size + data_length >= buffer_capacity) {
        // Double the buffer capacity or increase until it fits the data
        int new_capacity = buffer_capacity * 2;
        while (current_size + data_length >= new_capacity) {
            new_capacity *= 2;
        }

        char *new_buffer = realloc(*buffer, (size_t)new_capacity);
        if (!new_buffer) {
            fprintf(stderr, "Failed to reallocate memory for buffer.\n");
            free(*buffer);
            exit(EXIT_FAILURE);
        }

        *buffer = new_buffer;
        buffer_capacity = new_capacity;
    }

    // Append data to the buffer
    strcat(*buffer + current_size, data);
    current_size += data_length;
}

void write_log_to_file(const char *buffer) {
    // Step 1: Create the log directory if it doesn't exist
    const char *log_dir = "../log/breadboardtesting";
    struct stat st = {0};

    if (stat(log_dir, &st) == -1) {
        if (mkdir(log_dir, 0755) != 0) {
            perror("Failed to create log directory");
            return;
        }
    }

    // Step 2: Get the current date to create a unique filename
    char filename[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // Format filename as "log_<YYYY-MM-DD>.txt" in the log directory
    strftime(filename, sizeof(filename), "../log/breadboardtesting/log_%Y-%m-%d_%H-%M-%S.txt", t);
    
    // Step 3: Open the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }

    // Step 4: Write the buffer contents to the file
    fwrite(buffer, sizeof(char), strlen(buffer), file);

    // Step 5: Close the file
    fclose(file);
    printf("Log written to %s\n", filename);
}

char* getCurrentTimestamp(const char *prefix) {
    // Get the current time
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // Convert the seconds part to a human-readable format
    struct tm *timeinfo = localtime(&ts.tv_sec);

    // Format the timestamp part
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    snprintf(timestamp + 19, sizeof(timestamp) - 19, ".%09ld", ts.tv_nsec);

    // Calculate the total size needed for the prefix + timestamp + null terminator
    size_t totalSize = strlen(prefix) + strlen(timestamp) + 1;

    // Allocate memory for the result
    char *buffer = (char *)malloc(totalSize);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Combine the prefix and the timestamp into the buffer
    snprintf(buffer, totalSize, "%s%s", prefix, timestamp);

    return buffer;
}

void TimeStampToBuffer(char **buffer, const char* prefix)
{
    char *timeStampMessage = getCurrentTimestamp(prefix); 
    append_to_buffer(buffer, timeStampMessage);
    append_to_buffer(buffer, "\n");
    free(timeStampMessage); 
}

char* getCurrentTimestampWithTime(const char *prefix, struct timespec ts) {
    // Convert the seconds part to a human-readable format
    struct tm *timeinfo = localtime(&ts.tv_sec);

    // Format the timestamp part
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    snprintf(timestamp + 19, sizeof(timestamp) - 19, ".%09ld", ts.tv_nsec);

    // Calculate the total size needed for the prefix + timestamp + null terminator
    size_t totalSize = strlen(prefix) + strlen(timestamp) + 1;

    // Allocate memory for the result
    char *buffer = (char *)malloc(totalSize);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Combine the prefix and the timestamp into the buffer
    snprintf(buffer, totalSize, "%s%s", prefix, timestamp);

    return buffer;
}

void TimeStampToBufferWithTime(char **buffer, const char* prefix, struct timespec ts)
{
    char *timeStampMessage = getCurrentTimestampWithTime(prefix, ts);  
    append_to_buffer(buffer, timeStampMessage);
    append_to_buffer(buffer, "\n");
    free(timeStampMessage); 
}
