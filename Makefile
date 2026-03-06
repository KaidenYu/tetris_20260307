# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Target executable name
TARGET = tetris.exe

# Source files
SRCS = main.c

# Object files (automatically generated from SRCS)
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	@if exist $(TARGET) del $(TARGET)
	@if exist *.o del *.o

# Run the game
run: all
	./$(TARGET)

.PHONY: all clean run
