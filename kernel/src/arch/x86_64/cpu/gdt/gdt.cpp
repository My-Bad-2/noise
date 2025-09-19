#include "arch/x86_64/cpu/gdt.hpp"
#include "log.hpp"

// Indices into GDT (each index is 8 bytes; selector = index << 3)
#define KERNEL_CODE 1  // selector 0x08
#define KERNEL_DATA 2  // selector 0x10
#define USER_DATA 3    // selector 0x18
#define USER_CODE 4    // selector 0x20

extern "C" void load_gdt(arch::x86_64::cpu::GdtRegister*);
extern "C" void load_tss();

namespace arch::x86_64::cpu {
void GdtTable::fill(GdtTss* tss) {
  // Build simple GDT:
  //  0x00: null
  //  0x08: kernel code (ring 0, 64-bit)
  //  0x10: kernel data (ring 0)
  //  0x18: user data   (ring 3)
  //  0x20: user code   (ring 3, 64-bit)
  //  0x28: TSS descriptor (16 bytes) follows entries[] in memory
  this->entries[0] = {};
  this->entries[KERNEL_CODE] = {gdtCodeSegment, gdtLongMode | gdtGranularity};
  this->entries[KERNEL_DATA] = {gdtDataSegment, gdtDB | gdtGranularity};
  this->entries[USER_DATA] = {gdtDataSegment | gdtUser, gdtDB | gdtGranularity};
  this->entries[USER_CODE] = {gdtCodeSegment | gdtUser,
                              gdtLongMode | gdtGranularity};

  // Install TSS descriptor (base points to GdtTss structure)
  this->tss_segment = {tss};
}

void GdtTable::print() const {
  // Dump GDT entries and TSS descriptor for debugging.
  for (int i = 0; i < MAX_ENTRIES; ++i) {
    const GdtSegment& seg = this->entries[i];
    debug("GDT seg %d:\n\tBase: 0x%x\n\tLimit: 0x%x\n\tAccess: 0x%x\n\tGran: 0x%x",
          i, seg.get_base(), seg.get_limit(), seg.get_access(),
          seg.get_granularity());
  }

  debug("GDT TSS:\n\tBase: 0x%lx\n\tFlags: 0x%x",
        this->tss_segment.get_base(), this->tss_segment.get_flags());
}

void GdtRegister::load() {
  // Install GDTR and load TR with TSS selector; CS reload via far return in ASM.
  debug("[GDT] Loading GDTR base=0x%lx limit=0x%x",
        static_cast<uintptr_t>(this->base), static_cast<uint16_t>(this->limit));

  load_gdt(this);
  load_tss();

  debug("[GDT] Loaded GDTR and TSS");
}

void Gdt::initialize() {
  // Fill table with segments and TSS, then load GDTR/TR.
  // Note: If you need a known RSP0, initialize 'tss' with GdtTss(rsp0).
  this->table.fill(&this->tss);

  GdtRegister gdtr = {&this->table};
  gdtr.load();
}
}  // namespace arch::x86_64::cpu
