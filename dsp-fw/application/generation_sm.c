#include "generation_sm.h"
#include <math.h>


void _initHRPWM(uint32_t base1, uint32_t base2, uint32_t period, uint32_t deadband);
void _setupEPWMActiveHighComplementary(uint32_t base, uint32_t deadband);
void _update_pwm_voltage(float32_t duty);
void _update_pwm_current(float32_t duty);

// helpers
static inline float32_t _step_towards(float32_t cur, float32_t tgt, float32_t step)
{
    if (tgt > cur) { float32_t nx = cur + step; return (nx > tgt) ? tgt : nx; }
    if (tgt < cur) { float32_t nx = cur - step; return (nx < tgt) ? tgt : nx; }
    return cur;
}

void gen_sm_init(gen_runtime_t* r, gen_profile_t prof)
{
    (void)prof;
    *r = (gen_runtime_t){0};
    r->st = GEN_ST_IDLE;
}

void gen_sm_set_target(gen_runtime_t* r, gen_target_t t)
{
    r->tgt = t;
    // if waveform changed
    r->pending_wave_switch = (t.waveform_target != r->cur.waveform_target);
    // if gen was in idle, ramp up
    if (r->st == GEN_ST_IDLE || r->st == GEN_ST_HOLD) {
        r->st = r->pending_wave_switch ? GEN_ST_RAMP_DOWN : GEN_ST_RAMP_UP;
    }
}

void gen_sm_turning_on(gen_runtime_t* r){

    //Bridge V En - on
    GPIO_writePin(BRIDGE_V_EN_PIN, 1);
    //Bridge I En - on
    GPIO_writePin(BRIDGE_I_EN_PIN, 1);

    GPIO_writePin(SDFM_CLK_SEL_PIN,0);                              // SDFM_CLK_SEL: 0 from ext STD; 1 from int SI5351
    GPIO_writePin(CLK_AUX_OE_PIN,1);                                // CLK_AUX_OE = 1 clk out enabled


    GPIO_writePin(FB_EN_PIN, 1);
}


void gen_sm_on_zero_cross(gen_runtime_t* r) {
    r->zc_tick = true;
}

void gen_sm_on_protection(gen_runtime_t* r) {
    r->st = GEN_ST_FAULT;
}

// Aplicar setpoints (CPU1 → CLA): use seus campos/setpoints atuais
static void _apply_setpoints(application_t* app,
                             float32_t v_scale, float32_t i_scale,
                             uint8_t waveform) {
    // Exemplo: atualize os setpoints usados pela CLA
    // Você já deixa os controladores na CLA e atualiza setpoints externamente. :contentReference[oaicite:2]{index=2}
    app->generation.voltage.config.scale  = v_scale;
    app->generation.current.config.scale  = i_scale;
//    app->generation.voltage.ref.waveform  = waveform;
//    app->generation.current.ref.waveform  = waveform;
}

