#ifndef ZERO_PHASE_TIMER_H
#define ZERO_PHASE_TIMER_H

#include <stdint.h>
#include <stdbool.h>

// You can override this if needed, but Timer1 is a good choice.
#ifndef ZPT_TIMER_BASE
  #define ZPT_TIMER_BASE   CPUTIMER1_BASE
#endif

#ifndef SYSCLK_HZ
  #define SYSCLK_HZ        200000000UL   // 200 MHz on F28379D (adjust if different)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialize CPU Timer1 for maximum speed (no prescaler), PRD = 0xFFFFFFFF,
// free-run in emulation. Starts counting immediately.
void zpt_init_timer_fast(void);

// Call from your ZERO-CROSS ISR to "zero" elapsed time safely.
// This reloads TIM from PRD in hardware (no stop/start jitter).
void zpt_reset_in_isr(void);

// Get elapsed ticks since the last reset (wrap-safe).
// This is just (PRD - TIM) with PRD=0xFFFFFFFF.
uint32_t zpt_get_elapsed_ticks(void);

// Convert elapsed ticks to seconds using the configured tick period.
float zpt_ticks_to_seconds(uint32_t ticks);

// Convenience: read elapsed time (seconds) right now (for LED ISR).
float zpt_get_elapsed_seconds(void);

#ifdef __cplusplus
}
#endif

#endif // ZERO_PHASE_TIMER_H
