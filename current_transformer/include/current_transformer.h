

#ifndef CURRENT_TRANSFORMER_H
#define CURRENT_TRANSFORMER_H

#include "driver/adc.h"
#include "esp_adc_cal.h"


/**
 * @brief Initialize the ADC.
 */
void init_adc();

/**
 * @brief Read the voltage from an ADC input.
 *
 * @param channel The ADC channel to read from.
 * @return The voltage measurement in volts.
 */
float read_voltage(adc_channel_t channel);

/**
 * @brief Read the current from an ADC input.
 *
 * @param channel The ADC channel to read from.
 * @return The current measurement in amperes.
 */
float read_current(adc_channel_t channel);

/**
 * @brief Calculate the power, reactive power, and real power from current and voltage measurements.
 *
 * @param current The current measurement in amperes.
 * @param voltage The voltage measurement in volts.
 * @return An array containing the power, reactive power, and real power in watts.
 */
float* calculate_power(float current, float voltage);

#endif /* CURRENT_TRANSFORMER_H */
