#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct da_header_t {
  size_t count;
  size_t capacity;
  void* ptr;
} da_header_t;

da_header_t da_with_capacity(size_t, size_t);
void da_destroy(da_header_t);

void da_reserve_exact(da_header_t*, size_t, size_t);
void da_reserve(da_header_t*, size_t, size_t);

void da_push(da_header_t* restrict, void* restrict, size_t);
void* da_get(da_header_t, size_t, size_t);
void da_swap_remove(da_header_t*, size_t, size_t);

da_header_t group_new(INPUT_SIZE_TYPE, size_t);
void group_destroy(da_header_t);

static inline size_t binomial_coefficient(INPUT_SIZE_TYPE, INPUT_SIZE_TYPE);

#endif // DYNAMICARRAY_H

#ifdef DYNAMICARRAY_IMPLEMENTATION

da_header_t da_with_capacity(size_t capacity, size_t size) {
  void* ptr = malloc(sizeof(da_header_t) + size * capacity);

  da_header_t header = (da_header_t) {
    .count = 0,
    .capacity = capacity,
    .ptr = ptr
  };

  return header;
}

void da_destroy(da_header_t arr) {
  free(arr.ptr);
}

void da_reserve_exact(da_header_t* arr, size_t capacity, size_t size) {
  if (arr->capacity < capacity) {
    arr->ptr = realloc(arr->ptr, size * capacity);
  }
  arr->capacity = capacity;
}

void da_reserve(da_header_t* arr, size_t capacity, size_t size) {
  size_t new_capacity = arr->capacity;// ? arr->capacity : 4;

  while (new_capacity < capacity) new_capacity *= 2;

  da_reserve_exact(arr, new_capacity, size);
}

void da_push(da_header_t* restrict arr, void* restrict element, size_t size) {
  da_reserve(arr, arr->count + 1, size);

  memcpy((uint8_t*) arr->ptr + arr->count * size, element, size);
  arr->count += 1;
}

void* da_get(da_header_t arr, size_t index, size_t size) {
  assert(index < arr.capacity);

  return (void*) ((uint8_t*) arr.ptr + index * size);
}

void da_swap_remove(da_header_t* arr, size_t index, size_t size) {
  assert(index < arr->capacity);

  memcpy(da_get(*arr, index, size), da_get(*arr, arr->count - 1, size), size);
  arr->count -= 1;
}

da_header_t group_new(INPUT_SIZE_TYPE max_bits, size_t size) {
  da_header_t group = da_with_capacity(max_bits + 1, sizeof(da_header_t));

  for (INPUT_SIZE_TYPE i = 0; i < max_bits + 1; ++i) {
    size_t capacity = binomial_coefficient(i, max_bits);
    da_header_t* bucket = (da_header_t*) da_get(group, i, sizeof(da_header_t));

    *bucket = da_with_capacity(capacity, size);
  }
  group.count = max_bits + 1;

  return group;
}

void group_destroy(da_header_t group) {
  for (size_t i = 0; i < group.count; ++i) {
    da_destroy(((da_header_t*) group.ptr)[i]);
  }

  free(group.ptr);
}

static inline size_t binomial_coefficient(INPUT_SIZE_TYPE n, INPUT_SIZE_TYPE k) {
  if (n > k - n) n = k - n;

  size_t result = 1;
  for (size_t i = 1; i <= n; ++i) {
    result = result * (k - n + i) / i;
  }

  return result;
}

#endif // DYNAMICARRAY_IMPLEMENTATION

