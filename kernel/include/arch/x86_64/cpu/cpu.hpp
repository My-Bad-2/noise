#ifndef ARCH_CPU_HPP
#define ARCH_CPU_HPP

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
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_HPP