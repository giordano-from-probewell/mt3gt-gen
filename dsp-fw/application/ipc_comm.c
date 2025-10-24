#include "ipc_comm.h"
#include "F2837xD_Ipc_drivers.h"
#include <string.h>
// NOTE: Project-specific includes should be in ipc_handlers.c

// ============================================================================
// INTERNAL DATA STRUCTURES
// ============================================================================

// Statistics and diagnostics
typedef struct {
    uint32_t messages_sent;
    uint32_t messages_received;
    uint32_t communication_errors;
    uint32_t handler_errors;
    bool     communication_ok;
} ipc_stats_t;

// IPC state management
typedef struct {
    uint8_t         next_seq;
    uint8_t         last_ack;
    uint8_t         last_received_seq;
    bool            has_last_received;
    ipc_stats_t     stats;
} ipc_state_t;

// ============================================================================
// SHARED MEMORY SECTIONS
// ============================================================================

// Place mailboxes in MSGRAM segments
#pragma DATA_SECTION(g_ipc_c1_to_c2, "MSGRAM_CPU1_TO_CPU2")
volatile ipc_packet_t g_ipc_c1_to_c2;

#pragma DATA_SECTION(g_ipc_c2_to_c1, "MSGRAM_CPU2_TO_CPU1")
volatile ipc_packet_t g_ipc_c2_to_c1;

// ============================================================================
// LOCAL STATE VARIABLES
// ============================================================================

#ifdef CPU1
static ipc_state_t s_ipc_state = {0};
#endif

#ifdef CPU2
static ipc_state_t s_ipc_state = {0};
#endif

// Command handler registry (max 32 handlers)
#define MAX_COMMAND_HANDLERS 32
static ipc_command_handler_t s_command_handlers[MAX_COMMAND_HANDLERS];
static uint8_t s_num_handlers = 0;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static bool ipc_send_packet(uint8_t cmd, const void* payload, uint8_t len);
static bool ipc_receive_packet(uint8_t* out_cmd, uint8_t* out_payload, uint8_t* out_len);
static void ipc_dispatch_command(uint8_t cmd, const uint8_t* payload, uint8_t len);
static const char* ipc_get_command_name(uint8_t cmd);

// ============================================================================
// DEFAULT COMMAND HANDLERS
// ============================================================================

// System command handlers
static bool ipc_handle_ping(const uint8_t* payload, uint8_t len) {
    (void)payload; (void)len;
    // Could send a pong response back
    return true;
}

static bool ipc_handle_nop(const uint8_t* payload, uint8_t len) {
    (void)payload; (void)len;
    return true;
}

// Health check handler
static bool ipc_handle_health_check(const uint8_t* payload, uint8_t len) {
    (void)payload; (void)len;
    // System is responsive if we can handle this
    return true;
}

// Statistics request handler
static bool ipc_handle_get_stats(const uint8_t* payload, uint8_t len) {
    (void)payload; (void)len;
    // Could send back statistics via IPC if needed
    return true;
}

// NOTE: Project-specific handlers are implemented in ipc_handlers.c
// This keeps the base IPC system clean and reusable

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

static void ipc_register_default_handlers(void) {
    // Register only essential system commands
    ipc_register_handler(IPC_CMD_NOP, "NOP", 0, ipc_handle_nop);
    ipc_register_handler(IPC_CMD_PING, "PING", 0, ipc_handle_ping);
    ipc_register_handler(IPC_CMD_HEALTH_CHECK, "HEALTH_CHECK", 0, ipc_handle_health_check);
    ipc_register_handler(IPC_CMD_GET_STATS, "GET_STATS", 0, ipc_handle_get_stats);
    
    // NOTE: Project-specific handlers are registered in ipc_handlers_init_cpu1/2()
}

void ipc_init_cpu1(void) {
    // Clear IPC flags from previous boots
    IPC_clearFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_ALL);
    
    // Zero shared memory
    memset((void*)&g_ipc_c1_to_c2, 0, sizeof(g_ipc_c1_to_c2));
    memset((void*)&g_ipc_c2_to_c1, 0, sizeof(g_ipc_c2_to_c1));
    
    // Initialize state
    s_ipc_state.next_seq = 1;
    s_ipc_state.last_ack = 0;
    s_ipc_state.last_received_seq = 0;
    s_ipc_state.has_last_received = false;
    memset(&s_ipc_state.stats, 0, sizeof(s_ipc_state.stats));
    s_ipc_state.stats.communication_ok = true;
    
    // Register default command handlers
    ipc_register_default_handlers();
}

// ============================================================================
// ISR HANDLER (CPU2)
// ============================================================================

#ifdef CPU2
__interrupt void ipc_cpu2_isr(void) {
    // Process any incoming messages
    ipc_process_messages();

    // Clear interrupt
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}


