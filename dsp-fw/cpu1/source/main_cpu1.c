#include "F28x_Project.h"
#include "F2837xD_Ipc_drivers.h"
#include "F2837xD_sdfm_drivers.h"
#include "F2837xD_struct.h"
#include "F2837xD_device.h"
#include "F2837xD_cputimervars.h"

#include "CLAmath.h"

#include "driverlib.h"
#include "device.h"

#include "generic_definitions.h"
#include "application.h"

#include "cla1_standard_shared.h"

#include "external_sdram.h"

// External symbols created by the linker cmd file DSP28
// examples will use these to relocate code from one LOAD
// location in Flash to a different RUN location in internal RAM
extern uint16_t Cla1ProgRunStart;
extern uint16_t Cla1ProgLoadStart;
extern uint16_t Cla1ProgLoadSize;
extern uint16_t Cla1ConstRunStart;
extern uint16_t Cla1ConstLoadStart;
extern uint16_t Cla1ConstLoadSize;
extern uint16_t CLA1mathTablesRunStart;
extern uint16_t CLA1mathTablesLoadStart;
extern uint16_t CLA1mathTablesLoadSize;
extern uint16_t RamfuncsLoadStart, RamfuncsLoadSize, RamfuncsRunStart;

// Function Prototypes

//// Far memory test variables
//#define ARRAY_SIZE ((uint32_t)(50000)) // 64 KB
//__attribute__((far)) volatile uint32_t farArray[ARRAY_SIZE] __attribute__((section(".farbss")));

void _distribute_memories(void);
void _distribute_peripherals(void);


extern __interrupt void cpu_timer0_isr_main(void);
extern __interrupt void cpu_timer1_isr_main(void);
extern __interrupt void cpu_timer2_isr_main(void);
__interrupt void wakeup_isr(void);



void main(void)
{

    //Initialize device clock and peripherals
    Device_init();

    _distribute_memories();
    _distribute_peripherals();

#ifdef _STANDALONE
#ifdef _FLASH
    // Send boot command to allow the CPU2 application to begin execution
    Device_bootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#else
    // Send boot command to allow the CPU2 application to begin execution
    Device_bootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);

#endif // _FLASH
#endif // _STANDALONE

#ifdef _FLASH
    // Copy over code and tables from FLASH to RAM
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (uint32_t)&RamfuncsLoadSize);
    memcpy((uint32_t *)&Cla1ProgRunStart, (uint32_t *)&Cla1ProgLoadStart,
           (uint32_t)&Cla1ProgLoadSize);

    //#if !(CLA_MATH_TABLES_IN_ROM)
    //    // Copy over CLA Math tables from FLASH to RAM
    //    memcpy((uint32_t *)&CLA1mathTablesRunStart, (uint32_t *)&CLA1mathTablesLoadStart,
    //           (uint32_t)&CLA1mathTablesLoadSize);
    //#endif
#endif


    // Initialize GPIO and EMIF1 for SDRAM
    initEMIF1();
    run_sdram_tests(); //todo: this is used to test mode, delete or change for production mode

    // Initialize GPIO and configure the GPIO pin as a push-pull output
    Device_initGPIO();

    // Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize PIE and clear PIE registers. Disables CPU interrupts
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    Interrupt_initVectorTable();

    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    Interrupt_register(INT_TIMER0, &cpu_timer0_isr_main);
    Interrupt_register(INT_TIMER1, &cpu_timer1_isr_main);
    Interrupt_register(INT_TIMER2, &cpu_timer2_isr_main);
    // Re-map watchdog wake interrupt signal to call the ISR function
    Interrupt_register(INT_WAKE, &wakeup_isr);

    // Initialize the Device Peripheral. This function can be
    //         found in F2837xD_CpuTimers.c
    InitCpuTimers(); // For this example, only initialize the Cpu Timers

    // Configure CPU-Timer 0, 1, and 2:
    // 200MHz CPU Freq, Period (in uSeconds)
    // Timer 2 is specific for timing processing tasks
    // ConfigCpuTimer(&CpuTimer0, 200, 1000000);//PLL 200MHz
    //ConfigCpuTimer(&CpuTimer1, 200, 1000000); // PLL 200MHz
    ConfigCpuTimer(&CpuTimer2, 10, 1000);     // XTALL 10Mhz/ 10000 = 1ms

    // To ensure precise timing, use write-only instructions to write to the
    // entire register. Therefore, if any of the configuration bits are changed in
    // ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the below
    // settings must also be updated.
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer1Regs.TCR.all = 0x4000;
    CpuTimer2Regs.TCR.all = 0x4000;

    //    // Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
    //    // which is connected to CPU-Timer 1, and CPU int 14, which is connected
    //    // to CPU-Timer 2:
    //    Interrupt_enable(INT_TIMER0);
    //    Interrupt_enable(INT_TIMER1);
    //    IER |= M_INT14;
    //    // Enable TINT0 in the PIE: Group 1 __interrupt 7
    //    //
    //    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;



    app_init(&app);




    while(1)
    {

        app_run(&app);

    }
}

void _distribute_memories(void)
{
    //RAM GS5 -> CPU2
    MemCfg_setGSRAMControllerSel(MEMCFG_SECT_GS5,MEMCFG_GSRAMCONTROLLER_CPU2);
    //RAM GS12,13 -> CPU2
    MemCfg_setGSRAMControllerSel(MEMCFG_SECT_GS12,MEMCFG_GSRAMCONTROLLER_CPU2);
    MemCfg_setGSRAMControllerSel(MEMCFG_SECT_GS13,MEMCFG_GSRAMCONTROLLER_CPU2);
}



void _distribute_peripherals(void)//HW -> CPU2
{
    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_SCIC,   SYSCTL_CPUSEL_CPU2); //COMM         - SCIA
    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_SCID,   SYSCTL_CPUSEL_CPU2); //CLI          - SCID
    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_EPWM12, SYSCTL_CPUSEL_CPU2); //BUZZER       - PWM12
    SysCtl_selectCPUForPeripheralInstance(SYSCTL_CPUSEL_SPIA,   SYSCTL_CPUSEL_CPU2); //APLIFIERS    - SPIA
}


//
// Wakeup ISR - The interrupt service routine called when the watchdog
//              triggers the wake interrupt signal
//
__interrupt void wakeup_isr(void)
{

    ESTOP0;

    //boot_error = true;
    //
    // Acknowledge this interrupt located in group 1
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}
