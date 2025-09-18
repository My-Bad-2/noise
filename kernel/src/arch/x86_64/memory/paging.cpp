#include "arch/x86_64/cpu/cpu.hpp"
#include "arch/x86_64/memory/paging.hpp"
#include "boot.hpp"
#include "memory/memory.hpp"
#include "memory/pagemap.hpp"

#include <string.h>

#include <utility>

namespace memory::arch::x86_64 {
namespace {
bool pml3_available = false;
int max_levels = 0;
}  // namespace

PageSize from_type(PageSizeType type) noexcept {
  switch (type) {
    case PageLarge:
      return PageSize1GiB;
    case PageMedium:
      return PageSize2MiB;
    case PageSmall:
    default:
      return PageSize4KiB;
  }
}

PageSizeType max_page_size(size_t size) noexcept {
  if (size >= PageSize1GiB) {
    return PageLarge;
  }

  if (size >= PageSize2MiB) {
    return PageMedium;
  }

  return PageSmall;
}

uintptr_t convert_flags(size_t flags, CachingType cache,
                        PageSizeType type) noexcept {
  uintptr_t ret = PtPresent;

  if (flags & FlagWrite) {
    ret |= PtWrite;
  }

  if (flags & FlagUser) {
    ret |= PtUser;
  }

  if (flags & FlagGlobal) {
    ret |= PtGlobal;
  }

  if (!(flags & FlagExecute)) {
    ret |= PtNoExec;
  }

  if (type != PageSmall) {
    ret |= PtLPages;
  }

  const size_t pat = (type == PageSmall) ? PtPat : PtLPat;

  switch (cache) {
    case Uncacheable:
      ret |= PtPCD;
      break;
    case Device:
      ret |= PtPCD | PtPWT;
      break;
    case WriteThrough:
      ret |= PtPWT;
      break;
    case WriteProtected:
      ret |= pat;
      break;
    case WriteCombining:
      ret |= pat | PtPWT;
      break;
    case WriteBack:
      break;
    default:
      std::unreachable();
  }

  return ret;
}

std::pair<size_t, CachingType> convert_flags(size_t flags,
                                             PageSizeType type) noexcept {
  size_t ret = FlagExecute;
  CachingType cache = CachingType::WriteBack;

  const bool is_pat = (type == PageSmall) ? (flags & PtPat) : (flags & PtLPat);
  const bool is_pcd = (flags & PtPCD);
  const bool is_pwt = (flags & PtPWT);

  if (flags & PtPresent) {
    ret |= FlagRead;
  }

  if (flags & PtWrite) {
    ret |= FlagWrite;
  }

  if (flags & PtUser) {
    ret |= FlagUser;
  }

  if (flags & PtGlobal) {
    ret |= FlagGlobal;
  }

  if (flags & PtNoExec) {
    ret &= ~FlagExecute;
  }

  if (!is_pat && !is_pcd && !is_pwt) {
    cache = WriteBack;
  } else if (!is_pat && !is_pcd && is_pwt) {
    cache = WriteThrough;
  } else if (!is_pat && is_pcd && !is_pwt) {
    cache = Uncacheable;
  } else if (!is_pat && is_pcd && is_pwt) {
    cache = UncacheableStrong;
  } else if (is_pat && !is_pcd && !is_pwt) {
    cache = WriteProtected;
  } else if (is_pat && !is_pcd && is_pwt) {
    cache = WriteCombining;
  }

  return std::make_pair(ret, cache);
}

PageSizeType fix_page_size(PageSizeType type) noexcept {
  if ((type == PageLarge) && !pml3_available) {
    return PageMedium;
  }

  return type;
}
}  // namespace memory::arch::x86_64

namespace memory {
std::optional<std::reference_wrapper<PageEntry>> PageMap::get_page_entry(
    uintptr_t virt_addr, PageSizeType page_size, bool allocate) noexcept {
  const int levels = arch::x86_64::max_levels;
  const int shift_start = 12 + (levels - 1) * 9;
  const int ret_idx = ((levels == 5) ? 4 : levels) - page_size - 1;

  arch::PageTable* pml = to_higher_half(this->root_tbl);
  int shift = shift_start;

  for (int i = 0; i < levels; ++i) {
    PageEntry& entry = pml->entries[(virt_addr >> shift) & 0x1ff];

    if (i == ret_idx) {
      return std::ref(entry);
    }

    pml = get_next_lvl(entry, allocate);

    if (pml == nullptr) {
      return std::nullopt;
    }

    shift -= 9;
  }

  std::unreachable();
}

void PageMap::load() noexcept {
  const uintptr_t addr = reinterpret_cast<uintptr_t>(this->root_tbl);
  ::arch::x86_64::cpu::write_cr3(addr);
}

void PageMap::invalidate_page(uintptr_t addr) noexcept {
  ::arch::x86_64::cpu::invalidate_page(addr);
}

PageMap::PageMap() : root_tbl(new_table()) {
  if (!kernel_pagemap.valid()) {
    using namespace ::arch::x86_64;
    arch::x86_64::max_levels = 4;

    if (boot::paging_mode_request.response->mode ==
        LIMINE_PAGING_MODE_X86_64_5LVL) {
      arch::x86_64::max_levels = 5;
    }

    arch::x86_64::pml3_available = cpu::test_feature(FEATURE_HUGE_PAGE);

    arch::PageTable* tbl = to_higher_half(this->root_tbl);

    for (int i = MAX_ENTRIES / 2; i < MAX_ENTRIES; ++i) {
      get_next_lvl(tbl->entries[i], true);
    }
  } else {
    arch::PageTable* tbl = to_higher_half(this->root_tbl);
    const auto& kernel_table = to_higher_half(kernel_pagemap->root_tbl);

    memcpy(tbl->entries + 256, kernel_table + 256, 256 * sizeof(PageEntry));
  }
}
}  // namespace memory
