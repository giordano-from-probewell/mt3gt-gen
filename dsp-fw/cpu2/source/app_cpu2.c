/**
 * @file app_cpu2.c
 * @brief CPU2 Main Application - Handles CLI, Communications, Buzzer and IPC coordination
 * @version 1.0
 * @date 2025-10-22
 */

// ============================================================================
// INCLUDES
// ============================================================================

// System includes
#include <stdio.h>

// Application includes
#include "app_cpu2.h"
#include "my_time.h"
#include "buzzer.h"
#include "cli.h"
#include "communications.h"
#include "ipc_comm.h"
#include "ipc_handlers.h"
#include "log.h"





// ============================================================================
// LOG SYSTEM CONFIGURATION
// ============================================================================

/*static*/ log_handle_t g_log;


// Simple in-RAM array simulating EEPROM region
#ifndef LOG_RAM_BYTES
#define LOG_RAM_BYTES  (2u*1024u)
#endif
static uint8_t g_log_ram[LOG_RAM_BYTES];

static bool ram_read(uint32_t addr, void* dst, uint16_t len)
{
    if ((addr + len) > LOG_RAM_BYTES) return false;
    memcpy(dst, &g_log_ram[addr], len);
    return true;
}
static bool ram_write(uint32_t addr, const void* src, uint16_t len)
{
    if ((addr + len) > LOG_RAM_BYTES) return false;
    memcpy(&g_log_ram[addr], src, len);
    return true;
}
static bool ram_erase_range(uint32_t addr, uint32_t len)
{
    if ((addr + len) > LOG_RAM_BYTES) return false;
    memset(&g_log_ram[addr], 0xFF, len); /* simulate erased */
    return true;
}

log_store_if_t log_store_ram(void)
{
    log_store_if_t s = { ram_read, ram_write, ram_erase_range };
    return s;
}








/* --------------------------------------------------------------------------
 * Seed N fake log entries.
 * If count == 0, it seeds 20 by default.
 * The payload is a short ASCII line (< 24 bytes) compatible with log_slot_disk_t.
 * -------------------------------------------------------------------------- */
bool log_seed_fake_logs(log_handle_t* h, uint32_t count)
{
    uint32_t i;
    if (!h) return false;
    if (count == 0) count = 20;

    for (i = 0; i < count; ++i) {
        char line[24];
        int n = 0;

        /* Keep payload short to fit in 24 bytes (counter+len already consume 6 bytes) */
        n += snprintf(line + n, sizeof(line) - n, "fake #%lu ok", (unsigned long)(i + 1));
        if (n < 0) return false;

        if (!log_append(h, line, (uint16_t)n)) {
            return false; /* stop on first failure */
        }
    }
    return true;
}

/* --------------------------------------------------------------------------
 * Fetch the Nth-from-last log (n=1 => newest/last, n=2 => previous, ...).
 * Thin wrapper over log_get_last_n().
 * Returns true on success and fills *out.
 * -------------------------------------------------------------------------- */
bool log_fetch_last_nth(log_handle_t* h, uint32_t n, log_slot_disk_t* out)
{
    if (!h || !out || n == 0) return false;
    return log_get_last_n(h, n, out);
}

/* --------------------------------------------------------------------------
 * Fetch a log by its absolute counter value.
 * This linearly scans the ring (max_logs slots), reading each slot and
 * returning the first that matches 'counter'.
 * For small rings this is perfectly fine. For EEPROM, keep the ring modest
 * or add a sparse index later if needed.
 * -------------------------------------------------------------------------- */
bool log_find_by_counter(log_handle_t* h, uint32_t counter, log_slot_disk_t* out)
{
    uint32_t i ;
    if (!h || !out) return false;
    if (h->hdr.counter == 0) return false; /* empty */

    /* We need the storage ops to read arbitrary slots. */
    for (i = 0; i < h->hdr.max_logs; ++i) {
        /* Walk indices from the most recent backwards, so we usually stop early. */
        uint32_t idx = (h->hdr.wr_index + h->hdr.max_logs - 1u - i) % h->hdr.max_logs;
        uint32_t addr = h->base_addr + sizeof(log_header_disk_t) + (idx * LOG_SLOT_BYTES);

        log_slot_disk_t tmp;
        if (!h->store.read(addr, &tmp, sizeof(tmp))) {
            return false;
        }
        if (tmp.counter == counter) {
            *out = tmp;
            return true;
        }
    }
    return false; /* not found */
}


void logs_demo_boot(void)
{
    log_store_if_t st = log_store_ram();
    /* reserve first 4 KB for logs in the RAM backend */
    (void)log_init(&g_log, st, /*base_addr=*/0,
                   /*max_logs=*/(4096 - sizeof(log_header_disk_t))/LOG_SLOT_BYTES);

    /* seed 20 fake entries */
    (void)log_seed_fake_logs(&g_log, 20);
}

