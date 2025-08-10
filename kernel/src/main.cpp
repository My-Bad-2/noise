#include "arch/arch.hpp"
#include "drivers/manager.hpp"
#include "log.hpp"
#include "version.hpp"

extern "C" void kmain() {
  arch::initialize();
  drivers::initialize();

  KernelInfo info;

  info.print();

  info("Hello, World! {}", "testing");
  arch::halt();
}
