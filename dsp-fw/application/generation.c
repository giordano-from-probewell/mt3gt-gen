#include "generation.h"
#include "application.h"
#include <math.h>


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

    initHRPWM(current1, current2, pwm_period_ticks, pwm_deadband_ticks);
    initHRPWM(voltage1, voltage2, pwm_period_ticks, pwm_deadband_ticks);


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



void initHRPWM(uint32_t base1, uint32_t base2, uint32_t period, uint32_t deadband)
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


    setupEPWMActiveHighComplementary(base1,app.generation.config.deadband);
    setupEPWMActiveHighComplementary(base2,app.generation.config.deadband);





}


void setupEPWMActiveHighComplementary(uint32_t base, uint32_t deadband)
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
#pragma CODE_SECTION(update_pwm_voltage, ".TI.ramfunc")
void update_pwm_voltage(float32_t duty)
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
//
//    if(duty>app.generation.voltage.protection.data.max_pwm_duty)
//        app.generation.voltage.protection.data.max_pwm_duty=duty;
//
//    if(duty<app.generation.voltage.protection.data.min_pwm_duty)
//        app.generation.voltage.protection.data.min_pwm_duty=duty;
//
//    app.generation.voltage.protection.data.max_pwm_duty = app.generation.voltage.protection.data.max_pwm_duty - 0.001;
//    app.generation.voltage.protection.data.min_pwm_duty = app.generation.voltage.protection.data.min_pwm_duty + 0.001;

}


//inline void update_pwm_current(int32_t base1, uint32_t base2, float32_t duty)
#pragma CODE_SECTION(update_pwm_current, ".TI.ramfunc")
void update_pwm_current(float32_t duty)
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

//    if(duty>app.generation.current.protection.data.max_pwm_duty)
//        app.generation.current.protection.data.max_pwm_duty=duty;
//
//    if(duty<app.generation.current.protection.data.min_pwm_duty)
//        app.generation.current.protection.data.min_pwm_duty=duty;
//
//    app.generation.current.protection.data.max_pwm_duty = app.generation.current.protection.data.max_pwm_duty - 0.001;
//    app.generation.current.protection.data.min_pwm_duty = app.generation.current.protection.data.min_pwm_duty + 0.001;

}

