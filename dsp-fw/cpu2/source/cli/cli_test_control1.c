#include "CLI/cli_test_control1.h"
//#include "vgen_cla1_shared.h"
#include "F28x_Project.h"
#include "application.h"
//#include "generation.h"
//#include "reference_generation.h"

//#include "feedback.h"

#define PANEL_TAIL_GAP    6


static void _handle_menu_init(cli_menu_t *menu, const my_time_t now, generic_status_t *ret) {
    menu->tx->current_index = 0;
    menu->scheduling = now;
    menu->machine.state = STATE_CLI_MENU;
    *ret = STATUS_PREPARING;
    menu->initialized = INITIALIZED;
}

static void _handle_menu_1(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{
    int n = 0;
    n += update_display_for_menu(&(menu->tx->buffer[n]));
    print_line(menu->tx->buffer, &n," Control Testing                                                                ");
    menu->tx->size = n;
    menu->machine.state = STATE_CLI_DATA_HEADER;
    menu->scheduling = now+50;
    *ret = STATUS_PROCESSING;
}


static void _handle_data_header(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
{
    int n = 0,i;


    n += update_display_for_header(&(menu->tx->buffer[n]));

    print_line(menu->tx->buffer, &n," V conf   :\t\t\t\t  <-  w  s  ->");    //1
    print_line(menu->tx->buffer, &n," I conf   :\t\t\t\t  <-  r  f  ->");    //2
    print_line(menu->tx->buffer, &n,"");                                     //3
    print_line(menu->tx->buffer, &n," V pri   :");                           //4
    print_line(menu->tx->buffer, &n," I pri   :");                           //5
    print_line(menu->tx->buffer, &n,"");                                     //6
    print_line(menu->tx->buffer, &n," V sec    :");                          //7
    print_line(menu->tx->buffer, &n," I sec    :");                          //8
    print_line(menu->tx->buffer, &n,"");                                     //9
    print_line(menu->tx->buffer, &n,"");                                     //10
    print_line(menu->tx->buffer, &n,"");                                     //11
    print_line(menu->tx->buffer, &n," PWM LIM  :");                          //12
    print_line(menu->tx->buffer, &n," PWM V    :");                          //13
    print_line(menu->tx->buffer, &n," PWM I    :");                          //14

    //prepare space for log
    for(i=0;i<PANEL_TAIL_GAP;i++)
        print_line(menu->tx->buffer, &n,"");

    n += update_display_move_up(&(menu->tx->buffer[n]), 14+PANEL_TAIL_GAP);
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

        print_line(menu->tx->buffer, &n, "\t\t %3.4f", 0.0f);     // 1
        print_line(menu->tx->buffer, &n, "\t\t %3.4f", 0.0f);     // 2
        print_line(menu->tx->buffer, &n, "");                     // 3 (em branco, se precisa)
        print_line(menu->tx->buffer, &n, "\t\t %s  %6.4fV %6.4fA %6.4fW %6.4f", "ON", 0.f,0.f,0.f,0.f); // 4
        print_line(menu->tx->buffer, &n, "\t\t %s  %6.4fV %6.4fA %6.4fW %6.4f", "ON", 0.f,0.f,0.f,0.f); // 5
        print_line(menu->tx->buffer, &n, "");                     // 6
        print_line(menu->tx->buffer, &n, "\t\t  %6.4fV %6.4fA %6.4fW %6.4f", 0.f,0.f,0.f,0.f); // 7
        print_line(menu->tx->buffer, &n, "\t\t  %6.4fV %6.4fA %6.4fW %6.4f", 0.f,0.f,0.f,0.f); // 8
        print_line(menu->tx->buffer, &n, "");                     // 9
        print_line(menu->tx->buffer, &n, "  v=%x %x i=%x %x", 0,0,0,0); // 10
        print_line(menu->tx->buffer, &n, "");                     // 11
        print_line(menu->tx->buffer, &n, "\t\t %d", 0);           // 12
        print_line(menu->tx->buffer, &n, "\t\t %d", 0);           // 13
        print_line(menu->tx->buffer, &n, "\t\t %d", 0);           // 14

        // tail for LOG lines
        n += update_display_for_log(&(menu->tx->buffer[n]));
        n += cli_render_tail_log(&(menu->tx->buffer[n]), CLI_TEXT_BOX_SIZE - n);

        menu->scheduling = now + 200; // 5 Hz
    }

    menu->tx->size = n;
    *ret = STATUS_PROCESSING;
}
//static void _handle_data(cli_menu_t *menu, const my_time_t now, int16_t rcv, generic_status_t *ret)
//{
//    int16_t n = 0;
//
//    if ( now >= menu->scheduling || rcv != -1)
//    {
//        n+= update_display_restore_position(&(menu->tx->buffer[n]));
//
//        char v_on[4];
//        char i_on[4];
//
//        //if(app.generation.voltage.status.generating)
//            strcpy(v_on, "ON ");
//        //else
//        //    strcpy(v_on, "OFF");
//
//        //if(app.generation.current.status.generating)
//            strcpy(i_on, "ON ");
//        //else
//        //    strcpy(i_on, "OFF");
//
//        n += snprintf(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer) - n,
//                      "\r\n\r\n"
////                      "\r\n\t\t %02.3f Hz  "
//                      "\r\n\t\t %3.4f  "
//                      "\r\n\t\t %3.4f  "
//                      "\r\n"
//                      "\r\n\t\t %s\t  %6.4fV   %6.4fA   %6.4fW  %6.4f  "
//                      "\r\n\t\t %s\t  %6.4fV   %6.4fA   %6.4fW %6.4f  "
//                      "\r\n"
//                      "\r\n\t\t  %6.4fV   %6.4fA  %6.4fW %6.4f "
//                      "\r\n\t\t  %6.4fV   %6.4fA  %6.4fW %6.4f "
//                      "\r\n"
////                      "\r\n\t\t[%ld]    "
//                      "\r\n  v=%x %x i=%x %x    "
//                      "\r\n"
//                      "\r\n\t\t %d    "
//                      "\r\n\t\t %d    "
//                      "\r\n\t\t %d    "
//                      "\r\n",
////                      (float)(100000.0/app.generation.config.generation_freq),
//                      0,//(app.generation.voltage.config.scale_requested),
//                      0,//(app.generation.current.config.scale_requested),
//                      v_on,
//                      0,//(app.measures.primary.voltage.voltage_rms),
//                      0,//(app.measures.primary.voltage.current_rms),
//                      0,//(app.measures.primary.voltage.power),
//                      0,//(app.measures.primary.voltage.impedance),
//                      i_on,
//                      0,//(app.measures.primary.current.voltage_rms),
//                      0,//(app.measures.primary.current.current_rms),
//                      0,//(app.measures.primary.current.power),
//                      0,//(app.measures.primary.current.impedance),
//
//                      0,//(app.measures.secondary.voltage.voltage_rms),
//                      0,//(app.measures.secondary.voltage.current_rms),
//                      0,//(app.measures.secondary.voltage.power),
//                      0,//(app.measures.secondary.voltage.impedance),
//
//                      0,//(app.measures.secondary.current.voltage_rms),
//                      0,//(app.measures.secondary.current.current_rms),
//                      0,//(app.measures.secondary.current.power),
//                      0,//(app.measures.secondary.current.impedance),
//
////                      100000000/((2*app.generation.config.inverter_pwm_steps)/256),
//                      0,//voltage_event_old, 0;//app.generation.voltage.protection.data.event,
//                      0,//current_event_old, 0;//app.generation.current.protection.data.event,
//                      0,//(app.generation.voltage.protection.config.pwm_max_value - app.generation.voltage.protection.config.pwm_min_value)/2,
//                      0,//(int)(app.generation.voltage.protection.data.max_pwm_duty),
//                      0//(int)(app.generation.current.protection.data.max_pwm_duty)
//
//
//        );
//
//        n += update_display_save_position(&(menu->tx->buffer[n]));
//
//        n += update_display_move_down(&(menu->tx->buffer[n]), PANEL_TAIL_GAP);
//        n += cli_render_tail_log(&(menu->tx->buffer[n]), sizeof(menu->tx->buffer)-n);
//    }
// menu->scheduling = now + 200;
//    menu->tx->size = n;
//}

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
    //    else if(rcv == ' ')
    //    {
    //        if(app.generation.voltage.command.enable)
    //            app.generation.voltage.command.disable_from_cli = true;
    //        if(app.generation.current.command.enable)
    //            app.generation.current.command.disable_from_cli = true;
    //    }
    //    else if (rcv == 'w')
    //    {
    //        if (app.generation.voltage.config.scale_requested < VOLTAGE_MAX)
    //        {
    //            app.generation.voltage.config.scale_requested = app.generation.voltage.config.scale_requested + 0.001;
    //        }
    //    }
    //    else if (rcv == 'W')
    //    {
    //        if (app.generation.voltage.config.scale_requested < VOLTAGE_MAX )
    //        {
    //            app.generation.voltage.config.scale_requested = app.generation.voltage.config.scale_requested + 1.0;
    //        }
    //    }
    //    else if (rcv == 's')
    //    {
    //        if (app.generation.voltage.config.scale_requested > 0.001 )
    //        {
    //            app.generation.voltage.config.scale_requested = app.generation.voltage.config.scale_requested - 0.001;
    //        }
    //        else
    //        {
    //            app.generation.voltage.config.scale_requested = 0.0f;
    //        }
    //    }
    //    else if (rcv == 'S')
    //    {
    //        if (app.generation.voltage.config.scale_requested > 1.0 )
    //        {
    //            app.generation.voltage.config.scale_requested = app.generation.voltage.config.scale_requested - 1.0;
    //        }
    //        else
    //        {
    //            app.generation.voltage.config.scale_requested = 0.0f;
    //        }
    //    }
    //    else if (rcv == 'R')
    //    {
    //        //if (app.generation.current.config.scale_requested <= CURRENT_MAX )
    //        {
    //            app.generation.current.config.scale_requested = app.generation.current.config.scale_requested + 1.0;
    //        }
    //    }
    //    else if (rcv == 'r')
    //    {
    //        //if (app.generation.current.config.scale_requested <= CURRENT_MAX )
    //        {
    //            app.generation.current.config.scale_requested = app.generation.current.config.scale_requested + 0.1;
    //        }
    //    }
    //    else if (rcv == 'F')
    //    {
    //        if (app.generation.current.config.scale_requested > 1.0 )
    //        {
    //            app.generation.current.config.scale_requested = app.generation.current.config.scale_requested - 1.0;
    //        }
    //        else
    //        {
    //            app.generation.current.config.scale_requested = 0.0f;
    //        }
    //    }
    //    else if (rcv == 'f')
    //    {
    //        if (app.generation.current.config.scale_requested > 0.1 )
    //        {
    //            app.generation.current.config.scale_requested = app.generation.current.config.scale_requested - 0.1;
    //        }
    //        else
    //        {
    //            app.generation.current.config.scale_requested = 0.0f;
    //        }
    //    }
    ////    else if (rcv == 'd')
    ////    {
    ////        app.generation.config.generation_freq = app.generation.config.generation_freq -1;
    ////        HRPWM_setTimeBasePeriod(SDFM_CLK_HRPWM_BASE, app.generation.config.generation_freq );
    ////        HRPWM_setCounterCompareValue(SDFM_CLK_HRPWM_BASE, HRPWM_COUNTER_COMPARE_A, (app.generation.config.generation_freq /2));
    ////    }
    ////    else if (rcv == 'a')
    ////    {
    ////        app.generation.config.generation_freq  = app.generation.config.generation_freq +1;
    ////        HRPWM_setTimeBasePeriod(SDFM_CLK_HRPWM_BASE, app.generation.config.generation_freq );
    ////        HRPWM_setCounterCompareValue(SDFM_CLK_HRPWM_BASE, HRPWM_COUNTER_COMPARE_A, (app.generation.config.generation_freq /2));
    ////    }
    ////    else if (rcv == 'D')
    ////    {
    ////        app.generation.config.generation_freq  = app.generation.config.generation_freq -10;
    ////        HRPWM_setTimeBasePeriod(SDFM_CLK_HRPWM_BASE, app.generation.config.generation_freq );
    ////        HRPWM_setCounterCompareValue(SDFM_CLK_HRPWM_BASE, HRPWM_COUNTER_COMPARE_A, (app.generation.config.generation_freq /2));
    ////    }
    ////    else if (rcv == 'A')
    ////    {
    ////        app.generation.config.generation_freq  = app.generation.config.generation_freq +10;
    ////        HRPWM_setTimeBasePeriod(SDFM_CLK_HRPWM_BASE, app.generation.config.generation_freq );
    ////        HRPWM_setCounterCompareValue(SDFM_CLK_HRPWM_BASE, HRPWM_COUNTER_COMPARE_A, (app.generation.config.generation_freq /2));
    ////    }
    //
    //
    //
    //
    //
    ////    else if (rcv == '>')
    ////    {
    ////
    ////        app.generation.config.inverter_pwm_steps = app.generation.config.inverter_pwm_steps - 100;
    ////        HRPWM_setTimeBasePeriod(INV_PWM1_VOLTAGE, app.generation.config.inverter_pwm_steps);
    ////        HRPWM_setTimeBasePeriod(INV_PWM2_VOLTAGE, app.generation.config.inverter_pwm_steps);
    ////
    ////        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_A, ( (app.generation.config.inverter_pwm_steps/2) << 8) );
    ////        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_B, 0);
    ////        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_A, ( (app.generation.config.inverter_pwm_steps/2) << 8) );
    ////        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_B, 0);
    ////    }
    ////
    ////    else if (rcv == '<')
    ////    {
    ////        app.generation.config.inverter_pwm_steps = app.generation.config.inverter_pwm_steps + 100;
    ////        HRPWM_setTimeBasePeriod(INV_PWM1_VOLTAGE, app.generation.config.inverter_pwm_steps);
    ////        HRPWM_setTimeBasePeriod(INV_PWM2_VOLTAGE, app.generation.config.inverter_pwm_steps);
    ////
    ////        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_A, ( (app.generation.config.inverter_pwm_steps/2) << 8) );
    ////        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_B, 0);
    ////        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_A, ( (app.generation.config.inverter_pwm_steps/2) << 8) );
    ////        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_B, 0);
    ////    }
    ////
    ////    else if (rcv == '}')
    ////    {
    ////        app.generation.config.deadband = app.generation.config.deadband + 100;
    ////        HRPWM_setRisingEdgeDelay(INV_PWM1_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setFallingEdgeDelay(INV_PWM1_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setRisingEdgeDelay(INV_PWM2_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setFallingEdgeDelay(INV_PWM2_VOLTAGE,app.generation.config.deadband);
    ////    }
    ////    else if (rcv == '{')
    ////    {
    ////
    ////        app.generation.config.deadband = app.generation.config.deadband - 100;
    ////        HRPWM_setRisingEdgeDelay(INV_PWM1_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setFallingEdgeDelay(INV_PWM1_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setRisingEdgeDelay(INV_PWM2_VOLTAGE,app.generation.config.deadband);
    ////        HRPWM_setFallingEdgeDelay(INV_PWM2_VOLTAGE,app.generation.config.deadband);
    ////    }


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
    print_line(menu->tx->buffer, &n," Control Testing End                                                            ");
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


generic_status_t cli_control1_handler (const my_time_t now, void * cli_menu, int16_t rcv)
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
        _handle_menu_1(menu, now, rcv, &ret);
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

