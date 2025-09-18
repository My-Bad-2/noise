#ifndef ARCH_CPU_HPP
#define ARCH_CPU_HPP 1

#include <stdint.h>

#define CPUID_BIT(leaf, word, bit)                         \
  (::arch::x86_64::cpu::CpuidBit) {                          \
    (::arch::x86_64::cpu::CpuidLeafNum)(leaf), (word), (bit) \
  }

#define FEATURE_SSE3 CPUID_BIT(0x1, 2, 0)
#define FEATURE_MON CPUID_BIT(0x1, 2, 3)
#define FEATURE_VMX CPUID_BIT(0x1, 2, 5)
#define FEATURE_TM2 CPUID_BIT(0x1, 2, 8)
#define FEATURE_SSSE3 CPUID_BIT(0x1, 2, 9)
#define FEATURE_PDCM CPUID_BIT(0x1, 2, 15)
#define FEATURE_PCID CPUID_BIT(0x1, 2, 17)
#define FEATURE_SSE4_1 CPUID_BIT(0x1, 2, 19)
#define FEATURE_SSE4_2 CPUID_BIT(0x1, 2, 20)
#define FEATURE_X2APIC CPUID_BIT(0x1, 2, 21)
#define FEATURE_TSC_DEADLINE CPUID_BIT(0x1, 2, 24)
#define FEATURE_AESNI CPUID_BIT(0x1, 2, 25)
#define FEATURE_XSAVE CPUID_BIT(0x1, 2, 26)
#define FEATURE_AVX CPUID_BIT(0x1, 2, 28)
#define FEATURE_RDRAND CPUID_BIT(0x1, 2, 30)
#define FEATURE_HYPERVISOR CPUID_BIT(0x1, 2, 31)
#define FEATURE_FPU CPUID_BIT(0x1, 3, 0)
#define FEATURE_SEP CPUID_BIT(0x1, 3, 11)
#define FEATURE_CLFLUSH CPUID_BIT(0x1, 3, 19)
#define FEATURE_ACPI CPUID_BIT(0x1, 3, 22)
#define FEATURE_MMX CPUID_BIT(0x1, 3, 23)
#define FEATURE_FXSR CPUID_BIT(0x1, 3, 24)
#define FEATURE_SSE CPUID_BIT(0x1, 3, 25)
#define FEATURE_SSE2 CPUID_BIT(0x1, 3, 26)
#define FEATURE_TM CPUID_BIT(0x1, 3, 29)
#define FEATURE_DTS CPUID_BIT(0x6, 0, 0)
#define FEATURE_TURBO CPUID_BIT(0x6, 0, 1)
#define FEATURE_PLN CPUID_BIT(0x6, 0, 4)
#define FEATURE_PTM CPUID_BIT(0x6, 0, 6)
#define FEATURE_HWP CPUID_BIT(0x6, 0, 7)
#define FEATURE_HWP_NOT CPUID_BIT(0x6, 0, 8)
#define FEATURE_HWP_ACT CPUID_BIT(0x6, 0, 9)
#define FEATURE_HWP_PREF CPUID_BIT(0x6, 0, 10)
#define FEATURE_TURBO_MAX CPUID_BIT(0x6, 0, 14)
#define FEATURE_HW_FEEDBACK CPUID_BIT(0x6, 2, 0)
#define FEATURE_PERF_BIAS CPUID_BIT(0x6, 2, 3)
#define FEATURE_FSGSBASE CPUID_BIT(0x7, 1, 0)
#define FEATURE_TSC_ADJUST CPUID_BIT(0x7, 1, 1)
#define FEATURE_AVX2 CPUID_BIT(0x7, 1, 5)
#define FEATURE_SMEP CPUID_BIT(0x7, 1, 7)
#define FEATURE_ERMS CPUID_BIT(0x7, 1, 9)
#define FEATURE_INVPCID CPUID_BIT(0x7, 1, 10)
#define FEATURE_AVX512F CPUID_BIT(0x7, 1, 16)
#define FEATURE_AVX512DQ CPUID_BIT(0x7, 1, 17)
#define FEATURE_RDSEED CPUID_BIT(0x7, 1, 18)
#define FEATURE_SMAP CPUID_BIT(0x7, 1, 20)
#define FEATURE_AVX512IFMA CPUID_BIT(0x7, 1, 21)
#define FEATURE_CLFLUSHOPT CPUID_BIT(0x7, 1, 23)
#define FEATURE_CLWB CPUID_BIT(0x7, 1, 24)
#define FEATURE_PT CPUID_BIT(0x7, 1, 25)
#define FEATURE_AVX512PF CPUID_BIT(0x7, 1, 26)
#define FEATURE_AVX512ER CPUID_BIT(0x7, 1, 27)
#define FEATURE_AVX512CD CPUID_BIT(0x7, 1, 28)
#define FEATURE_AVX512BW CPUID_BIT(0x7, 1, 30)
#define FEATURE_AVX512VL CPUID_BIT(0x7, 1, 31)
#define FEATURE_AVX512VBMI CPUID_BIT(0x7, 2, 1)
#define FEATURE_UMIP CPUID_BIT(0x7, 2, 2)
#define FEATURE_PKU CPUID_BIT(0x7, 2, 3)
#define FEATURE_AVX512VBMI2 CPUID_BIT(0x7, 2, 6)
#define FEATURE_AVX512VNNI CPUID_BIT(0x7, 2, 11)
#define FEATURE_AVX512BITALG CPUID_BIT(0x7, 2, 12)
#define FEATURE_AVX512VPDQ CPUID_BIT(0x7, 2, 14)
#define FEATURE_AVX512QVNNIW CPUID_BIT(0x7, 3, 2)
#define FEATURE_AVX512QFMA CPUID_BIT(0x7, 3, 3)
#define FEATURE_MD_CLEAR CPUID_BIT(0x7, 3, 10)
#define FEATURE_IBRS_IBPB CPUID_BIT(0x7, 3, 26)
#define FEATURE_STIBP CPUID_BIT(0x7, 3, 27)
#define FEATURE_L1D_FLUSH CPUID_BIT(0x7, 3, 28)
#define FEATURE_ARCH_CAPABILITIES CPUID_BIT(0x7, 3, 29)
#define FEATURE_SSBD CPUID_BIT(0x7, 3, 31)
#define FEATURE_KVM_PV_CLOCK CPUID_BIT(0x40000001, 0, 3)
#define FEATURE_KVM_PV_EOI CPUID_BIT(0x40000001, 0, 6)
#define FEATURE_KVM_PV_IPI CPUID_BIT(0x40000001, 0, 11)
#define FEATURE_KVM_PV_CLOCK_STABLE CPUID_BIT(0x40000001, 0, 24)
#define FEATURE_AMD_TOPO CPUID_BIT(0x80000001, 2, 22)
#define FEATURE_SYSCALL CPUID_BIT(0x80000001, 3, 11)
#define FEATURE_NX CPUID_BIT(0x80000001, 3, 20)
#define FEATURE_HUGE_PAGE CPUID_BIT(0x80000001, 3, 26)
#define FEATURE_RDTSCP CPUID_BIT(0x80000001, 3, 27)
#define FEATURE_INVAR_TSC CPUID_BIT(0x80000007, 3, 8)
#define FEATURE_INVLPGB CPUID_BIT(0x80000008, 1, 3)

