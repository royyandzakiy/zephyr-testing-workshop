// Test-facing controls for the emulated BME280. Built only with CONFIG_EMUL
// (native_sim).

#ifndef BME280_EMUL_H_
#define BME280_EMUL_H_

#include <stdint.h>

/* Raw ADC codes from the Bosch datasheet worked compensation example. With the
 * calibration blob in bme280_emul.c they read about 25 degC and 100 kPa. */
#define BME280_EMUL_ADC_TEMP_DEFAULT   519888
#define BME280_EMUL_ADC_PRESS_DEFAULT  415148
#define BME280_EMUL_ADC_HUM_DEFAULT    31500

/**
 * Set the raw ADC codes the emulated chip will report.
 *
 * Raw codes, not engineering units. Accepting "25.5 degC" would mean
 * reimplementing the driver's compensation math here, and the test would then
 * check that math against itself. Raw codes keep the driver under test.
 *
 * Temperature and pressure are 20-bit, humidity is 16-bit.
 */
void bme280_emul_set_raw(int32_t adc_temp, int32_t adc_press, int32_t adc_hum);

#endif /* BME280_EMUL_H_ */
