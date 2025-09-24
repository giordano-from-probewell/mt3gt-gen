#ifndef AMPLIFIER_H
#define AMPLIFIER_H

#include <stdint.h>
#include <stdbool.h>
#include "application.h"       // usa analog_input_t já definido
#include "amplifier_wiring.h"  // AMP_WIRING[]
#include "ada4254.h"

// number of inputs managed (derivado da tabela)
#define AMP_INPUT_COUNT (8u)

// manager states
typedef enum {
    AMP_MGR_IDLE = 0,
    AMP_MGR_INIT_NEXT,
    AMP_MGR_INIT_POLL,
    AMP_MGR_SETGAIN_NEXT,
    AMP_MGR_SETGAIN_POLL,
    AMP_MGR_INITIATED,
    AMP_MGR_DONE
} amp_mgr_state_t;

typedef struct {
    // target app and inputs
    struct application_st* app;
    analog_input_t* inputs;     // aponta para app->meter.analog_input

    // wiring table
    const amp_wiring_t* table;
    uint8_t table_len;

    // per-channel desired config kept in manager (não polui analog_input_t)
    ada4254_in_gains_t  desired_in[AMP_INPUT_COUNT];
    ada4254_out_gains_t desired_out[AMP_INPUT_COUNT];
    bool                need_gain[AMP_INPUT_COUNT];

    // fsm
    amp_mgr_state_t state;
    uint8_t current_channel;

    // contexts para operações não-blocantes
    ada4254_init_ctx_t init_ctx;
    ada4254_gain_ctx_t gain_ctx;
} amplifier_manager_t;

// Manager API
void amplifier_mgr_init(amplifier_manager_t* mgr);
void amplifier_mgr_bind_from_table(amplifier_manager_t* mgr, application_t * app);
void amplifier_mgr_start(amplifier_manager_t* mgr);
void amplifier_mgr_poll(amplifier_manager_t* mgr, my_time_t now);
bool amplifier_mgr_ok(amplifier_manager_t* mgr);

// Simplified system API
void amplifier_system_start(application_t* app);
void amplifier_system_poll(my_time_t now);
bool amplifier_system_ready(void);

// Pede para mudar o ganho do canal logical (ai_idx) em runtime.
// Retorna true se aceitou o pedido (ai_idx válido), false se não.
bool amplifier_request_gain(uint8_t ai_idx,
                            ada4254_in_gains_t in_gain,
                            ada4254_out_gains_t out_gain);

// Útil para UI: há operação de ganho em progresso?
bool amplifier_is_busy(void);

#endif // AMPLIFIER_H
