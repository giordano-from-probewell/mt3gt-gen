#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdint.h>
#include "hw_types.h"


#define ERROR(error_code)   ((uint16_t)(error_code))
#define DATA(data_value)    ((uint16_t)(data_value))
#define SIZE(amount)        ((uint16_t)(amount))

#define CMD_PARAM(cast_type, cmd_data_value)   ((cast_type)(cmd_data_value))

enum commands_en
{
    // Universal commands
    COMMAND_PING                            = 0x0001,
    COMMAND_IDENTIFY                        = 0x0002,

    // Setters
    COMMAND_RESET_VOLTAGE_WAVEFORM          = 0x1001,
    COMMAND_RESET_CURRENT_WAVEFORM          = 0x1002,
    COMMAND_SET_VOLTAGE_WAVEFORM            = 0x1003,
    COMMAND_SET_CURRENT_WAVEFORM            = 0x1004,
    COMMAND_SET_VOLTAGE_SCALE               = 0x1005,
    COMMAND_SET_CURRENT_SCALE               = 0x1006,
    COMMAND_SET_FREQUENCY                   = 0x1007,
    COMMAND_SET_CALIBRATION                 = 0x1008,

    //Getters
    COMMAND_GET_VOLTAGE_WAVEFORM            = 0x1009,
    COMMAND_GET_CURRENT_WAVEFORM            = 0x100A,
    COMMAND_GET_VOLTAGE_SCALE               = 0x100B,
    COMMAND_GET_CURRENT_SCALE               = 0x100C,
    COMMAND_GET_FREQUENCY                   = 0x100D,
    COMMAND_GET_STATUS                      = 0x100E,
    COMMAND_GET_ERROR                       = 0x100F,
    COMMAND_GET_MEASURES                    = 0x1010,
    COMMAND_GET_CALIBRATION                 = 0x1011,

    //Control
    COMMAND_STOP_VOLTAGE_GENERATION         = 0x2001,
    COMMAND_STOP_CURRENT_GENERATION         = 0x2002,
    COMMAND_STOP_GENERATION                 = 0x2003,
    COMMAND_GPIO1_CLEAR                     = 0x2004,

    COMMAND_START_VOLTAGE_GENERATION        = 0x2101,
    COMMAND_START_CURRENT_GENERATION        = 0x2102,
    COMMAND_START_GENERATION                = 0x2103,
    COMMAND_GPIO1_SET                       = 0x2104,

};

enum command_results_en
{
    COMMAND_RESULT__ERROR       = -1,
    COMMAND_RESULT__SUCCESS     =  0,
    COMMAND_RESULT__YIELD       =  1,
};


extern int16_t cmd_ping                         (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_identify                     (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_reset_voltage_waveform       (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_reset_current_waveform       (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_voltage_waveform         (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_voltage_waveform         (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_current_waveform         (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_current_waveform         (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_stop_generation              (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_start_generation             (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_start_voltage_generation     (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_start_current_generation     (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_stop_voltage_generation      (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_stop_current_generation      (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_scale_voltage            (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_scale_voltage            (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_scale_current            (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_scale_current            (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_frequency                (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_frequency                (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_status                   (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_error                    (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_measures                 (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_get_calibration              (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_set_calibration              (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_gpio1_clear                  (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);
extern int16_t cmd_gpio1_set                    (uint8_t *error_category,uint8_t *error_code,uint16_t *command,uint8_t *data,uint16_t *size);






#endif
