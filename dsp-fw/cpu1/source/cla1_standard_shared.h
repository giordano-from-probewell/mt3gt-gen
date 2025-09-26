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


extern float32_t one_bit_resolution;





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
