#include "_comm_internals.h"
#include "cmd_comm.h"
#define _CMD_ERROR_FIELD__ERROR_CATEGORY    (0)
#define _CMD_ERROR_FIELD__ERROR_CODE        (1)



static const uint8_t _CMD_ERROR__NUMBER = 0xEEEE;


static const uint8_t _CMD_ERROR__SIZE = 2;



command_comm_status_t command_comm_error (int16_t *data)
{
    command_comm_status_t ret;
    uint8_t aux;

    if (_comm_ctrl.initialized == false)
    {
        ret = COMMAND_PARSER_STATUS__NOT_INITIALIZED;
    }
    else if (data == NULL)
    {
        ret = COMMAND_PARSER_STATUS__INVALID_PARAM;
    }
    else
    {
        switch (_comm_ctrl.error_state)
        {

        default:
        {
            break;
        }

        case _STATE_PARSER_ERROR__NONE:
        {
            break;
        }

        case _STATE_PARSER_ERROR__INIT:
        {
            _pkt_ctrl.checksum_computed = 0;
            _pkt_ctrl.idx = 0;
            _pkt_ctrl.packet.preamble = _COMMAND_PREAMBLE__FULL;
            _pkt_ctrl.packet.command.value = _CMD_ERROR__NUMBER;
            _pkt_ctrl.packet.size.value = _CMD_ERROR__SIZE;
            _pkt_ctrl.packet.data [ _CMD_ERROR_FIELD__ERROR_CATEGORY ] = _comm_ctrl.error_category;
            _pkt_ctrl.packet.data [ _CMD_ERROR_FIELD__ERROR_CODE ] = _comm_ctrl.error_code;
            _pkt_ctrl.packet.checksum = 0;
            _comm_ctrl.error_state = _STATE_PARSER_ERROR__PREAMBLE_1;
            *data = COMMAND_PARSER_DATA__UNAVAILABLE;
            ret = COMMAND_PARSER_STATUS__PREPARING;
            break;
        }

        case _STATE_PARSER_ERROR__PREAMBLE_1:
        {
            aux = _pkt_ctrl.packet.preamble >> 8;
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__PREAMBLE_1;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__PREAMBLE_2;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__PREAMBLE_2:
        {
            aux = _pkt_ctrl.packet.preamble & 0x00FF;
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__PREAMBLE_2;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__COMMAND;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__COMMAND:
        {
            //aux = _pkt_ctrl.packet.command.bytes[ _COMMAND_FIELD_SIZE__COMMAND - _pkt_ctrl.idx - 1 ];
            aux = _pkt_ctrl.packet.command.value >> (8 * (_COMMAND_FIELD_SIZE__COMMAND - _pkt_ctrl.idx - 1 ) );
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__COMMAND;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__COMMAND)
                {
                    _comm_ctrl.error_state = _STATE_PARSER_ERROR__COMMAND;
                }
                else
                {
                    _pkt_ctrl.idx = 0;
                    _comm_ctrl.error_state = _STATE_PARSER_ERROR__SIZE;
                }
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__SIZE:
        {
            //aux = _pkt_ctrl.packet.size.bytes[_COMMAND_FIELD_SIZE__SIZE - _pkt_ctrl.idx - 1];
            aux = _pkt_ctrl.packet.size.value >> (8 * (_COMMAND_FIELD_SIZE__SIZE - _pkt_ctrl.idx - 1 ) );
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__SIZE;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__SIZE)
                {
                    _comm_ctrl.error_state = _STATE_PARSER_ERROR__SIZE;
                }
                else
                {
                    _pkt_ctrl.idx = 0;
                    _comm_ctrl.error_state = _STATE_PARSER_ERROR__ERROR_CATEGORY;
                }
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__ERROR_CATEGORY:
        {

            aux = _pkt_ctrl.packet.data [ _CMD_ERROR_FIELD__ERROR_CATEGORY ];
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__ERROR_CATEGORY;
                _pkt_ctrl.idx++;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__ERROR_CODE;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__ERROR_CODE:
        {
            aux = _pkt_ctrl.packet.data [ _CMD_ERROR_FIELD__ERROR_CODE ];
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__ERROR_CODE;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__CHECKSUM_1;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__CHECKSUM_1:
        {
            aux = 0xCC;
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__CHECKSUM_1;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__CHECKSUM_2;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__CHECKSUM_2:
        {
            aux = 0xCC;
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__CHECKSUM_2;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__DONE;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }

        case _STATE_PARSER_ERROR__DONE:
        {
            _pkt_ctrl.checksum_computed = 0;
            _pkt_ctrl.idx = 0;
            _pkt_ctrl.packet.preamble = 0;
            _pkt_ctrl.packet.command.value = 0;
            _pkt_ctrl.packet.size.value = 0;
            _pkt_ctrl.packet.checksum = 0;
            _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
            *data = COMMAND_PARSER_DATA__UNAVAILABLE;
            ret = COMMAND_PARSER_STATUS__DONE;
            break;
        }

        }
    }

    return (ret);
}


