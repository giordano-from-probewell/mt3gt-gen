#include "app_cpu2.h"
#include "my_time.h"

#include "buzzer.h"
#include "cli.h"
#include "communications.h"
#include "ipc_simple.h"


#include <stdio.h>


static app_state_t app_cpu2_state = APP_STATE_IDLE;




void app_init_cpu2(application_t *app) {
    int boot_error = 0;
    app_cpu2_state = APP_STATE_START;


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

    static bool cpu1_running;
    my_time_t time_actual = 0;

    time_actual = my_time(NULL);

    switch (app_cpu2_state) {
    case APP_STATE_IDLE:
        break;

    case APP_STATE_START:
        cpu1_running = false;

        ipc_simple_init_cpu2();


        if(!cpu1_running && app->app_cpu1_state == APP_STATE_RUNNING)
        {
            int16_t status;

            cpu1_running = true;
            cli_update_display_raw("Gen CPU: Ok\n\r");


            app_cpu2_state = APP_STATE_RUNNING;
        }
        break;

    case APP_STATE_RUNNING:


        cli_processing (time_actual);
        comm_processing(time_actual);
        buzzer_state_machine(time_actual);

        break;

    case APP_STATE_ERROR:
        break;
    }
}


