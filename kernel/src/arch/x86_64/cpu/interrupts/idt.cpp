#include "arch/x86_64/cpu/exceptions.hpp"
#include "arch/x86_64/cpu/idt.hpp"
#include "log.hpp"
#include "arch/x86_64/cpu/pic.hpp"

extern "C" uintptr_t isr_table[MAX_IDT_ENTRIES];
extern "C" void load_idt(arch::x86_64::cpu::IdtRegister*);

namespace arch::x86_64::cpu {
void IdtTable::initialize() {
  // Build IDT entries: all present interrupt gates, user-accessible for breakpoint.
  for (int i = 0; i < MAX_IDT_ENTRIES; ++i) {
    const uint8_t attributes = segmentPresent | interruptGate |
                               ((i == exceptionBreakpoint) ? dplUser : 0);
    this->entries[i] = {isr_table[i], 0, attributes, 0x8 /* kernel CS */};
  }
  debug("[IDT] Initialized %d entries", MAX_IDT_ENTRIES);
}

void IdtTable::print() const {
  // Print a subset for brevity; fix bug to index [i], not [0].
  for (int i = 0; i < MAX_IDT_ENTRIES; ++i) {
    const IdtSegment& seg = this->entries[i];
    debug("IDT seg %d:\n\tBase: 0x%lx\n\tAttr: 0x%x\n\tSelector: 0x%x\n\tIST: 0x%x",
          i, seg.get_base(), seg.get_attributes(), seg.get_selector(),
          seg.get_ist());
  }
  debug("ISR table base: %p", isr_table);
}

void IdtRegister::load() {
  debug("[IDT] Loading IDTR base=0x%lx limit=0x%x",
        static_cast<uintptr_t>(this->base), static_cast<uint16_t>(this->limit));
  load_idt(this);
  debug("[IDT] Loaded");
}

void Idt::initialize() {
  this->table.initialize();

  IdtRegister idtr = {&this->table};
  idtr.load();
  // Optionally: this->table.print();
}

// Provide a definition for the header-declared helper.
// This routes EOI to the legacy PIC.
void send_eoi(uint8_t vector) {
#ifdef NOISE_DEBUG
  debug("[IDT] EOI vector=%u", vector);
#endif
  Pic::eoi(vector);
}
}  // namespace arch::x86_64::cpu
