#include <cli_measures.h>
#include "my_time.h"
#include "generic_definitions.h"
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
                  " V: \r\n"
                  " I: \r\n"
                  " W: \r\n"
                  " VAr: \r\n"
                  " VA: \r\n"
                  " phi: \r\n"
                  " pf: \r\n"
                  " Wh: \r\n"
                  " VArh: \r\n"
                  " VAh: \r\n"
                  " V AVG: \r\n"
                  " V MAX: \r\n"
                  " V MIN: \r\n"
                  " I AVG: \r\n"
                  " I MAX: \r\n"
                  " I MIN: \r\n"
                  " V num: \n\r"
                  " V den: \n\r"
                  " I num: \n\r"
                  " I den: \n\r"
                  " Current range: \n\r"
                  " Press 'x' to EXIT \r\n"
                  " Press 'r' to RESET ACCUMULATIVE METRICS "
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
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //v
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //w
                          "\t %08.5f \t%08.5f \t%08.5f \r\n" //var
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //va
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //phi
                          "\t \r\n" //pf
                          "\t \r\n" //wh
                          "\t \r\n" //varh
                          "\t \r\n" //vah
                          "\t \r\n" //v avg
                          "\t \r\n" //v max
                          "\t \r\n" //v min
                          "\t \r\n" //i vag
                          "\t \r\n" //i max
                          "\t \r\n" //i min
                          "\t \r\n" //v factor num
                          "\t \r\n" //v factor den
                          "\t \r\n" //i factor num
                          "\t \r\n" //i factor den
                          "\t \r\n", //range

                          app.meter.phase[1].measures.v,
                          app.meter.phase[2].measures.v,
                          app.meter.phase[3].measures.v,

                          app.meter.phase[1].measures.i,
                          app.meter.phase[2].measures.i,
                          app.meter.phase[3].measures.i,

                          app.meter.phase[1].measures.w,
                          app.meter.phase[2].measures.w,
                          app.meter.phase[3].measures.w,

                          app.meter.phase[1].measures.var,
                          app.meter.phase[2].measures.var,
                          app.meter.phase[3].measures.var,

                          app.meter.phase[1].measures.va,
                          app.meter.phase[2].measures.va,
                          app.meter.phase[3].measures.va,

                          app.meter.phase[1].measures.phi,
                          app.meter.phase[2].measures.phi,
                          app.meter.phase[3].measures.phi


            );


        }
        else if(show == 2){
            show = 3;

            n+= update_display_restore_position(&(menu->tx->buffer[n]));


            n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                          "\r\n"
                          "\t \r\n" //v
                          "\t \r\n" //i
                          "\t \r\n" //w
                          "\t \r\n" //var
                          "\t \r\n" //va
                          "\t \r\n" //phi
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //pf
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //wh
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //varh
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //vah
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //v avg
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //v max
                          "\t %08.4f \t%08.4f \t%08.4f \r\n" //v min
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i vag
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i max
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i min
                          "\t \r\n" //v factor num
                          "\t \r\n" //v factor den
                          "\t \r\n" //i factor num
                          "\t \r\n" //i factor den
                          "\t \r\n", //range



                          app.meter.phase[1].measures.power_factor,
                          app.meter.phase[2].measures.power_factor,
                          app.meter.phase[3].measures.power_factor,

                          app.meter.phase[1].measures.wh,
                          app.meter.phase[2].measures.wh,
                          app.meter.phase[3].measures.wh,

                          app.meter.phase[1].measures.varh,
                          app.meter.phase[2].measures.varh,
                          app.meter.phase[3].measures.varh,

                          app.meter.phase[1].measures.vah,
                          app.meter.phase[2].measures.vah,
                          app.meter.phase[3].measures.vah,

                          0,//app.meter.phase[1].voltage.metrics.rms_avg,
                          0,//app.meter.phase[2].voltage.metrics.rms_avg,
                          0,//app.meter.phase[3].voltage.metrics.rms_avg,

                          0,//app.meter.phase[1].voltage.metrics.rms_max,
                          0,//app.meter.phase[2].voltage.metrics.rms_max,
                          0,//app.meter.phase[3].voltage.metrics.rms_max,

                          0,//app.meter.phase[1].voltage.metrics.rms_min,
                          0,//app.meter.phase[2].voltage.metrics.rms_min,
                          0,//app.meter.phase[3].voltage.metrics.rms_min,

                          0,//app.meter.phase[1].current.metrics.rms_avg,
                          0,//app.meter.phase[2].current.metrics.rms_avg,
                          0,//app.meter.phase[3].current.metrics.rms_avg,

                          0,//app.meter.phase[1].current.metrics.rms_max,
                          0,//app.meter.phase[2].current.metrics.rms_max,
                          0,//app.meter.phase[3].current.metrics.rms_max,

                          0,//app.meter.phase[1].current.metrics.rms_min,
                          0,//app.meter.phase[2].current.metrics.rms_min,
                          0//app.meter.phase[3].current.metrics.rms_min

            );

        }
        else if(show == 3){
            show = 0;

            n+= update_display_restore_position(&(menu->tx->buffer[n]));


            n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
                          "\r\n"
                          "\t \r\n" //v
                          "\t \r\n" //i
                          "\t \r\n" //w
                          "\t \r\n" //var
                          "\t \r\n" //va
                          "\t \r\n" //phi
                          "\t \r\n" //pf
                          "\t \r\n" //wh
                          "\t \r\n" //varh
                          "\t \r\n" //vah
                          "\t \r\n" //v avg
                          "\t \r\n" //v max
                          "\t \r\n" //v min
                          "\t \r\n" //i vag
                          "\t \r\n" //i max
                          "\t \r\n" //i min
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //v factor num
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //v factor den
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i factor num
                          "\t %08.6f \t%08.6f \t%08.6f \r\n" //i factor den
                          "\t\t %d \r\n", //range



                          app.meter.phase[1].config.v_factor_num,
                          app.meter.phase[2].config.v_factor_num,
                          app.meter.phase[3].config.v_factor_num,

                          app.meter.phase[1].config.v_factor_den,
                          app.meter.phase[2].config.v_factor_den,
                          app.meter.phase[3].config.v_factor_den,

                          app.meter.phase[1].config.i_factor_num,
                          app.meter.phase[2].config.i_factor_num,
                          app.meter.phase[3].config.i_factor_num,

                          app.meter.phase[1].config.i_factor_den,
                          app.meter.phase[2].config.i_factor_den,
                          app.meter.phase[3].config.i_factor_den,

                          app.meter.phase[1].current_cal_index

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
