#include "F28x_Project.h"
#include "F2837xD_sdfm_drivers.h"
#include "F2837xD_struct.h"
#include "F2837xD_device.h"
#include "F2837xD_cputimervars.h"

#include "driverlib.h"
#include "device.h"

#include "generic_definitions.h"
#include "application.h"
#include "my_time.h"
#include "buzzer.h"
#include "cli.h"

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

extern buzzer_state_t buzzer;

// Function Prototypes
void syncCpu2CPU1(void);


__interrupt void cpu_timer0_isr_main(void);
__interrupt void cpu_timer1_isr_main(void);
__interrupt void cpu_timer2_isr_main(void);



void main(void)
{
    // Initialize device clock and peripherals
    Device_init();


#ifdef _FLASH
    //
    // Copy over code and tables from FLASH to RAM
    //
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (uint32_t)&RamfuncsLoadSize);
    //memcpy((uint32_t *)&Cla1ProgRunStart, (uint32_t *)&Cla1ProgLoadStart,
    //       (uint32_t)&Cla1ProgLoadSize);

    //#if !(CLA_MATH_TABLES_IN_ROM)
    //    //
    //    // Copy over CLA Math tables from FLASH to RAM
    //    //
    //    memcpy((uint32_t *)&CLA1mathTablesRunStart, (uint32_t *)&CLA1mathTablesLoadStart,
    //           (uint32_t)&CLA1mathTablesLoadSize);
    //#endif
#endif


    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    Interrupt_initVectorTable();


    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    Interrupt_register(INT_TIMER0, &cpu_timer0_isr_main);
    Interrupt_register(INT_TIMER1, &cpu_timer1_isr_main);
    Interrupt_register(INT_TIMER2, &cpu_timer2_isr_main);


    // Initialize the Device Peripheral. This function can be
    //         found in F2837xD_CpuTimers.c
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

    // Configure CPU-Timer 0, 1, and 2:
    // 200MHz CPU Freq, Period (in uSeconds)
    // Timer 2 is specific for timing processing tasks
    //ConfigCpuTimer(&CpuTimer0, 200, 1000000);//PLL 200MHz
    ConfigCpuTimer(&CpuTimer1, 200, 1000000);//PLL 200MHz
    ConfigCpuTimer(&CpuTimer2, 10, 1000);//XTALL 10Mhz/ 10000 = 1ms


    // To ensure precise timing, use write-only instructions to write to the
    // entire register. Therefore, if any of the configuration bits are changed in
    // ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the below
    // settings must also be updated.
    CpuTimer0Regs.TCR.all = 0x4000;
    CpuTimer1Regs.TCR.all = 0x4000;
    CpuTimer2Regs.TCR.all = 0x4000;

    // Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
    // which is connected to CPU-Timer 1, and CPU int 14, which is connected
    // to CPU-Timer 2:
    //        Interrupt_enable(INT_TIMER0);
    //        Interrupt_enable(INT_TIMER1);
    //IER |= M_INT14;
    //        // Enable TINT0 in the PIE: Group 1 __interrupt 7
    //        //
    //        PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    app_init(&app);

    while(1)
    {
        app_run(&app);
    }
}


//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr_main(void)
{
    // GPIO_WritePin(GPIO_PROFILING0, 1);
    CpuTimer0.InterruptCount++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
    // GPIO_WritePin(GPIO_PROFILING0, 0);
}

//
// cpu_timer1_isr - CPU Timer1 ISR
//
__interrupt void cpu_timer1_isr_main(void)
{
    // GPIO_WritePin(GPIO_PROFILING0, 1);
    CpuTimer1.InterruptCount++;
    // GPIO_WritePin(GPIO_PROFILING0, 0);
}

//
// cpu_timer2_isr CPU Timer2 ISR
//
__interrupt void cpu_timer2_isr_main(void)
{
    // GPIO_WritePin(GPIO_PROFILING0, 1);
    CpuTimer2.InterruptCount++;
    // GPIO_WritePin(GPIO_PROFILING0, 0);
}





