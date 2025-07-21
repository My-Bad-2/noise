#include <ctype.h>
#include <stdint.h>

#define ASCII_MASK 0x7Fu
#define LAST_CONTROL_CHAR 31

int isalnum(int symbol) {
  return isalpha(symbol) || isdigit(symbol);
}

int isalpha(int symbol) {
  return isupper(symbol) || islower(symbol);
}

int isascii(int symbol) {
  return symbol >= 0 && symbol <= INT8_MAX;
}

int isblank(int symbol) {
  return symbol == ' ' || symbol == '\t';
}

int iscntrl(int symbol) {
  return (symbol >= 0 && symbol <= LAST_CONTROL_CHAR) || symbol == INT8_MAX;
}

int isdigit(int symbol) {
  return symbol >= '0' && symbol <= '9';
}

int isgraph(int symbol) {
  return symbol > ' ' && symbol <= '~';
}

int islower(int symbol) {
  return symbol >= 'a' && symbol <= 'z';
}

int isprint(int symbol) {
  return symbol >= ' ' && symbol <= '~';
}

int isspace(int symbol) {
  return symbol == ' ' || (symbol >= '\t' && symbol <= '\r');
}

int isupper(int symbol) {
  return symbol >= 'A' && symbol <= 'Z';
}

int isxdigit(int symbol) {
  return isdigit(symbol) || (symbol >= 'A' && symbol <= 'F') ||
         (symbol >= 'a' && symbol <= 'f');
}

int toascii(int symbol) {
  return symbol & ASCII_MASK;
}

int tolower(int symbol) {
  return isupper(symbol) ? symbol + ('a' - 'A') : symbol;
}

int toupper(int symbol) {
  return islower(symbol) ? symbol - ('a' - 'A') : symbol;
}