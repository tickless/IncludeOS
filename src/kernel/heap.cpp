
#include <os.hpp>
#include <util/bitops.hpp>
#include <util/units.hpp>
#include <kernel.hpp>

using namespace util::literals;

size_t brk_bytes_used();
size_t mmap_bytes_used();
size_t mmap_allocation_end();



size_t kernel::heap_usage() noexcept
{
  return brk_bytes_used() + mmap_bytes_used();
}

size_t kernel::heap_avail() noexcept
{
  return (heap_max() - heap_begin()) - heap_usage();
}

uintptr_t kernel::heap_end() noexcept
{
  return mmap_allocation_end();
}

size_t os::total_memuse() noexcept {
  return kernel::heap_usage() + kernel::state().liveupdate_size + kernel::heap_begin();
}

constexpr size_t heap_alignment = 4096;
__attribute__((weak)) ssize_t __brk_max = 0x100000;

static bool __heap_ready = false;

extern void init_mmap(uintptr_t mmap_begin);


uintptr_t __init_brk(uintptr_t begin, size_t size);
uintptr_t __init_mmap(uintptr_t begin, size_t size);

bool kernel::heap_ready() { return __heap_ready; }
bool os::mem::heap_ready() { return kernel::heap_ready(); }

void kernel::init_heap(uintptr_t free_mem_begin, uintptr_t memory_end) noexcept {
  // NOTE: Initialize the heap before exceptions
  // cache-align heap, because its not aligned
  kernel::state().memory_end = memory_end;
  kernel::state().heap_max   = memory_end - 1;
  kernel::state().heap_begin = util::bits::roundto<heap_alignment>(free_mem_begin);
  auto brk_end  = __init_brk(kernel::heap_begin(), __brk_max);
  Expects(brk_end <= memory_end);
  __init_mmap(brk_end, memory_end);
  __heap_ready = true;
}
