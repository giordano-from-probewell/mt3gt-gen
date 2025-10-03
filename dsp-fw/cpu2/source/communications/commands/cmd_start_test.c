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

#define _CMD_ANSWER         0x8000|0x5100

int16_t cmd_start_test(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    uint16_t idx=0;
    uint16_t i=0;

    if (*size != 1 )
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    int8_t x=0;
    ipc_enqueue_to_cpu1(IPC_CMD_START_TEST, &x, 1);



    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



