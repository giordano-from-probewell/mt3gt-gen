#include "errors.h"

#include "commands.h"
#include "application.h"
#include "conversions.h"

#define _CMD_REQUEST_AMOUNT (0)

#define _CMD_ANSWER         0xA00C

int16_t cmd_get_scale_current(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{
    if (*size != SIZE(_CMD_REQUEST_AMOUNT))
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_WRONG_PARAMS_NUMBER);
        return (COMMAND_RESULT__ERROR);
    }

    //float32_to_uint8_t(data, app.generation.current.config.scale_requested);

    *size = SIZE(4);
    *command = _CMD_ANSWER;

    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    return (COMMAND_RESULT__SUCCESS);
}
