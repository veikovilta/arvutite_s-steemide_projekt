#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#define GPIO_READY_LED 23
#define GPIO_CHIP "/dev/gpiochip0" // Change if needed

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
int debounceButtonState(struct args_port* args);
void* readButtonState_thread(void* arg);
int CheckSync();
void ShowReady();
void ClosePort(struct port* openedPort);

#endif
