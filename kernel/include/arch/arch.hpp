#ifdef __x86_64__
#include "arch/x86_64/arch.hpp"

#define ARCH_NAMESPACE_PREFIX x86_64
#endif  // __x86_64__

namespace arch {
using ARCH_NAMESPACE_PREFIX::halt;
using ARCH_NAMESPACE_PREFIX::initialize;
using ARCH_NAMESPACE_PREFIX::int_status;
using ARCH_NAMESPACE_PREFIX::int_switch;
using ARCH_NAMESPACE_PREFIX::pause;
using ARCH_NAMESPACE_PREFIX::write;
}  // namespace arch
