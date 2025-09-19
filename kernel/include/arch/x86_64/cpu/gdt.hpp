#ifndef ARCH_CPU_GDT_HPP
#define ARCH_CPU_GDT_HPP 1

#include <stddef.h>
#include <stdint.h>

#define MAX_ENTRIES 5  // null + KCODE + KDATA + UDATA + UCODE

namespace arch::x86_64::cpu {
enum AccessFlags : uint8_t {
  // Access byte (type/present): |P|DPL|S|E|C/R|R/W|A|
  // gdtUser sets DPL=3 (ring 3). gdtPresent must be set for valid segments.
  gdtAccessed = (1 << 0),
  gdtReadWrite = (1 << 1),
  gdtConforming = (1 << 2),
  gdtExecutable = (1 << 3),
  gdtSegment = (1 << 4),
  gdtUser = (3 << 5),
  gdtPresent = (1 << 7),

  gdtDataSegment = gdtPresent | gdtSegment | gdtReadWrite,
  gdtCodeSegment = gdtDataSegment | gdtExecutable,
};

enum GranularityFlags : uint8_t {
  // High nibble of GDT descriptor second dword: |G|D/B|L|AVL|
  // In long mode: code uses L-bit=1, D/B must be 0 for 64-bit code.
  // Data segments ignore L and D/B; keep conventional settings.
  gdtLongMode = (1 << 1),  // L-bit: 64-bit code segment when set
  gdtDB = (1 << 2),        // D/B bit: 0 for 64-bit code segment; ignored for data in long mode
  gdtGranularity = (1 << 3),  // G=1 => limit is in 4KiB units
};

struct GdtTss {
 public:
  // TSS for x86_64: holds RSP0 and IST stacks; iopb_offset points past TSS if unused.
  constexpr GdtTss() = default;

  // Bugfix: iopb must be 16-bit; default to sizeof(GdtTss)
  constexpr GdtTss(uintptr_t sp, uint16_t iopb = sizeof(GdtTss))
      : iopb_offset(iopb) {
    rsp[0] = sp;  // RSP0 (ring 0 stack pointer)
  }

 private:
  // rsp[0] used as kernel stack for privilege transitions; IST[0..6] optional.
  uint32_t rsvd0;
  uint64_t rsp[3];
  uint64_t rsvd1;
  uint64_t ist[7];
  uint64_t rsvd2;
  uint16_t rsvd3;
  uint16_t iopb_offset;
} __attribute__((packed));

struct GdtSegment {
 public:
  // 8-byte legacy descriptor used for code/data segments in long mode.
  constexpr GdtSegment() = default;

  constexpr GdtSegment(uint32_t base, uint32_t limit, uint8_t access,
                       uint8_t granularity) {
    this->set_limit(limit);
    this->set_base(base);
    this->set_access(access);
    this->set_granularity(granularity);
  }

  constexpr GdtSegment(uint8_t access, uint8_t granularity)
      : GdtSegment(0, 0xffff, access, granularity) {}

  constexpr uint32_t get_base() const {
    uint32_t val = this->base_low;
    val |= (this->base_mid << 16);
    val |= (this->base_high << 24);
    return val;
  }

  constexpr uint32_t get_limit() const {
    uint32_t val = this->limit;
    val |= this->limit_high << 16;
    return val;
  }

  constexpr uint8_t get_access() const { return this->access; }
  constexpr uint8_t get_granularity() const { return this->granularity; }

 private:
  constexpr void set_limit(uint32_t limit) {
    this->limit = static_cast<uint16_t>(limit);
    this->limit_high = static_cast<uint8_t>((limit >> 16));
  }

  constexpr void set_base(uint32_t base) {
    this->base_low = static_cast<uint16_t>(base);
    this->base_mid = static_cast<uint8_t>(base >> 16);
    this->base_high = static_cast<uint8_t>(base >> 24);
  }

  constexpr void set_access(uint8_t access) { this->access = access; }
  constexpr void set_granularity(uint8_t granularity) { this->granularity = granularity; }

  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t limit_high : 4;
  uint8_t granularity : 4;
  uint8_t base_high;
} __attribute__((packed));

static_assert(sizeof(GdtSegment) == 8, "GDT segment descriptor must be 8 bytes");

struct GdtTssSeg {
 public:
  // 16-byte 64-bit TSS descriptor (available TSS, type 0x9).
  constexpr GdtTssSeg() = default;

  // 64-bit TSS descriptor (16 bytes). flags default to 0x89 (Available 64-bit TSS, present).
  constexpr GdtTssSeg(GdtTss* tss, uint16_t flags = 0x89)
      : limit(sizeof(GdtTss) - 1), rsvd(0) {
    this->set_base(reinterpret_cast<uintptr_t>(tss));
    this->set_flags(flags);
  }

  constexpr uintptr_t get_base() const {
    uintptr_t val = this->base_low;
    val |= (this->base_mid_lower << 16);
    val |= (this->base_mid_upper << 24);
    val |= (static_cast<uint64_t>(this->base_upper) << 32);
    return val;
  }

  constexpr uint16_t get_flags() const {
    uint16_t val = this->flags_low;
    val |= (this->flags_high << 8);
    return val;
  }

 private:
  constexpr void set_base(uintptr_t base) {
    this->base_low = static_cast<uint16_t>(base);
    this->base_mid_lower = static_cast<uint8_t>(base >> 16);
    this->base_mid_upper = static_cast<uint8_t>(base >> 24);
    this->base_upper = static_cast<uint32_t>(base >> 32);
  }

  constexpr void set_flags(uint16_t flags) {
    this->flags_low = static_cast<uint8_t>(flags);
    this->flags_high = static_cast<uint8_t>(flags >> 8);
  }

  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid_lower;
  uint8_t flags_low;
  uint8_t flags_high;
  uint8_t base_mid_upper;
  uint32_t base_upper;
  uint32_t rsvd;
} __attribute__((packed));

static_assert(sizeof(GdtTssSeg) == 16, "TSS descriptor must be 16 bytes");

struct GdtTable {
 public:
  // GDT layout containing 5 standard entries + a TSS descriptor.
  constexpr GdtTable() = default;

  void fill(GdtTss* tss);
  void print() const;

 private:
  GdtSegment entries[MAX_ENTRIES];
  GdtTssSeg tss_segment;
} __attribute__((packed));

struct GdtRegister {
 public:
  // Wrapper for GDTR (limit, base) used by lgdt.
  constexpr GdtRegister() = default;

  // GDTR.limit = sizeof(GDT)-1, GDTR.base = linear address of table
  constexpr GdtRegister(GdtTable* gdt)
      : limit(sizeof(GdtTable) - 1), base(reinterpret_cast<uintptr_t>(gdt)) {}

  void load();

 private:
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

class Gdt {
 public:
  // Initialize and load the GDT and TSS.
  void initialize();

 private:
  GdtTable table;
  GdtTss tss;
};
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_GDT_HPP
