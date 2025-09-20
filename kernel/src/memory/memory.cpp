#include "boot.hpp"
#include "log.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"
#include "memory/virtual.hpp"

namespace memory {
void initialize() {
  PhysicalMemoryManager& pmm = PhysicalMemoryManager::instance();
  VirtualMemoryManager& vmm = VirtualMemoryManager::instance();

  pmm.initialize(boot::memmap_request.response);

  const uintptr_t highest_addr = to_higher_half(pmm.get_highest_addr());

  vmm.initialize(boot::memmap_request.response, highest_addr, PageSize1GiB * 2);
}
}  // namespace memory
