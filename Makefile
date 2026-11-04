# Toolchain configuration
# Auto-detect toolchain prefix if not explicitly specified:
# Arch Linux / Homebrew use 'riscv64-elf-'
# Debian / Ubuntu / Fedora use 'riscv64-unknown-elf-'
ifeq ($(origin CROSS_COMPILE), undefined)
    ifneq ($(shell which riscv64-elf-gcc 2>/dev/null),)
        CROSS_COMPILE := riscv64-elf-
    else ifneq ($(shell which riscv64-unknown-elf-gcc 2>/dev/null),)
        CROSS_COMPILE := riscv64-unknown-elf-
    else
        CROSS_COMPILE := riscv64-elf-
    endif
endif

CC            := $(CROSS_COMPILE)gcc
AS            := $(CROSS_COMPILE)as
QEMU          := qemu-system-riscv64

# Compilation and linking flags
CFLAGS  ?= -Wall -Wextra -mcmodel=medany -ffreestanding -std=c23 -I$(INC_DIR)
LDFLAGS ?= -T $(LINKER_SCRIPT) -nostdlib
LDLIBS  ?= -lgcc

# Project directories and files
SRC_DIR       := src
INC_DIR       := include
BIN_DIR       := bin
LINKER_SCRIPT := linker.ld
TARGET        := rio.elf

# Source and object files
C_SRCS   := $(shell find $(SRC_DIR) -name '*.c')
ASM_SRCS := $(shell find $(SRC_DIR) -name '*.s')

C_OBJS   := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.s,$(BIN_DIR)/%.o,$(ASM_SRCS))
OBJS     := $(ASM_OBJS) $(C_OBJS)
DEPS     := $(C_OBJS:.o=.d)

.PHONY: all build run clean \
        docker-image podman-image \
        docker-run podman-run \
        docker-build podman-build \
        docker-clean podman-clean

all: build

build: $(TARGET)

# Link the kernel ELF
$(TARGET): $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

# Compile C source files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Assemble assembly files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	$(AS) -c $< -o $@

# Ensure output directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run kernel in QEMU
run: $(TARGET)
	$(QEMU) -machine virt -bios none -kernel $(TARGET) -serial mon:stdio

# Clean build artifacts
clean:
	rm -f $(TARGET)
	rm -rf $(BIN_DIR)/*

# Container engine configuration (auto-detect podman or docker)
CONTAINER_ENGINE ?= $(shell command -v podman 2>/dev/null || command -v docker 2>/dev/null)
CONTAINER_IMAGE  ?= rio
CHECK_ENGINE     := @if [ -z "$(CONTAINER_ENGINE)" ]; then echo "Error: neither podman nor docker found in PATH" >&2; exit 1; fi

# Container targets (delegates execution inside a container)
docker-image podman-image:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) build -t $(CONTAINER_IMAGE) .

docker-run podman-run:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) run --rm -it -v $$(pwd):/rio $(CONTAINER_IMAGE)

docker-build podman-build:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) run --rm -v $$(pwd):/rio $(CONTAINER_IMAGE) make build

docker-clean podman-clean:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) run --rm -v $$(pwd):/rio $(CONTAINER_IMAGE) make clean

# Include auto-generated header dependencies
-include $(DEPS)
