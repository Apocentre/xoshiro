#ifndef XOSHIRO_LIBXOSHIRO_H
#define XOSHIRO_LIBXOSHIRO_H

#include <stdint.h>

typedef uint32_t (*prng_alg)(uint64_t *);

typedef struct xoshiro_state {
  prng_alg alg;
  uint64_t bufsiz;
  uint64_t* state;
} xoshiro_state;


/* xoshiro 256+ */
uint32_t xoshiro256plus_roll(uint64_t *s);
/* xoshiro 256++ */
uint32_t xoshiro256plusplus_roll(uint64_t *s);
/* xoshiro 256** */
uint32_t xoshiro256starstar_roll(uint64_t *s);

/* xoshiro 512+ */
uint32_t xoshiro512plus_roll(uint64_t *s);
/* xoshiro 512++ */
uint32_t xoshiro512plusplus_roll(uint64_t *s);
/* xoshiro 512** */
uint32_t xoshiro512starstar_roll(uint64_t *s);

#endif //XOSHIRO_LIBXOSHIRO_H
