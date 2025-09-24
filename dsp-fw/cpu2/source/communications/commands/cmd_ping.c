#include "errors.h"

#include "commands.h"

#define _CMD_REQUEST_AMOUNT (0)
#define _CMD_ANSWER_AMOUNT  (0)

#define _CMD_ANSWER         0x8001

int16_t cmd_ping(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    if (*size != SIZE(_CMD_REQUEST_AMOUNT))
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_WRONG_PARAMS_NUMBER);
        return (COMMAND_RESULT__ERROR);
    }
    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(_CMD_ANSWER_AMOUNT);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}
