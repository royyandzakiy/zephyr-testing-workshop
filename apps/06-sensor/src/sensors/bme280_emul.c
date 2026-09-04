// src/sensors/bme280_emul.c
//
// An emulated BME280 hanging off the native_sim I2C emulation controller.
//
// Zephyr ships emulators for a handful of sensors (bmi160, bma4xx, f75303,
// akm09918c, ...) but NOT for the BME280, so this one is ours. It is modelled
// on drivers/sensor/f75303/f75303_emul.c, and the precedent for shipping an
// emulator next to the application rather than upstream is
// tests/drivers/sensor/ina230/src/ina230_emul.c.
//
// The point of the exercise: src/sensors/bme280.c is not modified, and neither
// is the Bosch driver. The devicetree decides whether the bus underneath is
// real or fake -- the same trick boards/native_sim_native.overlay already
// plays for the LED and button with gpio_emul, one layer further down.

#define DT_DRV_COMPAT bosch_bme280

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2c_emul.h>
#include <zephyr/logging/log.h>

#include "bme280_emul.h"

LOG_MODULE_REGISTER(bme280_emul, CONFIG_SENSOR_LOG_LEVEL);

/* Register map -- the subset the Bosch driver actually touches. Repeated here
 * rather than including the driver private bme280.h, which is not on the
 * application include path. */
#define REG_COMP_START      0x88  /* dig_T1..T3, dig_P1..P9, 24 bytes LE  */
#define REG_HUM_COMP_PART1  0xA1  /* dig_H1, 1 byte                       */
#define REG_ID              0xD0  /* must read back as CHIP_ID            */
#define REG_HUM_COMP_PART2  0xE1  /* dig_H2..H6, 7 packed bytes           */
#define REG_STATUS          0xF3
#define REG_PRESS_MSB       0xF7  /* 8-byte burst: P[3] T[3] H[2]         */

#define CHIP_ID             0x60  /* anything else and the driver returns
                                   * -ENOTSUP at init, and skips humidity */

#define NUM_REGS 256

struct bme280_emul_data {
	uint8_t reg[NUM_REGS];
};

struct bme280_emul_cfg {
};

/* Every board overlay in this app declares exactly one BME280, so a single
 * instance pointer is enough for the test-facing setter below. */
static struct bme280_emul_data *the_emul_data;

static void put_le16(uint8_t *dst, uint16_t v)
{
	dst[0] = v & 0xFF;
	dst[1] = v >> 8;
}

/*
 * Calibration blob.
 *
 * Reference values from the Bosch datasheet worked compensation example,
 * which is also where the default raw ADC codes come from. They have to be
 * self-consistent, not merely non-zero: the driver runs the real fixed-point
 * compensation over them, so an invented blob yields readings that are wildly
 * out of range. The range assertions in tests/emul are what catch that.
 */
static void load_calibration(struct bme280_emul_data *data)
{
	uint8_t *tp = &data->reg[REG_COMP_START];
	uint8_t *h;

	const int16_t dig_h2 = 362;
	const uint8_t dig_h3 = 0;
	const int16_t dig_h4 = 308;
	const int16_t dig_h5 = 50;
	const int8_t dig_h6 = 30;

	put_le16(tp + 0, 27504);             /* dig_T1, unsigned */
	put_le16(tp + 2, (uint16_t)26435);   /* dig_T2, signed   */
	put_le16(tp + 4, (uint16_t)-1000);   /* dig_T3           */

	put_le16(tp + 6, 36477);             /* dig_P1, unsigned */
	put_le16(tp + 8, (uint16_t)-10685);  /* dig_P2           */
	put_le16(tp + 10, (uint16_t)3024);   /* dig_P3           */
	put_le16(tp + 12, (uint16_t)2855);   /* dig_P4           */
	put_le16(tp + 14, (uint16_t)140);    /* dig_P5           */
	put_le16(tp + 16, (uint16_t)-7);     /* dig_P6           */
	put_le16(tp + 18, (uint16_t)15500);  /* dig_P7           */
	put_le16(tp + 20, (uint16_t)-14600); /* dig_P8           */
	put_le16(tp + 22, (uint16_t)6000);   /* dig_P9           */

	data->reg[REG_HUM_COMP_PART1] = 75;  /* dig_H1           */

	/* Humidity calibration is split across two ranges AND bit-packed. The
	 * driver unpacks it as:
	 *   dig_h2 = (h[1] << 8) | h[0]
	 *   dig_h3 =  h[2]
	 *   dig_h4 = (h[3] << 4) | (h[4] & 0x0F)
	 *   dig_h5 = ((h[4] >> 4) & 0x0F) | (h[5] << 4)
	 *   dig_h6 =  h[6]
	 * so h4 and h5 share the low and high nibbles of h[4]. Encoding those
	 * two backwards is the easiest mistake to make in this whole file.
	 */
	h = &data->reg[REG_HUM_COMP_PART2];
	put_le16(h + 0, (uint16_t)dig_h2);
	h[2] = dig_h3;
	h[3] = (dig_h4 >> 4) & 0xFF;
	h[4] = ((dig_h5 & 0x0F) << 4) | (dig_h4 & 0x0F);
	h[5] = (dig_h5 >> 4) & 0xFF;
	h[6] = (uint8_t)dig_h6;
}

