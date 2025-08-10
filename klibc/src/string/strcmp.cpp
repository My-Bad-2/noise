#include <stdint.h>
#include <string.h>

int strcmp(const char* str1, const char* str2) {
  const uint8_t* s1 = reinterpret_cast<const uint8_t*>(str1);
  const uint8_t* s2 = reinterpret_cast<const uint8_t*>(str2);

  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }

  return *s1 - *s2;
}

int strncmp(const char* str1, const char* str2, size_t max_size) {
  const uint8_t* s1 = reinterpret_cast<const uint8_t*>(str1);
  const uint8_t* s2 = reinterpret_cast<const uint8_t*>(str2);

  while (*s1 && (max_size > 0) && (*s1 == *s2)) {
    s1++;
    s2++;
    max_size--;
  }

  return (max_size == 0) ? 0 : *s1 - *s2;
}
