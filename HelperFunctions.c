#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include "LedBlink.h"

struct port* openPort(int port, char* debugName) {
    struct port* newPort = (struct port*) malloc(sizeof(struct port));
    if (newPort == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Open GPIO chip
    newPort->chip = gpiod_chip_open_by_number(port);
    if (!newPort->chip) {
        perror("Open GPIO chip failed");
        free(newPort);
        return NULL;
    }

    // Get GPIO line
    newPort->line = gpiod_chip_get_line(newPort->chip, port);
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
            printf("Sleep interrupted. Remaining: %lld seconds\n", rem.tv_sec);
        }
        else {
            // Handle other potential errors
            perror("clock_nanosleep failed");
        }
    }
}