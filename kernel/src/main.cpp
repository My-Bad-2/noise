#include "arch/arch.hpp"
#include "drivers/manager.hpp"

extern "C" void kmain() {
  arch::initialize();
  drivers::initialize();

  arch::test();
  arch::halt();
}