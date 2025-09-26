#include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "types.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"
#include "app_cpu2.h"
#include "ipc_simple.h"

#define _CMD_REQUEST_AMOUNT (4)
#define _CMD_ANSWER_AMOUNT  (0)

#define _CMD_ANSWER         0x8000|0x5001

int16_t cmd_set_calibration(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    int parameter;
    int phase, phase1;
    int index;
    myfloat_t tmp;
    uint16_t i=0;

    if (*size != 7 )
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    parameter   = data[i+0];
    phase       = data[i+1];
    if(phase == 0x0A) phase1 = 1;
    if(phase == 0x0B) phase1 = 2;
    if(phase == 0x0C) phase1 = 3;
    index       = data[i+2];

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

    tmp.byte.b3 = data[i+3];
    tmp.byte.b2 = data[i+4];
    tmp.byte.b1 = data[i+5];
    tmp.byte.b0 = data[i+6];




    uint8_t new_cal[3] = {parameter, phase1, index};
    ipc_send_to_cpu1(IPC_CMD_SET_CAL, &new_cal, 3);



    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(i);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



