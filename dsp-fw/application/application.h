#ifndef APPLICATON_H_
#define APPLICATON_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <file.h>
#include <ipc.h>

#include "types.h"
#include "my_time.h"

#include "frequency.h"

#include "generic_definitions.h"
#include "fw_info.h"
#include "status.h"
#include "identification.h"

#include "ada4254.h"
#include "buzzer.h"


#define IPC_CMD_SET_CAL             0x10
#define IPC_CMD_SET_CURRENT_RANGE   0x11
#define IPC_CMD_START_TEST          0x12
#define IPC_CMD_RESET_TEST          0x13
#define IPC_CMD_RESET_METRICS       0x14


/************************
 * Hardware Definitions *
 ************************/


#define ADA4254_A1  1
#define ADA4254_A2  2
#define ADA4254_A3  3
#define ADA4254_A4  4
#define ADA4254_A5  5
#define ADA4254_A6  6
#define ADA4254_A7  7
#define ADA4254_A8  8

#define ADS178_CH1  1
#define ADS178_CH2  2
#define ADS178_CH3  3
#define ADS178_CH4  4
#define ADS178_CH5  5
#define ADS178_CH6  6
#define ADS178_CH7  7
#define ADS178_CH8  8


#define SYNC_FLAG IPC_FLAG31


#define MY_ADDR_PIN0                108
#define MY_ADDR_PIN0_CONFIG         GPIO_108_GPIO108
#define MY_ADDR_PIN1                109
#define MY_ADDR_PIN1_CONFIG         GPIO_109_GPIO109
#define MY_ADDR_PIN2                110
#define MY_ADDR_PIN2_CONFIG         GPIO_110_GPIO110

// DEBUG
#define GPIO_DEBUG_0                4
#define GPIO_DEBUG_0_CONFIG         GPIO_4_GPIO4
#define GPIO_DEBUG_1                15
#define GPIO_DEBUG_1_CONFIG         GPIO_15_GPIO15
#define GPIO_DEBUG_2                27
#define GPIO_DEBUG_2_CONFIG         GPIO_27_GPIO27

//CLK GEN
//CLK_GEN power enable
#define CLKGEN_ON                  98
#define CLKGEN_ON_CONFIG           GPIO_98_GPIO98
//ADS clock selection input for Si5351C 0:xtal 1:ocxo
#define CLKGEN_ADS_SEL_IN          167
#define CLKGEN_ADS_SEL_IN_CONFIG   GPIO_167_GPIO167
//ADS clock selection outuput from 0:Si5341B 1:Si5351C
#define CLKGEN_ADS_SEL1_OUT        107
#define CLKGEN_ADS_SEL1_OUT_CONFIG GPIO_107_GPIO107
//ADS spi clock selection outuput from 0:Si5341B 1:Si5351C
#define CLKGEN_ADS_SEL2_OUT        107
#define CLKGEN_ADS_SEL2_OUT_CONFIG GPIO_107_GPIO107
//ADS spi clock outuput from Si5351C 0:disable 1:enable
#define CLKGEN_BSP_ENA              9
#define CLKGEN_BSP_ENA_CONFIG       GPIO_9_GPIO9

//Si5341B
//CLK2_RST Si5341B
#define Si5341B_RST                128
#define Si5341B_RST_CONFIG         GPIO_128_GPIO128
//CLK2_OE Si5341B
#define Si5341B_OE                 129
#define Si5341B_OE_CONFIG          GPIO_129_GPIO129


//I2C
//I2CA -> clock to ADS1278
#define SDA_ADS                     104
#define SDA_ADS_CONFIG              GPIO_104_SDAA
#define SCL_ADS                     105
#define SCL_ADS_CONFIG              GPIO_105_SCLA
//I2Cb -> clock to Generators
#define SDA_GEN                     34
#define SDA_GEN_CONFIG              GPIO_34_SDAB
#define SCL_GEN                     35
#define SCL_GEN_CONFIG              GPIO_35_SCLB


//CLI SERIAL PORT
#define CLI_SERIALPORT SCID_BASE
#define CLI_SCITX_GPIO 142
#define CLI_SCIRX_GPIO 141
#define CLI_SCIRX_GPIO_PIN_CONFIG GPIO_141_SCIRXDD
#define CLI_SCITX_GPIO_PIN_CONFIG GPIO_142_SCITXDD

//COMM SERIAL PORT
#define COMMS_SCI_BASE SCIC_BASE
#define COMMS_SCITX_GPIO 140
#define COMMS_SCIRX_GPIO 139
#define COMMS_SCIRX_GPIO_PIN_CONFIG GPIO_139_SCIRXDC
#define COMMS_SCITX_GPIO_PIN_CONFIG GPIO_140_SCITXDC


