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

void ClosePort(struct port* openedPort);
struct port* openPort(int portPin, char* debugName, bool inputOutput);
void preciseSleep(double seconds);
void *readButtonState(void* args);
void printDelaysToFile(const char *filename, double *data, int count, double averageDelay);
void* readButtonState_thread(void* arg);
int CheckSync(int i2cHandle);
void ShowReady();

#endif
