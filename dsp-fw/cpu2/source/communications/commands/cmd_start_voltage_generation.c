#include <math.h>
#include <string.h>
#include <stdio.h>

#include "application.h"
#include "errors.h"
#include "generic_definitions.h"
#include "types.h"
#include "commands.h"

#define _CMD_ANSWER_AMOUNT  (1)

#define _CMD_ANSWER         0xB101



int16_t cmd_start_voltage_generation(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    uint16_t idx=0;

    if (*size > 0)
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    send_command_to_CPU1(3,NULL);



    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}
