FROM debian:stable-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    gcc-riscv64-unknown-elf \
    binutils-riscv64-unknown-elf \
    qemu-system-riscv \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /rio

CMD ["make", "run"]
