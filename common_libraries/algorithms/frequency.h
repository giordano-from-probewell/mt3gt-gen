//#ifndef _FREQUENCY_H_
//#define _FREQUENCY_H_
//
//typedef struct
//{
//	float b0;	// filter constants coeficients
//	float b1;
//	float b2;
//	float a0;
//	float a1;
//	float a2;
//
//	float x_1;	// state variables
//	float x_2;
//	float y_1;
//	float y_2;
//
//} filter_bandpass_order1_st;
//
//typedef enum trigger_enum {PRE_TRIGGER, TRIGGER, POS_TRIGGER} trigger_state_t;
//
//#define SAMPLE_PERIOD (1.0f/(20E6/512.0))
//
//typedef struct
//{
//    float frequency;
//    float period;
//    trigger_state_t wave_state;
//} zero_cross_freq_st;
//
//
//extern zero_cross_freq_st zcf;
//
//// get one sample and returns filtered sample
////float filter_bandpass(const float x, filter_bandpass_order1_st* f);
//
//// state machine to determine the frequency
//int frequency_estimation(float sample);
//
//#endif
//
//
//


#ifndef _FREQUENCY_H_
#define _FREQUENCY_H_

#include <stdint.h>

typedef struct {
    // coeficientes do filtro passa‑faixa (ordem 2 em forma direta I)
    float b0, b1, b2;
    float a0, a1, a2;
    // estados do filtro
    float x_1, x_2;
    float y_1, y_2;
} filter_bandpass_order1_st;

typedef enum trigger_enum { PRE_TRIGGER, TRIGGER, POS_TRIGGER } trigger_state_t;

typedef struct {
    float frequency;          // Hz
    float period;             // s
    trigger_state_t state;
} zero_cross_freq_st;

// Contexto completo para um canal de estimação de frequência
typedef struct {
    filter_bandpass_order1_st filt; // filtro passa-faixa (com estados internos)
    // janela deslizante de 5 amostras filtradas (y0 = mais recente)
    float y0, y1, y2, y3, y4;
    // cronômetro e auxiliares
    float last_time;          // instante relativo do último zero (em amostras fracionárias)
    int   counter;            // amostras desde o último zero detectado
    int   counter_pos;        // quantas amostras consecutivas >= 0 (para “debounce”)
    float Ts;                 // período de amostragem (s)
    // resultado / FSM
    zero_cross_freq_st zcf;
} zc_ctx_t;

/** Inicializa um contexto.
 *  - `fcoef` com os coeficientes do filtro (a0 deve ser 1.0)
 *  - `Ts` período de amostragem em segundos
 *  - estado inicial PRE_TRIGGER para evitar falso gatilho
 */
void zc_init(zc_ctx_t *ctx, const filter_bandpass_order1_st *fcoef, float Ts);

/** Reincia somente a FSM/contadores (mantém os estados do filtro se quiser “aquecer”).
 *  Se quiser zerar tudo, chame zc_init novamente.
 */
void zc_reset_runtime(zc_ctx_t *ctx);

/** Processa UMA amostra bruta.
 *  Retorna 1 quando uma passagem por zero ASCENDENTE for confirmada e frequência/ período atualizados.
 *  Escreve em `out_freq` (Hz) e `out_period` (s) se não forem NULL.
 */
int zc_process_sample(zc_ctx_t *ctx, float sample, float *out_freq, float *out_period);

#endif // _FREQUENCY_H_