void bme280_emul_set_raw(int32_t adc_temp, int32_t adc_press, int32_t adc_hum)
{
	uint8_t *b;

	if (the_emul_data == NULL) {
		return;
	}

	b = &the_emul_data->reg[REG_PRESS_MSB];

	/* The driver reassembles the 20-bit channels as
	 *   adc = (b[0] << 12) | (b[1] << 4) | (b[2] >> 4)
	 * and humidity as (b[6] << 8) | b[7]. */
	b[0] = (adc_press >> 12) & 0xFF;
	b[1] = (adc_press >> 4) & 0xFF;
	b[2] = (adc_press << 4) & 0xF0;

	b[3] = (adc_temp >> 12) & 0xFF;
	b[4] = (adc_temp >> 4) & 0xFF;
	b[5] = (adc_temp << 4) & 0xF0;

	b[6] = (adc_hum >> 8) & 0xFF;
	b[7] = adc_hum & 0xFF;
}

static void bme280_emul_reset(struct bme280_emul_data *data)
{
	memset(data->reg, 0, NUM_REGS);

	data->reg[REG_ID] = CHIP_ID;

	/* STATUS must read 0. The driver spins on the MEASURING and IM_UPDATE
	 * bits and gives up with -EAGAIN if they never clear, so leaving junk
	 * here makes every fetch fail with no hint as to why. */
	data->reg[REG_STATUS] = 0x00;

	load_calibration(data);
}

static int bme280_emul_transfer_i2c(const struct emul *target, struct i2c_msg *msgs,
				    int num_msgs, int addr)
{
	struct bme280_emul_data *data = target->data;
	int reg;

	__ASSERT_NO_MSG(msgs && num_msgs);

	i2c_dump_msgs_rw(target->dev, msgs, num_msgs, addr, false);

	switch (num_msgs) {
	case 1:
		/* A register write. i2c_reg_write_byte_dt() sends ONE message
		 * carrying [reg, value] -- not two. Handling only the
		 * two-message shape below is why a from-scratch emulator NAKs
		 * every write it is given. */
		if (msgs->flags & I2C_MSG_READ) {
			LOG_ERR("unexpected bare read");
			return -EIO;
		}
		if (msgs->len != 2) {
			LOG_ERR("unexpected write length %d", msgs->len);
			return -EIO;
		}
		data->reg[msgs->buf[0]] = msgs->buf[1];
		return 0;

	case 2:
		/* A burst read: write the start address, then read N bytes.
		 * i2c_burst_read_dt() asks for 24 at 0x88, 7 at 0xE1 and 8 at
		 * 0xF7, so this has to serve arbitrary lengths. */
		if (msgs->flags & I2C_MSG_READ) {
			LOG_ERR("unexpected read in msg0");
			return -EIO;
		}
		if (msgs->len != 1) {
			LOG_ERR("unexpected msg0 length %d", msgs->len);
			return -EIO;
		}
		reg = msgs->buf[0];

		msgs++;
		if (!(msgs->flags & I2C_MSG_READ)) {
			LOG_ERR("expected a read in msg1");
			return -EIO;
		}
		if (reg + (int)msgs->len > NUM_REGS) {
			LOG_ERR("read of %d bytes from 0x%02x runs off the register file",
				msgs->len, reg);
			return -EIO;
		}
		memcpy(msgs->buf, &data->reg[reg], msgs->len);
		return 0;

	default:
		LOG_ERR("unexpected message count %d", num_msgs);
		return -EIO;
	}
}

static int bme280_emul_init(const struct emul *target, const struct device *parent)
{
	struct bme280_emul_data *data = target->data;

	ARG_UNUSED(parent);

	the_emul_data = data;

	bme280_emul_reset(data);
	bme280_emul_set_raw(BME280_EMUL_ADC_TEMP_DEFAULT,
			    BME280_EMUL_ADC_PRESS_DEFAULT,
			    BME280_EMUL_ADC_HUM_DEFAULT);

	return 0;
}

static const struct i2c_emul_api bme280_emul_api_i2c = {
	.transfer = bme280_emul_transfer_i2c,
};

/* The node already has a DEVICE_DT_DEFINE from the real Bosch driver
 * (CONFIG_BME280=y), so no EMUL_STUB_DEVICE is needed. The trailing NULL is
 * the optional emul_sensor_driver_api backend -- see bme280_emul.h for why we
 * do not implement it. */
#define BME280_EMUL(n)                                                        \
	static const struct bme280_emul_cfg bme280_emul_cfg_##n;              \
	static struct bme280_emul_data bme280_emul_data_##n;                  \
	EMUL_DT_INST_DEFINE(n, bme280_emul_init, &bme280_emul_data_##n,       \
			    &bme280_emul_cfg_##n, &bme280_emul_api_i2c, NULL);

DT_INST_FOREACH_STATUS_OKAY(BME280_EMUL)
