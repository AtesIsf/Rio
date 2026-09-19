build:
	riscv64-elf-gcc -Wall -Wextra -c -mcmodel=medany src/kernel.c -o bin/kernel.o -ffreestanding
	riscv64-elf-as -c src/boot.s -o bin/boot.o
	riscv64-elf-gcc -T linker.ld -nostdlib bin/kernel.o bin/boot.o -o rio.elf -lgcc
clean:
	rm rio.elf bin/*
run:
	qemu-system-riscv64 -machine virt -bios none -kernel rio.elf -serial mon:stdio
