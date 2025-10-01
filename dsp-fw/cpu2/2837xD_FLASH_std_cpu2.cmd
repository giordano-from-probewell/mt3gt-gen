/*
Section Name	Description								Section Type	Link Location
.text			Executable code and constants			Initalized		Load into non-volatile memory (flash). Run time-critical code in SARAM. See Note.
.cinit			Initalized global and static variables	Initalized		Non volatile memory (flash)
.econst		Constant data (e.g. const int k = 3;)	Initalized		Non volatile memory (flash)
.switch		Tables for switch statements			Initalized		Non volatile memory (flash)
.ebss			Global and static variables				Uninitalized	Volatile memory (SRAM)
.stack			Stack space								Uninitalized	Volatile memory (SRAM). The C28x Stack Pointer (SP) is 16 bits: .stack must be in the low 64k words.
.esysmem		Memory for malloc type functions		Uninitalized	Volatile memory (SRAM)
.reset			Reset vector							Initalized		This section is not typically used by C28x devices and should be set to type=DSECT.
*/

MEMORY
{
PAGE 0 :  /* Program Memory */
          /* Memory (RAM/FLASH) blocks can be moved to PAGE1 for data allocation */
          /* BEGIN is used for the "boot to SARAM" bootloader mode   */

   BEGIN           	: origin = 0x080000, length = 0x000002
   //RAMM0          : origin = 0x000122, length = 0x0002DE
   //RAMM1          : origin = 0x000400, length = 0x0003F8
   //RAMM1_RSVD     : origin = 0x0007F8, length = 0x000008  /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
   RAMD0_1          : origin = 0x00B000, length = 0x001000
   //RAMD0          : origin = 0x00B000, length = 0x000800
   //RAMD1          : origin = 0x00B800, length = 0x000800
   RAMLS_CLA1_PROG  : origin = 0x008000, length = 0x000800
   //RAMLS0         : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   //RAMLS2         : origin = 0x009000, length = 0x000800
   //RAMLS3         : origin = 0x009800, length = 0x000800
   RAMLS4      	: origin = 0x00A000, length = 0x000800
   RAMLS5         : origin = 0x00A800, length = 0x000800
   RESET           	: origin = 0x3FFFC0, length = 0x000002

   /* Flash sectors */
   FLASHA           : origin = 0x080002, length = 0x001FFE	/* on-chip Flash */
   FLASH_CPU2       : origin = 0x082000, length = 0x006000	/* on-chip Flash */
   //FLASHB         : origin = 0x082000, length = 0x002000	/* on-chip Flash */
   //FLASHC         : origin = 0x084000, length = 0x002000	/* on-chip Flash */
   //FLASHD         : origin = 0x086000, length = 0x002000	/* on-chip Flash */
   FLASH_TEXT2      : origin = 0x088000, length = 0x010000	/* on-chip Flash */
   //FLASHE         : origin = 0x088000, length = 0x008000	/* on-chip Flash */
   //FLASHF         : origin = 0x090000, length = 0x008000	/* on-chip Flash */
   FLASHG           : origin = 0x098000, length = 0x008000	/* on-chip Flash */
   FLASHH           : origin = 0x0A0000, length = 0x008000	/* on-chip Flash */
   FLASHI           : origin = 0x0A8000, length = 0x008000	/* on-chip Flash */
   FLASHJ           : origin = 0x0B0000, length = 0x008000	/* on-chip Flash */
   FLASH_K_L_M_N    : origin = 0x0B8000, length = 0x007FF0	/* on-chip Flash */
   //FLASHK         : origin = 0x0B8000, length = 0x002000	/* on-chip Flash */
   //FLASHL         : origin = 0x0BA000, length = 0x002000	/* on-chip Flash */
   //FLASHM         : origin = 0x0BC000, length = 0x002000	/* on-chip Flash */
   //FLASHN         : origin = 0x0BE000, length = 0x001FF0	/* on-chip Flash */
   //FLASHN_RSVD    : origin = 0x0BFFF0, length = 0x000010  /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

PAGE 1 : /* Data Memory */
         /* Memory (RAM/FLASH) blocks can be moved to PAGE0 for program allocation */

   BOOT_RSVD       : origin = 0x000002, length = 0x000121     /* Part of M0, BOOT rom will use this for stack */

   RAM_STACK       : origin = 0x000123, length = 0x0006D5
   //RAMM0         : origin = 0x000122, length = 0x0002DE
   //RAMM1         : origin = 0x000400, length = 0x0003F8
   //RAMM1_RSVD    : origin = 0x0007F8, length = 0x000008     /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
   //RAMD0         : origin = 0x00B000, length = 0x000800
   //RAMD1         : origin = 0x00B800, length = 0x000800
   //RAMLS0        : origin = 0x008000, length = 0x000800
   //RAMLS1        : origin = 0x008800, length = 0x000800
   RAMLS_CLA1_DATA : origin = 0x009000, length = 0x001800
   //RAMLS2          : origin = 0x009000, length = 0x000800
   //RAMLS3        : origin = 0x009800, length = 0x000800
   //RAMLS4        : origin = 0x00A000, length = 0x000800
   RAMLS5        : origin = 0x00A800, length = 0x000800

   RAMGS_DATA_CPU1 : origin = 0x00C000, length = 0x005000
   //RAMGS0        : origin = 0x00C000, length = 0x001000
   //RAMGS1        : origin = 0x00D000, length = 0x001000
   //RAMGS2        : origin = 0x00E000, length = 0x001000
   //RAMGS3        : origin = 0x00F000, length = 0x001000
   //RAMGS4        : origin = 0x010000, length = 0x001000
   RAMGS_DATA_CPU2 : origin = 0x011000, length = 0x001000
   //RAMGS5        : origin = 0x011000, length = 0x001000
   RAMGS_CPU2      : origin = 0x012000, length = 0x005FF8
   //RAMGS6        : origin = 0x012000, length = 0x001000
   //RAMGS7        : origin = 0x013000, length = 0x001000
   //RAMGS8        : origin = 0x014000, length = 0x001000
   //RAMGS9        : origin = 0x015000, length = 0x001000
   //RAMGS10       : origin = 0x016000, length = 0x001000
   //RAMGS11       : origin = 0x017000, length = 0x000FF8  /* Uncomment for F28374D, F28376D devices */
   RAMGS11_RSVD    : origin = 0x017FF8, length = 0x000008  /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */
   RAMGS_CPU1      : origin = 0x018000, length = 0x003ff8
   //RAMGS12       : origin = 0x018000, length = 0x001000  /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   //RAMGS13       : origin = 0x019000, length = 0x001000  /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   //RAMGS14       : origin = 0x01A000, length = 0x001000  /* Only Available on F28379D/_, F28377D/F28377S, F28375D/F28375S devices. Remove line on other devices. */
   //RAMGS15       : origin = 0x01B000, length = 0x000FF8  /* Only Available on F28379D/_, F28377D/F28377S, F28375D/F28375S devices. Remove line on other devices. */
   RAMGS15_RSVD    : origin = 0x01BFF8, length = 0x000008  /* Reserve and do not use for code as per the errata advisory "Memory: Prefetching Beyond Valid Memory" */

   CLA1_MSGRAMLOW  : origin = 0x001480, length = 0x000080
   CLA1_MSGRAMHIGH : origin = 0x001500, length = 0x000080

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   // far memory
   //SDRAM    : origin = 0x80000000, length = 0x04000000   /* Total SDRAM memory: 64 MB */
   SDRAM_CPU1    : origin = 0x80000000, length = 0x02000000  /* CPU1: 32MB SDRAM */
   SDRAM_CPU2    : origin = 0x82000000, length = 0x02000000  /* CPU1: 32MB SDRAM */
}

