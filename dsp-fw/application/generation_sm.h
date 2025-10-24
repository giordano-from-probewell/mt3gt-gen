#ifndef GENERATION_SM_H
#define GENERATION_SM_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "my_time.h"

// ---- Forward declarations to avoid include cycles hell ----
typedef struct waveform_generation_st  waveform_generation_t;
typedef struct reference_generation_st reference_generation_t;

/**
 * @brief High-level states for one generation channel (voltage OR current).
 */
typedef enum {
    GEN_SM_OFF = 0,     /**< Bridge off, CLA disabled, scale = 0 */
    GEN_SM_ARMING,      /**< Clearing trips, enabling CLA/control */
    GEN_SM_RAMP_UP,     /**< Ramping scale_cur -> scale_requested */
    GEN_SM_RUN,         /**< Steady state at requested scale */
    GEN_SM_RAMP_DOWN,   /**< Ramping to zero before disarm or waveform swap */
    GEN_SM_WAVEFORM_SWAP, /**< At zero, swapping waveform type */
    GEN_SM_DISARMING,   /**< Forcing trip, disabling CLA/control */
    GEN_SM_FAULT        /**< Faulted; requires external reset to OFF */
} gen_sm_state_t;

/**
 * @brief Hardware callback set for one channel.
 *
 * Keep these very short and deterministic � called in the main loop tick.
 */
typedef struct {
    void (*pwm_clear_trip)(void);  /**< Clear OST for both half-bridges of this channel */
    void (*pwm_force_trip)(void);  /**< Force OST for both half-bridges of this channel */
    void (*cla_on_task)(void);     /**< CLA task to start control for this channel */
    void (*cla_off_task)(void);    /**< CLA task to stop control for this channel */
} gen_sm_hw_cb_t;

/**
 * @brief FSM instance for one channel.
 *
 * It mirrors and writes into the "model" object (waveform_generation_t) that
 * lives under application.generation.{voltage|current}.
 */
typedef struct gen_sm_ch_st {
    waveform_generation_t *wg;   /**< Owner "model" with command/status/config fields */
    gen_sm_state_t state;        /**< Current FSM state */
    gen_sm_hw_cb_t hw;           /**< Hardware callbacks */

    // Ramp control
    float32_t ramp_slew;         /**< Increment per tick in [0..1] (normalized scale units) */
    float32_t scale_cur;         /**< Mirror of the applied scale (wg->config.scale) */

    // Waveform swap control
    float32_t scale_before_swap; /**< Scale to restore after waveform swap */
    bool waveform_swap_pending;  /**< Flag indicating waveform swap is requested */

    // Simple scheduling (ms)
    my_time_t next_tick_ms;      /**< Next eligible time for a tick */
    uint32_t  tick_period_ms;    /**< Tick period (ms), e.g. 5ms */
} gen_sm_ch_t;

// -------- API --------

/**
 * @brief Initialize a channel FSM.
 *
 * @param ch        Target FSM object
 * @param wg        Owner "model" (application.generation.voltage/current)
 * @param hw        Hardware callback set
 * @param ramp_slew_per_tick  Increment per tick (0..1). Example: 0.01f
 * @param tick_period_ms      Period for gen_sm_tick() calls. Example: 5 ms
 */
void gen_sm_init(gen_sm_ch_t *ch,
                 waveform_generation_t *wg,
                 const gen_sm_hw_cb_t *hw,
                 float32_t ramp_slew_per_tick,
                 uint32_t tick_period_ms);

/**
 * @brief Request a safe enable sequence (ARMING -> RAMP_UP/RUN).
 */
void gen_sm_request_enable(gen_sm_ch_t *ch);

/**
 * @brief Request a safe disable sequence (RAMP_DOWN -> DISARMING -> OFF).
 */
void gen_sm_request_disable(gen_sm_ch_t *ch);

/**
 * @brief Request a new target scale. FSM will ramp in RUN, or once armed.
 */
void gen_sm_request_scale(gen_sm_ch_t *ch, float32_t new_scale);

/**
 * @brief Enter FAULT state, forcing trip and disabling CLA/control.
 */
void gen_sm_fault(gen_sm_ch_t *ch);

/**
 * @brief Advance the FSM if 'now' reached the channel's next tick time.
 */
void gen_sm_tick(gen_sm_ch_t *ch, my_time_t now);

/**
 * @brief Request a "soft" waveform change: ramp down -> swap -> ramp up.
 *
 * This asks the FSM to ramp down to zero, toggle the waveform type between 
 * 1 and 2, then ramp back up to the previous scale.
 *
 * The waveform type alternates automatically between 1 and 2 on each call.
 */
void gen_sm_change_waveform_soft(gen_sm_ch_t *ch);

#endif // GENERATION_SM_H
