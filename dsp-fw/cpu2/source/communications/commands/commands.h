#include <stdint.h>
#include "hw_types.h"

#ifndef COMMANDS_H
#define COMMANDS_H
#define ERROR(error_code)   ((uint16_t)(error_code))
#define DATA(data_value)    ((uint16_t)(data_value))
#define SIZE(amount)        ((uint16_t)(amount))

#define CMD_PARAM(cast_type, cmd_data_value)   ((cast_type)(cmd_data_value))

enum commands_en
{
    COMMAND_PING                            = 0x0001,
    COMMAND_IDENTIFY                        = 0x0002,
    COMMAND_START_TX_PULSE                  = 0x0003,
    COMMAND_SET_KH                          = 0x5001,
    COMMAND_GET_KH                          = 0x5002,
    COMMAND_START_TEST                      = 0x5100,
    COMMAND_RESET_TEST                      = 0x5101,
    COMMAND_GET_MEASURES                    = 0x5200,
    COMMAND_GET_INST_MEASURES               = 0x5201,
    COMMAND_CHANGE_FACTOR                   = 0x5301,
    COMMAND_CURRENT_RANGE                   = 0x5302,
    COMMAND_GET_CALIBRATIONS                = 0x5303,
    COMMAND_GET_CALIBRATION                 = 0x5304,
    COMMAND_SET_CALIBRATION                 = 0x5305,

};

enum command_results_en
{
    COMMAND_RESULT__ERROR       = -1,
    COMMAND_RESULT__SUCCESS     =  0,
    COMMAND_RESULT__YIELD       =  1,
};

extern int16_t cmd_ping                         (uint8_t *error_category,uint8_t *error_code,uint16_t *Command,uint8_t *data,uint16_t *size);
extern int16_t cmd_identify                     (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);



#endif
