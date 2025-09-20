#include "memory/memory.hpp"
#include "memory/pagemap.hpp"
#include "memory/virtual.hpp"
#include "log.hpp"  // debug/info logs

#include <utility>

namespace memory {
static VirtualMemoryManager vmm_instance;

VirtualMemoryManager& VirtualMemoryManager::instance() {
  return vmm_instance;
}

void VirtualMemoryManager::initialize(limine_memmap_response* memmap_response,
                                      uintptr_t base, size_t length) {
  // Align the requested region to 1GiB boundaries (implementation detail; VA-only).
  base = align_up(base, std::to_underlying(PageSize1GiB));
  const uintptr_t end =
      align_up(base + length, std::to_underlying(PageSize1GiB));

  // Ensure paging is initialized before handing out VAs.
  initialize_paging(memmap_response);

  // Establish bounds and reset bump pointer.
  this->base_start = base;
  this->base_end = end;
  this->next = base;

  info("[VMM][INIT] VA pool: [0x%lx, 0x%lx) size=0x%lx (%lu MiB)",
       this->base_start, this->base_end, this->base_end - this->base_start,
       (this->base_end - this->base_start) / 1024 / 1024);
}

void* VirtualMemoryManager::allocate(size_t bytes) {
  // Reserve a 4KiB-aligned range from the pool; VA-only (no mapping here).
  const size_t alloc_length = align_up(bytes, std::to_underlying(PageSize4KiB));
  const uintptr_t alloc_start = this->next;

  const libs::LockGuard guard(this->lock);  // serialize updates to 'next'

  // Bump the pointer to the next aligned page boundary after this allocation.
  this->next =
      align_up(alloc_start + alloc_length, std::to_underlying(PageSize4KiB));
  this->allocations++;

  debug("[VMM][ALLOC] req=0x%lx aligned=0x%lx base=0x%lx next=0x%lx total=%lu",
        bytes, alloc_length, alloc_start, this->next, this->allocations);

  return reinterpret_cast<void*>(alloc_start);
}
}  // namespace memory
