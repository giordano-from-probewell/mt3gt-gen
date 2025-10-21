#include "ipc_simple.h"
#include "my_time.h"
#include <string.h>
#include "buzzer.h"
#include "application.h"
#include <ipc.h>

// ---- Mailboxes in MSGRAM () ----
// both CPUs compile; linker maps RAMs.
#pragma DATA_SECTION(g_c1_to_c2, "MSGRAM_CPU1_TO_CPU2")
volatile ipc_pkt_t g_c1_to_c2;
#pragma DATA_SECTION(g_c2_to_c1, "MSGRAM_CPU2_TO_CPU1")
volatile ipc_pkt_t g_c2_to_c1;

// ---- Local ring for CPU (assync send) ----
#pragma DATA_SECTION(s_txq_c1_to_c2, "MSGRAM_CPU1_TO_CPU2")
volatile ipc_ring_t s_txq_c1_to_c2;   // CPU1
#pragma DATA_SECTION(s_txq_c2_to_c1, "MSGRAM_CPU2_TO_CPU1")
volatile ipc_ring_t s_txq_c2_to_c1;   // CPU2

// Seqs (producer updates)
static volatile uint8_t s_cpu1_next_seq = 1u; // CPU1 -> CPU2
static volatile uint8_t s_cpu2_next_seq = 1u; // CPU2 -> CPU1

// simple barrier (seq before publish)
static inline void mem_barrier(void) { __asm(" RPT #7 || NOP"); }

/* ---------------- common ring helpers ---------------- */
static inline void ring_reset(volatile ipc_ring_t* r) { r->head = r->tail = 0u; }

static bool ring_push(volatile ipc_ring_t* r, const ipc_pkt_t* p)
{
    uint32_t h = r->head, t = r->tail;
    if ((h - t) >= IPC_RING_P2) return false;  /* full */
    r->buf[h & IPC_RING_MASK] = *p;
    mem_barrier();
    r->head = h + 1u;
    return true;
}

static bool ring_pop(volatile ipc_ring_t* r, ipc_pkt_t* out)
{
    uint32_t t = r->tail, h = r->head;
    if (t == h) return false;
    *out = r->buf[t & IPC_RING_MASK];
    r->tail = t + 1u;
    return true;
}


// =========================== CPU1 ===========================
#if defined(CPU1)

//// Weak callback
//#if defined(__TI_COMPILER_VERSION__)
//#pragma WEAK(ipc_on_cmd_from_cpu2)
//#endif
//__attribute__((weak)) void ipc_on_cmd_from_cpu2(uint8_t cmd, const uint8_t* payload, uint8_t len)
//{
//    (void)cmd; (void)payload; (void)len;
//}


void ipc_simple_init_cpu1(void)
{
    // clean flags
    IPC_clearFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_ALL);
    IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_ALL);

    // clean mailboxes
    memset((void*)&g_c1_to_c2, 0, sizeof(g_c1_to_c2));
    memset((void*)&g_c2_to_c1, 0, sizeof(g_c2_to_c1));

    // clean ring and seq
    ring_reset(&s_txq_c1_to_c2);
    s_cpu1_next_seq = 1u;
}

// Enqueue (non blocking). If len>6 -> false.
bool ipc_enqueue_to_cpu2(uint8_t cmd, const void* payload, uint8_t len)
{
    ipc_pkt_t p;
    uint8_t i;

    if (len > 6u) return false;

    p.seq = 0; // fill at send
    p.ack = 0;
    p.cmd = cmd;
    p.len = len;
    for (i = 0; i < len; ++i) p.payload[i] = ((const uint8_t*)payload)[i];
    for (; i < 6u; ++i) p.payload[i] = 0;

    return ring_push(&s_txq_c1_to_c2, &p);
}

