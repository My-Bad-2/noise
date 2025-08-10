#include <string.h>

char* strcat(char* restrict dest, const char* restrict src) {
  strcpy(dest + strlen(dest), src);
  return dest;
}

char* strncat(char* restrict dest, const char* restrict src, size_t max_size) {
  size_t size = strnlen(src, max_size);
  memcpy(dest + strlen(dest), src, size);
  dest[size] = '\0';

  return dest;
}
