#include "ipc_simple.h"
#include "F2837xD_Ipc_drivers.h"
#include <string.h>
#include "buzzer.h"
#include "application.h"

// Place one instance of each mailbox in MSGRAM segments.
// Both CPUs compile this; linker maps to the correct shared RAMs.

#pragma DATA_SECTION(g_c1_to_c2, "MSGRAM_CPU1_TO_CPU2")
volatile ipc_pkt_t g_c1_to_c2;
#pragma DATA_SECTION(g_c2_to_c1, "MSGRAM_CPU2_TO_CPU1")
volatile ipc_pkt_t g_c2_to_c1;

static volatile uint8_t s_cpu1_next_seq = 0;  // CPU1 toggles when sending
static volatile uint8_t s_cpu1_last_ack = 0;  // mirrored by CPU2


// ---------------- CPU1 side ----------------
#if defined(CPU1)

void ipc_simple_init_cpu1(void)
{
    // Clear flags from previous boots
    IPC_clearFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_ALL);

    // Zero mailboxes
    memset((void*)&g_c1_to_c2, 0, sizeof(g_c1_to_c2));
    memset((void*)&g_c2_to_c1, 0, sizeof(g_c2_to_c1));

    s_cpu1_next_seq = 1;
    s_cpu1_last_ack = 0;

    // CPU1 will just SEND and (optionally) poll ack. No ISR needed on CPU1 unless you want it.
    // If you want an interrupt when CPU2 acks (FLAG2), register INT_IPC_1 here on CPU1, too.
}

bool ipc_send_to_cpu2(uint8_t cmd, const void* payload, uint8_t len)
{
    uint8_t i;
    if (len > 6U) return false;

    //busy
    if (IPC_isFlagBusyLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2)) {
        return false;
    }


    // Fill packet (write data first, then seq)
    g_c1_to_c2.cmd = cmd;
    g_c1_to_c2.len = len;
    if (len) {
        const uint8_t* p = (const uint8_t*)payload;
        for (i = 0; i < len; ++i) g_c1_to_c2.payload[i] = p[i];
    }
    // optional: zero unused payload to ease debug
    for (i = len; i < 6U; ++i) g_c1_to_c2.payload[i] = 0;

    g_c1_to_c2.seq = s_cpu1_next_seq;  // publish message

    // Ring doorbell to CPU2
    IPC_setFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2);

    // Prepare next seq toggle
    s_cpu1_next_seq += 1;
    return true;
}

bool ipc_cpu2_ack_received(void)
{
    // CPU2 mirrors 'ack = seq' after processing
    uint8_t last_sent = (uint8_t)(s_cpu1_next_seq - 1U);
    bool ret = (g_c2_to_c1.ack == last_sent);
    return ret;
}




// Stub fraco: se vocÃª fornecer uma definiÃ§Ã£o no seu app, o linker usarÃ¡ a sua.
#if defined(__TI_COMPILER_VERSION__)
#pragma WEAK(ipc_on_cmd_from_cpu2)
#endif
__attribute__((weak)) void ipc_on_cmd_from_cpu2(uint8_t cmd, const uint8_t* payload, uint8_t len)
{
    (void)cmd; (void)payload; (void)len; // default: nada
}


// Estado local de recepÃ§Ã£o na CPU1
static uint8_t s_cpu1_last_seq = 0;
static bool    s_cpu1_has_last = false;


// LÃª uma nova mensagem (se houver). NÃ£o bloqueia.
bool ipc_try_receive_from_cpu2(uint8_t* out_cmd,
                               uint8_t* out_payload,
                               uint8_t* out_len)
{
    int8_t i;
    // Algum flag ativo vindo da CPU2? (do ponto de vista da CPU1 Ã© R->L)
    if (!IPC_isFlagBusyRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1)) {
        return false; // nada novo
    }

    uint8_t seq = g_c2_to_c1.seq;

    // Evita reprocessar a mesma mensagem se o flag ainda nÃ£o foi limpo
    if (s_cpu1_has_last && seq == s_cpu1_last_seq) {
        // JÃ¡ consumimos esta; ainda assim faÃ§a ACK do flag para liberar a outra CPU
        IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);
        return false;
    }

    // Snapshot do pacote
    uint8_t cmd = g_c2_to_c1.cmd;
    uint8_t len = g_c2_to_c1.len;
    if (len > 6u) len = 6u;

    if (out_cmd) *out_cmd = cmd;
    if (out_len) *out_len = len;
    if (out_payload) {
        for (i = 0; i < len; ++i) out_payload[i] = g_c2_to_c1.payload[i];
    }

    // Marca consumido e escreve ACK em MSGRAM que a CPU2 consegue ler
    s_cpu1_last_seq = seq;
    s_cpu1_has_last = true;
    g_c1_to_c2.ack  = seq;

    // Receptor: dÃ¡ ACK no flag recebido (R->L). (Quem limpa Ã© o emissor.)
    IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);

    return true;
}

