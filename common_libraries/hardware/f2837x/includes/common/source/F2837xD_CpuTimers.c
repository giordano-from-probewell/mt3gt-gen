//###########################################################################
//
// FILE:    F2837xD_CpuTimers.c
//
// TITLE:   CPU 32-bit Timers Initialization & Support Functions.
//
//###########################################################################
//
// $Release Date:  $
// $Copyright:
// Copyright (C) 2013-2023 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

//
// Included Files
//
#include "F2837xD_device.h"
#include "F2837xD_Examples.h"

//
// Globals
//
struct CPUTIMER_VARS CpuTimer0;
struct CPUTIMER_VARS CpuTimer1;
struct CPUTIMER_VARS CpuTimer2;

//
// InitCpuTimers - This function initializes all three CPU timers to a known
//                 state.
//
void InitCpuTimers(void)
{
    //
    // CPU Timer 0
    // Initialize address pointers to respective timer registers:
    //
    CpuTimer0.RegsAddr = &CpuTimer0Regs;

    //
    // Initialize timer period to maximum:
    //
    CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CpuTimer0Regs.TPR.all  = 0;
    CpuTimer0Regs.TPRH.all = 0;

    //
    // Make sure timer is stopped:
    //
    CpuTimer0Regs.TCR.bit.TSS = 1;

    //
    // Reload all counter register with period value:
    //
    CpuTimer0Regs.TCR.bit.TRB = 1;

    //
    // Reset interrupt counters:
    //
    CpuTimer0.InterruptCount = 0;

    //
    // Initialize address pointers to respective timer registers:
    //
    CpuTimer1.RegsAddr = &CpuTimer1Regs;
    CpuTimer2.RegsAddr = &CpuTimer2Regs;

    //
    // Initialize timer period to maximum:
    //
    CpuTimer1Regs.PRD.all  = 0xFFFFFFFF;
    CpuTimer2Regs.PRD.all  = 0xFFFFFFFF;

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CpuTimer1Regs.TPR.all  = 0;
    CpuTimer1Regs.TPRH.all = 0;
    CpuTimer2Regs.TPR.all  = 0;
    CpuTimer2Regs.TPRH.all = 0;

    //
    // Make sure timers are stopped:
    //
    CpuTimer1Regs.TCR.bit.TSS = 1;
    CpuTimer2Regs.TCR.bit.TSS = 1;

    //
    // Reload all counter register with period value:
    //
    CpuTimer1Regs.TCR.bit.TRB = 1;
    CpuTimer2Regs.TCR.bit.TRB = 1;

    //
    // Reset interrupt counters:
    //
    CpuTimer1.InterruptCount = 0;
    CpuTimer2.InterruptCount = 0;
}

//
// ConfigCpuTimer - This function initializes the selected timer to the period
//                  specified by the "Freq" and "Period" parameters. The "Freq"
//                  is entered as "MHz" and the period in "uSeconds". The timer
//                  is held in the stopped state after configuration.
//
void ConfigCpuTimer(struct CPUTIMER_VARS *Timer, float Freq, float Period)
{
    Uint32 temp;



    //modified Giordano 20230224
    if(Timer == &CpuTimer2)
    {

        EALLOW;
        CpuSysRegs.TMR2CLKCTL.bit.TMR2CLKSRCSEL = 3;
        CpuSysRegs.TMR2CLKCTL.bit.TMR2CLKPRESCALE = 0;
        EDIS;


        Timer->CPUFreqInMHz = Freq;
        Timer->PeriodInUSec = Period;

        CpuTimer2Regs.TCR.bit.TSS = 1;                  // Stop Timer

        CpuTimer2Regs.TIM.all = INT32_MAX;              //

        CpuTimer2Regs.PRD.all = INT32_MAX;              // C

#ifdef _LAUNCHXL_F28379D
        // Cpu Timer Prescale Counter => 10MHz / 10kHz => 1ms
        //
        //   PreScaler   "Value - 1"
        // 10.000 Hz - 1 => 9.999 => 0x270f
        //
        CpuTimer2Regs.TPR.bit.PSC = 0x0f;               //
        CpuTimer2Regs.TPRH.bit.PSCH = 0x27;             //


        CpuTimer2Regs.TPR.bit.TDDR = 0x0f;              //
        CpuTimer2Regs.TPRH.bit.TDDRH = 0x27;            //

        CpuTimer2Regs.TCR.bit.TSS = 0;                  // Start Timer

#elif defined(_STANDARD_REV_C)
        // Cpu Timer Prescale Counter => 25MHz / 25kHz => 1ms
        //
        //   PreScaler   "Value - 1"
        // 25.000 Hz - 1 => 24.999 => 0x6147
        //
        CpuTimer2Regs.TPR.bit.PSC = 0xA7;               //
        CpuTimer2Regs.TPRH.bit.PSCH = 0x61;             //


        CpuTimer2Regs.TPR.bit.TDDR = 0x0A7;              //
        CpuTimer2Regs.TPRH.bit.TDDRH = 0x061;            //

        CpuTimer2Regs.TCR.bit.TSS = 0;                  // Start Timer
#else
        // Cpu Timer Prescale Counter => 20MHz / 20kHz => 1ms
        //
        //   PreScaler   "Value - 1"
        // 20.000 Hz - 1 => 19.999 => 0x4E1F
        //
        CpuTimer2Regs.TPR.bit.PSC = 0x1f;               //
        CpuTimer2Regs.TPRH.bit.PSCH = 0x4e;             //


        CpuTimer2Regs.TPR.bit.TDDR = 0x01f;              //
        CpuTimer2Regs.TPRH.bit.TDDRH = 0x04E;            //

        CpuTimer2Regs.TCR.bit.TSS = 0;                  // Start Timer


#endif
    }
    else
    {

        //
        // Initialize timer period:
        //
        Timer->CPUFreqInMHz = Freq;
        Timer->PeriodInUSec = Period;
        temp = (long) (Freq * Period);

        //
        // Counter decrements PRD+1 times each period
        //
        Timer->RegsAddr->PRD.all = temp - 1;

        //
        // Set pre-scale counter to divide by 1 (SYSCLKOUT):
        //
        Timer->RegsAddr->TPR.all  = 0;
        Timer->RegsAddr->TPRH.all  = 0;

        //
        // Initialize timer control register:
        //
        Timer->RegsAddr->TCR.bit.TSS = 1;     // 1 = Stop timer, 0 = Start/Restart
        // Timer
        Timer->RegsAddr->TCR.bit.TRB = 1;     // 1 = reload timer
        Timer->RegsAddr->TCR.bit.SOFT = 0;
        Timer->RegsAddr->TCR.bit.FREE = 0;    // Timer Free Run Disabled
        Timer->RegsAddr->TCR.bit.TIE = 1;     // 0 = Disable/ 1 = Enable Timer
        // Interrupt

        //
        // Reset interrupt counter:
        //
        Timer->InterruptCount = 0;
    }
}


//
// End of file
//
