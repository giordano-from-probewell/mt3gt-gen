#ifndef __AMPLIFIER_WIRING_H_
#define __AMPLIFIER_WIRING_H_

#include "application.h"


typedef enum {
    AFE_IN = 0,
    AFE_VN = 1,
    AFE_IC = 2,
    AFE_VC = 3,
    AFE_IB = 4,
    AFE_VB = 5,
    AFE_IA = 6,
    AFE_VA = 7,
} amp_channel_t;

typedef struct {
    uint8_t   ai_idx;        // índice em app->meter.analog_input[]
    uint16_t  ads_ch;        // ADS178_CHx
    uint16_t  ada_id;        // ADA4254_Ax
    uint16_t  spi_base;      // SPIA_BASE / SPIB_BASE ...
    uint16_t  cs_pin;        // GPIO_PIN_CS_Ax
    uint16_t  in_gain;       // IN_GAIN_?
    uint16_t  out_gain;      // OUT_GAIN_?
    const char *label;       // para logs
} amp_wiring_t;

static const amp_wiring_t AMP_WIRING[] = {
    {AFE_IN, ADS178_CH1, ADA4254_A8, SPIA_BASE, GPIO_PIN_CS_A8, IN_GAIN_1, OUT_GAIN_1, "AFE IN"},
    {AFE_VN, ADS178_CH2, ADA4254_A4, SPIA_BASE, GPIO_PIN_CS_A4, IN_GAIN_1, OUT_GAIN_1, "AFE VN"},
    {AFE_IC, ADS178_CH3, ADA4254_A7, SPIA_BASE, GPIO_PIN_CS_A7, IN_GAIN_1, OUT_GAIN_1, "AFE IC"},
    {AFE_VC, ADS178_CH4, ADA4254_A3, SPIA_BASE, GPIO_PIN_CS_A3, IN_GAIN_1, OUT_GAIN_1, "AFE VC"},
    {AFE_IB, ADS178_CH5, ADA4254_A6, SPIA_BASE, GPIO_PIN_CS_A6, IN_GAIN_1, OUT_GAIN_1, "AFE IB"},
    {AFE_VB, ADS178_CH6, ADA4254_A2, SPIA_BASE, GPIO_PIN_CS_A2, IN_GAIN_1, OUT_GAIN_1, "AFE VB"},
    {AFE_IA, ADS178_CH7, ADA4254_A5, SPIA_BASE, GPIO_PIN_CS_A5, IN_GAIN_1, OUT_GAIN_1, "AFE IA"},
    {AFE_VA, ADS178_CH8, ADA4254_A1, SPIA_BASE, GPIO_PIN_CS_A1, IN_GAIN_1, OUT_GAIN_1, "AFE VA"},
};
static const uint8_t AMP_WIRING_N = sizeof(AMP_WIRING)/sizeof(AMP_WIRING[0]);




#endif
