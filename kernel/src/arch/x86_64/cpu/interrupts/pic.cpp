#include "arch/x86_64/cpu/exceptions.hpp"
#include "arch/x86_64/cpu/io.hpp"
#include "arch/x86_64/cpu/pic.hpp"
#include "log.hpp"

#define CASCADE_IRQ 2

namespace arch::x86_64::cpu {
namespace {
enum PicPorts : uint8_t {
  Pic1 = 0x20,
  Pic1Command = Pic1,
  Pic1Data = Pic1 + 1,
  Pic2 = 0xA0,
  Pic2Command = Pic2,
  Pic2Data = Pic2 + 1,
};

enum PicCommands : uint8_t {
  Icw1Icw4 = 0x01,
  Icw1Single = 0x02,
  Icw1Interval4 = 0x04,
  Icw1Level = 0x08,
  Icw1Init = 0x10,

  Icw48086 = 0x01,
  Icw4Auto = 0x02,
  Icw4BufSlave = 0x08,
  Icw4BufMaster = 0x0C,
  Icw4SFNM = 0x10,

  PicEoi = 0x20,

  PicReadIrr = 0x0A,
  PicReadIsr = 0x0B,
};

inline uint16_t get_irq_reg(int ocw3) {
  out<uint8_t>(Pic1Command, ocw3);
  out<uint8_t>(Pic2Command, ocw3);

  return in<uint8_t>(Pic1Command) | (in<uint8_t>(Pic2Command) << 8);
}
}  // namespace

void Pic::remap(uint8_t offset1, uint8_t offset2) {
  // starts the initialization sequence (in cascade mode)
  out<uint8_t>(Pic1Command, Icw1Init | Icw1Icw4);
  io_wait();
  out<uint8_t>(Pic2Command, Icw1Init | Icw1Icw4);
  io_wait();

  // ICW2: Naster PIC vector offset
  out<uint8_t>(Pic1Data, offset1);
  io_wait();

  // ICW2: Slave PIC vector offset
  out<uint8_t>(Pic2Data, offset2);
  io_wait();

  // ICW3: tell Master PIC that there is a slave PIC at IRQ2
  out<uint8_t>(Pic1Data, 1 << CASCADE_IRQ);
  io_wait();

  // ICW3: tell Slave PIC its cascade identity (0000 0010)
  out<uint8_t>(Pic2Data, CASCADE_IRQ);
  io_wait();

  // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  out<uint8_t>(Pic1Data, Icw48086);
  io_wait();
  out<uint8_t>(Pic2Data, Icw48086);
  io_wait();

  // Mask all IRQs
  Pic::disable();

  // Clear Mask for Cascade IRQ
  Pic::clear_mask(irqCascade);
}

void Pic::disable() {
#ifdef NOISE_DEBUG
  debug("[PIC] Masking all IRQs!");
#endif

  out<uint8_t>(Pic1Data, 0xff);
  out<uint8_t>(Pic2Data, 0xff);
}

void Pic::set_mask(uint8_t irq) {
#ifdef NOISE_DEBUG
  debug("[PIC] Masking Vector 0x%x", irq);
#endif

  uint8_t port = Pic1Data;
  irq -= platformInterruptBase;

  if (irq >= 8) {
    port = Pic2Data;
    irq -= 8;
  }

  uint8_t val = in<uint8_t>(port) | (1 << irq);
  out<uint8_t>(port, val);
}

void Pic::clear_mask(uint8_t irq) {
#ifdef NOISE_DEBUG
  debug("[PIC] Unmasking Vector 0x%x", irq);
#endif

  uint8_t port = Pic1Data;
  irq -= platformInterruptBase;

  if (irq >= 8) {
    port = Pic2Data;
    irq -= 8;
  }

  uint8_t val = in<uint8_t>(port) & ~(1 << irq);
  out<uint8_t>(port, val);
}

void Pic::eoi(uint8_t irq) {
  irq -= platformInterruptBase;

  if (irq >= 8) {
    out<uint8_t>(Pic2Command, PicEoi);
  }

  out<uint8_t>(Pic1Command, PicEoi);
}

uint16_t Pic::get_irr() {
  return get_irq_reg(PicReadIrr);
}

uint16_t Pic::get_isr() {
  return get_irq_reg(PicReadIsr);
}
}  // namespace arch::x86_64::cpu
