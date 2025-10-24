#ifndef CONVERSIONS_H_
#define CONVERSIONS_H_

#include "types.h"

struct communication_bytes {
 unsigned int LSB:8;
 unsigned int MSB:8;
};

void float32_to_uint8_t(uint8_t *data, float32_t value);
float32_t uint8_t_to_float32(const uint8_t *data);
#endif
