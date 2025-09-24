#include "driverlib.h"
#include "device.h"
#include "led_pulse_pwm.h"

// Prescaler tables
static const EPWM_ClockDivider   s_clkdiv_tbl[] = {
    EPWM_CLOCK_DIVIDER_1, EPWM_CLOCK_DIVIDER_2, EPWM_CLOCK_DIVIDER_4, EPWM_CLOCK_DIVIDER_8,
    EPWM_CLOCK_DIVIDER_16, EPWM_CLOCK_DIVIDER_32, EPWM_CLOCK_DIVIDER_64, EPWM_CLOCK_DIVIDER_128
};
static const EPWM_HSClockDivider s_hspdiv_tbl[] = {
    EPWM_HSCLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_2, EPWM_HSCLOCK_DIVIDER_4, EPWM_HSCLOCK_DIVIDER_6,
    EPWM_HSCLOCK_DIVIDER_8, EPWM_HSCLOCK_DIVIDER_10, EPWM_HSCLOCK_DIVIDER_12, EPWM_HSCLOCK_DIVIDER_14
};

// ===== estado para updates rápidos =====
static EPWM_ClockDivider   s_clkdiv   = EPWM_CLOCK_DIVIDER_1;
static EPWM_HSClockDivider s_hspdiv   = EPWM_HSCLOCK_DIVIDER_1;
static uint32_t            s_tbclk_hz = 0; // EPWMCLK_HZ / (clkdiv*hspdiv)

// helpers
static inline uint32_t div_value(EPWM_ClockDivider c){
    switch(c){default:
        case EPWM_CLOCK_DIVIDER_1:return 1; case EPWM_CLOCK_DIVIDER_2:return 2;
        case EPWM_CLOCK_DIVIDER_4:return 4; case EPWM_CLOCK_DIVIDER_8:return 8;
        case EPWM_CLOCK_DIVIDER_16:return 16; case EPWM_CLOCK_DIVIDER_32:return 32;
        case EPWM_CLOCK_DIVIDER_64:return 64; case EPWM_CLOCK_DIVIDER_128:return 128;}
}
static inline uint32_t hsp_value(EPWM_HSClockDivider h){
    switch(h){default:
        case EPWM_HSCLOCK_DIVIDER_1:return 1; case EPWM_HSCLOCK_DIVIDER_2:return 2;
        case EPWM_HSCLOCK_DIVIDER_4:return 4; case EPWM_HSCLOCK_DIVIDER_6:return 6;
        case EPWM_HSCLOCK_DIVIDER_8:return 8; case EPWM_HSCLOCK_DIVIDER_10:return 10;
        case EPWM_HSCLOCK_DIVIDER_12:return 12; case EPWM_HSCLOCK_DIVIDER_14:return 14;}
}
static inline void apply_cmpa_50(uint16_t tbprd){
    uint16_t cmpa = (uint16_t)((tbprd + 1U)/2U);
    EPWM_setCounterCompareValue(EPWM_LED_BASE, EPWM_COUNTER_COMPARE_A, cmpa);
    EPWM_setCounterCompareShadowLoadMode(EPWM_LED_BASE, EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);
}

