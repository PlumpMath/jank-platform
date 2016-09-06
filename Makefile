all: myos.iso

.PHONY: clean

clean:
	rm -f boot.o kernel.o myos.bin isodir

cxx_flags=-std=c++14 \
					-ffreestanding \
					-O3 -Wall -Wextra -Werror -pedantic \
					-nostdlib -fno-exceptions -fno-rtti \
					#-mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2

kernel.o:
	${CXX} -c src/kernel.cpp -o kernel.o ${cxx_flags}

boot.o:
	${AS} src/boot.S -o boot.o

myos.bin: kernel.o boot.o
	${CXX} -T linker.ld -o myos.bin ${cxx_flags} boot.o kernel.o -lgcc
	grub-file --is-x86-multiboot myos.bin

myos.iso: myos.bin
	mkdir -p isodir/boot/grub
	cp -f myos.bin isodir/boot/myos.bin
	cp -f grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

qemu: myos.bin
	qemu-system-i386 -cdrom myos.iso

qemu-64: myos.bin
	qemu-system-x86_64 -cdrom myos.iso
