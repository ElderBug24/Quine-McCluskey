#ifndef BIGINT_H
#define BIGINT_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "truthtable.h"


typedef struct uint8_s {
  uint8_t a : 1;
  uint8_t b : 1;
  uint8_t c : 1;
  uint8_t d : 1;
  uint8_t e : 1;
  uint8_t f : 1;
  uint8_t g : 1;
  uint8_t h : 1;
} uint8_s;

uint8_t uint8_binary_weight(uint8_t);
void uint8_format_bits(uint8_t, char* restrict);
void print_byte_bits(uint8_t, char* restrict);
void print_byte_bits_uint16(uint16_t, char* restrict);
void format_bits(void* restrict, size_t, char* restrict);
void print_bits(void* restrict, size_t, char* restrict);

typedef struct bigint_t {
  uint8_t* ptr;
  INPUT_SIZE_TYPE count;
} bigint_t;

bigint_t bigint_new(INPUT_SIZE_TYPE);
bigint_t bigint_from_allocation(void* restrict, INPUT_SIZE_TYPE);
void bigint_destroy(bigint_t);
void bigint_set(bigint_t* restrict, uint8_t* restrict, INPUT_SIZE_TYPE);
bigint_t bigint_clone(bigint_t);
void bigint_copy_into(bigint_t, bigint_t* restrict);

INPUT_SIZE_TYPE bigint_binary_weight(bigint_t);
uint8_t bigint_inc(bigint_t);

void bigint_print_binary(bigint_t, char*);

bool bigint_equals(bigint_t, bigint_t);
void bigint_bitand(bigint_t, bigint_t, bigint_t);
void bigint_bitor(bigint_t, bigint_t, bigint_t);
void bigint_bitxor(bigint_t, bigint_t, bigint_t);
void bigint_bitnot(bigint_t, bigint_t);

typedef struct byte_diff_result_t {
  uint16_t diff;
  uint8_t count;
} byte_diff_result_t;

typedef struct bigint_diff_t {
  uint16_t* ptr;
  INPUT_SIZE_TYPE count;
} bigint_diff_t;

typedef struct bigint_diff_result_t {
  bigint_diff_t diff;
  INPUT_SIZE_TYPE count;
} bigint_diff_result_t;

void bigint_diff_destroy(bigint_diff_t);

byte_diff_result_t diff_byte(uint8_t, uint8_t);
byte_diff_result_t diff_diff(uint16_t, uint16_t);

bigint_diff_result_t bigint_diff(bigint_t, bigint_t);
bigint_diff_result_t bigint_diff_diff(bigint_diff_t, bigint_diff_t);

void diff_format(uint16_t, char* restrict);
void diff_print(uint16_t, char* restrict);
void bigint_diff_print_binary(bigint_diff_t, char* restrict);
void bigint_diff_format(bigint_diff_t, char* restrict);
void bigint_diff_print(bigint_diff_t, char* restrict);

#endif // BIGINT_H

#ifdef BIGINT_IMPLEMENTATION

uint8_t uint8_binary_weight(uint8_t byte) {
  uint8_s bits = *((uint8_s*) &byte);

  return bits.a
       + bits.b
       + bits.c
       + bits.d
       + bits.e
       + bits.f
       + bits.g
       + bits.h;
}

void format_byte_bits(uint8_t num, char* restrict output) {
  for (size_t i = 0; i < 8; ++i)
    *output++ = ((num << i) & 0b10000000) == 0 ? '0' : '1';
}

void print_byte_bits(uint8_t num, char* restrict end) {
  char* str = malloc(8);
  format_byte_bits(num, str);
  printf("%.*s%s", 8, str, end);

  free(str);
}

void print_byte_bits_uint16(uint16_t num, char* restrict end) {
  char* str = malloc(8);
  format_byte_bits(*(((uint8_t*) &num) + 1), str);
  printf("%.*s", 8, str);
  format_byte_bits(*((uint8_t*) &num), str);
  printf("%.*s%s", 8, str, end);

  free(str);
}

