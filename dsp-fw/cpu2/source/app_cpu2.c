#include "app_cpu2.h"
#include "my_time.h"

#include "buzzer.h"
#include "cli.h"
#include "communications.h"
#include "ipc_simple.h"

#include "telemetry.h"

#include <stdio.h>

//// violação CLA (se CPU2 tiver ISR; se for CPU1, faça análogo lá)
//tlm_inc(&cpu2_wr.cnt.cla_access_violations);
//tlm_push_event((app_telemetry_block_t*)&cpu2_wr, my_time_ms(), EVT_CLA, /*code*/ 0);

static void cpu2_idle   (application_t *app, my_time_t now)
{
    app_sm_set(app->sm_cpu2, APP_STATE_START, now);
}

static void cpu2_start  (application_t *app, my_time_t now)
{
    ipc_simple_init_cpu2();
    // Wait until CPU1 running
    if (app->sm_cpu1.cur == APP_STATE_RUNNING) {
        cli_update_display_raw("Gen CPU: Ok\n\r");
        app_sm_set(app->sm_cpu2, APP_STATE_RUNNING, now);
        cli_update_display_raw("Aux CPU: Ok\n\r");
    }
}

//bool flag_dump = false;
bool flag_cli_log = false;
//void cli_cmd_stats(void)
//{
//    uint32_t i;
//    char buf[128];
//    // 1) counters (raw read; CPU2 NÃO escreve na RAM da outra CPU)
//    snprintf(buf, sizeof(buf),
//        "C1: TX:%lu RX:%lu  C2: TX:%lu RX:%lu  TO:%lu  CLA:%lu  ST:%lu\r\n",
//        (unsigned long)cpu1_wr.cnt.ipc_c1_to_c2_tx,
//        (unsigned long)cpu1_wr.cnt.ipc_c2_to_c1_rx,
//        (unsigned long)cpu2_wr.cnt.ipc_c2_to_c1_tx,
//        (unsigned long)cpu2_wr.cnt.ipc_c1_to_c2_rx,
//        (unsigned long)cpu1_wr.cnt.ipc_timeouts + cpu2_wr.cnt.ipc_timeouts,
//        (unsigned long)cpu1_wr.cnt.cla_access_violations + cpu2_wr.cnt.cla_access_violations,
//        (unsigned long)cpu1_wr.cnt.state_transitions + cpu2_wr.cnt.state_transitions);
//    cli_update_display_raw(buf);
//
//    // 2) últimos N eventos de cada lado
//    app_evt_t ev;
//    uint32_t n;
//
//    n = tlm_available((const app_telemetry_block_t*)&cpu1_wr);
//    snprintf(buf, sizeof(buf), "C1->log (%lu evts): ", (unsigned long)n);
//    cli_update_display_raw(buf);
//    for (i = 0; i < n && i < 16; ++i) {  // limita a 16 linhas por chamada
//        if (!tlm_pop_event((const app_telemetry_block_t*)&cpu1_wr, &ev)) break;
//        snprintf(buf, sizeof(buf), "  %8lu  code:%3u  data:%3u\r\n",
//                 (unsigned long)ev.t_ms, ev.code, ev.data);
//        cli_update_display_raw(buf);
//    }
//
//    n = tlm_available((const app_telemetry_block_t*)&cpu2_wr);
//    snprintf(buf, sizeof(buf), "C2->log (%lu evts): ", (unsigned long)n);
//    cli_update_display_raw(buf);
//    for (i = 0; i < n && i < 16; ++i) {
//        if (!tlm_pop_event((const app_telemetry_block_t*)&cpu2_wr, &ev)) break;
//        snprintf(buf, sizeof(buf), "  %8lu  code:%3u  data:%3u\r\n",
//                 (unsigned long)ev.t_ms, ev.code, ev.data);
//        cli_update_display_raw(buf);
//    }
//}

static void cpu2_running(application_t *app, my_time_t now) {
    cli_processing(now);
    comm_processing(now);
    buzzer_state_machine(now);

//    if(flag_dump){
//        cli_cmd_stats();
//        flag_dump = false;
//    }
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


    tlm_init_writer((app_telemetry_block_t*)&cpu2_wr);

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

    CPU2_HANDLERS[(app->sm_cpu2)->cur](app, now);

}


