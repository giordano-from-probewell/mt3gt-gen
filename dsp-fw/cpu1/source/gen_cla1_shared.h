#ifndef _GEN_CLA1_SHARED_H_
#define _GEN_CLA1_SHARED_H_

#include <stdint.h>

#include "F2837xD_Cla_defines.h"
#include "F2837xD_Cla_typedefs.h"
#include "F2837xD_device.h"
#include "F2837xD_sdfm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union
{
    float *ptr; // Aligned to lower 16-bits
    Uint32 pad; // 32-bits
}CLA_FPTR;


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


typedef enum cla_state_en
{
    STATE_CLA_CONTROLLER__OFF       =      0u,
    STATE_CLA_CONTROLLER__ON        =      1u,
}cla_state_t;

typedef struct cla_repetitive_controller_st
{
    float v_erro[1000];     // error vector
    float v_out[1000];      // output vector
    float kre;              // k error
    float krz;              // k sample
    float krl;              // k neighbors
    float z_erro;           // error with delay
    float out_m1;           // last out value
    float out_0;            // out
    float out_p1;           // future out value
    float acc_out_dc;
    float out;              // out
    float out_dc;           // used to remove dc from out transformers
    float acc_in_dc;
    float in_dc;            // used to remove dc error from input sensors
    int16   counter;        // samples for first cycle
    int16   index;          // sample index
    int16   delta;          // sensor delay
    bool  init_flag;        // first cycle flag
    cla_state_t state;      // controller state
}cla_repetitive_controller_t;


extern cla_repetitive_controller_t cla_voltage_controller;
extern cla_repetitive_controller_t cla_current_controller;

typedef struct cla_measure_st
{
    int32 raw;
    float32 data;
    float32 gain;
    float32 offset;
    float32 offset_acc;
}cla_measure_t;

extern cla_measure_t cla_vv1;
extern cla_measure_t cla_vi1;
extern cla_measure_t cla_iv1;
extern cla_measure_t cla_ii1;
extern cla_measure_t cla_vv2;
extern cla_measure_t cla_vi2;
extern cla_measure_t cla_iv2;
extern cla_measure_t cla_ii2;
extern float cla_voltage_setpoint;
extern float cla_current_setpoint;


extern int flag_start;



#ifdef __cplusplus
}
#endif // extern "C"

#endif //end of _CLA_SDFM_FILTER_SYNCH_SHARED_H_ definition

