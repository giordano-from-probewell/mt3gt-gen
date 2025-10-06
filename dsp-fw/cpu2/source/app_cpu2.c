#include "app_cpu2.h"
#include <stdio.h>

#include "my_time.h"
#include "buzzer.h"
#include "cli.h"
#include "communications.h"
#include "ipc_simple.h"
#include "log.h"





/////////////////////////// LOG TEST ////////////////////////////////////////////

/*static*/ log_handle_t g_log;


/* simple in-RAM array simulating EEPROM region */
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
    // Wait until CPU1 running
    if (app->sm_cpu1.cur == APP_STATE_RUNNING) {
        cli_update_display_raw("Gen CPU: Ok\n\r");
        app_sm_set(app->sm_cpu2, APP_STATE_RUNNING, now);
        cli_update_display_raw("Aux CPU: Ok\n\r");
    }

    log_store_if_t st = log_store_ram();
    (void)log_init(&g_log, st, /*base_addr=*/0, /*max_logs=*/(4096 - sizeof(log_header_disk_t))/LOG_SLOT_BYTES);
}



bool flag_test=false;
bool flag_cli_log = false;


static void cpu2_running(application_t *app, my_time_t now) {
    cli_processing(now);
    comm_processing(now);
    buzzer_state_machine(now);


    if(flag_test){
        flag_test = false;
        int payload = 0xcd;
        ipc_enqueue_to_cpu1(0xab,&payload,1 );    // enfileira

    }

    if(flag_cli_log){
        CLI_LOGI("IPC C1->C2 ack=%u", 123);
        CLI_LOGW("Overtemp: %d C", 56);
        CLI_LOGE("SDFM fault: 0x%04X", 0xabc);
        flag_cli_log = false;
    }


}

static void cpu2_error  (application_t *app, my_time_t now)
{
    //TODO: error handling
}

static const state_handler_t CPU2_HANDLERS[] = {
                                                [APP_STATE_IDLE]    = cpu2_idle,
                                                [APP_STATE_START]   = cpu2_start,
                                                [APP_STATE_RUNNING] = cpu2_running,
                                                [APP_STATE_ERROR]   = cpu2_error
};

void app_init_cpu2(application_t *app) {
    int boot_error = 0;
    *(app->sm_cpu2) = (app_sm_t){ .cur = APP_STATE_START };


    ipc_simple_init_cpu2();

    cli_init ();

    comm_init();
    cli_update_display_raw("Comm stack: Ok \n\r");

    buzzer_init(app->id.data.full.my_address);
    cli_update_display_raw("Buzzer: Ok \n\r");

    if (boot_error)
        buzzer_enqueue(boot_fail);
    else
        buzzer_enqueue(boot_ok);
}


void app_run_cpu2(application_t *app) {

    my_time_t now = my_time(NULL);
    ipc_service_cpu2();          // send when doorbell is free
    CPU2_HANDLERS[(app->sm_cpu2)->cur](app, now);

}


