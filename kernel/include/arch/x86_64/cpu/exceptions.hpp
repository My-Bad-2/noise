#ifndef ARCH_CPU_EXCEPTIONS_HPP
#define ARCH_CPU_EXCEPTIONS_HPP 1

#include <stdint.h>

namespace arch::x86_64::cpu {
enum exception_type : uint8_t {
  exceptionDivideByZero = 0,          // Divide-by-zero exception (vector 0)
  exceptionDebug = 1,                 // Debug exception (vector 1)
  exceptionNonMaskableInterrupt = 2,  // Non-maskable interrupt (vector 2)
  exceptionBreakpoint = 3,            // Breakpoint exception (vector 3)
  exceptionOverflow = 4,              // Overflow exception (vector 4)
  exceptionBoundRange = 5,            // Bound range exception (vector 5)
  exceptionInvalidOpcode = 6,         // Invalid opcode exception (vector 6)
  exceptionDeviceNA = 7,            // Device not available exception (vector 7)
  exceptionDoubleFault = 8,         // Double fault exception (vector 8)
  exceptionInvalidTss = 10,         // Invalid TSS exception (vector 10)
  exceptionSegmentNotPresent = 11,  // Segment not present exception (vector 11)
  exceptionStackSegmentFault = 12,  // Stack segment fault exception (vector 12)
  exceptionGeneralProtectionFault = 13,  // General protection fault (vector 13)
  exceptionpageFault = 14,               // Page fault exception (vector 14)
  exceptionx87FloatingPoint = 16,   // x87 floating point exception (vector 16)
  exceptionAlignmentCheck = 17,     // Alignment check exception (vector 17)
  exceptionMachineCheck = 18,       // Machine check exception (vector 18)
  exceptionSimdFloatingPoint = 19,  // SIMD floating point exception (vector 19)
  exceptionVirtualization = 20,     // Virtualization exception (vector 20)
  exceptionControlProtection = 21,  // Control protection exception (vector 21)
  exceptionGypervisorInjection =
      28,                          // Hypervisor injection exception (vector 28)
  exceptionVmmCommunication = 29,  // VMM communication exception (vector 29)
  exceptionSecurity = 30           // Security exception (vector 30)
};

enum interruptType : uint8_t {
  platformInterruptBase = 32,  // Base value for platform interrupts
  platformMax = 255,           // Maximum value for platform interrupts

  irqSystemTimer = 32,   // IRQ for the system timer
  irqKeyboard = 33,      // IRQ for the keyboard
  irqCascade = 34,       // IRQ for the cascade
  irqSerialPort2 = 35,   // IRQ for serial port 2
  irqSerialPort1 = 36,   // IRQ for serial port 1
  irqSound = 37,         // IRQ for sound
  irqFloppy = 38,        // IRQ for the floppy disk controller
  irqParallelPort = 39,  // IRQ for the parallel port
  irqRtc = 40,           // IRQ for the real-time clock
  irqAcpi = 41,          // IRQ for the ACPI
  irq10 = 42,            // IRQ 10
  irq11 = 43,            // IRQ 11
  irqMouse = 44,         // IRQ for the mouse
  irqCpuCOP = 45,        // IRQ for CPU coprocessor
  irqPrimaryAta = 46,    // IRQ for primary ATA (IDE) controller
  irqSecondaryAta = 47,  // IRQ for secondary ATA (IDE) controller

  interruptLocalApicBase = 240,  // Base value for local APIC interrupts
  interruptApicSpurious = 240,   // APIC spurious interrupt
  interruptApicTimer = 241,      // APIC timer interrupt
  interruptApicError = 242,      // APIC error interrupt
  interruptApicPmi = 243,        // APIC Performance Monitoring Interrupt
  interruptIpiGeneric = 244,     // Interprocessor Interrupt for generic use
  interruptIpiReschedule = 245,  // Interprocessor Interrupt for rescheduling
  interruptIpiInterrupt = 246,   // Interprocessor Interrupt for SI
  interruptIpiHalt = 247  // Interprocessor Interrupt to halt the processor
};

struct IFrame {
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
  uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
  uint64_t vector, error_code, rip, cs, rflags, rsp, ss;

  void print() const;
};

struct NmiFrame {
  IFrame regs;
  void* expected_gs;
};

using HandlerFunc = void (*)(IFrame*, void*);

struct InterruptHandler {
 public:
  constexpr InterruptHandler() = default;

  constexpr InterruptHandler(const InterruptHandler& other) {
    this->handler = other.handler;
    this->cookie = other.cookie;
    this->vector = other.vector;
  }

  constexpr bool reserve(int vector) {
    if (this->is_reserved()) {
      return false;
    }

    this->vector = vector;
    return true;
  }

  bool set(HandlerFunc func, void* cookie = nullptr) {
    if (this->handler != nullptr) {
      return false;
    }

    this->cookie = cookie;
    this->handler = func;

    return true;
  }

  constexpr bool is_used() const {
    return this->handler != nullptr;
  }

  constexpr bool is_reserved() const {
    return this->vector != 0;
  }

  constexpr void reset() {
    this->handler = nullptr;
    this->cookie = nullptr;
  }

  bool operator()(IFrame* frame) {
    if (!this->is_reserved()) {
      return false;
    }

    this->handler(frame, this->cookie);
    return true;
  }

 private:
  void* cookie;
  HandlerFunc handler;
  uint8_t vector;
};

InterruptHandler& allocate_handler(int hint = platformInterruptBase);
InterruptHandler& get_handler(int vector);
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_EXCEPTIONS_HPP
