#include "arch/arch.hpp"
#include "drivers/manager.hpp"
#include "log.hpp"
#include "version.hpp"

#include <printf_config.h>

#include <printf/printf.h>

extern "C" void putchar_(char c) {
  arch::write(c);
}

extern "C" void kmain() {
  arch::initialize();
  drivers::initialize();

  KernelInfo info;
  info.print();

  info("Hello, World! {}", "testing");
  arch::halt();
}
