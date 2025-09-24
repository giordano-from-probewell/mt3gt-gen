 #include <math.h>
#include <string.h>
#include <stdio.h>
#include "errors.h"
#include "generic_definitions.h"

#include "commands.h"

#include "application.h"
//#include "cla1_standard_shared.h"


#define _CMD_ANSWER         0x5200

int16_t cmd_get_measures(uint8_t *error_category, uint8_t *error_code,uint16_t *command, uint8_t *data, uint16_t *size)
{

    myfloat_t tmp;
    uint16_t idx=0;


    if (*size != 0 )
    {
        *error_category = ERROR(ERROR_CMD_PARAMS);
        *error_code = ERROR(ERROR_CMD_PARAMS__CMD_SIZE_EXCEED_MAX);
        return (COMMAND_RESULT__ERROR);
    }

//    //INSTANT METRICS
//    //VA
//    tmp.value = app.meter.phase[1].voltage.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //VB
//    tmp.value = app.meter.phase[2].voltage.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //VC
//    tmp.value = app.meter.phase[3].voltage.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //IA
//    tmp.value = app.meter.phase[1].current.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //IB
//    tmp.value = app.meter.phase[2].current.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //IC
//    tmp.value = app.meter.phase[3].current.measures.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //W A
//    tmp.value = app.meter.phase[1].measures.w;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //W B
//    tmp.value = app.meter.phase[2].measures.w;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //W C
//    tmp.value = app.meter.phase[3].measures.w;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;

    //VAr A
    tmp.value = app.meter.phase[1].measures.var;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VAr B
    tmp.value = app.meter.phase[2].measures.var;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VAr C
    tmp.value = app.meter.phase[3].measures.var;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VA A
    tmp.value = app.meter.phase[1].measures.va;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VA B
    tmp.value = app.meter.phase[2].measures.va;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VA C
    tmp.value = app.meter.phase[3].measures.va;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //I PHI A
    tmp.value = app.meter.phase[1].measures.phi;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //I PHI B
    tmp.value = app.meter.phase[2].measures.phi;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //I PHI C
    tmp.value = app.meter.phase[3].measures.phi;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

//    //U PHI A
//    tmp.value = app.meter.phase[1].measures.uPhi;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //U PHI B
//    tmp.value = app.meter.phase[2].measures.uPhi;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //U PHI C
//    tmp.value = app.meter.phase[3].measures.uPhi;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //PF A
//    tmp.value = app.meter.phase[1].measures.power_factor;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //PF B
//    tmp.value = app.meter.phase[2].measures.power_factor;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //PF C
//    tmp.value = app.meter.phase[3].measures.power_factor;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //ACCUMULATED METRICS
//
//    //V A AVG
//    tmp.value = app.meter.phase[1].voltage.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V A MAX
//    tmp.value = app.meter.phase[1].voltage.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V A MIN
//    tmp.value = app.meter.phase[1].voltage.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V B AVG
//    tmp.value = app.meter.phase[2].voltage.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V B MAX
//    tmp.value = app.meter.phase[2].voltage.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V B MIN
//    tmp.value = app.meter.phase[2].voltage.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//
//    //V C AVG
//    tmp.value = app.meter.phase[3].voltage.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V C MAX
//    tmp.value = app.meter.phase[3].voltage.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //V C MIN
//    tmp.value = app.meter.phase[3].voltage.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I A AVG
//    tmp.value = app.meter.phase[1].current.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I A MAX
//    tmp.value = app.meter.phase[1].current.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I A MIN
//    tmp.value = app.meter.phase[1].current.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I B AVG
//    tmp.value = app.meter.phase[2].current.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I B MAX
//    tmp.value = app.meter.phase[2].current.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I B MIN
//    tmp.value = app.meter.phase[2].current.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I C AVG
//    tmp.value = app.meter.phase[3].current.metrics.rms_avg;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I C MAX
//    tmp.value = app.meter.phase[3].current.metrics.rms_max;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;
//
//    //I C MIN
//    tmp.value = app.meter.phase[3].current.metrics.rms_min;
//    data[idx++] = tmp.byte.b3;
//    data[idx++] = tmp.byte.b2;
//    data[idx++] = tmp.byte.b1;
//    data[idx++] = tmp.byte.b0;

    //Wh A
    tmp.value = app.meter.phase[1].measures.wh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //Wh B
    tmp.value = app.meter.phase[2].measures.wh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //Wh C
    tmp.value = app.meter.phase[3].measures.wh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VArh A
    tmp.value = app.meter.phase[1].measures.varh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VArh B
    tmp.value = app.meter.phase[2].measures.varh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VArh C
    tmp.value = app.meter.phase[3].measures.varh;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VAh A
    tmp.value = app.meter.phase[1].measures.vah;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VAh B
    tmp.value = app.meter.phase[2].measures.vah;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //VAh C
    tmp.value = app.meter.phase[3].measures.vah;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;


    //TEST METRICS
    //tmp.value = cla_test.energy_instant;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //tmp.value = cla_test.energy_measured;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //tmp.value = cla_test.energy_target;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

    //tmp.value = cla_test.error;
    data[idx++] = tmp.byte.b3;
    data[idx++] = tmp.byte.b2;
    data[idx++] = tmp.byte.b1;
    data[idx++] = tmp.byte.b0;

//    data[idx++] = cla_test.flag_test_done;
//    data[idx++] = cla_test.pulses_expected;
//    data[idx++] = cla_test.pulses_generated;
//    data[idx++] = cla_test.flag_start_test;
//    data[idx++] = cla_test.flag_testing;


    *error_category = ERROR(ERROR_CATEGORY_NONE);
    *error_code = ERROR(ERROR_CODE_NONE);
    *size = SIZE(idx);
    *command = _CMD_ANSWER;
    return (COMMAND_RESULT__SUCCESS);
}