void ipc_init_cpu2(void) {
    // Register and enable IPC interrupt
    Interrupt_register(INT_IPC_1, &ipc_cpu2_isr);
    Interrupt_enable(INT_IPC_1);
    
    // Clear IPC flags
    IPC_clearFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);
    IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_ALL);
    
    // Zero shared memory
    memset((void*)&g_ipc_c1_to_c2, 0, sizeof(g_ipc_c1_to_c2));
    memset((void*)&g_ipc_c2_to_c1, 0, sizeof(g_ipc_c2_to_c1));
    
    // Initialize state
    s_ipc_state.next_seq = 1;
    s_ipc_state.last_ack = 0;
    s_ipc_state.last_received_seq = 0;
    s_ipc_state.has_last_received = false;
    memset(&s_ipc_state.stats, 0, sizeof(s_ipc_state.stats));
    s_ipc_state.stats.communication_ok = true;
    
    // Register default command handlers  
    ipc_register_default_handlers();
}

#endif
// ============================================================================
// COMMAND HANDLER REGISTRATION
// ============================================================================

bool ipc_register_handler(uint8_t cmd_id, const char* name, uint8_t expected_len, ipc_handler_func_t handler) {
    if (s_num_handlers >= MAX_COMMAND_HANDLERS || handler == NULL) {
        return false;
    }
    
    // Check if handler already exists (replace it)
    for (uint8_t i = 0; i < s_num_handlers; i++) {
        if (s_command_handlers[i].cmd_id == cmd_id) {
            s_command_handlers[i].name = name;
            s_command_handlers[i].expected_len = expected_len;
            s_command_handlers[i].handler = handler;
            return true;
        }
    }
    
    // Add new handler
    s_command_handlers[s_num_handlers].cmd_id = cmd_id;
    s_command_handlers[s_num_handlers].name = name;
    s_command_handlers[s_num_handlers].expected_len = expected_len;
    s_command_handlers[s_num_handlers].handler = handler;
    s_num_handlers++;
    
    return true;
}

// ============================================================================
// SENDING FUNCTIONS
// ============================================================================

bool ipc_send_system_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_packet(cmd, payload, len);
}

// NOTE: Project-specific category functions are implemented in ipc_handlers.c
// This keeps the base IPC system reusable across different projects

bool ipc_send_raw_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_packet(cmd, payload, len);
}

static bool ipc_send_packet(uint8_t cmd, const void* payload, uint8_t len) {
    if (len > IPC_MAX_PAYLOAD_SIZE) {
        s_ipc_state.stats.communication_errors++;
        return false;
    }

#ifdef CPU1
    // Check if previous message is still being processed
    if (IPC_isFlagBusyLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2)) {
        s_ipc_state.stats.communication_errors++;
        return false;
    }
    
    // Fill packet
    g_ipc_c1_to_c2.cmd = cmd;
    g_ipc_c1_to_c2.len = len;
    
    if (len > 0 && payload != NULL) {
        const uint8_t* p = (const uint8_t*)payload;
        for (uint8_t i = 0; i < len; i++) {
            g_ipc_c1_to_c2.payload[i] = p[i];
        }
    }
    
    // Zero unused payload for cleaner debugging
    for (uint8_t i = len; i < IPC_MAX_PAYLOAD_SIZE; i++) {
        g_ipc_c1_to_c2.payload[i] = 0;
    }
    
    // Set sequence number (this publishes the message)
    g_ipc_c1_to_c2.seq = s_ipc_state.next_seq;
    
    // Ring doorbell to CPU2
    IPC_setFlagLtoR(IPC_CPU1_L_CPU2_R, IPC_FLAG_C1_TO_C2);
    
    // Prepare next sequence number
    s_ipc_state.next_seq++;
    s_ipc_state.stats.messages_sent++;
    
#elif defined(CPU2)
    // Check if previous message is still being processed  
    if (IPC_isFlagBusyLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1)) {
        s_ipc_state.stats.communication_errors++;
        return false;
    }
    
    // Fill packet
    g_ipc_c2_to_c1.cmd = cmd;
    g_ipc_c2_to_c1.len = len;
    
    if (len > 0 && payload != NULL) {
        const uint8_t* p = (const uint8_t*)payload;
        for (uint8_t i = 0; i < len; i++) {
            g_ipc_c2_to_c1.payload[i] = p[i];
        }
    }
    
    // Zero unused payload
    for (uint8_t i = len; i < IPC_MAX_PAYLOAD_SIZE; i++) {
        g_ipc_c2_to_c1.payload[i] = 0;
    }
    
    // Set sequence number
    g_ipc_c2_to_c1.seq = s_ipc_state.next_seq;
    
    // Ring doorbell to CPU1
    IPC_setFlagLtoR(IPC_CPU2_L_CPU1_R, IPC_FLAG_C2_TO_C1);
    
    // Prepare next sequence number
    s_ipc_state.next_seq++;
    s_ipc_state.stats.messages_sent++;
