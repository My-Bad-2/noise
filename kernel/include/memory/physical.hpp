#ifndef MEMORY_PHYSICAL_HPP
#define MEMORY_PHYSICAL_HPP 1

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#include "spinlock.hpp"

// The lowest order block size (2^0 * PageSize4KiB = 4 KiB)
#define MIN_ORDER 0
// The highest order block size (2^18 * PageSize4KiB = 1 MiB)
#define MAX_ORDER 18

namespace memory {
struct FreeBlockNode {
  FreeBlockNode* prev;
  FreeBlockNode* next;
};

struct PageMetadata {
  bool is_free;
  uint8_t order : 7;
};

class PhysicalMemoryManager {
 public:
  PhysicalMemoryManager() = default;

  PhysicalMemoryManager(const PhysicalMemoryManager&) = delete;
  PhysicalMemoryManager(PhysicalMemoryManager&&) = delete;

  PhysicalMemoryManager& operator=(const PhysicalMemoryManager&) = delete;
  PhysicalMemoryManager& operator=(PhysicalMemoryManager&&) = delete;

  static PhysicalMemoryManager& instance();

  void initialize(limine_memmap_response* memmap_response);
  void print() const;

  size_t get_free_memory() const {
    return this->usable_memory;
  }

  size_t get_total_memory() const {
    return this->total_memory;
  }

  void* allocate(size_t bytes, bool clear = false);
  void deallocate(void* ptr);

  template <typename T = void*>
  T allocate(size_t size, bool clear = false) {
    return reinterpret_cast<T>(this->allocate(size, clear));
  }

  void deallocate(auto ptr) {
    return this->deallocate(reinterpret_cast<void*>(ptr));
  }

 private:
  uint8_t size_to_order(size_t size) const;
  uintptr_t get_buddy_address(uintptr_t addr, uint8_t order);

  void insert_block(uintptr_t addr, uint8_t order);
  void remove_block(uintptr_t addr, uint8_t order);

  void set_page_metadata(uintptr_t addr, uint8_t order, bool is_free);

 private:
  FreeBlockNode free_lists[MAX_ORDER + 1];
  PageMetadata* page_metadata = nullptr;

  size_t total_memory = 0;
  size_t total_pages = 0;
  size_t usable_memory = 0;

  mutable libs::SpinLock lock;
};
}  // namespace memory

#endif  // MEMORY_PHYSICAL_HPP
