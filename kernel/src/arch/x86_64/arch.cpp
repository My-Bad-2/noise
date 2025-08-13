#include "arch/x86_64/arch.hpp"
#include "arch/x86_64/cpu/cpu.hpp"
#include "arch/x86_64/cpu/gdt.hpp"
#include "arch/x86_64/drivers/uart.hpp"
#include "drivers/manager.hpp"
#include "arch/x86_64/cpu/idt.hpp"

namespace arch::x86_64 {
namespace {
drivers::UartDriver uart_driver;
cpu::Gdt gdt;
cpu::Idt idt;
}  // namespace

void halt(bool interrupts) {
  if (interrupts) {
    while (true) {
      cpu::halt();
    }
  } else {
    while (true) {
      cpu::disable_interrupts();
      cpu::halt();
    }
  }
}

void initialize() {
  uart_driver.set_port(drivers::PORT_A);

  drivers::install(&uart_driver);

  cpu::disable_interrupts();

  gdt.initialize();
  idt.initialize();

  cpu::enable_interrupts();
}

void write(char ch) {
  uart_driver.putchar(ch);
}

void write(const char* str) {
  while (*str) {
    write(*str++);
  }
}
}  // namespace arch::x86_64
