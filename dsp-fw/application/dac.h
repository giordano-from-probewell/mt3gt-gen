#ifndef _DAC_H_
#define _DAC_H_

#include <stdint.h>

//
// Defines
//
#define DLOG_SIZE             1024
#define REFERENCE_VDAC        0
#define REFERENCE_VREF        1
#define REFERENCE             REFERENCE_VREF
#define DACA                  1
#define DACB                  2
#define DACC                  3


extern volatile struct DAC_REGS* DAC_PTR[];

void DAC_init(uint16_t dac_num);

#endif
