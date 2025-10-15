#ifndef _GENERATION_H
#define _GENERATION_H

#include <stdint.h>
#include <stdbool.h>
#include "my_time.h"
#include "types.h"

#include "protection.h"
#include "reference_generation.h"

typedef enum {
    GEN_SM_OFF = 0,
    GEN_SM_ARMING,       // enable HW and prepare CLA/PWM
    GEN_SM_RAMP_UP,      // ramp up scale_requested
    GEN_SM_RUN,          // steady state
    GEN_SM_RAMP_DOWN,    // ramp down
    GEN_SM_DISARMING,    // disarm HW
    GEN_SM_FAULT         // error / protection
} gen_sm_state_t;

typedef enum mode_en
{
    MODE_OFF                        = 0,
    MODE_NONE                       = 1,
    MODE_START                      = 2,
    MODE_FEEDFORWARD                = 3,
    MODE_SFRA                       = 4,
    MODE_REPETITIVE                 = 7,
    MODE_REPETITIVE_FROM_CLA        = 8,
} mode_t;

typedef struct waveform_generation_st
{
    my_time_t scheduling;
    reference_generation_t ref;

    struct  {
        bool from_control_loop;
        bool start_generation;
    } trigger;

    struct  {
        float32_t scale;
        float32_t scale_requested;
    } config;

    struct  {
        bool enable;
        bool disable;
        bool enable_from_cli;
        bool disable_from_cli;
        bool enable_from_comm;
        bool disable_from_comm;
        bool disable_from_protection_by_control_error;
        bool disable_from_protection_by_saturation;
    } command;

    struct  {
        bool ready_to_generate;
        bool generating;
    } status;

    mode_t controller;
    //rep_controller_t control;
    protection_error_monitor_t protection;

} waveform_generation_t;


// Callbacks
typedef struct gen_sm_hw_cb_st {
    void (*pwm_clear_trip)(void);
    void (*pwm_force_trip)(void);
    void (*cla_on_task)(void);
    void (*cla_off_task)(void);
} gen_sm_hw_cb_t;



// General sm to generate current or voltage
typedef struct gen_sm_ch_st {
    waveform_generation_t *wg;
    gen_sm_state_t state;
    gen_sm_hw_cb_t hw;

    float32_t ramp_slew;        // inc by tick (0..1.0)
    float32_t scale_cur;        // mirror value (wg->config.scale)

    my_time_t next_tick_ms;
    uint32_t  tick_period_ms;
} gen_sm_ch_t;


typedef struct generation_st
{
    struct
    {
        uint32_t inverter_pwm_steps;
        uint32_t deadband;
        uint32_t generation_freq;
    }config;

    gen_sm_ch_t voltage;
    gen_sm_ch_t current;

    bool sync_flag;
    uint16_t mep_status;
    bool zero_trigger;

} generation_t;


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
