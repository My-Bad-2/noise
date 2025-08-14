#ifndef ARCH_CPU_PIC_HPP
#define ARCH_CPU_PIC_HPP 1

#include "arch/x86_64/cpu/exceptions.hpp"

#include <stdint.h>

namespace arch::x86_64::cpu {
class Pic {
 public:
  static void remap(uint8_t offset1 = platformInterruptBase,
                    uint8_t offset2 = platformInterruptBase + 8);
  static void disable();

  static void eoi(uint8_t irq);
  static void set_mask(uint8_t irq);
  static void clear_mask(uint8_t irq);

  static uint16_t get_irr();
  static uint16_t get_isr();
};
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_PIC_HPP