SECTIONS
{
//// CPU  INIT ////

   	codestart			:> BEGIN,								PAGE = 0, ALIGN(8)
   	.text               :> FLASH_TEXT2,    						PAGE = 0, ALIGN(8)
   	.cinit              :> FLASH_CPU2,     						PAGE = 0, ALIGN(8)
   	.pinit              :> FLASH_CPU2,     						PAGE = 0, ALIGN(8)


   	/* Allocate uninitalized data sections: */
   	.stack              :> RAM_STACK,        					PAGE = 1
   	.ebss               :> RAMGS_CPU2,       					PAGE = 1
   	.esysmem            :> RAMGS_CPU2,       					PAGE = 1

   	/* Initalized sections go in Flash */
   	.econst             :> FLASH_CPU2,  	    				PAGE = 0, ALIGN(8)
   	.switch             :> FLASH_CPU2,   	   					PAGE = 0, ALIGN(8)
   	.reset              :> RESET,     							PAGE = 0, TYPE = DSECT /* not used, */


 	app_data_cpu1		:> RAMGS_DATA_CPU1, 					PAGE = 1
 	app_data_cpu2		:> RAMGS_DATA_CPU2, 					PAGE = 1
 	.data               :> RAMGS_CPU2, 							PAGE = 1

       /* Far memory sections */
    .farbss             :> SDRAM_CPU2,                          PAGE = 1, ALIGN(4)  /* Far memory variables */
    .farconst           :> SDRAM_CPU2,                          PAGE = 1, ALIGN(4)  /* Far memory constants */


	//https://e2e.ti.com/support/microcontrollers/c2000-microcontrollers-group/c2000/f/c2000-microcontrollers-forum/932465/compiler-tms320f28377d-is-it-necessary-to-rename-section-ramfuncs-into-ti-ramfunc-after-compiler-upgrade
    ramfuncs :   		fill = 0x6666 //fill value for holes
                     	 {
                       		.+=0x4;  // empty space at head of section
                       		*(ramfuncs)
                       		*(.TI.ramfunc)
                      	 }
                      	 LOAD = FLASHG,
                         RUN = RAMD0_1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(8)

   	/* The following section definitions are required when using the IPC API Drivers */
    GROUP :> CPU1TOCPU2RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

  	GROUP :> CPU2TOCPU1RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    MSGRAM_CPU1_TO_CPU2 : > CPU1TOCPU2RAM, type=NOINIT
    MSGRAM_CPU2_TO_CPU1 : > CPU2TOCPU1RAM, type=NOINIT


//// CPU END ////

//// CLA INIT  ////

   Cla1Prog         :  fill = 0x5555 //fill value for holes
                       {
                         .+=0x4;  // empty space at head of section
                         *(Cla1Prog)
                       }
                       LOAD = FLASHH,
                       RUN = RAMLS_CLA1_PROG,
                       RUN_START(_Cla1ProgRunStart),
                       LOAD_START(_Cla1ProgLoadStart),
                       LOAD_SIZE(_Cla1ProgLoadSize),
                       PAGE = 0, ALIGN(8)


   /* CLA C compiler and test sections */
   //
   // Must be allocated to memory the CLA has write access to
   //
   .scratchpad		:> RAMLS_CLA1_DATA,								PAGE = 1
   .bss_cla		    :> RAMLS_CLA1_DATA,								PAGE = 1
   .const_cla	    :  LOAD = FLASHI,
                       RUN  = RAMLS_CLA1_DATA,
                       RUN_START(_Cla1ConstRunStart),
                       LOAD_START(_Cla1ConstLoadStart),
                       LOAD_SIZE(_Cla1ConstLoadSize),
                       PAGE = 1, ALIGN(8)

   Cla1ToCpuMsgRAM  		:> CLA1_MSGRAMLOW,   				PAGE = 1
   CpuToCla1MsgRAM  		:> CLA1_MSGRAMHIGH,  				PAGE = 1

//// CLA END ////
}





