#include "errors.h"

#include "commands.h"
#include "application.h"

#define _CMD_REQUEST_AMOUNT (0)

#define _CMD_ANSWER         0xA00E

int16_t cmd_get_status(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    if (*size != SIZE(_CMD_REQUEST_AMOUNT))
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_WRONG_PARAMS_NUMBER);
        return (COMMAND_RESULT__ERROR);
    }

  //  data[0] = app.generation.voltage.command.enable;
  //  data[1] = app.generation.current.command.enable;

    *size = SIZE(2);
    *command = _CMD_ANSWER;
    
    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    return (COMMAND_RESULT__SUCCESS);
}

