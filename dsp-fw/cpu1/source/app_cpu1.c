#include "app_cpu1.h"

#include "F28x_Project.h"
#include "F2837xD_sdfm_drivers.h"
#include "SFO_V8.h"
#include "my_time.h"
#include "gen_cla1_shared.h"
#include "ipc_simple.h"
#include "frequency.h"

#include "amplifier.h"
#include "generation.h"
#include "generation_sm.h"

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
void _cla1_memory_config(void);
void _cla1_link_from_sdfm(void);
void _sdfm_init(void);
void generation_init(application_t* app);

generic_status_t _ampops_init(application_t *app);

__interrupt void access_violation(void);
__interrupt void xint1_isr(void);
__interrupt void _sdfm_isr(void);

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
int c1,p1,l1;

int voltage_event = 0;
int current_event = 0;
int voltage_overpower_counter = 0;



// ---- VOLTAGE ----
static void voltage_pwm_on(void) {
    EPWM_clearTripZoneFlag(INV_PWM1_VOLTAGE, EPWM_TZ_FLAG_OST);
    EPWM_clearTripZoneFlag(INV_PWM2_VOLTAGE, EPWM_TZ_FLAG_OST);
    Cla1ForceTask3();
}

static void voltage_pwm_off(void) {
    EPWM_forceTripZoneEvent(INV_PWM1_VOLTAGE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(INV_PWM2_VOLTAGE, EPWM_TZ_FORCE_EVENT_OST);
    Cla1ForceTask4();
}

static void v_pwm_clear_trip(void) {
    EPWM_clearTripZoneFlag(INV_PWM1_VOLTAGE, EPWM_TZ_FLAG_OST);
    EPWM_clearTripZoneFlag(INV_PWM2_VOLTAGE, EPWM_TZ_FLAG_OST);
}

static void v_pwm_force_trip(void) {
    EPWM_forceTripZoneEvent(INV_PWM1_VOLTAGE, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(INV_PWM2_VOLTAGE, EPWM_TZ_FORCE_EVENT_OST);
}

static void v_cla_on(void)  { Cla1ForceTask3(); }

static void v_cla_off(void) { Cla1ForceTask4(); }


// ---- CURRENT ----
static void current_pwm_on(void) {
    EPWM_clearTripZoneFlag(INV_PWM1_CURRENT, EPWM_TZ_FLAG_OST);
    EPWM_clearTripZoneFlag(INV_PWM2_CURRENT, EPWM_TZ_FLAG_OST);
    Cla1ForceTask5();
}

static void current_pwm_off(void) {
    EPWM_forceTripZoneEvent(INV_PWM1_CURRENT, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(INV_PWM2_CURRENT, EPWM_TZ_FORCE_EVENT_OST);
    Cla1ForceTask6();
}

static void i_pwm_clear_trip(void) {
    EPWM_clearTripZoneFlag(INV_PWM1_CURRENT, EPWM_TZ_FLAG_OST);
    EPWM_clearTripZoneFlag(INV_PWM2_CURRENT, EPWM_TZ_FLAG_OST);
}

static void i_pwm_force_trip(void) {
    EPWM_forceTripZoneEvent(INV_PWM1_CURRENT, EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(INV_PWM2_CURRENT, EPWM_TZ_FORCE_EVENT_OST);
}

static void i_cla_on(void)  { Cla1ForceTask5(); }

static void i_cla_off(void) { Cla1ForceTask6(); }


static void voltage_setpoint_write(float val) {
    cla_voltage_setpoint = val;
}

static void current_setpoint_write(float val) {
    cla_current_setpoint = val;
}

void app_cpu1_init_generation(application_t *app)
{
    gen_sm_hw_cb_t vhw = { v_pwm_clear_trip, v_pwm_force_trip, v_cla_on, v_cla_off };
    gen_sm_hw_cb_t ihw = { i_pwm_clear_trip, i_pwm_force_trip, i_cla_on, i_cla_off };
    gen_sm_init(&app->generation.voltage, &app->voltage_wg, &vhw, 0.01f, 5);
    gen_sm_init(&app->generation.current, &app->current_wg, &ihw, 0.01f, 5);
}

void app_cpu1_generation_tick(my_time_t now_ms)
{
    gen_sm_tick(&app.generation.voltage, now_ms);
    gen_sm_tick(&app.generation.current, now_ms);
}

// IPC/CLI signals:
void cmd_set_scale_voltage(float32_t s) { gen_sm_request_scale(&app.generation.voltage, s); }
void cmd_set_scale_current(float32_t s) { gen_sm_request_scale(&app.generation.current, s); }

void cmd_enable_voltage(void) { gen_sm_request_enable(&app.generation.voltage); }
void cmd_disable_voltage(void){ gen_sm_request_disable(&app.generation.voltage); }

void cmd_enable_current(void) { gen_sm_request_enable(&app.generation.current); }
void cmd_disable_current(void){ gen_sm_request_disable(&app.generation.current); }

//todo:
// void on_protection_trip_voltage(void){ gen_sm_fault(&app.generation.voltage); }
// void on_protection_trip_current(void){ gen_sm_fault(&app.generation.current); }

void ipc_on_cmd_from_cpu2(uint8_t cmd, const uint8_t* payload, uint8_t len)
{
    ipc_log_event_from_cpu1(500, cmd, (uint16_t)(my_time(NULL)&0xFFFF));
}


static void handle_ipc_command(uint32_t cmd, float32_t arg)
{
    switch (cmd) {
    case 1:
        gen_ch_enable(&app.generation.voltage);
        gen_ch_enable(&app.generation.current);
        break;
    case 2:
        gen_ch_disable(&app.generation.voltage);
        gen_ch_disable(&app.generation.current);
        break;
    case 3:
        gen_ch_enable(&app.generation.voltage);
        break;
    case 4:
        gen_ch_disable(&app.generation.voltage);
        break;
    case 5:
        gen_ch_enable(&app.generation.current);
        break;
    case 6:
        gen_ch_disable(&app.generation.current);
        break;
    case 7: // set scale voltage
        //gen_ch_set_target(&app.generation.voltage, arg, app.generation.voltage.tgt.waveform_target);
        break;
    case 8: // set scale corrente
        //gen_ch_set_target(&app.generation.current, arg, app.generation.current.tgt.waveform_target);
        break;
    case 9: // trocar waveform dos dois (ex.: seno/quadrada etc.)
        //gen_ch_set_target(&app.generation.voltage, app.generation.voltage.tgt.scale_target, (uint8_t)arg);
        //gen_ch_set_target(&app.generation.current, app.generation.current.tgt.scale_target, (uint8_t)arg);
        break;
    default:
        break;
    }
}



static void cpu1_idle   (application_t *app, my_time_t now)
{
    app_sm_set(&app->sm_cpu1, APP_STATE_START, now);
}

static void cpu1_start  (application_t *app, my_time_t now)
{

    uint8_t address=0;
    app->sm_cpu1 = (app_sm_t){ .cur = APP_STATE_START };

    ipc_simple_init_cpu1();

    _app_gpio_init();


    if(GPIO_ReadPin(MY_ADDR_PIN0) == 1 )
        address|=0b001;
    if(GPIO_ReadPin(MY_ADDR_PIN1) == 1 )
        address|=0b010;

    app->id.data.full.my_address = address;

    memcpy(app->id.data.full.probewell_part_number, "9015-6200:GEN   ", 16);
    memcpy(app->id.data.full.serial_number,         "22531515        ", 16);
    memcpy(app->id.data.full.fabrication_date,      "20260101", 8);
    memcpy(app->id.data.full.last_verfication_date, "20260101", 8);


    Cla1ForceTask8andWait();

    WAITSTEP;

    GPIO_writePin(FB_EN_PIN, 1);
    _cla1_link_from_sdfm();
    _cla1_init();
    _cla1_memory_config();
    _cla1_interruption_config();
    Cla1ForceTask8andWait();

    _ampops_init(app);

    _sdfm_init();

    // Enable global Interrupts and higher priority real-time debug events:
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM

    GPIO_writePin(SDFM_CLK_SEL_PIN,0);                              // SDFM_CLK_SEL: 0 from ext STD; 1 from int SI5351
    GPIO_writePin(CLK_AUX_OE_PIN,1);                                // CLK_AUX_OE = 1 clk out enabled




    GPIO_writePin(BRIDGE_V_EN_PIN, 1);
    GPIO_writePin(BRIDGE_I_EN_PIN, 1);





    //    gen_sm_turning_on();
    //
    setupInverter(
            INV_PWM1_VOLTAGE,
            INV_PWM2_VOLTAGE,
            INV_PWM2_CURRENT,
            INV_PWM1_CURRENT,
            INV_PWM_PERIOD,
            INV_DEADBAND_PWM_COUNT
    );



    reference_init(&app->generation.voltage.wg->ref);
    reference_init(&app->generation.current.wg->ref);

    //    //PWM max = 90% PWM min = 10%
    //    init_protection(&app->generation.voltage.protection,
    //                    80, 200, 50.0, 10.0,
    //                    ((app->generation.config.inverter_pwm_steps)*90)/100,
    //                    ((app->generation.config.inverter_pwm_steps)*10)/100
    //    );
    //
    //    init_protection(&app->generation.current.protection,
    //                    60, 600, 90.0, 0.8,
    //                    ((app->generation.config.inverter_pwm_steps)*90)/100,
    //                    ((app->generation.config.inverter_pwm_steps)*10)/100
    //    );
    //
    //    app->generation.voltage.controller = MODE_NONE;
    //    app->generation.current.controller = MODE_NONE;
    //
    //    app->generation.voltage.config.scale_requested = 0.0;
    //    app->generation.current.config.scale_requested= 0.0;
    //
    //
    //    app->generation.current.controller = MODE_FEEDFORWARD;
    //    app->generation.voltage.controller = MODE_FEEDFORWARD;
    //
    //    app->generation.current.command.disable_from_cli = false;
    //    app->generation.current.command.enable_from_cli = false;
    //    app->generation.current.config.gen_type = GENERATING_CURRENT;
    //    app->generation.current.command.enable = false;
    //    app->generation.current.status.ready_to_generate = false;
    //    app->generation.current.status.generating = false;
    //    app->generation.current.trigger.from_control_loop = false;
    //    app->generation.current.sm.state = STATE_INIT;
    //
    //    app->generation.voltage.command.disable_from_cli = false;
    //    app->generation.voltage.command.enable_from_cli = false;
    //    app->generation.voltage.config.gen_type = GENERATING_VOLTAGE;
    //    app->generation.voltage.command.enable = false;
    //    app->generation.voltage.status.ready_to_generate = false;
    //    app->generation.voltage.status.generating = false;
    //    app->generation.voltage.trigger.from_control_loop = false;
    //    app->generation.voltage.sm.state = STATE_INIT;
    //
    //    app->generation.current.command.disable_from_cli = false;
    //    app->generation.current.command.disable_from_comm = false;
    //    app->generation.current.command.disable_from_protection_by_control_error = false;
    //    app->generation.current.command.disable_from_protection_by_saturation = false;
    //    app->generation.current.command.enable = false;
    //    app->generation.current.command.enable_from_cli = false;
    //    app->generation.current.command.disable_from_cli = false;
    //    app->generation.current.command.enable_from_comm = false;
    //
    //    app->generation.voltage.command.disable_from_cli = false;
    //    app->generation.voltage.command.disable_from_comm = false;
    //    app->generation.voltage.command.disable_from_protection_by_control_error = false;
    //    app->generation.voltage.command.disable_from_protection_by_saturation = false;
    //    app->generation.voltage.command.enable = false;
    //    app->generation.voltage.command.enable_from_cli = false;
    //    app->generation.voltage.command.disable_from_cli = false;
    //    app->generation.voltage.command.enable_from_comm = false;

    app->generation.sync_flag = false;


    app_sm_set(&app->sm_cpu1, APP_STATE_RUNNING, now);

    app_cpu1_init_generation(app);

}


bool flag_test=false;

bool flag_amp05 = false;
bool flag_amp1 = false;
bool flag_amp2 = false;

bool flag_gen1 = false;
bool flag_gen2 = false;
bool flag_gen3 = false;

float32_t new_scale = 1.0;


static void cpu1_running(application_t *app, my_time_t now)
{
    ipc_service_cpu1(); //try to send msg to cpu2
    ipc_rx_service_cpu1(); //treat incoming message from cpu2

    app_cpu1_generation_tick(now);

    amplifier_system_poll(now);

    if(flag_amp1){
        flag_amp1=false;
        amplifier_request_gain(AFE_VV1, IN_GAIN_1, OUT_GAIN_1);

    }
    if(flag_amp05){
        flag_amp05=false;
        amplifier_request_gain(AFE_VV1, IN_GAIN_0_5, OUT_GAIN_1);

    }
    if(flag_amp2){
        flag_amp2=false;
        amplifier_request_gain(AFE_VV1, IN_GAIN_2, OUT_GAIN_1);
    }

    if (flag_test)
    {
        ipc_buzzer_play(10);
        int16_t cmd = 0x12;
        ipc_log_event_from_cpu1(100/*EVT_IPC_TX 100*/, cmd, (now&0xFFFF));
        flag_test = false;
    }

    if(flag_gen1){
        gen_sm_request_enable(&app->generation.voltage);
        flag_gen1=false;
    }
    if(flag_gen2){
        gen_sm_request_disable(&app->generation.voltage);
        flag_gen2=false;
    }
    if(flag_gen3){
        gen_sm_request_scale(&app->generation.voltage, new_scale);
        flag_gen3=false;
    }



}

static void cpu1_error  (application_t *app, my_time_t now)
{
    // if error
}

static const state_handler_t CPU1_HANDLERS[] =
{
 [APP_STATE_IDLE]    = cpu1_idle,
 [APP_STATE_START]   = cpu1_start,
 [APP_STATE_RUNNING] = cpu1_running,
 [APP_STATE_ERROR]   = cpu1_error
};







void app_init_cpu1(application_t *app)
{
    app->sm_cpu1 = (app_sm_t){ .cur = APP_STATE_START };
}


void app_run_cpu1(application_t *app)
{
    my_time_t now = my_time(NULL);
    CPU1_HANDLERS[app->sm_cpu1.cur](app, now);
}


generic_status_t _ampops_init(application_t *app)
{
    generic_status_t status = STATUS_DONE;

    SPI_init(SPIC_BASE);
    amplifier_system_start(app);

    return status;
}


void _app_gpio_init(void)
{

    GPIO_setPadConfig(USR_GPIO0, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(USR_GPIO0, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(USR_GPIO0, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(USR_GPIO0_PIN_CONFIG);
    GPIO_setMasterCore(USR_GPIO0, GPIO_CORE_CPU1);

    GPIO_setPadConfig(USR_GPIO1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(USR_GPIO1, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(USR_GPIO1, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(USR_GPIO1_PIN_CONFIG);
    GPIO_setMasterCore(USR_GPIO1, GPIO_CORE_CPU1);

    GPIO_setPadConfig(USR_GPIO2, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(USR_GPIO2, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(USR_GPIO2, GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(USR_GPIO2_PIN_CONFIG);
    GPIO_setMasterCore(USR_GPIO2, GPIO_CORE_CPU1_CLA1);

    GPIO_WritePin(USR_GPIO0, 0);
    GPIO_WritePin(USR_GPIO1, 0);
    GPIO_WritePin(USR_GPIO2, 0);


    GPIO_setPadConfig(MY_ADDR_PIN0, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(MY_ADDR_PIN0, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(MY_ADDR_PIN0, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(MY_ADDR_PIN0_CONFIG);
    GPIO_setPadConfig(MY_ADDR_PIN1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(MY_ADDR_PIN1, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(MY_ADDR_PIN1, GPIO_QUAL_6SAMPLE);
    GPIO_setPinConfig(MY_ADDR_PIN1_CONFIG);




    // I2C
    {
        //I2CA - EEPROM and TMP75
        GPIO_setMasterCore(SDAB_PIN, GPIO_CORE_CPU1);
        GPIO_setPadConfig(SDAB_PIN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setDirectionMode(SDAB_PIN, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(SDAB_PIN,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SDAB_PIN_CONFIG);
        GPIO_setMasterCore(SCLB_PIN, GPIO_CORE_CPU1);
        GPIO_setPadConfig(SCLB_PIN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setDirectionMode(SCLB_PIN, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(SCLB_PIN,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SCLB_PIN_CONFIG);

        //I2CB - SSR
        GPIO_setMasterCore(SDAA_PIN, GPIO_CORE_CPU1);
        GPIO_setPadConfig(SDAA_PIN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setDirectionMode(SDAA_PIN, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(SDAA_PIN,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SDAA_PIN_CONFIG);
        GPIO_setMasterCore(SCLA_PIN, GPIO_CORE_CPU1);
        GPIO_setPadConfig(SCLA_PIN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setDirectionMode(SCLA_PIN, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(SCLA_PIN,GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SCLA_PIN_CONFIG);


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


        //SPIC
        GPIO_setPinConfig(SPIC_MOSI_PIN_CONFIG);
        GPIO_setPadConfig(SPIC_MOSI_PIN, GPIO_PIN_TYPE_STD);
        GPIO_setQualificationMode(SPIC_MOSI_PIN, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SPIC_MISO_PIN_CONFIG);
        GPIO_setPadConfig(SPIC_MISO_PIN, GPIO_PIN_TYPE_PULLUP);
        GPIO_setQualificationMode(SPIC_MISO_PIN, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(SPIC_CLK_PIN_CONFIG);
        GPIO_setPadConfig(SPIC_CLK_PIN, GPIO_PIN_TYPE_STD);
        GPIO_setQualificationMode(SPIC_CLK_PIN, GPIO_QUAL_ASYNC);
    }


    { //buzzer

        GPIO_setPadConfig(168, GPIO_PIN_TYPE_STD);
        GPIO_setDirectionMode(168, GPIO_DIR_MODE_OUT);
        GPIO_setQualificationMode(168, GPIO_QUAL_ASYNC);
        GPIO_setPinConfig(GPIO_168_EPWM12B);
        GPIO_setMasterCore(168, GPIO_CORE_CPU2);
    }

    //FB En
    GPIO_setMasterCore(FB_EN_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(FB_EN_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(FB_EN_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(FB_EN_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(FB_EN_PIN_CONFIG);
    //
    //    //SDFM_CLK_MULTIPLIER
    //    GPIO_setMasterCore(SDFM_CLK_MULT_PIN, GPIO_CORE_CPU1);
    //    GPIO_setPadConfig(SDFM_CLK_MULT_PIN, GPIO_PIN_TYPE_STD);
    //    GPIO_setDirectionMode(SDFM_CLK_MULT_PIN, GPIO_DIR_MODE_OUT);
    //    GPIO_setQualificationMode(SDFM_CLK_MULT_PIN,GPIO_QUAL_ASYNC);
    //    GPIO_setPinConfig(SDFM_CLK_MULT_PIN_CONFIG);

    GPIO_setMasterCore(CLK_AUX_OE_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(CLK_AUX_OE_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(CLK_AUX_OE_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CLK_AUX_OE_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CLK_AUX_OE_PIN_CONFIG);

    GPIO_setMasterCore(SDFM_CLK_SEL_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(SDFM_CLK_SEL_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(SDFM_CLK_SEL_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(SDFM_CLK_SEL_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(SDFM_CLK_SEL_PIN_CONFIG);


    //SPI ADA4254 CS
    GPIO_writePin(CS_VV1_PIN,1);
    GPIO_setDirectionMode(CS_VV1_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_VV1_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_VV1_PIN_CONFIG);

    GPIO_writePin(CS_VI1_PIN,1);
    GPIO_setDirectionMode(CS_VI1_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_VI1_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_VI1_PIN_CONFIG);

    GPIO_writePin(CS_IV1_PIN,1);
    GPIO_setDirectionMode(CS_IV1_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_IV1_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_IV1_PIN_CONFIG);

    GPIO_writePin(CS_II1_PIN,1);
    GPIO_setDirectionMode(CS_II1_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_II1_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_II1_PIN_CONFIG);

    GPIO_writePin(CS_VV2_PIN,1);
    GPIO_setDirectionMode(CS_VV2_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_VV2_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_VV2_PIN_CONFIG);

    GPIO_writePin(CS_VI2_PIN,1);
    GPIO_setDirectionMode(CS_VI2_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_VI2_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_VI2_PIN_CONFIG);

    GPIO_writePin(CS_IV2_PIN,1);
    GPIO_setDirectionMode(CS_IV2_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_IV2_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_IV2_PIN_CONFIG);

    GPIO_writePin(CS_II2_PIN,1);
    GPIO_setDirectionMode(CS_II2_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(CS_II2_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(CS_II2_PIN_CONFIG);

    //SDFM clk source
    GPIO_setDirectionMode(SDFM_CLK_HRPWM_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(SDFM_CLK_HRPWM_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPadConfig(SDFM_CLK_HRPWM_PIN, GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(SDFM_CLK_HRPWM_PIN_CONFIG);

    // Setup GPIO for SD
    GPIO_setPadConfig(SDFM_D_VV1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_VV1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_VV1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_VV1_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_VV1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_VV1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_VV1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_VV1_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_VI1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_VI1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_VI1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_VI1_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_VI1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_VI1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_VI1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_VI1_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_IV1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_IV1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_IV1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_IV1_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_IV1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_IV1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_IV1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_IV1_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_II1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_II1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_II1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_II1_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_II1_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_II1_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_II1_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_II1_PIN_CONFIG);


    GPIO_setPadConfig(SDFM_D_VV2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_VV2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_VV2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_VV2_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_VV2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_VV2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_VV2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_VV2_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_VI2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_VI2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_VI2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_VI2_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_VI2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_VI2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_VI2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_VI2_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_IV2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_IV2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_IV2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_IV2_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_IV2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_IV2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_IV2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_IV2_PIN_CONFIG);

    GPIO_setPadConfig(SDFM_D_II2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_D_II2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_D_II2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_D_II2_PIN_CONFIG);
    GPIO_setPadConfig(SDFM_C_II2_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SDFM_C_II2_PIN,GPIO_QUAL_SYNC);
    GPIO_setDirectionMode(SDFM_C_II2_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPinConfig(SDFM_C_II2_PIN_CONFIG);

    //HVBUS METER init
    GPIO_setMasterCore(BRIDGE_V_EN_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(BRIDGE_V_EN_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(BRIDGE_V_EN_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(BRIDGE_V_EN_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(BRIDGE_V_EN_PIN_CONFIG);

    GPIO_setMasterCore(BRIDGE_I_EN_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(BRIDGE_I_EN_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(BRIDGE_I_EN_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(BRIDGE_I_EN_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(BRIDGE_I_EN_PIN_CONFIG);

    GPIO_writePin(HVBUS_MEAS_ENA_PIN, 1); //( 0: ON | 1: OFF )
    GPIO_setMasterCore(HVBUS_MEAS_ENA_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_MEAS_ENA_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_MEAS_ENA_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_MEAS_ENA_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_MEAS_ENA_PIN_CONFIG);

    GPIO_writePin(HVBUS_CTRL_PIN, 0);
    GPIO_setMasterCore(HVBUS_CTRL_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_CTRL_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_CTRL_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_CTRL_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_CTRL_PIN_CONFIG);

    GPIO_writePin(HVBUS_CTRL_OWNER_PIN, 0);
    GPIO_setMasterCore(HVBUS_CTRL_OWNER_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_CTRL_OWNER_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_CTRL_OWNER_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_CTRL_OWNER_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_CTRL_OWNER_PIN_CONFIG);

    GPIO_writePin(HVBUS_IFAULT_P_PIN, 0);
    GPIO_setMasterCore(HVBUS_IFAULT_P_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_IFAULT_P_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_IFAULT_P_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_IFAULT_P_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_IFAULT_P_PIN_CONFIG);

    GPIO_writePin(HVBUS_IFAULT_N_PIN, 0);
    GPIO_setMasterCore(HVBUS_IFAULT_N_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_IFAULT_N_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_IFAULT_N_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_IFAULT_N_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_IFAULT_N_PIN_CONFIG);

    GPIO_writePin(HVBUS_ZC_P_PIN, 0);
    GPIO_setMasterCore(HVBUS_ZC_P_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_ZC_P_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_ZC_P_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_ZC_P_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_ZC_P_PIN_CONFIG);

    GPIO_writePin(HVBUS_ZC_N_PIN, 0);
    GPIO_setMasterCore(HVBUS_ZC_N_PIN, GPIO_CORE_CPU1);
    GPIO_setPadConfig(HVBUS_ZC_N_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(HVBUS_ZC_N_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(HVBUS_ZC_N_PIN,GPIO_QUAL_ASYNC);
    GPIO_setPinConfig(HVBUS_ZC_N_PIN_CONFIG);

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

// _cla1_memory_config - Initialize CLA memory map
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


void _cla1_link_from_sdfm(void)
{
    EALLOW;
    // Trigger Source for TASK1 of CLA1 = SDFM1
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = CLA_TRIG_SD1INT;

    // Trigger Source for TASK1 of CLA1 = SDFM2
    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK2 = CLA_TRIG_SD2INT;

    // Lock CLA1TASKSRCSEL1 register
    DmaClaSrcSelRegs.CLA1TASKSRCSELLOCK.bit.CLA1TASKSRCSEL1=1;
    EDIS;
}

void _sdfm_init(void)
{
    uint16_t  hlt, llt;
    uint32_t sdfm_base;


    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP5);
    Interrupt_register(INT_SD1, _sdfm_isr);
    //Interrupt_register(INT_SD2, sdfm2ISR);


    // Enable SDFM1 amd SDFM2 interrupts
    //Interrupt_enable(INT_SD1);
    //Interrupt_enable(INT_SD2);

    EALLOW;
    // Enable CPU INT5 which is connected to SDFM INT
    IER |= M_INT11;
    // Enable SDFM INTn in the PIE: Group 5 __interrupt 9-10
    PieCtrlRegs.PIEIER5.bit.INTx9 = 1;  // SDFM1 interrupt enabled
    //PieCtrlRegs.PIEIER5.bit.INTx10 = 1; // SDFM2 interrupt enabled
    EDIS;



    // Input Control Unit
    // Configure Input Control Unit: Modulator Clock rate = Modulator data rate
    SDFM_setupModulatorClock(SDFM1_BASE, SDFM_FILTER_1,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM1_BASE, SDFM_FILTER_2,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM1_BASE, SDFM_FILTER_3,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM1_BASE, SDFM_FILTER_4,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);

    SDFM_setupModulatorClock(SDFM2_BASE, SDFM_FILTER_1,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM2_BASE, SDFM_FILTER_2,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM2_BASE, SDFM_FILTER_3,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);
    SDFM_setupModulatorClock(SDFM2_BASE, SDFM_FILTER_4,
                             SDFM_MODULATOR_CLK_EQUAL_DATA_RATE);


    // Input Control Module
    // Configure Input Control Mode: Modulator Clock rate = Modulator data rate
    Sdfm_configureInputCtrl(SDFM1, FILTER1, MODE_0);
    Sdfm_configureInputCtrl(SDFM1, FILTER2, MODE_0);
    Sdfm_configureInputCtrl(SDFM1, FILTER3, MODE_0);
    Sdfm_configureInputCtrl(SDFM1, FILTER4, MODE_0);
    Sdfm_configureInputCtrl(SDFM2, FILTER1, MODE_0);
    Sdfm_configureInputCtrl(SDFM2, FILTER2, MODE_0);
    Sdfm_configureInputCtrl(SDFM2, FILTER3, MODE_0);
    Sdfm_configureInputCtrl(SDFM2, FILTER4, MODE_0);



    // Comparator Unit - over and under value threshold settings
    hlt = 0x7Ff0;
    llt = 0x0010;

    // Configure Comparator Unit's comparator filter type and comparator's
    // OSR value, higher threshold, lower threshold
    SDFM_configComparator(SDFM1_BASE,
                          (SDFM_FILTER_1 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM1_BASE,
                          (SDFM_FILTER_2 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM1_BASE,
                          (SDFM_FILTER_3 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM1_BASE,
                          (SDFM_FILTER_4 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));

    SDFM_configComparator(SDFM2_BASE,
                          (SDFM_FILTER_1 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM2_BASE,
                          (SDFM_FILTER_2 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM2_BASE,
                          (SDFM_FILTER_3 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));
    SDFM_configComparator(SDFM2_BASE,
                          (SDFM_FILTER_4 | SDFM_FILTER_SINC_3 | SDFM_SET_OSR(32)),
                          (SDFM_GET_LOW_THRESHOLD(llt) | SDFM_GET_HIGH_THRESHOLD(hlt)));

    // Data Filter Unit
    // Configure Data Filter Unit - filter type, OSR value and
    // enable / disable data filter
    Sdfm_configureData_filter(SDFM1, FILTER1, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM1, FILTER2, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM1, FILTER3, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM1, FILTER4, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);

    Sdfm_configureData_filter(SDFM2, FILTER1, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM2, FILTER2, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM2, FILTER3, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);
    Sdfm_configureData_filter(SDFM2, FILTER4, FILTER_ENABLE, SINC3,
                              OSR_256, DATA_32_BIT, SHIFT_0_BITS);


    // Enable Master filter bit: Unless this bit is set none of the filter modules
    // can be enabled. All the filter modules are synchronized when master filter
    // bit is enabled after individual filter modules are enabled.
    SDFM_enableMasterFilter(SDFM1_BASE);
    SDFM_enableMasterFilter(SDFM2_BASE);

    //    // PWM11.CMPC, PWM11.CMPD, PWM12.CMPC and PWM12.CMPD signals cannot synchronize
    //    // the filters. This option is not being used in this example.
    //    //
    //    Sdfm_configureExternalreset(gPeripheralNumber,FILTER_1_EXT_RESET_DISABLE,
    //                                FILTER_2_EXT_RESET_DISABLE,
    //                                FILTER_3_EXT_RESET_DISABLE,
    //                                FILTER_4_EXT_RESET_DISABLE);
    //
    //
    //
    // PWM11.CMPC, PWM11.CMPD, PWM12.CMPC and PWM12.CMPD signals cannot synchronize
    // the filters. This option is not being used in this example.
    //
    SDFM_disableExternalReset(SDFM1_BASE, SDFM_FILTER_1);
    SDFM_disableExternalReset(SDFM1_BASE, SDFM_FILTER_2);
    SDFM_disableExternalReset(SDFM1_BASE, SDFM_FILTER_3);
    SDFM_disableExternalReset(SDFM1_BASE, SDFM_FILTER_4);

    SDFM_disableExternalReset(SDFM2_BASE, SDFM_FILTER_1);
    SDFM_disableExternalReset(SDFM2_BASE, SDFM_FILTER_2);
    SDFM_disableExternalReset(SDFM2_BASE, SDFM_FILTER_3);
    SDFM_disableExternalReset(SDFM2_BASE, SDFM_FILTER_4);

    // Enable interrupts
    // Following SDFM interrupts can be enabled / disabled using this function.
    // Enable / disable comparator high threshold
    // Enable / disable comparator low threshold
    // Enable / disable modulator clock failure
    // Enable / disable data filter acknowledge
    //
    //    SDFM_enableInterrupt(SDFM1_BASE, SDFM_FILTER_1,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    SDFM_enableInterrupt(SDFM1_BASE, SDFM_FILTER_2,
                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    //    SDFM_enableInterrupt(SDFM1_BASE, SDFM_FILTER_3,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    //    SDFM_enableInterrupt(SDFM1_BASE, SDFM_FILTER_4,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    SDFM_disableInterrupt(SDFM1_BASE, SDFM_FILTER_1,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM1_BASE, SDFM_FILTER_2,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM1_BASE, SDFM_FILTER_3,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM1_BASE, SDFM_FILTER_4,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));

    //    SDFM_enableInterrupt(SDFM2_BASE, SDFM_FILTER_1,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    //    SDFM_enableInterrupt(SDFM2_BASE, SDFM_FILTER_2,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    //    SDFM_enableInterrupt(SDFM2_BASE, SDFM_FILTER_3,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));
    //    SDFM_enableInterrupt(SDFM2_BASE, SDFM_FILTER_4,
    //                         (SDFM_MODULATOR_FAILURE_INTERRUPT |
    //                                 SDFM_DATA_FILTER_ACKNOWLEDGE_INTERRUPT));

    SDFM_disableInterrupt(SDFM2_BASE, SDFM_FILTER_1,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM2_BASE, SDFM_FILTER_2,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM2_BASE, SDFM_FILTER_3,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));
    SDFM_disableInterrupt(SDFM2_BASE, SDFM_FILTER_4,
                          (SDFM_HIGH_LEVEL_THRESHOLD_INTERRUPT |
                                  SDFM_LOW_LEVEL_THRESHOLD_INTERRUPT));


    // Enable master interrupt so that any of the filter interrupts can trigger
    // by SDFM interrupt to CPU
    SDFM_enableMasterInterrupt(SDFM1_BASE);


}


#pragma CODE_SECTION(_sdfm_isr, ".TI.ramfunc")
__interrupt void _sdfm_isr(void)
{
    Uint32 sdfmReadFlagRegister = 0;

    //
    // Read SDFM flag register (SDIFLG)
    //
    sdfmReadFlagRegister = Sdfm_readFlagRegister(SDFM1);

    if(sdfmReadFlagRegister&(~0x8000F000))
    {
        ESTOP0;
    }

    //
    // Clear SDFM flag register
    //
    Sdfm_clearFlagRegister(SDFM1,sdfmReadFlagRegister);
    sdfmReadFlagRegister = Sdfm_readFlagRegister(SDFM1);
    if(sdfmReadFlagRegister != 0x0)
    {
        ESTOP0;
    }

    //
    // Acknowledge this __interrupt to receive more __interrupts from group 5
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;
}


#pragma CODE_SECTION(access_violation, ".TI.ramfunc")
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
#pragma CODE_SECTION(cla1Isr1, ".TI.ramfunc")
interrupt void cla1Isr1 ()
{
    float32_t rkv;
    float32_t rki;
    float32_t controller_out;
    static int loop = 0;
    static float32_t vv1rms  = 0.0f;
    static float32_t vi1rms  = 0.0f;
    static float32_t iv1rms  = 0.0f;
    static float32_t ii1rms  = 0.0f;
    static float32_t vv1avg  = 0.0f;
    static float32_t vi1avg  = 0.0f;
    static float32_t iv1avg  = 0.0f;
    static float32_t ii1avg  = 0.0f;
    static float32_t vv1ppk  = 0.0f;
    static float32_t vi1ppk  = 0.0f;
    static float32_t iv1ppk  = 0.0f;
    static float32_t ii1ppk  = 0.0f;
    static float32_t vv1npk  = 0.0f;
    static float32_t vi1npk  = 0.0f;
    static float32_t iv1npk  = 0.0f;
    static float32_t ii1npk  = 0.0f;
    static float32_t vv2rms  = 0.0f;
    static float32_t vi2rms  = 0.0f;
    static float32_t iv2rms  = 0.0f;
    static float32_t ii2rms  = 0.0f;
    static float32_t vv2avg  = 0.0f;
    static float32_t vi2avg  = 0.0f;
    static float32_t iv2avg  = 0.0f;
    static float32_t ii2avg  = 0.0f;
    static float32_t vv2ppk  = 0.0f;
    static float32_t vi2ppk  = 0.0f;
    static float32_t iv2ppk  = 0.0f;
    static float32_t ii2ppk  = 0.0f;
    static float32_t vv2npk  = 0.0f;
    static float32_t vi2npk  = 0.0f;
    static float32_t iv2npk  = 0.0f;
    static float32_t ii2npk  = 0.0f;
    static float32_t isetpoint  = 0.0f;
    static float32_t vsetpoint  = 0.0f;
    static float32_t ictrlerror = 0.0f;
    static float32_t vctrlerror = 0.0f;

    GPIO_writePin(USR_GPIO3, 1);
    if(app.generation.sync_flag){
        app.generation.sync_flag = false;
        loop = 0;
        app.generation.current.wg->ref.waveform_index = 0;
        app.generation.voltage.wg->ref.waveform_index = 0;
    }

    app.measures.primary.voltage.voltage[loop] = cla_vv1.data;
    app.measures.primary.voltage.current[loop] = cla_vi1.data;
    app.measures.primary.current.voltage[loop] = cla_iv1.data;
    app.measures.primary.current.current[loop] = cla_ii1.data;
    app.measures.secondary.voltage.voltage[loop] = cla_vv2.data;
    app.measures.secondary.voltage.current[loop] = cla_vi2.data;
    app.measures.secondary.current.voltage[loop] = cla_iv2.data;
    app.measures.secondary.current.current[loop] = cla_ii2.data;

    rki =  reference_routine(&app.generation.current.wg->ref);
    rki = rki*app.generation.current.wg->config.scale;
    rkv =  reference_routine(&app.generation.voltage.wg->ref);
    rkv = rkv*app.generation.voltage.wg->config.scale;


    //    if(!app.generation.current.command.enable)
    //        rki = 0.0f;
    //    if(!app.generation.voltage.command.enable)
    //        rkv = 0.0f;

    if(app.generation.current.wg->controller == MODE_REPETITIVE_FROM_CLA)
    {
        cla_current_setpoint = rki;
        update_pwm_current(cla_current_controller.out);
        app.generation.current.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.current.wg->controller == MODE_OFF)
    {
        cla_current_controller.counter = 0;
        cla_current_controller.init_flag = false;
        app.generation.current.wg->controller = MODE_NONE;
        update_pwm_current(0.0f);
    }
    else if(app.generation.current.wg->controller == MODE_FEEDFORWARD)
    {
        update_pwm_current(rki);
        app.generation.current.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.current.wg->controller == MODE_REPETITIVE)
    {
        // controller_out = repetitive_routine(&app.generation.current.control, rki, cla_igen_current.data);
        update_pwm_current(controller_out);
        app.generation.current.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.current.wg->controller == MODE_NONE)
    {
        update_pwm_current(0.0f);
    }


    if(app.generation.voltage.wg->controller == MODE_REPETITIVE_FROM_CLA)
    {
        cla_voltage_setpoint = rkv;
        update_pwm_voltage(cla_voltage_controller.out);
        app.generation.voltage.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.voltage.wg->controller == MODE_OFF)
    {
        cla_voltage_controller.counter = 0;
        cla_voltage_controller.init_flag = false;
        app.generation.voltage.wg->controller = MODE_NONE;
        update_pwm_voltage(0.0f);
    }
    else if(app.generation.voltage.wg->controller == MODE_FEEDFORWARD)
    {
        update_pwm_voltage(rkv);
        app.generation.voltage.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.voltage.wg->controller == MODE_REPETITIVE)
    {
        cla_voltage_setpoint = rkv;
        //controller_out = repetitive_routine(&app.generation.voltage.control, rkv, cla_v1_voltage.data);
        update_pwm_voltage(controller_out);
        app.generation.voltage.wg->trigger.from_control_loop = true;
    }
    else if(app.generation.voltage.wg->controller == MODE_NONE)
    {
        update_pwm_voltage(0.0f);
    }


    vv1rms += app.measures.primary.voltage.voltage[loop] * app.measures.primary.voltage.voltage[loop];
    vi1rms += app.measures.primary.voltage.current[loop] * app.measures.primary.voltage.current[loop];
    iv1rms += app.measures.primary.current.voltage[loop] * app.measures.primary.current.voltage[loop];
    ii1rms += app.measures.primary.current.current[loop] * app.measures.primary.current.current[loop];
    vv2rms += app.measures.secondary.voltage.voltage[loop] * app.measures.secondary.voltage.voltage[loop];
    vi2rms += app.measures.secondary.voltage.current[loop] * app.measures.secondary.voltage.current[loop];
    iv2rms += app.measures.secondary.current.voltage[loop] * app.measures.secondary.current.voltage[loop];
    ii2rms += app.measures.secondary.current.current[loop] * app.measures.secondary.current.current[loop];


    ictrlerror += cla_current_controller.v_erro[loop] * cla_current_controller.v_erro[loop];
    vctrlerror += cla_voltage_controller.v_erro[loop] * cla_voltage_controller.v_erro[loop];
    isetpoint += rki*rki;
    vsetpoint += rkv*rkv;

    vv1avg += app.measures.primary.voltage.voltage[loop];
    vi1avg += app.measures.primary.voltage.current[loop];
    iv1avg += app.measures.primary.current.voltage[loop];
    ii1avg += app.measures.primary.current.current[loop];
    vv2avg += app.measures.secondary.voltage.voltage[loop];
    vi2avg += app.measures.secondary.voltage.current[loop];
    iv2avg += app.measures.secondary.current.voltage[loop];
    ii2avg += app.measures.secondary.current.current[loop];

    app.measures.primary.voltage.power_sum += (app.measures.primary.voltage.voltage[loop] * app.measures.primary.voltage.current[loop]) / (1000.0f);
    app.measures.primary.current.power_sum += (app.measures.primary.current.voltage[loop] * app.measures.primary.current.current[loop]) / (1000.0f);
    app.measures.secondary.voltage.power_sum += (app.measures.secondary.voltage.voltage[loop] * app.measures.secondary.voltage.current[loop]) / (1000.0f);
    app.measures.secondary.current.power_sum += (app.measures.secondary.current.voltage[loop] * app.measures.secondary.current.current[loop]) / (1000.0f);

    if(vv1ppk < app.measures.primary.voltage.voltage[loop])
        vv1ppk = app.measures.primary.voltage.voltage[loop];
    if(vi1ppk < app.measures.primary.voltage.current[loop])
        vi1ppk = app.measures.primary.voltage.current[loop];
    if(iv1ppk < app.measures.primary.current.voltage[loop])
        iv1ppk = app.measures.primary.current.voltage[loop];
    if(ii1ppk < app.measures.primary.current.current[loop])
        ii1ppk = app.measures.primary.current.current[loop];
    if(vv2ppk < app.measures.secondary.voltage.voltage[loop])
        vv2ppk = app.measures.secondary.voltage.voltage[loop];
    if(vi2ppk < app.measures.secondary.voltage.current[loop])
        vi2ppk = app.measures.secondary.voltage.current[loop];
    if(iv2ppk < app.measures.secondary.current.voltage[loop])
        iv2ppk = app.measures.secondary.current.voltage[loop];
    if(ii2ppk < app.measures.secondary.current.current[loop])
        ii2ppk = app.measures.secondary.current.current[loop];

    if(vv1npk > app.measures.primary.voltage.voltage[loop])
        vv1npk = app.measures.primary.voltage.voltage[loop];
    if(vi1npk > app.measures.primary.voltage.current[loop])
        vi1npk = app.measures.primary.voltage.current[loop];
    if(iv1npk > app.measures.primary.current.voltage[loop])
        iv1npk = app.measures.primary.current.voltage[loop];
    if(ii1npk > app.measures.primary.current.current[loop])
        ii1npk = app.measures.primary.current.current[loop];
    if(vv2npk > app.measures.secondary.voltage.voltage[loop])
        vv2npk = app.measures.secondary.voltage.voltage[loop];
    if(vi2npk > app.measures.secondary.voltage.current[loop])
        vi2npk = app.measures.secondary.voltage.current[loop];
    if(iv2npk > app.measures.secondary.current.voltage[loop])
        iv2npk = app.measures.secondary.current.voltage[loop];
    if(ii2npk > app.measures.secondary.current.current[loop])
        ii2npk = app.measures.secondary.current.current[loop];

    if( ++loop >= 1000 )
    {
        app.measures.primary.voltage.setpoint_rms = sqrtf(vsetpoint/1000.0f);
        app.measures.primary.current.setpoint_rms = sqrtf(isetpoint/1000.0f);
        app.measures.secondary.voltage.setpoint_rms = sqrtf(vsetpoint/1000.0f);
        app.measures.secondary.current.setpoint_rms = sqrtf(isetpoint/1000.0f);

        app.measures.primary.voltage.voltage_rms = sqrtf(vv1rms/1000.0f);
        app.measures.primary.voltage.current_rms = sqrtf(vi1rms/1000.0f);
        app.measures.primary.current.voltage_rms = sqrtf(iv1rms/1000.0f);
        app.measures.primary.current.current_rms = sqrtf(ii1rms/1000.0f);

        app.measures.secondary.voltage.voltage_rms = sqrtf(vv2rms/1000.0f);
        app.measures.secondary.voltage.current_rms = sqrtf(vi2rms/1000.0f);
        app.measures.secondary.current.voltage_rms = sqrtf(iv2rms/1000.0f);
        app.measures.secondary.current.current_rms = sqrtf(ii2rms/1000.0f);

        app.measures.primary.current.ctrl_erro_rms = sqrtf(ictrlerror/1000.0f);
        app.measures.primary.voltage.ctrl_erro_rms = sqrtf(vctrlerror/1000.0f);
        app.measures.secondary.current.ctrl_erro_rms = sqrtf(ictrlerror/1000.0f);
        app.measures.secondary.voltage.ctrl_erro_rms = sqrtf(vctrlerror/1000.0f);

        app.measures.primary.voltage.voltage_avg = vv1avg/1000.0f;
        app.measures.primary.voltage.current_avg = vi1avg/1000.0f;
        app.measures.primary.current.voltage_avg = iv1avg/1000.0f;
        app.measures.primary.current.current_avg = ii1avg/1000.0f;

        app.measures.secondary.voltage.voltage_avg = vv2avg/1000.0f;
        app.measures.secondary.voltage.current_avg = vi2avg/1000.0f;
        app.measures.secondary.current.voltage_avg = iv2avg/1000.0f;
        app.measures.secondary.current.current_avg = ii2avg/1000.0f;

        app.measures.primary.voltage.voltage_ppeak = vv1ppk;
        app.measures.primary.voltage.current_ppeak = vi1ppk;
        app.measures.primary.current.voltage_ppeak = iv1ppk;
        app.measures.primary.current.current_ppeak = ii1ppk;

        app.measures.secondary.voltage.voltage_ppeak = vv2ppk;
        app.measures.secondary.voltage.current_ppeak = vi2ppk;
        app.measures.secondary.current.voltage_ppeak = iv2ppk;
        app.measures.secondary.current.current_ppeak = ii2ppk;

        app.measures.primary.voltage.voltage_npeak = vv1npk;
        app.measures.primary.voltage.current_npeak = vi1npk;
        app.measures.primary.current.voltage_npeak = iv1npk;
        app.measures.primary.current.current_npeak = ii1npk;

        app.measures.secondary.voltage.voltage_npeak = vv2npk;
        app.measures.secondary.voltage.current_npeak = vi2npk;
        app.measures.secondary.current.voltage_npeak = iv2npk;
        app.measures.secondary.current.current_npeak = ii2npk;

        app.measures.primary.voltage.power = app.measures.primary.voltage.power_sum;
        app.measures.primary.current.power = app.measures.primary.current.power_sum;
        app.measures.secondary.voltage.power = app.measures.secondary.voltage.power_sum;
        app.measures.secondary.current.power = app.measures.secondary.current.power_sum;

        app.measures.primary.voltage.power_sum = 0.0f;
        app.measures.primary.current.power_sum = 0.0f;
        app.measures.secondary.voltage.power_sum = 0.0f;
        app.measures.secondary.current.power_sum = 0.0f;


        app.measures.primary.voltage.impedance = app.measures.primary.voltage.voltage_rms / app.measures.primary.voltage.current_rms;
        app.measures.primary.current.impedance = app.measures.primary.current.voltage_rms / app.measures.primary.current.current_rms;

        app.measures.secondary.voltage.impedance = app.measures.secondary.voltage.voltage_rms / app.measures.secondary.voltage.current_rms;
        app.measures.secondary.current.impedance = app.measures.secondary.current.voltage_rms / app.measures.secondary.current.current_rms;



        if(app.measures.primary.voltage.power > 10E-3 ) //10mWW
        {
            if(app.measures.secondary.voltage.impedance < 20.0)
            {
                voltage_event = 1;
            }

            if( app.measures.primary.voltage.power  > 10.0 )
                if( app.measures.primary.voltage.power  > 10 * app.measures.secondary.voltage.power)
                {
                    if(++voltage_overpower_counter>20)
                        voltage_event = 2;
                }

        }

        if(app.measures.primary.current.power > 1E-4 ) //0.1mW
        {
            if(app.measures.secondary.current.impedance > 10.0)
            {
                current_event = 1;
            }
            if(app.measures.primary.current.impedance > 500.0)
            {
                current_event = 2;
            }
            if( app.measures.primary.current.power  > 10* app.measures.secondary.current.power)
            {
                current_event = 3;
            }
        }



        vv1avg = 0.0f;
        vi1avg = 0.0f;
        iv1avg = 0.0f;
        ii1avg = 0.0f;
        vv2avg = 0.0f;
        vi2avg = 0.0f;
        iv2avg = 0.0f;
        ii2avg = 0.0f;

        vv1rms = 0.0f;
        vi1rms = 0.0f;
        iv1rms = 0.0f;
        ii1rms = 0.0f;
        vv2rms = 0.0f;
        vi2rms = 0.0f;
        iv2rms = 0.0f;
        ii2rms = 0.0f;

        vv1ppk = 0.0f;
        vi1ppk = 0.0f;
        iv1ppk = 0.0f;
        ii1ppk = 0.0f;
        vv2ppk = 0.0f;
        vi2ppk = 0.0f;
        iv2ppk = 0.0f;
        ii2ppk = 0.0f;

        vv1npk = 0.0f;
        vi1npk = 0.0f;
        iv1npk = 0.0f;
        ii1npk = 0.0f;
        vv2npk = 0.0f;
        vi2npk = 0.0f;
        iv2npk = 0.0f;
        ii2npk = 0.0f;

        isetpoint = 0.0;
        vsetpoint = 0.0;
        ictrlerror = 0.0f;
        vctrlerror = 0.0f;

        loop = 0;

        app.generation.zero_trigger = true;

    }

    GPIO_writePin(USR_GPIO3, 0);
    // Acknowledge the end-of-task interrupt for task 1

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


