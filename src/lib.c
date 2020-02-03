#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <node_api.h>
#include "libxoshiro.h"


#define NAPI_CALL(env, call)                   \
  do {                                         \
    if ((call) != napi_ok) {                   \
      _napi_call(env);                         \
      return NULL;                             \
    }                                          \
  } while (0)

#define SHUFFLE(type, prng, buf, sz)           \
  do {                                         \
    type *b = buf;                             \
    while (sz > 1) {                           \
    uint32_t r = sz + ~xoshiro_roll(prng, sz); \
      if (r != --sz) {                         \
        b[r] ^= b[sz] ^= b[r] ^= b[sz];        \
      }                                        \
    }                                          \
  } while (0)


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

  prng *prng;
  NAPI_CALL(env, napi_unwrap(env, this, (void **) &prng));

  uint32_t sz;
  bool is_array_like;

  NAPI_CALL(env, napi_is_array(env, array, &is_array_like));
  if (is_array_like) {
    NAPI_CALL(env, napi_get_array_length(env, array, &sz));
    while (sz > 1) {
      uint32_t r = sz + ~xoshiro_roll(prng, sz);
      if (r != --sz) {
        /* swap */
        napi_value x, y;
        napi_get_element(env, array, r, &x);
        napi_get_element(env, array, sz, &y);
        napi_set_element(env, array, r, y);
        napi_set_element(env, array, sz, x);
      }
    }
    return NULL;
  }

  NAPI_CALL(env, napi_is_typedarray(env, array, &is_array_like));
  if (is_array_like) {
    napi_typedarray_type arr_type;
    void *buf;
    {
      size_t len;
      NAPI_CALL(env, napi_get_typedarray_info(env, array, &arr_type, &len, (void **) &buf, NULL, NULL));
      sz = len;
    }

    switch (arr_type) {
      case napi_int8_array:
      case napi_uint8_array:
      case napi_uint8_clamped_array:
        SHUFFLE(uint8_t, prng, buf, sz);
        break;
      case napi_int16_array:
      case napi_uint16_array:
        SHUFFLE(uint16_t, prng, buf, sz);
        break;
      case napi_int32_array:
      case napi_uint32_array:
      case napi_float32_array:
        SHUFFLE(uint32_t, prng, buf, sz);
        break;
      case napi_float64_array:
      case napi_bigint64_array:
      case napi_biguint64_array:
        SHUFFLE(uint64_t, prng, buf, sz);
        break;
      default:
        /* cannot reach here */
        ;
    }
    return NULL;
  }

  napi_throw_type_error(env, "", "An array-like object was expected");
  return NULL;
}


static size_t get_element_size(napi_typedarray_type arr_type) {
  switch (arr_type) {
    case napi_int8_array:
    case napi_uint8_array:
    case napi_uint8_clamped_array:
      return 1;
    case napi_int16_array:
    case napi_uint16_array:
      return 2;
    case napi_int32_array:
    case napi_uint32_array:
    case napi_float32_array:
      return 4;
    case napi_float64_array:
    case napi_bigint64_array:
    case napi_biguint64_array:
      return 8;
    default:
      /* cannot reach here */
      return 0;
  }
}


static const char *get_arraybuffer_info(
  napi_env env,
  napi_value value,
  napi_typedarray_type *arr_type,
  size_t *length,
  void **data
) {
  bool has_data;

  NAPI_CALL(env, napi_is_typedarray(env, value, &has_data));
  if (has_data) {
    NAPI_CALL(env, napi_get_typedarray_info(env, value, arr_type, length, data, NULL, NULL));
    return NULL;
  }

  NAPI_CALL(env, napi_is_arraybuffer(env, value, &has_data));
  if (has_data) {
    NAPI_CALL(env, napi_get_arraybuffer_info(env, value, data, length));
    arr_type && (*arr_type = napi_uint8_array);
    return NULL;
  }

  NAPI_CALL(env, napi_is_dataview(env, value, &has_data));
  if (has_data) {
    NAPI_CALL(env, napi_get_dataview_info(env, value, length, data, NULL, NULL));
    arr_type && (*arr_type = napi_uint8_array);
    return NULL;
  }

  return "A TypedArray, ArrayBuffer or Dataview was expected";
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
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], alg_name, 8, NULL));

  prng_alg_reg *reg = lookup(alg_name);

  if (!reg) {
    napi_throw_type_error(env, NULL, "unknown algorithm");
    return NULL;
  }

  /* seed should be an array-buffer or something using an array-buffer */
  uint64_t *buf;
  size_t length;
  napi_typedarray_type arr_type;
  const char *errmsg = get_arraybuffer_info(env, argv[1], &arr_type, &length, (void **) &buf);
  if (errmsg) {
    napi_throw_type_error(env, NULL, errmsg);
    return NULL;
  }

  /* the length of seed should be greater than or equal to some value according to the algorithm */
  size_t expected_length = reg->sz * sizeof(uint64_t);
  if (length * get_element_size(arr_type) < expected_length) {
    char msg[64];
    sprintf(msg, "at least %zu bytes was expected", expected_length);
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
  memcpy(r->state, buf, expected_length);

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
