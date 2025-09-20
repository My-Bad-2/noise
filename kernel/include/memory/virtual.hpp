#ifndef MEMORY_VIRTUAL_HPP
#define MEMORY_VIRTUAL_HPP 1

#include "spinlock.hpp"

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

namespace memory {
class VirtualMemoryManager {
 public:
  VirtualMemoryManager() = default;

  VirtualMemoryManager(const VirtualMemoryManager&) = delete;
  VirtualMemoryManager(VirtualMemoryManager&&) = delete;
  VirtualMemoryManager& operator=(const VirtualMemoryManager&) = delete;
  VirtualMemoryManager& operator=(VirtualMemoryManager&&) = delete;

  static VirtualMemoryManager& instance();

  // Initialize a contiguous virtual address space above `base` and within `size` bytes.
  // The region is aligned to large pages in the implementation for TLB friendliness.
  // Paging is initialized prior to setting up these bounds.
  void initialize(limine_memmap_response* memmap_response, uintptr_t base,
                  size_t size);

  // Reserve `bytes` from the virtual region. Returns a 4KiB-aligned base or nullptr.
  // Note: This only manages VA; mapping/backing is handled elsewhere.
  void* allocate(size_t bytes);

  // Typed convenience overload. The `clear` flag is not acted upon here
  // (this VMM only reserves VA). Delegates to the raw allocate().
  template <typename T = void*>
  T allocate(size_t size, bool clear = false) {
    return reinterpret_cast<T>(this->allocate(size, clear));
  }

 private:
  // Region bounds [base_start, base_end) and bump pointer (next).
  uintptr_t base_start = 0;
  uintptr_t base_end = 0;
  uintptr_t next = 0;

  // Debug/tracking counter of total successful allocations (not used for logic).
  size_t allocations = 0;

  // Simple synchronization; allocation is thread-safe via this lock.
  mutable libs::SpinLock lock;
};
}  // namespace memory

#endif  // MEMORY_VIRTUAL_HPP
