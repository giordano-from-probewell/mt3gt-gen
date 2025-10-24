// ============================================================================
// IPC USAGE EXAMPLES - How to use the improved IPC system
// ============================================================================
// NOTE: This is a reference/example file. Some code is commented out to avoid
//       compilation errors. Adapt the examples to your specific application needs.

#include "ipc_comm.h"
#include "ipc_handlers.h"
// #include "application.h"  // Include your application header if needed

// ============================================================================
// EXAMPLE 1: SIMPLE COMMAND SENDING
// ============================================================================

void example_send_simple_commands(void) {
    // Send a ping command
    IPC_SEND_PING();
    
    // Play a buzzer pattern
    IPC_SEND_BUZZER_PLAY(5);  // Play boot_ok sound
    
    // Start a test
    IPC_SEND_TEST_START();
    
    // Set calibration range
    IPC_SEND_CAL_SET_RANGE(1, 50);  // parameter 1, range 50
    
    // Enable generation on channel 0
    IPC_SEND_GEN_ENABLE(0x01);  // Channel mask: bit 0 = channel 0
    
    // Change waveform on channel 0
    IPC_SEND_GEN_CHANGE_WAVEFORM(0);
}

// ============================================================================
// EXAMPLE 2: CUSTOM COMMAND HANDLERS (implement in your application)
// ============================================================================

// Custom handler for test start command
bool my_test_start_handler(const uint8_t* payload, uint8_t len) {
    (void)payload; (void)len;
    
    // Your custom test start logic here
    // For example:
    // - Set flags
    // - Initialize test parameters
    // - Start test sequence
    
    // Example implementation:
    extern bool flag_start_test;
    flag_start_test = true;
    
    return true;  // Return true if successful
}

// Custom handler for calibration range setting
bool my_cal_range_handler(const uint8_t* payload, uint8_t len) {
    if (len != 2) return false;  // Expect exactly 2 bytes
    
    uint8_t parameter = payload[0];
    uint8_t range = payload[1];
    
    // Your calibration logic here
    // For example:
    // extern bool flag_new_range;
    // extern int8_t new_range_parameter, new_range;
    
    // flag_new_range = true;
    // new_range_parameter = parameter;
    // new_range = range;
    
    return true;
}

// Custom handler for generation scale setting
bool my_gen_scale_handler(const uint8_t* payload, uint8_t len) {
    if (len != 5) return false;  // Expect 1 byte channel + 4 bytes float
    
    uint8_t channel = payload[0];
    
    // Convert 4 bytes back to float (assuming little-endian)
    float scale;
    memcpy(&scale, &payload[1], sizeof(float));
    
    // Your generation scale logic here
    if (channel == 0) {
        // Set voltage generation scale
        // gen_sm_request_scale(&app.generation.voltage, scale);
    } else if (channel == 1) {
        // Set current generation scale  
        // gen_sm_request_scale(&app.generation.current, scale);
    }
    
    return true;
}

// Custom handler for waveform change
bool my_gen_waveform_handler(const uint8_t* payload, uint8_t len) {
    if (len != 1) return false;
    
    uint8_t channel = payload[0];
    
    // Your waveform change logic here
    if (channel == 0) {
        // gen_sm_change_waveform_soft(&app.generation.voltage);
    } else if (channel == 1) {
        // gen_sm_change_waveform_soft(&app.generation.current);
    }
    
    return true;
}

// ============================================================================
// EXAMPLE 3: CUSTOM INITIALIZATION WITH HANDLERS
// ============================================================================

void my_ipc_init(void) {
    // Initialize IPC system
#ifdef CPU1
    ipc_init_cpu1();
#else
    ipc_init_cpu2();
#endif
    
    // Register custom handlers (override default weak implementations)
    ipc_register_handler(IPC_CMD_TEST_START, "MY_TEST_START", 0, my_test_start_handler);
    ipc_register_handler(IPC_CMD_CAL_SET_RANGE, "MY_CAL_RANGE", 2, my_cal_range_handler);
    ipc_register_handler(IPC_CMD_GEN_SET_SCALE, "MY_GEN_SCALE", 5, my_gen_scale_handler);
    ipc_register_handler(IPC_CMD_GEN_CHANGE_WAVEFORM, "MY_GEN_WAVEFORM", 1, my_gen_waveform_handler);
}

