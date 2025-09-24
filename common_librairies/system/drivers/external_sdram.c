#include "external_sdram.h"
#include "device.h"

uint32_t TEST_STATUS;
uint16_t ErrCount = 0;

#define TEST_PASS             0xABCDABCD
#define TEST_FAIL             0xDEADDEAD
#define SDRAM_CS0_START_ADDR  0x80000000
#define SDRAM_CS0_SIZE        0x300000

void InitEmif1Gpio(void)
{
    uint16_t i;


    //GPIO_setPinConfig(GPIO_28_EM1CS4N);
    GPIO_setPinConfig(GPIO_29_EM1SDCKE);
    GPIO_setPinConfig(GPIO_30_EM1CLK);
    GPIO_setPinConfig(GPIO_31_EM1WEN);
    GPIO_setPinConfig(GPIO_32_EM1CS0N);
    //GPIO_setPinConfig(GPIO_33_EM1RNW);
    //GPIO_setPinConfig(GPIO_34_EM1CS2N);
    //GPIO_setPinConfig(GPIO_35_EM1CS3N);
    //GPIO_setPinConfig(GPIO_36_EM1WAIT);
    //GPIO_setPinConfig(GPIO_37_EM1OEN);

    //
    // Selecting address lines.
    //
    GPIO_setPinConfig(GPIO_38_EM1A0);
    GPIO_setPinConfig(GPIO_39_EM1A1);
    GPIO_setPinConfig(GPIO_40_EM1A2);
    GPIO_setPinConfig(GPIO_41_EM1A3);
    GPIO_setPinConfig(GPIO_44_EM1A4);
    GPIO_setPinConfig(GPIO_45_EM1A5);
    GPIO_setPinConfig(GPIO_46_EM1A6);
    GPIO_setPinConfig(GPIO_47_EM1A7);
    GPIO_setPinConfig(GPIO_48_EM1A8);
    GPIO_setPinConfig(GPIO_49_EM1A9);
    GPIO_setPinConfig(GPIO_50_EM1A10);
    GPIO_setPinConfig(GPIO_51_EM1A11);
    GPIO_setPinConfig(GPIO_52_EM1A12);

    //
    // Selecting data lines.
    //
    GPIO_setPinConfig(GPIO_53_EM1D31);
    GPIO_setPinConfig(GPIO_54_EM1D30);
    GPIO_setPinConfig(GPIO_55_EM1D29);
    GPIO_setPinConfig(GPIO_56_EM1D28);
    GPIO_setPinConfig(GPIO_57_EM1D27);
    GPIO_setPinConfig(GPIO_58_EM1D26);
    GPIO_setPinConfig(GPIO_59_EM1D25);
    GPIO_setPinConfig(GPIO_60_EM1D24);
    GPIO_setPinConfig(GPIO_61_EM1D23);
    GPIO_setPinConfig(GPIO_62_EM1D22);
    GPIO_setPinConfig(GPIO_63_EM1D21);
    GPIO_setPinConfig(GPIO_64_EM1D20);
    GPIO_setPinConfig(GPIO_65_EM1D19);
    GPIO_setPinConfig(GPIO_66_EM1D18);
    GPIO_setPinConfig(GPIO_67_EM1D17);
    GPIO_setPinConfig(GPIO_68_EM1D16);
    GPIO_setPinConfig(GPIO_69_EM1D15);
    GPIO_setPinConfig(GPIO_70_EM1D14);
    GPIO_setPinConfig(GPIO_71_EM1D13);
    GPIO_setPinConfig(GPIO_72_EM1D12);
    GPIO_setPinConfig(GPIO_73_EM1D11);
    GPIO_setPinConfig(GPIO_74_EM1D10);
    GPIO_setPinConfig(GPIO_75_EM1D9);
    GPIO_setPinConfig(GPIO_76_EM1D8);
    GPIO_setPinConfig(GPIO_77_EM1D7);
    GPIO_setPinConfig(GPIO_78_EM1D6);
    GPIO_setPinConfig(GPIO_79_EM1D5);
    GPIO_setPinConfig(GPIO_80_EM1D4);
    GPIO_setPinConfig(GPIO_81_EM1D3);
    GPIO_setPinConfig(GPIO_82_EM1D2);
    GPIO_setPinConfig(GPIO_83_EM1D1);
    GPIO_setPinConfig(GPIO_85_EM1D0);

    //
    // Selecting RAS, CAS, DQM & Bank select lines.
    //
    GPIO_setPinConfig(GPIO_86_EM1CAS);
    GPIO_setPinConfig(GPIO_87_EM1RAS);
    GPIO_setPinConfig(GPIO_88_EM1DQM0);
    GPIO_setPinConfig(GPIO_89_EM1DQM1);
    GPIO_setPinConfig(GPIO_90_EM1DQM2);
    GPIO_setPinConfig(GPIO_91_EM1DQM3);
    GPIO_setPinConfig(GPIO_92_EM1BA1);
    GPIO_setPinConfig(GPIO_93_EM1BA0);
    //GPIO_setPinConfig(GPIO_94_EM1A21);

    //
    // Configure Data pins for Async mode.
    //
    for(i = 53;i <= 85;i++)
    {
        if(i != 84)
        {
            GPIO_setPadConfig(i, GPIO_PIN_TYPE_PULLUP);
            GPIO_setQualificationMode(i, GPIO_QUAL_ASYNC);
        }
    }
    for(i = 88; i <= 91; i++)
    {
        GPIO_setPadConfig(i, GPIO_PIN_TYPE_PULLUP);
        GPIO_setQualificationMode(i, GPIO_QUAL_ASYNC);
    }
}

