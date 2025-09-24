//###########################################################################
//
// FILE:   type4_pwm_protection_source.h
//
// TITLE:  Prototypes and Definitions for the Type 4 PWM Delayed Protection
//         Library
//
//#############################################################################
// $TI Release: C2000Ware DigitalPower SDK v4.03.01.00 $
// $Release Date: Thu Jan 26 15:57:26 CST 2023 $
// $Copyright:
// Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/
//
// ALL RIGHTS RESERVED
// $
//#############################################################################

#ifndef TYPE4_PWM_PROTECTION_SOURCE_H
#define TYPE4_PWM_PROTECTION_SOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

//*****************************************************************************
//
//! \addtogroup TYPE4_PWM
//! @{
//
//*****************************************************************************


#include "driverlib.h"
#include "device.h"

//! \brief   Defines the initialization steps for an auxiliary PWM output
//! \details Another EPWMxA will be used as auxiliary trip signal for inner
//!          switches, with the dead-band insertion for the auxiliary PWM action
//!          at the system trip event.
//! \param   aux_pwm_base  PWM base register for the auxiliary PWM output
//! \param   system_trip_source  Trip input for the system trip event
//! \param   trip_delay  Customized delay timing with rising edge delay
//!
//! \return None
void initEPWM_aux_trip(uint32_t aux_pwm_base,
                       EPWM_DigitalCompareTripInput system_trip_source,
                       uint16_t trip_delay)
{

    EALLOW;
    EPWM_setTimeBasePeriod(aux_pwm_base, 2500);
    EPWM_setTimeBaseCounter(aux_pwm_base,0);
    EPWM_setPhaseShift(aux_pwm_base,0);
    EPWM_disablePhaseShiftLoad(aux_pwm_base);
    EPWM_setTimeBaseCounterMode(aux_pwm_base,EPWM_COUNTER_MODE_UP_DOWN);
    EPWM_setClockPrescaler(aux_pwm_base, EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    // S1 = 1
    EPWM_setDeadBandDelayMode(aux_pwm_base,EPWM_DB_RED, true);
    // S2 = 0
    EPWM_setDeadBandDelayPolarity(aux_pwm_base, EPWM_DB_RED,
                                  EPWM_DB_POLARITY_ACTIVE_HIGH);

    // S4 = 0
    EPWM_setRisingEdgeDeadBandDelayInput(aux_pwm_base, EPWM_DB_INPUT_EPWMA);
    EPWM_setRisingEdgeDelayCount(aux_pwm_base, trip_delay);

    //
    // DCBH = TRIPIN4 = INPUT X-BAR 1 = EPWM8A in this example
    //
    EPWM_selectDigitalCompareTripInput(aux_pwm_base, system_trip_source,
                                       EPWM_DC_TYPE_DCBH);
    //
    //Trigger DCBEVT1 when system_trip_source signal goes low(fault occurs)
    //Trigger DCBEVT2 when system_trip_source signal goes high(recover from fault)
    //
    EPWM_setTripZoneDigitalCompareEventCondition(aux_pwm_base,
                                                 EPWM_TZ_DC_OUTPUT_B1,
                                                 EPWM_TZ_EVENT_DCXH_LOW);
    EPWM_setTripZoneDigitalCompareEventCondition(aux_pwm_base,
                                                 EPWM_TZ_DC_OUTPUT_B2,
                                                 EPWM_TZ_EVENT_DCXH_HIGH);
    //
    // DCBEVT1/2 event as AQ T1/2
    //
    // T1 = DCBEVT1
    EPWM_setActionQualifierT1TriggerSource(aux_pwm_base,
                                           EPWM_AQ_TRIGGER_EVENT_TRIG_DCB_1);
    // T2 = DCBEVT2
    EPWM_setActionQualifierT2TriggerSource(aux_pwm_base,
                                           EPWM_AQ_TRIGGER_EVENT_TRIG_DCB_2);

    //
    // EPWMA Trip on AQ T1/T2
    //
    EPWM_setActionQualifierAction(aux_pwm_base,
                                  EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_T1_COUNT_UP);
    EPWM_setActionQualifierAction(aux_pwm_base,
                                  EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_T1_COUNT_DOWN);
    EPWM_setActionQualifierAction(aux_pwm_base,
                                  EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_T2_COUNT_UP);
    EPWM_setActionQualifierAction(aux_pwm_base,
                                  EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_T2_COUNT_DOWN);

    EPWM_setTripZoneAction(aux_pwm_base ,EPWM_TZ_ACTION_EVENT_DCAEVT1,
                           EPWM_TZ_ACTION_DISABLE);
    EPWM_setTripZoneAction(aux_pwm_base, EPWM_TZ_ACTION_EVENT_DCAEVT2,
                           EPWM_TZ_ACTION_DISABLE);
    EPWM_setTripZoneAction(aux_pwm_base, EPWM_TZ_ACTION_EVENT_DCBEVT1,
                           EPWM_TZ_ACTION_DISABLE);
    EPWM_setTripZoneAction(aux_pwm_base, EPWM_TZ_ACTION_EVENT_DCBEVT2,
                           EPWM_TZ_ACTION_DISABLE);
    EPWM_setTripZoneAction(aux_pwm_base, EPWM_TZ_ACTION_EVENT_TZA,
                           EPWM_TZ_ACTION_DISABLE);
    EPWM_setTripZoneAction(aux_pwm_base, EPWM_TZ_ACTION_EVENT_TZB,
                           EPWM_TZ_ACTION_DISABLE);
    EDIS;
}

//! \brief   Configures the delay trip action for inner switches(PWMxB).
//! \param base1   PWM base register for inner switches
//! \param aux_trip_source  Trip input for the auxiliary PWM output
//! \return None
void configEPWMDelayTrip(uint32_t base1,
                            EPWM_DigitalCompareTripInput aux_trip_source)
{
    //
    // Set force high actions for PWMxB during initialization
    //
    // Action When One-Time Software Force B is Invoked
    EPWM_setActionQualifierSWAction(base1, EPWM_AQ_OUTPUT_B,
                                    EPWM_AQ_OUTPUT_HIGH);
    // Initiates a single software forced event
    //
    EPWM_forceActionQualifierSWAction(base1,EPWM_AQ_OUTPUT_B);

    //
    // Configure EPWM DCBEVT1/2 as T1/2
    // DCBEVT1/2 generation based on aux PWM ouput set high and clear low
    //

    // DCBL = TRIPIN7 = INPUT X-BAR 3 = EPWM7A in this example
    EPWM_selectDigitalCompareTripInput(base1, aux_trip_source,
                                       EPWM_DC_TYPE_DCBL);

    //
    // DCBL = High, Trigger DCBEVT1 when auxiliary PWM goes high
    //
    EPWM_setTripZoneDigitalCompareEventCondition(base1, EPWM_TZ_DC_OUTPUT_B1,
                                                 EPWM_TZ_EVENT_DCXL_HIGH);

    //
    // DCBL = low, Trigger DCBEVT2 when auxiliary PWM goes low
    //
    EPWM_setTripZoneDigitalCompareEventCondition(base1, EPWM_TZ_DC_OUTPUT_B2,
                                                 EPWM_TZ_EVENT_DCXL_LOW);

    //
    // DCBEVT1/2 event as AQ T1/2
    // T1 = DCBEVT1
    // T2 = DCBEVT2
    //
    EPWM_setActionQualifierT1TriggerSource(base1,
                                           EPWM_AQ_TRIGGER_EVENT_TRIG_DCB_1);
    EPWM_setActionQualifierT2TriggerSource(base1,
                                           EPWM_AQ_TRIGGER_EVENT_TRIG_DCB_2);

    //
    // set T1/T2 action setting for EPWMxB
    //
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_T1_COUNT_UP);
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_T1_COUNT_DOWN);
    EPWM_setActionQualifierAction(base1, EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_T2_COUNT_UP);
    EPWM_setActionQualifierAction(base1,EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_T2_COUNT_DOWN);
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif // extern "C"

#endif // end of TYPE4_PWM_PROTECTION_SOURCE_H definition
