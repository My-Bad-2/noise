#ifndef BOOT_HPP
#define BOOT_HPP 1

#include <limine.h>
#include <stdint.h>

namespace boot {
extern volatile limine_memmap_request memmap_request;
extern volatile limine_hhdm_request hhdm_request;

inline const uintptr_t get_hhdm_offset() {
  return hhdm_request.response->offset;
}
}  // namespace boot

#endif  // BOOT_HPP