void initEMIF1(void)
{
    volatile long i=0;
    char ErrCount_local = 0;


    //
    //Configure GPIO pins for EMIF1
    //
    InitEmif1Gpio();


    Emif1Regs.SDRAM_CR.bit.SR = 1;


    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EMIF1);
    //Configure to run EMIF1 on half Rate (EMIF1CLK = CPU1SYSCLK/2)
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EMIF1CLKDIV = 0x1;
    EDIS;

    Emif1Regs.SDRAM_CR.bit.SR = 0;


    EALLOW;
    //
    // Grab EMIF1 For CPU1
    //
//    Emif1ConfigRegs.EMIF1MSEL.bit.KEY = 0x93A5CE7;
//    Emif1ConfigRegs.EMIF1MSEL.bit.MSEL_EMIF1 = 0x01;
//    if(Emif1ConfigRegs.EMIF1MSEL.bit.MSEL_EMIF1 != 0x01)
//    {
//        ErrCount_local++;
//    }
//
//    //
//    //Disable Access Protection (CPU_FETCH/CPU_WR/DMA_WR)
//    //
    Emif1ConfigRegs.EMIF1ACCPROT0.all = 0x0;
   if(Emif1ConfigRegs.EMIF1ACCPROT0.all != 0x0)
    {
        ErrCount_local++;
   }
//
//    //
//    // Commit the configuration related to protection. Till this bit remains
//    // set content of EMIF1ACCPROT0 register can't be changed.
//    //
//    Emif1ConfigRegs.EMIF1COMMIT.all = 0x1;
//    if(Emif1ConfigRegs.EMIF1COMMIT.all != 0x1)
//    {
//        ErrCount_local++;
//    }
//
//    //
//    // Lock the configuration so that EMIF1COMMIT register can't be changed
//    // any more.
//    //
//    Emif1ConfigRegs.EMIF1LOCK.all = 0x1;
//    if(Emif1ConfigRegs.EMIF1LOCK.all != 1)
//    {
//        ErrCount_local++;
//    }
   EDIS;






   Emif1Regs.SDRAM_RCR.bit.REFRESH_RATE = 782;
   Emif1Regs.SDRAM_TR.bit.T_RFC         =   5;
   Emif1Regs.SDRAM_TR.bit.T_RP          =   1;
   Emif1Regs.SDRAM_TR.bit.T_RCD         =   1;
   Emif1Regs.SDRAM_TR.bit.T_WR          =   1;
   Emif1Regs.SDRAM_TR.bit.T_RAS         =   4;
   Emif1Regs.SDRAM_TR.bit.T_RC          =   5;
   Emif1Regs.SDRAM_TR.bit.T_RRD         =   1;
   Emif1Regs.SDR_EXT_TMNG.bit.T_XS      =   6;
   Emif1Regs.SDRAM_CR.all = (0 << 14) | // NM
                                             (2 <<  9) | // CL
                                             (1 <<  8) | // LOCK
                                             (2 <<  4) | // IBANK
                                             (1 <<  0);  // PAGESIZE

