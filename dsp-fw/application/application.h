#ifndef APPLICATON_H_
#define APPLICATON_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <file.h>

#include "types.h"
#include "my_time.h"

#include "frequency.h"

#include "generic_definitions.h"
#include "fw_info.h"
#include "status.h"
#include "identification.h"

#include "ada4254.h"
#include "buzzer.h"

#include "reference_generation.h"



/* Generic All Devices Defines */
#define ADDRESS_STD         0
#define ADDRESS_PHASE_A     1
#define ADDRESS_PHASE_B     2
#define ADDRESS_PHASE_C     3
#define ADDRESS_UNDEFINED   7

#define IPC_CMD_10          0x10
#define IPC_CMD_11          0x11
#define IPC_CMD_12          0x12
#define IPC_CMD_13          0x13
#define IPC_CMD_14          0x14

#define SYNC_FLAG   IPC_FLAG31

//ADDR IN MIDPLANE
#define MY_ADDR_PIN0                    108
#define MY_ADDR_PIN0_CONFIG             GPIO_108_GPIO108
#define MY_ADDR_PIN1                    109
#define MY_ADDR_PIN1_CONFIG             GPIO_109_GPIO109

//CLI SERIAL PORT
#define CLI_SERIALPORT                  SCID_BASE
#define CLI_SCITX_GPIO                  142
#define CLI_SCIRX_GPIO                  141
#define CLI_SCIRX_GPIO_PIN_CONFIG       GPIO_141_SCIRXDD
#define CLI_SCITX_GPIO_PIN_CONFIG       GPIO_142_SCITXDD

//COMM SERIAL PORT
#define COMMS_SCI_BASE                  SCIC_BASE
#define COMMS_SCITX_GPIO                140
#define COMMS_SCIRX_GPIO                139
#define COMMS_SCIRX_GPIO_PIN_CONFIG     GPIO_139_SCIRXDC
#define COMMS_SCITX_GPIO_PIN_CONFIG     GPIO_140_SCITXDC

//SPI
#define SPIA_MOSI_GPIO                  16
#define SPIA_MOSI_PIN_CONFIG            GPIO_16_SPISIMOA
#define SPIA_MISO_GPIO                  17
#define SPIA_MISO_PIN_CONFIG            GPIO_17_SPISOMIA
#define SPIA_CLK_GPIO                   18
#define SPIA_CLK_PIN_CONFIG             GPIO_18_SPICLKA

#define SPIC_MOSI_PIN                   100
#define SPIC_MOSI_PIN_CONFIG            GPIO_100_SPISIMOC
#define SPIC_MISO_PIN                   101
#define SPIC_MISO_PIN_CONFIG            GPIO_101_SPISOMIC
#define SPIC_CLK_PIN                    102
#define SPIC_CLK_PIN_CONFIG             GPIO_102_SPICLKC

//I2C
#define SDAB_PIN                        34
#define SDAB_PIN_CONFIG                 GPIO_34_SDAB
#define SCLB_PIN                        35
#define SCLB_PIN_CONFIG                 GPIO_35_SCLB

#define SDAA_PIN                        104
#define SDAA_PIN_CONFIG                 GPIO_104_SDAA
#define SCLA_PIN                        105
#define SCLA_PIN_CONFIG                 GPIO_105_SCLA

//CAN
#define CAN_RS                          121
#define CAN_RS_PIN_CONFIG               GPIO_121_GPIO121
#define CAN_EN                          33
#define CAN_EN_PIN_CONFIG               GPIO_33_GPIO33
#define CAN_TXA                         37
#define CAN_TXA_PIN_CONFIG              GPIO_37_CANTXA
#define CAN_RXA                         36
#define CAN_RXA_PIN_CONFIG              GPIO_36_CANRXA




/* Device Related Defines */


/* Power Stage Related Values*/
#define INV_PWM_SWITCHING_FREQUENCY ((float)300000.0)

#define INV_DEADBAND_NS 600
#define INV_PWM_PERIOD ((PWMSYSCLOCK_FREQ)/(INV_PWM_SWITCHING_FREQUENCY))/2

#define INV_DEADBAND_PWM_COUNT (int)((float)INV_DEADBAND_NS*(float)PWMSYSCLOCK_FREQ*1e-9)

