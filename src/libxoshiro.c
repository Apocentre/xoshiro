#include "libxoshiro.h"

uint32_t xoshiro_roll(prng *r, uint32_t k) {
  uint32_t x = r->alg(r->state);
  r->cnt++;
  if (k == 0) {
    return x;
  }

  const uint32_t bound = UINT32_MAX - (UINT32_MAX % k);

  while (x >= bound) {
    x = r->alg(r->state);
    r->cnt++;
  }

  return x % k;
}

/*
 * ref: http://prng.di.unimi.it/
 */

#define rotl(x, k) (((x) << (k)) | ((x) >> (64 - (k))))

uint32_t xoshiro256plus_roll(uint64_t *s) {
  const uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;
  s[3] = rotl(s[3], 45);

  return s[0] + s[3];
}

uint32_t xoshiro256plusplus_roll(uint64_t *s) {
  const uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 45);

  return rotl(s[0] + s[3], 23) + s[0];
}

uint32_t xoshiro256starstar_roll(uint64_t *s) {
  const uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 45);

  return rotl(s[1] * 5, 7) * 9;
}

uint32_t xoshiro512plus_roll(uint64_t *s) {
  const uint64_t t = s[1] << 11;

  s[2] ^= s[0];
  s[5] ^= s[1];
  s[1] ^= s[2];
  s[7] ^= s[3];
  s[3] ^= s[4];
  s[4] ^= s[5];
  s[0] ^= s[6];
  s[6] ^= s[7];

  s[6] ^= t;

  s[7] = rotl(s[7], 21);

  return s[0] + s[2];
}

uint32_t xoshiro512plusplus_roll(uint64_t *s) {
  const uint64_t t = s[1] << 11;

  s[2] ^= s[0];
  s[5] ^= s[1];
  s[1] ^= s[2];
  s[7] ^= s[3];
  s[3] ^= s[4];
  s[4] ^= s[5];
  s[0] ^= s[6];
  s[6] ^= s[7];

  s[6] ^= t;

  s[7] = rotl(s[7], 21);

  return rotl(s[0] + s[2], 17) + s[2];
}

uint32_t xoshiro512starstar_roll(uint64_t *s) {
  const uint64_t t = s[1] << 11;

  s[2] ^= s[0];
  s[5] ^= s[1];
  s[1] ^= s[2];
  s[7] ^= s[3];
  s[3] ^= s[4];
  s[4] ^= s[5];
  s[0] ^= s[6];
  s[6] ^= s[7];

  s[6] ^= t;

  s[7] = rotl(s[7], 21);

  return rotl(s[1] * 5, 7) * 9;
}
