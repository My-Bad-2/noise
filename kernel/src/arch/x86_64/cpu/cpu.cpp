#include "arch/x86_64/cpu/cpu.hpp"
#include "log.hpp"  // debug/info logging

#include <cpuid.h>
#include <string.h>

#include <bits/feats.h>

#include "arch/x86_64/registers.h"

#define MAX_SUPPORTED_CPUID (0x17)
#define MAX_SUPPORTED_CPUID_HYP (0x40000001)
#define MAX_SUPPORTED_CPUID_EXT (0x8000001e)

namespace arch::x86_64::cpu {
enum PatTypes : size_t {
  Uncached = 0x00,
  WriteCombining = 0x01,
  WriteThrough = 0x04,
  WriteProtected = 0x05,
  WriteBack = 0x06,
  WeaklyUncached = 0x07,
};

constexpr inline uint64_t pat_state =
    (WriteCombining << 40) | (WriteProtected << 32) | (Uncached << 24) |
    (WeaklyUncached << 16) | (WriteThrough << 8) | WriteBack;

namespace {
CpuidLeaf cpuid[MAX_SUPPORTED_CPUID + 1];
CpuidLeaf cpuid_ext[MAX_SUPPORTED_CPUID_EXT - CpuidExtBase + 1];
CpuidLeaf cpuid_hyp[MAX_SUPPORTED_CPUID_HYP - CpuidHypBase + 1];

uint32_t max_cpuid = 0;
uint32_t max_hyp_cpuid = 0;
uint32_t max_ext_cpuid = 0;

ModelInfo model_info;
VendorList vendor;

const CpuidLeaf* get_leaf(CpuidLeafNum leaf) {
  if (leaf < CpuidHypBase) {
    if (unlikely(leaf > max_cpuid)) {
      return nullptr;
    }

    return &cpuid[leaf];
  } else if (leaf < CpuidExtBase) {
    if (unlikely(leaf > max_hyp_cpuid)) {
      return nullptr;
    }

    return &cpuid_hyp[leaf - CpuidHypBase];
  }

  if (unlikely(leaf > max_ext_cpuid)) {
    return nullptr;
  }

  return &cpuid_ext[leaf - CpuidExtBase];
}
}  // namespace

bool test_feature(CpuidBit bit) {
  if (bit.word > 3 || bit.bit > 31) {
    return false;
  }

  const CpuidLeaf* leaf = get_leaf(bit.leaf_num);

  if (leaf == nullptr) {
    return false;
  }

  switch (bit.word) {
    case 0:
      return !!((1u << bit.bit) & leaf->a);
    case 1:
      return !!((1u << bit.bit) & leaf->b);
    case 2:
      return !!((1u << bit.bit) & leaf->c);
    case 3:
      return !!((1u << bit.bit) & leaf->d);
    default:
      return false;
  }
}

void initialize() {
  // CPUID base leaf: discover max base leaf and vendor ID
  __cpuid(0, cpuid[0].a, cpuid[0].b, cpuid[0].c, cpuid[0].d);
  max_cpuid = cpuid[0].a;

  if (max_cpuid > MAX_SUPPORTED_CPUID) {
    max_cpuid = MAX_SUPPORTED_CPUID;
  }

  union {
    uint32_t vendor_id[3];
    char vendor_str[12];
  } vendor_info;

  vendor_info.vendor_id[0] = cpuid[0].b;
  vendor_info.vendor_id[1] = cpuid[0].d;
  vendor_info.vendor_id[2] = cpuid[0].c;

  if (!memcmp(vendor_info.vendor_str, "GenuineIntel",
              sizeof(vendor_info.vendor_str))) {
    vendor = VendorIntel;
  } else if (!memcmp(vendor_info.vendor_str, "AuthenticAMD",
                     sizeof(vendor_info.vendor_str))) {
    vendor = VendorAMD;
  } else {
    vendor = VendorUnknown;
  }

  // Enumerate base leaves 1..max_cpuid (inclusive)  [BUGFIX: was '< max_cpuid']
  for (uint32_t i = (CpuidBase + 1); i <= max_cpuid; ++i) {
    __cpuid(i, cpuid[i].a, cpuid[i].b, cpuid[i].c, cpuid[i].d);
  }

  // Extended CPUID range discovery and enumeration
  __cpuid(CpuidExtBase, cpuid_ext[0].a, cpuid_ext[0].b, cpuid_ext[0].c,
          cpuid_ext[0].d);
  max_ext_cpuid = cpuid_ext[0].a;

  if (max_ext_cpuid > MAX_SUPPORTED_CPUID_EXT) {
    max_ext_cpuid = MAX_SUPPORTED_CPUID_EXT;
  }

  for (uint32_t i = CpuidExtBase + 1; i < (max_ext_cpuid + 1); ++i) {
    uint32_t index = i - CpuidExtBase;
    __cpuid_count(i, 0, cpuid_ext[index].a, cpuid_ext[index].b,
                  cpuid_ext[index].c, cpuid_ext[index].d);
  }

  // Hypervisor CPUID range discovery and enumeration
  __cpuid(CpuidHypBase, cpuid_hyp[0].a, cpuid_hyp[0].b, cpuid_hyp[0].c,
          cpuid_hyp[0].d);
  max_hyp_cpuid = cpuid_hyp[0].a;

  if (max_hyp_cpuid > MAX_SUPPORTED_CPUID_HYP) {
    max_hyp_cpuid = MAX_SUPPORTED_CPUID_HYP;
  }

  for (uint32_t i = CpuidHypBase + 1; i < (max_hyp_cpuid + 1); ++i) {
    uint32_t index = i - CpuidHypBase;
    __cpuid_count(i, 0, cpuid_hyp[index].a, cpuid_hyp[index].b,
                  cpuid_hyp[index].c, cpuid_hyp[index].d);
  }

  // Decode model/family/stepping from standard leaf 1 if present
  const CpuidLeaf* leaf = get_leaf(CpuidModelFeatures);

  if (leaf != nullptr) {
    model_info.processor_type =
        static_cast<uint8_t>(bits_shift(leaf->a, 13, 12));
    model_info.family = static_cast<uint8_t>(bits_shift(leaf->a, 11, 8));
    model_info.model = static_cast<uint8_t>(bits_shift(leaf->a, 7, 4));
    model_info.stepping = static_cast<uint8_t>(bits_shift(leaf->a, 3, 0));

    model_info.display_family = model_info.family;
    model_info.display_model = model_info.model;

    // Extended family
    if (model_info.family == 0xf) {
      model_info.display_family +=
          static_cast<uint8_t>(bits_shift(leaf->a, 27, 20));
    }

    // Extended model: add high 4 bits to model (BUGFIX: was adding to family)
    if ((model_info.family == 0xf) || model_info.family == 0x6) {
      model_info.display_model +=
          static_cast<uint8_t>(bits_shift(leaf->a, 19, 16) << 4);
    }
  }

  enable_pat();

  // Log a brief CPU summary
  info("[CPU] Vendor: %.*s | Max CPUID: base=0x%x ext=0x%x hyp=0x%x",
       (int)sizeof(vendor_info.vendor_str), vendor_info.vendor_str, max_cpuid,
       max_ext_cpuid, max_hyp_cpuid);
  info("[CPU] Family: 0x%x (disp 0x%x) Model: 0x%x (disp 0x%x) Stepping: 0x%x",
       model_info.family, model_info.display_family, model_info.model,
       model_info.display_model, model_info.stepping);
}

ModelInfo get_model_info() {
  return model_info;
}

void enable_pat() {
  write_msr(MSR_PAT, pat_state);
}
}  // namespace arch::x86_64::cpu
