all: myos.bin

.PHONY: clean

clean:
	rm -f boot.o kernel.o myos.bin

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

qemu: myos.bin
	qemu-system-i386 -kernel myos.bin

qemu-64: myos.bin
	qemu-system-x86_64 -kernel myos.bin
