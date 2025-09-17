#ifndef ARCH_MEMORY_PAGEMAP_HPP
#define ARCH_MEMORY_PAGEMAP_HPP 1

#include <stddef.h>
#include <stdint.h>

namespace memory::arch::x86_64 {
struct PagingModeConfig {
  int levels;
  int pml3_translation;
  uint8_t shifts[5];
  size_t masks[5];
  uint64_t canonical_mask;
};
}  // namespace memory::arch::x86_64

#endif  // ARCH_MEMORY_PAGEMAP_HPP