// ServiÃ§o â€œdirigido por callbackâ€�: chama o handler se houver msg nova.
void ipc_rx_service_cpu1(void)
{
    uint8_t cmd, len, pl[6];
    if (ipc_try_receive_from_cpu2(&cmd, pl, &len)) {
        ipc_on_cmd_from_cpu2(cmd, pl, len);


        if(cmd == IPC_CMD_SET_CURRENT_RANGE)
        {
            flag_new_range = true;
            new_range = pl[0];

        }

        if(cmd == IPC_CMD_SET_CAL)
        {
            flag_new_calibration = true;
            new_cal_parameter = pl[0];
            new_cal_phase = pl[1];
            new_cal_index = pl[2];

        }

        if(cmd == IPC_CMD_RESET_TEST)
        {
            flag_reset_test = true;

        }

        if(cmd == IPC_CMD_START_TEST)
        {
            flag_start_test = true;

        }
    }
}




// ---------------- CPU2 side ----------------

#elif defined(CPU2)



void ipc_simple_init_cpu2(void)
{

    Interrupt_register(INT_IPC_1, &ipc_cpu2_isr);
    Interrupt_enable(INT_IPC_1);

    IPC_clearFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);
    IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);

    // Zera as duas mailboxes na MSGRAM compartilhada
    memset((void*)&g_c1_to_c2, 0, sizeof(g_c1_to_c2));
    memset((void*)&g_c2_to_c1, 0, sizeof(g_c2_to_c1));
}


static volatile uint8_t s_cpu2_next_seq = 1;

bool ipc_send_to_cpu1(uint8_t cmd, const void* payload, uint8_t len)
{
    uint8_t i;
    if (len > 6U) return false;

    // CPU2->CPU1 usa o doorbell IPC_FLAG_C2_TO_C1 na direï¿½ï¿½o L->R (a partir da CPU2)
    if (IPC_isFlagBusyLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1)) {
        return false; // ainda pendente; tente de novo depois
    }

    // Preenche o pacote (dados primeiro, depois o seq)
    g_c2_to_c1.cmd = cmd;
    g_c2_to_c1.len = len;

    if (len) {
        const uint8_t* p = (const uint8_t*)payload;
        for (i = 0; i < len; ++i) g_c2_to_c1.payload[i] = p[i];
    }
    for (i = len; i < 6U; ++i) g_c2_to_c1.payload[i] = 0;

    g_c2_to_c1.seq = s_cpu2_next_seq; // publica mensagem

    // Toca a campainha para a CPU1
    IPC_setFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1);

    s_cpu2_next_seq += 1;
    return true;
}

// Hook this ISR to INT_IPC_1 on CPU2
__interrupt void ipc_cpu2_isr(void)
{
    uint8_t i;
    // Check/clear the doorbell from CPU1
    if (IPC_isFlagBusyRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2)) {
        // New message?
        uint8_t seq = g_c1_to_c2.seq;
        if (seq != g_c1_to_c2.ack) {
            // Snapshot fields
            uint8_t cmd = g_c1_to_c2.cmd;
            uint8_t len = g_c1_to_c2.len;
            uint8_t pl[6];
            for (i = 0; i < 6; ++i) pl[i] = g_c1_to_c2.payload[i];

            // --- dispatch ---
            switch (cmd) {
                case IPC_CMD_BUZZER_PLAY:
                    if (len >= 1)
                    {
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
                        buzzer_enqueue(song);
                    }
                    break;
                case IPC_CMD_NOP:
                default:
                    break;
            }
            // ACK by mirroring seq
            g_c2_to_c1.ack = seq;

            // Optionally ring back to CPU1 to say "processed"
            IPC_setFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1);
        }

        // Ack and clear the incoming flag
        IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
        IPC_clearFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
    }

    // PIE ack
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

#else
#  error "Macro CPU1 or CPU2 undefined!"
#endif
