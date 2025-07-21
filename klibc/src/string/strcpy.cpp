#include <string.h>

char* strcpy(char* str1, const char* str2) {
  char* ret = str1;

  while (*str1 != '\0') {
    *str1 = *str2;
    str1++;
    str2++;
  }

  return str1;
}

char* strncpy(char* str1, const char* str2, size_t max_size) {
  size_t len = strnlen(str2, max_size);

  if (len != max_size) {
    memset(str1 + len, '\0', max_size - len);
  }

  return reinterpret_cast<char*>(memcpy((str1), str2, len));
}