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



#pragma DATA_SECTION(sm_cpu2, "app_data_cpu2");
app_sm_t sm_cpu2;
#pragma DATA_SECTION(app, "app_data_cpu1");
application_t app;


#ifdef CPU1

bool flag_new_range = false;
int8_t new_range = 0;

bool flag_new_calibration = false;
int8_t new_cal_parameter = 0;
int8_t new_cal_phase = 0;
int8_t new_cal_index = 0;

bool flag_start_test = false;
bool flag_reset_test = false;
bool flag_reset_metrics = false;

static void ipc_sync_cpu1(void)
{

    IpcRegs.IPCCLR.all = 0xFFFFFFFFUL;
    IpcRegs.IPCSET.bit.IPC0 = 1;
    while (IpcRegs.IPCSTS.bit.IPC0 == 0) {}
    IpcRegs.IPCCLR.bit.IPC0 = 1;
}


static inline void init_abi(application_t *a) {
    a->abi.magic   = ABI_MAGIC;
    a->abi.version = ABI_VERSION;
    a->abi.size    = (uint32_t)sizeof(*a);
    // barrier
    __asm(" RPT #3 || NOP");

}


#elif defined(CPU2)

#ifndef APP_ADDR_EXPECT
#define APP_ADDR_EXPECT ((uintptr_t)0x000C000u)  /* from linker file .cmd */
#endif

static void ipc_sync_cpu2(void)
{
    IpcRegs.IPCCLR.all = 0xFFFFFFFFUL;
    while (IpcRegs.IPCSTS.bit.IPC0 == 0) {}
    IpcRegs.IPCSET.bit.IPC0 = 1;


}

static inline bool abi_ready(const application_t *a) {

    // barrier
    __asm(" RPT #3 || NOP");

    return (a->abi.magic   == ABI_MAGIC)   &&
            (a->abi.version == ABI_VERSION) &&
            (a->abi.size    == (uint32_t)sizeof(application_t)) &&
            ((uintptr_t)&app == APP_ADDR_EXPECT);
}

static inline void check_abi(application_t *a) {
    const uint32_t MAX_SPINS = 500000u;
    uint32_t spins = 0u;
    while (!abi_ready(a) && spins++ < MAX_SPINS) {
        __asm(" NOP");
    }
    if (!abi_ready(a)) {

        ESTOP0;
    }
}

#endif



generic_status_t app_init(application_t * this_app)
{
    generic_status_t ret = STATUS_DONE;

#ifdef CPU1
    {

        init_abi(this_app);

        this_app->sm_cpu2 = &sm_cpu2;
        /*ret = */app_init_cpu1(this_app);

        // Synchronize both the cores
        ipc_sync_cpu1();


    }
#elif defined(CPU2)
    {


        // Enable Global Interrupt (INTM) and real time interrupt (DBGM)
        EINT;
        ERTM;

        ipc_sync_cpu2();

        check_abi(this_app);

        /*ret = */app_init_cpu2(this_app);
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














