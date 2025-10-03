#include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "types.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"

#include "ipc_simple.h"

#define _CMD_REQUEST_AMOUNT (1)
#define _CMD_ANSWER_AMOUNT  (0)

#define _CMD_ANSWER         0x8000|0x5302

int16_t cmd_current_range(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    uint16_t i=0;
    uint16_t idx=0;
    int16_t range = -1;

    if (*size != 1 )
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    range = data[i+0];
    if(range < 0 || range > 3)
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }




//    if (amplifier_system_ready())
//    {
//
//        if(range == 0)
//        {
//            amplifier_request_gain(AFE_IA, IN_GAIN_8, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IB, IN_GAIN_8, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IC, IN_GAIN_8, OUT_GAIN_1);
//
//        }
//        else if (range == 1)
//        {
//            amplifier_request_gain(AFE_IA, IN_GAIN_1, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IB, IN_GAIN_1, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IC, IN_GAIN_1, OUT_GAIN_1);
//        }
//        else if(range == 2)
//        {
//            amplifier_request_gain(AFE_IA, IN_GAIN_0_125, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IB, IN_GAIN_0_125, OUT_GAIN_1);
//            amplifier_request_gain(AFE_IC, IN_GAIN_0_125, OUT_GAIN_1);
//        }
//
//    }
//    else
//    {
//        *error_category = ERROR(ERROR_EQUIPMENT);
//        *error_code = ERROR(ERROR_EQUIPMENT__AMPLIFIER_IS_BUSY);
//        return (COMMAND_RESULT__ERROR);
//    }


    ipc_enqueue_to_cpu1(IPC_CMD_SET_CURRENT_RANGE, &range, 1);


    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



