#include "generation_sm.h"
#include "application.h"            // defines waveform_generation_t with command/config/status
#include "reference_generation.h"
#include <math.h>

static inline float32_t clamp01(float32_t x) {
//    if (x < 0.0f) return 0.0f;
//    if (x > 1.0f) return 1.0f;
    return x;
}

static void apply_scale(gen_sm_ch_t *ch, float32_t s)
{
    s = clamp01(s);
    ch->scale_cur = s;
    ch->wg->config.scale = s;   // mirror in the app "model"
}

void gen_sm_init(gen_sm_ch_t *ch,
                 waveform_generation_t *wg,
                 const gen_sm_hw_cb_t *hw,
                 float32_t ramp_slew_per_tick,
                 uint32_t tick_period_ms)
{
    ch->wg = wg;
    ch->hw = *hw;
    ch->ramp_slew      = (ramp_slew_per_tick > 0.f) ? ramp_slew_per_tick : 0.01f;
    ch->tick_period_ms = (tick_period_ms != 0u) ? tick_period_ms : 5u;

    ch->state = GEN_SM_OFF;
    ch->next_tick_ms = 0;
    ch->scale_cur = 0.f;
    ch->scale_before_swap = 0.f;
    ch->waveform_swap_pending = false;
    apply_scale(ch, 0.f);

    ch->wg->status.ready_to_generate = false;
    ch->wg->status.generating        = false;
}

void gen_sm_request_enable(gen_sm_ch_t *ch)
{
    if (ch->state == GEN_SM_OFF || ch->state == GEN_SM_DISARMING) {
        ch->state = GEN_SM_ARMING;
    }
}

void gen_sm_request_disable(gen_sm_ch_t *ch)
{
    if (ch->state == GEN_SM_RUN || ch->state == GEN_SM_RAMP_UP) {
        ch->state = GEN_SM_RAMP_DOWN;
        ch->wg->config.scale_requested = 0.f;
    }
}

void gen_sm_request_scale(gen_sm_ch_t *ch, float32_t new_scale)
{
    ch->wg->config.scale_requested = clamp01(new_scale);
    
    // Update state based on new scale request, regardless of current state
    if (ch->state == GEN_SM_RUN || ch->state == GEN_SM_RAMP_UP || ch->state == GEN_SM_RAMP_DOWN) {
        if (ch->wg->config.scale_requested > ch->scale_cur) {
            ch->state = GEN_SM_RAMP_UP;
        } else if (ch->wg->config.scale_requested < ch->scale_cur) {
            ch->state = GEN_SM_RAMP_DOWN;
        } else {
            ch->state = GEN_SM_RUN; // Target reached
        }
    }
}

void gen_sm_fault(gen_sm_ch_t *ch)
{
    ch->state = GEN_SM_FAULT;
    if (ch->hw.pwm_force_trip) ch->hw.pwm_force_trip();
    if (ch->hw.cla_off_task)   ch->hw.cla_off_task();
    ch->wg->status.generating = false;
}

static void step_ramp_to(gen_sm_ch_t *ch, float32_t target)
{
    float32_t s = ch->scale_cur;
    float32_t st = ch->ramp_slew;
    if (fabsf(target - s) <= st) s = target;
    else if (target > s)         s += st;
    else                         s -= st;
    apply_scale(ch, s);
}

void gen_sm_tick(gen_sm_ch_t *ch, my_time_t now)
{
    if (now < ch->next_tick_ms) return;
    ch->next_tick_ms = now + ch->tick_period_ms;

    // Protection shortcut: if any protection requests disable, fault out
    bool prot_err = ch->wg->command.disable_from_protection_by_control_error ||
                    ch->wg->command.disable_from_protection_by_saturation;
    if (prot_err && ch->state != GEN_SM_OFF && ch->state != GEN_SM_FAULT) {
        gen_sm_fault(ch);
        return;
    }

    switch (ch->state) {
    case GEN_SM_OFF:
        if (ch->wg->command.enable || ch->wg->command.enable_from_cli || ch->wg->command.enable_from_comm) {
            ch->state = GEN_SM_ARMING;
        }
        break;

    case GEN_SM_ARMING:
        if (ch->hw.pwm_clear_trip) ch->hw.pwm_clear_trip();
        if (ch->hw.cla_on_task)    ch->hw.cla_on_task();
        ch->wg->status.ready_to_generate = true;
        ch->wg->status.generating        = true;
        ch->state = (ch->wg->config.scale_requested <= ch->scale_cur) ? GEN_SM_RUN : GEN_SM_RAMP_UP;
        break;

    case GEN_SM_RAMP_UP:
        step_ramp_to(ch, ch->wg->config.scale_requested);
        if (ch->scale_cur >= ch->wg->config.scale_requested) {
            ch->state = GEN_SM_RUN;
        } else if (ch->wg->config.scale_requested < ch->scale_cur) {
            // Scale request changed direction during ramp
            ch->state = GEN_SM_RAMP_DOWN;
        }
        break;

    case GEN_SM_RUN:
        if (ch->wg->command.disable || ch->wg->command.disable_from_cli || ch->wg->command.disable_from_comm) {
            ch->state = GEN_SM_RAMP_DOWN;
            ch->wg->config.scale_requested = 0.f;
        }
        break;

    case GEN_SM_RAMP_DOWN:
        step_ramp_to(ch, ch->wg->config.scale_requested);
        if (ch->scale_cur <= ch->wg->config.scale_requested) {
            if (ch->wg->config.scale_requested == 0.f) {
                if (ch->waveform_swap_pending) {
                    ch->state = GEN_SM_WAVEFORM_SWAP;
                } else {
                    ch->state = GEN_SM_DISARMING;
                }
            } else {
                ch->state = GEN_SM_RUN;
            }
        } else if (ch->wg->config.scale_requested > ch->scale_cur) {
            // Scale request changed direction during ramp
            ch->state = GEN_SM_RAMP_UP;
        }
        break;

    case GEN_SM_WAVEFORM_SWAP:
        // Toggle waveform type between 1 and 2
        if (ch->wg->ref.type == 1) {
            ch->wg->ref.type = 2;
        } else {
            ch->wg->ref.type = 1;
        }
        
        // Clear the swap pending flag
        ch->waveform_swap_pending = false;
        
        // Request ramp up to previous scale
        ch->wg->config.scale_requested = ch->scale_before_swap;
        
        if (ch->wg->config.scale_requested > 0.f) {
            ch->state = GEN_SM_RAMP_UP;
        } else {
            ch->state = GEN_SM_RUN;
        }
        break;

    case GEN_SM_DISARMING:
        if (ch->hw.pwm_force_trip) ch->hw.pwm_force_trip();
        if (ch->hw.cla_off_task)   ch->hw.cla_off_task();
        ch->wg->status.generating        = false;
        ch->wg->status.ready_to_generate = false;
        ch->state = GEN_SM_OFF;
        break;

    case GEN_SM_FAULT:
    default:
        break;
    }
}

void gen_sm_change_waveform_soft(gen_sm_ch_t *ch)
{
    // Only allow waveform change if we're running, ramping, or already swapping
    if (ch->state == GEN_SM_RUN || ch->state == GEN_SM_RAMP_UP || ch->state == GEN_SM_RAMP_DOWN || ch->state == GEN_SM_WAVEFORM_SWAP) {
        // Save current scale to restore after swap
        ch->scale_before_swap = ch->wg->config.scale_requested;
        ch->waveform_swap_pending = true;
        
        // Request ramp down to zero
        ch->wg->config.scale_requested = 0.f;
        ch->state = GEN_SM_RAMP_DOWN;
    }
}

