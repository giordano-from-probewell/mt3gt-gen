/**
 * @file ipc_comm.h
 * @brief Inter-Processor Communication System for TI C2000 Dual-Core DSP
 * @version 2.0
 * @date 2025-10-22
 * 
 * Enhanced IPC system with categorized commands, type-safe handlers,
 * automatic statistics, and reliable communication protocols.
 */

#ifndef _IPC_COMM_H
#define _IPC_COMM_H

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
#include "ipc.h"

// ============================================================================
// IPC COMMUNICATION SYSTEM
// ============================================================================
// ============================================================================

// Doorbell flags (set/ack via IPC)
#ifndef IPC_FLAG_C1_TO_C2
#define IPC_FLAG_C1_TO_C2   IPC_FLAG1
#endif
#ifndef IPC_FLAG_C2_TO_C1
#define IPC_FLAG_C2_TO_C1   IPC_FLAG2
#endif

// Maximum payload size for IPC messages
#define IPC_MAX_PAYLOAD_SIZE    6

// IPC packet structure (10 bytes total)
typedef struct ipc_packet_st {
    uint8_t  seq;                           // Sequence number for reliability
    uint8_t  ack;                           // Acknowledgment number
    uint8_t  cmd;                           // Command ID
    uint8_t  len;                           // Payload length (0..IPC_MAX_PAYLOAD_SIZE)
    uint8_t  payload[IPC_MAX_PAYLOAD_SIZE]; // Command payload data
} ipc_packet_t;

// ============================================================================
// COMMAND DEFINITIONS - Easy to extend
// ============================================================================

// Base command categories (upper nibble) - Add more as needed in ipc_handlers.h
#define IPC_CMD_CATEGORY_SYSTEM     0x00    // System commands (0x00-0x0F)

// Essential System Commands (0x00-0x0F) - Always available
typedef enum {
    IPC_CMD_NOP                 = 0x00,     // No operation
    IPC_CMD_PING                = 0x01,     // Ping/Pong for testing communication
    IPC_CMD_RESET               = 0x02,     // Request system reset
    IPC_CMD_STATUS_REQUEST      = 0x03,     // Request status information
    IPC_CMD_GET_STATS           = 0x04,     // Get communication statistics
    IPC_CMD_HEALTH_CHECK        = 0x05,     // Check system health
} ipc_system_cmd_t;

// NOTE: Project-specific commands are defined in ipc_handlers.h
// This keeps the base IPC system reusable across different projects

// ============================================================================
// COMMAND HANDLER DEFINITIONS - Type-safe handlers
// ============================================================================

// Forward declarations for command handlers
typedef struct ipc_command_handler_st ipc_command_handler_t;

// Command handler function prototype
typedef bool (*ipc_handler_func_t)(const uint8_t* payload, uint8_t len);

// Command handler structure
struct ipc_command_handler_st {
    uint8_t             cmd_id;         // Command ID
    const char*         name;           // Human-readable command name (for debugging)
    uint8_t             expected_len;   // Expected payload length (0 = any length)
    ipc_handler_func_t  handler;        // Handler function pointer
};

// ============================================================================
// IPC API - Improved interface
// ============================================================================

// Initialization functions
void ipc_init_cpu1(void);
void ipc_init_cpu2(void);

// High-level sending functions
bool ipc_send_system_cmd(uint8_t cmd, const void* payload, uint8_t len);

// Category-specific sending functions (implement in ipc_handlers.c for your project)
bool ipc_send_audio_cmd(uint8_t cmd, const void* payload, uint8_t len);
bool ipc_send_test_cmd(uint8_t cmd, const void* payload, uint8_t len);
bool ipc_send_calibration_cmd(uint8_t cmd, const void* payload, uint8_t len);
bool ipc_send_generation_cmd(uint8_t cmd, const void* payload, uint8_t len);

// Low-level sending function (for direct control)
bool ipc_send_raw_cmd(uint8_t cmd, const void* payload, uint8_t len);

// Reception and processing
void ipc_process_messages(void);
bool ipc_register_handler(uint8_t cmd_id, const char* name, uint8_t expected_len, ipc_handler_func_t handler);

// Status and diagnostics
bool ipc_is_communication_ok(void);
uint32_t ipc_get_messages_sent(void);
uint32_t ipc_get_messages_received(void);
uint32_t ipc_get_communication_errors(void);

// CPU2 ISR (remains the same)
__interrupt void ipc_cpu2_isr(void);

// ============================================================================
// ESSENTIAL SYSTEM MACROS
// ============================================================================

// Basic system commands (always available)
#define IPC_SEND_PING()                     ipc_send_system_cmd(IPC_CMD_PING, NULL, 0)
#define IPC_SEND_RESET()                    ipc_send_system_cmd(IPC_CMD_RESET, NULL, 0)
#define IPC_SEND_GET_STATS()                ipc_send_system_cmd(IPC_CMD_GET_STATS, NULL, 0)
#define IPC_SEND_HEALTH_CHECK()             ipc_send_system_cmd(IPC_CMD_HEALTH_CHECK, NULL, 0)

// Generic data sending macros
#define IPC_SEND_SIMPLE(cmd)                ipc_send_raw_cmd((cmd), NULL, 0)
#define IPC_SEND_BYTE(cmd, value)           do { uint8_t v = (value); ipc_send_raw_cmd((cmd), &v, 1); } while(0)
#define IPC_SEND_WORD(cmd, value)           do { uint16_t v = (value); ipc_send_raw_cmd((cmd), &v, 2); } while(0)
#define IPC_SEND_DWORD(cmd, value)          do { uint32_t v = (value); ipc_send_raw_cmd((cmd), &v, 4); } while(0)
#define IPC_SEND_FLOAT(cmd, value)          do { float v = (value); ipc_send_raw_cmd((cmd), &v, 4); } while(0)
#define IPC_SEND_BUFFER(cmd, buffer, len)   ipc_send_raw_cmd((cmd), (buffer), (len))

// NOTE: Project-specific macros are defined in ipc_handlers.h

// ============================================================================
// BACKWARD COMPATIBILITY (if needed)
// ============================================================================

// Legacy function names (can be removed after migration)
#define ipc_simple_init_cpu1()              ipc_init_cpu1()
#define ipc_simple_init_cpu2()              ipc_init_cpu2()
#define ipc_send_to_cpu2(cmd, payload, len) ipc_send_raw_cmd(cmd, payload, len)
#define ipc_send_to_cpu1(cmd, payload, len) ipc_send_raw_cmd(cmd, payload, len)

#endif // _IPC_COMM_H
