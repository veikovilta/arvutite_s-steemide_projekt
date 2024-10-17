#include <gpiod.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>

void* ledBlinking20(void* arg) {
    struct args_port* args = (struct args_port*) arg;
    struct port *openedPort = openPort(args->portPin, args->debugName, args->inputOutput);

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

//~ void* ledBlinkOnce_thread(void* arg) {
    //~ struct args_port* args = (struct args_port*) arg;
    //~ struct port *openedPort = openPort(args->portPin, args->debugName, args->inputOutput);

    //~ if (openedPort == NULL) {
        //~ return NULL;
    //~ }

    //~ gpiod_line_set_value(openedPort->line, 1);
    //~ preciseSleep(1);

	//~ gpiod_line_set_value(openedPort->line, 0);

    //~ gpiod_line_release(openedPort->line);
    //~ gpiod_chip_close(openedPort->chip);
    //~ free(openedPort);

    //~ return NULL;
//~ }

timespec* ledBlinkOnce(struct* args_port newPort) {
    // Open the port for controlling the LED
    struct port *openedPort = openPort(newPort->portPin, newPort->debugName, newPort->inputOutput); // 'true' indicates output
	struct timespec blinkTime;

    if (openedPort == NULL) {
        printf("Failed to open port for LED\n");
        return;
    }

    // Turn the LED on
    gpiod_line_set_value(openedPort->line, 1);
    clock_gettime(CLOCK_REALTIME, &blinkTime);
    preciseSleep(1);  // Sleep for 1 second

    // Turn the LED off
    gpiod_line_set_value(openedPort->line, 0);

    // Clean up
    gpiod_line_release(openedPort->line);
    gpiod_chip_close(openedPort->chip);
    free(openedPort);
    
    return &blinkTime;
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