//    //
//    // Configure SDRAM control registers
//    // Need to be programmed based on SDRAM Data-Sheet.
//    // Configure SDRAM timing for 100MHz (example timing, match your needs)
//    A.SDRAM_TR.bit.T_RFC = 6;    // tRC = 68ns / 10ns = 7
//    Emif1Regs.SDRAM_TR.bit.T_RP = 1;     // tRP = 20ns / 10ns = 2
//    Emif1Regs.SDRAM_TR.bit.T_RCD = 1;    // tRCD = 20ns / 10ns = 2
//    Emif1Regs.SDRAM_TR.bit.T_WR = 1;     // tWR = 20ns / 10ns = 2
//    Emif1Regs.SDRAM_TR.bit.T_RAS = 4;    // tRAS = 50ns / 10ns = 5
//    Emif1Regs.SDRAM_TR.bit.T_RC = 6;     // tRC = 68ns / 10ns = 7
//    Emif1Regs.SDRAM_TR.bit.T_RRD = 1;    // tRRD = 14ns / 10ns = 2
//
//
//    Emif1Regs.SDR_EXT_TMNG.bit.T_XS = 6;  // tXSR = 68ns / 10ns = 7
//
//    // SDRAM refresh rate = 64ms/8192 = 7.8125μs
//    Emif1Regs.SDRAM_RCR.bit.REFRESH_RATE = 782;
//
//
//
//
//
//    //
//    //PAGESIZE=0 (256 elements per ROW), IBANK = 2 (4 BANK), CL = 3,
//    //NM = 0 (32bit)
//    //
//    //Emif1Regs.SDRAM_CR.all = 0x00000720;
//    Emif1Regs.SDRAM_CR.bit.PAGESIGE=0x00; // 256 words
//    Emif1Regs.SDRAM_CR.bit.IBANK=0x02; //   4 BANKS
//    Emif1Regs.SDRAM_CR.bit.BIT_11_9_LOCK=0x00;
//    Emif1Regs.SDRAM_CR.bit.CL=0x03;
//    Emif1Regs.SDRAM_CR.bit.NM=0x00; //32 bits
//    Emif1Regs.SDRAM_CR.bit.PDWR=0x00;
//    Emif1Regs.SDRAM_CR.bit.PD=0x00;




    //
    //Add some delay
    //
    for(i=0;i<100000;i++) { }


    //    Emif1ConfigRegs.EMIF1MSEL.all = 0x2;
    //    if(Emif1ConfigRegs.EMIF1MSEL.all != 0x2)
    //    {
    //        ErrCount_local++;
    //    }
    //
    //    Emif1ConfigRegs.EMIF1MSEL.all = 0x1;
    //    if(Emif1ConfigRegs.EMIF1MSEL.all != 0x1)
    //    {
    //        ErrCount_local++;
    //    }

}







//
// sdram_data_walk - This function performs a walking 0 & 1 For SDRAM RD & WR
//
char
sdram_data_walk(Uint32 start_addr, Uint32 mem_size)
{
    Uint32  XM_p, XMEM_p;
    unsigned long sdram_rdl;
    unsigned long sdram_wdl;
    int i;
    int k;
    int m;

    XM_p = start_addr;

    for (i=0; i < mem_size; i=i+64)
    {
        for (m=0; m < 2; m++)
        {
            XMEM_p = XM_p;

            //
            //Write loop
            //
            sdram_wdl = 0x0001;
            for (k=0; k < 32; k++)
            {
                if(m==0)
                {
                    __addr32_write_uint32(XMEM_p, sdram_wdl);
                }
                else
                {
                    __addr32_write_uint32(XMEM_p, ~sdram_wdl);
                }
                XMEM_p = XMEM_p+2;
                sdram_wdl   = sdram_wdl<<1;
            }

            //
            //Read loop
            //
            XMEM_p = XM_p;
            sdram_wdl = 0x0001;
            for (k=0; k < 32; k++)
            {
                sdram_rdl =  __addr32_read_uint32(XMEM_p);
                if(m==1)
                {
                    sdram_rdl = ~sdram_rdl;
                }
                if(sdram_rdl != sdram_wdl)
                {
                    return(1);
                }

                XMEM_p = XMEM_p+2;
                sdram_wdl=sdram_wdl<<1;
            }
        }
        XM_p = XMEM_p;
    }
    return(0);
}

//
// sdram_addr_walk - This function performs a toggle on each address bit.
//                   In this case memory assumed is 4Mb.
//                   MA = BA[1:0]ROW[11:0]COL[7:0]
//
char
sdram_addr_walk(Uint32 start_addr, Uint32 addr_size)
{
    Uint32  XMEM_p;
    unsigned long sdram_rdl;
    unsigned long sdram_wdl;
    int i;
    unsigned long xshift;
    unsigned long xshift2;

    //
    //Write loop
    //
    xshift = 0x00000001;
    sdram_wdl = 0x5678abcd;
    for (i=0; i < addr_size; i++)
    {
        xshift2 = ((xshift+1)<<1);
        XMEM_p =  start_addr + xshift2;
        __addr32_write_uint32(XMEM_p, sdram_wdl);
        sdram_wdl = sdram_wdl+0x11111111;
        xshift   = xshift<<1;
    }

    //
    //Read loop
    //
    xshift = 0x00000001;
    sdram_wdl = 0x5678abcd;
    for (i=0; i < addr_size; i++)
    {
        xshift2 = ((xshift+1)<<1);
        XMEM_p= start_addr + xshift2;
        sdram_rdl = __addr32_read_uint32(XMEM_p);
        if( sdram_rdl != sdram_wdl)
        {
            return(1);
        }
        xshift   = xshift<<1;
        sdram_wdl = sdram_wdl + 0x11111111;
    }
    return(0);
}