namespace arch::x86_64::cpu {
enum CpuidLeafNum {
  CpuidBase = 0,
  CpuidModelFeatures = 0x1,
  CpuidCacheV1 = 0x2,
  CpuidCacheV2 = 0x4,
  CpuidMon = 0x5,
  CpuidThermalAndPower = 0x6,
  CpuidExtendedFeatureFlags = 0x7,
  CpuidPerformanceMonitoring = 0xa,
  CpuidTopology = 0xb,
  CpuidXsave = 0xd,
  CpuidPT = 0x14,
  CpuidTSC = 0x15,

  CpuidHypBase = 0x40000000,
  CpuidHypVendor = 0x40000000,
  CpuidKvmFeatures = 0x40000001,

  CpuidExtBase = 0x80000000,
  CpuidBrand = 0x80000002,
  CpuidAddrWidth = 0x80000008,
  CpuidAMDTopology = 0x8000001e,
};

enum VendorList {
  VendorUnknown,
  VendorIntel,
  VendorAMD,
};

enum x86_hypervisor_list {
  HypervisorUnknown,
  HypervisorNone,
  HypervisorKvm,
};

struct CpuidBit {
  CpuidLeafNum leaf_num;
  uint8_t word;
  uint8_t bit;
};

struct CpuidLeaf {
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
};

struct ModelInfo {
  uint8_t processor_type;
  uint8_t family;
  uint8_t model;
  uint8_t stepping;

  uint32_t display_family;
  uint32_t display_model;

  uint32_t patch_level;
};

inline void pause() {
  asm volatile("pause");
}

inline void halt() {
  asm volatile("hlt");
}

inline void disable_interrupts() {
  asm volatile("cli");
}

inline void enable_interrupts() {
  asm volatile("sti");
}

inline void invalidate_page(uintptr_t addr) {
  asm volatile("invlpg %0" ::"m"(*reinterpret_cast<const char*>(addr))
               : "memory");
}

inline void write_cr3(uintptr_t addr) {
  asm volatile("mov %0, %%cr3" ::"r"(addr) : "memory");
}

bool test_feature(CpuidBit bit);
ModelInfo get_model_info();
void initialize();
}  // namespace arch::x86_64::cpu

#endif  // ARCH_CPU_HPP
