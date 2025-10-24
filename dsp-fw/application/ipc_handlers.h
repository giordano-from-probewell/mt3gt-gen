/**
 * @file ipc_handlers.h
 * @brief Project-specific IPC command handlers and definitions
 * @version 1.0
 * @date 2025-10-22
 * 
 * This file contains all project-specific IPC commands and handlers.
 * Modify this file for different projects while keeping ipc_comm.c/h unchanged.
 */

#ifndef _IPC_HANDLERS_H
#define _IPC_HANDLERS_H

#include "ipc_comm.h"
#include "application.h"

// ============================================================================
// PROJECT-SPECIFIC COMMAND DEFINITIONS
// ============================================================================

// Audio Commands (0x10-0x1F)
typedef enum {
    IPC_CMD_BUZZER_PLAY         = 0x10,     // Play buzzer pattern: payload[0] = pattern_id
    IPC_CMD_BUZZER_STOP         = 0x11,     // Stop buzzer
    IPC_CMD_BUZZER_VOLUME       = 0x12,     // Set buzzer volume: payload[0] = volume (0-100)
} ipc_audio_cmd_t;

// Generation Commands (0x40-0x4F)
typedef enum {
    IPC_CMD_GEN_ENABLE          = 0x40,     // Enable generation: payload[0] = channel_mask
    IPC_CMD_GEN_DISABLE         = 0x41,     // Disable generation: payload[0] = channel_mask
    IPC_CMD_GEN_SET_SCALE       = 0x42,     // Set scale: payload[0]=ch, payload[1-4]=float scale
    IPC_CMD_GEN_CHANGE_WAVEFORM = 0x43,     // Change waveform: payload[0] = channel
} ipc_generation_cmd_t;

// ============================================================================
// PROJECT-SPECIFIC CONVENIENCE MACROS
// ============================================================================

// Audio/Buzzer commands
#define IPC_SEND_BUZZER_PLAY(pattern)        ipc_send_audio_cmd(IPC_CMD_BUZZER_PLAY, &(uint8_t){pattern}, 1)
#define IPC_SEND_BUZZER_STOP()               ipc_send_audio_cmd(IPC_CMD_BUZZER_STOP, NULL, 0)
#define IPC_SEND_BUZZER_VOLUME(vol)          ipc_send_audio_cmd(IPC_CMD_BUZZER_VOLUME, &(uint8_t){vol}, 1)

// Test commands
#define IPC_SEND_TEST_START()                ipc_send_test_cmd(IPC_CMD_TEST_START, NULL, 0)
#define IPC_SEND_TEST_STOP()                 ipc_send_test_cmd(IPC_CMD_TEST_STOP, NULL, 0)
#define IPC_SEND_TEST_RESET()                ipc_send_test_cmd(IPC_CMD_TEST_RESET, NULL, 0)
#define IPC_SEND_TEST_SET_PARAM(param, val)  do { uint8_t data[] = {param, val}; ipc_send_test_cmd(IPC_CMD_TEST_SET_PARAM, data, 2); } while(0)

// Calibration commands
#define IPC_SEND_CAL_SET_RANGE(param, range) do { uint8_t data[] = {param, range}; ipc_send_calibration_cmd(IPC_CMD_CAL_SET_RANGE, data, 2); } while(0)
#define IPC_SEND_CAL_RESET()                 ipc_send_calibration_cmd(IPC_CMD_CAL_RESET, NULL, 0)
#define IPC_SEND_CAL_SAVE()                  ipc_send_calibration_cmd(IPC_CMD_CAL_SAVE, NULL, 0)

// Generation commands
#define IPC_SEND_GEN_ENABLE(ch)             ipc_send_generation_cmd(IPC_CMD_GEN_ENABLE, &(uint8_t){ch}, 1)
#define IPC_SEND_GEN_DISABLE(ch)            ipc_send_generation_cmd(IPC_CMD_GEN_DISABLE, &(uint8_t){ch}, 1)
#define IPC_SEND_GEN_CHANGE_WAVEFORM(ch)    ipc_send_generation_cmd(IPC_CMD_GEN_CHANGE_WAVEFORM, &(uint8_t){ch}, 1)

// Special macros for float data
#define IPC_SEND_GEN_SET_SCALE(ch, scale_val) do { \
    uint8_t payload[5]; \
    payload[0] = (ch); \
    memcpy(&payload[1], &(scale_val), sizeof(float)); \
    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5); \
} while(0)

// ============================================================================
// PROJECT-SPECIFIC VARIABLES (shared between handlers and application)
// ============================================================================

// Test flags (accessible by both CPUs)
extern bool flag_start_test;
extern bool flag_reset_metrics;
extern bool flag_new_calibration;

// Calibration variables (CPU1 specific)
extern int8_t new_cal_parameter;
extern float32_t new_scale;

// ============================================================================
// PROJECT-SPECIFIC HANDLER FUNCTIONS
// ============================================================================

// Initialize project-specific handlers
void ipc_handlers_init_cpu1(void);
void ipc_handlers_init_cpu2(void);

// CPU1-specific handler prototypes (handlers that run on CPU1)
#ifdef CPU1
bool handle_generation_start_command_cpu1(const uint8_t* payload, uint8_t len);
bool handle_generation_stop_command_cpu1(const uint8_t* payload, uint8_t len);
bool handle_generation_scale_command_cpu1(const uint8_t* payload, uint8_t len);
bool handle_generation_wf_command_cpu1(const uint8_t* payload, uint8_t len);

#endif

// CPU2-specific handler prototypes (handlers that run on CPU2) 
#ifdef CPU2
bool handle_buzzer_command_cpu2(const uint8_t* payload, uint8_t len);
bool handle_status_from_cpu1_cpu2(const uint8_t* payload, uint8_t len);
bool handle_control_from_cpu1_cpu2(const uint8_t* payload, uint8_t len);
#endif

#endif // _IPC_HANDLERS_H
