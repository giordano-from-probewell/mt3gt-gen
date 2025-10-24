/**
 * @file ipc_handlers.c
 * @brief Project-specific IPC command handlers implementation
 * @version 1.0
 * @date 2025-10-24
 * @author MT3GT Development Team
 * 
 * This file implements project-specific IPC command handlers for dual-CPU
 * communication. Each CPU has dedicated handlers for received commands.
 * 
 * @note Modify this file for different projects while keeping ipc_comm.c/h unchanged
 */

#include "ipc_handlers.h"

#ifdef CPU1
#include "generation_sm.h"
#endif

#ifdef CPU2
#include "buzzer.h"
#include "cli.h"
#endif

// ============================================================================
// PROJECT-SPECIFIC VARIABLES
// ============================================================================

/** @brief Test flag to start system test */
bool flag_start_test = false;

/** @brief Test flag to reset metrics */
bool flag_reset_metrics = false;

/** @brief Calibration flag for new calibration request */
bool flag_new_calibration = false;

/** @brief New calibration parameter value */
int8_t new_cal_parameter = 0;

/** @brief New scale value for generation */
float32_t new_scale = 0.0f;

// ============================================================================
// CPU1-SPECIFIC HANDLERS (Receives commands from CPU2)
// ============================================================================

#ifdef CPU1

/**
 * @brief Handle generation commands from CPU2
 * @param payload Command payload containing channel and/or scale data
 * @param len Payload length in bytes
 * @return true if command processed successfully, false otherwise
 * 
 * @details This handler processes generation control commands:
 *          - SET_SCALE (5 bytes): Channel + 4-byte float scale value
 *          - ENABLE/DISABLE/CHANGE_WAVEFORM (1 byte): Sub-command ID
 * 
 * @note For SET_SCALE: payload[0] = channel (0=voltage, 1=current)
 *                      payload[1-4] = float scale value
 */
bool handle_generation_scale_command_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;
    uint8_t channel = payload[0];
    if (len == 5) {

        float scale;
        memcpy(&scale, &payload[1], sizeof(float));
        
        if (channel == 0) {
            gen_sm_request_scale(&app.generation.voltage, scale);
        } else if (channel == 1) {
            gen_sm_request_scale(&app.generation.current, scale);
        }
        return true;
    }
    
    return false;
}

bool handle_generation_wf_command_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;
    uint8_t channel = payload[0];
 //todo: make it
    return false;
}

bool handle_generation_start_command_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t channel = payload[0];
    switch (channel) {
        case 0:
            gen_sm_request_enable(&app.generation.voltage);
            break;
        case 1:
            gen_sm_request_enable(&app.generation.current);
            break;
        default:
            return false;
    }
    return true;
}


bool handle_generation_stop_command_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t channel = payload[0];
    switch (channel) {
        case 0:
            gen_sm_request_disable(&app.generation.voltage);
            break;
        case 1:
            gen_sm_request_disable(&app.generation.current);
            break;
        default:
            return false;
    }
    return true;
}

#endif // CPU1

// ============================================================================
// CPU2-SPECIFIC HANDLERS (Receives commands from CPU1)
// ============================================================================

#ifdef CPU2

/**
 * @brief Handle status updates from CPU1
 * @param payload Status data from CPU1
 * @param len Payload length (expected: 6 bytes)
 * @return true if status processed successfully, false otherwise
 * 
 * @details Expected payload format:
 *          - payload[0]: CPU1 state
 *          - payload[1]: Generation active flag
 *          - payload[2]: Scale percentage (0-100)
 *          - payload[3-4]: Reserved
 *          - payload[5]: XOR checksum of bytes 0-4
 */
bool handle_status_from_cpu1_cpu2(const uint8_t* payload, uint8_t len) {
    if (len < 6) return false;

    uint8_t cpu1_state = payload[0];
    uint8_t generation_active = payload[1];
    uint8_t scale_percent = payload[2];

    uint8_t checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum ^= payload[i];
    }

    if (checksum != payload[5]) {
        return false;
    }

    CLI_LOGI("CPU1 Status: State=%d, Gen=%d, Scale=%d%%",
             cpu1_state, generation_active, scale_percent);

    return true;
}

