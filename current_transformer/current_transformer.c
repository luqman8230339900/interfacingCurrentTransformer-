#include <stdio.h>

#include "esp_adc_cal.h"
#include "driver/adc.h"
#include <math.h>

#define DEFAULT_VREF    1100        // Default ADC reference voltage in mV
#define NO_OF_SAMPLES   64          // Number of samples for averaging

static esp_adc_cal_characteristics_t *adc_chars;    // ADC characteristics structure
static const adc_channel_t channel = ADC_CHANNEL_6; // GPIO 34, corresponding to ADC1_CH6
static const adc_atten_t atten = ADC_ATTEN_DB_11;   // 11dB attenuation, for a voltage range of 0-3.6V
static const adc_unit_t unit = ADC_UNIT_1;          // ADC unit 1, corresponding to ADC1


/**
 * @brief Initialize the ADC with the specified attenuation and number of samples for averaging.
 *
 * @param attenuation The ADC attenuation to use.
 * @param num_samples The number of samples to use for averaging.
 */
static void init_adc(adc_atten_t attenuation, int num_samples) {
    // Configure the ADC
    adc1_config_width(ADC_WIDTH_BIT_12);                      // 12-bit resolution
    adc1_config_channel_atten(channel, attenuation);          // Set the attenuation
    adc1_config_channel_atten(ADC_CHANNEL_7, attenuation);    // Configure the second channel as well (needed for AC current measurements)

    // Configure the ADC characteristics
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, attenuation, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    // Configure the ADC for continuous mode
    adc1_continuous_mode_enable(num_samples);
}


/**
 * @brief Read a single sample from the ADC using oneshot mode.
 *
 * @return The raw ADC reading.
 */
static int read_oneshot() {
    // Perform a single ADC conversion and return the raw value
    int raw = adc1_get_raw(channel);
    return raw;
}


/**
 * @brief Read continuously from the ADC using continuous mode.
 *
 * @param buffer The buffer to store the ADC readings in.
 * @param length The length of the buffer.
 */
static void read_continuous(int *buffer, int length) {
    // Start the ADC in continuous mode
    adc1_continuous_start(channel, atten);

    // Read the ADC continuously and store the values in the buffer
    for (int i = 0; i < length; i++) {
        buffer[i] = adc1_get_raw(channel);
    }

    // Stop the ADC in continuous mode
    adc1_continuous_stop();
}

/**
 * @brief Filter out noise from the ADC readings by averaging multiple samples.
 *
 * @param buffer The buffer of raw ADC readings.
 * @param length The length of the buffer.
 * @return The average of the ADC readings.
 */
static int filter_noise(int *buffer, int length) {
    // Calculate the average of the ADC readings
    int sum = 0;
    for (int i = 0; i < length; i++) {
        sum += buffer[i];
    }
    int average = sum / length;

    // Return the average value
    return average;
}


/**
 * @brief Convert an ADC reading to a voltage value.
 *
 * @param raw The raw ADC reading.
 * @return The voltage value in millivolts.
 */
static int convert_to_voltage(int raw) {
    // Convert the raw ADC reading to a voltage value using the ADC characteristics structure
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, adc_chars);

    // Return the voltage value in millivolts
    return voltage;
}


/**
 * @brief Read the current from an ADC input.
 *
 * @param channel The ADC channel to read from.
 * @return The current measurement in amperes.
 */
static float read_current(adc_channel_t channel) {
    // Read the raw ADC measurement
    int raw = adc1_get_raw(channel);

    // Convert the raw ADC measurement to a current value in amperes
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    float voltage = (float)raw / ((float)4095 / DEFAULT_VREF);
    float current = voltage / 100.0;    // Assuming a 100-ohm burden resistor

    return current;
}



/**
 * @brief Calculate the power, reactive power, and real power from current and voltage measurements.
 *
 * @param current The current measurement in amperes.
 * @param voltage The voltage measurement in volts.
 * @return An array containing the power, reactive power, and real power in watts.
 */
static float* calculate_power(float current, float voltage) {
    // Calculate the power factor and phase angle
    float pf = cos(0);    // Assuming a purely resistive load
    float theta = acos(pf);

    // Calculate the power, reactive power, and real power
    float p = current * voltage * pf;
    float q = current * voltage * sin(theta);
    float s = current * voltage;

    // Create an array to hold the power values
    static float power[3];
    power[0] = p;
    power[1] = q;
    power[2] = s;

    return power;
}


