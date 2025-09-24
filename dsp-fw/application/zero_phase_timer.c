#include "driverlib.h"
#include "device.h"
#include "zero_phase_timer.h"

// Cached config
static uint32_t s_prd_ticks = 0xFFFFFFFFUL;
static float    s_tick_to_seconds = 0.0f; // = 1 / (SYSCLK_HZ / (prescale+1))

void zpt_init_timer_fast(void)
{
    // Stop while configuring (optional)
    CPUTimer_stopTimer(ZPT_TIMER_BASE);

    // Fastest tick: prescaler = 0 -> divide-by-1
    CPUTimer_setPreScaler(ZPT_TIMER_BASE, 0);

    // Max period for comfortable math: free-run down from 0xFFFFFFFF
    CPUTimer_setPeriod(ZPT_TIMER_BASE, s_prd_ticks);

    // Emulation: keep running in debug
    CPUTimer_setEmulationMode(ZPT_TIMER_BASE, CPUTIMER_EMULATIONMODE_RUNFREE);

    // Clear overflow flag and load PRD into TIM
    CPUTimer_clearOverflowFlag(ZPT_TIMER_BASE);
    CPUTimer_reloadTimerCounter(ZPT_TIMER_BASE);

    // Start timer
    CPUTimer_startTimer(ZPT_TIMER_BASE);

    // Cache tick period (seconds per tick)
    // CPU Timer ticks at SYSCLK / (prescaler+1) -> here prescaler=0
    s_tick_to_seconds = 1.0f / (float)SYSCLK_HZ;
}

// ---- inline helpers (defined here so they can be inlined) ----
void zpt_reset_in_isr(void)
{
    // Reload TIM from PRD atomically (no need to stop the timer)
    CPUTimer_reloadTimerCounter(ZPT_TIMER_BASE);
    // Optional: clear overflow flag in case you also use it elsewhere
    CPUTimer_clearOverflowFlag(ZPT_TIMER_BASE);
}

uint32_t zpt_get_elapsed_ticks(void)
{
    // Timer counts DOWN from PRD; elapsed = PRD - TIM (wrap-safe for 32-bit)
    uint32_t tim = CPUTimer_getTimerCount(ZPT_TIMER_BASE);
    return s_prd_ticks - tim;
}

float zpt_ticks_to_seconds(uint32_t ticks)
{
    return (float)ticks * s_tick_to_seconds;
}

float zpt_get_elapsed_seconds(void)
{
    return zpt_ticks_to_seconds(zpt_get_elapsed_ticks());
}

// Provide external linkage for the inlines if compiler requires
void zpt_reset_in_isr_link(void){ zpt_reset_in_isr(); }
uint32_t zpt_get_elapsed_ticks_link(void){ return zpt_get_elapsed_ticks(); }
float zpt_get_elapsed_seconds_link(void){ return zpt_get_elapsed_seconds(); }
