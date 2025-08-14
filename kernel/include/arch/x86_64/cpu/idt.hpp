#ifndef ARCH_CPU_IDT_HPP
#define ARCH_CPU_IDT_HPP 1

#include <stddef.h>
#include <stdint.h>

#define MAX_IDT_ENTRIES 256

namespace arch::x86_64::cpu {
enum TypeAttributes : uint8_t {
  interruptGate = 0xE,
  trapGate = 0xF,
  dplUser = (3 << 5),
  segmentPresent = (1 << 7),
};

struct IdtSegment {
 public:
  constexpr IdtSegment() = default;

  constexpr IdtSegment(uintptr_t base, uint8_t ist, uint8_t attributes,
                       uint8_t selector)
      : base_low(base & 0xffff),
        selector(selector),
        ist(ist),
        type_attributes(attributes),
        base_mid((base >> 16) & 0xffff),
        base_high((base >> 32) & 0xffffffff),
        rsvd(0) {
  }

  constexpr uintptr_t get_base() const {
    uintptr_t val = this->base_low;
    val |= (static_cast<uintptr_t>(this->base_mid) << 16);
    val |= (static_cast<uintptr_t>(this->base_high) << 32);

    return val;
  }

  constexpr uint8_t get_attributes() const {
    return this->type_attributes;
  }

  constexpr uint8_t get_selector() const {
    return this->selector;
  }

  constexpr uint8_t get_ist() const {
    return this->ist;
  }

 private:
  uint16_t base_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attributes;
  uint16_t base_mid;
  uint32_t base_high;
  uint32_t rsvd;
} __attribute__((packed));

struct IdtTable {
 public:
  void initialize();
  void print() const;

 private:
  IdtSegment entries[MAX_IDT_ENTRIES];
};

struct IdtRegister {
  constexpr IdtRegister() = default;

  constexpr IdtRegister(IdtTable* idt)
      : limit(sizeof(IdtTable) - 1), base(reinterpret_cast<uintptr_t>(idt)) {
  }

  void load();

 private:
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

class Idt {
 public:
  void initialize();

 private:
  IdtTable table;
};

void send_eoi(uint8_t vector);
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_IDT_HPP
