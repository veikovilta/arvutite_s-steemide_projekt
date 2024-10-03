#include "Main.h"

void* sync_with_chrony(void* arg);
void* other_jobs(void* arg);

//jagatud muutuja bool
bool loop = true;
int currentTime = clock_gettime(CLOCK_REALTIME) 

int main(void)
{
    //väljakutse chrony start
    system("chronyc systemctl start");
    if (pin15 = high)
    {
        /* siis on vastuvõtja */
    }
    else
    {
        /* siis on saatja */
    }
    
    //check if sync
    //sleep 60 sec vnii chrony salvestamiseks
    //pärast checki kui hea ka

    //lediga näitama et on cynced ja ready (mõlemal)

    //järgmisene vajutad start ja sellest läheb 
    // käima järgmise täis minutiga test protsess

    pthread_t syncThread, jobThread, displayThread;

    if(pthread_create(&syncThread, NULL, syncWithChrony, NULL) != 0){
        perror("Failed to create thread");
        return 1;
    }
    
    // Create a thread for other jobs
    if(pthread_create(&jobThread, NULL, otherJobs, NULL) != 0){
        perror("Failed to create thread");
        return 1;
    }
    
    //create thread for displaying info on the screen
    if(pthread_create(&displayThread, NULL, displayInfo , NULL) != 0){
        perror("Failed to create thread");
        return 1;
    }
    // Wait for the threads to complete (they won't in this case)
    pthread_join(sync_thread, NULL);
    pthread_join(job_thread, NULL);
    pthread_join(displayThread, NULL);

    return 0;
}





void * displayInfo(int pinout)
{
    displayToScreen();

    return NULL;
}

void* sync_with_chrony(void* arg) {
    if (check == )
    while (loop) {
        // Sync system time with Chrony
        current_time = clock_gettime(CLOCK_REALTIME, ....);

        // Wait for 2 seconds before syncing again
        preciseSleep(2);
    }
    return NULL;
}

void* other_jobs(void* arg) {
    int i = 0;
    array[20];
    for(i = 0; i < 60; i++)
    {
        pin1 = high;
        array[i] = current_time; //debug
        preciseSleep(1); // teeme oma sleep funktsiooni kasutades current time ülemine asi
        pin1 = low;
        preciseSleep(2)
    }
    SaveFile(); //debug
    
    return NULL;
}

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