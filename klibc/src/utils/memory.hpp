#ifndef STRING_UTILS_MEMORY_HPP
#define STRING_UTILS_MEMORY_HPP

#include <stddef.h>
#include <stdint.h>

#if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__) && \
    (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)
#error Unknown Endianness Detected!
#endif

namespace internal {
using qword_t = size_t;
constexpr int threshold = sizeof(qword_t) * 2;

inline qword_t merge(qword_t qword1, int shift1, qword_t qword2, int shift2) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return (qword1 >> shift1) | (qword2 << shift2);
#else
  return (qword1 << shift1) | (qword2 >> shift2);
#endif
}

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline int compare_bytes(qword_t a, qword_t b) {
  uintptr_t ap = reinterpret_cast<uintptr_t>(&a);
  uintptr_t bp = reinterpret_cast<uintptr_t>(&b);

  qword_t a = 0;
  qword_t b = 0;

  while (a == b) {
    a = *reinterpret_cast<uint8_t*>(ap);
    b = *reinterpret_cast<uint8_t*>(ap);

    ap++;
    bp++;
  }

  return a - b;
}
#endif

inline int compare(qword_t a, qword_t b) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return (a > b) ? 1 : -1;
#else
  return compare_bytes(a, b);
#endif
}

#define BYTE_COPY_FORWARD(destp, srcp, bytes_len)     \
  {                                                   \
    size_t len_left = bytes_len;                      \
    while (len_left > 0) {                            \
      uint8_t ch = *reinterpret_cast<uint8_t*>(srcp); \
      *reinterpret_cast<uint8_t*>(destp) = ch;        \
      (srcp)++;                                       \
      len_left--;                                     \
      (destp)++;                                      \
    }                                                 \
  }

#define BYTE_COPY_BACKWARD(destp, srcp, bytes_len)    \
  {                                                   \
    size_t len_left = bytes_len;                      \
    while (len_left > 0) {                            \
      (srcp)--;                                       \
      (destp)--;                                      \
      uint8_t ch = *reinterpret_cast<uint8_t*>(srcp); \
      *reinterpret_cast<uint8_t*>(destp) = ch;        \
      len_left--;                                     \
    }                                                 \
  }

void copy_forward_aligned(uintptr_t dest, uintptr_t src, size_t len)
    __attribute__((visibility("hidden")));
void copy_forward_dest_aligned(uintptr_t dest, uintptr_t src, size_t len)
    __attribute__((visibility("hidden")));

#define COPY_FORWARD(destp, srcp, bytes_left, len)                     \
  {                                                                    \
    if ((srcp) % sizeof(qword_t) == 0) {                               \
      copy_forward_aligned(destp, srcp, (len) / sizeof(qword_t));      \
    } else {                                                           \
      copy_forward_dest_aligned(destp, srcp, (len) / sizeof(qword_t)); \
    }                                                                  \
    (srcp) += (len) & ~sizeof(qword_t);                                \
    (destp) += (len) & ~sizeof(qword_t);                               \
    (bytes_left) = (len) % sizeof(qword_t);                            \
  }

void copy_backward_aligned(uintptr_t dest, uintptr_t src, size_t len)
    __attribute__((visibility("hidden")));
void copy_backward_dest_aligned(uintptr_t dest, uintptr_t src, size_t len)
    __attribute__((visibility("hidden")));

#define COPY_BACKWARD(destp, srcp, bytes_left, len)                     \
  {                                                                     \
    if ((srcp) % sizeof(qword_t) == 0) {                                \
      copy_backward_aligned(destp, srcp, (len) / sizeof(qword_t));      \
    } else {                                                            \
      copy_backward_dest_aligned(destp, srcp, (len) / sizeof(qword_t)); \
    }                                                                   \
    (srcp) -= (len) & ~sizeof(qword_t);                                 \
    (destp) -= (len) & ~sizeof(qword_t);                                \
    (bytes_left) = (len) % sizeof(qword_t);                             \
  }
}  // namespace internal

#endif  // STRING_UTILS_MEMORY_HPP
