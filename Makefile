# Compiler
CC = gcc

#-Werror kasuta l]plikul asjal
# Compiler flags for warnings, strict checks, and optimizations
CFLAGS = -Wall -Wextra -pedantic -Wshadow -Wformat -Wconversion -g -O2

# Target executable name
TARGET = build/projekt

# Source and header directories
SRCDIR = src
INCDIR = include

# Source files
SRCS = $(SRCDIR)/Main.c $(SRCDIR)/Sensor.c $(SRCDIR)/HelperFunctions.c $(SRCDIR)/display.c $(SRCDIR)/LedBlink.c $(SRCDIR)/Files.c

# Object files (generated from the source files)
OBJS = $(SRCS:$(SRCDIR)/%.c=build/%.o)

# Library flags
LIBS = -lgpiod -lwiringPi -lpigpio -lrt

# Default target
all: $(TARGET)

# Rule to link the object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Pattern rule to compile each .c file into corresponding .o file
build/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h | build
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Create the build directory if it doesn't exist
build:
	mkdir -p build

# Clean rule to remove object files and the executable within the build directory, without deleting the directory itself
clean:
	rm -f build/*.o build/projekt

.PHONY: all clean
