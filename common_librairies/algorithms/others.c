#include "others.h"
#include "filter.h"


typedef struct zerocrossing_st
{
    int32_t decimation;
    int32_t decimation_index;
    filter_fir_t filter;
    float32_t filtered_value;
    float32_t previous_filtered_value;

} zerocrossing_t;



bool detect_zero_crossing(float32_t value) {

    bool ret = false;

    static zerocrossing_t zc = {
        .decimation = 10, // once each 10 samples
        .decimation_index = 0,
        .filter = { //lowpass order 20 fir for 80 Hz in 52734/10 sps (from octave auxiliary)
            .coeffs = {1.318812418137423e-02, 2.002057839334476e-02, 2.768669066933868e-02, 3.587548510633092e-02, 4.420834927019446e-02,
                       5.226387629153584e-02, 5.960780797043170e-02, 6.582550631746176e-02, 7.055404686179979e-02, 7.351098440778829e-02,
                       7.451710106079924e-02, 7.351098440778829e-02, 7.055404686179979e-02, 6.582550631746177e-02, 5.960780797043170e-02,
                       5.226387629153584e-02, 4.420834927019446e-02, 3.587548510633092e-02, 2.768669066933868e-02, 2.002057839334476e-02,
                       1.318812418137423e-02},
            .x = {0.0f}, // Initializing to zero
            .out = 0.0f,
            .order = 20,
        },
        .previous_filtered_value = 0.0f,
    };

//        static zerocrossing_t zc = {
//            .decimation = 30, // once each 10 samples
//            .decimation_index = 0,
//            .filter = { //lowpass order 12 fir for 80 Hz in 52734/10 sps (from octave auxiliary)
//                .coeffs = {        4.386712286926205e-02,   1.268892042680077e-01,   2.081684185134729e-01,   2.421505086985148e-01,   2.081684185134729e-01,   1.268892042680077e-01,   4.386712286926205e-02
//                },
//                .x = {0.0f}, // Initializing to zero
//                .out = 0.0f,
//                .order = 6,
//            },
//            .previous_filtered_value = 0.0f,
//        };





    if(++zc.decimation_index >= zc.decimation)
    {
        zc.filtered_value = fir_filter(&zc.filter, value);
        ret = (zc.filtered_value * zc.previous_filtered_value < 0.0f) & (zc.previous_filtered_value  < 0.0f);
        zc.previous_filtered_value  = zc.filtered_value ;
        zc.decimation_index = 0;
    }


    return (ret);
}
