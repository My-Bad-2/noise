#ifndef MEMORY_PAGEMAP_HPP
#define MEMORY_PAGEMAP_HPP 1

#include "lazy.hpp"
#include "spinlock.hpp"

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#include <functional>
#include <optional>

namespace memory {
namespace arch {
struct PageTable;
}

enum CachingType {
  Uncacheable,
  UncacheableStrong,
  WriteBack,
  WriteCombining,
  WriteThrough,
  WriteProtected,
  Device,
};

enum Flags {
  FlagRead = 1u << 0,
  FlagWrite = 1u << 1,
  FlagUser = 1u << 2,
  FlagExecute = 1u << 3,
  FlagGlobal = 1u << 4,
  FlagRw = FlagRead | FlagWrite,
  FlagRwx = FlagRw | FlagExecute,
};

enum PageSizeType : int {
  PageSmall,
  PageMedium,
  PageLarge,
};

struct PageEntry {
  uintptr_t val = 0;

#ifdef __x86_64__
  static constexpr inline uintptr_t page_mask = 0x000ffffffffff000ull;
#else
  static constexpr inline uintptr_t page_mask = 0;
#endif

  inline void clear() noexcept {
    val = 0;
  }
  inline void clear_flags() noexcept {
    val &= ~page_mask;
  }

  inline void set(uintptr_t flags, bool enabled) noexcept {
    if (enabled) {
      val |= flags;
    } else {
      val &= ~flags;
    }
  }

  inline void set(uintptr_t addr) noexcept {
    val &= ~page_mask;
    val |= (addr & page_mask);
  }

  [[nodiscard]] inline bool get(uintptr_t flag) const noexcept {
    return (val & flag) == flag;
  }

  [[nodiscard]] inline uintptr_t get() const noexcept {
    return val & page_mask;
  }

  [[nodiscard]] inline size_t get_flags() const noexcept {
    return val & ~page_mask;
  }
};

class PageMap {
 public:
  PageMap();
  [[nodiscard]] bool map(uintptr_t virt_addr, uintptr_t phys_addr,
                         size_t length, size_t flags = FlagRw,
                         PageSizeType type = PageSmall,
                         CachingType cache = WriteBack) noexcept;
  [[nodiscard]] bool unmap(uintptr_t virt_addr, size_t length,
                           PageSizeType type = PageSmall) noexcept;
  [[nodiscard]] std::optional<uintptr_t> translate(
      uintptr_t virt_addr, PageSizeType type = PageSmall) noexcept;
  [[nodiscard]] bool protect(uintptr_t virt_addr, size_t length,
                             size_t flags = FlagRw,
                             PageSizeType type = PageSmall,
                             CachingType cache = WriteBack) noexcept;
  [[nodiscard]] bool map(uintptr_t virt_addr, size_t length,
                         size_t flags = FlagRw, PageSizeType type = PageSmall,
                         CachingType cache = WriteBack) noexcept;
  [[nodiscard]] bool unmap_dealloc(uintptr_t virt_addr, size_t length,
                                   PageSizeType type = PageSmall) noexcept;

  void load() noexcept;

 private:
  std::optional<std::reference_wrapper<PageEntry>> get_page_entry(
      uintptr_t virt_addr, PageSizeType page_size, bool allocate) noexcept;
  void invalidate_page(uintptr_t virt_addr) noexcept;

 private:
  arch::PageTable* root_tbl;
  mutable libs::SpinLock lock;
};

void initialize_paging(limine_memmap_response* memmap_response);

extern libs::Lazy<PageMap> kernel_pagemap;
}  // namespace memory

#endif  // MEMORY_VIRTUAL_HPP
