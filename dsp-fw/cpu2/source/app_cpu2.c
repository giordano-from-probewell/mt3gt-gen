#include "app_cpu2.h"
#include <stdio.h>

#include "my_time.h"
#include "buzzer.h"
#include "cli.h"
#include "communications.h"
#include "ipc_simple.h"

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


