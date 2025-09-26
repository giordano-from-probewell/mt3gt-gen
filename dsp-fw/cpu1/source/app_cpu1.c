#include "app_cpu1.h"
#include "my_time.h"
#include "cla1_standard_shared.h"

#include "SFO_V8.h"



#include "ipc_simple.h"
#include "frequency.h"




#define TO_CPU1 0
#define TO_CPU2 1
#define CONNECT_TO_CLA1 0
#define CONNECT_TO_DMA 1
#define CONNECT_TO_CLA2 2
#define ENABLE 1
#define DISABLE 0

#define CPU1_CLA1(x)                        \
        EALLOW;                             \
        DevCfgRegs.DC1.bit.CPU1_CLA1 = x;   \
        EDIS
#define CPU2_CLA1(x)                        \
        EALLOW;                             \
        DevCfgRegs.DC1.bit.CPU2_CLA1 = x;   \
        EDIS
#define WAITSTEP asm(" RPT #255 || NOP")







void _app_gpio_init(void);




void _cla1_init(void);
void _cla1_interruption_config(void);


__interrupt void access_violation(void);
__interrupt void xint1_isr(void);





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


volatile uint32_t fetchAddress;





bool flag_zc = false; //zero crossing mark



void app_init_cpu1(application_t *app)
{
    app->app_cpu1_state = APP_STATE_START;
    uint8_t address = 0;

    _app_gpio_init();




    _cla1_init();

    if(GPIO_ReadPin(MY_ADDR_PIN0) == 1 )
        address|=0b001;
    if(GPIO_ReadPin(MY_ADDR_PIN1) == 1 )
        address|=0b010;
    if(GPIO_ReadPin(MY_ADDR_PIN2) == 1 )
        address|=0b100;

    app->id.data.full.my_address = address;

    memcpy(app->id.data.full.probewell_part_number, "9015-6200:GEN   ", 16);
    memcpy(app->id.data.full.serial_number,         "22531515        ", 16);
    memcpy(app->id.data.full.fabrication_date,      "20260101", 8);
    memcpy(app->id.data.full.last_verfication_date, "20260101", 8);


    Cla1ForceTask8andWait();
    WAITSTEP;
}



extern IPC_MessageQueue_t messageQueue;
extern IPC_Message_t      TxMsg;



void app_run_cpu1(application_t *app)
{
    my_time_t time_actual = 0;
    time_actual = my_time(NULL);






    // Poll uma mensagem vinda da CPU2 e despache (se existir)
    ipc_rx_service_cpu1();

    //    // (opcional) se a CPU1 também envia para a CPU2, limpe o flag após ACK:
    //    ipc_tx_service(); // sua função CPU1 que faz clear do C1->C2 após ver g_c2_to_c1_ack


    switch (app->app_cpu1_state) {

    case APP_STATE_IDLE:
        app->app_cpu1_state = APP_STATE_START;
        break;

    case APP_STATE_START:
    {

        ipc_simple_init_cpu1();

        // Enable Global Interrupt (INTM) and real time interrupt (DBGM)
        EINT;
        ERTM;
    }

    app->app_cpu1_state = APP_STATE_RUNNING;
    break;

    case APP_STATE_RUNNING:

        break;

    case APP_STATE_ERROR:
        break;
    }
}