// Try to send first pkt to CPU2 (if doorbell is free)
void ipc_service_cpu1(void)
{
    uint8_t i;
    ipc_pkt_t p;

    // Doorbell busy? nothing to do.
    if (IPC_isFlagBusyLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2)) {
        return;
    }
    // Nada na fila? sai.
    if (!ring_pop(&s_txq_c1_to_c2, &p)) {
        return;
    }

    // Fill mailbox (data first, seq after)
    g_c1_to_c2.cmd = p.cmd;
    g_c1_to_c2.len = p.len;
    for (i = 0; i < 6u; ++i) g_c1_to_c2.payload[i] = p.payload[i];

    g_c1_to_c2.seq = s_cpu1_next_seq;
    mem_barrier();
    IPC_setFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2);

    s_cpu1_next_seq += 1u;
}

// RX CPU2->CPU1 (pooling mode)
static uint8_t s_cpu1_last_seq = 0;
static bool    s_cpu1_has_last = false;

bool ipc_try_receive_from_cpu2(uint8_t* out_cmd,
                               uint8_t* out_payload,
                               uint8_t* out_len)
{
    uint8_t i;

    if (!IPC_isFlagBusyRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1)) {
        return false;
    }

    uint8_t seq = g_c2_to_c1.seq;
    if (s_cpu1_has_last && seq == s_cpu1_last_seq) {
        IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);
        return false; /* already consumed */
    }

    uint8_t cmd = g_c2_to_c1.cmd;
    uint8_t len = g_c2_to_c1.len;
    if (len > 6u) len = 6u;

    if (out_cmd) *out_cmd = cmd;
    if (out_len) *out_len = len;
    if (out_payload) {
        for (i = 0; i < len; ++i) out_payload[i] = g_c2_to_c1.payload[i];
    }

    s_cpu1_last_seq = seq;
    s_cpu1_has_last = true;
    g_c1_to_c2.ack  = seq;

    IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);
    return true;
}

void ipc_rx_service_cpu1(void)
{
    uint8_t cmd, len, pl[6];
    if (ipc_try_receive_from_cpu2(&cmd, pl, &len)) {
        // seu app trata aqui (callback fraco por padrÃ£o)
        ipc_on_cmd_from_cpu2(cmd, pl, len);
    }
}

// =========================== CPU2 ===========================
#elif defined(CPU2)

#include "interrupt.h"

/* Provide your own sinks in app (no 'extern' on definitions!) */
extern void cli_log_info(const char* s);
extern void cli_log_info1(const char* s);


void ipc_simple_init_cpu2(void)
{
    /* bind ISR via driverlib or PIE (your choice) */
    Interrupt_register(INT_IPC_1, &ipc_cpu2_isr);
    Interrupt_enable(INT_IPC_1);

    IPC_clearFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);
    IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);

    memset((void*)&g_c1_to_c2, 0, sizeof(g_c1_to_c2));
    memset((void*)&g_c2_to_c1, 0, sizeof(g_c2_to_c1));

    ring_reset(&s_txq_c2_to_c1);
    s_cpu2_next_seq = 1u;
}

/* CPU2 can also enqueue to CPU1 (for commands) */
bool ipc_enqueue_to_cpu1(uint8_t cmd, const void* payload, uint8_t len)
{
    ipc_pkt_t p;
    uint8_t i;
    if (len > 6u) return false;
    p.seq = 0; p.ack = 0; p.cmd = cmd; p.len = len;
    for (i = 0; i < len; ++i) p.payload[i] = ((const uint8_t*)payload)[i];
    for (; i < 6u; ++i) p.payload[i] = 0;
    return ring_push(&s_txq_c2_to_c1, &p);
}

