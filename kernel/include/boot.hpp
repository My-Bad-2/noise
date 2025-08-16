#ifndef BOOT_HPP
#define BOOT_HPP 1

#include <limine.h>
#include <stddef.h>

namespace boot {
extern volatile limine_memmap_request memmap_request;
extern volatile limine_hhdm_request hhdm_request;
extern volatile limine_executable_file_request file_request;
extern volatile limine_executable_address_request address_request;

inline const uintptr_t get_hhdm_offset() {
  return hhdm_request.response->offset;
}

const char* type_name(size_t memmap_type);
}  // namespace boot

#endif  // BOOT_HPP
