#ifndef ARCH_CPU_GDT_HPP
#define ARCH_CPU_GDT_HPP 1

#include <stddef.h>
#include <stdint.h>

#define MAX_ENTRIES 5

namespace arch::x86_64::cpu {
enum AccessFlags : uint8_t {
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
  gdtLongMode = (1 << 1),
  gdtDB = (1 << 2),
  gdtGranularity = (1 << 3),
};

struct GdtTss {
 public:
  constexpr GdtTss() = default;

  constexpr GdtTss(uintptr_t sp, uint8_t iopb = sizeof(GdtTss))
      : iopb_offset(iopb) {
    rsp[0] = sp;
  }

 private:
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
  constexpr GdtSegment() = default;

  constexpr GdtSegment(uint32_t base, uint32_t limit, uint8_t access,
                       uint8_t granularity) {
    this->set_limit(limit);
    this->set_base(base);
    this->set_access(access);
    this->set_granularity(granularity);
  }

  constexpr GdtSegment(uint8_t access, uint8_t granularity)
      : GdtSegment(0, 0xffff, access, granularity) {
  }

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

  constexpr uint8_t get_access() const {
    return this->access;
  }

  constexpr uint8_t get_granularity() const {
    return this->granularity;
  }

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

  constexpr void set_access(uint8_t access) {
    this->access = access;
  }

  constexpr void set_granularity(uint8_t granularity) {
    this->granularity = granularity;
  }

  uint16_t limit;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t limit_high : 4;
  uint8_t granularity : 4;
  uint8_t base_high;
} __attribute__((packed));

struct GdtTssSeg {
 public:
  constexpr GdtTssSeg() = default;

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

struct GdtTable {
 public:
  constexpr GdtTable() = default;

  void fill(GdtTss* tss);
  void print() const;

 private:
  GdtSegment entries[MAX_ENTRIES];
  GdtTssSeg tss_segment;
} __attribute__((packed));

struct GdtRegister {
 public:
  constexpr GdtRegister() = default;

  constexpr GdtRegister(GdtTable* gdt)
      : limit(sizeof(GdtTable) - 1), base(reinterpret_cast<uintptr_t>(gdt)) {
  }

  void load();

 private:
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

class Gdt {
 public:
  void initialize();

 private:
  GdtTable table;
  GdtTss tss;
};
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_GDT_HPP
