#include "arch/x86_64/arch.hpp"
#include "arch/x86_64/cpu/exceptions.hpp"
#include "arch/x86_64/cpu/idt.hpp"
#include "log.hpp"

namespace arch::x86_64::cpu {
namespace {
// Flat array of platform interrupt handlers [32..255]
InterruptHandler handlers[platformMax - platformInterruptBase + 1];
}  // namespace

void IFrame::print() const {
  info(
      "\n\tCS : 0x%.16lx RIP: 0x%.16lx EFL: 0x%.16lx\n\t"
      "RAX: 0x%.16lx RBX: 0x%.16lx RCX: 0x%.16lx\n\t"
      "RDX: 0x%.16lx RSI: 0x%.16lx RBP: 0x%.16lx\n\t"
      "RSP: 0x%.16lx R8 : 0x%.16lx R9 : 0x%.16lx\n\t"
      "R10: 0x%.16lx R11: 0x%.16lx R12: 0x%.16lx\n\t"
      "R13: 0x%.16lx R14: 0x%.16lx R15: 0x%.16lx\n\t"
      "EC : 0x%.16lx USP: 0x%.16lx USS: 0x%.16lx",
      this->cs, this->rip, this->rflags, this->rax, this->rbx, this->rcx,
      this->rdx, this->rsi, this->rbp, this->rsp, this->r8, this->r9, this->r10,
      this->r11, this->r12, this->r13, this->r14, this->r15, this->error_code,
      this->rsp, this->ss);
}

InterruptHandler& allocate_handler(int hint) {
  // Reserve a handler slot starting from 'hint' (or base if earlier).
  const int start =
      (hint < platformInterruptBase) ? platformInterruptBase : hint;
  const int base = platformInterruptBase;
  const int count = platformMax - platformInterruptBase + 1;

  for (int i = start - base; i < count; ++i) {
    if (handlers[i].is_reserved()) {
      continue;
    }

    handlers[i].reserve(i + base);
    debug("[IDT][ALLOC] Reserved handler vector=%d (index=%d)", i + base, i);
    return handlers[i];
  }

  panic("Out of Known Interrupt Handler!");
  while (true);
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
    // CPU exception (vectors 0..31)
    frame->print();
    panic("Exception %lu Triggered!", frame->vector);
  } else {
    // External IRQ or platform interrupt (vectors >= 32)
    InterruptHandler& handler = handlers[frame->vector - platformInterruptBase];

    if (handler.is_used()) {
      debug("[IDT][DISPATCH] vector=%lu", frame->vector);
      interrupt_handled = handler(frame);
    } else {
      debug("[IDT][DISPATCH] No handler for vector=%lu", frame->vector);
    }
  }

  if (!interrupt_handled) {
    arch::x86_64::halt(false);
  }

  // Only send EOI for external interrupts (not CPU exceptions)
  if (frame->vector >= platformInterruptBase) {
    send_eoi(static_cast<uint8_t>(frame->vector));
  }
}

extern "C" void nmi_handler(NmiFrame* frame) {
  bool interrupt_handled = false;

  frame->regs.print();

  if (!interrupt_handled) {
    arch::x86_64::halt(false);
  }
}
}  // namespace arch::x86_64::cpu
