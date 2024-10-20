#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H



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
void *readButtonState(void* args);
void printDelaysToFile(const char *filename, double *data, int count, double averageDelay);
void* readButtonState_thread(void* arg);
int CheckSync(int i2cHandle);
void ShowReady();

#endif
