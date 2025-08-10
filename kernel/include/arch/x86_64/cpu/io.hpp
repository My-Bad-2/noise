#ifndef ARCH_CPU_IO_HPP
#define ARCH_CPU_IO_HPP 1

#include <stdint.h>

#include <concepts>

namespace arch::x86_64::cpu {
template <typename T>
  requires(sizeof(T) <= sizeof(uint32_t))
inline T in(uint16_t port) {
  T ret = 0;

  if constexpr (std::same_as<T, uint8_t>) {
    asm volatile("in %b0, %w1" : "=a"(ret) : "Nd"(port));
  } else if constexpr (std::same_as<T, uint16_t>) {
    asm volatile("in %w0, %w1" : "=a"(ret) : "Nd"(port));
  } else if constexpr (std::same_as<T, uint32_t>) {
    asm volatile("in %0, %w1" : "=a"(ret) : "Nd"(port));
  } else {
    static_assert(false,
                  "Unsupported type size for cpu::in() operation. Only 1, 2, "
                  "or 4 bytes supported.");
  }

  return ret;
}

template <typename T>
  requires(sizeof(T) <= sizeof(uint32_t))
inline void out(uint16_t port, T value) {
  if constexpr (std::same_as<T, uint8_t>) {
    asm volatile("out %w1, %b0" : : "a"(value), "Nd"(port));
  } else if constexpr (std::same_as<T, uint16_t>) {
    asm volatile("out %w1, %w0" : : "a"(value), "Nd"(port));
  } else if constexpr (std::same_as<T, uint32_t>) {
    asm volatile("out %w1, %0" : : "a"(value), "Nd"(port));
  } else {
    static_assert(false,
                  "Unsupported type size for cpu::out() operation. Only 1, 2, "
                  "or 4 bytes supported.");
  }
}

inline void io_wait() {
  out<uint8_t>(0x80, 0);
}
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_IO_HPP
