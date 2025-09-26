#include "cli_measures.h"
#include "my_time.h"
#include "generic_definitions.h"

#include "ipc_simple.h"
#include "application.h"

static void _handle_menu_init(cli_menu_t *menu, const my_time_t time_actual, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = time_actual;
    menu->machine.state = STATE_CLI_MENU;
    *ret = STATUS_PREPARING;
    menu->initialized = INITIALIZED;
}

static void _handle_menu_1(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    int n = 0;
    n += update_display_for_menu(&(menu->tx->buffer[n]));
    n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n ,
                  "\r\n"
                  " Measures                                                              \r\n");
    menu->tx->size = n;
    menu->machine.state = STATE_CLI_MENU_WAINTNG_CMD;
    menu->scheduling = time_actual;
    *ret = STATUS_PROCESSING;
}

static void _handle_menu_1_cmd(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    menu->machine.state = STATE_CLI_DATA_1_HEADER;
    menu->scheduling = time_actual + 10;
    *ret = STATUS_PROCESSING;

    menu->scheduling = time_actual;
}

static void _handle_data_1_header(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    int n = 0;


    n += update_display_for_header(&(menu->tx->buffer[n]));
    n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                  " Phase:    A              B               C          \r\n"

    );
    n += update_display_for_data(&(menu->tx->buffer[n]));
    n += update_display_move_up(&(menu->tx->buffer[n]), 23);
    n += update_display_save_position(&(menu->tx->buffer[n]));

    menu->machine.state = STATE_CLI_DATA_1_WAINTNG_CMD;
    menu->scheduling = time_actual;
    menu->tx->size = n;
}

static void _handle_data_1(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    int16_t n = 0;
    static int show = 0;

    if ( time_actual >= menu->scheduling || rcv != -1)
    {

        if(show == 0){
            show = 1;

        }
        else if(show == 1){
            show = 2;

            n+= update_display_restore_position(&(menu->tx->buffer[n]));


            n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                          "\r\n"
                          "\t teste \r\n" //v


            );


        }
        else if(show == 2){
            show = 3;

            n+= update_display_restore_position(&(menu->tx->buffer[n]));


            n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                          "\r\n"
                          "\t teste1 \r\n" //v


            );

        }
        else if(show == 3){
            show = 0;

            n+= update_display_restore_position(&(menu->tx->buffer[n]));


            n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                          "\r\n"
                          "\t teste 2\r\n" //v


            );


        }



        menu->scheduling = time_actual + 100;
    }

    menu->tx->size = n;
}

static void _handle_data_1_cmd(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    menu->machine.state = STATE_CLI_DATA_1_WAINTNG_CMD;
    *ret = STATUS_PROCESSING;
    if(rcv == 'x' || rcv == 'X'){
        menu->machine.state = STATE_CLI_MENU_ENDING;
        *ret = STATUS_PROCESSING;
    }
    if(rcv == 'r' || rcv == 'R'){

        int8_t x=0;
        ipc_send_to_cpu1(IPC_CMD_RESET_METRICS, &x, 1);

        menu->machine.state = STATE_CLI_DATA_1_WAINTNG_CMD;
        *ret = STATUS_PROCESSING;
    }
    else {
        menu->machine.state = STATE_CLI_DATA_1_WAINTNG_CMD;
        *ret = STATUS_PROCESSING;
    }

}

static void _handle_menu_ending(cli_menu_t *menu, const my_time_t time_actual, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = time_actual;
    int n = 0;
    n += update_display_for_menu(&(menu->tx->buffer[n]));
    n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n ,
                  "\r\n"
                  " Measures Exit                                                          \r\n");
    menu->tx->size = n;
    menu->scheduling = time_actual+10;
    menu->machine.state = STATE_CLI_MENU_END;
    *ret = STATUS_WAITING;
}



static void _handle_menu_end(cli_menu_t *menu, const my_time_t time_actual, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = time_actual;
    menu->machine.state = STATE_CLI_MENU_INIT;
    *ret = STATUS_DONE;
}

generic_status_t cli_measures_menu_handler (const my_time_t time_actual, void * cli_menu, int16_t rcv)
{
    cli_menu_t *menu = (cli_menu_t *)cli_menu;

    generic_status_t ret = STATUS_INVALID_INFO;

    if(menu->initialized != INITIALIZED){
        menu->machine.state = STATE_CLI_MENU_INIT;
    }


    switch(menu->machine.state)
    {

    case STATE_CLI_MENU_INIT:
        _handle_menu_init(menu, time_actual, &ret);
        break;

    case STATE_CLI_MENU:
        _handle_menu_1(menu, time_actual, rcv, &ret);
        break;

    case STATE_CLI_MENU_WAINTNG_CMD:
        _handle_menu_1_cmd(menu, time_actual, rcv, &ret);
        break;

    case STATE_CLI_DATA_1_HEADER:
        _handle_data_1_header(menu, time_actual, rcv, &ret);
        break;

    case STATE_CLI_DATA_1_WAINTNG_CMD:
        _handle_data_1_cmd(menu, time_actual, rcv, &ret);
        _handle_data_1(menu, time_actual, rcv, &ret);
        break;

    case STATE_CLI_MENU_ENDING:
        _handle_menu_ending(menu, time_actual, &ret);
        break;

    case STATE_CLI_MENU_END:
        _handle_menu_end(menu, time_actual, &ret);
        break;

    case STATE_CLI_MENU_ERROR:
        ret = STATUS_ERROR;
        break;

    default:
        menu->machine.state = STATE_CLI_MENU_ERROR;
        ret = STATUS_ERROR;
        break;
    }

    return ret;


}
