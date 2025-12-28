CC = /opt/homebrew/bin/aarch64-elf-gcc
LD = /opt/homebrew/bin/aarch64-elf-ld
OBJCOPY = /opt/homebrew/bin/aarch64-elf-objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdlib -mcpu=cortex-a53 -mgeneral-regs-only -I include
LDFLAGS = -T linker.ld -nostdlib

SRCS = src/boot.S src/vectors.S src/switch.S src/kernel.c src/shell.c src/sched.c src/mm.c src/utils.c src/drivers/uart.c src/drivers/gic.c src/drivers/timer.c
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)
TARGET = kernel.elf

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	/opt/homebrew/bin/qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -nographic -kernel $(TARGET)

clean:
	rm -f src/*.o src/drivers/*.o $(TARGET)
