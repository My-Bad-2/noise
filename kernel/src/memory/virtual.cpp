#include "memory/pagemap.hpp"
#include "memory/virtual.hpp"

namespace memory {
static VirtualMemoryManager vmm_instance;

VirtualMemoryManager& VirtualMemoryManager::instance() {
  return vmm_instance;
}

void VirtualMemoryManager::initialize(limine_memmap_response* memmap_response) {
  initialize_paging(memmap_response);
}
}  // namespace memory
