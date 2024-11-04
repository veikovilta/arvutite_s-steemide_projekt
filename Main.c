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
#include "Sensor.h"
#include "Main.h"


int main(void)
{
    if (system("sudo systemctl start chrony") != 0) {
        perror("Failed to start chrony service");
    }

    int i2cHandle = i2cInit("/dev/i2c-1", OLED_I2C_ADDR);
    if (i2cHandle < 0) return -1; // Exit if failed
    // message string
    char message[100] = "";  
    char numberStr[20] = "";

    oledInit(i2cHandle); // Initialize the OLED
    oledClear(i2cHandle); // Clear the display
    oledWriteText(i2cHandle, 0, 0, "Program started");
    
    // eraldi thread enne käima mis checkib nuppu
    pthread_t buttonThread;
    struct args_port args;

    // Initialize thread arguments
    args.portPin = GPIO_BUTTON; // Set GPIO port number
    args.debugName = "InputButton";  // Set debug name
	args.inputOutput = true;

    if(pthread_create(&buttonThread, NULL, readButtonState_thread, (void*)&args) < 0)
    {
        perror("Failed to create thread");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 0, 0, "ERROR Failed to create thread");
        oledWriteText(i2cHandle, 2, 0, "Shutting Down");
        
        if (system ("sudo shutdown -h now") != 0) {
            perror("Failed to shutdown");
            oledClear(i2cHandle);
            oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
            // Handle the error or exit
        }
        return 1;
    } 

    // teeb 60 sekundilist checki kui hea kell on
    // 10min vähemalt
    int minutes = 0;
    while(1)
    {
        preciseSleep(5);
    
        if (CheckSync(i2cHandle) == 0)
        {
            break;
        }
        else 
        {
            // kirjuta ekraanile et oodatud 5 sek
            sprintf(numberStr, "%d", minutes+5);
            snprintf(message, sizeof(message), "seconds waited : %s", numberStr);
            printf("%s\n", message);
            oledWriteText(i2cHandle, 0, 0, message);
        }
        
        minutes+=5;

        // kui ei ole syncis 10 mintaga siis error
        if (minutes == 120)
        {
            oledClear(i2cHandle);
            // Display a message on the OLED
            oledWriteText(i2cHandle, 0, 0, "NOT SYNCED");
            oledWriteText(i2cHandle, 2, 0, "ERROR BAD RECEPTION");
            oledWriteText(i2cHandle, 4, 0, "Shutting Down");
           
            if (system ("sudo shutdown -h now") != 0) {
                perror("Failed to shutdown");
                oledClear(i2cHandle);
                oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
                // Handle the error or exit
            }
            return 1;
        }
        
    }
    printf("synced\n");
    //lediga naitama et on synced ja ready (molemal)
    ShowReady();
    printf("Showed that im ready\n");
    double *delaysCalculated = RegisterBlinks(i2cHandle); 
    printf("Calculated delays and returned\n");
    
    int numOfValidCalculations = 0;
    double averageDelay = calculateAverage(delaysCalculated, &numOfValidCalculations); 
    
    char averageDelayStr[50]; 
    sprintf(averageDelayStr, "Average Delay: %.5f\n", averageDelay); // Format average delay
    printf("%s\n",averageDelayStr);
    
    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, averageDelayStr);
    
    printDelaysToFile("delays.txt", delaysCalculated, numOfValidCalculations, averageDelay);
    
    pthread_join(buttonThread, NULL);
    oledClear(i2cHandle);
    oledWriteText(i2cHandle, 0, 0, "Program finished");
    oledWriteText(i2cHandle, 2, 0, "Shutting Down");

    close(i2cHandle);

    /*if (system ("sudo shutdown -h now") != 0) {
        perror("Failed to shutdown");
        oledClear(i2cHandle);
        oledWriteText(i2cHandle, 2, 0, "Shutting Down failed");
        // Handle the error or exit
    }*/
    return 0;
}
