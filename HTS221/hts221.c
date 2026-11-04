/*
 * hts221.c
 *
 * Kniznica pre snimac vlhkosti a teploty HTS221
 */

#include "hts221.h"

/* callbacky pre i2c, nastavia sa cez hts221_register_callbacks */
static hts221_read_cb_t i2c_read = 0;
static hts221_write_cb_t i2c_write = 0;

void hts221_register_callbacks(hts221_read_cb_t read_cb, hts221_write_cb_t write_cb)
{
	i2c_read = read_cb;
	i2c_write = write_cb;
}

void hts221_read_reg(uint8_t *data, uint8_t length, uint8_t reg)
{
	// pri viacbajtovom citani sa nastavi MSB adresy (auto-inkrement)
	if(length > 1)
	{
		reg |= HTS221_AUTO_INCREMENT;
	}
	i2c_read(data, length, reg, HTS221_I2C_ADDRESS);
}

void hts221_write_reg(uint8_t *data, uint8_t length, uint8_t reg)
{
	if(length > 1)
	{
		reg |= HTS221_AUTO_INCREMENT;
	}
	i2c_write(data, length, reg, HTS221_I2C_ADDRESS);
}

// vrati 1 ak je senzor pripojeny a nakonfigurovany, inak 0
uint8_t hts221_init(void)
{
	uint8_t who_am_i = 0;
	uint8_t config;

	if(i2c_read == 0 || i2c_write == 0)
	{
		return 0;
	}

	// overenie pripojenia senzora
	hts221_read_reg(&who_am_i, 1, HTS221_WHO_AM_I_ADDR);
	if(who_am_i != HTS221_WHO_AM_I_VALUE)
	{
		return 0;
	}

	// zapnutie senzora, block data update, frekvencia merania 12.5 Hz
	config = HTS221_CTRL_REG1_PD | HTS221_CTRL_REG1_BDU | HTS221_CTRL_REG1_ODR_12_5HZ;
	hts221_write_reg(&config, 1, HTS221_CTRL_REG1_ADDR);

	return 1;
}