#define PWM_CH              9   // # of PWM channels + 1 -*ePWM[0] is defined as dummy value not used


#define VOLTAGE_MAX 500.0f*1.4142f  //Vrms
#define CURRENT_MAX 35.0f*1.4142f   //Irms
#define VOLTAGE_MAX 500.0f*1.4142f  //Vrms
#define FREQUENCY_MAX 70.0f
#define FREQUENCY_MIN 40.0f

//USER AND DEBUG PINS
#define USR_GPIO0                       43
#define USR_GPIO0_PIN_CONFIG            GPIO_43_GPIO43
#define USR_GPIO1                       42
#define USR_GPIO1_PIN_CONFIG            GPIO_42_GPIO42
#define USR_GPIO2                       94
#define USR_GPIO2_PIN_CONFIG            GPIO_94_GPIO94
#define USR_GPIO3                       144
#define USR_GPIO3_PIN_CONFIG            GPIO_144_GPIO144

//BRIDGE
#define INV_PWM1_VOLTAGE                   EPWM2_BASE
#define INV_PWM1_VOLTAGE_H_PIN             2
#define INV_PWM1_VOLTAGE_H_PIN_CONFIG      GPIO_2_EPWM2A
#define INV_PWM1_VOLTAGE_L_PIN             3
#define INV_PWM1_VOLTAGE_L_PIN_CONFIG      GPIO_3_EPWM2B

#define INV_PWM2_VOLTAGE                   EPWM1_BASE
#define INV_PWM2_VOLTAGE_H_PIN             0
#define INV_PWM2_VOLTAGE_H_PIN_CONFIG      GPIO_0_EPWM1A
#define INV_PWM2_VOLTAGE_L_PIN             1
#define INV_PWM2_VOLTAGE_L_PIN_CONFIG      GPIO_1_EPWM1B

#define INV_PWM1_CURRENT                   EPWM5_BASE
#define INV_PWM1_CURRENT_H_PIN             8
#define INV_PWM1_CURRENT_H_PIN_CONFIG      GPIO_8_EPWM5A
#define INV_PWM1_CURRENT_L_PIN             9
#define INV_PWM1_CURRENT_L_PIN_CONFIG      GPIO_9_EPWM5B

#define INV_PWM2_CURRENT                   EPWM6_BASE
#define INV_PWM2_CURRENT_H_PIN             10
#define INV_PWM2_CURRENT_H_PIN_CONFIG      GPIO_10_EPWM6A
#define INV_PWM2_CURRENT_L_PIN             11
#define INV_PWM2_CURRENT_L_PIN_CONFIG      GPIO_11_EPWM6B

#define BRIDGE_V_EN_PIN                 112
#define BRIDGE_V_EN_PIN_CONFIG          GPIO_112_GPIO112
#define BRIDGE_I_EN_PIN                 113
#define BRIDGE_I_EN_PIN_CONFIG          GPIO_113_GPIO113


//HV BUS METER
#define HVBUS_MEAS_ENA_PIN              114
#define HVBUS_MEAS_ENA_PIN_CONFIG       GPIO_114_GPIO114
#define HVBUS_CTRL_OWNER_PIN            111
#define HVBUS_CTRL_OWNER_PIN_CONFIG     GPIO_111_GPIO111
#define HVBUS_CTRL_PIN                  115
#define HVBUS_CTRL_PIN_CONFIG           GPIO_115_GPIO115

#define HVBUS_IFAULT_P_PIN              12
#define HVBUS_IFAULT_P_PIN_CONFIG       GPIO_12_GPIO12
#define HVBUS_IFAULT_N_PIN              13
#define HVBUS_IFAULT_N_PIN_CONFIG       GPIO_13_GPIO13
#define HVBUS_ZC_P_PIN                  110
#define HVBUS_ZC_P_PIN_CONFIG           GPIO_110_GPIO110
#define HVBUS_ZC_N_PIN                  143
#define HVBUS_ZC_N_PIN_CONFIG           GPIO_143_GPIO143


//SDFM MULTIPLIER
#define SDFM_CLK_HRPWM_BASE             EPWM5_BASE
#define SDFM_CLK_HRPWM_PIN              153
#define SDFM_CLK_HRPWM_PIN_CONFIG       GPIO_153_EPWM5A

