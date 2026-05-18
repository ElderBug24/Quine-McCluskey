#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


inline void exit_error(int);

static inline uint8_t uint8_binary_weight(const uint8_t);

void uint8_format_bits(const uint8_t, char* restrict);
void print_byte_bits(const uint8_t, const char* restrict);
void print_byte_bits_uint16(const uint16_t, const char* restrict);
void format_bits(const void* restrict, size_t, char* restrict);
void print_bits(const void* restrict, size_t, const char* restrict);

typedef struct bigint_t {
  uint8_t* ptr;
  INPUT_SIZE_TYPE count;
} bigint_t;

static inline bigint_t bigint_new(const INPUT_SIZE_TYPE);
static inline bigint_t bigint_from_ptr(void* restrict, INPUT_SIZE_TYPE);
static inline void bigint_destroy(bigint_t);
static inline void bigint_set(bigint_t* restrict, const uint8_t* restrict, const INPUT_SIZE_TYPE);
static inline void bigint_copy_into(const bigint_t, bigint_t);

static inline INPUT_SIZE_TYPE bigint_binary_weight(const bigint_t);
static inline uint8_t bigint_inc(bigint_t);
static inline uint8_t bigint_inc_into(const bigint_t, bigint_t);
static inline uintptr_t bigint_into_ptrdiff(const bigint_t);
static inline bool bigint_get_bit(const bigint_t, INPUT_SIZE_TYPE);

static inline bool bigint_equals(const bigint_t, const bigint_t);
static inline bool bigint_equals_zero(const bigint_t);

void bigint_print_binary(const bigint_t, const char*);

typedef struct byte_diff_result_t {
  uint16_t diff;
  uint8_t count;
} byte_diff_result_t;

typedef struct bigint_diff_t {
  uint16_t* ptr;
  INPUT_SIZE_TYPE count;
} bigint_diff_t;

static inline bigint_diff_t bigint_diff_from_ptr(void* restrict, INPUT_SIZE_TYPE);
static inline void bigint_diff_destroy(bigint_diff_t);

static inline byte_diff_result_t diff_byte(const uint8_t, const uint8_t);
static inline byte_diff_result_t diff_diff(const uint16_t, const uint16_t);

static inline INPUT_SIZE_TYPE bigint_diff_into(const bigint_t, const bigint_t, bigint_diff_t);
static inline INPUT_SIZE_TYPE bigint_diff_diff_into(const bigint_diff_t, const bigint_diff_t, bigint_diff_t);
static inline bool bigint_diff_into_smart(const bigint_t, const bigint_t, bigint_diff_t);
static inline bool bigint_diff_diff_into_smart(const bigint_diff_t, const bigint_diff_t, bigint_diff_t);

static inline uint16_t byte_into_diff(const uint8_t);
static inline void bigint_into_diff(const bigint_t, bigint_diff_t);

static inline bool bigint_diff_equals(const bigint_diff_t, const bigint_diff_t);
static inline uint8_t diff_computational_cost(const uint16_t);
static inline uint8_t diff_count_zeros(const uint16_t);
static inline size_t bigint_diff_computational_cost(const bigint_diff_t);
static inline size_t bigint_diff_count_zeros(const bigint_diff_t);

void diff_format(const uint16_t, char* restrict);
void diff_print(const uint16_t, const char* restrict);
void bigint_diff_print_binary(const bigint_diff_t, const char* restrict);
void bigint_diff_format(const bigint_diff_t, char* restrict);
void bigint_diff_print(const bigint_diff_t, const char* restrict);

#endif // BIGINT_H

#ifdef BIGINT_IMPLEMENTATION

static inline uint8_t uint8_binary_weight(uint8_t byte) {
    byte = (uint8_t)(byte - ((byte >> 1u) & 0x55u));
    byte = (uint8_t)((byte & 0x33u) + ((byte >> 2u) & 0x33u));

    return (uint8_t)((byte + (byte >> 4u)) & 0x0Fu);
}

void format_byte_bits(const uint8_t num, char* restrict output) {
  for (size_t i = 0; i < 8; ++i)
    *output++ = ((num << i) & 0b10000000) == 0 ? '0' : '1';
}

