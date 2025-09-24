#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "_comm_internals.h"
#include "cmd_comm.h"

command_comm_status_t command_comm_process(void)
{
    _command_result_t cmd_res;
    command_comm_status_t ret;

    if (_comm_ctrl.initialized == false)
    {
        ret = COMMAND_PARSER_STATUS__NOT_INITIALIZED;
    }
    else
    {
        switch (_comm_ctrl.process_state)
        {
        case _STATE_PARSER_PROCESS__NONE:
        {
            break;
        }

        case _STATE_PARSER_PROCESS__INIT:
        {
            _comm_ctrl.map_idx = 0;
            _comm_ctrl.process_state = _STATE_PARSER_PROCESS__FIND;
            _comm_ctrl.error_category = (uint8_t) ERROR_CATEGORY_NONE;
            _comm_ctrl.error_code = (uint8_t) ERROR_CODE_NONE;
            ret = COMMAND_PARSER_STATUS__PREPARING;

            break;
        }

        case _STATE_PARSER_PROCESS__FIND:
        {
            if (_comm_ctrl.map_idx < _comm_ctrl.map_count)
            {
                if (_pkt_ctrl.packet.command.value == _comm_ctrl.map_cmds[_comm_ctrl.map_idx].number)
                {
                    _comm_ctrl.process_state = _STATE_PARSER_PROCESS__EXECUTE;
                }
                else
                {
                    _comm_ctrl.map_idx++;
                }
                ret = COMMAND_PARSER_STATUS__PREPARING;
            }
            else
            {
                _comm_ctrl.map_idx = 0;
                _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                _comm_ctrl.error_category = (uint8_t) ERROR_APPLICATION_LAYER;
                _comm_ctrl.error_code = (uint8_t) ERROR_APP_LAYER__CMD_NOT_RECOGNIZED;
                ret = COMMAND_PARSER_STATUS__ERROR;
            }
            break;
        }

        case _STATE_PARSER_PROCESS__EXECUTE:
        {
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            cmd_res = _COMMAND_RESULT__ERROR;


            cmd_res =      (_command_result_t) _comm_ctrl.map_cmds[_comm_ctrl.map_idx].handler
                    (
                            &_comm_ctrl.error_category,
                            &_comm_ctrl.error_code,
                            &_pkt_ctrl.packet.command.value,
                            _pkt_ctrl.packet.data,
                            &_pkt_ctrl.packet.size.value
                    );

            switch (cmd_res)
            {
            default:
            {
                ret = COMMAND_PARSER_STATUS__ERROR;
                _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                _comm_ctrl.error_category = (uint8_t) ERROR_APPLICATION_LAYER;
                _comm_ctrl.error_code = (uint8_t) ERROR_APP_LAYER__CMD_INVALID_RETURN;
                break;
            }

            case _COMMAND_RESULT__ERROR:
            {
                _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                ret = COMMAND_PARSER_STATUS__ERROR;
                break;
            }

            case _COMMAND_RESULT__SUCCESS:
            {
                _comm_ctrl.process_state = _STATE_PARSER_PROCESS__DONE;
                break;
            }

            case _COMMAND_RESULT__YIELD:
            {
                _comm_ctrl.process_state = _STATE_PARSER_PROCESS__EXECUTE;
                break;
            }
            }

            break;
        }

        case _STATE_PARSER_PROCESS__DONE:
        {
            _comm_ctrl.map_idx = 0;
            _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
            _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__INIT;
            ret = COMMAND_PARSER_STATUS__DONE;

            break;
        }
        }
    }


    return (ret);
}
