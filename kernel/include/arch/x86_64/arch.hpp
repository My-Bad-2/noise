#ifndef ARCH_HPP
#define ARCH_HPP 1

namespace arch::x86_64 {
[[noreturn]] void halt(bool interrupts = true);
inline void pause() {
  asm volatile("pause");
}

bool int_status();
void int_switch(bool on);

void initialize();
void write(char ch);
void write(const char* ch);
}  // namespace arch::x86_64

#endif  // ARCH_HPP
