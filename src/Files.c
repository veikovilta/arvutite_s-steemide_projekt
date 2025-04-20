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

    // Reset static variables if the buffer is NULL
    if (*buffer == NULL) {
        current_size = 0;
        buffer_capacity = 0;
    }

    // Initialize the buffer if it's the first call or after being reset
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
    // Step 1: Base path to the main log folder
    const char *base_path = "/home/reval/Documents/reval/arvutite_s-steemide_projekt/log";

    // Step 2: Get current date for subdirectory
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char dated_dir[256];
    strftime(dated_dir, sizeof(dated_dir), "%Y-%m-%d", t);

    // Step 3: Combine base path and dated directory
    char full_dir[512];
    snprintf(full_dir, sizeof(full_dir), "%s/%s", base_path, dated_dir);

    // Step 4: Create the dated directory if it doesn't exist
    struct stat st = {0};
    if (stat(full_dir, &st) == -1) {
        if (mkdir(full_dir, 0777) != 0) {
            perror("Failed to create dated log directory");
            return;
        }
    }

    // Step 5: Create the log filename (based on time)
    char time_filename[64];
    strftime(time_filename, sizeof(time_filename), "log_%H:%M:%S.txt", t);

    // Step 6: Combine full path to file
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", full_dir, time_filename);

    // Step 7: Open and write
    FILE *file = fopen(full_path, "w");
    if (file == NULL) {
        perror("Failed to open log file");
        return;
    }

    fwrite(buffer, sizeof(char), strlen(buffer), file);
    fclose(file);

    printf("Log written to %s\n", full_path);
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
