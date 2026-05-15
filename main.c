#include <stdint.h>
typedef uint8_t INPUT_SIZE_TYPE; // this type must be able to hold up to inputbits + 1

#define BIGINT_IMPLEMENTATION
#include "bigint.h"
#define DYNAMICARRAY_IMPLEMENTATION
#include "dynamic_array.h"

#include <stdio.h>
#include <inttypes.h>


typedef void (* wrapper_t)(uint8_t*, uint8_t*);

void wrapper(uint8_t* input, uint8_t* output) {
  *output = *input;
}

static inline uint8_t* compute_truthtable(INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, uintptr_t, wrapper_t);
static inline void push_prime_implicant(da_header_t*, uint8_t*, INPUT_SIZE_TYPE, size_t);

static inline void print_truthtable(uint8_t*, INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, uintptr_t, unsigned int);
static inline void print_prime_implicants(da_header_t, INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, unsigned int, size_t);
static inline void print_prime_implicant_chart_essentials(da_header_t, da_header_t, da_header_t, size_t, INPUT_SIZE_TYPE, unsigned int, size_t);
static inline void print_diff_group(da_header_t, size_t, INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, unsigned int, unsigned int);
static inline void print_bigint_group(da_header_t, size_t, INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, unsigned int, unsigned int);
// static inline void print_size(unsigned long long);

