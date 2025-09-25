#ifndef _CLA1_STANDARD_SHARED_H_
#define _CLA1_STANDARD_SHARED_H_

//
// Included Files
//
#include <stdint.h>
#include "F2837xD_Cla_defines.h"
#include "F2837xD_Cla_typedefs.h"
#include "F2837xD_device.h"
#include "power_meas_sine_analyzer.h"
#include <stdint.h>

//#ifndef __TMS320C28XX_CLA__
//#include <math.h>
//#else
#include <CLAmath.h>
//#endif

#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PI 3.14159265358979323846

#define UWH_PER_WH   (1000000)
#define NANOWH_PER_WH   (1000000000)
#define SEC_PER_HOUR_F      (3600.0f)
#define CYCLE_SAMPLES_F     (512.0f)
//depends on HW configurarion (shut -> 50Ohms) (Div Res = ---1.995M---5k---- -> 1/400)
#define VOLTAGE_RELATION ((((1995000.0+5000.0)/5000.0)*2.5) / 16777216.0)
#define CURRENT_RELATION -((2000.0/50.0)*2.5 / 16777216.0)


typedef enum {
    LEDTEST_IDLE = 0,
    LEDTEST_ARMED,
    LEDTEST_WAIT_FIRST_ZC,
    LEDTEST_RUNNING,
    LEDTEST_WAIT_LAST_ZC,
    LEDTEST_DONE,
    LEDTEST_ABORTED
} ledtest_state_t;

typedef struct cla_analog_channel_st
{
    float32_t fvalue;
    int32_t value;
    int32_t dc_acc;
    int32_t dc;
}cla_analog_channel_t;


typedef struct {
    uint32_t kh_imp_per_kwh;   //  meter kH(imp/kWh)
    uint32_t target_pulses;    // num o pulses to finish the test
    uint32_t debounce_us;      // sanity time
    uint32_t max_test_ms;      // timeout
    uint16_t  start_on_first_pulse; // 0: start test; 1: test count
} ledtest_cfg_t;

typedef struct {
    ledtest_state_t state;

    // External meter
    uint32_t pulse_count;

//    uint64_t last_pulse_ticks;
//    uint64_t first_pulse_ticks;
//    uint64_t last_event_ticks;
    uint64_t ref_energy_uWh;   // pulses * led_energy_comp_uWh (integer)
    uint32_t led_energy_comp_uWh;

    bool flag_first_pulse;
    bool flag_last_pulse;
    bool flag_testing;
    bool flag_first_zc;
    bool flag_last_zc;
    // Energy accumulated in the std meter
    // RUNNING state

    int64_t energy_test_init_uwh;
    int64_t energy_test_end_uwh;

    int64_t frac_first_uWh;                 // fração em uWh no instante do 1º pulso
    int64_t frac_last_uWh;                  // fração em uWh no instante do último pulso

    // Terget
    uint32_t target_pulses;
    uint32_t max_test_ms;

    // Results
    float32_t error_pct;     // (measured - ref)/ref * 100
    uint32_t duration_ms;

    // Bookkeeping
    //uint64_t start_ts_us;
    uint16_t  armed_after_first_pulse;


    uint32_t cycle_ticks;
    uint32_t test_ticks;
    int16_t sample_index_old;

    uint32_t cycle_samples;
    uint32_t cycle_pulse_index;
    int16_t sample_index_new;

    float32_t period;
    float32_t frequency;
} ledtest_rt_t;

typedef struct cla_meter_test_st {
    float32_t energy_dnwh_a[512];
    int32_t energy_cycle_uwh_a;    // accumulated active energy for the LED, in uWh (integer)
    int32_t energy_acc_uwh_a;
    int32_t energy_acc_wh_a;

    float32_t energy_dnwh_b[512];
    int32_t energy_cycle_uwh_b;    // accumulated active energy for the LED, in uWh (integer)
    int32_t energy_acc_uwh_b;
    int32_t energy_acc_wh_b;

    float32_t energy_dnwh_c[512];
    int32_t energy_cycle_uwh_c;    // accumulated active energy for the LED, in uWh (integer)
    int32_t energy_acc_uwh_c;
    int32_t energy_acc_wh_c;

} cla_meter_test_t;





typedef struct {
    int32_t cycle_uwh;
    int32_t cycle_uvarh;
    int32_t cycle_uvah;

    int32_t uwh_acc;
    int32_t wh;

    int32_t uvarh_acc;
    int32_t varh;

    uint32_t uvah_acc;
    uint32_t vah;
} cla_energy_acc_t;

typedef struct cla_calibrations_st
{
    float32_t voltage_gain[3];
    float32_t current_gain[3];
    float32_t voltage_cal[3];
    float32_t current_cal[3];
    float32_t alpha[3];         // angle correction

    int32_t voltage_cal_index;
    int32_t current_cal_index;

}cla_calibrations_t;

