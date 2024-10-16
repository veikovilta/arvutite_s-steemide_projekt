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
#include "Sensor.h"
#include "Main.h"

int main(void)
{
    //väljakutse chrony start
    system("chronyc systemctl start");

    int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
    if (i2cHandle < 0) return -1; // Exit if failed

    // message string
    char message[100] = "";  
    char numberStr[20] = "";

    oledInit(i2cHandle); // Initialize the OLED
    oledClear(i2cHandle); // Clear the display

    // eraldi thread enne käima mis checkib nuppu
    pthread_t buttonThread;
    struct args_port args;

    // Initialize thread arguments
    args.portPin = GPIO_BUTTON; // Set GPIO port number
    args.debugName = "InputButton";  // Set debug name
	args.inputOutput = false;
    if(pthread_create(&buttonThread, NULL, readButtonState(), (void*)&args) < 0)
    {
        perror("Failed to create thread");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread")
        oledWriteText(i2cHandle, 2, 0, "Shutting Down")
        system ("sudo shutdown -h now"); 
        return 1;
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
            sprintf(numberStr, "%d", minutes);
            message = "Syncing for another minute   minutes waited - ";
            strcat(message, numberStr);
            oledClear(i2cHandle);
            // Display a message on the OLED
            oledWriteText(i2cHandle, 0, 0, message);
        }
        minutes++;

        if (minutes == 10)
        {
            oledClear(i2cHandle);
            // Display a message on the OLED
            oledWriteText(i2cHandle, 0, 0, "NOT SYNCED");
            oledWriteText(i2cHandle, 2, 0, "ERROR BAD RECEPTION")
            oledWriteText(i2cHandle, 4, 0, "Shutting Down")
            system ("sudo shutdown -h now"); 
            return 1;
        }
        
    }

    //lediga näitama et on cynced ja ready (mõlemal)
    ShowReady();

    RegisterBlinks(); 
    // siia faili salvestamine juurde ja hilistuse arvutus
    // TODO!
    // !!!!
    
    // Wait for the threads to complete
    // TODO exit neile funktsioonidele vaja juurde teha
    pthread_join(buttonThread, NULL);

    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished")
    oledWriteText(i2cHandle, 2, 0, "Shutting Down")

    close(i2cHandle);
    system ("sudo shutdown -h now");
    return 0;
}