/**
 * @brief Handle control commands from CPU1
 * @param payload Control command data
 * @param len Payload length (minimum: 1 byte)
 * @return true if command processed successfully, false otherwise
 * 
 * @details Supported commands:
 *          - cmd=0: Reset system
 *          - cmd=1: Change mode (requires payload[1] = new_mode)
 */
bool handle_control_from_cpu1_cpu2(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t cmd = payload[0];

    switch (cmd) {
        case 0:
            CLI_LOGI("Reset command from CPU1");
            break;
        case 1:
            if (len >= 2) {
                uint8_t new_mode = payload[1];
                CLI_LOGI("Mode change to %d from CPU1", new_mode);
            }
            break;
        default:
            return false;
    }
    return true;
}

/**
 * @brief Custom buzzer command handler
 * @param payload Buzzer pattern data
 * @param len Payload length (expected: 1 byte)
 * @return true if pattern processed successfully, false otherwise
 * 
 * @details Plays predefined audio patterns based on pattern ID.
 *          Supports patterns 0-12 for various system events.
 */
bool handle_buzzer_command_cpu2(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t pattern = payload[0];

    CLI_LOGI("Buzzer play pattern %d from CPU1", pattern);

    const note_t* song = NULL;
    switch (pattern) {
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

    if (song) {
        buzzer_enqueue(song);
    }

    return true;
}

#endif // CPU2

// ============================================================================
// HANDLER INITIALIZATION
// ============================================================================

/**
 * @brief Initialize IPC handlers for CPU1
 * 
 * @details Registers command handlers for commands that CPU1 receives from CPU2.
 *          Includes generation control commands with proper payload lengths.
 */
void ipc_handlers_init_cpu1(void) {
#ifdef CPU1
    ipc_register_handler(IPC_CMD_GEN_ENABLE, "GEN_CMD", 1, handle_generation_start_command_cpu1);
    ipc_register_handler(IPC_CMD_GEN_DISABLE, "GEN_CMD", 1, handle_generation_stop_command_cpu1);
    ipc_register_handler(IPC_CMD_GEN_SET_SCALE, "GEN_CMD", 5, handle_generation_scale_command_cpu1);
    ipc_register_handler(IPC_CMD_GEN_CHANGE_WAVEFORM, "GEN_CMD", 1, handle_generation_wf_command_cpu1);
#endif
}

/**
 * @brief Initialize IPC handlers for CPU2
 * 
 * @details Registers command handlers for commands that CPU2 receives from CPU1.
 *          Includes custom buzzer handler and status/control command handlers.
 */
void ipc_handlers_init_cpu2(void) {
#ifdef CPU2
    ipc_register_handler(IPC_CMD_BUZZER_PLAY, "CUSTOM_BUZZER", 1, handle_buzzer_command_cpu2);
    ipc_register_handler(IPC_CMD_STATUS_REQUEST, "STATUS_FROM_CPU1", 0, handle_status_from_cpu1_cpu2);
    ipc_register_handler(IPC_CMD_RESET, "CONTROL_FROM_CPU1", 0, handle_control_from_cpu1_cpu2);
#endif
}

// ============================================================================
// PROJECT-SPECIFIC CATEGORY SENDING FUNCTIONS
// ============================================================================

/**
 * @brief Send audio/buzzer commands
 * @param cmd Audio command ID
 * @param payload Command payload data
 * @param len Payload length in bytes
 * @return true if command sent successfully, false otherwise
 */
bool ipc_send_audio_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_raw_cmd(cmd, payload, len);
}

/**
 * @brief Send test commands
 * @param cmd Test command ID
 * @param payload Command payload data
 * @param len Payload length in bytes
 * @return true if command sent successfully, false otherwise
 */
bool ipc_send_test_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_raw_cmd(cmd, payload, len);
}

/**
 * @brief Send calibration commands
 * @param cmd Calibration command ID
 * @param payload Command payload data
 * @param len Payload length in bytes
 * @return true if command sent successfully, false otherwise
 */
bool ipc_send_calibration_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_raw_cmd(cmd, payload, len);
}

/**
 * @brief Send generation commands
 * @param cmd Generation command ID
 * @param payload Command payload data
 * @param len Payload length in bytes
 * @return true if command sent successfully, false otherwise
 */
bool ipc_send_generation_cmd(uint8_t cmd, const void* payload, uint8_t len) {
    return ipc_send_raw_cmd(cmd, payload, len);
}
