#include "boot.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"
#include "memory/virtual.hpp"

namespace memory {
void initialize() {
  PhysicalMemoryManager& pmm = PhysicalMemoryManager::instance();
  VirtualMemoryManager& vmm = VirtualMemoryManager::instance();

  pmm.initialize(boot::memmap_request.response);
  vmm.initialize(boot::memmap_request.response);
}
}  // namespace memory
