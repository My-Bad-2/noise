#include "../utils/memory.hpp"

#include <stdint.h>
#include <string.h>

void* memset(void* dest, int ch, size_t len) {
  using namespace internal;

  uintptr_t destp = reinterpret_cast<uintptr_t>(dest);

  if (len >= 8) {
    size_t word = static_cast<uint8_t>(ch);
    word |= word << 8;
    word |= word << 16;

    if (sizeof(qword_t) > sizeof(uint32_t)) {
      word |= (word << 16) << 16;
    }

    while (reinterpret_cast<uintptr_t>(destp) % sizeof(size_t) != 0) {
      *reinterpret_cast<uint8_t*>(destp) = ch;
      destp++;
      len--;
    }

    size_t xlen = len / (sizeof(size_t) * 8);

    // Write 8 `size_t` iteration until less than 64 bytes remain.
    while (xlen > 0) {
      reinterpret_cast<qword_t*>(destp)[0] = word;
      reinterpret_cast<qword_t*>(destp)[1] = word;
      reinterpret_cast<qword_t*>(destp)[2] = word;
      reinterpret_cast<qword_t*>(destp)[3] = word;
      reinterpret_cast<qword_t*>(destp)[4] = word;
      reinterpret_cast<qword_t*>(destp)[5] = word;
      reinterpret_cast<qword_t*>(destp)[6] = word;
      reinterpret_cast<qword_t*>(destp)[7] = word;

      destp += sizeof(size_t) * 8;
      xlen--;
    }

    len %= sizeof(size_t) * 8;
    xlen = len / sizeof(size_t);

    while (xlen > 0) {
      *reinterpret_cast<qword_t*>(destp) = word;
      destp += sizeof(size_t);
      xlen--;
    }

    len %= sizeof(size_t);
  }

  // Write the last few bytes
  while (len > 0) {
    *reinterpret_cast<uint8_t*>(destp) = ch;

    destp++;
    len--;
  }

  return dest;
}