//bool gen_sm_process(gen_runtime_t* r,
//                     gen_profile_t prof,
//                     application_t* app)
//{
//    if (!r->zc_tick) return false;     // só avança por zero-cross
//    r->zc_tick = false;
//
//    bool changed = false;
//
//    switch (r->st) {
//    case GEN_ST_IDLE:
//        // nada a fazer
//        break;
//
//    case GEN_ST_RAMP_DOWN: {
//        float32_t step = fmaxf(prof.slew_down_per_zc, prof.min_step);
//        float32_t v = _step_towards(r->cur.scale_target, 0.0f, step);
//        r->cur.scale_target = v;
//        _apply_setpoints(app, v, v, r->cur.waveform_target);
//        changed = true;
//
//        if (v <= 0.0f + 1e-6f) {
//            r->st = r->pending_wave_switch ? GEN_ST_SWITCH_WAVEFORM : GEN_ST_RAMP_UP;
//        }
//    } break;
//
//    case GEN_ST_SWITCH_WAVEFORM:
//        r->cur.waveform_target = r->tgt.waveform_target; // troca “em zero”
//        r->pending_wave_switch = false;
//        r->st = GEN_ST_RAMP_UP;
//        // aplica já com 0 de escala (continua em zero)
//        _apply_setpoints(app, 0.0f, 0.0f, r->cur.waveform_target);
//        changed = true;
//        break;
//
//    case GEN_ST_RAMP_UP: {
//        float32_t step = fmaxf(prof.slew_up_per_zc, prof.min_step);
//        float32_t v = _step_towards(r->cur.scale_target, r->tgt.scale_target, step);
//        r->cur.scale_target = v;
//        _apply_setpoints(app, v, v, r->cur.waveform_target);
//        changed = true;
//
//        if (fabsf(v - r->tgt.scale_target) <= 1e-6f) {
//            r->st = GEN_ST_HOLD;
//        }
//    } break;
//
//    case GEN_ST_HOLD:
//        // alvos podem mudar a qualquer momento via gen_fsm_set_target()
//        break;
//
//    case GEN_ST_FAULT:
//        // mantenha em zero e aguarde reset externo (proteção/CLI)
//        _apply_setpoints(app, 0.0f, 0.0f, r->cur.waveform_target);
//        changed = true;
//        break;
//    }
//
//    return changed;
//}




//TODO setupInverterPWM
void setupInverter(uint32_t voltage1, uint32_t voltage2,
                   uint32_t current1, uint32_t current2,
                   uint16_t pwm_period_ticks,
                   uint16_t pwm_deadband_ticks)
{

    GPIO_setDirectionMode(INV_PWM1_VOLTAGE_H_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM1_VOLTAGE_H_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM1_VOLTAGE_H_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM1_VOLTAGE_L_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM1_VOLTAGE_L_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM1_VOLTAGE_L_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM2_VOLTAGE_H_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM2_VOLTAGE_H_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM2_VOLTAGE_H_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM2_VOLTAGE_L_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM2_VOLTAGE_L_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM2_VOLTAGE_L_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM1_CURRENT_H_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM1_CURRENT_H_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM1_CURRENT_H_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM1_CURRENT_L_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM1_CURRENT_L_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM1_CURRENT_L_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM2_CURRENT_H_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM2_CURRENT_H_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM2_CURRENT_H_PIN_CONFIG);

    GPIO_setDirectionMode(INV_PWM2_CURRENT_L_PIN,GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(INV_PWM2_CURRENT_L_PIN,GPIO_PIN_TYPE_STD); // disable pull up
    GPIO_setPinConfig(INV_PWM2_CURRENT_L_PIN_CONFIG);

    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    _initHRPWM(current1, current2, pwm_period_ticks, pwm_deadband_ticks);
    _initHRPWM(voltage1, voltage2, pwm_period_ticks, pwm_deadband_ticks);


    //todo: macros need correction
    EPWM_setupEPWMLinks(EPWM2_BASE, EPWM_LINK_WITH_EPWM_1, EPWM_LINK_TBPRD);
    EPWM_setupEPWMLinks(EPWM5_BASE, EPWM_LINK_WITH_EPWM_1, EPWM_LINK_TBPRD);
    EPWM_setupEPWMLinks(EPWM6_BASE, EPWM_LINK_WITH_EPWM_1, EPWM_LINK_TBPRD);

    //    EPWM_setupEPWMLinks(EPWM2_BASE, EPWM_LINK_WITH_EPWM_1, EPWM_LINK_COMP_A);
    //    EPWM_setupEPWMLinks(EPWM2_BASE, EPWM_LINK_WITH_EPWM_1, EPWM_LINK_COMP_B);
    //    EPWM_setupEPWMLinks(EPWM4_BASE, EPWM_LINK_WITH_EPWM_3, EPWM_LINK_COMP_A);
    //    EPWM_setupEPWMLinks(EPWM4_BASE, EPWM_LINK_WITH_EPWM_3, EPWM_LINK_COMP_B);

    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);


}



