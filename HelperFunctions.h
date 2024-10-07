#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

struct port {
    struct gpiod_chip *chip;
    struct gpiod_line *line;
};

struct thread_args_port {
    int port;
    char* debugName;
};

struct port* openPort(int port, char* debugName)
void preciseSleep(int seconds);

#endif
