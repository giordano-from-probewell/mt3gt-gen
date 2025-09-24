#ifndef _PARSER_INTERNALS_H
#define _PARSER_INTERNALS_H

#include <stdint.h>
#include "cmd_comm.h"
#include "hw_types.h"
#include "types.h"
#include "time.h"


#define _COMMAND_FIELD_SIZE__PREAMBLE   (1)   //   1 uint16_t
#define _COMMAND_FIELD_SIZE__COMMAND    (2)   //   1 uint16_t
#define _COMMAND_FIELD_SIZE__SIZE       (2)   //   1 uint16_t
#define _COMMAND_FIELD_SIZE__DATA       (COMMAND_BUFFER_SIZE) //  x * uint8_t
#define _COMMAND_FIELD_SIZE__CHECKSUM   (2)   //   1 uint16_t

#define _COMMAND_FRAME_SIZE (_COMMAND_FIELD_SIZE__PREAMBLE + \
        _COMMAND_FIELD_SIZE__CMD_NUMBER + \
        _COMMAND_FIELD_SIZE__CMD_SIZE + \
        _COMMAND_FIELD_SIZE__CMD_DATA + \
        _COMMAND_FIELD_SIZE__CHECKSUM)

//----------------------------------------------------------------
#define _COMMAND_PARSER_RECEIVE__TIMEOUT    (100)

extern const uint16_t _COMMAND_PREAMBLE__FULL;
extern const uint8_t _COMMAND_PREAMBLE__PART_1;
extern const uint8_t _COMMAND_PREAMBLE__PART_2;

typedef enum _comm_receive_states_en
{
    _STATE_PARSER_RECEIVE__NONE = 0,
    _STATE_PARSER_RECEIVE__PREAMBLE_1,
    _STATE_PARSER_RECEIVE__PREAMBLE_2,
    _STATE_PARSER_RECEIVE__COMMAND,
    _STATE_PARSER_RECEIVE__SIZE,
    _STATE_PARSER_RECEIVE__DATA,
    _STATE_PARSER_RECEIVE__CHECKSUM_1,
    _STATE_PARSER_RECEIVE__CHECKSUM_2,
    _STATE_PARSER_RECEIVE__VERIFICATION,
    _STATE_PARSER_RECEIVE__DONE,
} _comm_receive_states_t;

typedef enum _comm_process_states_en
{
    _STATE_PARSER_PROCESS__NONE = 0,
    _STATE_PARSER_PROCESS__INIT,
    _STATE_PARSER_PROCESS__FIND,
    _STATE_PARSER_PROCESS__EXECUTE,
    _STATE_PARSER_PROCESS__DONE,
} _comm_process_states_t;

typedef enum _comm_transmit_states_en
{
    _STATE_PARSER_TRANSMIT__NONE = 0,
    _STATE_PARSER_TRANSMIT__INIT,
    _STATE_PARSER_TRANSMIT__PREAMBLE_1,
    _STATE_PARSER_TRANSMIT__PREAMBLE_2,
    _STATE_PARSER_TRANSMIT__COMMAND,
    _STATE_PARSER_TRANSMIT__SIZE,
    _STATE_PARSER_TRANSMIT__DATA,
    _STATE_PARSER_TRANSMIT__CHECKSUM_1,
    _STATE_PARSER_TRANSMIT__CHECKSUM_2,
    _STATE_PARSER_TRANSMIT__DONE,
} _comm_transmit_states_t;

typedef enum _comm_error_states_en
{
    _STATE_PARSER_ERROR__NONE = 0,
    _STATE_PARSER_ERROR__INIT,
    _STATE_PARSER_ERROR__PREAMBLE_1,
    _STATE_PARSER_ERROR__PREAMBLE_2,
    _STATE_PARSER_ERROR__COMMAND,
    _STATE_PARSER_ERROR__SIZE,
    _STATE_PARSER_ERROR__ERROR_CATEGORY,
    _STATE_PARSER_ERROR__ERROR_CODE,
    _STATE_PARSER_ERROR__CHECKSUM_1,
    _STATE_PARSER_ERROR__CHECKSUM_2,
    _STATE_PARSER_ERROR__DONE,
} _comm_error_states_t;

typedef enum _command_results_en
{
    _COMMAND_RESULT__ERROR = -1,
    _COMMAND_RESULT__SUCCESS = 0,
    _COMMAND_RESULT__YIELD = 1,
} _command_result_t;


//union {
//    uint16_t value;
//    struct {
//        uint16_t low: 8;
//        uint16_t high: 8;
//    } byte;
//} size;


//    union {
//      uint16_t value;
//      uint8_t bytes[2];
//    }command;

typedef struct _packet_cmd_st
{
    uint16_t preamble;
    uint16_t checksum;


    union {
        uint16_t value;
        struct {
            uint16_t low: 8;
            uint16_t high: 8;
        } byte;
    }command;
    union {
        uint16_t value;
        struct {
            uint16_t low: 8;
            uint16_t high: 8;
        } byte;
    }size;
    uint8_t data[_COMMAND_FIELD_SIZE__DATA];
} _packet_cmd_t;

typedef struct _packet_control_st
{
    _packet_cmd_t packet;
    uint16_t checksum_computed;
    uint16_t idx;
} _packet_control_t;

typedef struct _timeout_control_st
{
    time_t scheduled;
    bool on_off;
} _timeout_control_t;

typedef struct _comm_control_st
{
    const command_comm_map_t *map_cmds;
    _comm_receive_states_t receive_state;
    _comm_transmit_states_t transmit_state;
    _comm_error_states_t error_state;
    _comm_process_states_t process_state;
    uint16_t map_count;
    uint16_t map_idx;
    bool initialized;
    uint8_t error_category;
    uint8_t error_code;

} _comm_control_t;

extern _packet_control_t _pkt_ctrl;
extern _timeout_control_t _timeout_ctrl;
extern _comm_control_t _comm_ctrl;

#endif
