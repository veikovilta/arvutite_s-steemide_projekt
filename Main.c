#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "stdbool.h"

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
        sleep(2);
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
        sleep(1); // teeme oma sleep funktsiooni kasutades current time ülemine asi
        pin1 = low;
        sleep(2)
    }
    SaveFile(); //debug
    
    return NULL;
}