void logs_demo_read_examples(void)
{
    log_slot_disk_t rec;

    /* 1) get the last (newest) log */
    if (log_fetch_last_nth(&g_log, 1, &rec)) {
        /* rec.data[0..rec.len-1] has the message */
    }

    /* 2) find a specific counter (e.g., 7) */
    if (log_find_by_counter(&g_log, 7, &rec)) {
        /* found counter == 7 */
    }
}


/////////////////////////// LOG TEST END /////////////////////////////////////////




/* when IPC ISR receives IPC_CMD_LOG_EVT: */
void sink_log_to_storage(uint16_t code, uint16_t data, uint32_t t32)
{
    /* pack a short, human parsable line (<=24 bytes) */
    char line[24];
    int n = 0;
    n += snprintf(line+n, sizeof(line)-n, "%lu ", (unsigned long)t32);
    n += snprintf(line+n, sizeof(line)-n, "C%u D%u", (unsigned)code, (unsigned)data);
    log_append(&g_log, line, (uint16_t)n);
    /* and also show on CLI live log */
}


extern void cli_log_info(const char* s)
{
    CLI_LOGI("IPC C1->C2: %s", s);
}


extern void cli_log_info1(const char* s)
{
    CLI_LOGI("IPC C2->C1: %s", s);
}

static void cpu2_idle   (application_t *app, my_time_t now)
{
    app_sm_set(app->sm_cpu2, APP_STATE_START, now);
}

static void cpu2_start  (application_t *app, my_time_t now)
{

    int boot_error = 0;
    *(app->sm_cpu2) = (app_sm_t){ .cur = APP_STATE_START };


    ipc_init_cpu2();
    // Registrar handlers customizados
    ipc_handlers_init_cpu2();

    cli_init ();

    comm_init();
    cli_update_display_raw("Comm stack: Ok \n\r");

    buzzer_init(app->id.data.full.my_address);
    cli_update_display_raw("Buzzer: Ok \n\r");

    if (boot_error)
        buzzer_enqueue(boot_fail);
    else
        buzzer_enqueue(boot_ok);

    // Wait until CPU1 running
    if (app->sm_cpu1.cur == APP_STATE_RUNNING) {
        cli_update_display_raw("Gen CPU: Ok\n\r");
        app_sm_set(app->sm_cpu2, APP_STATE_RUNNING, now);
        cli_update_display_raw("Aux CPU: Ok\n\r");
    }

    log_store_if_t st = log_store_ram();
    (void)log_init(&g_log, st, /*base_addr=*/0, /*max_logs=*/(4096 - sizeof(log_header_disk_t))/LOG_SLOT_BYTES);
}

// ============================================================================
// IPC COMMUNICATION HANDLERS
// ============================================================================

// Handler to receive status from CPU1
bool handle_status_from_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 6) return false;

    uint8_t cpu1_state = payload[0];
    uint8_t generation_active = payload[1];
    uint8_t scale_percent = payload[2];

    // Verify checksum
    uint8_t checksum = 0;
    for (int i = 0; i < 5; i++) {
        checksum ^= payload[i];
    }

    if (checksum != payload[5]) {
        // Invalid checksum
        return false;
    }

    // Process received status
    CLI_LOGI("CPU1 Status: State=%d, Gen=%d, Scale=%d%%",
             cpu1_state, generation_active, scale_percent);

    // Update local variables or send via communication
    // ...

    return true;
}

