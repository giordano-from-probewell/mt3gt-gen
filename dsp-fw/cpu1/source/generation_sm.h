#ifndef _GENERAGION_SM_H
#define _GENERAGION_SM_H

#include <stdint.h>
#include <stdbool.h>
#include "application.h"

typedef enum gen_event_en {
    GEN_EVT_NONE = 0,
    GEN_EVT_CMD_NEW_TARGET,      // new target from comm
    GEN_EVT_ZERO_CROSS,          // passou por zero
    GEN_EVT_PROTECTION_TRIP,     // proteção disparou
} gen_event_t;

typedef enum gen_state_en {
    GEN_ST_IDLE = 0,             // parado
    GEN_ST_RAMP_DOWN,            // rampa até 0
    GEN_ST_SWITCH_WAVEFORM,      // troca forma
    GEN_ST_RAMP_UP,              // rampa até o alvo
    GEN_ST_HOLD,                 // gerando normal
    GEN_ST_FAULT,                // proteção
} gen_state_t;

typedef struct gen_target_st {
    float32_t scale_target;      // alvo de magnitude (Vrms/Arms) normalizado
    uint8_t   waveform_target;   // enum/ID de forma
} gen_target_t;

typedef struct gen_profile_st {
    float32_t slew_up_per_zc;    // passo por meia-ciclo (ex.: 0.05 = 5 % por ZC)
    float32_t slew_down_per_zc;
    float32_t min_step;          // garante progresso (>0)
} gen_profile_t;

typedef struct gen_runtime_st {
    gen_state_t  st;
    gen_target_t tgt;            // alvo desejado
    gen_target_t cur;            // efetivo (o que está aplicado)
    bool         pending_wave_switch;
    bool         zc_tick;        // setado a cada ZC
} gen_runtime_t;

void gen_sm_init(gen_runtime_t* r, gen_profile_t prof);
void gen_sm_set_target(gen_runtime_t* r, gen_target_t t);
void gen_sm_on_zero_cross(gen_runtime_t* r);
void gen_sm_on_protection(gen_runtime_t* r);
void gen_sm_turning_on(gen_runtime_t* r);

// chama no loop principal após ZC; ajusta setpoints e retorna se atualizou algo
bool gen_sm_process(gen_runtime_t* r,
                     gen_profile_t prof,
                     application_t* app);

#endif //_GENERAGION_SM_H
