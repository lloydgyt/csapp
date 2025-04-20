#ifndef CSIM_H
#define CSIM_H

#include <stdbool.h>

typedef struct {
    unsigned long tag;
    unsigned int LRU_bits;
    bool valid;
} cache_line;

#endif