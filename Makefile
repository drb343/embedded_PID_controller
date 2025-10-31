# ------------------------------------------------------------
# Project Settings
# ------------------------------------------------------------
CROSS_COMPILE ?= arm-none-eabi-
CC      := $(CROSS_COMPILE)gcc
CFLAGS  := -Wall -Wextra -O2 -std=c11
LDFLAGS := -lm
TARGET  := pid_controller

# Source files
SRC     := main.c pid_controller.c
OBJ     := $(SRC:.c=.o)
DEP     := $(SRC:.c=.d)

# ------------------------------------------------------------
# Build Rules
# ------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# ------------------------------------------------------------
# Dependency Management
# ------------------------------------------------------------
-include $(DEP)

.PHONY: all clean run