void _app_gpio_init(void)
{

    GPIO_setPadConfig(GPIO_DEBUG_0, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_DEBUG_0, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_DEBUG_0, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(GPIO_DEBUG_0_CONFIG);
    GPIO_setMasterCore(GPIO_DEBUG_0, GPIO_CORE_CPU1);

    GPIO_setPadConfig(GPIO_DEBUG_1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_DEBUG_1, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_DEBUG_1, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(GPIO_DEBUG_1_CONFIG);
    GPIO_setMasterCore(GPIO_DEBUG_1, GPIO_CORE_CPU1_CLA1);

    GPIO_setPadConfig(GPIO_DEBUG_2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(GPIO_DEBUG_2, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(GPIO_DEBUG_2, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(GPIO_DEBUG_2_CONFIG);
    GPIO_setMasterCore(GPIO_DEBUG_2, GPIO_CORE_CPU1);

    GPIO_WritePin(GPIO_DEBUG_0, 0);
    GPIO_WritePin(GPIO_DEBUG_1, 0);
    GPIO_WritePin(GPIO_DEBUG_2, 0);


    GPIO_setPadConfig(MY_ADDR_PIN0, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(MY_ADDR_PIN0, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(MY_ADDR_PIN0, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(MY_ADDR_PIN0_CONFIG);
    GPIO_setPadConfig(MY_ADDR_PIN1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(MY_ADDR_PIN1, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(MY_ADDR_PIN1, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(MY_ADDR_PIN1_CONFIG);
    GPIO_setPadConfig(MY_ADDR_PIN2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(MY_ADDR_PIN2, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(MY_ADDR_PIN2, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(MY_ADDR_PIN2_CONFIG);

    GPIO_setPadConfig(CLKGEN_ON, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLKGEN_ON, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLKGEN_ON, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(CLKGEN_ON_CONFIG);
    GPIO_WritePin(CLKGEN_ON, 1);

    GPIO_setPadConfig(CLKGEN_ADS_SEL_IN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLKGEN_ADS_SEL_IN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLKGEN_ADS_SEL_IN, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(CLKGEN_ADS_SEL_IN_CONFIG);
    GPIO_WritePin(CLKGEN_ADS_SEL_IN, 1);

    GPIO_setPadConfig(CLKGEN_ADS_SEL1_OUT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLKGEN_ADS_SEL1_OUT, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLKGEN_ADS_SEL1_OUT, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(CLKGEN_ADS_SEL1_OUT_CONFIG);
    GPIO_WritePin(CLKGEN_ADS_SEL1_OUT, 1);

    GPIO_setPadConfig(CLKGEN_ADS_SEL2_OUT, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLKGEN_ADS_SEL2_OUT, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLKGEN_ADS_SEL2_OUT, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(CLKGEN_ADS_SEL2_OUT_CONFIG);
    GPIO_WritePin(CLKGEN_ADS_SEL2_OUT, 0);

    GPIO_setPadConfig(CLKGEN_BSP_ENA, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLKGEN_BSP_ENA, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLKGEN_BSP_ENA, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(CLKGEN_BSP_ENA_CONFIG);
    GPIO_WritePin(CLKGEN_BSP_ENA, 1);

    //    //fs 106
    //    GPIO_setPadConfig(106, GPIO_PIN_TYPE_STD);
    //    GPIO_setDirectionMode(106, GPIO_DIR_MODE_OUT);
    //    GPIO_setQualificationMode(106,GPIO_QUAL_6SAMPLE);
    //    GPIO_setPinConfig(GPIO_106_GPIO106);
    //    GPIO_WritePin(106, 1);
    //    //ssc0 107
    //    GPIO_setPadConfig(107, GPIO_PIN_TYPE_STD);
    //    GPIO_setDirectionMode(107, GPIO_DIR_MODE_OUT);
    //    GPIO_setQualificationMode(107,GPIO_QUAL_6SAMPLE);
    //    GPIO_setPinConfig(GPIO_107_GPIO107);
    //    GPIO_WritePin(107, 0);
    //    //ssc1 167
    //    GPIO_setPadConfig(167, GPIO_PIN_TYPE_STD);
    //    GPIO_setDirectionMode(167, GPIO_DIR_MODE_OUT);
    //    GPIO_setQualificationMode(167,GPIO_QUAL_6SAMPLE);
    //    GPIO_setPinConfig(GPIO_167_GPIO167);
    //    GPIO_WritePin(167, 0);

    GPIO_setPadConfig(Si5341B_RST, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(Si5341B_RST, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(Si5341B_RST, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(Si5341B_RST_CONFIG);
    GPIO_WritePin(Si5341B_RST, 0);

    GPIO_setPadConfig(Si5341B_OE, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(Si5341B_OE, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(Si5341B_OE, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(Si5341B_OE_CONFIG);
    GPIO_WritePin(Si5341B_OE, 0);

    // I2C
    {
        // I2CA -> clock to ADS1278
        GPIO_setDirectionMode(SDA_ADS, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SDA_ADS, GPIO_PIN_TYPE_STD);
        GPIO_setMasterCore(SDA_ADS, GPIO_CORE_CPU1);
        GPIO_setQualificationMode(SDA_ADS, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SDA_ADS_CONFIG);
        GPIO_setMasterCore(SDA_ADS, GPIO_CORE_CPU1);

        GPIO_setDirectionMode(SCL_ADS, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SCL_ADS, GPIO_PIN_TYPE_STD);
        GPIO_setMasterCore(SCL_ADS, GPIO_CORE_CPU1);
        GPIO_setQualificationMode(SCL_ADS, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SCL_ADS_CONFIG);
        GPIO_setMasterCore(SCL_ADS, GPIO_CORE_CPU1);

        // I2Cb -> clock to Generators
        GPIO_setDirectionMode(SDA_GEN, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SDA_GEN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setMasterCore(SDA_GEN, GPIO_CORE_CPU1);
        GPIO_setQualificationMode(SDA_GEN, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SDA_GEN_CONFIG);
        GPIO_setMasterCore(SDA_GEN, GPIO_CORE_CPU1);

        GPIO_setDirectionMode(SCL_GEN, GPIO_DIR_MODE_OUT);
        GPIO_setPadConfig(SCL_GEN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setMasterCore(SCL_GEN, GPIO_CORE_CPU1);
        GPIO_setQualificationMode(SCL_GEN, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SCL_GEN_CONFIG);
        GPIO_setMasterCore(SCL_GEN, GPIO_CORE_CPU1);


    }

    // Initialize SCI
    {
        //CLI
        GPIO_setPinConfig(CLI_SCIRX_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(CLI_SCITX_GPIO_PIN_CONFIG);
        GPIO_setQualificationMode(CLI_SCITX_GPIO, GPIO_QUAL_ASYNC);
        GPIO_setQualificationMode(CLI_SCIRX_GPIO, GPIO_QUAL_ASYNC);

        //COMM
        GPIO_setPinConfig(COMMS_SCIRX_GPIO_PIN_CONFIG);
        GPIO_setPinConfig(COMMS_SCITX_GPIO_PIN_CONFIG);
        GPIO_setQualificationMode(COMMS_SCITX_GPIO, GPIO_QUAL_ASYNC);
        GPIO_setQualificationMode(COMMS_SCIRX_GPIO, GPIO_QUAL_ASYNC);
    }
    //SPI
    {
        //SPIA
        GPIO_setPinConfig(SPIA_MOSI_PIN_CONFIG);
        GPIO_setPadConfig(SPIA_MOSI_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setQualificationMode(SPIA_MOSI_GPIO, GPIO_QUAL_ASYNC);
        GPIO_setMasterCore(SPIA_MOSI_GPIO, GPIO_CORE_CPU1);

        GPIO_setPinConfig(SPIA_MISO_PIN_CONFIG);
        GPIO_setPadConfig(SPIA_MISO_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setQualificationMode(SPIA_MISO_GPIO, GPIO_QUAL_ASYNC);
        GPIO_setMasterCore(SPIA_MISO_GPIO, GPIO_CORE_CPU1);

        GPIO_setPinConfig(SPIA_CLK_PIN_CONFIG);
        GPIO_setPadConfig(SPIA_CLK_GPIO, GPIO_PIN_TYPE_STD);
        GPIO_setQualificationMode(SPIA_CLK_GPIO, GPIO_QUAL_ASYNC);
        GPIO_setMasterCore(SPIA_CLK_GPIO, GPIO_CORE_CPU1);


        //    //SPIB
        //    GPIO_setPinConfig(SPIB_MOSI_PIN_CONFIG);
        //    GPIO_setPadConfig(SPIB_MOSI_GPIO, GPIO_PIN_TYPE_STD);
        //    GPIO_setQualificationMode(SPIB_MOSI_GPIO, GPIO_QUAL_ASYNC);
        //
        //    GPIO_setPinConfig(SPIB_MISO_PIN_CONFIG);
        //    GPIO_setPadConfig(SPIB_MISO_GPIO, GPIO_PIN_TYPE_STD);
        //    GPIO_setQualificationMode(SPIB_MISO_GPIO, GPIO_QUAL_ASYNC);
        //
        //    GPIO_setPinConfig(SPIB_CLK_PIN_CONFIG);
        //    GPIO_setPadConfig(SPIB_CLK_GPIO, GPIO_PIN_TYPE_STD);
        //    GPIO_setQualificationMode(SPIB_CLK_GPIO, GPIO_QUAL_ASYNC);



        //CS
        GPIO_writePin(GPIO_PIN_CS_A1,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A1,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A1,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A1_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A1, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A2,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A2,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A2,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A2_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A2, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A3,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A3,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A3,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A3_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A3, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A4,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A4,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A4,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A4_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A4, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A5,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A5,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A5,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A5_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A5, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A6,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A6,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A6,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A6_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A6, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A7,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A7,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A7,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A7_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A7, GPIO_CORE_CPU2);

        GPIO_writePin(GPIO_PIN_CS_A8,1);
        GPIO_setDirectionMode(GPIO_PIN_CS_A8,GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(GPIO_PIN_CS_A8,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_PIN_CS_A8_PIN_CONFIG);
        GPIO_setMasterCore(GPIO_PIN_CS_A8, GPIO_CORE_CPU2);
    }

    { //buzzer

        GPIO_setPadConfig(168, GPIO_PIN_TYPE_STD);
        GPIO_setDirectionMode(168, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(168, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_168_EPWM12B);
        GPIO_setMasterCore(168, GPIO_CORE_CPU2);
    }
}





void _cla1_init(void)
{

    //
    // Suppressing #770-D conversion from pointer to smaller integer
    // The CLA address range is 16 bits so the addresses passed to the MVECT
    // registers will be in the lower 64KW address space. Turn the warning
    // back on after the MVECTs are assigned addresses
    //
#pragma diag_suppress=770

#ifdef CPU1
    EALLOW;
    CPU1_CLA1(ENABLE); // Enable CPU1.CLA module
    CPU2_CLA1(ENABLE); // Enable CPU2.CLA module

    CONNECT_SD1(TO_CPU1); // Connect SDFM1 to CPU1
    // CONNECT_SD2(TO_CPU1);       //Connect SDFM2 to CPU1
    VBUS32_1(CONNECT_TO_CLA1); // Connect VBUS32_1 (SDFM bus) to CPU1
    EDIS;
#endif

    // Configure the CLA memory spaces
    _cla1_memory_config();

#ifdef CPU1
    // Configure the CLA task vectors for CPU1
    _cla1_interruption_config();
#endif
#ifdef CPU2
    // Configure the CLA task vectors for CPU2
    CLA_initCpu2Cla();
#endif

}

// CLA_initCpu1Cla - Initialize CLA tasks and end of task ISRs
void _cla1_interruption_config(void)
{
    EALLOW;

    // Compute all CLA task vectors
    // On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
    // Adding a pragma to suppress the warnings when casting the CLA tasks'
    // function pointers to uint16_t. Compiler believes this to be improper
    // although the CLA only has 16-bit addressing.
#pragma diag_suppress = 770

    //
    // Compute all CLA task vectors
    // On Type-2 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
    // Assign the task vectors
    //
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_1, (uint16_t)&Cla1Task1);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_2, (uint16_t)&Cla1Task2);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_3, (uint16_t)&Cla1Task3);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_4, (uint16_t)&Cla1Task4);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_5, (uint16_t)&Cla1Task5);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_6, (uint16_t)&Cla1Task6);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_7, (uint16_t)&Cla1Task7);
    CLA_mapTaskVector(CLA1_BASE, CLA_MVECT_8, (uint16_t)&Cla1Task8);
#pragma diag_warning=770

    // Enable the IACK instruction to start a task on CLA in software
    // for all  8 CLA tasks. Also, globally enable all 8 tasks (or a
    // subset of tasks) by writing to their respective bits in the
    // MIER register
    __asm("   RPT #3 || NOP");
    CLA_enableIACK(CLA1_BASE);
    CLA_enableTasks(CLA1_BASE, CLA_TASKFLAG_ALL);

    //
    // Configure the vectors for the end-of-task interrupt for all
    // 8 tasks
    Interrupt_register(INT_CLA1_1, cla1Isr1);
    Interrupt_register(INT_CLA1_2, cla1Isr2);
    Interrupt_register(INT_CLA1_3, cla1Isr3);
    Interrupt_register(INT_CLA1_4, cla1Isr4);
    Interrupt_register(INT_CLA1_5, cla1Isr5);
    Interrupt_register(INT_CLA1_6, cla1Isr6);
    Interrupt_register(INT_CLA1_7, cla1Isr7);
    Interrupt_register(INT_CLA1_8, cla1Isr8);

    // Enable CLA interrupts at the group and subgroup levels
    Interrupt_enable(INT_CLA1_1);
    Interrupt_enable(INT_CLA1_2);
    Interrupt_enable(INT_CLA1_3);
    Interrupt_enable(INT_CLA1_4);
    Interrupt_enable(INT_CLA1_5);
    Interrupt_enable(INT_CLA1_6);
    Interrupt_enable(INT_CLA1_7);
    Interrupt_enable(INT_CLA1_8);
    IER |= INTERRUPT_CPU_INT11;


}

// Cla_initMemoryMap - Initialize CLA memory map
//
void _cla1_memory_config(void)
{



    // Give CLA control over program and data RAM(s)
    //     - The MemCfgRegs register is described in TRM 2.14.17
    //  Configure LS0RAM and LS1RAM as program spaces for the CLA
    //  First configure the CLA to be the master for LS5 and then
    //  set the spaces to be program blocks

    EALLOW;
    //
    // Initialize and wait for CLA1ToCPUMsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
    while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1)
    {
    };

    //
    // Initialize and wait for CPUToCLA1MsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
    while (MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1)
    {
    };

    //
    // Select LS0 LS1 RAM to be the programming space for the CLA
    //
    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 1;

    //
    // Select LS5 to be data RAM for the CLA
    //
    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1;     // LS3RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 0; // LS3RAM setup as data memory
    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;     // LS2RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 0; // LS2RAM setup as data memory
    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;     // LS3RAM is shared between CPU and CLA
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 0; // LS3RAM setup as data memory

    // Detect any CLA fetch access violations, enable
    // interrupt for it (TRM SPRUHM8, 2.11.1.7.4 & 2.14.18)
    AccessProtectionRegs.NMAVSET.bit.CLA1FETCH = 1;
    AccessProtectionRegs.NMAVINTEN.bit.CLA1FETCH = 1;
    // Set the ISR for access violation fault
    PieVectTable.RAM_ACCESS_VIOLATION_INT = access_violation;
    PieCtrlRegs.PIEIER12.bit.INTx12 = 1;
    IER |= M_INT12;

    EDIS;
}





#pragma CODE_SECTION(access_violation, "ramfuncs")
__interrupt void access_violation(void)
{
    // Read the fetch address where the violation occurred
    fetchAddress = AccessProtectionRegs.NMCLA1FAVADDR;

    EALLOW;
    // clear the fault
    AccessProtectionRegs.NMAVCLR.bit.CLA1FETCH = 1;
    EDIS;
    //__asm(" ESTOP0");
    // Acknowledge the RAM access violation interrupt
    PieCtrlRegs.PIEACK.all = M_INT12;
}

//
// cla1Isr1 - CLA 1 ISR 1
//
interrupt void cla1Isr1()
{

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr2 - CLA 1 ISR 2
//
interrupt void cla1Isr2()
{
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr3 - CLA 1 ISR 3
//
interrupt void cla1Isr3()
{
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr4 - CLA 1 ISR 4
//
interrupt void cla1Isr4()
{
    //    asm(" ESTOP0");
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr5 - CLA 1 ISR 5
//
interrupt void cla1Isr5()
{
    //    asm(" ESTOP0");
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr6 - CLA 1 ISR 6
//
interrupt void cla1Isr6()
{
    //    asm(" ESTOP0");
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr7 - CLA 1 ISR 7
//
interrupt void cla1Isr7()
{
    //    asm(" ESTOP0");
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cla1Isr8 - CLA 1 ISR 8
//
interrupt void cla1Isr8()
{
    //    asm(" ESTOP0");
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
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