// ============================================================================
// EXAMPLE 4: MAIN LOOP INTEGRATION
// ============================================================================

void my_main_loop(void) {
    while (1) {
        // Process IPC messages (call this regularly)
        ipc_process_messages();
        
        // Your other main loop tasks...
        
        // Optional: Check IPC health
        if (!ipc_is_communication_ok()) {
            // Handle communication problems
        }
        
        // Optional: Log statistics periodically
        static uint32_t last_log_time = 0;
        uint32_t now = get_time_ms();  // Your time function
        if ((now - last_log_time) > 10000) {  // Every 10 seconds
            uint32_t sent = ipc_get_messages_sent();
            uint32_t received = ipc_get_messages_received();
            uint32_t errors = ipc_get_communication_errors();
            
            // Log or display statistics
            printf("IPC Stats: Sent=%lu, Received=%lu, Errors=%lu\\n", 
                   sent, received, errors);
            
            last_log_time = now;
        }
    }
}

// ============================================================================
// EXAMPLE 5: COMPLEX COMMANDS WITH STRUCTURED DATA
// ============================================================================

// Example: Send a complex calibration command with multiple parameters
void send_complex_calibration(uint8_t param_id, uint8_t phase, uint8_t index, float value) {
    uint8_t payload[8];  // 3 bytes + 4 bytes for float + 1 spare
    
    payload[0] = param_id;
    payload[1] = phase;
    payload[2] = index;
    
    // Pack float into bytes (little-endian)
    memcpy(&payload[3], &value, sizeof(float));
    
    // Send custom calibration command
    ipc_send_calibration_cmd(IPC_CMD_CAL_SET_VALUE, payload, 7);
}

// Example: Send generation scale as a float
void send_generation_scale(uint8_t channel, float scale) {
    uint8_t payload[5];  // 1 byte channel + 4 bytes float
    
    payload[0] = channel;
    memcpy(&payload[1], &scale, sizeof(float));
    
    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
}

// ============================================================================
// EXAMPLE 6: BACKWARD COMPATIBILITY
// ============================================================================

// If you have existing code using old IPC functions, you can still use them:
void example_backward_compatibility(void) {
    // Old style (still works)
    uint8_t buzzer_pattern = 3;
    ipc_send_to_cpu2(IPC_CMD_BUZZER_PLAY, &buzzer_pattern, 1);
    
    // New style (recommended)
    IPC_SEND_BUZZER_PLAY(3);
}

// ============================================================================
// EXAMPLE 7: ERROR HANDLING
// ============================================================================

void example_error_handling(void) {
    // Check if command was sent successfully
    if (!IPC_SEND_BUZZER_PLAY(5)) {
        // Handle send error (e.g., buffer full, payload too large)
        printf("Failed to send buzzer command\\n");
    }
    
    // Check if a raw command was sent
    uint8_t data[] = {1, 2, 3};
    if (!ipc_send_raw_cmd(0x50, data, 3)) {
        // Handle error
        printf("Failed to send raw command\\n");
    }
}

// ============================================================================
// MIGRATION GUIDE FROM OLD IPC SYSTEM
// ============================================================================

/*
OLD CODE:
    ipc_simple_init_cpu1();
    ipc_send_to_cpu2(IPC_CMD_BUZZER_PLAY, &pattern, 1);
    ipc_rx_service_cpu1();

NEW CODE:
    ipc_init_cpu1();
    IPC_SEND_BUZZER_PLAY(pattern);
    ipc_process_messages();

BENEFITS OF NEW SYSTEM:
1. Type-safe command categories
2. Automatic handler registration
3. Built-in error handling and statistics
4. Easy to extend with new commands
5. Better debugging with command names
6. Cleaner API with convenience macros
7. Structured payload handling
8. Backward compatibility maintained
*/
