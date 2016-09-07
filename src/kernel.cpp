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

char const* itoa(int val, int const base)
{
  if(val == 0)
  { return "0"; }

  static char buf[32]{};
  int i{ 30 };
  for(; val && i; --i, val /= base)
  { buf[i] = "0123456789abcdef"[val % base]; }
  return &buf[i + 1];
}

void putpixel(unsigned char * const screen,
              size_t const where,
              int32_t const color)
{
    screen[where] = color & 255; // B
    screen[where + 1] = (color >> 8) & 255; // G
    screen[where + 2] = (color >> 16) & 255; // R
}

extern "C" void kernel_main(multiboot_info const * const multiboot)
{
  init_serial();
  write_serial("serial initialized\n");

  if(!multiboot)
  {
    write_serial("multiboot is invalid\n");
    return;
  }

  auto const &mboot(*multiboot);

  write_serial("mem_lower: ");
  write_serial(itoa(mboot.mem_lower, 10));
  write_serial("KB mem_upper: ");
  write_serial(itoa(mboot.mem_upper, 10));
  write_serial("KB\n");

  auto const width(mboot.framebuffer_width);
  write_serial("framebuffer: ");
  write_serial(itoa(width, 10));

  auto const height(mboot.framebuffer_height);
  write_serial("x");
  write_serial(itoa(height, 10));

  uint32_t const bpp{ mboot.framebuffer_bpp };
  write_serial(" bpp: ");
  write_serial(itoa(bpp, 10));

  uint32_t const pitch{ mboot.framebuffer_pitch };
  write_serial(" pitch: ");
  write_serial(itoa(pitch, 10));
  write_serial("\n");

  for(size_t y{}; y < height; ++y)
  {
    for(size_t x{}; x < width; ++x)
    {
      uint32_t const pixel_width{ bpp / 8 };
      uint64_t const framebuffer{ mboot.framebuffer_addr };
      uint64_t pixel_offset = (y * pitch) + (x * pixel_width);
      putpixel(reinterpret_cast<unsigned char*>(framebuffer),
               pixel_offset,
               0xFF0000);
    }
  }
}
