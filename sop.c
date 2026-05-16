#ifndef SOP_H
#define SOP_H

#include <stdint.h>

typedef struct product_t {
  size_t count;
  uint8_t terms[];
} product_t;

#endif // SOP_H

#ifdef SOP_IMPLEMENTATION

#endif SOP_IMPLEMENTATION

