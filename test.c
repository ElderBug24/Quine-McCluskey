#define INPUT_SIZE_TYPE uint8_t

#define BIGINT_IMPLEMENTATION
#include "bigint.h"

#define DYNAMICARRAY_IMPLEMENTATION
#include "dynamic_array.h"


void bigint_print_decimal(const bigint_t num) {
  if (num.count == 0) {
    putc('0', stdout);
    return;
  }

  size_t decimal_len = num.count * 3 + 1;
  uint8_t* decimal = calloc(decimal_len, 1);

  size_t used = 1;
  decimal[0] = 0;

  for (size_t i = num.count; i-- > 0;) {
    uint32_t carry = num.ptr[i];

    for (size_t j = 0; j < used; ++j) {
      uint32_t value = decimal[j] * 256 + carry;
      decimal[j] = value % 10;
      carry = value / 10;
    }

    while (carry) {
      decimal[used++] = carry % 10;
      carry /= 10;
    }
  }

  while (used > 1 && decimal[used - 1] == 0) {
    --used;
  }

  char* out = malloc(used + 1);

  for (size_t i = 0; i < used; ++i) {
    out[i] = decimal[used - 1 - i] + '0';
  }

  out[used] = '\0';

  free(decimal);

  printf("%s", out);

  free(out);
}

int main() {
  // for (uint8_t i = 0; i < 16; ++i) {
  //   printf("%u: %u\n", i, uint8_binary_weight(i));
  // }
  //
  // bigint_t a = bigint_new(2);
  //
  // ({
  //   uint16_t temp = 0b0011000011111011;
  //   bigint_set(&a, (uint8_t*) &temp, 2);
  // });
  //
  // printf("%zu\n", bigint_binary_weight(a));
  //
  // bigint_print_binary(a, "\n");
  //
  // for (int i = 0; i < 10; ++i) {
  //   bigint_inc(&a);
  //
  //   bigint_print_binary(a, "\n");
  // }

  // uint8_t a = 0b01011101;
  // uint8_t b = 0b00111001;
  //
  // byte_diff_result_t diff_result = diff_byte(a, b);
  //
  // puts(" | | | | | | | |");
  // print_byte_bits_uint16(diff_result.diff, "\n");
  // printf("%u\n", diff_result.count);
  //
  // uint16_t diff = diff_result.diff; // 0b0010100101100001
  // uint16_t diff2 = 0b0001100100101001;
  //
  // diff_print(diff, "\n");
  //
  // diff_result = diff_diff(diff, diff2);
  //
  // puts(" | | | | | | | |");
  // print_byte_bits_uint16(diff_result.diff, "\n");
  // printf("%u\n", diff_result.count);
  // diff_print(diff_result.diff, "\n");

  // bigint_t a = bigint_new(8);
  // bigint_t b = bigint_new(8);
  //
  // uint64_t temp = 0b0000000001101011010010010011100001100001101011011011110010011101; // 30198329013812381
  // bigint_set(&a, (uint8_t*) &temp, 8);
  // temp =          0b0000000001101011010010010011100001100001111010000011100010011111;
  // bigint_set(&b, (uint8_t*) &temp, 8);
  //
  // puts("0000000001101011010010010011100001100001101011011011110010011101");
  // bigint_print_binary(a, "\n");
  // bigint_print_binary(b, "\n");
  //
  // bigint_diff_result_t result = bigint_diff(a, b);
  // bigint_diff_print(result.diff, "\n");
  // // bigint_diff_print_binary(result.diff, "\n");
  // printf("%u\n", result.count);
  //
  // bigint_diff_t diff2 = {
  //   .ptr = malloc(8),
  //   .count = 8
  // };
  // *((uint64_t*) diff2.ptr + 1) = 0b1001010010000000001001000100010100010000010000010000010101000000;
  // *((uint64_t*) diff2.ptr) = 0b0001010000100001011001000110001010001001011000000100000101011001;
  //
  // bigint_diff_print(diff2, "\n");
  //
  // bigint_diff_result_t result2 = bigint_diff_diff(result.diff, diff2);
  // bigint_diff_print(result2.diff, "\n");
  // printf("%u\n", result2.count);
  //
  // bigint_destroy(a);
  // bigint_destroy(b);
  // bigint_diff_destroy(result.diff);
  // bigint_diff_destroy(diff2);
  //
  // uint16_t aaa = 0b1111111100000000;
  // print_bits((void*) &aaa, 2, "\n");

  // uint8_t aa = 0b10111001;
  // uint16_t diff_ = byte_into_diff(aa);
  // print_bits((void*) &aa, 1, "\n");
  // print_bits((void*) &diff_, 2, "\n");
  //
  // bigint_t a = bigint_new(8);
  // uint64_t temp = 0b0000000001101011010010010011100001100001101011011011110010011101; // 30198329013812381
  // bigint_set(&a, (uint8_t*) &temp, 8);
  // bigint_print_binary(a, "\n");
  // bigint_diff_t diff = bigint_into_diff(a);
  // bigint_diff_print(diff, "\n");

  // bigint_t a = bigint_new(8);
  // for (size_t i = 0; i < 123456789; ++i) {
  //   bigint_inc(a);
  // }
  //
  // bigint_print_decimal(a);
  //
  // bigint_destroy(a);

  void* arr = dynamic_array_with_capacity(1, sizeof(uint16_t));
  // printf("cap: %zu\n", dynamic_array_capacity(arr));

  uint16_t temp = 17;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 69;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 70;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 71;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 72;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 73;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 74;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));
  temp = 75;
  dynamic_array_push(&arr, &temp, sizeof(uint16_t));
  printf("cap: %zu\n", dynamic_array_capacity(arr));

  for (size_t i = 0; i < dynamic_array_len(arr); ++i) {
    printf("%u\n", ((uint16_t*) arr)[i]);
  }

  dynamic_array_destroy(arr);

  // for (size_t i = 0; i < 3; ++i) {
  //   size_t m = 1 << i;
  //   size_t arr[m] = {};
  //
  //   for (size_t j = 0; j < m; ++j) {
  //     arr[j] = 2 * j + 1;
  //   }
  //
  //   for (size_t j = m; j != 0;) {
  //     printf("%zu, ", arr[--j]);
  //   }
  //   puts("");
  // }

  puts("\nexiting without errors");
  return 0;
}

