#ifndef _GENERATION_SM_H
#define _GENERATION_SM_H

#include <stdint.h>
#include <stdbool.h>
#include "generation.h"
#include "generation_sm.h"
#include "reference_generation.h"
#include "protection.h"




// API
void gen_sm_init(gen_sm_ch_t *ch,
                 waveform_generation_t *wg,
                 const gen_sm_hw_cb_t *hw,
                 float32_t ramp_slew_per_tick,
                 uint32_t tick_period_ms);

void gen_sm_request_enable(gen_sm_ch_t *ch);
void gen_sm_request_disable(gen_sm_ch_t *ch);
void gen_sm_request_scale(gen_sm_ch_t *ch, float32_t new_scale);
void gen_sm_fault(gen_sm_ch_t *ch);

void gen_sm_tick(gen_sm_ch_t *ch, my_time_t now);

void gen_sm_change_waveform_soft(gen_sm_ch_t *ch,
                                 reference_generation_t *new_ref);



#endif //_GENERATION_SM_H