typedef struct cla_phase_st
{
    float32_t v;
    float32_t i;
    float32_t v2;
    float32_t i2;
    float32_t i2_prev;
    float32_t i3;
    float32_t i_dc;
    float32_t i_dc_acc;
    float32_t v_dc;
    float32_t v_dc_acc;

    float32_t v_rms;
    float32_t v_rms_acc;
    float32_t v_norm;
    float32_t i_rms;
    float32_t i_rms_acc;
    float32_t i_norm;

    float32_t dv_i_sum;     // sum (dv)*i  -> signal(Q)
    float32_t v2_prev;      // v2[k-1]
    float32_t w_inst;
    float32_t w_cycle;
    float32_t w_cycle_acc;
    float32_t var_cycle;
    float32_t va_cycle;

    float32_t period;
    float32_t frequency;
    float32_t period1;

    float32_t phi;
    float32_t power_factor;

    cla_calibrations_t cal;


    float32_t v_factor_num;
    float32_t v_factor_den;
    float32_t i_factor_num;
    float32_t i_factor_den;

    cla_energy_acc_t energy;

}cla_phase_t;



#define NUM_DATA             8
#define TO_CPU1              0
#define TO_CPU2              1
#define CONNECT_TO_CLA1      0
#define CONNECT_TO_DMA       1
#define CONNECT_TO_CLA2      2
#define ENABLE               1
#define DISABLE              0


#define CPU1_CLA1(x)      EALLOW; DevCfgRegs.DC1.bit.CPU1_CLA1 = x; EDIS
#define CPU2_CLA1(x)      EALLOW; DevCfgRegs.DC1.bit.CPU2_CLA1 = x; EDIS

#define CONNECT_SD1(x)    EALLOW; DevCfgRegs.CPUSEL4.bit.SD1 = x; EDIS
#define CONNECT_SD2(x)    EALLOW; DevCfgRegs.CPUSEL4.bit.SD2 = x; EDIS

#define VBUS32_1(x)       EALLOW; CpuSysRegs.SECMSEL.bit.PF1SEL = x; EDIS
#define VBUS32_2(x)       EALLOW; CpuSysRegs.SECMSEL.bit.PF2SEL = x; EDIS
#define VBUS32_3(x)       EALLOW; CpuSysRegs.SECMSEL.bit.VBUS32_3 = x; EDIS
#define VBUS32_4(x)       EALLOW; CpuSysRegs.SECMSEL.bit.VBUS32_4 = x; EDIS
#define VBUS32_5(x)       EALLOW; CpuSysRegs.SECMSEL.bit.VBUS32_5 = x; EDIS
#define VBUS32_6(x)       EALLOW; CpuSysRegs.SECMSEL.bit.VBUS32_6 = x; EDIS
#define VBUS32_7(x)       EALLOW; CpuSysRegs.SECMSEL.bit.VBUS32_7 = x; EDIS

extern bool flag_zc_detected;
extern float32_t one_bit_resolution;

extern cla_phase_t cla_phase_aux;
extern cla_phase_t cla_phase_a;
extern cla_phase_t cla_phase_b;
extern cla_phase_t cla_phase_c;

extern float32_t v_vector[];
extern float32_t i_vector[];

extern cla_meter_test_t cla_test_aux;
extern ledtest_rt_t meter_test;


// The following are symbols defined in the CLA assembly code
// Including them in the shared header file makes them
// .global and the main CPU can make use of them.

//
//CLA C Tasks
//
__interrupt void Cla1Task1();
__interrupt void Cla1Task2();
__interrupt void Cla1Task3();
__interrupt void Cla1Task4();
__interrupt void Cla1Task5();
__interrupt void Cla1Task6();
__interrupt void Cla1Task7();
__interrupt void Cla1Task8();

__interrupt void cla1Isr1();
__interrupt void cla1Isr2();
__interrupt void cla1Isr3();
__interrupt void cla1Isr4();
__interrupt void cla1Isr5();
__interrupt void cla1Isr6();
__interrupt void cla1Isr7();
__interrupt void cla1Isr8();

//
// Linker command variables
//
extern Uint32 Cla1funcsLoadStart;
extern Uint32 Cla1funcsLoadEnd;
extern Uint32 Cla1funcsRunStart;
extern Uint32 Cla1funcsLoadSize;

extern int32_t clatocpu1;
extern float32_t clatocpu1_f1;
extern float32_t clatocpu1_f2;
extern float32_t clatocpu1_f3;

extern int32_t cpu1tocla;



#ifdef __cplusplus
}
#endif // extern "C"

#endif //end of _CLA_SDFM_FILTER_SYNCH_SHARED_H_ definition

//
// End of file
//
