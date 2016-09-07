#include <stddef.h>
#include <stdint.h>

#include "multiboot.hpp"
#include "asm.hpp"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "This OS needs to be compiled with a ix86-elf compiler"
#endif

#define PORT 0x3f8   /* COM1 */

void init_serial()
{
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   //outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int is_transmit_empty()
{ return inb(PORT + 5) & 0x20; }

void write_serial(char const * const s)
{
  while(is_transmit_empty() == 0)
  { } /* Wait */

  for(auto c(s); *c; ++c)
  { outb(PORT, *c); }
}

void putpixel(unsigned char * const screen,
              size_t const where,
              int32_t const color)
{
    screen[where] = color & 255; // B
    screen[where + 1] = (color >> 8) & 255; // G
    screen[where + 2] = (color >> 16) & 255; // R
}

extern "C" void kernel_main(int32_t const ebx, int32_t const )
{
  init_serial();
  write_serial("meow!\n");

  if(!ebx)
  { return; } // TODO: Error

  auto const &multiboot(*reinterpret_cast<multiboot_info*>(ebx));

  auto const width(multiboot.framebuffer_width);
  auto const height(multiboot.framebuffer_height);
  for(size_t y{}; y < height; ++y)
  {
    for(size_t x{}; x < width; ++x)
    {
      uint32_t const pitch{ multiboot.framebuffer_pitch };
      uint32_t const bpp{ multiboot.framebuffer_bpp };
      uint32_t const pixel_width{ bpp / 8 };
      uint64_t const framebuffer{ multiboot.framebuffer_addr };
      uint64_t pixel_offset = (y * pitch) + (x * pixel_width);
      putpixel(reinterpret_cast<unsigned char*>(framebuffer),
               pixel_offset,
               0x000000);
    }
  }
}
