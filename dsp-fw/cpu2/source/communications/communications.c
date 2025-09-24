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
 { .handler = cmd_start_tx_pulse,                  .number = COMMAND_START_TX_PULSE                 , },
 { .handler = cmd_set_kh,                          .number = COMMAND_SET_KH                         , },
 { .handler = cmd_get_kh,                          .number = COMMAND_GET_KH                         , },
 { .handler = cmd_start_test,                      .number = COMMAND_START_TEST                     , },
 { .handler = cmd_reset_test,                      .number = COMMAND_RESET_TEST                     , },
 { .handler = cmd_get_measures,                    .number = COMMAND_GET_MEASURES                   , },
 { .handler = cmd_get_inst_measures,               .number = COMMAND_GET_INST_MEASURES              , },
 { .handler = cmd_change_factor,                   .number = COMMAND_CHANGE_FACTOR                  , },
 { .handler = cmd_current_range,                   .number = COMMAND_CURRENT_RANGE                  , },
 { .handler = cmd_get_calibrations,                .number = COMMAND_GET_CALIBRATIONS               , },
 { .handler = cmd_get_calibration,                 .number = COMMAND_GET_CALIBRATION                , },
 { .handler = cmd_set_calibration,                 .number = COMMAND_SET_CALIBRATION                , },
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

//    //JUST FOR DEBUGGING
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

