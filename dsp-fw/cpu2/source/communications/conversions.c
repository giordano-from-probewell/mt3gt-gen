#include "conversions.h"
#include <stdlib.h>

void float32_to_uint8_t(uint8_t *data, float value)
{
    size_t i;
    struct communication_bytes bytes[sizeof(float)];
    memcpy(bytes, &value, sizeof(float));

    for (i = 0; i < sizeof(float); i++) {
        data[2*i] = bytes[i].LSB;
        data[2*i+1] = bytes[i].MSB;
    }}
