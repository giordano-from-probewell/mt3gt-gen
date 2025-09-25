
#ifndef _OTHERS_H_
#define _OTHERS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "types.h"


/**
 * @brief Detect zero-crossing in the filtered output.
 * @param in Current output sample of the filter.
 * old is an internal variable, the previous output sample of the filter.
 * @return True if a zero-crossing is detected, false otherwise.
 */
bool detect_zero_crossing(float32_t in);

#endif
