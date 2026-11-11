/*
 * lps25hb.c
 *
 * Kniznica pre barometricky snimac tlaku LPS25HB
 */

#include "lps25hb.h"
#include <math.h>

static lps25hb_read_cb_t i2c_read = 0;
static lps25hb_write_cb_t i2c_write = 0;

static uint8_t device_address = LPS25HB_I2C_ADDRESS_SA0_HIGH;

void lps25hb_register_callbacks(lps25hb_read_cb_t read_cb, lps25hb_write_cb_t write_cb)
{
	i2c_read = read_cb;
	i2c_write = write_cb;
}

void lps25hb_read_reg(uint8_t *data, uint8_t length, uint8_t reg)
{
	if(length > 1)
	{
		reg |= LPS25HB_AUTO_INCREMENT;
	}
	i2c_read(data, length, reg, device_address);
}

void lps25hb_write_reg(uint8_t *data, uint8_t length, uint8_t reg)
{
	if(length > 1)
	{
		reg |= LPS25HB_AUTO_INCREMENT;
	}
	i2c_write(data, length, reg, device_address);
}

uint8_t lps25hb_init(void)
{
	uint8_t who_am_i = 0;
	uint8_t config;

	if(i2c_read == 0 || i2c_write == 0)
	{
		return 0;
	}

	device_address = LPS25HB_I2C_ADDRESS_SA0_HIGH;
	lps25hb_read_reg(&who_am_i, 1, LPS25HB_WHO_AM_I_ADDR);

	if(who_am_i != LPS25HB_WHO_AM_I_VALUE)
	{
		device_address = LPS25HB_I2C_ADDRESS_SA0_LOW;
		who_am_i = 0;
		lps25hb_read_reg(&who_am_i, 1, LPS25HB_WHO_AM_I_ADDR);

		if(who_am_i != LPS25HB_WHO_AM_I_VALUE)
		{
			return 0;
		}
	}

	config = LPS25HB_CTRL_REG1_PD | LPS25HB_CTRL_REG1_ODR_12_5HZ | LPS25HB_CTRL_REG1_BDU;
	lps25hb_write_reg(&config, 1, LPS25HB_CTRL_REG1_ADDR);

	return 1;
}

float lps25hb_get_pressure(void)
{
	uint8_t data[3];

	lps25hb_read_reg(data, 3, LPS25HB_PRESS_OUT_XL_ADDR);

	int32_t raw = ((int32_t)data[2] << 16) | ((int32_t)data[1] << 8) | data[0];

	if(raw & 0x800000)
	{
		raw -= 0x1000000;
	}

	return raw / 4096.0f;
}

float lps25hb_calc_altitude(float pressure_hpa, float reference_hpa)
{
	if(reference_hpa <= 0.0f)
	{
		return 0.0f;
	}

	return 44330.0f * (1.0f - powf(pressure_hpa / reference_hpa, 1.0f / 5.255f));
}
