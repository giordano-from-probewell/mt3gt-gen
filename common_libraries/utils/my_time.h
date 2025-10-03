/*
 * my_time.h
 *
 *  Created on: 23 févr. 2023
 *      Author: GiordanoWolaniuk
 */


#ifndef _TIME
#define _TIME

#include "linkage.h"

#ifndef NULL
#define NULL      0
#endif

#ifdef __cplusplus
extern "C" namespace std {
#endif

/* milliseconds based in cpu clicks */
typedef unsigned long my_time_t; // em milisegundos


/****************************************************************************/
/* FUNCTION DECLARATIONS.                                                   */
/****************************************************************************/
_CODE_ACCESS my_time_t     my_time (my_time_t *_timer);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _TIME */

#if defined(__cplusplus) && !defined(_CPP_STYLE_HEADER)
using std::my_time_t;
using std::my_time;
#endif
