#include <stddef.h>
#include <stdint.h>

#include "multiboot.hpp"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "This OS needs to be compiled with a ix86-elf compiler"
#endif

void putpixel(unsigned char * const screen,
              size_t const where,
              int32_t const color)
{
    screen[where] = color & 255; // B
    screen[where + 1] = (color >> 8) & 255; // G
    screen[where + 2] = (color >> 16) & 255; // R
}

extern "C" void kernel_main(int32_t const ebx)
{
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
