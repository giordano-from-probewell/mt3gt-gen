/** Numerical Analysis 9th ed - Burden, Faires (Ch. 3 Natural Cubic Spline, Pg. 149) */
// https://en.wikipedia.org/wiki/Spline_(mathematics)
// code for calculation of spline with Homogeneous delta x, equal sampling rate

#include <stdio.h>
#include <math.h>

#include "spline.h"

volatile float x[SPLINE_VECTOR_SIZE]; // data x input points

volatile float A[SPLINE_VECTOR_SIZE]; // coeficients of tridiagonal matrix
volatile float l[SPLINE_VECTOR_SIZE]; // coeficients of tridiagonal matrix
volatile float u[SPLINE_VECTOR_SIZE]; // coeficients of tridiagonal matrix
volatile float z[SPLINE_VECTOR_SIZE]; // coeficients of tridiagonal matrix

volatile float a[SPLINE_VECTOR_SIZE]; // copy of data y input points = zero order coeficient of cubic spline segments
volatile float b[SPLINE_VECTOR_SIZE]; // first order coeficient of cubic spline segments
volatile float c[SPLINE_VECTOR_SIZE]; // second order coeficient of cubic spline segments
volatile float d[SPLINE_VECTOR_SIZE]; // third order coeficient of cubic spline segments

#pragma CODE_SECTION(search_index, "ramfuncs");
int search_index(float xi, int n)
{
    int i;

    if (xi < x[0]) return -1;
    if (xi > x[n-1]) return -1;

    for (i = 0; i < n-1; i++)
    {
        if (xi >= x[i] && xi < x[i+1])
            return i;
    }
    return n - 1;
}

// calc_spline_coef must be called first
// x, a, b, c, d : must be calculated

// x = input x vector
// a = input y vector

// returns yi = (xi - xo)*d^3 + (xi - xo)*c^2 + (xi - xo)*b^1 + a;
#pragma CODE_SECTION(calc_spline_value, "ramfuncs");
float calc_spline_value(float xi, int n)
{
    float yi;
    float di, ci, bi, ai;
    float dx;

    int idx;

    // given xi, must find the index of x that defines the spline segment 
    //idx = search_index(xi, n); 
    idx = floor(xi); // se o espaçamento do vetor ti é unitário, então simplifica aqui

    // if xi is out of range, return NAN
    // if (idx == -1) return NAN;

    dx = xi - x[idx];

    di = d[idx];
    ci = c[idx];
    bi = b[idx];
    ai = a[idx];

    yi = di * dx * dx * dx  + ci * dx * dx  + bi * dx  + ai;

    return yi;
}



// t = input time vector
// y = input amplitude vector

// yi = (xi - xo)*d^3 + (xi - xo)*c^2 + (xi - xo)*b^1 + a;
// calculate the coeficients [d,c,b,a]
#pragma CODE_SECTION(calc_spline_coef, "ramfuncs");
void calc_spline_coef(float t[], float y[], int n)
{
    int i, j;

    //copy the y to a vector
    for (i = 0; i < n; ++i)
    {
        x[i] = t[i];
        a[i] = y[i];
    }

    n--; //number of points minus 1 is the number of cubic functions

    // sampling interval (as sample rate is constant, we can optimize here)
    // all h[i] should be equal to sampling period, normalized here to hi = 1;

    //   for (i = 0; i <= n - 1; ++i)
    //       h[i] = x[i + 1] - x[i];

    // coeficients of tridiagonal matrix
    for (i = 1; i <= n - 1; ++i)
    {
        A[i] = 3.0f * (a[i + 1] - a[i]) - 3.0f * (a[i] - a[i - 1]); // hi = 1
    }
    // coeficients of tridiagonal matrix
    l[0] = 1.0f;
    u[0] = 0.0f;
    z[0] = 0.0f;

    for (i = 1; i <= n - 1; ++i) 
    {
        l[i] = 4.0f - u[i - 1]; //l[i] = 2.0 * (x[i + 1] - x[i - 1]) - u[i - 1]; but hi = 1 and dX = 2
        u[i] = 1 / l[i]; 
        z[i] = (A[i] - z[i - 1]) / l[i]; 
    }

    // tri-diagonal matrix factorization 
    l[n] = 1.0f;
    z[n] = 0.0f;
    c[n] = 0.0f;

    for (j = n - 1; j >= 0; --j) 
    {
        c[j] = z[j] - u[j] * c[j + 1];
        b[j] = (a[j + 1] - a[j]) - (c[j + 1] + 2 * c[j]) / 3.0f;
        d[j] = (c[j + 1] - c[j]) / 3.0f;
    }
}




// cap  = ring capacity must by 2^x
// cnt  = n of samples
// last = last sample index
// mask = (cap-1)
#pragma CODE_SECTION(calc_spline_coef_ring_idx, "ramfuncs");
void calc_spline_coef_ring_idx(float32_t *ring, int cap, int cnt,
                               int last, int mask)
{
    int i, j;
    int n = cnt;
    int start = last - (cnt - 1);
    if (start < 0) start += cap;

    // linear vector
    if (mask >= 0) { // (cap-1) must be 2^x
        for (i = 0; i < n; ++i) {
            int idx = (start + i) & mask;
            x[i] = (float)i;
            a[i] = ring[idx];
        }
    } else {         // cap -> use %
        for (i = 0; i < n; ++i) {
            int idx = (start + i) % cap;
            x[i] = (float)i;
            a[i] = ring[idx];
        }
    }

    // === original code ===
    n--; // n o cubic segments

    for (i = 1; i <= n - 1; ++i)
        A[i] = 3.0f * (a[i + 1] - a[i]) - 3.0f * (a[i] - a[i - 1]); // hi = 1

    l[0] = 1.0f; u[0] = 0.0f; z[0] = 0.0f;

    for (i = 1; i <= n - 1; ++i) {
        l[i] = 4.0f - u[i - 1];
        u[i] = 1.0f / l[i];
        z[i] = (A[i] - z[i - 1]) / l[i];
    }

    l[n] = 1.0f; z[n] = 0.0f; c[n] = 0.0f;

    for (j = n - 1; j >= 0; --j) {
        c[j] = z[j] - u[j] * c[j + 1];
        b[j] = (a[j + 1] - a[j]) - (c[j + 1] + 2.0f * c[j]) / 3.0f;
        d[j] = (c[j + 1] - c[j]) / 3.0f;
    }
}
