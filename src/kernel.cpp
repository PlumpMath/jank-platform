#include <stddef.h>
#include <stdint.h>

#include "multiboot.hpp"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "This OS needs to be compiled with a ix86-elf compiler"
#endif

extern "C" void kernel_main(int32_t const ebx)
{
  if(!ebx)
  { return; } // TODO: Error

  auto const &multiboot(*reinterpret_cast<multiboot_info*>(ebx));
  static_cast<void>(multiboot);
}