void led_pwm_init_for_fast_update(void)
{
    // GPIO mux
    GPIO_setPadConfig(EPWM_LED_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(EPWM_LED_GPIOCFG);
    GPIO_setDirectionMode(EPWM_LED_GPIO, GPIO_DIR_MODE_OUT);

    // ePWM base
    EPWM_disablePhaseShiftLoad(EPWM_LED_BASE);
    EPWM_setTimeBaseCounterMode(EPWM_LED_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_setEmulationMode(EPWM_LED_BASE, EPWM_EMULATION_FREE_RUN);

    // AQ: 50% duty em ePWMxA (HIGH @ ZERO, LOW @ CMPA_UP) → f_out = TBCLK/(TBPRD+1)
    EPWM_setActionQualifierAction(EPWM_LED_BASE, EPWM_AQ_OUTPUT_A,
        EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    EPWM_setActionQualifierAction(EPWM_LED_BASE, EPWM_AQ_OUTPUT_A,
        EPWM_AQ_OUTPUT_LOW,  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    // Prescalers FIXOS → máxima TBCLK para melhor resolução na faixa 18–41 kHz
    s_clkdiv = EPWM_CLOCK_DIVIDER_1;
    s_hspdiv = EPWM_HSCLOCK_DIVIDER_1;
    EPWM_setClockPrescaler(EPWM_LED_BASE, s_clkdiv, s_hspdiv);

    // TBCLK e período placeholder
    s_tbclk_hz = EPWMCLK_HZ / (div_value(s_clkdiv) * hsp_value(s_hspdiv));
    uint16_t tbprd = 1000; // placeholder
    EPWM_setTimeBasePeriod(EPWM_LED_BASE, tbprd);
    apply_cmpa_50(tbprd);
    EPWM_setTimeBaseCounter(EPWM_LED_BASE, 0);

    // Trip Zone: saída forçada LOW (parado)
    EPWM_enableTripZoneSignals(EPWM_LED_BASE, 0);
    EPWM_forceTripZoneEvent(EPWM_LED_BASE, EPWM_TZ_FORCE_EVENT_OST);

    // INT: configurada, mas DESABILITADA inicialmente
    EPWM_setInterruptSource(EPWM_LED_BASE, EPWM_INT_TBCTR_PERIOD);
    EPWM_setInterruptEventCount(EPWM_LED_BASE, 1);
    EPWM_clearEventTriggerInterruptFlag(EPWM_LED_BASE);
    EPWM_disableInterrupt(EPWM_LED_BASE);

    // Registrar no PIE e habilitar a linha
    Interrupt_register(EPWM_LED_INT, &led_energy_toggle_isr);
    //Interrupt_enable(EPWM_LED_INT);

    // TBCLKSYNC ON (time-base roda, mas sem saída por TZ e sem INT)
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}



void led_pwm_set_frequency_fast(float f_hz)
{
    if (s_tbclk_hz == 0U) return;

    if (f_hz <= 0.0f) {
        EPWM_disableInterrupt(EPWM_LED_BASE);
        EPWM_forceTripZoneEvent(EPWM_LED_BASE, EPWM_TZ_FORCE_EVENT_OST);
        return;
    }

    // prd_plus1 = TBCLK / f
    float prd_f = (float)s_tbclk_hz / f_hz;
    if (prd_f < 2.0f)       prd_f = 2.0f;       // => TBPRD >= 1 (evita 0)
    if (prd_f > 65536.0f)   prd_f = 65536.0f;   // => TBPRD <= 65535

    uint16_t tbprd = (uint16_t)((uint32_t)(prd_f + 0.5f) - 1U);

    EPWM_setTimeBasePeriod(EPWM_LED_BASE, tbprd);
    apply_cmpa_50(tbprd);

    EPWM_clearTripZoneFlag(EPWM_LED_BASE, EPWM_TZ_FLAG_OST);
    EPWM_clearEventTriggerInterruptFlag(EPWM_LED_BASE);
    EPWM_enableInterrupt(EPWM_LED_BASE);
}



void led_pwm_get_current_range(float* fmin, float* fmax)
{
    if (fmin) *fmin = (float)s_tbclk_hz / 65536.0f; // ~1.525 kHz com 1:1
    if (fmax) *fmax = (float)s_tbclk_hz / 2.0f;     // ~50 MHz com TBPRD=1
}


void led_pwm_set_from_grid_freq(float f_grid_hz)
{
    // clamp em [35,80] por segurança
    if (f_grid_hz < 35.0f) f_grid_hz = 35.0f;
    if (f_grid_hz > 80.0f) f_grid_hz = 80.0f;

    float f_pwm = (float)LED_PWM_GRID_MULT * f_grid_hz; // 512 × f
    led_pwm_set_frequency_fast(f_pwm);
}

bool led_pwm_set_frequency_slow(float f_hz)
{
    int i,j;
    if (f_hz <= 0.0f) return false;
    uint32_t f_des = (uint32_t)(f_hz + 0.5f);

    EPWM_ClockDivider   best_clk = s_clkdiv;
    EPWM_HSClockDivider best_hsp = s_hspdiv;
    uint16_t            best_prd = 60000;
    uint32_t            best_div = 0xFFFFFFFFUL;

    const EPWM_ClockDivider   clk_tbl[] = {
        EPWM_CLOCK_DIVIDER_1, EPWM_CLOCK_DIVIDER_2, EPWM_CLOCK_DIVIDER_4, EPWM_CLOCK_DIVIDER_8,
        EPWM_CLOCK_DIVIDER_16,EPWM_CLOCK_DIVIDER_32,EPWM_CLOCK_DIVIDER_64,EPWM_CLOCK_DIVIDER_128
    };
    const EPWM_HSClockDivider hsp_tbl[] = {
        EPWM_HSCLOCK_DIVIDER_1,EPWM_HSCLOCK_DIVIDER_2,EPWM_HSCLOCK_DIVIDER_4,EPWM_HSCLOCK_DIVIDER_6,
        EPWM_HSCLOCK_DIVIDER_8,EPWM_HSCLOCK_DIVIDER_10,EPWM_HSCLOCK_DIVIDER_12,EPWM_HSCLOCK_DIVIDER_14
    };

    for ( i=0;i<8;i++){
        for ( j=0;j<8;j++){
            uint32_t div   = div_value(clk_tbl[i]) * hsp_value(hsp_tbl[j]);
            uint32_t tbclk = EPWMCLK_HZ / div;
            if (!tbclk) continue;

            uint32_t prd1 = (tbclk + f_des/2U) / f_des; // arredondado
            if (prd1 == 0U || prd1 > 65536U) continue;

            if (div < best_div){
                best_div = div;
                best_clk = clk_tbl[i];
                best_hsp = hsp_tbl[j];
                best_prd = (uint16_t)(prd1 - 1U);
            }
        }
    }

    bool changed = (best_clk != s_clkdiv) || (best_hsp != s_hspdiv);

    s_clkdiv   = best_clk;
    s_hspdiv   = best_hsp;
    s_tbclk_hz = EPWMCLK_HZ / (div_value(s_clkdiv) * hsp_value(s_hspdiv));

    EPWM_setClockPrescaler(EPWM_LED_BASE, s_clkdiv, s_hspdiv);
    EPWM_setTimeBasePeriod(EPWM_LED_BASE, best_prd);
    apply_cmpa_50(best_prd);

    return changed;
}

float32_t led_freq_from_energy(uint64_t uwh, float32_t cycle_s, uint32_t quantum_uwh)
{
    if (cycle_s <= 0.0f || quantum_uwh == 0U) {
        return 0.0f;
    }
    float energy_uwh = (float)uwh;
    return (energy_uwh / cycle_s) / (float)quantum_uwh;
}
