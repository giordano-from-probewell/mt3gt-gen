#include <F28x_Project.h>
#include <errno.h>

#include <stdint.h>
#include "driverlib.h"
#include "device.h"

#include "Communications/Commands/commands.h"
#include "Communications/Parser/cmd_comm.h"

#include "application.h"


#define COMMS_VBUS_CLK 50000000
#define COMMS_SCI_BAUDRATE 115200


typedef enum comm_state
{
    state_init = 0,
    state_reset,
    state_receive,
    state_execute,
    state_transmit,
    state_error,
} _communication_states_t;


typedef struct CommControl
{
    _communication_states_t state;
    int16_t data;
} _communication_control_t;


/*static*/ _communication_control_t commControl =
{
 .state = state_init,
};


static const command_comm_map_t _cmd_map[] =
{
 { .handler = cmd_ping,                            .number = COMMAND_PING                           , },
 { .handler = cmd_identify,                        .number = COMMAND_IDENTIFY                       , },
 { .handler = cmd_reset_voltage_waveform,          .number = COMMAND_RESET_VOLTAGE_WAVEFORM         , },
 { .handler = cmd_reset_current_waveform,          .number = COMMAND_RESET_CURRENT_WAVEFORM         , },
 { .handler = cmd_set_voltage_waveform,            .number = COMMAND_SET_VOLTAGE_WAVEFORM           , },
 { .handler = cmd_get_voltage_waveform,            .number = COMMAND_GET_VOLTAGE_WAVEFORM           , },
 { .handler = cmd_set_current_waveform,            .number = COMMAND_SET_CURRENT_WAVEFORM           , },
 { .handler = cmd_get_current_waveform,            .number = COMMAND_GET_CURRENT_WAVEFORM           , },
 { .handler = cmd_stop_generation,                 .number = COMMAND_STOP_GENERATION                , },
 { .handler = cmd_start_generation,                .number = COMMAND_START_GENERATION               , },
 { .handler = cmd_start_voltage_generation,        .number = COMMAND_START_VOLTAGE_GENERATION       , },
 { .handler = cmd_stop_voltage_generation,         .number = COMMAND_STOP_VOLTAGE_GENERATION        , },
 { .handler = cmd_start_current_generation,        .number = COMMAND_START_CURRENT_GENERATION       , },
 { .handler = cmd_stop_current_generation,         .number = COMMAND_STOP_CURRENT_GENERATION        , },
 { .handler = cmd_set_scale_current,               .number = COMMAND_SET_CURRENT_SCALE              , },
 { .handler = cmd_get_scale_current,               .number = COMMAND_GET_CURRENT_SCALE              , },
 { .handler = cmd_set_scale_voltage,               .number = COMMAND_SET_VOLTAGE_SCALE              , },
 { .handler = cmd_get_scale_voltage,               .number = COMMAND_GET_VOLTAGE_SCALE              , },
 { .handler = cmd_set_frequency,                   .number = COMMAND_SET_FREQUENCY                  , },
 { .handler = cmd_get_frequency,                   .number = COMMAND_GET_FREQUENCY                  , },
 { .handler = cmd_get_status,                      .number = COMMAND_GET_STATUS                     , },
 { .handler = cmd_get_error,                       .number = COMMAND_GET_ERROR                      , },
 { .handler = cmd_get_measures,                    .number = COMMAND_GET_MEASURES                   , },
 { .handler = cmd_get_calibration,                 .number = COMMAND_GET_CALIBRATION                , },
 { .handler = cmd_set_calibration,                 .number = COMMAND_SET_CALIBRATION                , },
 { .handler = cmd_gpio1_clear,                     .number = COMMAND_GPIO1_CLEAR                    , },
 { .handler = cmd_gpio1_set,                       .number = COMMAND_GPIO1_SET                      , },
};





void _hw_config(volatile uint32_t sci_base,
                uint32_t vbus_clk,
                uint32_t baudrate)
{


    SCI_clearInterruptStatus(sci_base, SCI_INT_TXFF);
    SCI_enableFIFO(sci_base);
    SCI_setFIFOInterruptLevel(sci_base, SCI_FIFO_TX0, SCI_FIFO_RX4);
    SCI_clearInterruptStatus(sci_base, SCI_INT_RXFF);

    SCI_setConfig(sci_base, vbus_clk, baudrate, (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));

    SCI_resetChannels(sci_base);
    SCI_resetRxFIFO(sci_base);
    SCI_resetTxFIFO(sci_base);
    SCI_clearInterruptStatus(sci_base, SCI_INT_TXFF | SCI_INT_RXFF);
    //SCI_disableLoopback(sci_base);
    SCI_enableFIFO(sci_base);

    SCI_enableModule(sci_base);
    SCI_performSoftwareReset(sci_base);
}


void comm_init(void)
{

    _hw_config (
            COMMS_SCI_BASE,
            COMMS_VBUS_CLK,
            COMMS_SCI_BAUDRATE
    );
    commControl.state = state_init;
}

