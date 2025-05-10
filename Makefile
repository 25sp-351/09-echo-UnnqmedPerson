# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread

# Target executable
TARGET = echo_server

# Source files
SRCS = echo_server.c

# Default target
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)
