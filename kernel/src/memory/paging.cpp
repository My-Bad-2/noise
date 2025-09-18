#include "arch/paging.hpp"
#include "log.hpp"
#include "memory/pagemap.hpp"
#include "memory/physical.hpp"

namespace memory {
libs::Lazy<PageMap> kernel_pagemap;

// Allocate a fresh (zeroed) page table structure.
arch::PageTable* new_table() {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();
  auto* tbl = instance.allocate<arch::PageTable*>(sizeof(arch::PageTable));
  // debug("[PG][ALLOC] New page table @ 0x%lx", reinterpret_cast<uintptr_t>(tbl));
  return tbl;
}

// Descend or allocate next paging level for an entry.
// Returns higher-half pointer to the child table or nullptr if missing and !allocate.
arch::PageTable* get_next_lvl(PageEntry& entry, bool allocate) {
  arch::PageTable* tbl = nullptr;

  if (!entry.get(arch::is_valid_flags)) {
    if (!allocate) {
      return nullptr;
    }
    // Allocate and install a new intermediate table.
    entry.clear();
    entry.set(reinterpret_cast<uintptr_t>(tbl = new_table()));
    entry.set(arch::new_table_flags);
  } else {
    // Reuse existing table.
    tbl = reinterpret_cast<arch::PageTable*>(entry.get());
  }
  return to_higher_half(tbl);
}

// Map existing physical pages to a virtual range.
bool PageMap::map(uintptr_t virt_addr, uintptr_t phys_addr, size_t length,
                  size_t flags, PageSizeType type, CachingType cache) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if ((virt_addr % page_size) || (phys_addr % page_size)) {
    err("[PG][MAP] Alignment error virt=0x%lx phys=0x%lx size=0x%lx",
        virt_addr, phys_addr, page_size);
    return false;
  }

  const libs::LockGuard guard(this->lock);

  flags = arch::convert_flags(flags, cache, type);

  for (size_t i = 0; i < length; i += page_size) {
    auto ret = get_page_entry(virt_addr + i, type, true);
    if (!ret.has_value()) {
      // Rollback already-created entries for this call.
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

  debug("[PG][MAP] virt=0x%lx -> phys=0x%lx len=0x%lx pages=%zu ps=%zu flags=0x%lx",
        virt_addr, phys_addr, length, length / page_size,
        static_cast<size_t>(page_size), flags);
  return true;
}

// Allocate physical pages and map them.
bool PageMap::map(uintptr_t virt_addr, size_t length, size_t flags,
                  PageSizeType type, CachingType cache) noexcept {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("[PG][MAP-AUTO] Alignment error virt=0x%lx size=0x%lx",
        virt_addr, page_size);
    return false;
  }

  for (size_t i = 0; i < length; i += page_size) {
    const uintptr_t phys_addr = instance.allocate<uintptr_t>(page_size, true);
    if (!map(virt_addr + i, phys_addr, page_size, flags, type, cache)) {
      instance.deallocate(phys_addr);
      return false;
    }
  }
  debug("[PG][MAP-AUTO] virt=0x%lx len=0x%lx pages=%zu ps=%zu flags=0x%lx",
        virt_addr, length, length / page_size, static_cast<size_t>(page_size),
        flags);
  return true;
}

// Unmap without freeing physical memory.
bool PageMap::unmap(uintptr_t virt_addr, size_t length,
                    PageSizeType type) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("[PG][UNMAP] Alignment error virt=0x%lx size=0x%lx",
        virt_addr, page_size);
    return false;
  }

  const libs::LockGuard guard(this->lock);

  for (size_t i = 0; i < length; i += page_size) {
    const auto ret = get_page_entry(virt_addr + i, type, false);
    if (!ret.has_value()) {
      return false;
    }
    PageEntry& entry = ret.value().get();
    entry.clear();
    this->invalidate_page(virt_addr + i);
  }

  debug("[PG][UNMAP] virt=0x%lx len=0x%lx pages=%zu", virt_addr, length,
        length / page_size);
  return true;
}

// Unmap and free physical pages.
bool PageMap::unmap_dealloc(uintptr_t virt_addr, size_t length,
                            PageSizeType type) noexcept {
  PhysicalMemoryManager& instance = PhysicalMemoryManager::instance();

  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("[PG][UNMAP-DEL] Alignment error virt=0x%lx size=0x%lx",
        virt_addr, page_size);
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

  debug("[PG][UNMAP-DEL] virt=0x%lx len=0x%lx pages=%zu",
        virt_addr, length, length / page_size);
  return true;
}