void format_bits(void* restrict num, size_t count, char* restrict output) {
  uint8_t* ptr = (uint8_t*) num + count - 1;

  while ((void*) ptr >= num) {
    format_byte_bits(*ptr--, output);
    output += 8;
  }
}

void print_bits(void* restrict num, size_t count, char* restrict end) {
  char* str = malloc(count * 8);
  format_bits(num, count, str);
  printf("%.*s%s", (int) count * 8, str, end);

  free(str);
}

bigint_t bigint_new(INPUT_SIZE_TYPE size) {
  return (bigint_t) {
    .ptr = calloc(size, 1),
    .count = size
  };
}

bigint_t bigint_from_allocation(void* restrict allocation, INPUT_SIZE_TYPE size) {
  memset(allocation, 0, size);

  return (bigint_t) {
    .ptr = allocation,
    .count = size
  };
}

void bigint_destroy(bigint_t num) {
  free(num.ptr);
}

void bigint_set(bigint_t* restrict num, uint8_t* restrict ptr, INPUT_SIZE_TYPE count) {
  assert(count <= num->count);

  for (size_t i = 0; i < count; ++i) {
    num->ptr[i] = ptr[i];
  }
}

bigint_t bigint_clone(bigint_t num) {
  bigint_t new = bigint_new(num.count);
  bigint_copy_into(num, &new);

  return new;
}

void bigint_copy_into(bigint_t num, bigint_t* restrict other) {
  assert(num.count <= other->count);

  memcpy((void*) other->ptr, (void*) num.ptr, num.count);
}

INPUT_SIZE_TYPE bigint_binary_weight(bigint_t num) {
  size_t sum = 0;

  for (size_t i = 0; i < num.count; ++i) {
    sum += uint8_binary_weight(num.ptr[i]);
  }

  return sum;
}

void bigint_print_binary(bigint_t num, char* restrict end) {
  char* str = malloc(num.count * 8);
  format_bits(num.ptr, num.count, str);
  printf("%.*s%s", (int) num.count * 8, str, end);

  free(str);
}

void bigint_diff_print_binary(bigint_diff_t diff, char* restrict end) {
  char* str = malloc(diff.count * 8 * sizeof(uint16_t));
  format_bits((uint8_t*) diff.ptr, diff.count * sizeof(uint16_t), str);
  printf("%.*s%s", (int) (diff.count * 8 * sizeof(uint16_t)), str, end);

  free(str);
}

void diff_format(uint16_t diff, char* restrict output) {
  for (size_t i = 0; i < 8; ++i) {
    uint16_t mask = (diff << 2 * i) & 0b1100000000000000;
    *output++ = (mask & 0b1000000000000000) ? '-' : ((mask == 0) ? '0' : '1');
  }
}

void diff_print(uint16_t diff, char* restrict end) {
  char* str = malloc(8);
  diff_format(diff, str);
  printf("%.*s%s", (int) 8, str, end);

  free(str);
}

void bigint_diff_format(bigint_diff_t diff, char* restrict output) {
  uint16_t* ptr = diff.ptr + diff.count - 1;

  while (ptr >= diff.ptr) {
    diff_format(*ptr--, output);
    output += 8;
  }
}

void bigint_diff_print(bigint_diff_t diff, char* restrict end) {
  char* str = malloc(diff.count * 8);
  bigint_diff_format(diff, str);
  printf("%.*s%s", (int) diff.count * 8, str, end);

  free(str);
}

uint8_t bigint_inc(bigint_t num) {
  uint8_t carry = 1;
  uint8_t* ptr = num.ptr;

  for (size_t i = 0; carry && i < num.count; ++i) {
    carry = (*ptr++ += carry) == 0;
  }

  return carry;
}

uint8_t bigint_inc_into(bigint_t num, bigint_t other) {
  uint8_t carry = 1;
  uint8_t* ptra = num.ptr;
  uint8_t* ptrb = other.ptr;

  for (size_t i = 0; carry && i < num.count; ++i) {
    carry = (*ptrb = *ptra++ + carry) == 0;
  }

  return carry;
}

bool bigint_equals(bigint_t a, bigint_t b) {
  assert(a.count == b.count);

  for (size_t i = 0; i < a.count; ++i) {
    if (a.ptr[i] != b.ptr[i]) return false;
  }

  return true;
}

