# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -g

# Target executable name
TARGET = my_program

# Source files
SRCS = main.c led.c

# Header files
HEADERS = main.h Led.h

# Object files (generated from the source files)
OBJS = main.o led.o

# Default target
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile main.c into main.o
main.o: main.c main.h Led.h
	$(CC) $(CFLAGS) -c main.c

# Rule to compile led.c into led.o
led.o: led.c Led.h
	$(CC) $(CFLAGS) -c led.c

# Clean up all generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets are not actual files
.PHONY: all clean
