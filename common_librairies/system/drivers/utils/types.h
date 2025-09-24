#ifndef _TYPES_H_
#define _TYPES_H_

#include "hw_types.h"

//typedef          char   int8_t;
//typedef unsigned char  uint8_t;

//typedef          int    int16_t;
//typedef unsigned int   uint16_t;
//typedef          long   int32_t;
//typedef unsigned long  uint32_t;


//data IEEE754 to C2000 memory format
//Ex: 0.00628300011 - IEEE754: 0x3BCDE1A0 - C2000: [0xE2D5][0x3BCD]
typedef union myfloat_u{
        float32_t value;
        struct {
            uint8_t b0: 8;
            uint8_t b1: 8;
            uint8_t b2: 8;
            uint8_t b3: 8;
        } byte;
}myfloat_t;


#endif /* UTILS_TYPES_H_ */
