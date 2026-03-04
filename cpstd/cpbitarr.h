#pragma once

#include "cpbase.h"
#include <stdlib.h>

typedef struct {
    u8 *data;
    u32 bits;
} bit_arr;

void bit_arr_init(bit_arr *a, u32 bits) {
    a->bits = bits;
    u32 bytes = (bits + 7) / 8;
    a->data = calloc(bytes, 1);
}
void bit_arr_destroy(bit_arr *a) {
    free(a->data);
    a->bits = 0;
}
void bit_arr_set(bit_arr *a, u32 i) {
    u32 byte = i >> 3;
    u32 bit = i & 7;
    a->data[byte] |= (1 << bit);
}
void bit_arr_clear(bit_arr *a, u32 i) {
    u32 byte = i >> 3;
    u32 bit = i & 7;
    a->data[byte] &= ~(1 << bit);
}
b8 bit_arr_get(bit_arr *a, u32 i) {
    u32 byte = i >> 3;
    u32 bit = i & 7;
    return (a->data[byte] >> bit) & 1;
}