int main() {
  constexpr INPUT_SIZE_TYPE   inputbits = 4;
  constexpr INPUT_SIZE_TYPE  outputbits = 1;
  constexpr INPUT_SIZE_TYPE  inputbytes = ( inputbits + 7) / 8;
  constexpr INPUT_SIZE_TYPE outputbytes = (outputbits + 7) / 8;
  constexpr uintptr_t inputmax = (size_t) 1 << inputbits;
  constexpr size_t final_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << inputbits) + sizeof(INPUT_SIZE_TYPE) + sizeof(bool);
  constexpr unsigned int inputcharlen = 2; // inputbits < 10 ^ inputcharlen - 1
  constexpr unsigned int inputncharlen = 2; // 2 ^ inputbits < 10 ^ inputncharlen - 1

  static_assert(inputbits);
  static_assert(outputbits);
  static_assert(inputbytes);
  static_assert(outputbytes);
  static_assert(inputmax);
  static_assert(final_element_size);
  static_assert(inputcharlen);
  static_assert(inputncharlen);

  static_assert(inputbits <= sizeof(uintptr_t) * 8, "This system can not handle so many bits");

  bigint_t input = bigint_new(inputbytes);

  fputs("Generating truth table... ", stdout);
  uint8_t* truthtable = compute_truthtable(inputbytes, outputbytes, inputmax, wrapper);

  void* scratch = malloc(final_element_size);
  da_header_t minterms = da_with_capacity(inputmax, inputbytes);
  da_header_t dontcares = da_with_capacity(1, inputbytes); // not used unless user defines some
  da_header_t prime_implicants = da_with_capacity(inputmax, final_element_size);
  da_header_t final_diffs = da_with_capacity(1, inputbytes * sizeof(uint16_t));
  da_header_t final_minterms = da_with_capacity(1, inputbytes);
  da_header_t final_implicants = da_with_capacity(1, final_element_size);
  if (!truthtable || !scratch || !minterms.ptr || !dontcares.ptr || !prime_implicants.ptr || !final_diffs.ptr || !final_minterms.ptr || !final_implicants.ptr) { puts("\nERROR: Allocation failed"); return 1; }
  puts("done");

  print_truthtable(truthtable, inputbytes, outputbytes, inputmax, inputncharlen);

  for (INPUT_SIZE_TYPE bit = 0; bit < outputbits; ++bit) {
    printf("\n-------------------------------- Bit %*u --------------------------------\n", inputcharlen, bit);
    size_t element_size = inputbytes + sizeof(bool);
    size_t next_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << 1) + sizeof(bool);

    da_header_t group = group_new(inputbits + 1, element_size);

    bigint_set_zero(input);
    // for (uintptr_t i = 0; i < inputmax; ++i, bigint_inc(input)) {
    //   bigint_t output = bigint_from_ptr(truthtable + i * outputbytes, outputbytes);
    //   if (bigint_get_bit(output, bit)) {
    //     da_push(&minterms, input.ptr, inputbytes);
    //   }
    // }

    minterms.count = 0;
    uint8_t temp = 0;
    temp = 4; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 6; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 9; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 10; da_push(&minterms, &temp, sizeof(uint8_t));
    // temp = 11; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 13; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 03; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 14; da_push(&minterms, &temp, sizeof(uint8_t));

    temp = 2; da_push(&dontcares, &temp, sizeof(uint8_t));
    temp = 12; da_push(&dontcares, &temp, sizeof(uint8_t));
    temp = 15; da_push(&dontcares, &temp, sizeof(uint8_t));

    for (size_t i = 0; i < minterms.count; ++i) {
      uint8_t* minterm_ptr = da_get(minterms, i, inputbytes);
      bigint_t minterm = bigint_from_ptr(minterm_ptr, inputbytes);

      INPUT_SIZE_TYPE binary_weight = bigint_binary_weight(minterm);
      assert(binary_weight <= inputbits + 1);
      da_header_t* bucket = da_get(group, binary_weight, sizeof(da_header_t));
      da_push(bucket, minterm.ptr, element_size);
      memset(((uint8_t*) bucket->ptr) + (bucket->count - 1) * element_size + element_size - sizeof(bool), false, sizeof(bool));
    }
    for (size_t i = 0; i < dontcares.count; ++i) {
      uint8_t* dontcare_ptr = da_get(dontcares, i, inputbytes);
      bigint_t dontcare = bigint_from_ptr(dontcare_ptr, inputbytes);

      INPUT_SIZE_TYPE binary_weight = bigint_binary_weight(dontcare);
      da_header_t* bucket = da_get(group, binary_weight, sizeof(da_header_t));
      da_push(bucket, dontcare.ptr, element_size);
      memset(((uint8_t*) bucket->ptr) + (bucket->count - 1) * element_size + element_size - sizeof(bool), false, sizeof(bool));
    }

    INPUT_SIZE_TYPE depth = 0;
    size_t pushed = 0;

    da_header_t next_group = group_new(inputbits + 1, next_element_size);

    bigint_diff_t diff_result = bigint_diff_from_ptr(malloc(inputbytes * sizeof(uint16_t)), inputbytes);

    for (size_t i = 0; i < group.count - 1; ++i) {
      da_header_t* bucket = da_get(group, i, sizeof(da_header_t));
      da_header_t* next_bucket = da_get(group, i + 1, sizeof(da_header_t));

      for (size_t j = 0; j < bucket->count; ++j) {
        uint8_t* num_ptr = da_get(*bucket, j, element_size);
        bigint_t num = bigint_from_ptr(num_ptr, inputbytes);

        for (size_t k = 0; k < next_bucket->count; ++k) {
          uint8_t* num2_ptr = da_get(*next_bucket, k, element_size);
          bigint_t num2 = bigint_from_ptr(num2_ptr, inputbytes);

          INPUT_SIZE_TYPE count = bigint_diff_into(num, num2, diff_result);
          assert(count);

          if (count == 1) {
            *(num_ptr + element_size - sizeof(bool)) = true;
            *(num2_ptr + element_size - sizeof(bool)) = true;
            pushed += 1;
            memcpy(scratch, diff_result.ptr, inputbytes * sizeof(uint16_t));
            memcpy((void*) ((uint8_t*) scratch + inputbytes * sizeof(uint16_t)), num_ptr, inputbytes);
            memcpy((void*) ((uint8_t*) scratch + inputbytes * sizeof(uint16_t) + inputbytes), num2_ptr, inputbytes);

            da_header_t* bucket_next_group = da_get(next_group, i, sizeof(da_header_t));
            da_push(bucket_next_group, scratch, next_element_size);
            memset((uint8_t*) da_get(*bucket_next_group, bucket_next_group->count - 1, next_element_size) + next_element_size - sizeof(bool), false, sizeof(bool));
          }
        }
      }
    }

    print_bigint_group(group, element_size, depth, inputbytes, inputcharlen, inputncharlen);
    for (size_t i = 0; i < group.count; ++i) {
      da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

      for (size_t j = 0; j < bucket->count; ++j) {
        uint8_t* num_ptr = da_get(*bucket, j, element_size);
        bigint_t num = bigint_from_ptr(num_ptr, inputbytes);
        bool used = *(num_ptr + element_size - sizeof(bool));

        if (!used) {
          bigint_into_diff(num, diff_result);
          memset(scratch, 0, final_element_size);
          memcpy(scratch, diff_result.ptr, inputbytes * sizeof(uint16_t));
          memcpy((uint8_t*) scratch + inputbytes * sizeof(uint16_t), num.ptr, inputbytes);

          push_prime_implicant(&prime_implicants, scratch, inputbytes, final_element_size);
        }
      }
    }

    group_destroy(group);

    group = next_group;

    while (pushed) {
      pushed = 0;
      depth += 1;
      assert(depth <= inputbits);

      element_size = inputbytes * sizeof(uint16_t) + (inputbytes << depth) + sizeof(bool);
      next_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << (depth + 1)) + sizeof(bool);
      da_header_t next_group = group_new(inputbits - depth + 1, next_element_size);

      for (size_t i = 0; i < group.count - 1; ++i) {
        da_header_t* bucket = da_get(group, i, sizeof(da_header_t));
        da_header_t* next_bucket = da_get(group, i + 1, sizeof(da_header_t));

        for (size_t j = 0; j < bucket->count; ++j) {
          uint8_t* diff_ptr = da_get(*bucket, j, element_size);
          bigint_diff_t diff = bigint_diff_from_ptr(diff_ptr, inputbytes);

          for (size_t k = 0; k < next_bucket->count; ++k) {
            uint8_t* diff2_ptr = da_get(*next_bucket, k, element_size);
            bigint_diff_t diff2 = bigint_diff_from_ptr(diff2_ptr, inputbytes);

            INPUT_SIZE_TYPE count = bigint_diff_diff_into(diff, diff2, diff_result);
            assert(count);

            if (count == 1) {
              *(diff_ptr + element_size - sizeof(bool)) = true;
              *(diff2_ptr + element_size - sizeof(bool)) = true;
              pushed += 1;

              memcpy(scratch, diff_result.ptr, inputbytes * sizeof(uint16_t));
              memcpy((uint8_t*) scratch + inputbytes * sizeof(uint16_t), (uint8_t*) diff_ptr + inputbytes * sizeof(uint16_t), (size_t) inputbytes << depth);
              memcpy((uint8_t*) scratch + inputbytes * sizeof(uint16_t) + ((size_t) inputbytes << depth), (uint8_t*) diff2_ptr + inputbytes * sizeof(uint16_t), (size_t) inputbytes << depth);

              da_header_t* bucket_next_group = da_get(next_group, i, sizeof(da_header_t));
              da_push(bucket_next_group, scratch, next_element_size);
              memset((uint8_t*) da_get(*bucket_next_group, bucket_next_group->count - 1, next_element_size) + next_element_size - sizeof(bool), false, sizeof(bool));
            }
          }
        }
      }

      print_diff_group(group, element_size, depth, inputbytes, inputcharlen, inputncharlen);
      for (size_t i = 0; i < group.count; ++i) {
        da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

        for (size_t j = 0; j < bucket->count; ++j) {
          uint8_t* diff_ptr = da_get(*bucket, j, element_size);
          bool used = *(diff_ptr + element_size - sizeof(bool));

          if (!used) {
            memset(scratch, 0, final_element_size);
            memcpy(scratch, diff_ptr, element_size);
            INPUT_SIZE_TYPE temp = depth;
            memcpy((uint8_t*) scratch + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE), &temp, sizeof(INPUT_SIZE_TYPE));

            push_prime_implicant(&prime_implicants, scratch, inputbytes, final_element_size);
          }
        }
      }

      group_destroy(group);
      group = next_group;
    }

    da_reserve(&final_diffs, prime_implicants.count / 2, inputbytes * sizeof(uint16_t));
    da_reserve(&final_minterms, minterms.count, inputbytes);
    da_reserve(&final_implicants, prime_implicants.count, final_element_size);

    for (size_t i = 0; i < minterms.count; ++i) {
      bigint_t minterm = bigint_from_ptr(da_get(minterms, i, inputbytes), inputbytes);
      size_t sum = 0;
      size_t last_j = 0;

      for (size_t j = 0; j < prime_implicants.count; ++j) {
        uint8_t* implicant_ptr = da_get(prime_implicants, j, final_element_size);
        uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);

        size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));

        for (size_t n = 0; n < ids_count; ++n) {
          bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);
          if (bigint_equals(id, minterm)) {
            last_j = j;
            sum += 1;
            break;
          }
        }
      }

      assert(sum);

      if (sum == 1) {
        *(bool*) ((uint8_t*) da_get(prime_implicants, last_j, final_element_size) + final_element_size - sizeof(bool)) = true;
      } else {
        da_push(&final_minterms, minterm.ptr, inputbytes);
      }
    }

    for (size_t i = 0; i < prime_implicants.count; ++i) {
      uint8_t* implicant_ptr = da_get(prime_implicants, i, final_element_size);

      if (*(bool*) (implicant_ptr + final_element_size - sizeof(bool))) {
        da_push(&final_diffs, implicant_ptr, inputbytes * sizeof(uint16_t));

        size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));

        uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
        for (size_t n = 0; n < ids_count; ++n) {
          bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);

          for (size_t j = 0; j < final_minterms.count; ++j) {
            bigint_t minterm = bigint_from_ptr(da_get(final_minterms, j, inputbytes), inputbytes);
            if (bigint_equals(id, minterm)) {
              da_swap_remove(&final_minterms, j, inputbytes);
              break;
            }
          }
        }
      } else {
        da_push(&final_implicants, implicant_ptr, final_element_size);
      }
    }

    print_prime_implicant_chart_essentials(prime_implicants, minterms, final_minterms, depth, inputbytes, inputncharlen, final_element_size);

    bigint_diff_destroy(diff_result);

    group_destroy(group);
    minterms.count = 0;
    dontcares.count = 0;
    prime_implicants.count = 0;
    final_diffs.count = 0;
    final_minterms.count = 0;
    final_implicants.count = 0;
  }

  free(truthtable);
  free(scratch);
  da_destroy(minterms);
  da_destroy(dontcares);
  da_destroy(prime_implicants);
  da_destroy(final_diffs);
  da_destroy(final_minterms);
  da_destroy(final_implicants);

  puts("\nExiting without errors");
  return 0;
}