// SDFM Secondaries
#define SDFM_D_IV1_PIN                  122
#define SDFM_D_IV1_PIN_CONFIG           GPIO_122_SD1_D1
#define SDFM_C_IV1_PIN                  123
#define SDFM_C_IV1_PIN_CONFIG           GPIO_123_SD1_C1

#define SDFM_D_II1_PIN                  124
#define SDFM_D_II1_PIN_CONFIG           GPIO_124_SD1_D2
#define SDFM_C_II1_PIN                  125
#define SDFM_C_II1_PIN_CONFIG           GPIO_125_SD1_C2

#define SDFM_D_IV2_PIN                  130
#define SDFM_D_IV2_PIN_CONFIG           GPIO_130_SD2_D1
#define SDFM_C_IV2_PIN                  131
#define SDFM_C_IV2_PIN_CONFIG           GPIO_131_SD2_C1

#define SDFM_D_II2_PIN                  132
#define SDFM_D_II2_PIN_CONFIG           GPIO_132_SD2_D2
#define SDFM_C_II2_PIN                  133
#define SDFM_C_II2_PIN_CONFIG           GPIO_133_SD2_C2

#define SDFM_D_VV1_PIN                  126
#define SDFM_D_VV1_PIN_CONFIG           GPIO_126_SD1_D3
#define SDFM_C_VV1_PIN                  127
#define SDFM_C_VV1_PIN_CONFIG           GPIO_127_SD1_C3

#define SDFM_D_VI1_PIN                  128
#define SDFM_D_VI1_PIN_CONFIG           GPIO_128_SD1_D4
#define SDFM_C_VI1_PIN                  129
#define SDFM_C_VI1_PIN_CONFIG           GPIO_129_SD1_C4

#define SDFM_D_VV2_PIN                  134
#define SDFM_D_VV2_PIN_CONFIG           GPIO_134_SD2_D3
#define SDFM_C_VV2_PIN                  135
#define SDFM_C_VV2_PIN_CONFIG           GPIO_135_SD2_C3

#define SDFM_D_VI2_PIN                  136
#define SDFM_D_VI2_PIN_CONFIG           GPIO_136_SD2_D4
#define SDFM_C_VI2_PIN                  137
#define SDFM_C_VI2_PIN_CONFIG           GPIO_137_SD2_C4


//#define SDFM_CLK_MULT_PIN             10
//#define SDFM_CLK_MULT_PIN_CONFIG      GPIO_10_GPIO10
#define CLK_AUX_OE_PIN                  107
#define CLK_AUX_OE_PIN_CONFIG           GPIO_107_GPIO107
#define SDFM_CLK_SEL_PIN                96
#define SDFM_CLK_SEL_PIN_CONFIG         GPIO_96_GPIO96


// ADA
#define CS_VV2_PIN                   160
#define CS_VV2_PIN_CONFIG            GPIO_160_GPIO160
#define CS_VI2_PIN                   159
#define CS_VI2_PIN_CONFIG            GPIO_159_GPIO159
#define CS_IV2_PIN                   152
#define CS_IV2_PIN_CONFIG            GPIO_152_GPIO152
#define CS_II2_PIN                   151
#define CS_II2_PIN_CONFIG            GPIO_151_GPIO151
#define CS_VV1_PIN                   148
#define CS_VV1_PIN_CONFIG            GPIO_148_GPIO148
#define CS_VI1_PIN                   147
#define CS_VI1_PIN_CONFIG            GPIO_147_GPIO147
#define CS_IV1_PIN                   150
#define CS_IV1_PIN_CONFIG            GPIO_150_GPIO150
#define CS_II1_PIN                   149
#define CS_II1_PIN_CONFIG            GPIO_149_GPIO149

//FB
#define FB_EN_PIN                       167                 // FB_V_EN and FB_I_EN merged
#define FB_EN_PIN_CONFIG                GPIO_167_GPIO167

#define FB_CLK_AUX_OE                   107
#define FB_CLK_AUX_OE_PIN_CONFIG        GPIO_107_GPIO107
#define FB_CLK_AUX_FS1                  99
#define FB_CLK_AUX_FS1_PIN_CONFIG       GPIO_99_GPIO99
#define FB_CLK_AUX_FS2                  116
#define FB_CLK_AUX_FS2_PIN_CONFIG       GPIO_116_GPIO116