void _initHRPWM(uint32_t base1, uint32_t base2, uint32_t period, uint32_t deadband)
{

    app.generation.config.deadband = ((deadband << 7) | 0x00);
    app.generation.config.inverter_pwm_steps = (period << 8);

    //TRIP ZONE
    //    EPWM_setTripZoneAction(INV_PWM1_VOLTAGE,EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    //    EPWM_setTripZoneAction(INV_PWM1_VOLTAGE,EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
    //    EPWM_setTripZoneAction(INV_PWM2_VOLTAGE,EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_LOW);
    //    EPWM_setTripZoneAction(INV_PWM2_VOLTAGE,EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_LOW);
    // Clear any spurious fault
    EPWM_clearTripZoneFlag(base1, EPWM_TZ_FLAG_CBC     +
                           EPWM_TZ_FLAG_OST     +
                           EPWM_TZ_FLAG_DCAEVT1 +
                           EPWM_TZ_FLAG_DCAEVT2 +
                           EPWM_TZ_FLAG_DCBEVT1 );
    EPWM_clearTripZoneFlag(base2, EPWM_TZ_FLAG_CBC     +
                           EPWM_TZ_FLAG_OST     +
                           EPWM_TZ_FLAG_DCAEVT1 +
                           EPWM_TZ_FLAG_DCAEVT2 +
                           EPWM_TZ_FLAG_DCBEVT1 );

//    EPWM_forceTripZoneEvent(base1, EPWM_TZ_FORCE_EVENT_OST);
//    EPWM_forceTripZoneEvent(base2, EPWM_TZ_FORCE_EVENT_OST);


    EPWM_setPeriodLoadMode(base1, EPWM_PERIOD_SHADOW_LOAD);
    EPWM_setPeriodLoadMode(base2, EPWM_PERIOD_SHADOW_LOAD);
    EPWM_setTimeBasePeriod(base1, period);
    EPWM_setTimeBasePeriod(base2, period);

    // Set duty cycle of 50% by setting CMPA & CMPB values for EPWM1A & EPWM1B
    // signals respectively
    HRPWM_setCounterCompareValue(base1, HRPWM_COUNTER_COMPARE_A, ( (period/2) << 8) );
    HRPWM_setCounterCompareValue(base1, HRPWM_COUNTER_COMPARE_B, 0);
    HRPWM_setCounterCompareValue(base2, HRPWM_COUNTER_COMPARE_A, ( (period/2) << 8) );
    HRPWM_setCounterCompareValue(base2, HRPWM_COUNTER_COMPARE_B, 0);

    // Configure CMPA & CMPB load event & shadow modes.
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base2, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base2, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);

    EPWM_setPhaseShift(base1, 0U);
    EPWM_disablePhaseShiftLoad(base1);
    EPWM_setPhaseShift(base2, 0U);
    EPWM_disablePhaseShiftLoad(base2);

    EPWM_setTimeBaseCounter(base1, 0U);
    //EPWM_setTimeBaseCounter(base2, (period)/2);
    EPWM_setTimeBaseCounter(base2, 0U);

    EPWM_setEmulationMode(base1, EPWM_EMULATION_FREE_RUN);
    EPWM_setEmulationMode(base2, EPWM_EMULATION_FREE_RUN);

    EPWM_setTimeBaseCounterMode(base1, EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setTimeBaseCounterMode(base2, EPWM_COUNTER_MODE_UP_DOWN);

    EPWM_setSyncOutPulseMode(base1, EPWM_SYNC_OUT_PULSE_DISABLED);
    EPWM_setSyncOutPulseMode(base2, EPWM_SYNC_OUT_PULSE_DISABLED);

    EPWM_setClockPrescaler(base1, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    EPWM_setClockPrescaler(base2, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    EPWM_setActionQualifierShadowLoadMode(base1, EPWM_ACTION_QUALIFIER_A, EPWM_AQ_LOAD_ON_CNTR_ZERO);
    EPWM_setActionQualifierShadowLoadMode(base1, EPWM_ACTION_QUALIFIER_B, EPWM_AQ_LOAD_ON_CNTR_ZERO);
    EPWM_setActionQualifierShadowLoadMode(base2, EPWM_ACTION_QUALIFIER_A, EPWM_AQ_LOAD_ON_CNTR_ZERO);
    EPWM_setActionQualifierShadowLoadMode(base2, EPWM_ACTION_QUALIFIER_B, EPWM_AQ_LOAD_ON_CNTR_ZERO);

    //    EPWM_setCounterCompareShadowLoadMode(base1, EPWM_COUNTER_COMPARE_A,EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    //    EPWM_setCounterCompareShadowLoadMode(base1,EPWM_COUNTER_COMPARE_B,EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    //    EPWM_setCounterCompareShadowLoadMode(base2, EPWM_COUNTER_COMPARE_A,EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    //    EPWM_setCounterCompareShadowLoadMode(base2,EPWM_COUNTER_COMPARE_B,EPWM_COMP_LOAD_ON_CNTR_ZERO_PERIOD);
    //
    //    HRPWM_setCounterCompareShadowLoadEvent(base1, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //    HRPWM_setCounterCompareShadowLoadEvent(base1, HRPWM_CHANNEL_B, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //    HRPWM_setCounterCompareShadowLoadEvent(base2, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //    HRPWM_setCounterCompareShadowLoadEvent(base2, HRPWM_CHANNEL_B, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);

    EPWM_setActionQualifierAction(base1,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base1,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setActionQualifierAction(base1,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base1,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    EPWM_setActionQualifierAction(base2,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base2,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);
    EPWM_setActionQualifierAction(base2,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    EPWM_setActionQualifierAction(base2,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    EALLOW;
    HWREGH(base1 + HRPWM_O_HRCNFG) = 0x1353;
    HWREGH(base2 + HRPWM_O_HRCNFG) = 0x1353;
    EDIS;
    HRPWM_setTranslatorRemainder(base1, 0x0111);
    HRPWM_setTranslatorRemainder(base2, 0x0111);


    //        HRPWM_setMEPEdgeSelect(base1, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_RISING_AND_FALLING_EDGE);
    //

    HRPWM_setMEPControlMode(base1, HRPWM_CHANNEL_A, HRPWM_MEP_DUTY_PERIOD_CTRL);
    //
    //        HRPWM_setMEPEdgeSelect(base2, HRPWM_CHANNEL_A, HRPWM_MEP_CTRL_RISING_AND_FALLING_EDGE);
    //        HRPWM_setMEPControlMode(base2, HRPWM_CHANNEL_A, HRPWM_MEP_DUTY_PERIOD_CTRL);
    //
    //        //
    //        // Set up shadowing
    //        // MUST BE CTR=(ZER & PRD)
    //        //
    //        HRPWM_setCounterCompareShadowLoadEvent(base1, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //        HRPWM_setMEPEdgeSelect(base1, HRPWM_CHANNEL_B, HRPWM_MEP_CTRL_RISING_AND_FALLING_EDGE);
    //        HRPWM_setMEPControlMode(base1, HRPWM_CHANNEL_B, HRPWM_MEP_DUTY_PERIOD_CTRL);
    //
    //        HRPWM_setCounterCompareShadowLoadEvent(base2, HRPWM_CHANNEL_A, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //        HRPWM_setMEPEdgeSelect(base2, HRPWM_CHANNEL_B, HRPWM_MEP_CTRL_RISING_AND_FALLING_EDGE);
    //        HRPWM_setMEPControlMode(base2, HRPWM_CHANNEL_B, HRPWM_MEP_DUTY_PERIOD_CTRL);
    //
    //        //
    //        // Set up shadowing
    //        // MUST BE CTR=(ZER & PRD)
    //        //
    //        HRPWM_setCounterCompareShadowLoadEvent(base1, HRPWM_CHANNEL_B, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //        HRPWM_enableAutoConversion(base1);
    //        HRPWM_setCounterCompareShadowLoadEvent(base2, HRPWM_CHANNEL_B, HRPWM_LOAD_ON_CNTR_ZERO_PERIOD);
    //        HRPWM_enableAutoConversion(base2);

    //
    // Turn on high-resolution period control for DUTY to take HR on BOTH EDGEs.
    //
    HRPWM_enablePeriodControl(base1);
    HRPWM_disablePhaseShiftLoad(base1);
    HRPWM_enablePeriodControl(base2);
    HRPWM_disablePhaseShiftLoad(base2);

    //
    // Interrupt where we will change the Compare Values
    // Select INT on Time base counter zero event,
    // Enable INT, generate INT on 1st event
    //
    //EPWM_setInterruptSource(INV_PWM4_BASE, EPWM_INT_TBCTR_ZERO);
    //EPWM_enableInterrupt(INV_PWM4_BASE);
    //EPWM_setInterruptEventCount(INV_PWM4_BASE, 1U);


    HRPWM_setOutputSwapMode(base2, true);


    _setupEPWMActiveHighComplementary(base1,app.generation.config.deadband);
    _setupEPWMActiveHighComplementary(base2,app.generation.config.deadband);





}


void _setupEPWMActiveHighComplementary(uint32_t base, uint32_t deadband)
{
    //
    // Use EPWMA as the input for both RED and FED
    //
    EPWM_setRisingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);
    EPWM_setFallingEdgeDeadBandDelayInput(base, EPWM_DB_INPUT_EPWMA);

    //
    // Set the RED and FED values
    //
    HRPWM_setRisingEdgeDelay(base,deadband);
    HRPWM_setFallingEdgeDelay(base,deadband);

    //
    // Invert only the Falling Edge delayed output (AHC)
    //
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_RED, EPWM_DB_POLARITY_ACTIVE_HIGH);
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_FED, EPWM_DB_POLARITY_ACTIVE_LOW);

    //
    // Use the delayed signals instead of the original signals
    //
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED, true);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_FED, true);

    //
    // DO NOT Switch Output A with Output B
    //
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_A, false);
    EPWM_setDeadBandOutputSwapMode(base, EPWM_DB_OUTPUT_B, false);


    //!   -   - MEP (Micro Edge Positioner) controls both

    HRPWM_setDeadbandMEPEdgeSelect(base, HRPWM_DB_MEP_CTRL_RED_FED);

    EPWM_setDeadBandCounterClock(base,  EPWM_DB_COUNTER_CLOCK_HALF_CYCLE);

}



