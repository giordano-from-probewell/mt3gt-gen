

#include "my_time.h"
#include "CLI/cli_test.h"


#define PANEL_TAIL_GAP    6

static void _handle_menu_init(cli_menu_t *menu, const my_time_t now, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = now;
    menu->machine.state = STATE_CLI_MENU;
    *ret = STATUS_PREPARING;
    menu->initialized = INITIALIZED;
}

static void _handle_menu(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{
    int n = 0;
    n += update_display_for_menu(&(menu->tx->buffer[n]));
    print_line(menu->tx->buffer, &n," Test Menu                                                                      ");
    menu->tx->size = n;
    menu->machine.state = STATE_CLI_DATA_HEADER;
    menu->scheduling = now;
    *ret = STATUS_PROCESSING;
}


static void _handle_data_header(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{
    int n = 0,i;

    n += update_display_for_header(&(menu->tx->buffer[n]));
    print_line(menu->tx->buffer, &n," time stamp :"); //1
    print_line(menu->tx->buffer, &n," char sent  :"); //2

    //prepare space for log
    for(i=0;i<PANEL_TAIL_GAP;i++)
        print_line(menu->tx->buffer, &n,"");

    n += update_display_move_up(&(menu->tx->buffer[n]), 2+PANEL_TAIL_GAP);
    n += update_display_save_position(&(menu->tx->buffer[n]));


    menu->machine.state = STATE_CLI_DATA_WAINTNG_CMD;
    menu->scheduling = now;
    menu->tx->size = n;
    *ret = STATUS_PROCESSING;
}

static void _handle_data(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{
    int n = 0;
    if (now >= menu->scheduling || rcv != -1)
    {
        n += update_display_restore_position(&(menu->tx->buffer[n]));
        n += update_display_for_data(&(menu->tx->buffer[n]));
        print_line(menu->tx->buffer, &n,"\t\t %ld", (long)now);
        print_line(menu->tx->buffer, &n,"\t\t %c", (rcv!=-1)?(char)rcv:' ');

        // tail for LOG lines
        n += update_display_for_log(&(menu->tx->buffer[n]));
        n += cli_render_tail_log(&(menu->tx->buffer[n]), CLI_TEXT_BOX_SIZE - n);

        menu->scheduling = now + 200; //5Hz
    }
    menu->tx->size = n;
    *ret = STATUS_PROCESSING;
}

static void _handle_data_cmd(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{

    if (rcv == 'g' || rcv == 'G') {
        static uint8_t lines = 0;
        lines = (lines == 0) ? 4 : 0;
        cli_set_panel_log_lines(lines);
    }


    if(rcv == 'x' || rcv == 'X'){
        menu->machine.state = STATE_CLI_MENU_ENDING;
        *ret = STATUS_PROCESSING;
    }
    else {
        menu->machine.state = STATE_CLI_DATA_WAINTNG_CMD;
        *ret = STATUS_PROCESSING;
    }
}


static void _handle_menu_ending(cli_menu_t *menu, const my_time_t now, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = now;
    int n = 0;
    n += update_display_for_menu(&(menu->tx->buffer[n]));
    print_line(menu->tx->buffer, &n," Test Menu End                                                                  ");

    menu->tx->size = n;
    menu->scheduling = now+10;
    menu->machine.state = STATE_CLI_MENU_END;
    *ret = STATUS_WAITING;
}


static void _handle_menu_end(cli_menu_t *menu, const my_time_t now, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = now;
    menu->machine.state = STATE_CLI_MENU_INIT;
    *ret = STATUS_DONE;
}

generic_status_t cli_test_menu_handler (const my_time_t now, void * cli_menu, int16_t rcv)
{
    cli_menu_t *menu = (cli_menu_t *)cli_menu;

    generic_status_t ret = STATUS_INVALID_INFO;

    if(menu->initialized != INITIALIZED){
        menu->machine.state = STATE_CLI_MENU_INIT;
    }


    switch(menu->machine.state)
    {
    case STATE_CLI_MENU_INIT:
        _handle_menu_init(menu, now, &ret);
        break;

    case STATE_CLI_MENU:
        _handle_menu(menu, now, rcv, &ret);
        break;

    case STATE_CLI_DATA_HEADER:
        _handle_data_header(menu, now, rcv, &ret);
        break;

    case STATE_CLI_DATA_WAINTNG_CMD:
        _handle_data_cmd(menu, now, rcv, &ret);
        _handle_data(menu, now, rcv, &ret);
        break;

    case STATE_CLI_MENU_ENDING:
        _handle_menu_ending(menu, now, &ret);
        break;

    case STATE_CLI_MENU_END:
        _handle_menu_end(menu, now, &ret);
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