void print_byte_bits(const uint8_t num, const char* restrict end) {
  char* str = malloc(8);
  if (!str) exit_error(2);
  format_byte_bits(num, str);
  printf("%.*s%s", 8, str, end);

  free(str);
}

  void print_byte_bits_uint16(const uint16_t num, const char* restrict end) {
  char* str = malloc(8);
  if (!str) exit_error(2);
  format_byte_bits(*(((uint8_t*) &num) + 1), str);
  printf("%.*s", 8, str);
  format_byte_bits(*(uint8_t*) &num, str);
  printf("%.*s%s", 8, str, end);

  free(str);
}

void format_bits(const void* restrict num, const size_t count, char* restrict output) {
  uint8_t* ptr = (uint8_t*) num + count - 1;

  while ((void*) ptr >= num) {
    format_byte_bits(*ptr--, output);
    output += 8;
  }
}

void print_bits(const void* restrict num, const size_t count, const char* restrict end) {
  char* str = malloc(count * 8);
  if (!str) exit_error(2);
  format_bits(num, count, str);
  printf("%.*s%s", (int) count * 8, str, end);

  free(str);
}

static inline bigint_t bigint_new(const INPUT_SIZE_TYPE size) {
  void* ptr = calloc(size, 1);
  if (!ptr) exit_error(2);
  return (bigint_t) {
    .ptr = ptr,
    .count = size
  };
}

static inline bigint_t bigint_from_ptr(void* restrict allocation, const INPUT_SIZE_TYPE size) {
  return (bigint_t) {
    .ptr = allocation,
    .count = size
  };
}

static inline void bigint_destroy(bigint_t num) {
  free(num.ptr);
}

static inline void bigint_set(bigint_t* restrict num, const uint8_t* restrict ptr, const INPUT_SIZE_TYPE count) {
  assert(count <= num->count);

  for (size_t i = 0; i < count; ++i) {
    num->ptr[i] = ptr[i];
  }
}

static inline void bigint_set_zero(bigint_t num) {
  memset((void*) num.ptr, 0, num.count);
}

static inline void bigint_copy_into(const bigint_t num, bigint_t other) {
  assert(num.count <= other.count);

  memcpy((void*) other.ptr, (void*) num.ptr, num.count);
}

static inline INPUT_SIZE_TYPE bigint_binary_weight(const bigint_t num) {
  INPUT_SIZE_TYPE sum = 0;

  for (size_t i = 0; i < num.count; ++i) {
    sum += uint8_binary_weight(num.ptr[i]);
  }

  return sum;
}

void bigint_print_binary(const bigint_t num, const char* restrict end) {
  char* str = malloc(num.count * 8);
  if (!str) exit_error(2);
  format_bits(num.ptr, num.count, str);
  printf("%.*s%s", (int) num.count * 8, str, end);

  free(str);
}

void bigint_diff_print_binary(const bigint_diff_t diff, const char* restrict end) {
  char* str = malloc(diff.count * 8 * sizeof(uint16_t));
  if (!str) exit_error(2);
  format_bits((uint8_t*) diff.ptr, diff.count * sizeof(uint16_t), str);
  printf("%.*s%s", (int) (diff.count * 8 * sizeof(uint16_t)), str, end);

  free(str);
}

void diff_format(const uint16_t diff, char* restrict output) {
  for (size_t i = 0; i < 8; ++i) {
    uint16_t mask = (diff << 2 * i) & 0b1100000000000000;
    *output++ = (mask & 0b1000000000000000) ? '-' : ((mask == 0) ? '0' : '1');
  }
}

void diff_print(const uint16_t diff, const char* restrict end) {
  char* str = malloc(8);
  if (!str) exit_error(2);
  diff_format(diff, str);
  printf("%.*s%s", (int) 8, str, end);

  free(str);
}

void bigint_diff_format(const bigint_diff_t diff, char* restrict output) {
  uint16_t* ptr = diff.ptr + diff.count - 1;

  while (ptr >= diff.ptr) {
    diff_format(*ptr--, output);
    output += 8;
  }
}

void bigint_diff_print(const bigint_diff_t diff, const char* restrict end) {
  char* str = malloc(diff.count * 8);
  if (!str) exit_error(2);
  bigint_diff_format(diff, str);
  printf("%.*s%s", (int) diff.count * 8, str, end);

  free(str);
}

static inline uint8_t bigint_inc(bigint_t num) {
  uint8_t carry = 1;
  uint8_t* ptr = num.ptr;

  for (size_t i = 0; carry && i < num.count; ++i) {
    carry = (*ptr++ += carry) == 0;
  }

  return carry;

  // return (*(size_t*) num.ptr)++ == 0;
}

