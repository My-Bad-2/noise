#ifndef BITS_FEATS_H
#define BITS_FEATS_H 1

#ifdef __cplusplus
#define restrict __restrict
#endif

#define CHECK_ALIGNMENT(addr, base) ((addr) % (base) == 0)

#endif  // BITS_FEAT_H