void gen_off(void)
{
    //HV_CTRL - on
    GPIO_writePin(HVBUS_CTRL_PIN, 0);
}

void gen_on(void)
{
    GPIO_writePin(HVBUS_CTRL_PIN, 1);
}


int32_t voltage_pwm_dc1 = -1;
int32_t voltage_pwm_dc2 = -1;

int32_t current_pwm_dc1 = -1;
int32_t current_pwm_dc2 = -1;



//inline void update_pwm_voltage(int32_t base1, uint32_t base2, float32_t duty)
#pragma CODE_SECTION(_update_pwm_voltage, ".TI.ramfunc")
void _update_pwm_voltage(float32_t duty)
{
    int32_t voltage_inv_duty;

    voltage_inv_duty = (app.generation.config.inverter_pwm_steps/2) + duty;

//    if(voltage_inv_duty > app.generation.voltage.protection.config.pwm_max_value )
//    {
//        voltage_inv_duty = app.generation.voltage.protection.config.pwm_max_value;
//        app.generation.voltage.command.disable_from_protection_by_saturation = true;
//    }
//    else if(voltage_inv_duty < app.generation.voltage.protection.config.pwm_min_value)
//    {
//        voltage_inv_duty = app.generation.voltage.protection.config.pwm_min_value;
//        app.generation.voltage.command.disable_from_protection_by_saturation = true;
//    }

    if(duty>0)
    {
        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_B, voltage_inv_duty + voltage_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_A, voltage_inv_duty + voltage_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_B, voltage_inv_duty + voltage_pwm_dc2);
        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_A, voltage_inv_duty + voltage_pwm_dc2);
    }
    else
    {
        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_A, voltage_inv_duty + voltage_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM1_VOLTAGE, HRPWM_COUNTER_COMPARE_B, voltage_inv_duty + voltage_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_A, voltage_inv_duty + voltage_pwm_dc2);
        HRPWM_setCounterCompareValue(INV_PWM2_VOLTAGE, HRPWM_COUNTER_COMPARE_B, voltage_inv_duty + voltage_pwm_dc2);
    }

    if(duty>app.generation.voltage.protection.data.max_pwm_duty)
        app.generation.voltage.protection.data.max_pwm_duty=duty;

    if(duty<app.generation.voltage.protection.data.min_pwm_duty)
        app.generation.voltage.protection.data.min_pwm_duty=duty;

    app.generation.voltage.protection.data.max_pwm_duty = app.generation.voltage.protection.data.max_pwm_duty - 0.001;
    app.generation.voltage.protection.data.min_pwm_duty = app.generation.voltage.protection.data.min_pwm_duty + 0.001;

}


