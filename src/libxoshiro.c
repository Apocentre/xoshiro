#include "libxoshiro.h"

/*
 * ref: http://prng.di.unimi.it/
 */

#define times5(x) ((x) << 2 | (x))
#define times9(x) ((x) << 3 | (x))

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
  uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 45);

  t = s[0] + s[3];

  return rotl(t, 23) + s[0];
}

uint32_t xoshiro256starstar_roll(uint64_t *s) {
  uint64_t t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 45);

  t = times5(s[1]);
  t = rotl(t, 7);
  t = times9(t);

  return t;
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
  uint64_t t = s[1] << 11;

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

  t = s[0] + s[2];

  return rotl(t, 17) + s[2];
}

uint32_t xoshiro512starstar_roll(uint64_t *s) {
  uint64_t t = s[1] << 11;

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

  t = times5(s[1]);
  t = rotl(t, 7);
  t = times9(t);

  return t;
}
