#ifndef MEMORY_HPP
#define MEMORY_HPP 1

#include "boot.hpp"

#include <stddef.h>
#include <stdint.h>

#include <concepts>
#include <type_traits>

namespace memory {
enum PageSize : size_t {
  PageSize4KiB = 0x1000,
  PageSize2MiB = PageSize4KiB * 512,
  PageSize1GiB = PageSize2MiB * 512,
};

template <typename T>
using RetType = std::conditional_t<
    std::is_integral<T>::value,
    std::conditional_t<std::is_unsigned<T>::value, uintptr_t, intptr_t>, T>;

inline constexpr auto align_down(std::unsigned_integral auto addr,
                                 std::unsigned_integral auto base) {
  return uintptr_t(addr) & ~(uintptr_t(base) - 1);
}

inline constexpr auto align_up(std::unsigned_integral auto addr,
                               std::unsigned_integral auto base) {
  return align_down(addr + base - 1, base);
}

inline constexpr auto div_roundup(std::unsigned_integral auto addr,
                                  std::unsigned_integral auto base) {
  return align_up(addr, base) / base;
}

bool is_aligned(std::unsigned_integral auto addr,
                std::unsigned_integral auto base) {
  return (uintptr_t(addr) & uintptr_t(base)) == 0;
}

inline constexpr bool is_higher_half(auto val) {
  return uintptr_t(val) >= boot::get_hhdm_offset();
}

template <typename T, typename Ret = RetType<T>>
inline constexpr Ret to_higher_half(T val) {
  return is_higher_half(val) ? Ret(val)
                             : Ret(uintptr_t(val) + boot::get_hhdm_offset());
}

template <typename T, typename Ret = RetType<T>>
inline constexpr Ret from_higher_half(T val) {
  return !is_higher_half(val) ? Ret(val)
                              : Ret(uintptr_t(val) - boot::get_hhdm_offset());
}

void initialize();
}  // namespace memory

#endif  // MEMORY_HPP
