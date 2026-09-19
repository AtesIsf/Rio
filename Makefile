# Toolchain configuration
CROSS_COMPILE ?= riscv64-elf-
CC            := $(CROSS_COMPILE)gcc
AS            := $(CROSS_COMPILE)as
QEMU          := qemu-system-riscv64

# Compilation and linking flags
CFLAGS  ?= -Wall -Wextra -mcmodel=medany -ffreestanding
LDFLAGS ?= -T $(LINKER_SCRIPT) -nostdlib
LDLIBS  ?= -lgcc

# Project directories and files
SRC_DIR       := src
BIN_DIR       := bin
LINKER_SCRIPT := linker.ld
TARGET        := rio.elf

# Source and object files
C_SRCS   := $(wildcard $(SRC_DIR)/*.c)
ASM_SRCS := $(wildcard $(SRC_DIR)/*.s)

C_OBJS   := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.s,$(BIN_DIR)/%.o,$(ASM_SRCS))
OBJS     := $(ASM_OBJS) $(C_OBJS)
DEPS     := $(C_OBJS:.o=.d)

.PHONY: all build run clean

all: build

build: $(TARGET)

# Link the kernel ELF
$(TARGET): $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

# Compile C source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Assemble assembly files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.s | $(BIN_DIR)
	$(AS) -c $< -o $@

# Ensure output directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run kernel in QEMU
run: $(TARGET)
	$(QEMU) -machine virt -bios none -kernel $(TARGET) -serial mon:stdio

# Clean build artifacts
clean:
	rm -f $(TARGET) $(BIN_DIR)/*.o $(BIN_DIR)/*.d

# Include auto-generated header dependencies
-include $(DEPS)