// Translate virtual to physical (returns base of page).
std::optional<uintptr_t> PageMap::translate(uintptr_t virt_addr,
                                            PageSizeType type) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("[PG][XLATE] Alignment error virt=0x%lx size=0x%lx",
        virt_addr, page_size);
    return std::nullopt;
  }

  const auto ret = get_page_entry(virt_addr, type, false);
  if (!ret.has_value()) {
    return std::nullopt;
  }
  return ret.value().get().get();
}

// Change flags on existing mappings.
bool PageMap::protect(uintptr_t virt_addr, size_t length, size_t flags,
                      PageSizeType type, CachingType cache) noexcept {
  type = arch::fix_page_size(type);
  const PageSize page_size = arch::from_type(type);

  if (virt_addr % page_size) {
    err("[PG][PROTECT] Alignment error virt=0x%lx size=0x%lx",
        virt_addr, page_size);
    return false;
  }

  const libs::LockGuard guard(this->lock);

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

  debug("[PG][PROTECT] virt=0x%lx len=0x%lx pages=%zu new_flags=0x%lx",
        virt_addr, length, length / page_size, flags);
  return true;
}

// Build initial kernel mappings from Limine memory map.
void initialize_paging(limine_memmap_response* memmap_response) {
  debug("[INFO][PG-INIT] Paging initialization start");

  kernel_pagemap.initialize();

  const size_t memmap_count = memmap_response->entry_count;
  size_t regions_considered = 0;
  size_t regions_mapped = 0;
  size_t bytes_mapped = 0;

  for (size_t i = 0; i < memmap_count; ++i) {
    const limine_memmap_entry* memmap = memmap_response->entries[i];
    const size_t type = memmap->type;

    debug("[PG-INIT][RAW] idx=%zu type=%lu base=0x%lx len=0x%lx",
          i, type, memmap->base, memmap->length);

    if (type != LIMINE_MEMMAP_USABLE &&
        type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE &&
        type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES &&
        type != LIMINE_MEMMAP_FRAMEBUFFER) {
      continue;
    }

    if (memmap->length == 0) {
      continue;
    }

    ++regions_considered;

    const PageSizeType ps_type = arch::max_page_size(memmap->length);
    const size_t page_size = arch::from_type(ps_type);

    const uintptr_t base = align_down(memmap->base, page_size);
    const uintptr_t top = align_up(memmap->base + memmap->length, page_size);
    const size_t length = top - base;

    CachingType cache = WriteBack;
    if (type == LIMINE_MEMMAP_FRAMEBUFFER) {
      cache = WriteCombining;
    }

    if (length == 0) {
      debug("[PG-INIT][SKIP] idx=%zu zero-length after align", i);
      continue;
    }

    const uintptr_t virt_addr = to_higher_half(base);

    debug("[PG-INIT][ALIGN] idx=%zu base=0x%lx top=0x%lx len=0x%lx ps=0x%lx ps_type=%d cache=%d virt=0x%lx",
          i, base, top, length, page_size, static_cast<int>(ps_type),
          static_cast<int>(cache), virt_addr);

    if (!kernel_pagemap->map(virt_addr, base, length, FlagRw, ps_type, cache)) {
      panic("[PG-INIT] Map failure virt=0x%lx len=0x%lx", virt_addr, length);
    }

    ++regions_mapped;
    bytes_mapped += length;
  }

  // Map kernel executable (4KiB granularity enforced).
  const uintptr_t phys_base = boot::address_request.response->physical_base;
  const uintptr_t virt_base = boot::address_request.response->virtual_base;
  const size_t kernel_size = boot::file_request.response->executable_file->size;

  size_t kernel_pages = 0;
  for (size_t i = 0; i < kernel_size; i += PageSize4KiB) {
    if (!kernel_pagemap->map(virt_base + i, phys_base + i, PageSize4KiB,
                             FlagRwx)) {
      panic("[PG-INIT] Kernel image map failed at virt=0x%lx",
            virt_base + i);
    }
    ++kernel_pages;
  }

  debug("[PG-INIT][KERNEL] phys_base=0x%lx virt_base=0x%lx size=0x%lx pages=%zu",
        phys_base, virt_base, kernel_size, kernel_pages);

  kernel_pagemap->load();
  debug("[INFO][PG-INIT] Completed: regions_considered=%zu regions_mapped=%zu bytes_mapped=0x%lx kernel_pages=%zu",
        regions_considered, regions_mapped, bytes_mapped, kernel_pages);
}
}  // namespace memory
