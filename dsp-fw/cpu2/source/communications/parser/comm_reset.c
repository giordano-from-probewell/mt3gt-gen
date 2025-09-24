
#include <string.h>
#include <time.h>
#include <errno.h>


#include "_comm_internals.h"
#include "cmd_comm.h"

command_comm_status_t command_comm_reset(void)
{
    command_comm_status_t ret;

    if (_comm_ctrl.initialized == false)
    {
        ret = COMMAND_PARSER_STATUS__NOT_INITIALIZED;
    }
    else
    {
        ret = COMMAND_PARSER_STATUS__DONE;

        _comm_ctrl.map_idx = 0;
        _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
        _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
        _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__INIT;
        _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
        _comm_ctrl.error_category = 0;

        //memset(&_pkt_ctrl, 0, sizeof(_packet_control_t));
        _pkt_ctrl.checksum_computed = 0;
        _pkt_ctrl.idx = 0;
    }

    return (ret);
}


