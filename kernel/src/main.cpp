#include "arch/arch.hpp"
#include "drivers/manager.hpp"
#include "log.hpp"

extern "C" void kmain() {
  arch::initialize();
  drivers::initialize();

  info("Hello, World! {}", "testing");
  arch::halt();
}