//inline void update_pwm_current(int32_t base1, uint32_t base2, float32_t duty)
#pragma CODE_SECTION(_update_pwm_current, ".TI.ramfunc")
void _update_pwm_current(float32_t duty)
{
    uint32_t current_inv_duty;

    current_inv_duty = (app.generation.config.inverter_pwm_steps/2) + duty;

//    if(current_inv_duty > app.generation.current.protection.config.pwm_max_value )
//    {
//        current_inv_duty = app.generation.current.protection.config.pwm_max_value;
//        app.generation.current.command.disable_from_protection_by_saturation = true;
//    }
//    else if(current_inv_duty < app.generation.current.protection.config.pwm_min_value)
//    {
//        current_inv_duty = app.generation.current.protection.config.pwm_min_value;
//        app.generation.current.command.disable_from_protection_by_saturation = true;
//    }

    if(duty>0)
    {
        HRPWM_setCounterCompareValue(INV_PWM1_CURRENT, HRPWM_COUNTER_COMPARE_B, current_inv_duty + current_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM1_CURRENT, HRPWM_COUNTER_COMPARE_A, current_inv_duty + current_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM2_CURRENT, HRPWM_COUNTER_COMPARE_B, current_inv_duty + current_pwm_dc2);
        HRPWM_setCounterCompareValue(INV_PWM2_CURRENT, HRPWM_COUNTER_COMPARE_A, current_inv_duty + current_pwm_dc2);
    }
    else
    {
        HRPWM_setCounterCompareValue(INV_PWM1_CURRENT, HRPWM_COUNTER_COMPARE_A, current_inv_duty + current_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM1_CURRENT, HRPWM_COUNTER_COMPARE_B, current_inv_duty + current_pwm_dc1);
        HRPWM_setCounterCompareValue(INV_PWM2_CURRENT, HRPWM_COUNTER_COMPARE_A, current_inv_duty + current_pwm_dc2);
        HRPWM_setCounterCompareValue(INV_PWM2_CURRENT, HRPWM_COUNTER_COMPARE_B, current_inv_duty + current_pwm_dc2);
    }

    if(duty>app.generation.current.protection.data.max_pwm_duty)
        app.generation.current.protection.data.max_pwm_duty=duty;

    if(duty<app.generation.current.protection.data.min_pwm_duty)
        app.generation.current.protection.data.min_pwm_duty=duty;

    app.generation.current.protection.data.max_pwm_duty = app.generation.current.protection.data.max_pwm_duty - 0.001;
    app.generation.current.protection.data.min_pwm_duty = app.generation.current.protection.data.min_pwm_duty + 0.001;

}

