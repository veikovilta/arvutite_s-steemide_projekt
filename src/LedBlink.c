#include <gpiod.h>
#include "LedBlink.h"
#include "HelperFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <gpiod.h>
#include "Files.h"
#include <time.h>


void ledBlinking20(struct args_port* args, char* buffer)
{
    struct port *openedPort = openPort(args->portPin, args->debugName, false); // 'true' for output

    for (int i = 0; i < 20; i++) {
        // Turn the LED on
        gpiod_line_set_value(openedPort->line, 1);
        printf("Blink number: %d\n", i + 1); // Print the current blink count
        fflush(stdout);
        TimeStampToBuffer(buffer, "Blink: ");
        preciseSleep(1); // Sleep for 1 second
        
        // Turn the LED off
        gpiod_line_set_value(openedPort->line, 0);
        preciseSleep(1); // Sleep for 2 seconds
    }

    // Clean up
    ClosePort(openedPort);
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
//~

struct timespec ledBlinkOnce(struct args_port *newPort, char* buffer) {
    // Open the port for controlling the LED
    struct port *openedPort = openPort(newPort->portPin, newPort->debugName, false); // 'FALSE' for output
    static struct timespec blinkTime; // Use static to return a pointer safely

    // Turn the LED on
    gpiod_line_set_value(openedPort->line, 1);
    clock_gettime(CLOCK_REALTIME, &blinkTime);
    TimeStampToBuffer(buffer, "First sync blink: ");
    preciseSleep(1);  // Sleep for 1 second

    // Turn the LED off
    gpiod_line_set_value(openedPort->line, 0);

    ClosePort(openedPort); 
    
    return blinkTime; // Return the address of blinkTime
}

/*int main() {
    pthread_t thread;
    struct thread_args args;

    // Initialize thread arguments
    args.port = 15;                 // Set GPIO port number    args.debugName = "LEDController";  // Set debug name

    // Create a thread with the port number and debug name
    if (pthread_create(&thread, NULL, ledBlinking, (void*)&args) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    return 0;
}*/