static inline uint8_t* compute_truthtable(INPUT_SIZE_TYPE inputbytes, INPUT_SIZE_TYPE outputbytes, uintptr_t inputmax, wrapper_t wrapper) {
  bigint_t  input = bigint_new(inputbytes);
  bigint_t output = bigint_new(outputbytes);

  uint8_t* truthtable = malloc(inputmax * outputbytes);
  if (!truthtable) return NULL;

  for (uintptr_t i = 0; i < inputmax; ++i) {
    wrapper(input.ptr, output.ptr);
    memcpy((void*) (truthtable + bigint_into_ptrdiff(input) * outputbytes), (void*) output.ptr, output.count);

    bigint_inc(input);
  }

  bigint_destroy(input);
  bigint_destroy(output);

  return truthtable;
}

static inline void push_prime_implicant(da_header_t* prime_implicants, uint8_t* scratch, INPUT_SIZE_TYPE inputbytes, size_t final_element_size) {
  bigint_diff_t diff = bigint_diff_from_ptr(scratch, inputbytes);

  for (size_t i = 0; i < prime_implicants->count; ++i) {
    bigint_diff_t diff2 = bigint_diff_from_ptr(da_get(*prime_implicants, i, final_element_size), inputbytes);

    if (bigint_diff_equals(diff, diff2)) return;
  }

  da_push(prime_implicants, scratch, final_element_size);
}

