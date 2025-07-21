#ifndef ARCH_HPP
#define ARCH_HPP 1

#include <string_view>

namespace arch::x86_64 {
[[noreturn]] void halt(bool interrupts = true);

void initialize();
void write(char ch);
void write(std::string_view ch);
}  // namespace arch::x86_64

#endif  // ARCH_HPP