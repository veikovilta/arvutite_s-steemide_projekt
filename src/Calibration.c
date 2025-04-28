//ask if user wants to calibrate 
//needs to be implemented on both versions 
//saatja blinks 5 times 
//vastuvõtja displays number of blinks it got 
//if blink count doesnt equal to 5 program will reccomend user which way to turn the 
//ask user to confirm calibration or run it again 
//
////////////// vastuvotja ///////////////////
///
///-ask if user wants to calibrate
///-if yes then start waiting for blinks
///- (if user has seen that saatja has ended blinking(failure)) - wait for button press OR has gotten 5 blinks then ends by itself
///-asks if wants to run again if previus step was failure then suggests solutions OR ask confirmation to contiue
///-contiue program
///
////////////// saatja //////////////////
///
///-ask if user wants to calibrate
///-starts blinking 
///-blinks 5 times
///-asks if wants to run again OR ask confirmation to contiue
///-contiue program
///
//PROBLEMS
//-vastuvotja needs to be ready beforehand
//-communication between users
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <gpiod.h> 
#include <signal.h> 
#include "stdbool.h"
#include "HelperFunctions.h"
#include "display.h"
#include <string.h>
#include "Sensor.h"
#include "LedBlink.h"
#include "Files.h"
#include "Main.h"
#include "Calibration.h"

void Calibrate()
{
    const char* saatjaOrVastuvotja = WaitForButtonAndSelectConfig("Blinker", "Receiver", "Return MAIN");
    while (strcmp(saatjaOrVastuvotja, "Return MAIN") != 0) {
        if (strcmp(saatjaOrVastuvotja, "Blinker") == 0) {
            ledBlinkingCalibration(BLINK_COUNT_CALIBRATION);
        } else if (strcmp(saatjaOrVastuvotja, "Receiver") == 0) {
            CountBlinks();
        }
        saatjaOrVastuvotja = WaitForButtonAndSelectConfig("Blinker", "Receiver", "Return MAIN");
        preciseSleep(0.5);
    }

    return; 
}