//
// sdram_data_size - This function performs different data type
//                  (HALFWORD/WORD) access.
//
char
sdram_data_size(Uint32 start_addr, Uint32 mem_size)
{
    unsigned short sdram_rds;
    unsigned long  sdram_rdl;
    unsigned short sdram_wds;
    unsigned long  sdram_wdl;
    int i;
    Uint32 XMEM_p;

    //
    //Write data short
    //
    XMEM_p = start_addr;
    sdram_wds = 0x0605;
    for (i=0; i < 2; i++)
    {
        __addr32_write_uint16(XMEM_p, sdram_wds);
        XMEM_p++;
        sdram_wds += 0x0202;
    }

    //
    //Write data long
    //
    sdram_wdl = 0x0C0B0A09;
    for (i=0; i < 2; i++)
    {
        __addr32_write_uint32(XMEM_p, sdram_wdl);
        XMEM_p = XMEM_p+2;
        sdram_wdl += 0x04040404;
    }

    //
    //Read data short
    //
    XMEM_p = start_addr;
    sdram_wds=0x0605;
    for (i=0; i < 6; i++)
    {
        sdram_rds = __addr32_read_uint16(XMEM_p);
        if( sdram_rds != sdram_wds)
        {
            return(1);
        }
        XMEM_p++;
        sdram_wds += 0x0202;
    }

    //
    //Read data long
    //
    XMEM_p = start_addr;
    sdram_wdl=0x08070605;
    for (i=0; i < 3; i++)
    {
        sdram_rdl = __addr32_read_uint32(XMEM_p);
        if( sdram_rdl != sdram_wdl)
        {
            return(1);
        }
        XMEM_p = XMEM_p+2;
        sdram_wdl += 0x04040404;
    }
    return(0);
}

//
// sdram_read_write - This function performs simple read/write accesses
//                    to memory.
//
char
sdram_read_write(Uint32 start_addr, Uint32 mem_size)
{
    unsigned long mem_rdl;
    unsigned long mem_wdl;
    Uint32 XMEM_p;
    Uint32 i;

    //
    //Write data
    //
    XMEM_p = start_addr;

    //
    //Fill memory
    //
    mem_wdl = 0x01234567;
    for (i=0; i < mem_size; i++)
    {
        __addr32_write_uint32(XMEM_p, mem_wdl);
        XMEM_p = XMEM_p+2;
        mem_wdl += 0x11111111;
    }

    //
    //Verify memory
    //
    mem_wdl = 0x01234567;
    XMEM_p = start_addr;
    for (i=0; i < mem_size; i++)
    {
        mem_rdl = __addr32_read_uint32(XMEM_p);
        if( mem_rdl != mem_wdl)
        {
            return(1);
        }
        XMEM_p = XMEM_p+2;
        mem_wdl += 0x11111111;
    }

    XMEM_p = start_addr;
    mem_wdl = 0xeeeeeeee;
    for (i=0; i < mem_size; i++)
    {
        __addr32_write_uint32(XMEM_p, mem_wdl);
        XMEM_p = XMEM_p+2;
    }
    return(0);
}


void run_sdram_tests(void)
{
    uint16_t ErrCount_local;

    ErrCount_local = sdram_read_write(SDRAM_CS0_START_ADDR, SDRAM_CS0_SIZE);
    ErrCount += ErrCount_local;
    //ESTOP0;
//    ErrCount_local = sdram_addr_walk(SDRAM_CS0_START_ADDR, 15);
//    ErrCount += ErrCount_local;
//
//    ErrCount_local = sdram_data_walk(SDRAM_CS0_START_ADDR, SDRAM_CS0_SIZE);
//    ErrCount += ErrCount_local;
//
//    ErrCount_local = sdram_data_size(SDRAM_CS0_START_ADDR, 4);
//    ErrCount += ErrCount_local;


    TEST_STATUS = (ErrCount == 0x0) ? TEST_PASS : TEST_FAIL;
}
