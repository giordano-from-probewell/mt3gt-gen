#include "errors.h"

#include "commands.h"
#include "application.h"

#define _CMD_REQUEST_AMOUNT (0)

#define _CMD_ANSWER         0xA00F

int16_t cmd_get_error(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    *size = SIZE(4);
    *command = 0x6666;

    return (COMMAND_RESULT__SUCCESS);
}

