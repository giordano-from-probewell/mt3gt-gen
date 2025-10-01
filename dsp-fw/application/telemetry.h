#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <stdbool.h>

/* --------- Config --------- */
#define APP_EVENT_RING_P2   64u           //must be power of 2
#define APP_EVENT_RING_MASK (APP_EVENT_RING_P2 - 1u)

typedef enum {
    EVT_NONE   = 0,
    EVT_STATE  = 200,   // data = new
    EVT_IPC_TX = 100,   // data = channel/cmd
    EVT_IPC_RX = 101,   // data = channel/cmd
    EVT_CLA    = 300,   // data = violation code, etc.
} evt_code_t;

/* Evento enxuto para serial */
typedef struct {
    uint32_t t_ms;      // timestamp local do escritor (cada CPU no seu tempo)
    uint16_t code;      // evt_code_t
    uint16_t data;      // payload curto
} app_evt_t;

/* Ring SPSC: escritor atualiza head; leitor atualiza tail */
typedef struct {
    volatile uint32_t head;             // escrito apenas pelo produtor
    volatile uint32_t tail;             // escrito apenas pelo consumidor
    app_evt_t         buf[APP_EVENT_RING_P2];
} app_ring_t;

/* Contadores do escritor (por CPU) */
typedef struct {
    uint32_t ipc_c1_to_c2_tx;
    uint32_t ipc_c1_to_c2_rx;
    uint32_t ipc_c2_to_c1_tx;
    uint32_t ipc_c2_to_c1_rx;
    uint32_t ipc_timeouts;
    uint32_t cla_access_violations;
    uint32_t state_transitions;
} app_counters_t;

/* Bloco de telemetria que a CPU ESCRITORA controla (ring + counters) */
typedef struct {
    app_ring_t     ring;
    app_counters_t cnt;
} app_telemetry_block_t;

/* API do produtor (rodar NA CPU que TEM direito de escrita) */
void tlm_init_writer(app_telemetry_block_t *tb);
void tlm_push_event(app_telemetry_block_t *tb, uint32_t t_ms, uint16_t code, uint16_t data);

/* API do consumidor (rodar NA CPU que SÓ lê) */
bool tlm_pop_event(app_telemetry_block_t const *tb, app_evt_t *out);  // avança tail
uint32_t tlm_available(app_telemetry_block_t const *tb);

/* Helpers de contadores (rodar na CPU que escreve) */
static inline void tlm_inc(volatile uint32_t *c) { (*c)++; }

#endif
