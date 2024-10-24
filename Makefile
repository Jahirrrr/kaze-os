# Kernel version
kernel_version=0.0.1

# Directories
ISO_DIR=bin/iso
BINARY_DIR=bin/binary
KERNEL_DIR=$(ISO_DIR)/kaze/kernel/

# Kernel source files
ASM_SOURCE=kernel.asm
C_SOURCES=kernel.c fs.c string.c

# Object files
ASM_OBJ=kernel_asm.o
C_OBJS=kernel_c.o fs.o string.o printf.o

# Toolchain commands
NASM=nasm
GCC=gcc
LD=ld
GRUB=grub-mkrescue

# Compilation flags
CFLAGS=-fno-stack-protector -m32 -c
LDFLAGS=-m elf_i386 -T link.ld -o kernel

# Build rules
all: build

# Build rule
build:
	@echo "Building the kernel..."
	nasm -f elf32 kernel.asm -o kernel_asm.o
	gcc -m32 -z noexecstack -c kernel.c -o kernel_c.o -fno-stack-protector
	gcc -m32 -c fs.c -o fs.o -fno-stack-protector
	gcc -m32 -c string.c -o string.o -fno-stack-protector
	gcc -m32 -c printf.c -o printf.o -fno-stack-protector
	ld -m elf_i386 -T link.ld -o kernel kernel_asm.o kernel_c.o fs.o string.o printf.o -z noexecstack
	
	@echo "Kernel built successfully!"
	
	@echo "Preparing ISO structure..."
	rm -rf bin/iso
	rm -rf bin
	mkdir -p bin/iso/kaze/kernel/
	mkdir -p bin/iso/boot/grub/
	mkdir -p bin/binary/
	mkdir -p dist
	cp kernel bin/iso/kaze/kernel//kernel-0.0.1
	cp grub.cfg bin/iso/boot/grub/grub.cfg
	mv kernel bin/binary/kernel
	
	@echo "ISO structure prepared!"

	@echo "Building the ISO..."
	$(GRUB) -o dist/kaze.iso $(ISO_DIR)

clean:
	rm -rf dist
	rm -rf bin

install:
	sudo apt-get install nasm gcc ld grub qemu-system-i386

run:
	qemu-system-i386 -cdrom dist/kaze.iso
