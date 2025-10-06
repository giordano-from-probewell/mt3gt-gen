#ifndef __GENERIC_DEFINITIONS_H_
#define __GENERIC_DEFINITIONS_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#include "my_time.h"

extern char *INITIALIZED;


#define WAITSTEP                  asm(" RPT #255 || NOP")


typedef enum generic_status_en
{
    STATUS_TIMEOUT          = -3,
    STATUS_ERROR            = -2,
    STATUS_INVALID_INFO     = -1,
    STATUS_NOT_INITIALIZED  = 0,
    STATUS_PREPARING        = 1,
    STATUS_PROCESSING       = 2,
    STATUS_WAITING          = 3,
    STATUS_FINISHING        = 4,
    STATUS_DONE             = 5,
    STATUS_BUSY             = 6,

} generic_status_t;

typedef  enum generic_states_en
{
    STATE_INIT             = 0,
    STATE_RESET            = 1,
    STATE_IDLE             = 2,
    STATE_EXEC             = 3,
    STATE_COMPLETE         = 4,
    STATE_NOT_INITIALIZED  = -1,
    STATE_ERROR            = -2,

} generic_states_t;






void setupProfilingGPIO(void);

#endif
