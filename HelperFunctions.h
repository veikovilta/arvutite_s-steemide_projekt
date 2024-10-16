#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#define GPIO_READY_LED 23

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

struct port* openPort(int portPin, char* debugName, bool inputOutput);
void preciseSleep(double seconds);
int readButtonState(struct args_port* args);
void* readButtonState_thread(void* arg);
int CheckSync(int i2cHandle);
void ShowReady();

#endif