static inline uint8_t bigint_inc_into(const bigint_t num, bigint_t other) {
  uint8_t carry = 1;
  uint8_t* ptra = num.ptr;
  uint8_t* ptrb = other.ptr;

  for (size_t i = 0; carry && i < num.count; ++i) {
    carry = (*ptrb = *ptra++ + carry) == 0;
  }

  return carry;
}

static inline uintptr_t bigint_into_ptrdiff(const bigint_t num) {
  assert(num.count <= sizeof(uintptr_t));
  uintptr_t ptr = 0;

  for (size_t i = 0; i < num.count; ++i) {
    ptr += ((uintptr_t) num.ptr[i]) << 8 * i;
  }

  return ptr;
}

static inline bool bigint_get_bit(const bigint_t num, INPUT_SIZE_TYPE bit) {
  div_t r = div(bit, 8);
  uint8_t byte = num.ptr[r.quot];

  return byte & (1 << r.rem);
}

static inline bool bigint_equals(const bigint_t a, const bigint_t b) {
  assert(a.count == b.count);

  for (size_t i = 0; i < a.count; ++i) {
    if (a.ptr[i] != b.ptr[i]) return false;
  }

  return true;
}

static inline bool bigint_equals_zero(const bigint_t num) {
  for (size_t i = 0; i < num.count; ++i) {
    if (num.ptr[i] != 0) return false;
  }

  return true;
}

// for each bit: same -> same, different -> 10
// and counts the amount of different bits
static inline byte_diff_result_t diff_byte(const uint8_t a, const uint8_t b) {
  uint8_t xor = a ^ b;

  uint16_t diff = 0;
  uint8_t count = 0;
  uint16_t dash;

  dash = xor & 0b10000000;
  diff |= dash << 8 | (~xor & a & 0b10000000) << 7;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b01000000;
  diff |= dash << 7 | (~xor & a & 0b01000000) << 6;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00100000;
  diff |= dash << 6 | (~xor & a & 0b00100000) << 5;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00010000;
  diff |= dash << 5 | (~xor & a & 0b00010000) << 4;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00001000;
  diff |= dash << 4 | (~xor & a & 0b00001000) << 3;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00000100;
  diff |= dash << 3 | (~xor & a & 0b00000100) << 2;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00000010;
  diff |= dash << 2 | (~xor & a & 0b00000010) << 1;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));
  dash = xor & 0b00000001;
  diff |= dash << 1 | (~xor & a & 0b00000001) << 0;
  count = (uint8_t)(count + (uint8_t)(dash != 0U));

  return (byte_diff_result_t) {
    .diff = diff,
    .count = count
  };
}

// for each 2bits: same -> same, different -> 10
// and counts the amount of different 2bits
static inline byte_diff_result_t diff_diff(const uint16_t a, const uint16_t b) {
  uint16_t xor = a ^ b;

  uint16_t diff = 0;
  uint16_t count = 0;
  uint16_t dash;

  dash = (xor & 0b1100000000000000) != 0;
  diff |= (((uint16_t)-dash) & 0b1000000000000000) | (((uint16_t)-!dash) & a & 0b1100000000000000);
  count += dash;
  dash = (xor & 0b0011000000000000) != 0;
  diff |= (((uint16_t)-dash) & 0b0010000000000000) | (((uint16_t)-!dash) & a & 0b0011000000000000);
  count += dash;
  dash = (xor & 0b0000110000000000) != 0;
  diff |= (((uint16_t)-dash) & 0b0000100000000000) | (((uint16_t)-!dash) & a & 0b0000110000000000);
  count += dash;
  dash = (xor & 0b0000001100000000) != 0;
  diff |= (((uint16_t)-dash) & 0b0000001000000000) | (((uint16_t)-!dash) & a & 0b0000001100000000);
  count += dash;
  dash = (xor & 0b0000000011000000) != 0;
  diff |= (((uint16_t)-dash) & 0b0000000010000000) | (((uint16_t)-!dash) & a & 0b0000000011000000);
  count += dash;
  dash = (xor & 0b0000000000110000) != 0;
  diff |= (((uint16_t)-dash) & 0b0000000000100000) | (((uint16_t)-!dash) & a & 0b0000000000110000);
  count += dash;
  dash = (xor & 0b0000000000001100) != 0;
  diff |= (((uint16_t)-dash) & 0b0000000000001000) | (((uint16_t)-!dash) & a & 0b0000000000001100);
  count += dash;
  dash = (xor & 0b0000000000000011) != 0;
  diff |= (((uint16_t)-dash) & 0b0000000000000010) | (((uint16_t)-!dash) & a & 0b0000000000000011);
  count += dash;

  return (byte_diff_result_t) {
    .diff = diff,
    .count = (INPUT_SIZE_TYPE) count
  };
}

