CROSS_COMPILE ?= $(shell which riscv64-elf-gcc >/dev/null 2>&1 && echo riscv64-elf- || echo riscv64-unknown-elf-)
CC            := $(CROSS_COMPILE)gcc
AS            := $(CROSS_COMPILE)as
QEMU          := qemu-system-riscv64

CFLAGS        ?= -Wall -Wextra -mcmodel=medany -ffreestanding -std=c23 -Iinclude
LDFLAGS       ?= -T linker.ld -nostdlib
LDLIBS        ?= -lgcc

TARGET        := rio.elf
TEST_TARGET   := rio-test.elf
BIN_DIR       := bin

C_SRCS        := $(shell find src -name '*.c')
ASM_SRCS      := $(shell find src -name '*.s')
C_OBJS        := $(patsubst src/%.c,$(BIN_DIR)/%.o,$(C_SRCS))
ASM_OBJS      := $(patsubst src/%.s,$(BIN_DIR)/%.o,$(ASM_SRCS))
OBJS          := $(ASM_OBJS) $(C_OBJS)

TEST_SRCS     := $(shell find test -name '*.c' 2>/dev/null)
TEST_OBJS     := $(ASM_OBJS) $(filter-out $(BIN_DIR)/core/kmain.o,$(C_OBJS)) $(patsubst test/%.c,$(BIN_DIR)/test/%.o,$(TEST_SRCS))

.PHONY: all build run test test-build clean \
        docker-image podman-image docker-run podman-run \
        docker-% podman-%

all build: $(TARGET)

$(TARGET): $(OBJS) linker.ld
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

test-build: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) linker.ld
	$(CC) $(LDFLAGS) $(TEST_OBJS) -o $@ $(LDLIBS)

$(BIN_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BIN_DIR)/%.o: src/%.s
	@mkdir -p $(dir $@)
	$(AS) -c $< -o $@

$(BIN_DIR)/test/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Itest -MMD -MP -c $< -o $@

run: $(TARGET)
	$(QEMU) -machine virt -bios none -kernel $(TARGET) -serial mon:stdio

test: $(TEST_TARGET)
	$(QEMU) -machine virt -bios none -kernel $(TEST_TARGET) -serial mon:stdio -nographic

clean:
	rm -f $(TARGET) $(TEST_TARGET)
	rm -rf $(BIN_DIR)/*

# Container engine integration
CONTAINER_ENGINE ?= $(shell command -v podman 2>/dev/null || command -v docker 2>/dev/null)
CONTAINER_IMAGE  ?= rio
CHECK_ENGINE     := @if [ -z "$(CONTAINER_ENGINE)" ]; then echo "Error: podman/docker not found" >&2; exit 1; fi

docker-image podman-image:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) build -t $(CONTAINER_IMAGE) .

docker-run podman-run:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) run --rm -it -v $$(pwd):/rio $(CONTAINER_IMAGE)

docker-% podman-%:
	$(CHECK_ENGINE)
	$(CONTAINER_ENGINE) run --rm -v $$(pwd):/rio $(CONTAINER_IMAGE) make $*

-include $(C_OBJS:.o=.d) $(patsubst test/%.c,$(BIN_DIR)/test/%.d,$(TEST_SRCS))
