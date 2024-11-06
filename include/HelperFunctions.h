#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

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
static pthread_mutex_t buttonLock;

int ChronySync(int i2cHandle);
void ClosePort(struct port* openedPort);
struct port* openPort(int portPin, char* debugName, bool inputOutput);
void preciseSleep(double seconds);
void printDelaysToFile(const char *filename, double *data, int count, double averageDelay);
void* readButtonState_thread(void* arg);
int CheckSync(int i2cHandle);
const char* waitForButtonState(int port1, int port2);
const char* checkButtonState(struct port* port1, struct port* port2);
const char* waitForButtonState();
void WaitForNextMinuteBlinker(struct timespec firstblink);
const char* WaitForButtonAndSelectConfig(int i2cHandle);
int CreateButtonThread(int i2cHandle, pthread_t* buttonThread);


#endif
