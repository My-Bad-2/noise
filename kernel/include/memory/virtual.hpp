#ifndef MEMORY_VIRTUAL_HPP
#define MEMORY_VIRTUAL_HPP 1

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

namespace memory {
// TODO: Add some way to allocate and deallocate virtual objects.
class VirtualMemoryManager {
 public:
  VirtualMemoryManager() = default;

  VirtualMemoryManager(const VirtualMemoryManager&) = delete;
  VirtualMemoryManager(VirtualMemoryManager&&) = delete;
  VirtualMemoryManager& operator=(const VirtualMemoryManager&) = delete;
  VirtualMemoryManager& operator=(VirtualMemoryManager&&) = delete;

  static VirtualMemoryManager& instance();

  void initialize(limine_memmap_response* memmap_response);
};
}  // namespace memory

#endif  // MEMORY_VIRTUAL_HPP
