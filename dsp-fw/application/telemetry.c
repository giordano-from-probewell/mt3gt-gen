#include "telemetry.h"

/* Barreira simples p/ ordenar writes na MSGRAM antes do head (C28x) */
static inline void mem_barrier(void) { __asm(" RPT #7 || NOP"); }

/* Produtor */
void tlm_init_writer(app_telemetry_block_t *tb) {
    tb->ring.head = 0u;
    tb->ring.tail = 0u;
    /* zera counters */
    tb->cnt.ipc_c1_to_c2_tx = 0;
    tb->cnt.ipc_c1_to_c2_rx = 0;
    tb->cnt.ipc_c2_to_c1_tx = 0;
    tb->cnt.ipc_c2_to_c1_rx = 0;
    tb->cnt.ipc_timeouts = 0;
    tb->cnt.cla_access_violations = 0;
    tb->cnt.state_transitions = 0;
}

/* lock-free SPSC: escreve na posição livre, publica com head++ */
void tlm_push_event(app_telemetry_block_t *tb, uint32_t t_ms, uint16_t code, uint16_t data) {
    uint32_t h = tb->ring.head;
    uint32_t t = tb->ring.tail;             // lido, mas só o leitor altera
    if ((h - t) >= APP_EVENT_RING_P2) {
        // buffer cheio -> descarta o mais antigo (avança tail)
        tb->ring.tail = t + 1u;
    }
    app_evt_t *slot = &tb->ring.buf[h & APP_EVENT_RING_MASK];
    slot->t_ms = t_ms;
    slot->code = code;
    slot->data = data;

    mem_barrier();               // garante que slot foi escrito antes de publicar
    tb->ring.head = h + 1u;      // publicar
}

/* Consumidor */
bool tlm_pop_event(app_telemetry_block_t const *tb, app_evt_t *out) {
    uint32_t t = tb->ring.tail;
    uint32_t h = tb->ring.head;
    if (t == h) return false;  // vazio
    *out = tb->ring.buf[t & APP_EVENT_RING_MASK];
    // ATENÇÃO: quem pode escrever tb->ring.tail é o leitor consumidor.
    ((app_ring_t *)&tb->ring)->tail = t + 1u;  // cast p/ remover const no campo
    return true;
}

uint32_t tlm_available(app_telemetry_block_t const *tb) {
    return (tb->ring.head - tb->ring.tail);
}
