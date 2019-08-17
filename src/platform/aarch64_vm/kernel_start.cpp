
#include <kprint>

#include <cstdint>
uintptr_t _multiboot_free_begin(uintptr_t boot_addr);
uintptr_t _multiboot_memory_end(uintptr_t boot_addr);
extern bool os_default_stdout;

extern "C"
uint8_t nibble2hex(uint8_t nibble)
{
  nibble=nibble&0xF;
  switch(nibble)
  {
    case 0x00: return '0';
    case 0x01: return '1';
    case 0x02: return '2';
    case 0x03: return '3';
    case 0x04: return '4';
    case 0x05: return '5';
    case 0x06: return '6';
    case 0x07: return '7';
    case 0x08: return '8';
    case 0x09: return '9';
    case 0x0A: return 'A';
    case 0x0B: return 'B';
    case 0x0C: return 'C';
    case 0x0D: return 'D';
    case 0x0E: return 'E';
    case 0x0F: return 'F';
    default: return 0x00; // unreachable
  }
  //unreachable
}

extern "C"
void print_memory(const char *name,const char * mem,int size)
{
  kprint(name);
  kprint(":\n");
  int i;
  for (i=0;i<size;i++)
  {
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]>>4);
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]);
    if (((i+1)%4)==0)
    {
      kprint(" ");
    }
    if (((i+1)%16)==0)
    {
      kprint("\n");
    }

  }
  kprint("\n");
}

void print_le(const char *mem,int size)
{
  for (int i = (size-1);i >= 0; i--)
  {
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]>>4);
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]);
  }
}

void print_le_named(const char *name,const char *mem,int size)
{
  kprint(name);
  kprint(" ");
  print_le(mem,size);
  kprint("\r\n");
}

void print_le_named32(const char *name,const char *ptr)
{
  print_le_named(name,ptr,sizeof(uint32_t));
}

void print_le_named64(const char *name,const char *ptr)
{
  print_le_named(name,ptr ,sizeof(uint64_t));
}

void print_be(const char *mem,int size)
{
  for (int i =0;i < size; i++)
  {
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]>>4);
    *((volatile unsigned int *) 0x09000000) =nibble2hex(mem[i]);
  }
  *((volatile unsigned int *) 0x09000000) =' ';
}

#include <kernel.hpp>
#include <info>
#include <os>

//#include <kernel/os.hpp>
#include <kernel/service.hpp>
//#include <boot/multiboot.h>
extern "C" {
  #include <libfdt.h>
}

#include "init_libc.hpp"
//#include "idt.h"
#include <cpu.h>

extern "C" {/*
  void __init_sanity_checks();
  uintptr_t _move_symbols(uintptr_t loc);
//  void _init_bss();
  void _init_heap(uintptr_t);
  void _init_syscalls();
*/
  void __init_sanity_checks();
  void kernel_sanity_checks();
  void _init_bss();
  uintptr_t _move_symbols(uintptr_t loc);
  void _init_elf_parser();
  void _init_syscalls();
  void __elf_validate_section(const void*);
}



static os::Machine* __machine = nullptr;
os::Machine& os::machine() noexcept {
  LL_ASSERT(__machine != nullptr);
  return *__machine;
}


//__attribute__((no_sanitize("all")))
extern "C"
void _init_bss()
{
  extern char _BSS_START_, _BSS_END_;
  __builtin_memset(&_BSS_START_, 0, &_BSS_END_ - &_BSS_START_);
}

extern "C"
//__attribute__((no_sanitize("all")))
void kernel_start(uintptr_t magic, uintptr_t addrin)
{
  kprintf("Magic %zx addrin %zx\n",magic,addrin);

  __init_sanity_checks();

  cpu_print_current_el();
  //its a "RAM address 0"
  const struct fdt_property *prop;
  int addr_cells = 0, size_cells = 0;
  int proplen;

  //TODO find this somewhere ?.. although it is at memory 0x00
  uint64_t fdt_addr=0x40000000;
  char *fdt=(char*)fdt_addr;


  //OK so these get overidden in the for loop which should return a map of memory and not just a single one
  uint64_t addr = 0;
  uint64_t size = 0;

  //checks both magic and version
  if ( fdt_check_header(fdt) != 0 )
  {
    kprint("FDT Header check failed\r\n");
    return;
  }

  size_cells = fdt_size_cells(fdt,0);
  print_le_named32("size_cells :",(char *)&size_cells);
  addr_cells = fdt_address_cells(fdt, 0);//fdt32_ld((const fdt32_t *)prop->data);
  print_le_named32("addr_cells :",(char *)&addr_cells);

  const int mem_offset = fdt_path_offset(fdt, "/memory");
  if (mem_offset < 0)
    return;

  print_le_named32("mem_offset :",(char *)&mem_offset);

  prop = fdt_get_property(fdt, mem_offset, "reg", &proplen);
  int cellslen = (int)sizeof(uint32_t) * (addr_cells + size_cells);

  for (int i = 0; i < proplen / cellslen; ++i) {

  	for (int j = 0; j < addr_cells; ++j) {
  		int offset = (cellslen * i) + (sizeof(uint32_t) * j);

  		addr |= (uint64_t)fdt32_ld((const fdt32_t *)((char *)prop->data + offset)) <<
  			((addr_cells - j - 1) * 32);
  	}
  	for (int j = 0; j < size_cells; ++j) {
  		int offset = (cellslen * i) +
  			(sizeof(uint32_t) * (j + addr_cells));

  		size |= (uint64_t)fdt32_ld((const fdt32_t *)((char *)prop->data + offset)) <<
  			((size_cells - j - 1) * 32);
  	}
	}

  print_le_named64("RAM BASE :",(char *)&addr);
  print_le_named64("RAM SIZE :",(char *)&size);

  uint64_t mem_end=addr+size;

  extern char _end;
  uintptr_t free_mem_begin = reinterpret_cast<uintptr_t>(&_end);

    //ok now its sane
  free_mem_begin += _move_symbols(free_mem_begin);

  // Initialize .bss
  _init_bss();

  // Instantiate machine
  size_t memsize = mem_end - free_mem_begin;
  __machine = os::Machine::create((void*)free_mem_begin, memsize);

  _init_elf_parser();
  // Begin portable HAL initialization
  __machine->init();

  // Initialize system calls
  _init_syscalls();

  //probably not very sane!
  cpu_debug_enable();
  cpu_fiq_enable();
  cpu_irq_enable();
  cpu_serror_enable();

  aarch64::init_libc((uintptr_t)fdt_addr);

}
