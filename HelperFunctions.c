#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <gpiod.h> // Add this line in your HelperFunctions.h and Main.c files if not already present


struct port* openPort(int portPin, char* debugName) {
    struct port* newPort = (struct port*) malloc(sizeof(struct port));
    if (newPort == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Open GPIO chip
    newPort->chip = gpiod_chip_open_by_number(portPin);
    if (!newPort->chip) {
        perror("Open GPIO chip failed");
        free(newPort);
        return NULL;
    }

    // Get GPIO line
    newPort->line = gpiod_chip_get_line(newPort->chip, portPin);
    if (!newPort->line) {
        perror("Get GPIO line failed");
        gpiod_chip_close(newPort->chip);
        free(newPort);
        return NULL;
    }

    // Request line as output
    int lineRequestReturn = gpiod_line_request_output(newPort->line, debugName, 0);
    if (lineRequestReturn < 0) {
        perror("Request line as output failed");
        gpiod_chip_close(newPort->chip);
        free(newPort);
        return NULL;
    }

    return newPort;
}

void preciseSleep(int seconds) {
    struct timespec req, rem;

    // Set the timespec structure with the requested sleep time
    req.tv_sec = seconds;
    req.tv_nsec = 0;  // No nanoseconds since we are working with whole seconds

    // Use clock_nanosleep with CLOCK_MONOTONIC for precision and robustness
    int ret = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);

    if (ret != 0) {
        if (ret == EINTR) {
            // Interrupted by a signal handler, display remaining time
            printf("Sleep interrupted. Remaining: %ld seconds\n", rem.tv_sec);
        }
        else {
            // Handle other potential errors
            perror("clock_nanosleep failed");
        }
    }
}

void* readButtonState_thread(void* arg) {
    struct args_port* args = (struct args_port*) arg;
    struct port *openedPort = openPort(args->portPin, args->debugName);

    if (openedPort == NULL) {
        return NULL;
    }

    gpiod_line_request_input(openedPort->line, "button_monitor");

    while (1) {
        int value = gpiod_line_get_value(openedPort->line);
        if (value < 0) {
            perror("Failed to read button state");
            break; // Exit the loop on error
        }

        if (value == 1) {
            printf("Button is pressed!\n");
        } else {
            printf("Button is not pressed.\n");
        }

        usleep(200000);
    }

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return NULL;
}

int readButtonState(struct args_port* args) {
    
    struct port *openedPort = openPort(args->portPin, args->debugName);

    if (openedPort == NULL) {
        fprintf(stderr, "Failed to open port.\n");
        return -1;
    }

    gpiod_line_request_input(openedPort->line, "button_read");

    int value = gpiod_line_get_value(openedPort->line);
    if (value < 0) {
        perror("Failed to read button state");
        gpiod_line_release(openedPort->line);
        gpiod_chip_close(openedPort->chip);
        free(openedPort);
        return -1;
    }

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return value;
}
