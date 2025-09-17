#include "arch/x86_64/arch.hpp"
#include "arch/x86_64/cpu/exceptions.hpp"
#include "arch/x86_64/cpu/pic.hpp"
#include "log.hpp"

namespace arch::x86_64::cpu {
namespace {
InterruptHandler handlers[platformMax - platformInterruptBase + 1];

inline void send_eoi(uint8_t vector) {
  Pic::eoi(vector);
}
}  // namespace

void IFrame::print() const {
  info(
      "\n\tCS : 0x%.16lx RIP: 0x%.16lx EFL: 0x%.16lx\n\t"
      "RAX: 0x%.16lx RBX: 0x%.16lx RCX: 0x%.16lx\n\t"
      "RDX: 0x%.16lx RSI: 0x%.16lx RBP: 0x%.16lx\n\t"
      "RSP: 0x%.16lx R8 : 0x%.16lx R9 : 0x%.16lx\n\t"
      "R10: 0x%.16lx R11: 0x%.16lx R12: 0x%.16lx\n\t"
      "R13: 0x%.16lx R14: 0x%.16lx R15: 0x%.16lx\n\t"
      "EC : 0x%.16lx USP: 0x%.16lx USS: 0x%.16lx\n\t",
      this->cs, this->rip, this->rflags, this->rax, this->rbx, this->rcx,
      this->rdx, this->rsi, this->rbp, this->rsp, this->r8, this->r9, this->r10,
      this->r11, this->r12, this->r13, this->r14, this->r15, this->error_code,
      this->rsp, this->ss);
}

InterruptHandler& allocate_handler(int hint) {
  for (int i = hint - platformInterruptBase; i < (platformMax + 1); ++i) {
    if (handlers[i].is_reserved()) {
      continue;
    }

    handlers[i].reserve(i + platformInterruptBase);
    return handlers[i];
  }

  panic("Out of Known Interrupt Handler!");
  while (true) {
  }
}

InterruptHandler& get_handler(int vector) {
  InterruptHandler& handler = handlers[vector - platformInterruptBase];

  if (!handler.is_reserved()) {
    panic("Interrupt Handler %d not found!", vector);
  }

  return handler;
}

extern "C" void exception_handler(IFrame* frame) {
  bool interrupt_handled = false;

  if (frame->vector < platformInterruptBase) {
    frame->print();
    panic("Exception %lu Triggered!", frame->vector);
  } else {
    InterruptHandler& handler = handlers[frame->vector - platformInterruptBase];

    if (handler.is_used()) {
      interrupt_handled = handler(frame);
    }
  }

  if (!interrupt_handled) {
    arch::x86_64::halt(false);
  }

  send_eoi(frame->vector);
}

extern "C" void nmi_handler(NmiFrame* frame) {
  bool interrupt_handled = false;

  frame->regs.print();

  if (!interrupt_handled) {
    arch::x86_64::halt(false);
  }
}
}  // namespace arch::x86_64::cpu
