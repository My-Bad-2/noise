#include "boot.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"

namespace memory {
void initialize() {
  PhysicalMemoryManager& pmm = PhysicalMemoryManager::instance();
  pmm.initialize(boot::memmap_request.response);
}
}  // namespace memory
