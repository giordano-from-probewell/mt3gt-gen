#ifndef _GENERATION_H
#define _GENERATION_H

#include <stdint.h>
#include <stdbool.h>
#include "my_time.h"
#include "types.h"

#include "protection.h"
#include "reference_generation.h"






void initHRPWM(uint32_t base1, uint32_t base2, uint32_t period, uint32_t deadband);
void setupEPWMActiveHighComplementary(uint32_t base, uint32_t deadband);


void setupInverter(uint32_t voltage1, uint32_t voltage2,
                   uint32_t current1, uint32_t current2,
                   uint16_t pwm_period_ticks,
                   uint16_t pwm_deadband_ticks);

void gen_off(void);
void gen_on(void);

void update_pwm_voltage(float32_t duty);
void update_pwm_current(float32_t duty);

#endif
