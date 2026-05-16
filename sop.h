#ifndef SOP_H
#define SOP_H

#include <stdint.h>

typedef struct product_t {
  size_t count;
  uint8_t terms[];
} product_t;

typedef struct sop_t {
  size_t count;
  size_t size;
  size_t* ptr;
} sop_t;

#endif // SOP_H

#ifdef SOP_IMPLEMENTATION

#endif // SOP_IMPLEMENTATION

