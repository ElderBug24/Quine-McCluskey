#define BIGINT_IMPLEMENTATION
#include "bigint.h"

#include <stdio.h>

#define INPUT_SIZE_TYPE uint8_t


void wrapper(uint8_t* input, uint8_t* output) {}

int main() {
  constexpr INPUT_SIZE_TYPE   inputbits = 8;
  constexpr INPUT_SIZE_TYPE  outputbits = 8;
  constexpr INPUT_SIZE_TYPE  inputbytes = ( inputbits + 7) / 8;
  constexpr INPUT_SIZE_TYPE outputbytes = (outputbits + 7) / 8;

  bigint_t  input = bigint_new(inputbytes);
  bigint_t output = bigint_new(outputbytes);

  size_t max = 1 << inputbits;
  for (size_t i = 0; i < max; ++i) {
    bigint_print_binary(input, " -> ");
    bigint_print_binary(output, "\n");
    wrapper(input.ptr, output.ptr);

    bigint_inc(input);
  }

  return 0;
}

