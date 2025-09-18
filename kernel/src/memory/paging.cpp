#include "arch/paging.hpp"
#include "log.hpp"
#include "memory/pagemap.hpp"
#include "memory/physical.hpp"

namespace memory {
libs::Lazy<PageMap> kernel_pagemap;

arch::PageTable* new_table() {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();
  return instance.allocate<arch::PageTable*>(sizeof(arch::PageTable));
}

arch::PageTable* get_next_lvl(PageEntry& entry, bool allocate) {
  arch::PageTable* tbl = nullptr;

  if (!entry.get(arch::is_valid_flags)) {
    if (!allocate) {
      return nullptr;
    }

    entry.clear();
    entry.set(reinterpret_cast<uintptr_t>(tbl = new_table()));
    entry.set(arch::new_table_flags);
  } else {
    tbl = reinterpret_cast<arch::PageTable*>(entry.get());
  }

  return to_higher_half(tbl);
}

bool PageMap::map(uintptr_t virt_addr, uintptr_t phys_addr, size_t length,
                  size_t flags, PageSizeType type, CachingType cache) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if ((virt_addr % page_size) || (phys_addr % page_size)) {
    err("Address not aligned");
    return false;
  }

  // const libs::LockGuard guard(this->lock);

  flags = arch::convert_flags(flags, cache, type);

  for (size_t i = 0; i < length; i += page_size) {
    auto ret = get_page_entry(virt_addr + i, type, true);

    if (!ret.has_value()) {
      for (size_t j = 0; j < i; j += page_size) {
        ret = get_page_entry(virt_addr + j, type, true);

        if (!ret.has_value()) {
          return false;
        }

        PageEntry& entry = ret.value().get();
        entry.clear();
        this->invalidate_page(virt_addr + j);
      }

      return false;
    }

    PageEntry& entry = ret.value().get();
    entry.clear();
    entry.set(phys_addr + i);
    entry.set(flags, true);
  }

  return true;
}

bool PageMap::map(uintptr_t virt_addr, size_t length, size_t flags,
                  PageSizeType type, CachingType cache) noexcept {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("Address not aligned");
    return false;
  }

  for (size_t i = 0; i < length; i += page_size) {
    const uintptr_t phys_addr = instance.allocate<uintptr_t>(page_size, true);
    if (!map(virt_addr + i, phys_addr, page_size, flags, type, cache)) {
      instance.deallocate(phys_addr);
      return false;
    }
  }
  return true;
}

bool PageMap::unmap(uintptr_t virt_addr, size_t length,
                    PageSizeType type) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("Address not aligned");
    return false;
  }

  // const libs::LockGuard guard(this->lock);

  for (size_t i = 0; i < length; i += page_size) {
    const auto ret = get_page_entry(virt_addr + i, type, false);

    if (!ret.has_value()) {
      return false;
    }

    PageEntry& entry = ret.value().get();
    entry.clear();
    this->invalidate_page(virt_addr + i);
  }

  return true;
}

bool PageMap::unmap_dealloc(uintptr_t virt_addr, size_t length,
                            PageSizeType type) noexcept {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();

  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("Address not aligned");
    return false;
  }

  for (size_t i = 0; i < length; i += page_size) {
    const auto phys_addr = translate(virt_addr + i, type);

    if (!phys_addr.has_value()) {
      return false;
    }

    if (!unmap(virt_addr + i, page_size, type)) {
      return false;
    }

    instance.deallocate(phys_addr.value());
  }

  return true;
}

std::optional<uintptr_t> PageMap::translate(uintptr_t virt_addr,
                                            PageSizeType type) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("Address not aligned");
    return std::nullopt;
  }

  const auto ret = get_page_entry(virt_addr, type, false);

  if (!ret.has_value()) {
    return std::nullopt;
  }

  return ret.value().get().get();
}

bool PageMap::protect(uintptr_t virt_addr, size_t length, size_t flags,
                      PageSizeType type, CachingType cache) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("Address not aligned");
    return false;
  }

  // const libs::LockGuard guard(this->lock);

  flags = arch::convert_flags(flags, cache, type);

  for (size_t i = 0; i < length; i += page_size) {
    const auto ret = get_page_entry(virt_addr + i, type, false);

    if (!ret.has_value()) {
      return false;
    }

    PageEntry& entry = ret.value().get();
    entry.clear_flags();
    entry.set(flags, true);
    this->invalidate_page(virt_addr + i);
  }

  return true;
}

void initialize_paging(limine_memmap_response* memmap_response) {
  kernel_pagemap.initialize();

  const size_t memmap_count = memmap_response->entry_count;

  for (size_t i = 0; i < memmap_count; ++i) {
    const limine_memmap_entry* memmap = memmap_response->entries[i];
    const size_t type = memmap->type;

    if (type != LIMINE_MEMMAP_USABLE &&
        type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE &&
        type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES &&
        type != LIMINE_MEMMAP_FRAMEBUFFER) {
      continue;
    }

    if (memmap->length == 0) {
      continue;
    }

    const PageSizeType ps_type = arch::max_page_size(memmap->length);
    const size_t page_size = arch::from_type(ps_type);

    const uintptr_t base = align_down(memmap->base, page_size);
    const uintptr_t top = align_up(memmap->base + memmap->length, page_size);

    CachingType cache = WriteBack;

    if (type == LIMINE_MEMMAP_FRAMEBUFFER) {
      cache = WriteCombining;
    }

    const uintptr_t virt_addr = to_higher_half(base);
    const size_t length = top - base;

    if (length == 0) {
      continue;
    }

    debug("Page - \n\tType: %lu\n\tSize: 0x%lx bytes\n\tBase: 0x%lx -> 0x%lx",
          type, length, base, virt_addr);

    if (!kernel_pagemap->map(virt_addr, base, length, FlagRw, ps_type, cache)) {
      panic("Could not map virtual memory 0x%lx!", virt_addr);
    }
  }

  const uintptr_t phys_base = boot::address_request.response->physical_base;
  const uintptr_t virt_base = boot::address_request.response->virtual_base;
  const size_t kernel_size = boot::file_request.response->executable_file->size;

  for (size_t i = 0; i < kernel_size; i += PageSize4KiB) {
    if (!kernel_pagemap->map(virt_base + i, phys_base + i, PageSize4KiB,
                             FlagRwx)) {
      panic("Could not map virtual memory 0x%lx!", virt_base + i);
    }
  }

  kernel_pagemap->load();
}
}  // namespace memory
