#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
#include "ipc.h"
#include "buzzer.h"

// Doorbell flags (set/ack via IPC)
#ifndef IPC_FLAG_C1_TO_C2
#define IPC_FLAG_C1_TO_C2   IPC_FLAG1
#endif
#ifndef IPC_FLAG_C2_TO_C1
#define IPC_FLAG_C2_TO_C1   IPC_FLAG2
#endif

// 10-byte packet
typedef struct  ipc_pkt_st {
    uint8_t  seq;
    uint8_t  ack;
    uint8_t  cmd;
    uint8_t  len;        // 0..6
    uint8_t  payload[6]; // total struct = 10 bytes
} ipc_pkt_t;

// Simple command set (extend as needed)
typedef enum {
    IPC_CMD_NOP          = 0x00,
    IPC_CMD_BUZZER_PLAY  = 0x01,   // payload[0] = pattern id (example)
} ipc_cmd_t;

// Mailboxes in MSGRAM (one per direction)
typedef struct {
    volatile ipc_pkt_t c1_to_c2;
    volatile ipc_pkt_t c2_to_c1;
} ipc_mailboxes_t;

// --- API ---
// Call once at boot on each CPU:
void ipc_simple_init_cpu1(void);
void ipc_simple_init_cpu2(void);

// Tenta ler 1 mensagem de CPU2 (nÃ£o bloqueia). Retorna true se consumiu uma nova.
bool ipc_try_receive_from_cpu2(uint8_t* out_cmd,
                               uint8_t* out_payload,  // buffer >= 6 bytes (pode ser NULL)
                               uint8_t* out_len);

// VersÃ£o â€œdirigida por callbackâ€�: chama ipc_on_cmd_from_cpu2(cmd,payload,len) se houver msg nova.
void ipc_rx_service_cpu1(void);

// Handler de aplicaÃ§Ã£o (CPU1) â€“ implemente no seu app (ou use stub fraco do .c)
void ipc_on_cmd_from_cpu2(uint8_t cmd, const uint8_t* payload, uint8_t len);

// CPUy to CPUx: non-blocking send (returns false if len>6)
bool ipc_send_to_cpu2(uint8_t cmd, const void* payload, uint8_t len);
bool ipc_send_to_cpu1(uint8_t cmd, const void* payload, uint8_t len);

// CPU1: optional quick check if last CPU2 ack matches (non-blocking)
bool ipc_cpu2_ack_received(void);

// CPU2 ISR handler (wire to INT_IPC_1 on CPU2)
__interrupt void ipc_cpu2_isr(void);

// Convenience: CPU1 tells CPU2 to play a buzzer pattern id
static inline bool ipc_buzzer_play(uint8_t * song)
{
    return ipc_send_to_cpu2(IPC_CMD_BUZZER_PLAY, song, 1);
}
