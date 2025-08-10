#ifndef STRING_H
#define STRING_H

#include <stddef.h>

#include <bits/feats.h>

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy(void* restrict dest, const void* restrict src, size_t len)
    __attribute__((nonnull(1, 2)));
void* memmove(void* dest, const void* src, size_t len)
    __attribute__((nonnull(1, 2)));
void* memset(void* dest, int ch, size_t len) __attribute__((nonnull(1)));
int memcmp(const void* src1, const void* src2, size_t len) __attribute__((pure))
__attribute__((nonnull(1, 2)));

char* strcpy(char* restrict dest, const char* restrict src)
    __attribute__((nonnull(1, 2)));
char* strncpy(char* restrict dest, const char* restrict src, size_t max_size)
    __attribute__((nonnull(1, 2)));

char* strcat(char* restrict dest, const char* restrict src)
    __attribute__((nonnull(1, 2)));
char* strncat(char* restrict dest, const char* restrict src, size_t max_size)
    __attribute__((nonnull(1, 2)));

int strcmp(const char* str1, const char* str2) __attribute__((pure))
__attribute__((nonnull(1, 2)));
int strncmp(const char* str1, const char* str2, size_t max_size)
    __attribute__((pure)) __attribute__((nonnull(1, 2)));

size_t strlen(const char* str) __attribute__((pure))
__attribute__((nonnull(1)));
size_t strnlen(const char* str, size_t max_len) __attribute__((pure))
__attribute__((nonnull(1)));

char* strchr(const char* str, int ch) __attribute__((pure))
__attribute__((nonnull(1)));

#ifdef __cplusplus
}
#endif

#endif  // STRING_H
