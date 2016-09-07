all: jank-platform.iso

.PHONY: clean

clean:
	rm -f boot.o kernel.o jank-platform.bin isodir

cxx_flags=-std=c++14 \
					-ffreestanding \
					-ggdb \
					-O3 -Wall -Wextra -Werror -pedantic \
					-nostdlib -fno-exceptions -fno-rtti \
					#-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2

kernel.o:
	${CXX} -c src/kernel.cpp -o kernel.o ${cxx_flags}

boot.o:
	${AS} src/boot.S -o boot.o

jank-platform.bin: kernel.o boot.o
	${CXX} -T linker.ld -o jank-platform.bin ${cxx_flags} boot.o kernel.o -lgcc
	grub-file --is-x86-multiboot jank-platform.bin

jank-platform.iso: jank-platform.bin
	mkdir -p isodir/boot/grub
	cp -f jank-platform.bin isodir/boot/jank-platform.bin
	cp -f grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o jank-platform.iso isodir

qemu: jank-platform.bin
	qemu-system-i386 -cdrom jank-platform.iso

gdb: jank-platform.bin
	qemu-system-i386 -s -S -cdrom jank-platform.iso
