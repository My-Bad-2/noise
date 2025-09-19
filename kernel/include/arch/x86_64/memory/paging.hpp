#ifndef ARCH_MEMORY_PAGING_HPP
#define ARCH_MEMORY_PAGING_HPP 1

#include "memory/memory.hpp"
#include "memory/pagemap.hpp"

#include <stddef.h>
#include <stdint.h>

#include <utility>

#define MAX_ENTRIES 512

namespace memory::arch::x86_64 {
enum PtFlags : size_t {
  PtPresent = (1ull << 0),
  PtWrite = (1ull << 1),
  PtUser = (1ull << 2),
  PtPWT = (1ull << 3),
  PtPCD = (1ull << 4),
  PtAccessed = (1ull << 5),
  PtDirty = (1ull << 6),
  // Bit 7: PS (large page) in PDE/PDPTE, PAT in 4KiB PTE; we alias both:
  PtLPages = (1ull << 7),
  PtPat = (1ull << 7),
  PtGlobal = (1ull << 8),
  PtLPat = (1ull << 12),
  PtNoExec = (1ull << 63),
};

void initialize();
[[nodiscard]] PageSizeType fix_page_size(PageSizeType type) noexcept;
[[nodiscard]] uintptr_t convert_flags(size_t flags, CachingType cache,
                                      PageSizeType type) noexcept;
[[nodiscard]] std::pair<size_t, CachingType> convert_flags(
    size_t flags, PageSizeType type) noexcept;
[[nodiscard]] PageSize from_type(PageSizeType type) noexcept;
[[nodiscard]] PageSizeType max_page_size(size_t size) noexcept;
}  // namespace memory::arch::x86_64

namespace memory {
namespace arch {
struct PageTable {
  // 512 entries, each 8 bytes on x86_64 -> 4096-byte page-sized table
  PageEntry entries[MAX_ENTRIES];
};

static_assert(sizeof(PageEntry) == 8, "x86_64 PTE must be 8 bytes");
static_assert(sizeof(PageTable) == MAX_ENTRIES * sizeof(PageEntry),
              "PageTable must be exactly one page (4096 bytes)");
static_assert(alignof(PageTable) == alignof(PageEntry),
              "PageTable alignment should match PageEntry");

constexpr size_t is_valid_flags = x86_64::PtPresent;
constexpr size_t new_table_flags =
    x86_64::PtPresent | x86_64::PtWrite | x86_64::PtUser;
}  // namespace arch

arch::PageTable* new_table();
arch::PageTable* get_next_lvl(PageEntry& entry, bool allocate);
}  // namespace memory

#endif  // ARCH_MEMORY_PAGING_HPP
