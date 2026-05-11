#define INPUT_SIZE_TYPE uint8_t // this type must be able to hold up to inputbits + 1

#define BIGINT_IMPLEMENTATION
#include "bigint.h"
#define DYNAMICARRAY_IMPLEMENTATION
#include "dynamic_array.h"

#include <stdio.h>


void wrapper(uint8_t* input, uint8_t* output) {
  // *output = *input << 2 ^ *input;
  *output = *input;
}

void print_truthtable(uint8_t*, ptrdiff_t, INPUT_SIZE_TYPE, INPUT_SIZE_TYPE, int);
// void print_size(unsigned long long);

int main() {
  constexpr INPUT_SIZE_TYPE   inputbits = 4;
  constexpr INPUT_SIZE_TYPE  outputbits = 1;
  constexpr INPUT_SIZE_TYPE  inputbytes = ( inputbits + 7) / 8;
  constexpr INPUT_SIZE_TYPE outputbytes = (outputbits + 7) / 8;
  constexpr ptrdiff_t inputmax = (size_t) 1 << inputbits;
  constexpr size_t final_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << inputbits);
  constexpr int inputcharlen = 2; // inputbits < 10 ^ inputcharlen - 1
  constexpr int inputncharlen = 2; // 2 ^ inputbits < 10 ^ inputncharlen - 1

  static_assert(inputbits <= sizeof(ptrdiff_t) * 8 - 1, "This system can not handle so many bits");

  bigint_t  input = bigint_new(inputbytes);
  bigint_t output = bigint_new(outputbytes);

  uint8_t* truthtable = malloc(inputmax * outputbytes);
  void* scratch = malloc(final_element_size);
  if (!truthtable || !scratch) { puts("ERROR: Allocation failed"); return 1; }

  fputs("Generating truth table... ", stdout);
  for (ptrdiff_t i = 0; i < inputmax; ++i) {
    wrapper(input.ptr, output.ptr);
    memcpy((void*) (truthtable + bigint_into_ptrdiff(input) * outputbytes), (void*) output.ptr, output.count);

    bigint_inc(input);
  }
  puts("done");

  bigint_destroy(output);

  print_truthtable(truthtable, inputmax, inputbytes, outputbytes, inputncharlen);

  da_header_t minterms = da_with_capacity(inputmax, inputbytes);
  da_header_t dontcares = da_with_capacity(1, inputbytes); // not used unless user defines some
  da_header_t finals = da_with_capacity(inputmax, final_element_size);
  if (!minterms.ptr || !dontcares.ptr || !finals.ptr) { puts("ERROR: Allocation failed"); return 1; }

  for (INPUT_SIZE_TYPE bit = 0; bit < outputbits; ++bit) {
    printf("\n-------------------------------- Bit %*u --------------------------------\n", inputcharlen, bit);
    size_t element_size = inputbytes + sizeof(bool);
    size_t next_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << 1) + sizeof(bool);

    da_header_t group = group_new(inputbits, element_size);

    bigint_set_zero(input);
    for (ptrdiff_t i = 0; i < inputmax; ++i, bigint_inc(input)) {
      bigint_t output = bigint_from_allocation(truthtable + i * outputbytes, outputbytes);
      if (bigint_get_bit(output, bit)) {
        da_push(&minterms, input.ptr, inputbytes);
      }
    }

    minterms.count = 0;
    uint8_t temp = 0;
    temp = 4; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 6; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 9; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 10; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 11; da_push(&minterms, &temp, sizeof(uint8_t));
    temp = 13; da_push(&minterms, &temp, sizeof(uint8_t));

    temp = 2; da_push(&dontcares, &temp, sizeof(uint8_t));
    temp = 12; da_push(&dontcares, &temp, sizeof(uint8_t));
    temp = 15; da_push(&dontcares, &temp, sizeof(uint8_t));

    for (size_t i = 0; i < minterms.count; ++i) {
      uint8_t* minterm_ptr = da_get(minterms, i, inputbytes);
      bigint_t minterm = bigint_from_allocation(minterm_ptr, inputbytes);

      INPUT_SIZE_TYPE binary_weight = bigint_binary_weight(minterm);
      da_header_t* bucket = da_get(group, binary_weight, sizeof(da_header_t));
      da_push(bucket, minterm.ptr, element_size);
      memset(((uint8_t*) bucket->ptr) + (bucket->count - 1) * element_size + element_size - sizeof(bool), false, sizeof(bool));
    }
    for (size_t i = 0; i < dontcares.count; ++i) {
      uint8_t* dontcare_ptr = da_get(dontcares, i, inputbytes);
      bigint_t dontcare = bigint_from_allocation(dontcare_ptr, inputbytes);

      INPUT_SIZE_TYPE binary_weight = bigint_binary_weight(dontcare);
      da_header_t* bucket = da_get(group, binary_weight, sizeof(da_header_t));
      da_push(bucket, dontcare.ptr, element_size);
      memset(((uint8_t*) bucket->ptr) + (bucket->count - 1) * element_size + element_size - sizeof(bool), false, sizeof(bool));
    }

    size_t depth = 0;
    size_t pushed = 0;

    da_header_t next_group = group_new(inputbits - 1, next_element_size);

    bigint_diff_t diff_result = bigint_diff_from_allocation(malloc(inputbytes * sizeof(uint16_t)), inputbytes);

    for (size_t i = 0; i < group.count - 1; ++i) {
      da_header_t* bucket = da_get(group, i, sizeof(da_header_t));
      da_header_t* next_bucket = da_get(group, i + 1, sizeof(da_header_t));

      for (size_t j = 0; j < bucket->count; ++j) {
        uint8_t* num_ptr = da_get(*bucket, j, element_size);
        bigint_t num = bigint_from_allocation(num_ptr, inputbytes);

        for (size_t k = 0; k < next_bucket->count; ++k) {
          uint8_t* num2_ptr = da_get(*next_bucket, k, element_size);
          bigint_t num2 = bigint_from_allocation(num2_ptr, inputbytes);

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

    printf("\n+");
    for (size_t i = 0; i < inputcharlen; ++i) putc('-', stdout);
    printf("---------+\n| depth: %*zu |\n", inputcharlen, depth);
    size_t c1 = 2 + inputcharlen;
    size_t c2 = 2 + inputncharlen;
    size_t c3 = inputbytes * 8 + 2 + 2;
    putc('+', stdout);
    for (size_t i = 0; i < c1; ++i) putc('-', stdout);
    putc('+', stdout);
    for (size_t i = 0; i < c2; ++i) putc('-', stdout);
    putc('+', stdout);
    for (size_t i = 0; i < c3; ++i) putc('-', stdout);
    printf("+\n");
    for (size_t i = 0; i < group.count; ++i) {
      da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

      for (size_t j = 0; j < bucket->count; ++j) {

        if (!j) printf("| %*zu | ", inputcharlen, i);
        else {
          printf("| ");
          for (size_t i = 0; i < inputcharlen; ++i) putc(' ', stdout);
          printf(" | ");
        }

        uint8_t* num_ptr = da_get(*bucket, j, element_size);
        bigint_t num = bigint_from_allocation(num_ptr, inputbytes);
        bool used = *(num_ptr + element_size - sizeof(bool));

        printf("%*td | ", inputncharlen, bigint_into_ptrdiff(num));

        if (used) {
          bigint_print_binary(num, " / |\n");
        } else {
          bigint_print_binary(num, "   |\n");

          bigint_into_diff(num, diff_result);
          memset(scratch, 0, final_element_size);
          memcpy(scratch, diff_result.ptr, inputbytes * sizeof(uint16_t));
          memcpy((uint8_t*) scratch + inputbytes * sizeof(uint16_t), num.ptr, inputbytes);

          da_push(&finals, scratch, final_element_size);
        }
      }
    }
    putc('+', stdout);
    for (size_t i = 0; i < c1; ++i) putc('-', stdout);
    putc('+', stdout);
    for (size_t i = 0; i < c2; ++i) putc('-', stdout);
    putc('+', stdout);
    for (size_t i = 0; i < c3; ++i) putc('-', stdout);
    printf("+\n");

    group_destroy(group);

    group = next_group;

    while (pushed) {
      pushed = 0;
      depth += 1;
      assert(depth <= inputbits);

      element_size = inputbytes * sizeof(uint16_t) + (inputbytes << depth) + sizeof(bool);
      next_element_size = inputbytes * sizeof(uint16_t) + (inputbytes << (depth + 1)) + sizeof(bool);
      da_header_t next_group = group_new(inputbits - depth - 1, next_element_size);

      for (size_t i = 0; i < group.count - 1; ++i) {
        da_header_t* bucket = da_get(group, i, sizeof(da_header_t));
        da_header_t* next_bucket = da_get(group, i + 1, sizeof(da_header_t));

        for (size_t j = 0; j < bucket->count; ++j) {
          uint8_t* diff_ptr = da_get(*bucket, j, element_size);
          bigint_diff_t diff = bigint_diff_from_allocation(diff_ptr, inputbytes);

          for (size_t k = 0; k < next_bucket->count; ++k) {
            uint8_t* diff2_ptr = da_get(*next_bucket, k, element_size);
            bigint_diff_t diff2 = bigint_diff_from_allocation(diff2_ptr, inputbytes);

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

      printf("\n+");
      for (size_t i = 0; i < inputcharlen; ++i) putc('-', stdout);
      printf("---------+\n| depth: %*zu |\n", inputcharlen, depth);
      size_t c1 = 2 + inputcharlen;
      size_t c2 = 2 + (2 + inputncharlen) * ((size_t) 1 << depth) - 1;
      size_t c3 = inputbytes * 8 + 2 + 2;
      putc('+', stdout);
      for (size_t i = 0; i < c1; ++i) putc('-', stdout);
      putc('+', stdout);
      for (size_t i = 0; i < c2; ++i) putc('-', stdout);
      putc('+', stdout);
      for (size_t i = 0; i < c3; ++i) putc('-', stdout);
      printf("+\n");
      for (size_t i = 0; i < group.count; ++i) {
        da_header_t* bucket = da_get(group, i, sizeof(da_header_t));

        for (size_t j = 0; j < bucket->count; ++j) {
          if (!j) printf("| %*zu | ", inputcharlen, i);
          else {
            printf("| ");
            for (size_t i = 0; i < inputcharlen; ++i) putc(' ', stdout);
            printf(" | ");
          }

          uint8_t* diff_ptr = da_get(*bucket, j, element_size);
          bigint_diff_t diff = bigint_diff_from_allocation(diff_ptr, inputbytes);
          bool used = *(diff_ptr + element_size - sizeof(bool));

          for (size_t n = 0; n < (size_t) 1 << depth; ++n)
            printf("%*td, ", inputncharlen, bigint_into_ptrdiff(bigint_from_allocation(diff_ptr + inputbytes * sizeof(uint16_t) + n * inputbytes, inputbytes)));
          printf("| ");

          if (used) {
            bigint_diff_print(diff, " / |\n");
          } else {
            bigint_diff_print(diff, "   |\n");

            memset(scratch, 0, final_element_size);
            memcpy(scratch, diff_ptr, element_size);

            da_push(&finals, scratch, final_element_size);
          }
        }
      }
      putc('+', stdout);
      for (size_t i = 0; i < c1; ++i) putc('-', stdout);
      putc('+', stdout);
      for (size_t i = 0; i < c2; ++i) putc('-', stdout);
      putc('+', stdout);
      for (size_t i = 0; i < c3; ++i) putc('-', stdout);
      printf("+\n");

      group_destroy(group);
      group = next_group;
    }

    printf("\n+");
    for (size_t i = 0; i < 20 + inputncharlen; ++i) putc('-', stdout);
    printf("+\n");
    printf("| Prime implicants: %*zu |\n", inputncharlen, finals.count);
    putc('+', stdout);
    size_t max_ids = (size_t) 1 << depth;
    size_t c = 4 + inputbytes * 8 + (2 + inputncharlen) * max_ids;
    size_t m = c;
    if (c < 20 + inputncharlen) m = 20 + inputncharlen;
    for (size_t i = 0; i < m; ++i) putc('-', stdout);
    printf("+\n");

    for (size_t i = 0; i < finals.count; ++i) {
      uint8_t* implicant_ptr = da_get(finals, i, final_element_size);
      uint8_t* ids_ptr = implicant_ptr + inputbytes * sizeof(uint16_t);
      bigint_diff_t implicant = bigint_diff_from_allocation(implicant_ptr, inputbytes);

      size_t ids_count;
      for (ids_count = 1; ids_count < max_ids; ++ids_count) {
        bigint_t num = bigint_from_allocation(ids_ptr + ids_count * inputbytes, inputbytes);
        if (bigint_equals_zero(num)) break;
      }
      size_t skipped = max_ids - ids_count;

      printf("| ");
      for (size_t n = 0; n < skipped; ++n) {
        for (size_t j = 0; j < inputncharlen; ++j) putc(' ', stdout);
        printf("  ");
      }
      for (size_t n = 0; n < max_ids - skipped; ++n) {
        bigint_t id = bigint_from_allocation(ids_ptr + n * inputbytes, inputbytes);
        printf("%*td, ", inputncharlen, bigint_into_ptrdiff(id));
      }
      printf("| ");
      bigint_diff_print(implicant, " |\n");
    }

    putc('+', stdout);
    for (size_t i = 0; i < c; ++i) putc('-', stdout);
    printf("+\n");

    bigint_diff_destroy(diff_result);

    group_destroy(group);
    minterms.count = 0;
    dontcares.count = 0;
    finals.count = 0;
  }

  free(truthtable);
  free(scratch);
  da_destroy(minterms);
  da_destroy(dontcares);
  da_destroy(finals);

  puts("\nExiting without errors");
  return 0;
}

void print_truthtable(uint8_t* truthtable, ptrdiff_t inputmax, INPUT_SIZE_TYPE inputbytes, INPUT_SIZE_TYPE outputbytes, int inputncharlen) {
  size_t c1 = inputbytes * 8 + inputncharlen + 5;
  size_t c2 = inputbytes * 8 + 2;
  printf("\n+");
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  printf("+\n");
  printf("| %-*s", (int) (c1 - 1), "input");
  printf("| %-*s|\n", (int) (c2 - 1), "output");
  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  printf("+\n");

  for (ptrdiff_t i = 0; i < inputmax; ++i) {
    printf("| %*td = ", inputncharlen, i);
    print_bits(&i, inputbytes, "");
    printf(" | ");
    bigint_t out = bigint_from_allocation(truthtable + i, outputbytes);
    bigint_print_binary(out, " |\n");
  }

  putc('+', stdout);
  for (size_t i = 0; i < c1; ++i) putc('-', stdout);
  putc('+', stdout);
  for (size_t i = 0; i < c2; ++i) putc('-', stdout);
  putc('+', stdout);
  putc('\n', stdout);
}

// void print_size(unsigned long long bytes) {
//   const char* units[] = { "bytes", "Kb", "Mb", "Gb", "Tb", "Pb" };
//   double size = (double)bytes;
//   int unit = 0;
//   while (size >= 1024.0 && unit < 5) { size /= 1024.0; unit++; }
//   if (unit == 0) { printf("%llu %s", bytes, units[unit]); return; }
//
//   printf("%.0f %s", size, units[unit]);
// }

