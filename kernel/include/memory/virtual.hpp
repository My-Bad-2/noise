#ifndef MEMORY_VIRTUAL_HPP
#define MEMORY_VIRTUAL_HPP 1

#include "memory/memory.hpp"
#include "spinlock.hpp"

#include <stddef.h>
#include <stdint.h>

namespace memory {
namespace arch {
struct PageTable;
}

enum caching_type {
  VMEM_CACHE_UNCACHEABLE,
  VMEM_CACHE_UNCACHEABLE_STRONG,
  VMEM_CACHE_WRITE_BACK,
  VMEM_CACHE_WRITE_COMBINING,
  VMEM_CACHE_WRITE_THROUGH,
  VMEM_CACHE_WRITE_PROTECTED,
};

enum flags {
  VMEM_FLAG_READ,
  VMEM_FLAG_WRITE,
  VMEM_FLAG_USER,
  VMEM_FLAG_EXECUTE,
  VMEM_FLAG_GLOBAL,

  VMEM_FLAG_RW = VMEM_FLAG_READ | VMEM_FLAG_WRITE,
  VMEM_FLAG_RWX = VMEM_FLAG_RW | VMEM_FLAG_EXECUTE,
};

class PageMap {
 public:
  bool map(uintptr_t virt_addr, uintptr_t phys_addr,
           size_t flags = VMEM_FLAG_RW, size_t page_size = PageSize4KiB,
           caching_type cache = VMEM_CACHE_WRITE_BACK);

  bool unmap(uintptr_t virt_addr, size_t page_size = PageSize4KiB);

  uintptr_t translate(uintptr_t virt_addr, PageSize page_size = PageSize4KiB);

 private:
  arch::PageTable* root_tbl;
  mutable libs::SpinLock lock;
};
}  // namespace memory

#endif  // MEMORY_VIRTUAL_HPP
