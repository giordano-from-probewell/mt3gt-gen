// led_pulse_pwm.h
#ifndef LED_PULSE_PWM_H
#define LED_PULSE_PWM_H

#include <stdint.h>
#include <stdbool.h>

// ========= CONFIG OVERRIDES (define before including this header) =========
#ifndef EPWM_LED_BASE
#define EPWM_LED_BASE      EPWM6_BASE
#endif
#ifndef EPWM_LED_INT
#define EPWM_LED_INT       INT_EPWM6
#endif
#ifndef EPWM_LED_GPIO
#define EPWM_LED_GPIO      10
#endif
#ifndef EPWM_LED_GPIOCFG
#define EPWM_LED_GPIOCFG   GPIO_10_GPIO10
#endif

#ifndef SYSCLK_HZ
#define SYSCLK_HZ          200000000UL
#endif
#ifndef EPWMCLK_HZ
#define EPWMCLK_HZ         (SYSCLK_HZ/2)   // 100 MHz
#endif

// Multiplicador do PWM (512 amostras por ciclo)
#ifndef LED_PWM_GRID_MULT
#define LED_PWM_GRID_MULT  512U
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Init único: configura GPIO, ePWM (up-count, AQ 50%), INT (1 por período),
// prescalers fixos, e inicia PARADO (Trip Zone ON e INT OFF).
void led_pwm_init_for_fast_update(void);

// Atualiza a frequência de chaveamento (Hz) de forma rápida (ISR-safe).
// f_hz <= 0 → para (Trip Zone + INT OFF). f_hz > 0 → aplica TBPRD/CMPA e habilita.
void led_pwm_set_frequency_fast(float f_hz);

// Novo: recebe a frequência de rede (35..80 Hz) e programa PWM = 512 × f_rede.
void led_pwm_set_from_grid_freq(float f_grid_hz);

// (Opcional, fora da ISR) reescolhe prescalers e aplica f (para faixas diferentes).
bool led_pwm_set_frequency_slow(float f_hz);

// Utilitário existente (mantido)
float32_t led_freq_from_energy(uint64_t uwh, float32_t cycle_s, uint32_t quantum_uwh);

// Sua ISR (implemente em outro lugar se quiser lógica extra)
extern __interrupt void led_energy_toggle_isr(void);

#ifdef __cplusplus
}
#endif
#endif // LED_PULSE_PWM_H