// Handler for control commands from CPU1
bool handle_control_from_cpu1(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t cmd = payload[0];

    switch (cmd) {
    case 0: // Reset system
        // Implement reset
        CLI_LOGI("Reset command from CPU1");
        break;
    case 1: // Change mode
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

// Custom buzzer handler (overrides default)
bool handle_buzzer_custom(const uint8_t* payload, uint8_t len) {
    if (len < 1) return false;

    uint8_t pattern = payload[0];

    CLI_LOGI("Buzzer play pattern %d from CPU1", pattern);

    // Custom buzzer implementation
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

// NOTE: IPC handlers are now configured in ipc_handlers.c

// ============================================================================
// APPLICATION FLAGS AND VARIABLES
// ============================================================================

// Test flags
bool flag_test = false;
bool flag_cli_log = false;


// ============================================================================
// MAIN APPLICATION LOOP
// ============================================================================

static void cpu2_running(application_t *app, my_time_t now) {
    // Process IPC messages (already called automatically by ISR,
    // but can call here too for safety)
    ipc_process_messages();
    cli_processing(now);
    comm_processing(now);
    buzzer_state_machine(now);


    // Example: Send commands to CPU1 based on CLI
    process_cli_to_ipc_commands();

    // Example: Monitor communication
    monitor_ipc_health();

    if(flag_test){
        flag_test = false;
        uint8_t payload = 0xcd;
        ipc_send_raw_cmd(0xab, &payload, 1);    // Use the new IPC system

    }

    if(flag_cli_log){
        CLI_LOGI("IPC C1->C2 ack=%u", 123);
        CLI_LOGW("Overtemp: %d C", 56);
        CLI_LOGE("SDFM fault: 0x%04X", 0xabc);
        flag_cli_log = false;
    }


}

// ============================================================================
// CLI TO IPC COMMAND EXAMPLES 
// ============================================================================

/**
 * @brief Example function showing how to send commands to CPU1 from CLI
 * @note This function is not called anywhere - it's for reference only
 */
void send_commands_to_cpu1_from_cli(void) {

    // Generation commands
    IPC_SEND_GEN_ENABLE(0x03);      // Enable channels 0 and 1
    IPC_SEND_GEN_DISABLE(0x01);     // Disable channel 0
    IPC_SEND_GEN_CHANGE_WAVEFORM(0); // Change waveform of channel 0
}

// CLI command flags
bool cli_flag_gen_enable = false;
float cli_new_scale = 1.0f;

// Process CLI commands that should be sent to CPU1
void process_cli_to_ipc_commands(void) {
    // Example: If user typed command in CLI, send to CPU1
    static bool last_gen_enable = false;

    if (cli_flag_gen_enable && !last_gen_enable) {
        // User requested to enable generation
        uint8_t gen_cmd[] = {0, 1};  // Enable voltage + current
        ipc_send_generation_cmd(IPC_CMD_GEN_ENABLE, gen_cmd, 2);
        last_gen_enable = true;
    }

    if (!cli_flag_gen_enable) {
        last_gen_enable = false;
    }

    // Exemplo: Enviar scale personalizado
    static float last_scale = -1.0f;
    if (cli_new_scale != last_scale) {
        uint8_t scale_cmd[5];
        scale_cmd[0] = 0;  // Canal 0 (voltage)
        memcpy(&scale_cmd[1], &cli_new_scale, sizeof(float));
        ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, scale_cmd, 5);
        last_scale = cli_new_scale;
    }
}

// ============================================================================
// 5. COMANDOS CLI QUE CONTROLAM CPU1 VIA IPC
// ============================================================================

// Comando CLI: "gen enable"
void cli_cmd_gen_enable(void) {
    IPC_SEND_GEN_ENABLE(0x01);  // Habilitar canal 0
    CLI_LOGI("Generation enable sent to CPU1");
}

// Comando CLI: "gen disable"
void cli_cmd_gen_disable(void) {
    IPC_SEND_GEN_DISABLE(0x01);  // Desabilitar canal 0
    CLI_LOGI("Generation disable sent to CPU1");
}

// Comando CLI: "gen scale <value>"
// EXEMPLO DE USO:
// Para chamar no CLI, você pode:
// 1. Usar via variável global: cli_new_scale = 1.5f; (depois será processado automaticamente)
// 2. Chamar diretamente: cli_cmd_gen_scale(1.5f);
// 3. Integrar com parser de comando de texto se disponível
void cli_cmd_gen_scale(float scale) {
    uint8_t payload[5];
    payload[0] = 0;  // Canal 0
    memcpy(&payload[1], &scale, sizeof(float));

    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
    CLI_LOGI("Generation scale %.2f sent to CPU1", scale);
}

// CLI Command: "gen wave"
void cli_cmd_gen_waveform(void) {
    IPC_SEND_GEN_CHANGE_WAVEFORM(0);  // Change waveform of channel 0
    CLI_LOGI("Waveform change sent to CPU1");
}



// ============================================================================
// MONITORING AND DIAGNOSTICS
// ============================================================================

/**
 * @brief Monitor IPC communication health and log statistics
 */
void monitor_ipc_health(void) {
    static uint32_t last_check = 0;
    uint32_t now = 0;//get_time_ms();  // Your time function

    if ((now - last_check) >= 10000) {  // Every 10 seconds
        // Check communication health
        if (!ipc_is_communication_ok()) {
            CLI_LOGE("IPC communication error!");
        }

        // Log statistics
        uint32_t sent = ipc_get_messages_sent();
        uint32_t received = ipc_get_messages_received();
        uint32_t errors = ipc_get_communication_errors();

        CLI_LOGI("IPC Stats: TX=%lu, RX=%lu, ERR=%lu", sent, received, errors);

        last_check = now;
    }
}


// ============================================================================
// APPLICATION STATE MACHINE
// ============================================================================

static void cpu2_error(application_t *app, my_time_t now)
{
    //TODO: error handling
}

static const state_handler_t CPU2_HANDLERS[] = {
                                                [APP_STATE_IDLE]    = cpu2_idle,
                                                [APP_STATE_START]   = cpu2_start,
                                                [APP_STATE_RUNNING] = cpu2_running,
                                                [APP_STATE_ERROR]   = cpu2_error
};

void app_init_cpu2(application_t *app)
{
    *(app->sm_cpu2) = (app_sm_t){ .cur = APP_STATE_START };
}


void app_run_cpu2(application_t *app) {

    my_time_t now = my_time(NULL);
    CPU2_HANDLERS[(app->sm_cpu2)->cur](app, now);

}


