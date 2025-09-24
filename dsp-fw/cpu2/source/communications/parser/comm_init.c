#include <string.h>
#include <time.h>
#include <errno.h>

#include "errors.h"

#include "_comm_internals.h"
#include "cmd_comm.h"

command_comm_status_t command_comm_initiate (const command_comm_map_t *commands, const uint16_t count)
{
    command_comm_status_t ret;

    if ((commands == NULL) || (count == 0))
    {
        ret = COMMAND_PARSER_STATUS__INVALID_PARAM;
    }
    else
    {
        ret = COMMAND_PARSER_STATUS__DONE;
        memset (&_comm_ctrl, 0, sizeof (_comm_control_t));
        memset (&_pkt_ctrl, 0, sizeof (_packet_control_t));
        _comm_ctrl.map_cmds = commands;
        _comm_ctrl.map_count = count;
        _comm_ctrl.initialized = true;
        _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
        _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__INIT;
        _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
        _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
    }
    return (ret);
}


