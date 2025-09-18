#include "boot.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"
#include "memory/pagemap.hpp"

namespace memory {
void initialize() {
  PhysicalMemoryManager& pmm = PhysicalMemoryManager::instance();

  pmm.initialize(boot::memmap_request.response);
  initialize_paging(boot::memmap_request.response);
}
}  // namespace memory
