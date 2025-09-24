#include "application.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <file.h>
#include "F28x_Project.h"



#ifdef CPU1
#include "app_cpu1.h"
#elif defined(CPU2)
#include "app_cpu2.h"
#endif








#pragma DATA_SECTION(fw_info,"fw_info");
const fw_data_t fw_info =
{
 .fw_crc = 0x12345678,
 .version = {"00.01dev"},
 .compilation_date = __DATE__,
 .compilation_time = __TIME__,
#ifdef CPU1
 .cpu = '1',
#elif defined(CPU2)
 .cpu = '2',
#else
 .cpu = '-',
#endif
};




#pragma DATA_SECTION(analog_inputs, "app_data_cpu2");
analog_input_t analog_inputs[8];

#pragma DATA_SECTION(cal_a_rx, "app_data_cpu2");
calibration_t cal_a_rx;
#pragma DATA_SECTION(cal_b_rx, "app_data_cpu2");
calibration_t cal_b_rx;
#pragma DATA_SECTION(cal_c_rx, "app_data_cpu2");
calibration_t cal_c_rx;

#pragma DATA_SECTION(cal_a_tx, "app_data_cpu1");
calibration_t cal_a_tx;
#pragma DATA_SECTION(cal_b_tx, "app_data_cpu1");
calibration_t cal_b_tx;
#pragma DATA_SECTION(cal_c_tx, "app_data_cpu1");
calibration_t cal_c_tx;


#pragma DATA_SECTION(app, "app_data_cpu1");
application_t app;

bool flag_new_range = false;
int8_t new_range = 0;

bool flag_new_calibration = false;
int8_t new_cal_parameter = 0;
int8_t new_cal_phase = 0;
int8_t new_cal_index = 0;





#ifdef CPU1

static inline void app_init_links(void) {
    app.meter.analog_input = analog_inputs;
    app.meter.phase[1].cal_rx = &cal_a_rx;
    app.meter.phase[2].cal_rx = &cal_b_rx;
    app.meter.phase[3].cal_rx = &cal_c_rx;
    app.meter.phase[1].cal_tx = &cal_a_tx;
    app.meter.phase[2].cal_tx = &cal_b_tx;
    app.meter.phase[3].cal_tx = &cal_c_tx;

    app.abi_magic   = 0x41505041U;    // 'APPA'
    app.abi_version = 0x00010000U;    // v1.0
}

#elif defined(CPU2)


void abi_check(void) {
    int error;
    // endereços esperados – devem bater com o .cmd
    const uintptr_t APP_ADDR_EXPECT = 0x0C000;
    const uintptr_t AI_ADDR_EXPECT  = 0x0C400;

    if ((uintptr_t)&app != APP_ADDR_EXPECT)   error=1;//fault();  // linker divergente
    if ((uintptr_t)&analog_inputs != AI_ADDR_EXPECT) error =2;//fault();

    if (app.abi_magic != 0x41505041U) error=4;//fault();
    if (app.abi_version != 0x00010000U) error=5;//fault();
}


#endif






generic_status_t app_init(application_t * this_app)
{
    generic_status_t ret = STATUS_DONE;

#ifdef CPU1
    {
        app_init_links();
        app_init_cpu1(this_app);
    }
#elif defined(CPU2)
    {
        abi_check() ;
        app_init_cpu2(this_app);
    }
#endif

    return ret;
}




generic_status_t app_run(application_t * this_app)
{
    generic_status_t ret = STATUS_DONE;

#ifdef CPU1
    {
        app_run_cpu1(this_app);
    }
#elif defined(CPU2)
    {
        app_run_cpu2(this_app);
    }
#endif

    return ret;
}














