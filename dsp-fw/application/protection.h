#ifndef _PROTECTION_H_
#define _PROTECTION_H_

#include "types.h"

typedef enum protection_sm_enum{
    STATE_PROTECTION__INIT,
    STATE_PROTECTION__MONITOR,
    STATE_PROTECTION__ERROR_DETECTED,
    STATE_PROTECTION__STAGNATION_DETECTED,
    STATE_PROTECTION__RESET
} protection_sm_state_t;

typedef struct protection_sm{
    char *initialized;
    protection_sm_state_t state;
} protection_sm_t;


typedef struct protection_error_monitor_st{
    protection_sm_t sm;
    struct
    {
        uint16_t ctrl_error_growth_threshold; //n of cycles with error_rms increasing in sequence to generation error
        uint16_t pwm_max_value; //maximum acceptable value;
        uint16_t pwm_min_value; //minimum acceptable value;
        uint16_t stagnation_threshold; //n of cycles with error_rms to consider stagnated
        float32_t acceptable_percentage_diff;
        float32_t acceptable_error;
    }config;
    struct
    {
        float32_t last_ctrl_error;
        uint16_t consecutive_ctrl_error_growth_count;
        uint16_t stagnation_count;
        float32_t previous_difference;
        float32_t max_pwm_duty;
        float32_t min_pwm_duty;
        int16_t event;
    }data;  //ephemeral data

} protection_error_monitor_t;


void init_protection(
        protection_error_monitor_t *monitor,
        int error_growth_threshold,
        int stagnation_threshold,
        float32_t max_acceptable_difference,
        float32_t max_acceptable_error,
        int pwm_max,
        int pwm_min
);
int process_protection(protection_error_monitor_t *monitor, float32_t ctrl_error_rms, float32_t setpoint_rms, float32_t out_rms);
void reset_protection(protection_error_monitor_t *monitor);
#endif