/* CPU2 drains its queue to CPU1 */
void ipc_service_cpu2(void)
{
    uint8_t i;
    ipc_pkt_t p;
    if (IPC_isFlagBusyLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1)) {
        return;
    }
    if (!ring_pop(&s_txq_c2_to_c1, &p)) {
        return;
    }
    g_c2_to_c1.cmd = p.cmd;
    g_c2_to_c1.len = p.len;
    for (i = 0; i < 6u; ++i) g_c2_to_c1.payload[i] = p.payload[i];
    g_c2_to_c1.seq = s_cpu2_next_seq;
    mem_barrier();
    IPC_setFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1);
    s_cpu2_next_seq += 1u;


    char line[64];
    int n = 0;
    n += snprintf(&line[n], sizeof(line)-n, "%lu ", (unsigned long)0);
    n += snprintf(&line[n], sizeof(line)-n, "IPC_TX ");
    n += snprintf(&line[n], sizeof(line)-n, "d=0x%04X", (unsigned)p.cmd);

    cli_log_info1(line);
}

/* ISR (CPU2) – receive from CPU1 and log/dispatch */
__interrupt void ipc_cpu2_isr(void)
{
    uint8_t i;
    if (IPC_isFlagBusyRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2)) {

        uint8_t seq = g_c1_to_c2.seq;
        if (seq != g_c1_to_c2.ack) {
            // Snapshot
            uint8_t cmd = g_c1_to_c2.cmd;
            uint8_t len = g_c1_to_c2.len;
            uint8_t pl[6];
            for (i = 0; i < 6u; ++i) pl[i] = g_c1_to_c2.payload[i];

            // --- Dispatch / Log ---
            switch (cmd) {
                case IPC_CMD_LOG_EVT: {
                    uint16_t code = (uint16_t)(pl[0] | ((uint16_t)pl[1] << 8));
                    uint16_t data = (uint16_t)(pl[2] | ((uint16_t)pl[3] << 8));
                    uint16_t t16  = (uint16_t)(pl[4] | ((uint16_t)pl[5] << 8));
                    // unwrap simples 16->32
                    static uint32_t last_t32 = 0;
                    uint32_t base = last_t32 & 0xFFFF0000u;
                    uint32_t t32  = base | t16;
                    if ((t32 - last_t32) > 0x8000u) { t32 = (base + 0x10000u) | t16; }
                    last_t32 = t32;

                    // send ao CLI
                    if (cli_log_info) {
                        char line[64];
                        int n = 0;
                        n += snprintf(&line[n], sizeof(line)-n, "%lu ", (unsigned long)t32);
                        switch (code) {
                            case 100: n += snprintf(&line[n], sizeof(line)-n, "IPC_TX "); break;
                            case 101: n += snprintf(&line[n], sizeof(line)-n, "IPC_RX "); break;
                            case 200: n += snprintf(&line[n], sizeof(line)-n, "STATE  "); break;
                            default:  n += snprintf(&line[n], sizeof(line)-n, "EVT%u ", (unsigned)code); break;
                        }
                        n += snprintf(&line[n], sizeof(line)-n, "d=0x%04X", (unsigned)data);
                        cli_log_info(line);
                    }
                    break;
                }
                case IPC_CMD_BUZZER_PLAY: {
                    if (len >= 1) {

                        const note_t* song = NULL;
                        switch (pl[0])
                        {
                        case 0: song = comm_ok; break;
                        case 1: song = comm_error; break;
                        case 2: song = warning_beep; break;
                        case 3: song = event_beep; break;
                        case 4: song = event_led_beep; break;
                        case 5: song = boot_ok; break;
                        case 6: song = boot_fail; break;
                        case 7: song = std_boot_ok; break;
                        case 8: song = gen_a_boot_ok; break;
                        case 9: song = gen_b_boot_ok; break;
                        case 10: song = gen_c_boot_ok; break;
                        case 11: song = mario_theme; break;
                        case 12: song = star_wars; break;
                        default: song = event_beep; break;
                        }
                        extern void buzzer_enqueue(const note_t *song);
                        buzzer_enqueue(song);

                    }
                    break;
                }
                case IPC_CMD_NOP:
                default:
                    break;
            }

            /* mirror ACK and optionally ring back */
            g_c2_to_c1.ack = seq;
            IPC_setFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1);
        }

        /* clear received doorbell */
        IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
        IPC_clearFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
    }

    // PIE ack
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

#else
#  error "Need definition: CPU1 or CPU2!"
#endif