#define FB_CLK_ERR                      97
#define FB_CLK_ERR_PIN_CONFIG           GPIO_97_GPIO97


typedef enum ada4254_channel_st
{
    ADA4254_VV1      =      0u,
    ADA4254_VI1      =      1u,
    ADA4254_IV1      =      2u,
    ADA4254_II1      =      3u,
    ADA4254_VV2      =      4u,
    ADA4254_VI2      =      5u,
    ADA4254_IV2      =      6u,
    ADA4254_II2      =      7u
} ada4254_channel_t;

#define ADDRESS_STD         0
#define ADDRESS_PHASE_A     1
#define ADDRESS_PHASE_B     2
#define ADDRESS_PHASE_C     3
#define ADDRESS_UNDEFINED   7



#define VOLTAGE_MAX 500.0f*1.4142f  //Vrms
#define CURRENT_MAX 35.0f*1.4142f   //Irms
#define VOLTAGE_MAX 500.0f*1.4142f  //Vrms
#define FREQUENCY_MAX 70.0f
#define FREQUENCY_MIN 40.0f

#define CALIBRATION_CURRENT_GAINS           (6)
#define CONFIG__SERIAL_NUMBER_SIZE          (16)

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


typedef struct feedback_st
{

        ada4254_t *voltage_gain, *current_gain;

        float32_t voltage_rms;
        float32_t voltage_avg;
        float32_t voltage_ppeak;
        float32_t voltage_npeak;

        float32_t current_rms;
        float32_t current_avg;
        float32_t current_ppeak;
        float32_t current_npeak;

        float32_t power_sum;
        float32_t power;

        float32_t impedance;

        float32_t setpoint_rms;
        float32_t ctrl_erro_rms;

        float32_t voltage[1000];
        float32_t current[1000];


} feedback_t;

typedef enum mode_en
{
    MODE_OFF                        = 0,
    MODE_NONE                       = 1,
    MODE_START                      = 2,
    MODE_FEEDFORWARD                = 3,
    MODE_SFRA                       = 4,
    MODE_REPETITIVE                 = 7,
    MODE_REPETITIVE_FROM_CLA        = 8,
} mode_t;

typedef enum generation_sm_type_en
{
    GENERATING_VOLTAGE     = 'V',
    GENERATING_CURRENT     = 'I'

} generation_sm_type_t;


typedef struct waveform_generation_st
{
    my_time_t scheduling;
    reference_generation_t ref;
    struct  {

        generic_states_t state;
        generic_status_t status;
    } sm;

    struct  {
        bool from_control_loop;
        bool start_generation;
    } trigger;

    struct  {
        generation_sm_type_t gen_type;
        float32_t scale;
        float32_t scale_requested;
    } config;

    struct  {
        bool enable;
        bool enable_from_cli;
        bool disable_from_cli;
        bool enable_from_comm;
        bool disable_from_comm;
        bool disable_from_protection_by_control_error;
        bool disable_from_protection_by_saturation;
    } command;

    struct  {
        bool ready_to_generate;
        bool generating;
    } status;

    mode_t controller;
    //rep_controller_t control;
    //protection_error_monitor_t protection;

} waveform_generation_t;


typedef struct generation_st
{
    struct
    {
        uint32_t inverter_pwm_steps;
        uint32_t deadband;
        uint32_t generation_freq;
    }config;

    waveform_generation_t voltage;
    waveform_generation_t current;

    bool sync_flag;
    uint16_t mep_status;
    bool zero_trigger;

} generation_t;

typedef struct analog_input_st
{
    ada4254_t ada_channel;
} analog_input_t;


typedef struct measures_st
{
    analog_input_t analog_input[8];
    struct {
        feedback_t voltage;
        feedback_t current;
    } primary;
    struct {
        feedback_t voltage;
        feedback_t current;
    } secondary;
} measures_t;



typedef struct application_st
{
    abi_t               abi;
    identification_t    id;
    generation_t        generation;
    measures_t          measures;
    app_sm_t            sm_cpu1;
    app_sm_t            *sm_cpu2;
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
