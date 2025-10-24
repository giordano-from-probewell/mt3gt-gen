#include <math.h>
#include <string.h>
#include <stdio.h>

#include "application.h"
#include "errors.h"
#include "generic_definitions.h"
#include "types.h"
#include "commands.h"
#include "ipc_comm.h"
#include "ipc_handlers.h"
#include "cli.h"

#define _CMD_ANSWER_AMOUNT  (1)

#define _CMD_ANSWER         0xB001

static void cmd_stop_voltage_gen_cpu2(void) {
    uint8_t channel = 0;  // 0 = voltage, 1 = current
    IPC_SEND_GEN_DISABLE(channel);
    CLI_LOGI("Voltage gen stop sent to CPU1");
}

int16_t cmd_stop_voltage_generation(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    uint16_t idx=0;

    if (*size > 0)
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    cmd_stop_voltage_gen_cpu2();


    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}
