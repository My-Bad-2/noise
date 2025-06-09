#include "arch/x86_64/arch.hpp"
#include "arch/x86_64/cpu/cpu.hpp"
#include "arch/x86_64/drivers/uart.hpp"
#include "drivers/manager.hpp"

namespace arch::x86_64 {
namespace {
drivers::UartDriver uart_driver;
}

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
}

void test() {
  uart_driver.putchar('H');
  uart_driver.putchar('E');
  uart_driver.putchar('L');
  uart_driver.putchar('L');
  uart_driver.putchar('O');
  uart_driver.putchar('!');
  uart_driver.putchar('\n');
}
}  // namespace arch::x86_64