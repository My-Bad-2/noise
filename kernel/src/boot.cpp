#include "boot.hpp"

namespace boot {
// clang-format off
__attribute__((used)) __attribute__((section(".requests")))
volatile LIMINE_BASE_REVISION(3);

__attribute__((used)) __attribute__((section(".requests")))
volatile limine_memmap_request memmap_request = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0,
  .response = nullptr,
};

__attribute__((used)) __attribute__((section(".requests")))
volatile limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0,
  .response = nullptr,
};

__attribute__((used)) __attribute__((section(".requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used)) __attribute__((section(".requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;
// clang-format on
}  // namespace boot
