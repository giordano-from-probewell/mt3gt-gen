#ifndef _SPLINE_H_
#define _SPLINE_H_

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "hw_ints.h"
#include "hw_types.h"
#include "types.h"

/** Numerical Analysis 9th ed - Burden, Faires (Ch. 3 Natural Cubic Spline, Pg. 149) */
#define SPLINE_VECTOR_SIZE (1024+1+20) // num of points in one cycle + extra 10 points on head/tail

extern volatile float a[SPLINE_VECTOR_SIZE];
extern volatile float c[SPLINE_VECTOR_SIZE];
extern volatile float b[SPLINE_VECTOR_SIZE];
extern volatile float d[SPLINE_VECTOR_SIZE];

void calc_spline_coef(float t[], float y[], int n);
void calc_spline_coef_ring_idx(float32_t *ring, int cap, int cnt,
                               int last, int mask);

float calc_spline_value(float xi, int n);



#endif