static inline bigint_diff_t bigint_diff_from_ptr(void* restrict allocation, INPUT_SIZE_TYPE size) {
  return (bigint_diff_t) {
    .ptr = allocation,
    .count = size
  };
}

static inline void bigint_diff_destroy(bigint_diff_t diff) {
  free(diff.ptr);
}

static inline INPUT_SIZE_TYPE bigint_diff_into(const bigint_t a, const bigint_t b, bigint_diff_t diff) {
  assert(a.count == b.count);
  assert(a.count == diff.count);

  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_byte(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;
  }

  return count;
}

// returns true if result is 1 else returns 0 and diff is invalid
static inline bool bigint_diff_into_smart(const bigint_t a, const bigint_t b, bigint_diff_t diff) {
  assert(a.count == b.count);
  assert(a.count == diff.count);

  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_byte(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;

    if (count > 1) return false;
  }

  if (count == 0) return false;
  return true;
}

static inline INPUT_SIZE_TYPE bigint_diff_diff_into(const bigint_diff_t a, const bigint_diff_t b, bigint_diff_t diff) {
  assert(a.count == b.count);
  assert(a.count == diff.count);

  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_diff(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;
  }

  return count;
}

// returns true if result is 1 else returns 0 and diff is invalid
static inline bool bigint_diff_diff_into_smart(const bigint_diff_t a, const bigint_diff_t b, bigint_diff_t diff) {
  assert(a.count == b.count);
  assert(a.count == diff.count);

  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_diff(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;

    if (count > 1) return false;
  }

  if (count == 0) return false;
  return count;
}

static inline uint16_t byte_into_diff(const uint8_t num) {
  uint16_t x = num;

  x = (x | (x << 4)) & 0b0000111100001111;
  x = (x | (x << 2)) & 0b0011001100110011;
  x = (x | (x << 1)) & 0b0101010101010101;

  return x;
}

static inline void bigint_into_diff(const bigint_t num, bigint_diff_t diff) {
  assert(num.count == diff.count);

  for (size_t i = 0; i < num.count; ++i) {
    diff.ptr[i] = byte_into_diff(num.ptr[i]);
  }
}

static inline bool bigint_diff_equals(const bigint_diff_t a, const bigint_diff_t b) {
  assert(a.count == b.count);

  for (size_t i = 0; i < a.count; ++i) {
    if (a.ptr[i] != b.ptr[i]) return false;
  }

  return true;
}

static inline uint8_t diff_computational_cost(uint16_t diff) {
    uint16_t v = (~diff) & 0b1010101010101010;
    uint8_t c = 0;

    c += (v >> 15) & 1;
    c += (v >> 13) & 1;
    c += (v >> 11) & 1;
    c += (v >> 9) & 1;
    c += (v >> 7) & 1;
    c += (v >> 5) & 1;
    c += (v >> 3) & 1;
    c += (v >> 1) & 1;

    return c;
}

static inline uint8_t diff_count_zeros(const uint16_t diff) {
  return (uint8_t) (((diff & 0b1100000000000000) == 0)
                  + ((diff & 0b0011000000000000) == 0)
                  + ((diff & 0b0000110000000000) == 0)
                  + ((diff & 0b0000001100000000) == 0)
                  + ((diff & 0b0000000011000000) == 0)
                  + ((diff & 0b0000000000110000) == 0)
                  + ((diff & 0b0000000000001100) == 0)
                  + ((diff & 0b0000000000000011) == 0));
}

static inline size_t bigint_diff_computational_cost(const bigint_diff_t diff) {
  size_t sum = 0;

  for (size_t i = 0; i < diff.count; ++i) {
    sum += diff_computational_cost(diff.ptr[i]);
  }

  return sum;
}

static inline size_t bigint_diff_count_zeros(const bigint_diff_t diff) {
  size_t sum = 0;

  for (size_t i = 0; i < diff.count; ++i) {
    sum += diff_count_zeros(diff.ptr[i]);
  }

  return sum;
}

#endif // BIGINT_IMPLEMENTATION

