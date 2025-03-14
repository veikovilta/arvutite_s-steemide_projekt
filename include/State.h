#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <pthread.h>


enum ProgramState {
    UNDEFINEDSTATE,
    STARTING,
    SYNCHRONIZING,
    PICKING_CONFIG,
    WAITING_NEXT_MINUTE_LED,
    WAITING_NEXT_MINUTE_SENSOR,
    PAUSED,
    LED_BLINKING,
    BLINKING_FINISHED,
    SENSOR_WAITING_FIRST_BLINK,
    SENSOR_READING_BLINKS,
    SENSOR_SHOWING_RESULTS,
    PROGRAM_FINISHED
};

static enum ProgramState InstanceState = UNDEFINEDSTATE;  

enum ChosenConfig {
    UNDEFINED,
    SAATJA,
    VASTUVOTJA
};

static enum ChosenConfig Saatja_Vastuvotja_State = UNDEFINED;   

#endif
