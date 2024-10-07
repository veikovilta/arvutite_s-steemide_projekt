#include <gpiod.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>

void* ledBlinking(void* arg) {
    struct thread_args* args = (struct thread_args*) arg;
    struct port *openedPort = openPort(args->port, args->debugName);

    if (openedPort == NULL) {
        return NULL;
    }

    int i;
    for (i = 0; i < 60; i++) {
        gpiod_line_set_value(openedPort->line, 1);
        preciseSleep(1);
        gpiod_line_set_value(openedPort->line, 0);
        preciseSleep(2);
    }

    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);

    return NULL;
}

/*int main() {
    pthread_t thread;
    struct thread_args args;

    // Initialize thread arguments
    args.port = 15;                 // Set GPIO port number
    args.debugName = "LEDController";  // Set debug name

    // Create a thread with the port number and debug name
    if (pthread_create(&thread, NULL, ledBlinking, (void*)&args) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    return 0;
}*/
