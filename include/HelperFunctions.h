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

void AddSystemOffsetToBuffer(char** buffer);
void* oled_thread(void* arg);
void signalHandler(int signum);
int ChronySync(char** buffer);
void ClosePort(struct port* openedPort);
struct port* openPort(int portPin, char* debugName, bool inputOutput);
void preciseSleep(double seconds);
void* readButtonState_thread(void* arg);
int CheckSync(char** buffer);
void WaitForNextMinuteBlinker(struct timespec firstblink);
void ShowReady(int outputValue);
int IsButtonPressed(void);
const char* waitForButtonState(int port1, int port2, const char* state1Value, const char* state2Value); 
const char* WaitForButtonAndSelectConfig(const char* state1Value, const char* state2Value, const char* state3Value);
int check_ethernet_connected(void);

#endif