#endif
    
    return true;
}

// ============================================================================
// RECEIVING AND PROCESSING FUNCTIONS
// ============================================================================

void ipc_process_messages(void) {
    uint8_t cmd, len, payload[IPC_MAX_PAYLOAD_SIZE];
    
    while (ipc_receive_packet(&cmd, payload, &len)) {
        ipc_dispatch_command(cmd, payload, len);
    }
}

static bool ipc_receive_packet(uint8_t* out_cmd, uint8_t* out_payload, uint8_t* out_len) {
#ifdef CPU1
    // Check for incoming message from CPU2
    if (!IPC_isFlagBusyRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1)) {
        return false; // No new message
    }
    
    uint8_t seq = g_ipc_c2_to_c1.seq;
    
    // Avoid reprocessing the same message
    if (s_ipc_state.has_last_received && seq == s_ipc_state.last_received_seq) {
        // Already processed this message; ACK the flag to release CPU2
        IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);
        return false;
    }
    
    // Extract message
    *out_cmd = g_ipc_c2_to_c1.cmd;
    *out_len = g_ipc_c2_to_c1.len;
    if (*out_len > IPC_MAX_PAYLOAD_SIZE) *out_len = IPC_MAX_PAYLOAD_SIZE;
    
    for (uint8_t i = 0; i < *out_len; i++) {
        out_payload[i] = g_ipc_c2_to_c1.payload[i];
    }
    
    // Mark as processed and send ACK
    s_ipc_state.last_received_seq = seq;
    s_ipc_state.has_last_received = true;
    g_ipc_c1_to_c2.ack = seq;
    
    // ACK the flag
    IPC_ackFlagRtoL(IPC_CPU1_L_CPU2_R, IPC_FLAG_C2_TO_C1);
    
    s_ipc_state.stats.messages_received++;
    return true;
    
#elif defined(CPU2)
    // Similar implementation for CPU2 receiving from CPU1
    if (!IPC_isFlagBusyRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2)) {
        return false;
    }
    
    uint8_t seq = g_ipc_c1_to_c2.seq;
    
    if (s_ipc_state.has_last_received && seq == s_ipc_state.last_received_seq) {
        IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
        return false;
    }
    
    *out_cmd = g_ipc_c1_to_c2.cmd;
    *out_len = g_ipc_c1_to_c2.len;
    if (*out_len > IPC_MAX_PAYLOAD_SIZE) *out_len = IPC_MAX_PAYLOAD_SIZE;
    
    for (uint8_t i = 0; i < *out_len; i++) {
        out_payload[i] = g_ipc_c1_to_c2.payload[i];
    }
    
    s_ipc_state.last_received_seq = seq;
    s_ipc_state.has_last_received = true;
    g_ipc_c2_to_c1.ack = seq;
    
    IPC_ackFlagRtoL(IPC_CPU2_L_CPU1_R, IPC_FLAG_C1_TO_C2);
    
    s_ipc_state.stats.messages_received++;
    return true;
#else
    return false;
#endif
}

static void ipc_dispatch_command(uint8_t cmd, const uint8_t* payload, uint8_t len) {
    bool handled = false;
    
    // Find and execute handler
    for (uint8_t i = 0; i < s_num_handlers; i++) {
        if (s_command_handlers[i].cmd_id == cmd) {
            // Validate payload length if specified
            if (s_command_handlers[i].expected_len != 0 && 
                s_command_handlers[i].expected_len != len) {
                s_ipc_state.stats.handler_errors++;
                return;
            }
            
            // Execute handler
            if (!s_command_handlers[i].handler(payload, len)) {
                s_ipc_state.stats.handler_errors++;
            }
            handled = true;
            break;
        }
    }
    
    if (!handled) {
        // Unknown command
        s_ipc_state.stats.communication_errors++;
    }
}

// ============================================================================
// STATUS AND DIAGNOSTICS
// ============================================================================

bool ipc_is_communication_ok(void) {
    return s_ipc_state.stats.communication_ok;
}

uint32_t ipc_get_messages_sent(void) {
    return s_ipc_state.stats.messages_sent;
}

uint32_t ipc_get_messages_received(void) {
    return s_ipc_state.stats.messages_received;
}

uint32_t ipc_get_communication_errors(void) {
    return s_ipc_state.stats.communication_errors;
}

static const char* ipc_get_command_name(uint8_t cmd) {
    for (uint8_t i = 0; i < s_num_handlers; i++) {
        if (s_command_handlers[i].cmd_id == cmd) {
            return s_command_handlers[i].name;
        }
    }
    return "UNKNOWN";
}

