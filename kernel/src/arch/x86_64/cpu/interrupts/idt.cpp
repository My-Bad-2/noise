#include "arch/x86_64/cpu/exceptions.hpp"
#include "arch/x86_64/cpu/idt.hpp"
#include "log.hpp"

extern "C" uintptr_t isr_table[MAX_IDT_ENTRIES];
extern "C" void load_idt(arch::x86_64::cpu::IdtRegister*);

namespace arch::x86_64::cpu {
void IdtTable::initialize() {
  for (int i = 0; i < MAX_IDT_ENTRIES; ++i) {
    const uint8_t attributes = segmentPresent | interruptGate |
                               ((i == exceptionBreakpoint) ? dplUser : 0);
    this->entries[i] = {isr_table[i], 0, attributes, 0x8};
  }
}

void IdtTable::print() const {
  for (int i = 0; i < MAX_IDT_ENTRIES; ++i) {
    IdtSegment seg = this->entries[0];

    debug(
        "Segment %d:\n\tBase: 0x%lx\n\tType Attributes: 0x%x\n\tSelector: "
        "0x%x\n\tIST: 0x%x",
        i, seg.get_base(), seg.get_attributes(), seg.get_selector(),
        seg.get_ist());
  }

  debug("Base ISR addr: %p", isr_table);
}

void IdtRegister::load() {
  load_idt(this);
}

void Idt::initialize() {
  this->table.initialize();

  IdtRegister idtr = {&this->table};
  idtr.load();
}
}  // namespace arch::x86_64::cpu
