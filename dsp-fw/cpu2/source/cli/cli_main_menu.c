#include "my_time.h"
#include "cli_main_menu.h"


static void _handle_menu_init(cli_menu_t *menu, const my_time_t time_actual, generic_status_t *ret)
{
    menu->tx->current_index = 0;
    menu->scheduling = time_actual;
    menu->machine.state = STATE_CLI_MENU;
    *ret = STATUS_PROCESSING;
    menu->initialized = INITIALIZED;
}

static void _handle_menu_1(cli_menu_t *menu, const my_time_t time_actual, int16_t rcv, generic_status_t *ret)
{
    int n = update_display_for_menu(menu->tx->buffer);
    n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer)  - n ,
                  "\r\n"
                  "  ___________________________  \r\n"
                  " |Main Menu [?]              | \r\n"
                  " |Measures :       [m|c]     | \r\n"
                  " |                           | \r\n"
                  " |LOG Menu:        [l]       | \r\n"
                  " |___________________________| \r\n"
                  "                               \r\n");

    n += update_display_for_data(&(menu->tx->buffer[n]));
    menu->tx->size = n;
    menu->scheduling = time_actual + 10; // 100ms
    menu->machine.state = STATE_CLI_MENU_END;
    *ret = STATUS_DONE;
}

static void _handle_menu_end(cli_menu_t *menu, const my_time_t time_actual, generic_status_t *ret)
{
    menu->tx->current_index = 0;
    menu->scheduling = time_actual;
    menu->machine.state = STATE_CLI_MENU;
    *ret = STATUS_WAITING;;
}



generic_status_t cli_main_menu_handler (const my_time_t time_actual, void * cli_menu, uint16_t rcv)
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

    case STATE_CLI_MENU_END:
        _handle_menu_end(menu, time_actual, &ret);
        ret = STATUS_DONE;
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
