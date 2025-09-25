/*
 * pw_IIRfilter.c
 *
 *  Created on: May 27, 2024
 *      Author: thiago.greboge
 */

#include "filter.h"


float32_t IIR_FRQ[51] = {                                  // Frequencies
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    71, 72, 73, 74, 75, 76, 77, 78, 79, 80};

float32_t IIR_PSH[51] = {                              // Phase shifts
    -1.43828, -1.48424, -1.529, -1.57251, -1.61471, -1.65558, -1.6951, -1.73326, -1.77006, -1.80552,
    -1.83966, -1.87251, -1.90409, -1.93445, -1.96362, -1.99165, -2.01857, -2.04444, -2.0693, -2.09318,
    -2.11614, -2.13821, -2.15943, -2.17984, -2.19949, -2.2184, -2.23661, -2.25416, -2.27107, -2.28738,
    -2.30311, -2.31829, -2.33295, -2.34711, -2.3608, -2.37404, -2.38684, -2.39922, -2.41122, -2.42283,
    -2.43409, -2.445, -2.45559, -2.46585, -2.47582, -2.4855, -2.49489, -2.50403, -2.51291, -2.52154, -2.52994};

/*
 *  Calculate the phase shift caused by the IIR filter between 30 and 80Hz
 *  Input in Hz and output in rad
 */





float32_t iir_filter(filter_iir_t *filter, float32_t in) {
    // Shift the old samples
    int i;
    for (i = 4; i > 0; i--) {
        filter->x[i] = filter->x[i - 1];
        filter->y[i] = filter->y[i - 1];
    }
    filter->x[0] = in;

    // Compute the new output
    filter->y[0] = filter->b[0] * filter->x[0] +
                   filter->b[1] * filter->x[1] +
                   filter->b[2] * filter->x[2] +
                   filter->b[3] * filter->x[3] +
                   filter->b[4] * filter->x[4] -
                   filter->a[1] * filter->y[1] -
                   filter->a[2] * filter->y[2] -
                   filter->a[3] * filter->y[3] -
                   filter->a[4] * filter->y[4];

    filter->out = filter->y[0];
    return filter->out;
}


/**
 * @brief Apply FIR filter to the input sample.
 * @param filter Pointer to the FIR filter structure.
 * @param in New input sample to be filtered.
 * @return Filtered output sample.
 */
float32_t fir_filter(filter_fir_t *filter, float32_t in) {
    // Shift the old samples
    int i;
    for (i = filter->order; i > 0; i--) {
        filter->x[i] = filter->x[i - 1];
    }
    filter->x[0] = in;

    // Compute the new output
    filter->out = 0.0f;
    for (i = 0; i <= filter->order; i++) {
        filter->out += filter->coeffs[i] * filter->x[i];
    }

    return filter->out;
}





//float32_t interpolate_phase_shift(float32_t frequency)
//{
//    uint8_t i = 0;
//
//    if (frequency < 30 || frequency > 80) {         // Out of interval for fundamental frequency
//        return 0.0;                                 // Return 0
//    }
//
//    for (i = 0; i < sizeof(IIR_FRQ) - 1; i++) {
//        if (frequency >= IIR_FRQ[i] && frequency <= IIR_FRQ[i + 1]) {
//            float32_t x1 = IIR_FRQ[i];
//            float32_t x2 = IIR_FRQ[i + 1];
//            float32_t y1 = IIR_PSH[i];
//            float32_t y2 = IIR_PSH[i + 1];
//            return y1 + (y2 - y1) * (frequency - x1) / (x2 - x1);
//        }
//    }
//    return 0.0;
//}
