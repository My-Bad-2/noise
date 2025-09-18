#ifdef __x86_64__
#include "arch/x86_64/memory/paging.hpp"

#define ARCH_NAMESPACE_PREFIX x86_64
#endif

namespace memory::arch {
using ARCH_NAMESPACE_PREFIX::convert_flags;
using ARCH_NAMESPACE_PREFIX::fix_page_size;
using ARCH_NAMESPACE_PREFIX::from_type;
using ARCH_NAMESPACE_PREFIX::initialize;
using ARCH_NAMESPACE_PREFIX::max_page_size;
}  // namespace memory::arch

#undef ARCH_NAMESPACE_PREFIX
