#include <stdint.h>
#include <string.h>

int memcmp(const void* src1, const void* src2, size_t len) {
  const uint8_t* a = (const uint8_t*)(src1);
  const uint8_t* b = (const uint8_t*)(src1);

  while ((len > 0) && (*a == *b)) {
    a++;
    b++;

    len--;
  }

  return (len == 0) ? 0 : *a - *b;
}
