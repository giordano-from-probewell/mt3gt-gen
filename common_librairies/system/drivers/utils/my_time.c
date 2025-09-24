/*
 * my_time.c
 *
 *  Created on: 23 févr. 2023
 *      Author: GiordanoWolaniuk
 */



#include "my_time.h"
#include "F28x_Project.h"

//microsseconds time for tasks scheduling
#pragma CODE_SECTION (my_time, ".TI.ramfunc")
_CODE_ACCESS my_time_t my_time(my_time_t *timer)
{
    my_time_t result = INT32_MAX - CpuTimer2Regs.TIM.all;

    if (timer)
        *timer = result;

    return (result);
}
