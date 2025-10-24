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

#define _CMD_ANSWER         0xA005


static void cmd_set_scale_voltage_cpu2(float scale_value) {
    uint8_t channel = 0;  // 0 = voltage, 1 = current
    IPC_SEND_GEN_SET_SCALE(channel, scale_value);
    CLI_LOGI("Voltage scale %.2f sent to CPU1", scale_value);
}

//void cmd_set_scale_voltage_manual(float scale_value) {
//    uint8_t payload[5];
//    payload[0] = 0;  // Canal voltage
//    memcpy(&payload[1], &scale_value, sizeof(float));
//
//    ipc_send_generation_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
//}

//void cmd_set_scale_voltage_lowlevel(float scale_value) {
//    uint8_t payload[5];
//    payload[0] = 0;  // Canal voltage
//    memcpy(&payload[1], &scale_value, sizeof(float));
//
//    ipc_send_raw_cmd(IPC_CMD_GEN_SET_SCALE, payload, 5);
//}

int16_t cmd_set_scale_voltage(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    myfloat_t tmp;
    uint16_t idx=0;
    uint16_t i=0;

    if (*size != 4 )
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
    //if( tmp.value >= VOLTAGE_MAX )
    //        tmp.value = VOLTAGE_MAX;

    cmd_set_scale_voltage_cpu2(tmp.value);

    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}

