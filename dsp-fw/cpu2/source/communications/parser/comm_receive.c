
#include <string.h>
#include "my_time.h"
#include <errno.h>
#include "errors.h"

#include "_comm_internals.h"
#include "cmd_comm.h"
#include "crc_utils.h"

#define _IN_DEVELOPMENT_STAGE
#ifdef _IN_DEVELOPMENT_STAGE
#define _DEVELOPMENT_CHECKSUM   (0xCCCC)
#endif


command_comm_status_t command_comm_receive(const my_time_t time_actual, const int16_t data)
{
    command_comm_status_t ret;
    //time_t time_actual;

    if (_comm_ctrl.initialized == false)
    {
        ret = COMMAND_PARSER_STATUS__NOT_INITIALIZED;
    }
    else
    {
        //time_actual = time(NULL);

        if (_timeout_ctrl.on_off == true)
            if (time_actual >= _timeout_ctrl.scheduled)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                _comm_ctrl.error_category = (uint8_t) ERROR_DATA_LINK_LAYER;
                _comm_ctrl.error_code =  (uint8_t) ERROR_DATA_LINK_LAYER__PKT_FRAME_SIZE;
                _timeout_ctrl.on_off = false;
                _timeout_ctrl.scheduled = 0;

                return (COMMAND_PARSER_STATUS__ERROR);
            }

        switch (_comm_ctrl.receive_state)
        {
        default:
        {
            break;
        }

        case _STATE_PARSER_RECEIVE__NONE:
        {
            break;
        }

        case _STATE_PARSER_RECEIVE__PREAMBLE_1:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                if ((uint8_t) data == _COMMAND_PREAMBLE__PART_1)
                {
                    _pkt_ctrl.packet.preamble = ((uint16_t) data) << 8;
                    _comm_ctrl.receive_state =  _STATE_PARSER_RECEIVE__PREAMBLE_2;
                    ret = COMMAND_PARSER_STATUS__PROCESSING;
                }
                else
                {
                    _comm_ctrl.receive_state =  _STATE_PARSER_RECEIVE__PREAMBLE_1;
                    ret = COMMAND_PARSER_STATUS__NOTHING;
                }
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__PREAMBLE_2:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_2;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                if ((uint8_t) data == _COMMAND_PREAMBLE__PART_2)
                {
                    _pkt_ctrl.packet.preamble |= (uint8_t) data;
                    _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__COMMAND;
                    _timeout_ctrl.on_off = true;                 
                    _pkt_ctrl.idx = 0;
                    _pkt_ctrl.packet.command.value = 0x0000;
                    _timeout_ctrl.scheduled = time_actual + _COMMAND_PARSER_RECEIVE__TIMEOUT;
                    ret = COMMAND_PARSER_STATUS__PROCESSING;
                }
                else
                {
                    _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
                    _pkt_ctrl.checksum_computed = 0;
                    _pkt_ctrl.packet.preamble = 0;
                    ret = COMMAND_PARSER_STATUS__NOTHING;
                }
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__COMMAND:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__COMMAND;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                //_pkt_ctrl.packet.command.bytes[_COMMAND_FIELD_SIZE__COMMAND -_pkt_ctrl.idx-1] = (uint8_t)data;
                _pkt_ctrl.packet.command.value = _pkt_ctrl.packet.command.value << 8;;
                _pkt_ctrl.packet.command.value += (uint8_t)data;
                _timeout_ctrl.scheduled = time_actual + _COMMAND_PARSER_RECEIVE__TIMEOUT;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__COMMAND)
                {
                    _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__COMMAND;
                }
                else
                {
                    _pkt_ctrl.idx = 0;
                    _pkt_ctrl.packet.size.value = 0x0000;
                    _comm_ctrl.receive_state =  _STATE_PARSER_RECEIVE__SIZE;
                }
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__SIZE:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__SIZE;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                //_pkt_ctrl.packet.size.bytes[_COMMAND_FIELD_SIZE__SIZE -_pkt_ctrl.idx-1] = (uint8_t)data;
                _pkt_ctrl.packet.size.value = _pkt_ctrl.packet.size.value << 8;;
                _pkt_ctrl.packet.size.value += (uint8_t)data;
                _timeout_ctrl.scheduled = time_actual + _COMMAND_PARSER_RECEIVE__TIMEOUT;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _COMMAND_FIELD_SIZE__SIZE)
                {
                    _comm_ctrl.receive_state =  _STATE_PARSER_RECEIVE__SIZE;
                }
                else
                {
                    if (_pkt_ctrl.packet.size.value == 0)
                    {
                        _comm_ctrl.receive_state =  _STATE_PARSER_RECEIVE__CHECKSUM_1;
                    }
                    else if (_pkt_ctrl.packet.size.value >= _COMMAND_FIELD_SIZE__DATA)
                    {
                        _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
                        _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                        _comm_ctrl.error_category = (uint8_t) ERROR_DATA_LINK_LAYER;
                        _comm_ctrl.error_code =  (uint8_t) ERROR_DATA_LINK_LAYER__PKT_EXCEEDS_MAXIMUM_SIZE;
                        ret = COMMAND_PARSER_STATUS__ERROR;
                    }
                    else
                    {
                        _pkt_ctrl.idx = 0;
                        _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__DATA;
                    }
                }
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }

            break;
        }

        case _STATE_PARSER_RECEIVE__DATA:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__DATA;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                _timeout_ctrl.scheduled = time_actual + _COMMAND_PARSER_RECEIVE__TIMEOUT;
                _pkt_ctrl.packet.data[_pkt_ctrl.idx] = (uint8_t) data;
                _pkt_ctrl.idx++;
                if (_pkt_ctrl.idx < _pkt_ctrl.packet.size.value)
                {
                    _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__DATA;
                }
                else
                {
                    _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__CHECKSUM_1;
                }
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }

            break;
        }

        case _STATE_PARSER_RECEIVE__CHECKSUM_1:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__CHECKSUM_1;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                _pkt_ctrl.packet.checksum = ((uint16_t) data) << 8;

                _timeout_ctrl.scheduled = time_actual + _COMMAND_PARSER_RECEIVE__TIMEOUT;
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__CHECKSUM_2;
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__CHECKSUM_2:
        {
            if (data == COMMAND_PARSER_DATA__UNAVAILABLE)
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__CHECKSUM_2;
                ret = COMMAND_PARSER_STATUS__NOTHING;
            }
            else
            {
                _pkt_ctrl.packet.checksum |= ((uint8_t) data);
                _timeout_ctrl.on_off = false;
                _timeout_ctrl.scheduled = 0;
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__VERIFICATION;
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__VERIFICATION:
        {
            // Calculate the CRC of the received packet
            uint16_t computed_crc = calculate_crc16(&_pkt_ctrl.packet);

            #ifdef _IN_DEVELOPMENT_STAGE
            if ((_pkt_ctrl.packet.checksum == _DEVELOPMENT_CHECKSUM) || (computed_crc == _pkt_ctrl.packet.checksum))
            #else
            if (computed_crc == _pkt_ctrl.packet.checksum)
            #endif
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__DONE;
                ret = COMMAND_PARSER_STATUS__PROCESSING;
            }
            else
            {
                _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
                _comm_ctrl.error_state = _STATE_PARSER_ERROR__INIT;
                _comm_ctrl.error_category = (uint8_t) ERROR_DATA_LINK_LAYER;
                _comm_ctrl.error_code =  (uint8_t) ERROR_DATA_LINK_LAYER__PKT_INVALID_CHECKSUM;
                ret = COMMAND_PARSER_STATUS__ERROR;
            }
            break;
        }

        case _STATE_PARSER_RECEIVE__DONE:
        {
            _pkt_ctrl.checksum_computed = 0;
            _pkt_ctrl.idx = 0;
            _pkt_ctrl.packet.preamble = 0;
            _pkt_ctrl.packet.checksum = 0;
            _comm_ctrl.receive_state = _STATE_PARSER_RECEIVE__PREAMBLE_1;
            _comm_ctrl.process_state = _STATE_PARSER_PROCESS__INIT;
            ret = COMMAND_PARSER_STATUS__DONE;
            break;
        }
        }
    }

    return (ret);
}

