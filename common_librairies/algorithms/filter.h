#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "types.h"
#include "generic_definitions.h"


typedef struct filter_iir_st
{
    float32_t a[5];
    float32_t b[5];
    float32_t x[5];   // Old inputs
    float32_t y[5];   // Old outputs
    float32_t out;

} filter_iir_t;

//filter_iir_t filter = {
//    .a = {1.000000000000000, -4.614493103854256, 8.531237084959097, -7.898293500298919, 3.661438265075875, -0.679868828631250},
//    .b = {6.224140795707157e-07, 3.112070397853578e-06, 6.224140795707157e-06, 6.224140795707157e-06, 3.112070397853578e-06, 6.224140795707157e-07},
//    .x = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
//    .y = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
//    .out = 0.0f
//};



typedef struct filter_fir_st {
    float32_t coeffs[21]; // Filter coefficients (b0, b1, ..., b20)
    float32_t x[21];      // Previous inputs (x[n], x[n-1], ..., x[n-20])
    float32_t out;        // Output of the filter
    int order;
} filter_fir_t;


//filter_fir_t filter = {
//    .coeffs = {1.318812418137423e-02, 2.002057839334476e-02, 2.768669066933868e-02, 3.587548510633092e-02, 4.420834927019446e-02,
//               5.226387629153584e-02, 5.960780797043170e-02, 6.582550631746176e-02, 7.055404686179979e-02, 7.351098440778829e-02,
//               7.451710106079924e-02, 7.351098440778829e-02, 7.055404686179979e-02, 6.582550631746177e-02, 5.960780797043170e-02,
//               5.226387629153584e-02, 4.420834927019446e-02, 3.587548510633092e-02, 2.768669066933868e-02, 2.002057839334476e-02,
//               1.318812418137423e-02},
//    .x = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
//    .out = 0.0f
//};



float32_t iir_filter(filter_iir_t * filter, float32_t in);
float32_t fir_filter(filter_fir_t *filter, float32_t in) ;
//float32_t interpolate_phase_shift(float32_t frequency);

#endif /* _FILTER_H_ */
