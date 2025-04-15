#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <pthread.h>

#define GPIO_READY_LED 23
#define GPIO_CHIP "/dev/gpiochip0"


struct port 
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
};

struct args_port 
{
    int portPin;
    char* debugName;
    bool inputOutput; // input True, Output False
};

static volatile int buttonPressed = 0;
static volatile int programRunning = 1;  // Flag to control the thread loop
static pthread_mutex_t buttonLock;
static pthread_t buttonThread;

//static pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
//static volatile char oledBuffer[100] = "";
/*
static pthread_t oledThread;

static volatile char oledBuffer[100];
static volatile int bufferHasBeenUpdated = 0;
static pthread_mutex_t oledLock;
*/

void AddSystemOffsetToBuffer(char** buffer);
void* oled_thread(void* arg);
void signalHandler(int signum);
int ChronySync(char** buffer);
void ClosePort(struct port* openedPort);
struct port* openPort(int portPin, char* debugName, bool inputOutput);
void preciseSleep(double seconds);
void printDelaysToFile(const char *filename, double *data, int count, double averageDelay);
void* readButtonState_thread(void* arg);
int CheckSync(char** buffer);
const char* checkButtonState(struct port* port1, struct port* port2);
//const char* waitForButtonState();
void WaitForNextMinuteBlinker(struct timespec firstblink);
void ShowReady(int outputValue);
int IsButtonPressed(void);
const char* waitForButtonState(int port1, int port2, const char* state1Value, const char* state2Value); 
const char* WaitForButtonAndSelectConfig(const char* state1Value, const char* state2Value, const char* state3Value);
const char* checkButtonState(struct port* port1, struct port* port2);
int check_ethernet_connected(void);

#endif
