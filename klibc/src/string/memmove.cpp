#include "../utils/memory.hpp"

#include <stdint.h>
#include <string.h>

void* memmove(void* dest, const void* src, size_t len) {
  using namespace internal;
  uintptr_t destp = reinterpret_cast<uintptr_t>(dest);
  uintptr_t srcp = reinterpret_cast<uintptr_t>(src);

  if ((destp - srcp) >= len) {
    if (len >= threshold) {
      len -= (-destp) % sizeof(qword_t);

      BYTE_COPY_FORWARD(destp, srcp, len);
      COPY_FORWARD(destp, srcp, len, len);
    }

    BYTE_COPY_FORWARD(destp, srcp, len);
  } else {
    srcp += len;
    destp += len;

    if (len >= threshold) {
      len -= destp % sizeof(qword_t);
      BYTE_COPY_BACKWARD(destp, srcp, destp % sizeof(qword_t));

      COPY_BACKWARD(destp, srcp, len, len);
    }

    BYTE_COPY_BACKWARD(destp, srcp, len);
  }

  return dest;
}