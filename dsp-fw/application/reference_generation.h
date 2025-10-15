/*
 * reference_generation.h
 *
 *  Created on: Sep 7, 2023
 *      Author: GiordanoWolaniuk
 */

#ifndef __REFERENCE_GENERATION_H_
#define __REFERENCE_GENERATION_H_

#include "rampgen.h"
#include "hw_types.h"

#define REF_GEN_DEFAULT_SIZE    1000
#define REF_GEN_MAX_SIZE        1000




typedef struct reference_generation_st
{
    float32_t *waveform1;
    float32_t *waveform2;

    uint16_t waveform_size;
    uint16_t waveform_index;
    uint8_t type;
    RAMPGEN rgen;
    bool cycle_completed;


} reference_generation_t;

extern reference_generation_t ref;

void reference_init(reference_generation_t *r);
float reference_routine(reference_generation_t *r);

#endif
