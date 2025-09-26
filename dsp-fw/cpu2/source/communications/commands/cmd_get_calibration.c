#include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"



#define _CMD_ANSWER         (0x8000|0x5304)

int16_t cmd_get_calibration(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    int parameter;
    int phase, phase1;
    int index;
    myfloat_t tmp;
    uint16_t idx=0;


    if (*size != 3 )
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    parameter   = data[idx+0];
    phase       = data[idx+1];
    if(phase == 0x0A) phase1 = 1;
    if(phase == 0x0B) phase1 = 2;
    if(phase == 0x0C) phase1 = 3;
    index       = data[idx+2];

    if(parameter<0 || parameter>3){
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_INVALID_PARAM);
        return (COMMAND_RESULT__ERROR);
    }
    if(phase<0x0A || parameter>0x0C){
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_INVALID_PARAM);
        return (COMMAND_RESULT__ERROR);
    }
    if(index<0 || index>3){
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_INVALID_PARAM);
        return (COMMAND_RESULT__ERROR);
    }




    idx = 0;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



