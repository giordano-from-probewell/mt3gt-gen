#include "dac.h"
#include "F28x_Project.h"






//
// Globals
//

volatile struct DAC_REGS* DAC_PTR[4] = {0x0,&DacaRegs,&DacbRegs,&DaccRegs};


//
// configureDAC - Enable and configure the requested DAC module
//
void DAC_init(uint16_t dac_num)
{
    EALLOW;

    DAC_PTR[dac_num]->DACCTL.bit.DACREFSEL = REFERENCE;
    DAC_PTR[dac_num]->DACOUTEN.bit.DACOUTEN = 1;
    DAC_PTR[dac_num]->DACVALS.all = 0;

    DELAY_US(10); // Delay for buffered DAC to power up

    EDIS;
}
