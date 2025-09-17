#ifndef BITS_FEATS_H
#define BITS_FEATS_H 1

#ifdef __cplusplus
#define restrict __restrict
#endif

#define CHECK_ALIGNMENT(addr, base) ((addr) % (base) == 0)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define _one(x) (1 + ((x) - (x)))

#define bit(x, bit) ((x) & (_one(x) << (bit)))
#define bit_shift(x, bit) (((x) >> (bit)) & 1)
#define bits(x, high, low) \
  ((x) & (((_one(x) << ((high) + 1)) - 1) & ~((_one(x) << (low)) - 1)))
#define bits_shift(x, high, low) \
  (((x) >> (low)) & ((_one(x) << ((high) - (low) + 1)) - 1))
#define bit_set(x, bit) (((x) & (_one(x) << (bit))) ? 1 : 0)

#endif  // BITS_FEAT_H
