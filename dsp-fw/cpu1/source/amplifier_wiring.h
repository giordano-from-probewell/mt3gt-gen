#ifndef __AMPLIFIER_WIRING_H_
#define __AMPLIFIER_WIRING_H_

#include "application.h"

#define ADA4254_A1  1
#define ADA4254_A2  2
#define ADA4254_A3  3
#define ADA4254_A4  4
#define ADA4254_A5  5
#define ADA4254_A6  6
#define ADA4254_A7  7
#define ADA4254_A8  8

typedef enum {
    AFE_VV1 = 0,
    AFE_VI1 = 1,
    AFE_VV2 = 2,
    AFE_VI2 = 3,
    AFE_II1 = 4,
    AFE_IV1 = 5,
    AFE_II2 = 6,
    AFE_IV2 = 7,
} amp_channel_t;

typedef struct {
    uint8_t   ai_idx;        // index app->meter.analog_input[]
    uint16_t  ada_id;        // ADA4254_Ax
    uint16_t  spi_base;      // SPIA_BASE / SPIB_BASE ...
    uint16_t  cs_pin;        // GPIO_PIN_CS_Ax
    uint16_t  in_gain;       // IN_GAIN_?
    uint16_t  out_gain;      // OUT_GAIN_?
    ada4254_t *link;
    const char *label;       // for logs
} amp_wiring_t;

static amp_wiring_t AMP_WIRING[] = {
    {AFE_VV1, ADA4254_A8, SPIC_BASE, CS_VV1_PIN, IN_GAIN_1, OUT_GAIN_1, NULL, "AFE_VV1"},
    {AFE_VI1, ADA4254_A4, SPIC_BASE, CS_VI1_PIN, IN_GAIN_1, OUT_GAIN_1, NULL, "AFE_VI1"},
    {AFE_VV2, ADA4254_A7, SPIC_BASE, CS_VV2_PIN, IN_GAIN_2, OUT_GAIN_1, NULL, "AFE_VV2"},
    {AFE_VI2, ADA4254_A3, SPIC_BASE, CS_VI2_PIN, IN_GAIN_8, OUT_GAIN_1, NULL, "AFE_VI2"},
    {AFE_II1, ADA4254_A6, SPIC_BASE, CS_II1_PIN, IN_GAIN_1, OUT_GAIN_1, NULL, "AFE_II1"},
    {AFE_IV1, ADA4254_A2, SPIC_BASE, CS_IV1_PIN, IN_GAIN_1, OUT_GAIN_1, NULL, "AFE_IV1"},
    {AFE_II2, ADA4254_A5, SPIC_BASE, CS_II2_PIN, IN_GAIN_2, OUT_GAIN_1, NULL, "AFE_II2"},
    {AFE_IV2, ADA4254_A1, SPIC_BASE, CS_IV2_PIN, IN_GAIN_1, OUT_GAIN_1, NULL, "AFE_IV2"},
};
static uint8_t AMP_WIRING_N = sizeof(AMP_WIRING)/sizeof(AMP_WIRING[0]);



//    if(ada4254_init(&app.measures.secondary.voltage.voltage_gain, ADA4254_VV2, SPIC_BASE, CS_VV2_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.secondary.voltage.voltage_gain, IN_GAIN_2, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_init(&app.measures.secondary.voltage.current_gain, ADA4254_VI2, SPIC_BASE, CS_VI2_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.secondary.voltage.current_gain, IN_GAIN_8, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_init(&app.measures.secondary.current.voltage_gain, ADA4254_IV2, SPIC_BASE, CS_IV2_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.secondary.current.voltage_gain, IN_GAIN_1, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_init(&app.measures.secondary.current.current_gain, ADA4254_II2, SPIC_BASE, CS_II2_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.secondary.current.current_gain, IN_GAIN_2, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//
//
//    if(ada4254_init(&app.measures.primary.voltage.voltage_gain, ADA4254_VV1, SPIC_BASE, CS_VV1_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.primary.voltage.voltage_gain, IN_GAIN_1, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//
//    if(ada4254_init(&app.measures.primary.voltage.current_gain, ADA4254_VI1, SPIC_BASE, CS_VI1_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.primary.voltage.current_gain, IN_GAIN_1, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//
//    if(ada4254_init(&app.measures.primary.current.voltage_gain, ADA4254_IV1, SPIC_BASE, CS_IV1_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.primary.current.voltage_gain, IN_GAIN_1, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//
//    if(ada4254_init(&app.measures.primary.current.current_gain, ADA4254_II1, SPIC_BASE, CS_II1_PIN) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;
//    if(ada4254_set_gain(&app.measures.primary.current.current_gain, IN_GAIN_1, OUT_GAIN_1) != ADA4254_STATUS_OK)
//        status = STATUS_ERROR;





#endif
