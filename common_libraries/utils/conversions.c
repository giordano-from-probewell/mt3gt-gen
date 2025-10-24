#include "conversions.h"
#include <stdlib.h>

void float32_to_uint8_t(uint8_t *data, float32_t value)
{
    size_t i;
    struct communication_bytes bytes[sizeof(float32_t)];
    memcpy(bytes, &value, sizeof(float32_t));

    for (i = 0; i < sizeof(float32_t); i++) {
        data[2*i] = bytes[i].LSB;
        data[2*i+1] = bytes[i].MSB;
    }
}

float32_t uint8_t_to_float32(const uint8_t *data)
{

    float32_t value;
    myfloat_t tmp;

    tmp.byte.b3 = data[0];
    tmp.byte.b2 = data[1];
    tmp.byte.b1 = data[2];
    tmp.byte.b0 = data[3];

    value = (float32_t)tmp.value;

    return value;
}
