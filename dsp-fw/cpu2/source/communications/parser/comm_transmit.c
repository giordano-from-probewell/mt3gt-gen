#include <string.h>
#include <time.h>
#include <errno.h>
#include "errors.h"

#include "_comm_internals.h"
#include "cmd_comm.h"
#include "crc_utils.h"

command_comm_status_t command_comm_transmit(int16_t *data)
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
        switch (_comm_ctrl.transmit_state)
        {


        default:
        {
            break;
        }


        case _STATE_PARSER_TRANSMIT__NONE:
        {
            break;
        }


        case _STATE_PARSER_TRANSMIT__INIT:
        {
            _pkt_ctrl.checksum_computed = 0;
            _pkt_ctrl.idx = 0;
            _pkt_ctrl.packet.preamble = _COMMAND_PREAMBLE__FULL;
            uint16_t crc = calculate_crc16(&_pkt_ctrl.packet);
            _pkt_ctrl.packet.checksum = crc;
            _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__PREAMBLE_1;
            *data = COMMAND_PARSER_DATA__UNAVAILABLE;
            ret = COMMAND_PARSER_STATUS__PREPARING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__PREAMBLE_1:
        {
            aux = _COMMAND_PREAMBLE__PART_1;//_pkt_ctrl.packet.preamble >> 8;
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__PREAMBLE_1;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.transmit_state =  _STATE_PARSER_TRANSMIT__PREAMBLE_2;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__PREAMBLE_2:
        {
            aux = _COMMAND_PREAMBLE__PART_2;//(uint8_t) (_pkt_ctrl.packet.preamble & 0x00FF);
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__PREAMBLE_2;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx = 0;
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__COMMAND;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__COMMAND:
        {
            //aux = _pkt_ctrl.packet.command.bytes[ _COMMAND_FIELD_SIZE__COMMAND - _pkt_ctrl.idx - 1 ];
            aux = _pkt_ctrl.packet.command.value >> (8 * ( _COMMAND_FIELD_SIZE__COMMAND - _pkt_ctrl.idx - 1 ) );
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__COMMAND;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__COMMAND)
                {
                    _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__COMMAND;
                }
                else
                {
                    _pkt_ctrl.idx = 0;
                    _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__SIZE;
                }
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__SIZE:
        {
            //aux = _pkt_ctrl.packet.size.bytes[ _COMMAND_FIELD_SIZE__SIZE - _pkt_ctrl.idx - 1 ];
            aux = _pkt_ctrl.packet.size.value >> (8 * (_COMMAND_FIELD_SIZE__SIZE - _pkt_ctrl.idx - 1 ) );
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__SIZE;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__SIZE)
                {
                    _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__SIZE;
                }
                else
                {
                    _pkt_ctrl.idx = 0;
                    if (_pkt_ctrl.packet.size.value == 0)
                     {
                         _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__CHECKSUM_1;
                     }
                     else
                     {
                         _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__DATA;
                     }
                }
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__DATA:
        {
            aux = _pkt_ctrl.packet.data[_pkt_ctrl.idx];
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__DATA;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _pkt_ctrl.packet.size.value)
                {
                    _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__DATA;
                }
                else
                {
                    _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__CHECKSUM_1;
                }
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__CHECKSUM_1:
        {
            aux = (uint8_t)(_pkt_ctrl.packet.checksum >> 8);
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__CHECKSUM_1;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__CHECKSUM_2;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__CHECKSUM_2:
        {
            aux = (uint8_t)(_pkt_ctrl.packet.checksum & 0xFF);
            if (*data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__CHECKSUM_2;
                *data = aux;
            }
            else
            {
                *data = COMMAND_PARSER_DATA__UNAVAILABLE;
                _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__DONE;
            }
            ret = COMMAND_PARSER_STATUS__PROCESSING;
            break;
        }


        case _STATE_PARSER_TRANSMIT__DONE:
        {
            _pkt_ctrl.checksum_computed = 0;
            _pkt_ctrl.idx = 0;
            _pkt_ctrl.packet.preamble = 0;
            _pkt_ctrl.packet.command.value = 0;
            _pkt_ctrl.packet.size.value = 0;
            _pkt_ctrl.packet.checksum = 0;
            _comm_ctrl.transmit_state = _STATE_PARSER_TRANSMIT__INIT;
            *data = COMMAND_PARSER_DATA__UNAVAILABLE;
            ret = COMMAND_PARSER_STATUS__DONE;
            break;
        }


        }
    }
    return (ret);
}
