#include "libxoshiro.h"
#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int error_occurred(napi_env env) {
  const napi_extended_error_info *error_info = NULL;
  napi_get_last_error_info(env, &error_info);

  if (error_info->error_code == napi_ok)
    return 0;

  bool is_exception_pending;
  napi_is_exception_pending(env, &is_exception_pending);
  if (is_exception_pending)
    return 1;

  napi_throw_error(env, NULL, error_info->error_message);
  return 1;
}

#define ASSERT(env)          \
  if (error_occurred(env)) \
  return NULL


typedef struct prng_alg_reg {
  const char *name;
  size_t sz;
  prng_alg alg;
} prng_alg_reg;

static prng_alg_reg prng_alg_table[] = {
  {"256+", 4, xoshiro256plus_roll},
  {"256++", 4, xoshiro256plusplus_roll},
  {"256**", 4, xoshiro256starstar_roll},
  {"512+", 8, xoshiro512plus_roll},
  {"512++", 8, xoshiro512plusplus_roll},
  {"512**", 8, xoshiro512starstar_roll},
  {NULL, 0, NULL},
};


static prng_alg_reg *lookup(const char *alg) {
  for (prng_alg_reg *c = prng_alg_table; c->name; c++) {
    if (strcmp(alg, c->name) == 0)
      return c;
  }
  return NULL;
}


static napi_value prng_roll(napi_env env, napi_callback_info cb_info) {
  napi_value this;
  napi_get_cb_info(env, cb_info, NULL, NULL, &this, NULL);
  ASSERT(env);

  xoshiro_state *gen;
  napi_unwrap(env, this, (void **)&gen);
  ASSERT(env);

  uint64_t x = gen->alg(gen->state);
  napi_value out;
  napi_create_bigint_uint64(env, x, &out);
  ASSERT(env);

  return out;
}


static const void *get_arraybuffer_info(
  napi_env env,
  napi_value value,
  void **data,
  size_t *length) {
  bool has_data;
  size_t offset = 0;

  napi_is_typedarray(env, value, &has_data);
  ASSERT(env);
  if (has_data) {
    napi_get_typedarray_info(env, value, NULL, NULL, NULL, &value, &offset);
    ASSERT(env);
  } else {
    napi_is_dataview(env, value, &has_data);
    ASSERT(env);
    if (has_data) {
      napi_get_dataview_info(env, value, NULL, NULL, &value, &offset);
      ASSERT(env);
    }
  }

  napi_is_arraybuffer(env, value, &has_data);
  ASSERT(env);
  if (has_data) {
    napi_get_arraybuffer_info(env, value, data, length);
    ASSERT(env);
    *data = *(uint8_t **)data + offset;
    *length -= offset;
    return NULL;
  }

  napi_throw_error(env, NULL, "A TypedArray, ArrayBuffer or Dataview was expected");
  return NULL;
}


static void state_cleanup(napi_env env, void *data, void *hint) {
#ifdef DEBUG
  fprintf(stderr, "xoshiro free state: %p\n", ((xoshiro_state *)data)->state);
  fprintf(stderr, "xoshiro free object: %p\n", data);
#endif
  free(((xoshiro_state *)data)->state);
  free(data);
}


static napi_value prng_set_state(napi_env env, napi_callback_info cb_info) {
  napi_value this;
  size_t argc = 1;
  napi_value argv;
  napi_get_cb_info(env, cb_info, &argc, &argv, &this, NULL);
  ASSERT(env);

  xoshiro_state *gen;
  napi_unwrap(env, this, (void **)&gen);
  ASSERT(env);

  /* seed should be an array-buffer or something using an array-buffer */
  uint64_t *buf;
  size_t length;
  get_arraybuffer_info(env, argv, (void **)&buf, &length);

  const size_t expected_length = gen->bufsiz << 3;
  if (length < expected_length) {
    char msg[64];
    sprintf(msg, "at least %zd bytes was expected", expected_length);
    napi_throw_error(env, NULL, msg);
    return NULL;
  }

  memcpy(gen->state, buf, expected_length);
  return NULL;
}


static napi_value prng_get_state(napi_env env, napi_callback_info cb_info) {
  napi_value this;
  napi_get_cb_info(env, cb_info, NULL, NULL, &this, NULL);
  ASSERT(env);

  xoshiro_state *gen;
  napi_unwrap(env, this, (void **)&gen);
  ASSERT(env);

  napi_value out;
  const size_t length = gen->bufsiz << 3;
  char *data;
  napi_create_buffer(env, length, (void **)&data, &out);
  ASSERT(env);

  memcpy(data, gen->state, length);

  return out;
}


static napi_value create_state(napi_env env, napi_callback_info cb_info) {
  /* 2 parameters required: algorithm and seed */
  size_t argc = 2;
  napi_value argv[2];
  napi_get_cb_info(env, cb_info, &argc, argv, NULL, NULL);
  ASSERT(env);

  /* algorithm should be in the table */
  char alg_name[8];
  napi_get_value_string_utf8(env, argv[0], alg_name, 8, NULL);
  ASSERT(env);

  prng_alg_reg *reg = lookup(alg_name);

  if (!reg) {
    napi_throw_type_error(env, NULL, "unknown algorithm");
    return NULL;
  }

  uint8_t *data;
  size_t length;
  get_arraybuffer_info(env, argv[1], (void **)&data, &length);
  ASSERT(env);

  const size_t expected_length = reg->sz << 3;

  if (length < expected_length) {
    char msg[64];
    sprintf(msg, "at least %zd bytes was expected", expected_length);
    napi_throw_error(env, NULL, msg);
    return NULL;
  }

  /* create the internal PRNG state */
  xoshiro_state *r = malloc(sizeof(xoshiro_state));
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
  r->bufsiz = expected_length >> 3;

  // @patch for UR compatibility
  for(int i = 0; i < 4; i++) {
      int o = i * 8;
      uint64_t v = 0;
      for(int n = 0; n < 8; n++) {
          v <<= 8;
          v |= data[o + n];
      }
      r->state[i] = v;
  }

  /* the object to return */
  napi_value out = NULL;
  napi_create_object(env, &out);
  ASSERT(env);
  napi_wrap(env, out, r, state_cleanup, NULL, NULL);
  ASSERT(env);

  napi_property_descriptor props[] = {
    {"roll",    NULL, prng_roll,    NULL,   NULL,            NULL, napi_default, NULL},
    {"state",    NULL, NULL, prng_get_state, prng_set_state, NULL, napi_default, NULL},
  };
  napi_define_properties(env, out, sizeof(props) / sizeof(napi_property_descriptor), props);
  ASSERT(env);

  return out;
}


NAPI_MODULE_INIT(/* env, exports */) {
#ifdef DEBUG
  puts("loading xoshiro...");
#endif

  napi_property_descriptor props[] = {
    {"create", NULL, create_state, NULL, NULL, NULL, napi_enumerable, NULL},
  };
  napi_define_properties(env, exports, sizeof(props) / sizeof(napi_property_descriptor), props);
  ASSERT(env);
  return exports;
}
