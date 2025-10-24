#include <math.h>
#include <string.h>
#include <stdio.h>

#include "application.h"
#include "errors.h"
#include "generic_definitions.h"
#include "types.h"
#include "commands.h"

#define _CMD_ANSWER_AMOUNT  (1)

#define _CMD_ANSWER         0xA006



int16_t cmd_set_scale_current(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    myfloat_t tmp;
    uint16_t idx=0;
    uint16_t i=0;

    if (*size != 4)
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }



    tmp.byte.b3 = data[i+0];
    tmp.byte.b2 = data[i+1];
    tmp.byte.b1 = data[i+2];
    tmp.byte.b0 = data[i+3];

    if( tmp.value <= 0.0f )
        tmp.value = 0.0f;
    if( tmp.value >= CURRENT_MAX )
            tmp.value = CURRENT_MAX;

    //send_command_to_CPU1(8, tmp.value);



    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}

