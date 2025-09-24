#ifndef APP_CPU1_H_
#define APP_CPU1_H_

#include "application.h"

#include "fpu_rfft.h"
//#include "filter.h"
#include "others.h"







// Set the ADC SCLK rate
// note: SCLK may be run as fast as the CLK frequency.
// note: For best performance, limit fSCLK/fCLK to ratios of 1, 1/2, 1/4, 1/8, etc.
//#define ADC_FREQ_SSI_DATA 10000000 // Can be Read by logic 8
#define ADC_FREQ_SSI_DATA 20000000
//#define ADS_FREQ 19E6 //maximum tested
#define ADS_FREQ 15728640 //512 samples per cycle in 60Hz generation

#define MCBSP_CYCLE_NOP0(n)  __asm(" RPT #(" #n ") || NOP")
#define MCBSP_CYCLE_NOP(n)   MCBSP_CYCLE_NOP0(n)



// Clock Input Selection (hardware jumper)
#define ADC_CLK_M1PWM5   0x00000000 // use M1PWM5 as the source of the clock
#define ADC_CLK_SSI2_CLK 0x00000001 // use SSI2_CLK as the source of the clock
// DAC Power Mode Control - Indicates which
#define ADC_PWUP1       0x00000001
#define ADC_PWUP2       0x00000002
#define ADC_PWUP3       0x00000004
#define ADC_PWUP4       0x00000008
#define ADC_PWUP5       0x00000010
#define ADC_PWUP6       0x00000020
#define ADC_PWUP7       0x00000040
#define ADC_PWUP8       0x00000080
#define ADC_PWUP_ALL    0x000000FF

/******************
 * ADC Parameters *
 ******************/
// Mode Selection
#define ADC_MODE_HIGH_SPEED  0x00000000 // 00 | MAX Fdata = 144,531
#define ADC_MODE_HIGH_RESOL  0x00000040 // 01 | MAX Fdata =  52,734
#define ADC_MODE_LOW_POWER   0x00000080 // 10 | MAX Fdata =  52,734
#define ADC_MODE_LOW_SPEED   0x000000C0 // 11 | MAX Fdata =  10,547
// CLK Frequency Settings
#define ADC_FREQ_37MHZ_HS     0x00000000 // Fclk = 37MHz,   Mode = high speed mode
#define ADC_FREQ_27MHZ_HR     0x00000001 // Fclk = 27MHz,   Mode = high resolution mode
#define ADC_FREQ_27MHZ_LP     0x00000002 // Fclk = 27MHz,   Mode = low power mode
#define ADC_FREQ_13p5MHZ_LP   0x00000004 // Fclk = 13.5MHz, Mode = low power mode
#define ADC_FREQ_27MHZ_LS     0x00000008 // Fclk = 27MHz,   Mode = low speed mode
#define ADC_FREQ_5p4MHZ_LS    0x00000010 // Fclk = 13.5MHz, Mode = low speed mode
// Test Mode
#define ADC_TEST_MODE_TEST    0x00000010 // 11 | Continuity test of the digital I/O pins
#define ADC_TEST_MODE_NORMAL 0x00000000 // 00 | Normal operation
// Clock Input Divider
#define ADC_CLKDIV_HIGHF      0x00000020 // 37MHz (High-Speed mode)/otherwise 27MHz
#define ADC_CLKDIV_LOWF       0x00000000 // 13.5MHz (low-power)/5.4MHz (low-speed)
// PWM frequency divider
#define ADC_PWM_DIV2          0x00000002 // PWM frequency = system clock /2
#define ADC_PWM_DIV3          0x00000003 // PWM frequency = system clock /3

                                             // Interface  |   Dout   | Data
                                             // Protocol   |   Mode   | Position
// Serial Input Format                       //----------------------------------
#define ADC_FORMAT_SPI_TDM_DYN    0x00000000 //    SPI     |    TDM   | Dynamic
#define ADC_FORMAT_SPI_TDM_FIX    0x00000001 //    SPI     |    TDM   | Fixed
#define ADC_FORMAT_SPI_DISCRETE   0x00000002 //    SPI     | Discrete |
#define ADC_FORMAT_FSYNC_TDM_DYN  0x00000003 // Frame Sync |    TDM   | Dynamic
#define ADC_FORMAT_FSYNC_TDM_FIX  0x00000004 // Frame Sync |    TDM   | Fixed
#define ADC_FORMAT_FSYNC_DISCRETE 0x00000005 // Frame Sync | Discrete |
#define ADC_FORMAT_MODULATOR       0x00000006 // Modulator Mode
// uDMA Functions
#define UDMA_CHANNEL_SSI2RX        12 // uDMA channel number for SSI2 RX channel
#define UDMA_DATA_BUFFER_BYTE2     0  // uDMA RX data buffer byte 2
#define UDMA_DATA_BUFFER_BYTE1     1
#define UDMA_DATA_BUFFER_BYTE0     2








void app_init_cpu1(application_t *app);
void app_run_cpu1(application_t *app);

#endif /* APP_CPU1_H_ */
