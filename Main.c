#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <gpiod.h> 
#include "stdbool.h"
#include "HelperFunctions.h"
#include "display.h"
#include <string.h>
#include "Main.h"
#include "display.h"
#include "LedBlink.h"


#define WAIT_TIME_BEFORE_NEXT_MINUTE 10

int main(void)
{
    //väljakutse chrony start
    // Correct this command
    //~ system("sudo chronyc systemctl start");
    //~ printf("1st check");


    //~ int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
    //~ if (i2cHandle < 0) return -1; // Exit if failed
        //~ printf("2nd check");

    //~ oledInit(i2cHandle); // Initialize the OLED
    //~ oledClear(i2cHandle); // Clear the display

    // eraldi thread enne käima mis checkib nuppu
    //~ pthread_t buttonThread;
    //~ struct args_port args;

    //~ // Initialize thread arguments
    //~ args.portPin = GPIO_BUTTON; // Set GPIO port number
    //~ args.debugName = "InputButton";  // Set debug name
	//~ args.inputOutput = false;

    //~ if(pthread_create(&buttonThread, NULL, debounceButtonState, (void*)&args) < 0)
    //~ {
        //~ perror("Failed to create thread");
        //~ oledClear(i2cHandle);
        //~ oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread");
        //~ oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        //~ system ("sudo shutdown -h now"); 
        //~ return 1;
    //~ }

    // teeb 60 sekundilist checki kui hea kell on
    // 10min vähemalt
    int minutes = 0;
    
    printf("Starting syrcronization"); 
    fflush(stdout);
    while(1)
    {
        preciseSleep(5);
        int i = 5; 
        //CheckSync(i2cHandle) == 0
        if ( CheckSync() == 0)
        {
            printf("hello\n");
            fflush(stdout);
            break;
        }
        else
        {
            printf("notsynced\n");
            fflush(stdout);
        }
    }
    
    printf("hello 2\n");
    fflush(stdout);
    
    //lediga näitama et on cynced ja ready (mõlemal)
    ShowReady();

	struct args_port ledBlinkPort; 

	ledBlinkPort.portPin = GPIO_LINE_MAIN_BLINK; 
	ledBlinkPort.debugName = "ledBlink";
	ledBlinkPort.inputOutput = false;

	struct timespec firstblink = ledBlinkOnce(&ledBlinkPort);

    // Print the time
    printf("LED blink time: %ld seconds and %ld nanoseconds\n", 
           (long)firstblink.tv_sec, (long)firstblink.tv_nsec);
    fflush(stdout);
    
    printf("Checking if its less than 10sec to the next full minute\n");
    fflush(stdout);
    
    
    //if (60 - (firstblink.tv_sec % 60) <= 10)
    //{
        //preciseSleep(11);
    //}
    
    printf("Waiting next minute\n");
    fflush(stdout);
    
    struct timespec currentTime; 
    
    //while (1)
    //{
        //clock_gettime(CLOCK_REALTIME, &currentTime);
        
        //if ((currentTime.tv_sec % 60) == 0 && currentTime.tv_nsec < 1e6)
        //{
            //break;
        //}
        
        //preciseSleep(0.0001);
    //}
    
    printf("Start blinking\n");
    fflush(stdout);
    
    ledBlinking20(&ledBlinkPort);
    
    // Wait for the threads to complete
    // TODO exit neile funktsioonidele vaja juurde teha
    //pthread_join(buttonThread, NULL);

    //~ oledClear(i2cHandle);
    //~ oledWriteText(i2cHandle, 0, 0, "Program finished");
    //~ oledWriteText(i2cHandle, 2, 0, "Shutting Down");

    //~ close(i2cHandle);
    //system ("sudo shutdown -h now");
    return 0;
}


//~ void* sync_with_chrony(void* arg) {
    //~ if (check == )
    //~ while (loop) {
        //~ // Sync system time with Chrony
        //~ current_time = clock_gettime(CLOCK_REALTIME, ....);

        //~ // Wait for 2 seconds before syncing again
        //~ preciseSleep(2);
    //~ }
    //~ return NULL;
//~ }

//~ void* other_jobs(void* arg) {
    //~ int i = 0;
    //~ array[20];
    //~ for(i = 0; i < 60; i++)
    //~ {
        //~ pin1 = high;
        //~ array[i] = current_time; //debug
        //~ preciseSleep(1); // teeme oma sleep funktsiooni kasutades current time ülemine asi
        //~ pin1 = low;
        //~ preciseSleep(2)
    //~ }
    //~ SaveFile(); //debug
    
    //~ return NULL;
//~ }

/* kuidas gpiod chipiga kirjutada #define GPIO_CHIP "/dev/gpiochip0"  // GPIO chip device
#define GPIO_LINE 17                // GPIO line (GPIO 17)

int main() {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    // Open the GPIO chip (e.g., /dev/gpiochip0)
    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        perror("Open GPIO chip failed");
        return 1;
    }

    // Get the GPIO line (pin)
    line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) {
        perror("Get GPIO line failed");
        gpiod_chip_close(chip);
        return 1;
    }

    // Request the line as output and set initial value to 0 (low)
    ret = gpiod_line_request_output(line, "myapp", 0);
    if (ret < 0) {
        perror("Request line as output failed");
        gpiod_chip_close(chip);
        return 1;
    }

    // Toggle the GPIO line: ON (1) and OFF (0) every second
    for (int i = 0; i < 10; ++i) {
        // Set the line high (turn on)
        gpiod_line_set_value(line, 1);
        printf("GPIO %d: ON\n", GPIO_LINE);
        sleep(1);

        // Set the line low (turn off)
        gpiod_line_set_value(line, 0);
        printf("GPIO %d: OFF\n", GPIO_LINE);
        sleep(1);
    }

    // Release the line and close the chip
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}*/


    //~ // Use firstblink to calculate waiting time
    //~ long currentSeconds = firstblink.tv_sec % 60;
    //~ long currentNanoseconds = firstblink.tv_nsec;
    //~ long nanosecondsToWait = 0;
    //~ long secondsToWait = 0;
    //~ double totalSecondsToWait = 0;

    //~ // If the seconds are less than 10, wait until the next full minute
    //~ if (currentSeconds < WAIT_TIME_BEFORE_NEXT_MINUTE) {
        //~ // Calculate how many seconds are left until the next full minute
        //~ secondsToWait = 60 - currentSeconds; // Corrected calculation
        //~ nanosecondsToWait = 1000000000 - currentNanoseconds;

        //~ // Adjust the seconds to wait if we are already in the current second
        //~ if (nanosecondsToWait < 1000000000) {
            //~ secondsToWait += 1; // Add a second since we have to wait for the full second
        //~ }
        
        //~ totalSecondsToWait = (double)secondsToWait + ((double)nanosecondsToWait / 1e9);
    //~ } else {
        //~ nanosecondsToWait = 1000000000 - currentNanoseconds;

        //~ // Adjust the seconds to wait if we are already in the current second
        //~ if (nanosecondsToWait < 1000000000) {
            //~ secondsToWait += 1; // Add a second since we have to wait for the full second
        //~ }   
        
        //~ totalSecondsToWait = (double)secondsToWait + (double)currentSeconds + ((double)nanosecondsToWait / 1e9);
    //~ }
    
