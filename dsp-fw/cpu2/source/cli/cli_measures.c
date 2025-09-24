#include <cli_measures.h>
#include "my_time.h"
#include "generic_definitions.h"


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
                  "\r\n\r\n"
                  //                      "\r\n\t\t %02.3f Hz  "
                  "\r\n\t\tasdfsadfas  "
                  "\r\n\t\t asfadsfas  "
                  "\r\n"
                  "\r\n\t\t asfdsaf  "
                  "\r\n\t\t asfdsfas  "
                  "\r\n"
                  "\r\n\t\tasfasdff "
                  "\r\n\t\t asddfa4f "
                  "\r\n"
                  //                      "\r\n\t\t[%ld]    "
                  "\r\n  asdsdf    "
                  "\r\n"
                  "\r\n\t\t as  "
                  "\r\n\t\t asd  "
                  "\r\n\t\t asdfdsa  "
                  "\r\n"
                  //                      (float)(100000.0/app.generation.config.generation_freq),

    );
    n += update_display_for_data(&(menu->tx->buffer[n]));
    n += update_display_move_up(&(menu->tx->buffer[n]), 17);
    n += update_display_save_position(&(menu->tx->buffer[n]));

    menu->machine.state = STATE_CLI_DATA_1_WAINTNG_CMD;
    menu->scheduling = time_actual;
    menu->tx->size = n;
}

static void _handle_data_1(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    int16_t n = 0;

    if ( time_actual >= menu->scheduling || rcv != -1)
    {
        n+= update_display_restore_position(&(menu->tx->buffer[n]));



        n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                      "\r\n\r\n"
                      //                      "\r\n\t\t %02.3f Hz  "
                      "\r\n\t\tasdfsadfas  "
                      "\r\n\t\t asfadsfas  "
                      "\r\n"
                      "\r\n\t\t asfdsaf  "
                      "\r\n\t\t asfdsfas  "
                      "\r\n"
                      "\r\n\t\tasfasdff "
                      "\r\n\t\t asddfa4f "
                      "\r\n"
                      //                      "\r\n\t\t[%ld]    "
                      "\r\n  asdsdf    "
                      "\r\n"
                      "\r\n\t\t as  "
                      "\r\n\t\t asd  "
                      "\r\n\t\t asdfdsa  "
                      "\r\n"
                      //                      (float)(100000.0/app.generation.config.generation_freq),

        );


        menu->scheduling = time_actual + 1000;
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
                  " Control Testing Exit                                                          \r\n");
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
