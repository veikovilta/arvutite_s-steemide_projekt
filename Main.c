#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <gpiod.h> 
#include "stdbool.h"
#include "LedBlink.h"
#include "HelperFunctions.h"
#include "display.h"
#include "Sensor.h"

#include "Main.h"

//jagatud muutuja bool
//~ atomic bool loop = true;
//~ atomic int currentTime = clock_gettime(CLOCK_REALTIME) 

int main(void)
{
    //väljakutse chrony start
    system("chronyc systemctl start");


    // eraldi thread enne käima mis checkib nuppu
    pthread_t buttonThread, displayThread;
    struct args_port args;

    // Initialize thread arguments
    args.portPin = GPIO_BUTTON; // Set GPIO port number
    args.debugName = "InputButton";  // Set debug name
	args.inputOutput = false;
    int result= 0;
    if(pthread_create(&buttonThread, NULL, readButtonState(), (void*)&args) < 0){
        perror("Failed to create thread");
        return 1;
    }
    else
    {
        //button pressed?
    }

    // teeb 60 sekundilist checki kui hea kell on
    // 10min vähemalt
    int minutes = 0;
    while(1)
    {
        preciseSleep(60);
    
        if (CheckSync() == 0)
        {
            break;
        }
        else 
        {
            // kirjuta ekraanile et 60 sek ootama veel
            printf("Syncing for another 1 minute \n%d - minutes waited\n", minutes)
        }
        minutes++;

        if (minutes == 10)
        {
            // kirjuta ekraanile et ei suutnud syncida
            printf("NOT SYNCED \n")
            printf("ERROR BAD RECEPTION\n")
            //shutdown?
            return 0;
        }
        
    }
        //create thread for displaying info on the screen
    if(pthread_create(&displayThread, NULL, displayInfo, NULL) != 0){
        perror("Failed to create thread");
        return 1
    }
    //returnib funktsioon, mis checkib kas on low või high
    //ootab kuni inimene teeb nupuga valiku 
    // kuvab kumb on
    struct args_port mingiNupp = { .portPin = 4, .debugName = "GPIO Port 4", .inputOutput = true};
    int pin15 = readButtonState(&mingiNupp);


    //lediga näitama et on cynced ja ready (mõlemal)
    ShowReady();

    RegisterBlinks();
    
    // Wait for the threads to complete (they won't in this case)
    //~ pthread_join(sync_thread, NULL);
    pthread_join(displayThread, NULL);
    pthread_join(buttonThread, NULL);

    return 0;
}