#ifndef __IPC_SIMPLE_H_
#define __IPC_SIMPLE_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
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
    uint8_t  payload[32]; // total struct = 10 bytes
} ipc_pkt_t;

// Simple command set
typedef enum {
    IPC_CMD_NOP          = 0x00,
    IPC_CMD_BUZZER_PLAY  = 0x01,   // payload[0] = pattern id
    // --- LOG ---
    // payload: [code_lo, code_hi, data_lo, data_hi, t16_lo, t16_hi]
    IPC_CMD_LOG_EVT      = 0x10,
} ipc_cmd_t;

#ifndef IPC_RING_P2
#define IPC_RING_P2   16u                 // 16 pacotes por direção
#endif
#define IPC_RING_MASK (IPC_RING_P2 - 1u)


typedef struct {
    volatile uint32_t head;               // producer write
    volatile uint32_t tail;               // consumer write
    ipc_pkt_t         buf[IPC_RING_P2];
} ipc_ring_t;


// Mailboxes in MSGRAM (one per direction)
typedef struct {
    volatile ipc_pkt_t c1_to_c2;
    volatile ipc_pkt_t c2_to_c1;
} ipc_mailboxes_t;

// --- API ---
// Call once at boot on each CPU:
void ipc_simple_init_cpu1(void);
void ipc_simple_init_cpu2(void);

//non blocking queue, return false if len>6 or full
bool ipc_enqueue_to_cpu2(uint8_t cmd, const void* payload, uint8_t len);
bool ipc_enqueue_to_cpu1(uint8_t cmd, const void* payload, uint8_t len);

//service
void ipc_service_cpu1(void);
void ipc_service_cpu2(void);


//CPU1: try to read msg from cpu2, non blocking
bool ipc_try_receive_from_cpu2(uint8_t* out_cmd,
                               uint8_t* out_payload,  // buffer >= 6 bytes (or NULL)
                               uint8_t* out_len);

// CPU1: callback if receives msg
void ipc_rx_service_cpu1(void);

// Handler (CPU1)
void ipc_on_cmd_from_cpu2(uint8_t cmd, const uint8_t* payload, uint8_t len);


// CPU2 ISR handler (wire to INT_IPC_1 on CPU2)
__interrupt void ipc_cpu2_isr(void);

// Convenience: CPU1 tells CPU2 to play a buzzer pattern id
static inline bool ipc_buzzer_play(uint8_t pattern_id)
{
    return ipc_enqueue_to_cpu2(IPC_CMD_BUZZER_PLAY, &pattern_id, 1);
}


//log form cpu1 to cpu2
static inline bool ipc_log_event_from_cpu1(uint16_t code, uint16_t data, uint16_t t16)
{
    uint8_t pl[6];
    pl[0] = (uint8_t)(code & 0xFF);
    pl[1] = (uint8_t)(code >> 8);
    pl[2] = (uint8_t)(data & 0xFF);
    pl[3] = (uint8_t)(data >> 8);
    pl[4] = (uint8_t)(t16  & 0xFF);
    pl[5] = (uint8_t)(t16  >> 8);
    return ipc_enqueue_to_cpu2(IPC_CMD_LOG_EVT, pl, 6);
}




#endif
