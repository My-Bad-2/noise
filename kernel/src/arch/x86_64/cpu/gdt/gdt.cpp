#include "arch/x86_64/cpu/gdt.hpp"
#include <log.hpp>

#define KERNEL_CODE 1
#define KERNEL_DATA 2
#define USER_DATA 3
#define USER_CODE 4

extern "C" void load_gdt(arch::x86_64::cpu::GdtRegister*);
extern "C" void load_tss();

namespace arch::x86_64::cpu {
void GdtTable::fill(GdtTss* tss) {
  this->entries[0] = {};
  this->entries[KERNEL_CODE] = {gdtCodeSegment, gdtLongMode | gdtGranularity};
  this->entries[KERNEL_DATA] = {gdtDataSegment, gdtDB | gdtGranularity};
  this->entries[USER_DATA] = {gdtDataSegment | gdtUser, gdtDB | gdtGranularity};
  this->entries[USER_CODE] = {gdtCodeSegment | gdtUser,
                              gdtLongMode | gdtGranularity};

  this->tss_segment = {tss};
}

void GdtTable::print() const {
  for (int i = 0; i < MAX_ENTRIES; ++i) {
    const GdtSegment& seg = this->entries[i];
    debug(
        "Segment %d:\n\tBase: 0x%x\n\tLimit: 0x%x\n\tAccess: "
        "0x%x\n\tGranularity: 0x%x",
        i, seg.get_base(), seg.get_limit(), seg.get_access(),
        seg.get_granularity());
  }

  debug("Tss Segment:\n\tBase: 0x%lx\n\tFlags: 0x%x",
        this->tss_segment.get_base(), this->tss_segment.get_flags());
}

void GdtRegister::load() {
  load_gdt(this);
  load_tss();
}

void Gdt::initialize() {
  this->table.fill(&this->tss);

#ifdef NOISE_DEBUG
  this->table.print();
#endif

  GdtRegister gdtr = {&this->table};
  gdtr.load();
}
}  // namespace arch::x86_64::cpu
