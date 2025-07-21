#include <string.h>

size_t strlen(const char* str) {
  const char* s = str;

  while (*s) {
    s++;
  }

  return s - str;
}

size_t strnlen(const char* str, size_t max_len) {
  size_t i = 0;

  while ((i < max_len) && *str) {
    i++;
  }

  return i;
}