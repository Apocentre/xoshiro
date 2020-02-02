#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <node_api.h>
#include "libxoshiro.h"

#define NAPI_CALL(env, call) do { if ((call) != napi_ok) { _napi_call(env); return NULL; } } while (0)

static void _napi_call(napi_env env) {
  const napi_extended_error_info *error_info = NULL;
  napi_get_last_error_info(env, &error_info);

  bool is_exception_pending;
  napi_is_exception_pending(env, &is_exception_pending);
  if (is_exception_pending) return;

  const char *message = error_info->error_message;
  if (message == NULL) {
    message = "An error occurred without any explicit message";
  }
  napi_throw_error(env, NULL, message);
}


typedef struct prng_alg_reg {
  const char *name;
  size_t sz;
  prng_alg alg;
} prng_alg_reg;

static prng_alg_reg prng_alg_table[] = {
  {"256+",  4, xoshiro256plus_roll},
  {"256++", 4, xoshiro256plusplus_roll},
  {"256**", 4, xoshiro256starstar_roll},
  {"512+",  8, xoshiro512plus_roll},
  {"512++", 8, xoshiro512plusplus_roll},
  {"512**", 8, xoshiro512starstar_roll},
  {NULL,    0, NULL},
};


static prng_alg_reg *lookup(const char *alg) {
  for (prng_alg_reg *c = prng_alg_table; c->name; c++) {
    if (strcmp(alg, c->name) == 0) return c;
  }
  return NULL;
}


static napi_value prng_roll(napi_env env, napi_callback_info cb_info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_value this;
  NAPI_CALL(env, napi_get_cb_info(env, cb_info, &argc, argv, &this, NULL));

  prng *gen;
  NAPI_CALL(env, napi_unwrap(env, this, (void **) &gen));

  uint32_t k = 0;
  if (argc > 0) {
    NAPI_CALL(env, napi_get_value_uint32(env, argv[0], &k));
  }

  uint32_t x = xoshiro_roll(gen, k);
  napi_value out;
  NAPI_CALL(env, napi_create_uint32(env, x, &out));

  return out;
}


static napi_value prng_shuffle(napi_env env, napi_callback_info cb_info) {
  size_t argc = 1;
  napi_value array;
  napi_value this;
  NAPI_CALL(env, napi_get_cb_info(env, cb_info, &argc, &array, &this, NULL));

  uint32_t length;
  NAPI_CALL(env, napi_get_array_length(env, array, &length));

  prng *prng;
  NAPI_CALL(env, napi_unwrap(env, this, (void **) &prng));

  while (length > 1) {
    uint32_t r = length + ~xoshiro_roll(prng, length);
    if (r != --length) {
      /* swap */
      napi_value x, y;
      napi_get_element(env, array, r, &x);
      napi_get_element(env, array, length, &y);
      napi_set_element(env, array, r, y);
      napi_set_element(env, array, length, x);
    }
  }

  return array;
}


static bool check_buffer(napi_env env, napi_value value, void **buf, size_t *len) {
  bool is_buffer;
  NAPI_CALL(env, napi_is_buffer(env, value, &is_buffer));
  if (!is_buffer) return false;
  NAPI_CALL(env, napi_get_buffer_info(env, value, buf, len));
  return true;
}


static bool check_arraybuffer(napi_env env, napi_value value, void **buf, size_t *len) {
  bool is_buffer;
  NAPI_CALL(env, napi_is_arraybuffer(env, value, &is_buffer));
  if (!is_buffer) return false;
  NAPI_CALL(env, napi_get_arraybuffer_info(env, value, buf, len));
  return true;
}


static void state_cleanup(napi_env env, void *data, void *hint) {
#ifdef DEBUG
  fprintf(stderr, "xoshiro free state: %p\n", ((prng *) data)->state);
  fprintf(stderr, "xoshiro free object: %p\n", data);
#endif
  free(((prng *) data)->state);
  free(data);
}

static napi_value create_state(napi_env env, napi_callback_info cb_info) {
  /* 2 parameters required: algorithm and seed */
  size_t argc = 2;
  napi_value argv[2];
  NAPI_CALL(env, napi_get_cb_info(env, cb_info, &argc, argv, NULL, NULL));

  /* algorithm should be in the table */
  char alg_name[8];
  size_t sz;
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], alg_name, 8, &sz));

  prng_alg_reg *reg = lookup(alg_name);

  if (!reg) {
    napi_throw_type_error(env, NULL, "unknown algorithm");
    return NULL;
  }

  /* seed should be a buffer or array-buffer */
  uint64_t *buf;
  size_t length;
  if (!check_buffer(env, argv[1], (void **) &buf, &length) &&
      !check_arraybuffer(env, argv[1], (void **) &buf, &length)) {
    napi_throw_type_error(env, NULL, "A Buffer or ArrayBuffer was expected");
    return NULL;
  }

  /* the length of seed should be greater than or equal to some value according to the algorithm */
  size_t expected_length = reg->sz * sizeof(uint64_t);
  if (length < expected_length) {
    char msg[64];
    sprintf(msg, "insufficient length: at least %lu expected", expected_length);
    napi_throw_error(env, NULL, msg);
    return NULL;
  }

  /* create the internal PRNG state */
  prng *r = malloc(sizeof(prng));
  if (!r) {
    napi_throw_error(env, NULL, "out of memory");
    return NULL;
  }

#ifdef DEBUG
  fprintf(stderr, "xoshiro alloc object: %p\n", r);
#endif

  r->state = malloc(expected_length);
  if (!r->state) {
    napi_throw_error(env, NULL, "out of memory");
    return NULL;
  }

#ifdef DEBUG
  fprintf(stderr, "xoshiro alloc state: %p\n", r->state);
#endif

  r->alg = reg->alg;

  /* ensure consistence of endian on each platform */
  for (size_t i = 0; i < reg->sz; i++) {
    r->state[i] = htonll(buf[i]);
  }

  /* the object to return */
  napi_value out;
  NAPI_CALL(env, napi_create_object(env, &out));
  NAPI_CALL(env, napi_wrap(env, out, r, state_cleanup, NULL, NULL));

  napi_property_descriptor props[] = {
    {"roll",    NULL, prng_roll,    NULL, NULL, NULL, napi_default, NULL},
    {"shuffle", NULL, prng_shuffle, NULL, NULL, NULL, napi_default, NULL},
  };
  NAPI_CALL(env, napi_define_properties(env, out, sizeof(props) / sizeof(napi_property_descriptor), props));
  return out;
}


NAPI_MODULE_INIT(/* env, exports */) {
#ifdef DEBUG
  puts("loading xoshiro...");
#endif

  napi_property_descriptor props[] = {
    {"create", NULL, create_state, NULL, NULL, NULL, napi_enumerable, NULL},
  };
  NAPI_CALL(env, napi_define_properties(env, exports, sizeof(props) / sizeof(napi_property_descriptor), props));
  return exports;
}