void comm_processing(const my_time_t time_actual)
{
    command_comm_status_t cmd_status;
    int st = SCI_getRxStatus(COMMS_SCI_BASE);

    if(st & SCI_RXSTATUS_ERROR)
    {
        SCI_performSoftwareReset(COMMS_SCI_BASE);
        SCI_writeCharBlockingNonFIFO(COMMS_SCI_BASE, '1');

        commControl.state = state_reset;
    }

    //    //if you want to discover the serial port...
    //    {
    //        static my_time_t t = 0;
    //
    //        if(t<time_actual){
    //            t = time_actual+2000;
    //
    //            SCI_writeCharBlockingNonFIFO(COMMS_SCI_BASE, '*');
    //        }
    //    }

    //SCI_writeCharBlockingNonFIFO(COMMS_SCI_BASE, 's');

    switch (commControl.state)
    {

    case state_init:
    {
        cmd_status = command_comm_initiate( _cmd_map, sizeof(_cmd_map) / sizeof(command_comm_map_t));
        commControl.state = state_reset;

        break;
    }

    case state_reset:
    {
        commControl.state = state_receive;
        cmd_status = command_comm_reset();


        break;
    }

    case state_receive:
    {
        commControl.data = 0;
        errno = 0;

        if(SCI_getRxFIFOStatus(COMMS_SCI_BASE) != SCI_FIFO_RX0)
        {
            commControl.data = SCI_readCharNonBlocking(COMMS_SCI_BASE);
        }
        else
        {
            commControl.data = COMMAND_PARSER_DATA__UNAVAILABLE;
        }
        cmd_status = command_comm_receive(time_actual,commControl.data);
        switch (cmd_status)
        {
        case COMMAND_PARSER_STATUS__NOTHING:
        {
            commControl.state = state_receive;
            break;
        }
        case COMMAND_PARSER_STATUS__NOT_INITIALIZED:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__PREPARING:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__PROCESSING:
        {
            commControl.state = state_receive;
            break;
        }
        case COMMAND_PARSER_STATUS__DONE:
        {
            commControl.state = state_execute;
            break;
        }
        case COMMAND_PARSER_STATUS__ERROR:
        {
            commControl.data = 0;
            commControl.state = state_error;
            break;
        }
        }
        break;
    }

    case state_execute:
    {
        cmd_status = command_comm_process();
        switch (cmd_status)
        {
        case COMMAND_PARSER_STATUS__NOTHING:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__NOT_INITIALIZED:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__PREPARING:
        {
            commControl.state = state_execute;
            break;
        }
        case COMMAND_PARSER_STATUS__PROCESSING:
        {
            commControl.state = state_execute;
            break;
        }
        case COMMAND_PARSER_STATUS__DONE:
        {
            commControl.state = state_transmit;
            break;
        }
        case COMMAND_PARSER_STATUS__ERROR:
        {
            commControl.data = 0;
            commControl.state = state_error;
            break;
        }
        }
        break;
    }



    case state_transmit:
    {
        cmd_status = command_comm_transmit(&commControl.data);
        switch (cmd_status)
        {
        case COMMAND_PARSER_STATUS__NOTHING:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__NOT_INITIALIZED:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__PREPARING:
        {
            commControl.state = state_transmit;
            break;
        }
        case COMMAND_PARSER_STATUS__PROCESSING:
        {
            if (commControl.data != COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                errno = 0;
                if(!SCI_isTransmitterBusy(COMMS_SCI_BASE))
                {
                    SCI_writeCharNonBlocking(COMMS_SCI_BASE,commControl.data );
                }
                //if (ret < 0 && errno == ENOSPC)
                else
                {
                    commControl.data = COMMAND_PARSER_DATA__UNAVAILABLE;
                }
            }
            commControl.state = state_transmit;
            break;
        }
        case COMMAND_PARSER_STATUS__DONE:
        {
            commControl.state = state_reset;
            break;
        }
        case COMMAND_PARSER_STATUS__ERROR:
        {
            break;
        }
        }
        break;
    }



    case state_error:
    {
        cmd_status = command_comm_error(&commControl.data);
        switch (cmd_status)
        {
        case COMMAND_PARSER_STATUS__NOTHING:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__NOT_INITIALIZED:
        {
            break;
        }
        case COMMAND_PARSER_STATUS__PREPARING:
        {
            commControl.state = state_error;
            break;
        }
        case COMMAND_PARSER_STATUS__PROCESSING:
        {
            if (commControl.data != COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                errno = 0;
                if(!SCI_isTransmitterBusy(COMMS_SCI_BASE))
                {
                    SCI_writeCharNonBlocking(COMMS_SCI_BASE,commControl.data );
                }
                //if (ret < 0 && errno == ENOSPC)
                else
                {
                    commControl.data = COMMAND_PARSER_DATA__UNAVAILABLE;
                }
            }
            commControl.state = state_error;
            break;
        }
        case COMMAND_PARSER_STATUS__DONE:
        {
            commControl.state = state_reset;
            break;
        }
        case COMMAND_PARSER_STATUS__ERROR:
        {
            break;
        }
        }
        break;
    }
    }
}

