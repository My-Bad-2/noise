#include <string.h>

char* strchr(const char* str, int ch) {
  do {
    if (*str == ch) {
      return const_cast<char*>(str);
    }
  } while (*str++);

  return nullptr;
}