static inline void print_truthtable(uint8_t* truthtable, INPUT_SIZE_TYPE inputbytes, INPUT_SIZE_TYPE outputbytes, uintptr_t inputmax, unsigned int inputncharlen) {
  size_t c1 = (size_t) inputbytes * 8 + (size_t) inputncharlen + 5;
  size_t c2 = (size_t) inputbytes * 8 + 2;
  printf("\n+");
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  printf("+\n");
  printf("| %-*s", (int) (c1 - 1), "Input");
  printf("| %-*s|\n", (int) (c2 - 1), "Output");
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  printf("+\n");

  for (uintptr_t i = 0; i < inputmax; ++i) {
    printf("| %*" PRIuPTR " = ", inputncharlen, i);
    print_bits(&i, inputbytes, "");
    printf(" | ");
    bigint_t out = bigint_from_ptr(truthtable + i, outputbytes);
    bigint_print_binary(out, " |\n");
  }

  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  putc('\n', stdout);
}

static inline void print_prime_implicants(da_header_t prime_implicants, INPUT_SIZE_TYPE depth, INPUT_SIZE_TYPE inputbytes, unsigned int inputncharlen, size_t final_element_size) {
  if (!prime_implicants.count) return;

  printf("\n+");
  for (unsigned int i = 0; i < 20 + inputncharlen; ++i) putc('-', stdout);
  printf("+\n");
  printf("| Prime implicants: %*zu |\n", inputncharlen, prime_implicants.count);
  size_t max_ids = (size_t) 1 << depth;
  size_t c1 = 2 + (2 + inputncharlen) * max_ids;
  size_t c2 = 2 + (size_t) inputbytes * 8;
  putc('+', stdout);
  for (size_t i = 0; i < c1 - 1; ++i) { if (i == 20 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) { if (i + c1 == 20 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  printf("+\n");

  for (size_t i = 0; i < prime_implicants.count; ++i) {
    uint8_t* implicant_ptr = da_get(prime_implicants, i, final_element_size);
    uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
    bigint_diff_t implicant = bigint_diff_from_ptr(implicant_ptr, inputbytes);

    size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));
    size_t skipped = max_ids - ids_count;

    printf("| ");
    for (size_t n = 0; n < skipped; ++n) {
      for (unsigned int j = 0; j < inputncharlen; ++j) putc(' ', stdout);
      printf("  ");
    }
    for (size_t n = 0; n < max_ids - skipped; ++n) {
      bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);
      printf("%*" PRIuPTR ", ", inputncharlen, bigint_into_ptrdiff(id));
    }
    printf("| ");
    bigint_diff_print(implicant, " |\n");
  }

  putc('+', stdout);
  for (size_t i = 0; i < c1 - 1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = c1; i < c1 + c2; ++i) putc('-', stdout);
  printf("+\n");
}

