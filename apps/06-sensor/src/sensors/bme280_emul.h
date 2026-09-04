// src/sensors/bme280_emul.h
//
// Test-facing controls for the emulated BME280. Only built when CONFIG_EMUL
// is on, which in this app means native_sim.

#ifndef BME280_EMUL_H_
#define BME280_EMUL_H_

#include <stdint.h>

/* Raw ADC codes from the Bosch datasheet worked compensation example. Paired
 * with the calibration blob in bme280_emul.c they land around 25 degC and
 * 100 kPa -- plausible indoor values, which is all the app needs to look
 * alive on a laptop. */
#define BME280_EMUL_ADC_TEMP_DEFAULT   519888
#define BME280_EMUL_ADC_PRESS_DEFAULT  415148
#define BME280_EMUL_ADC_HUM_DEFAULT    31500

/**
 * Set the raw ADC codes the emulated chip will report.
 *
 * Raw codes, not engineering units. The real Bosch driver runs the datasheet
 * compensation math over the calibration blob, and inverting that so a test
 * could ask for "25.5 degC" would mean reimplementing that math here -- at
 * which point the test would be checking our arithmetic against itself.
 * Feeding raw codes keeps the driver as the only thing under test.
 *
 * Temperature and pressure are 20-bit, humidity is 16-bit.
 */
void bme280_emul_set_raw(int32_t adc_temp, int32_t adc_press, int32_t adc_hum);

#endif /* BME280_EMUL_H_ */
