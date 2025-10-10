#include "generic_definitions.h"
#include "protection.h"


/**
 * @brief Initializes the protection error monitoring structure with specified thresholds and limits.
 *
 * This function sets up the initial values for the error monitoring system. It configures the thresholds
 * for error growth and stagnation, as well as the acceptable error limits.
 *
 * @param monitor A pointer to the protection_error_monitor_t structure that holds the error monitoring data.
 * @param error_growth_threshold The threshold for consecutive error growth, measured in cycles.
 * @param stagnation_threshold The threshold for error stagnation, measured in cycles.
 * @param max_acceptable_difference The maximum acceptable percentage difference for the error.
 * @param max_acceptable_error The maximum acceptable error value, which could be in volts (Vrms) or amperes (Arms).
 */
void init_protection
(
        protection_error_monitor_t *monitor,
        int error_growth_threshold,
        int stagnation_threshold,
        float32_t max_acceptable_difference,
        float32_t max_acceptable_error,
        int pwm_max,
        int pwm_min
)
{
    monitor->data.event = 0;
    monitor->data.last_ctrl_error = 0.0f;
    monitor->data.consecutive_ctrl_error_growth_count = 0;
    monitor->data.stagnation_count = 0;
    monitor->config.ctrl_error_growth_threshold = error_growth_threshold;
    monitor->config.stagnation_threshold = stagnation_threshold;
    monitor->config.acceptable_percentage_diff = max_acceptable_difference;
    monitor->config.acceptable_error= max_acceptable_error;
    monitor->config.pwm_max_value = pwm_max;
    monitor->config.pwm_min_value = pwm_min;
}

// Monitors the error of the protection system
int _monitor_error(protection_error_monitor_t*monitor, float32_t ctrl_error, float32_t setpoint, float32_t out) {
    float32_t error = fabs(setpoint - out);

    // Track consecutive control error growth
    if (ctrl_error > monitor->data.last_ctrl_error)
        monitor->data.consecutive_ctrl_error_growth_count++;
    else
        monitor->data.consecutive_ctrl_error_growth_count = 0;

    monitor->data.last_ctrl_error = ctrl_error;

    // Track stagnation
    if( error > monitor->config.acceptable_error ) {

        float adjusted_setpoint = (setpoint < 1.0f) ? 1.0f : setpoint; // Avoid division by small setpoints
        float percentage_error = (error / adjusted_setpoint) * 100.0f;

        if (percentage_error > monitor->config.acceptable_percentage_diff)
            monitor->data.stagnation_count++;
        else
            monitor->data.stagnation_count = 0;
    }
    else {
        monitor->data.stagnation_count = 0;
        monitor->data.consecutive_ctrl_error_growth_count = 0;
    }

    // Check for problem conditions
    if (monitor->data.consecutive_ctrl_error_growth_count >= monitor->config.ctrl_error_growth_threshold)
    {
        monitor->data.event = monitor->data.event | 0x10;
        return -1; // Indicate problem
    }
    if (monitor->data.stagnation_count >= monitor->config.stagnation_threshold)
    {
        monitor->data.event = monitor->data.event | 0x20;
        return -2; // Indicate problem
    }
    else return 0;
}



int _sm_protection_handler(protection_error_monitor_t *monitor, float32_t ctrl_error_rms, float32_t setpoint_rms, float32_t out_rms) {

    int ret = 0;

    if (monitor->sm.initialized != INITIALIZED) {
        monitor->sm.initialized = (char *)INITIALIZED;
        monitor->sm.state = STATE_PROTECTION__INIT;
    }

    switch (monitor->sm.state) {
    case STATE_PROTECTION__INIT:
        monitor->data.last_ctrl_error = 0.0f;
        monitor->data.consecutive_ctrl_error_growth_count = 0;
        monitor->data.stagnation_count = 0;


        monitor->sm.state = STATE_PROTECTION__MONITOR;
        break;

    case STATE_PROTECTION__MONITOR:
        if (_monitor_error(monitor, ctrl_error_rms, setpoint_rms, out_rms) != 0)
        {
            monitor->sm.state = STATE_PROTECTION__ERROR_DETECTED;
            ret = -1;
        }
        else
            ret = 0;
        break;

    case STATE_PROTECTION__ERROR_DETECTED:
        // Handle problem detected state
        ret = -1;
        break;

    case STATE_PROTECTION__STAGNATION_DETECTED:
        // Handle problem detected state
        ret = -2;
        break;

    case STATE_PROTECTION__RESET:
        monitor->sm.state = STATE_PROTECTION__INIT;
        ret = 0;
        break;

    default:
        // Handle unknown state
        ret = -4;
        break;
    }
    return ret;
}

// use this after log the problem and before turn on the inverter
void reset_protection(protection_error_monitor_t *monitor)
{
//    monitor->data.event = 0;
    monitor->data.last_ctrl_error = 0.0f;
    monitor->data.consecutive_ctrl_error_growth_count = 0;
    monitor->data.stagnation_count = 0;
    monitor->sm.state = STATE_PROTECTION__RESET;
}

// to run in main loop
int process_protection(protection_error_monitor_t *monitor, float32_t ctrl_error_rms, float32_t setpoint_rms, float32_t out_rms)
{
    return _sm_protection_handler(monitor, ctrl_error_rms, setpoint_rms, out_rms);
}
