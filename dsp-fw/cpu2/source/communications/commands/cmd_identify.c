#include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"

#define _CMD_REQUEST_AMOUNT (0)
#define _CMD_ANSWER_AMOUNT  (48)

#define _CMD_ANSWER         0x8002

int16_t cmd_identify(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    uint16_t idx, j;

    if (*size != SIZE(_CMD_REQUEST_AMOUNT))
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_WRONG_PARAMS_NUMBER);
        return (COMMAND_RESULT__ERROR);
    }

    idx = 0;

    for (j=0;j<16;j++)
        data [ idx++ ] = app.id.data.full.probewell_part_number[j];
    for (j=0;j<16;j++)
        data [ idx++ ] = app.id.data.full.serial_number[j];
    for (j=0;j<8;j++)
        data [ idx++ ] = app.id.data.full.fabrication_date[j];
    for (j=0;j<8;j++)
        data [ idx++ ] = app.id.data.full.last_verfication_date[j];
    data [ idx++ ] = (uint8_t)app.id.data.full.comm_buffer_size>>8;
    data [ idx++ ] = (uint8_t)app.id.data.full.comm_buffer_size;

    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    //todo: size must be _CMD_ANSWER_AMOUNT
    *size = SIZE (idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