void bigint_bitand(bigint_t a, bigint_t b, bigint_t output) {
  assert(a.count == b.count);
  assert(a.count == output.count);

  for (size_t i = 0; i < a.count; ++i) {
    output.ptr[i] = a.ptr[i] & b.ptr[i];
  }
}

void bigint_bitor(bigint_t a, bigint_t b, bigint_t output) {
  assert(a.count == b.count);
  assert(a.count == output.count);

  for (size_t i = 0; i < a.count; ++i) {
    output.ptr[i] = a.ptr[i] | b.ptr[i];
  }
}

void bigint_bitxor(bigint_t a, bigint_t b, bigint_t output) {
  assert(a.count == b.count);
  assert(a.count == output.count);

  for (size_t i = 0; i < a.count; ++i) {
    output.ptr[i] = a.ptr[i] ^ b.ptr[i];
  }
}

void bigint_bitnot(bigint_t a, bigint_t output) {
  assert(a.count == output.count);

  for (size_t i = 0; i < a.count; ++i) {
    output.ptr[i] = ~a.ptr[i];
  }
}

// for each bit: same -> same, different -> 10
// and counts the amount of different bits
byte_diff_result_t diff_byte(uint8_t a, uint8_t b) {
  uint8_t xor = a ^ b;

  uint16_t diff = 0;
  uint8_t count = 0;
  uint16_t dash;

  dash = xor & 0b10000000;
  diff |= dash << 8 | (~xor & a & 0b10000000) << 7;
  count += dash != 0;
  dash = xor & 0b01000000;
  diff |= dash << 7 | (~xor & a & 0b01000000) << 6;
  count += dash != 0;
  dash = xor & 0b00100000;
  diff |= dash << 6 | (~xor & a & 0b00100000) << 5;
  count += dash != 0;
  dash = xor & 0b00010000;
  diff |= dash << 5 | (~xor & a & 0b00010000) << 4;
  count += dash != 0;
  dash = xor & 0b00001000;
  diff |= dash << 4 | (~xor & a & 0b00001000) << 3;
  count += dash != 0;
  dash = xor & 0b00000100;
  diff |= dash << 3 | (~xor & a & 0b00000100) << 2;
  count += dash != 0;
  dash = xor & 0b00000010;
  diff |= dash << 2 | (~xor & a & 0b00000010) << 1;
  count += dash != 0;
  dash = xor & 0b00000001;
  diff |= dash << 1 | (~xor & a & 0b00000001) << 0;
  count += dash != 0;

  return (byte_diff_result_t) {
    .diff = diff,
    .count = count
  };
}

// for each 2bits: same -> same, different -> 10
// and counts the amount of different 2bits
byte_diff_result_t diff_diff(uint16_t a, uint16_t b) {
  uint16_t xor = a ^ b;

  uint16_t diff = 0;
  INPUT_SIZE_TYPE count = 0;
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
    .count = count
  };
}

void bigint_diff_destroy(bigint_diff_t diff) {
  free(diff.ptr);
}

bigint_diff_result_t bigint_diff(bigint_t a, bigint_t b) {
  assert(a.count == b.count);

  bigint_diff_t diff = {
    .ptr = calloc(a.count, sizeof(uint16_t)),
    .count = a.count
  };
  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_byte(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;
  }

  return (bigint_diff_result_t) {
    .diff = diff,
    .count = count
  };
}

bigint_diff_result_t bigint_diff_diff(bigint_diff_t a, bigint_diff_t b) {
  assert(a.count == b.count);

  bigint_diff_t diff = {
    .ptr = calloc(a.count, sizeof(uint16_t)),
    .count = a.count
  };
  INPUT_SIZE_TYPE count = 0;

  for (size_t i = 0; i < a.count; ++i) {
    byte_diff_result_t result = diff_diff(a.ptr[i], b.ptr[i]);
    diff.ptr[i] = result.diff;
    count += result.count;
  }

  return (bigint_diff_result_t) {
    .diff = diff,
    .count = count
  };
}

#endif // BIGINT_IMPLEMENTATION