// ADS1278
#define GPIO_ADC_TEST0              154
#define GPIO_ADC_TEST0_PIN_CONFIG   GPIO_154_GPIO154
#define GPIO_ADC_TEST1              153
#define GPIO_ADC_TEST1_PIN_CONFIG   GPIO_153_GPIO153
#define GPIO_ADC_CLKDIV             147
#define GPIO_ADC_CLKDIV_PIN_CONFIG  GPIO_147_GPIO147
#define GPIO_ADC_NSYNC              146
#define GPIO_ADC_NSYNC_PIN_CONFIG   GPIO_145_GPIO145
#define GPIO_ADC_CLK_SEL            145
#define GPIO_ADC_CLK_SEL_PIN_CONFIG GPIO_145_GPIO145
#define GPIO_ADC_MODE0              148
#define GPIO_ADC_MODE0_PIN_CONFIG   GPIO_148_GPIO148
#define GPIO_ADC_MODE1              149
#define GPIO_ADC_MODE1_PIN_CONFIG   GPIO_149_GPIO149
#define GPIO_ADC_FORMAT0            150
#define GPIO_ADC_FORMAT0_PIN_CONFIG GPIO_150_GPIO150
#define GPIO_ADC_FORMAT1            151
#define GPIO_ADC_FORMAT1_PIN_CONFIG GPIO_151_GPIO151
#define GPIO_ADC_FORMAT2            152
#define GPIO_ADC_FORMAT2_PIN_CONFIG GPIO_152_GPIO152
#define GPIO_ADS_SYNC               146
#define GPIO_ADS_SYNC_CONFIG        GPIO_146_GPIO146

// ADA
#define GPIO_PIN_SPIA_MOSI          16
#define SPIA_MOSI_GPIO              16
#define SPIA_MOSI_PIN_CONFIG        GPIO_16_SPISIMOA
#define GPIO_PIN_SPIA_MISO          17
#define SPIA_MISO_GPIO              17
#define SPIA_MISO_PIN_CONFIG        GPIO_17_SPISOMIA
#define GPIO_PIN_SPIA_CLK           18
#define SPIA_CLK_GPIO               18
#define SPIA_CLK_PIN_CONFIG         GPIO_18_SPICLKA


#define GPIO_PIN_CS_A1             161
#define GPIO_PIN_CS_A1_PIN_CONFIG  GPIO_161_GPIO161
#define GPIO_PIN_CS_A2             162
#define GPIO_PIN_CS_A2_PIN_CONFIG  GPIO_162_GPIO162
#define GPIO_PIN_CS_A3             163
#define GPIO_PIN_CS_A3_PIN_CONFIG  GPIO_163_GPIO163
#define GPIO_PIN_CS_A4             164
#define GPIO_PIN_CS_A4_PIN_CONFIG  GPIO_164_GPIO164
#define GPIO_PIN_CS_A5             165
#define GPIO_PIN_CS_A5_PIN_CONFIG  GPIO_165_GPIO165
#define GPIO_PIN_CS_A6             166
#define GPIO_PIN_CS_A6_PIN_CONFIG  GPIO_166_GPIO166
#define GPIO_PIN_CS_A7             122
#define GPIO_PIN_CS_A7_PIN_CONFIG  GPIO_122_GPIO122
#define GPIO_PIN_CS_A8             158
#define GPIO_PIN_CS_A8_PIN_CONFIG  GPIO_158_GPIO158

//PULSE IN
#define PULSE_IN_GPIO_PIN          6U
#define PULSE_IN_GPIO_CFG          GPIO_6_GPIO6

//PULSE OUT
#define PULSE_OUT_GPIO_PIN         10U
#define PULSE_OUT_GPIO_CFG         GPIO_10_GPIO10


typedef enum {
    APP_STATE_IDLE = 0,
    APP_STATE_START,
    APP_STATE_RUNNING,
    APP_STATE_ERROR,
} app_state_t;

typedef enum aqc_buff_en
{
    BUFF_A      = 0,
    BUFF_B      = 1
} aqc_buff_t;


typedef enum phase_desc_enum
{
    NEUTRAL = 0,
    PHASE_A = 1,
    PHASE_B = 2,
    PHASE_C = 3,
}phase_desc_t;



#define ABI_MAGIC      0xDEADCA75       // Fixed value for all
#define ABI_VERSION    (0x00010000u)    /*  v1.0 */

typedef struct {
    uint32_t magic;      // ID unic
    uint32_t version;    // ABI Version
    uint32_t size;       // Total app size
    uint32_t status;     // TODO: use this
    uint8_t  reserved[8];// Reserved for expansion
} abi_t;

typedef struct {
    app_state_t cur;
    app_state_t prev;
    uint32_t    enter_count[4];   // IDLE/START/RUNNING/ERROR
    uint32_t    last_change_ms;
} app_sm_t;


typedef struct application_st
{
    abi_t abi;
    identification_t id;
    equipment_t equipment;
    app_sm_t        sm_cpu1;
    app_sm_t        *sm_cpu2;
} application_t;


extern const fw_data_t fw_info;
extern application_t app;

#ifdef CPU1

extern bool flag_new_range;
extern int8_t new_range;

extern bool flag_new_calibration;
extern int8_t new_cal_parameter;
extern int8_t new_cal_phase;
extern int8_t new_cal_index;

extern bool flag_start_test;
extern bool flag_reset_test;
extern bool flag_reset_metrics;

#endif

generic_status_t app_init(application_t * this_app);
generic_status_t app_run(application_t * this_app);

typedef void (*state_handler_t)(application_t *app, my_time_t now);

static inline void app_sm_set(app_sm_t *sm, app_state_t s, my_time_t now) {
    if (sm->cur != s) {
        sm->prev = sm->cur;
        sm->cur  = s;
        sm->enter_count[(int)s]++;
        sm->last_change_ms = (uint32_t)now;


    }
}




#endif