static inline void print_prime_implicant_chart_essentials(da_header_t prime_implicants, da_header_t minterms, da_header_t final_minterms, size_t depth, INPUT_SIZE_TYPE inputbytes, unsigned int inputncharlen, size_t final_element_size) {
  if (!minterms.count || !prime_implicants.count) return;

  size_t w = minterms.count * ((size_t) inputncharlen + 1);
  size_t h = prime_implicants.count * 2 - 1;
  size_t max_ids = (size_t) 1 << depth;

  size_t c1 = 2 + (2 + inputncharlen) * max_ids;
  size_t c2 = 4 + (size_t) inputbytes * 8;
  size_t c3 = 1 + w;

  char* buffer = malloc(w * h);
  memset(buffer, ' ', w * h);

  for (size_t i = 0; i < minterms.count; ++i) {
    bigint_t minterm = bigint_from_ptr(da_get(minterms, i, inputbytes), inputbytes);
    size_t sum = 0;
    size_t last_j = 0;

    for (size_t j = 0; j < prime_implicants.count; ++j) {
      uint8_t* implicant_ptr = da_get(prime_implicants, j, final_element_size);
      uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);

      size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));

      for (size_t n = 0; n < ids_count; ++n) {
        bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);
        if (bigint_equals(id, minterm)) {
          last_j = j;
          sum += 1;

          size_t x = inputncharlen - 1 + i * (inputncharlen + 1);
          size_t y = j * 2;
          buffer[x + y * w] = 'x';

          break;
        }
      }
    }

    if (sum == 1) {
      size_t x = inputncharlen - 1 + i * (inputncharlen + 1);
      size_t y = last_j * 2;
      buffer[x + y * w] = '#';
    }
  }

  for (size_t i = 0; i < prime_implicants.count; ++i) {
    uint8_t* implicant_ptr = da_get(prime_implicants, i, final_element_size);
    bool essential = *(bool*) (implicant_ptr + final_element_size - sizeof(bool));

    if (essential) {
      uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
      size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));
      size_t minterm_first = 0;
      size_t minterm_next = 0;

      for (size_t j = 0; j < minterms.count; ++j) {
        bigint_t minterm = bigint_from_ptr(da_get(minterms, j, inputbytes), inputbytes);

        for (size_t n = 0; n < ids_count; ++n) {
          bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);

          if (bigint_equals(id, minterm)) {
            size_t implicant_first = 0;
            size_t implicant_next = 0;

            for (size_t k = 0; k < prime_implicants.count; ++k) {
              uint8_t* implicant_ptr = da_get(prime_implicants, k, final_element_size);
              uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
              size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));

              for (size_t n = 0; n < ids_count; ++n) {
                bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);

                if (bigint_equals(id, minterm)) {
                  if (implicant_first) {
                    implicant_next = k;
                    size_t x = inputncharlen - 1 + j * (inputncharlen + 1);

                    for (size_t y = implicant_first * 2 - 1; y < implicant_next * 2; ++y) {
                      if (buffer[x + y * w] != '-') buffer[x + y * w] = '|';
                    }

                    implicant_first = k + 1;
                  } else {
                    implicant_first = k + 1;
                  }

                  break;
                }
              }
            }

            if (minterm_first) {
              minterm_next = j;

              for (size_t x = (1 + inputncharlen) * (minterm_first - 1) + inputncharlen; x < (1 + inputncharlen) * minterm_next + inputncharlen - 1; ++x) {
                buffer[x + i * 2 * w] = '-';
              }
            }
            minterm_first = j + 1;

            break;
          }
        }
      }
    }
  }

  putc('\n', stdout);
  for (size_t i = 0; i < c1 + c2 + 1; ++i) putc(' ', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) putc('-', stdout);
  printf("+\n");
  for (size_t i = 0; i < c1 + c2 + 1; ++i) putc(' ', stdout);
  printf("| ");
  for (size_t i = 0; i < minterms.count; ++i) printf("%*" PRIuPTR " ", inputncharlen, bigint_into_ptrdiff(bigint_from_ptr(da_get(minterms, i, inputbytes), inputbytes)));
  printf("|\n");
  for (size_t i = 0; i < c1 + c2 + 1; ++i) putc(' ', stdout);
  printf("| ");
  for (size_t i = 0; i < minterms.count; ++i) {
    bigint_t minterm = bigint_from_ptr(da_get(minterms, i, inputbytes), inputbytes);

    bool used = false;
    for (size_t j = 0; j < final_minterms.count; ++j) {
      bigint_t final_minterm = bigint_from_ptr(da_get(final_minterms, j, inputbytes), inputbytes);

      if (bigint_equals(minterm, final_minterm)) {
        used = true;

        break;
      }
    }

    if (used) {
      for (size_t k = 0; k < inputncharlen + 1; ++k) putc(' ', stdout);
    } else {
      for (size_t k = 0; k < inputncharlen - 1; ++k) putc(' ', stdout);
      putc('*', stdout);
      putc(' ', stdout);
    }
  }
  printf("|\n");
  putc('+', stdout);
  for (size_t i = 0; i < c1 - 1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) putc('-', stdout);
  printf("+\n");

  for (size_t i = 0; i < h; ++i) {
    printf("| ");
    if (i & 1) {
      for (size_t j = 0; j < c1 - 2; ++j) putc(' ', stdout);
      putc('|', stdout);
      for (size_t j = 0; j < c2; ++j) putc(' ', stdout);
    } else {
      uint8_t* implicant_ptr = da_get(prime_implicants, i / 2, final_element_size);
      uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
      bigint_diff_t implicant = bigint_diff_from_ptr(implicant_ptr, inputbytes);
      bool essential = *(bool*) (implicant_ptr + final_element_size - sizeof(bool));

      size_t ids_count = (size_t) 1 << *(INPUT_SIZE_TYPE*) (implicant_ptr + final_element_size - sizeof(bool) - sizeof(INPUT_SIZE_TYPE));
      size_t skipped = max_ids - ids_count;

      for (size_t n = 0; n < skipped; ++n) {
        for (unsigned int j = 0; j < inputncharlen; ++j) putc(' ', stdout);
        printf("  ");
      }
      for (size_t n = 0; n < max_ids - skipped; ++n) {
        bigint_t id = bigint_from_ptr(ids_ptr + n * inputbytes, inputbytes);
        printf("%*" PRIuPTR ", ", inputncharlen, bigint_into_ptrdiff(id));
      }

      printf("| ");
      bigint_diff_print(implicant, " ");

      if (essential) printf("* ");
      else printf("  ");
    }

    printf("| ");
    printf("%.*s", (unsigned int) w, buffer + i * w);
    printf("|\n");
  }

  putc('+', stdout);
  for (size_t i = 0; i < c1 - 1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) putc('-', stdout);
  printf("+\n");

  free(buffer);
}

