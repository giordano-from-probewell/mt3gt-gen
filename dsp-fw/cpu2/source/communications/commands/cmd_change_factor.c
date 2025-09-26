#include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "types.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"
//#include "cla1_standard_shared.h"

#define _CMD_REQUEST_AMOUNT (1)
#define _CMD_ANSWER_AMOUNT  (0)

#define _CMD_ANSWER         0x8000|0x5301

int16_t cmd_change_factor(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    myfloat_t tmp;
    uint16_t idx=0;
    uint16_t i=0;

    if (*size != 8*8 )  // 8 ch (1 num and 1 den) = 64 floats
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }


    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



