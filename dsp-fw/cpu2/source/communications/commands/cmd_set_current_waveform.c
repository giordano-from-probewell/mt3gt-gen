#include <math.h>
#include <string.h>
#include <stdio.h>

#include "application.h"
#include "errors.h"
#include "generic_definitions.h"
#include "types.h"
#include "commands.h"

#define _CMD_ANSWER_AMOUNT  (1)

#define _CMD_ANSWER         0xA004



int16_t cmd_set_current_waveform(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    uint16_t idx=0;
    myfloat_t tmp;
    uint16_t i=0,j=0;
    uint16_t data_size = (uint16_t)(*size);

    if (*size > SIZE(REF_GEN_MAX_SIZE*4))
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

    //copy protocol data IEEE754 to C2000 memory format
    //Ex: 0.00628300011 - IEEE754: 0x3BCDE1A0 - C2000: [0xE1A0][0x3BCD]
    for(i=0; i<data_size; i+=4)
    {
        tmp.byte.b3 = data[i+0];
        tmp.byte.b2 = data[i+1];
        tmp.byte.b1 = data[i+2];
        tmp.byte.b0 = data[i+3];
        if( tmp.value>1.0|| tmp.value<-1.0)
        {
            data [ idx++ ] = 0xEE;
            data [ idx++ ] = (uint8_t)(i>>8);
            data [ idx++ ] = (uint8_t)(i);
            data [ idx++ ] = data[i+0];
            data [ idx++ ] = data[i+1];
            data [ idx++ ] = data[i+2];
            data [ idx++ ] = data[i+3];


            *error_category = ERROR(ERROR_CATEGORY_NONE);
            *error_code = ERROR(ERROR_CODE_NONE);
            *size = SIZE(idx);
            *command = _CMD_ANSWER;
            return (COMMAND_RESULT__SUCCESS);

        }

        //app.generation.current.ref.waveform1[j]=tmp.value;
        //app.generation.current.ref.waveform2[j]=tmp.value;
        j++;
    }





    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}
