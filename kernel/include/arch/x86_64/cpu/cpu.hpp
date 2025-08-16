#ifndef ARCH_CPU_HPP
#define ARCH_CPU_HPP 1

#include <stdint.h>

namespace arch::x86_64::cpu {
inline void pause() {
  asm volatile("pause");
}

inline void halt() {
  asm volatile("hlt");
}

inline void disable_interrupts() {
  asm volatile("cli");
}

inline void enable_interrupts() {
  asm volatile("sti");
}

inline void invalidate_page(uintptr_t addr) {
  asm volatile("invlpg %0" ::"m"(*reinterpret_cast<const char*>(addr)) : "memory");
}

inline void write_cr3(uintptr_t addr) {
  asm volatile("mov %0, %%cr3" ::"r"(addr) : "memory");
}
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_HPP
