#ifndef __FEEDBACK_CALIBRATION_H_
#define __FEEDBACK_CALIBRATION_H_

typedef struct calibration_feedback_data_st
{
    char date[8];
    char last_verification_date[8];
    float voltage_gain;
    float voltage_offset;
    uint16_t voltage_delta;
    float current_gain;
    float current_offset;
    uint16_t current_delta;
} calibration_feedback_data_t;

typedef struct calibration_feedback_st
{
    union calibration_feedback_un
    {
        calibration_feedback_data_t full;
        uint16_t raw[ VGEN_CONFIG__SIZE];
    } data;
} calibration_t;

#endif


