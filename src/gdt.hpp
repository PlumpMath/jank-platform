#pragma once

#include <stddef.h>
#include <stdint.h>

struct gdt_entry
{
  uint16_t limit_low;   // The lower 16 bits of the limit.
  uint16_t base_low;    // The lower 16 bits of the base.
  uint8_t  base_middle; // The next 8 bits of the base.
  uint8_t  access;
  uint8_t  granularity;
  uint8_t  base_high;   // The last 8 bits of the base.
} __attribute__((packed));

struct idt_entry
{
  uint16_t base_lo;   // The lower 16 bits of the address to jump to when this interrupt fires.
  uint16_t sel;       // Kernel segment selector.
  uint8_t  always0;   // This must always be zero.
  uint8_t  flags;     // More flags. See documentation.
  uint16_t base_hi;   // The upper 16 bits of the address to jump to.
} __attribute__((packed));

struct descriptor_ptr
{
  uint16_t limit; // The upper 16 bits of all selector limits.
  uint32_t base;  // The address of the first entry.
} __attribute__((packed));

extern "C" void gdt_flush(uint32_t);
extern "C" void idt_flush(uint32_t);

gdt_entry gdt_entries[5]{};
descriptor_ptr gdt_ptr{};

idt_entry idt_entries[256]{};
descriptor_ptr idt_ptr{};

// Set the value of one GDT entry.
void gdt_set_gate(int32_t num, uint32_t base,
                  uint32_t limit, uint8_t access, uint8_t gran)
{
  gdt_entries[num].base_low    = (base & 0xFFFF);
  gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  gdt_entries[num].base_high   = (base >> 24) & 0xFF;

  gdt_entries[num].limit_low   = (limit & 0xFFFF);
  gdt_entries[num].granularity = (limit >> 16) & 0x0F;

  gdt_entries[num].granularity |= gran & 0xF0;
  gdt_entries[num].access      = access;
}

void init_gdt()
{
  gdt_ptr.limit = (sizeof(gdt_entry) * 5) - 1;
  gdt_ptr.base  = reinterpret_cast<uint32_t>(&gdt_entries);

  gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

  gdt_flush(reinterpret_cast<uint32_t>(&gdt_ptr));
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
   idt_entries[num].base_lo = base & 0xFFFF;
   idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].always0 = 0;
   // We must uncomment the OR below when we get to using user-mode.
   // It sets the interrupt gate's privilege level to 3.
   idt_entries[num].flags   = flags /* | 0x60 */;
}

void init_idt()
{
   idt_ptr.limit = sizeof(idt_entry) * 256 -1;
   idt_ptr.base  = (uint32_t)&idt_entries;

   memset(&idt_entries, 0, sizeof(idt_entry) * 256);

   idt_set_gate(0, (uint32_t)isr0 , 0x08, 0x8E);
   idt_set_gate(1, (uint32_t)isr1 , 0x08, 0x8E);
   ...
   idt_set_gate(31, (uint32_t)isr32, 0x08, 0x8E);

   idt_flush((uint32_t)&idt_ptr);
}

void init_descriptor_tables()
{
  init_gdt();
  init_idt();
}
