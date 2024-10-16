# Compiler
CC = gcc

#-Werror kasuta l]plikul asjal
# Compiler flags for warnings, strict checks, and optimizations
CFLAGS = -Wall -Wextra -pedantic -Wshadow -Wformat -Wconversion -g -O2

# Target executable name
TARGET = projekt

# Source files
SRCS = Main.c Sensor.c HelperFunctions.c display.c

# Object files (generated from the source files)
OBJS = $(SRCS:.c=.o)

# Library flags (for gpiod)
LIBS = -lgpiod -lwiringPi -lpigpio -lrt

# Default target
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Pattern rule to compile each .c file into corresponding .o file
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

# Clean rule to remove object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