static inline void print_diff_group(da_header_t group, size_t element_size, INPUT_SIZE_TYPE depth, INPUT_SIZE_TYPE inputbytes, unsigned int inputcharlen, unsigned int inputncharlen) {
  printf("\n+");
  for (unsigned int i = 0; i < inputcharlen; ++i) putc('-', stdout);
  printf("---------+\n| Depth: %*u |\n", inputcharlen, depth);
  size_t c1 = 2 + inputcharlen;
  size_t c2 = 2 + (2 + inputncharlen) * ((size_t) 1 << depth) - 1;
  size_t c3 = (size_t) inputbytes * 8 + 2 + 2;
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) { if (i == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) { if (i + c1 + 1 == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) { if (i + c1 + c2 + 2 == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  printf("+\n");
  for (size_t i = 0; i < group.count; ++i) {
    da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

    for (size_t j = 0; j < bucket->count; ++j) {
      if (!j) printf("| %*zu | ", inputcharlen, i);
      else {
        printf("| ");
        for (unsigned int i = 0; i < inputcharlen; ++i) putc(' ', stdout);
        printf(" | ");
      }

      uint8_t* diff_ptr = da_get(*bucket, j, element_size);
      bigint_diff_t diff = bigint_diff_from_ptr(diff_ptr, inputbytes);
      bool used = *(diff_ptr + element_size - sizeof(bool));

      for (size_t n = 0; n < (size_t) 1 << depth; ++n)
        printf("%*" PRIuPTR ", ", inputncharlen, bigint_into_ptrdiff(bigint_from_ptr(diff_ptr + inputbytes * sizeof(uint16_t) + n * inputbytes, inputbytes)));
      printf("| ");

      if (used) bigint_diff_print(diff, " / |\n");
      else bigint_diff_print(diff, "   |\n");
    }
  }
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) putc('-', stdout);
  printf("+\n");
}

static inline void print_bigint_group(da_header_t group, size_t element_size, INPUT_SIZE_TYPE depth, INPUT_SIZE_TYPE inputbytes, unsigned int inputcharlen, unsigned int inputncharlen) {
  printf("\n+");
  for (unsigned int i = 0; i < inputcharlen; ++i) putc('-', stdout);
  printf("---------+\n| Depth: %*u |\n", inputcharlen, depth);
  size_t c1 = 2 + inputcharlen;
  size_t c2 = 2 + inputncharlen;
  size_t c3 = (size_t) inputbytes * 8 + 2 + 2;
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) { if (i == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) { if (i + c1 + 1 == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) { if (i + c1 + c2 + 2 == 9 + (size_t) inputncharlen) putc('+', stdout); else putc('-', stdout); }
  printf("+\n");
  for (size_t i = 0; i < group.count; ++i) {
    da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

    for (size_t j = 0; j < bucket->count; ++j) {
      if (!j) printf("| %*zu | ", inputcharlen, i);
      else {
        printf("| ");
        for (unsigned int i = 0; i < inputcharlen; ++i) putc(' ', stdout);
        printf(" | ");
      }

      uint8_t* num_ptr = da_get(*bucket, j, element_size);
      bigint_t num = bigint_from_ptr(num_ptr, inputbytes);
      bool used = *(num_ptr + element_size - sizeof(bool));

      printf("%*" PRIuPTR " | ", inputncharlen, bigint_into_ptrdiff(num));

      if (used) bigint_print_binary(num, " / |\n");
      else bigint_print_binary(num, "   |\n");
    }
  }
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c3; ++i) putc('-', stdout);
  printf("+\n");
}

// static inline void print_size(unsigned long long bytes) {
//   const char* units[] = { "bytes", "Kb", "Mb", "Gb", "Tb", "Pb" };
//   double size = (double)bytes;
//   int unit = 0;
//   while (size >= 1024.0 && unit < 5) { size /= 1024.0; unit++; }
//   if (unit == 0) { printf("%llu %s", bytes, units[unit]); return; }
//
//   printf("%.0f %s", size, units[unit]);
// }

