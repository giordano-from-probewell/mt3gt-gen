#include "errors.h"

#include "_comm_internals.h"

const uint16_t _COMMAND_PREAMBLE__FULL = 0x5057;
const uint8_t _COMMAND_PREAMBLE__PART_1 = 0x50;
const uint8_t _COMMAND_PREAMBLE__PART_2 = 0x57;

_packet_control_t _pkt_ctrl;

_timeout_control_t _timeout_ctrl = { .on_off = false, };

_comm_control_t _comm_ctrl = { .initialized = false, };

