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

/* kalibracne data (registre 0x30 - 0x3F) a z nich vypocitane hodnoty pre teplotu */
static uint8_t calib[16];
static float T0_degC, T1_degC;
static int16_t T0_out, T1_out;
static float H0_rH, H1_rH;
static int16_t H0_T0_out, H1_T0_out;

static void hts221_calc_temp_calibration(void)
{
	// T0 a T1 maju 10 bitov, horne 2 bity su v registri 0x35, hodnoty su nasobene 8
	uint16_t T0_degC_x8 = calib[0x02] | ((calib[0x05] & 0x03) << 8);
	uint16_t T1_degC_x8 = calib[0x03] | ((calib[0x05] & 0x0C) << 6);

	T0_degC = T0_degC_x8 / 8.0f;
	T1_degC = T1_degC_x8 / 8.0f;

	T0_out = (int16_t)((calib[0x0D] << 8) | calib[0x0C]);
	T1_out = (int16_t)((calib[0x0F] << 8) | calib[0x0E]);
}
static void hts221_calc_hum_calibration(void)
{
	// hodnoty vlhkosti v kalibracnych registroch su nasobene 2
	H0_rH = calib[0x00] / 2.0f;
	H1_rH = calib[0x01] / 2.0f;

	H0_T0_out = (int16_t)((calib[0x07] << 8) | calib[0x06]);
	H1_T0_out = (int16_t)((calib[0x0B] << 8) | calib[0x0A]);
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
	// nacitanie kalibracnych registrov naraz (multi-byte citanie)
	hts221_read_reg(calib, 16, HTS221_H0_RH_X2_ADDR);
	hts221_calc_temp_calibration();
	hts221_calc_hum_calibration();

	return 1;
}
// vrati teplotu v stupnoch Celzia
float hts221_get_temperature(void)
{
	uint8_t data[2];

	hts221_read_reg(data, 2, HTS221_TEMP_OUT_L_ADDR);
	int16_t t_out = (int16_t)((data[1] << 8) | data[0]);

	// linearna interpolacia medzi kalibracnymi bodmi
	return T0_degC + (float)(t_out - T0_out) * (T1_degC - T0_degC) / (float)(T1_out - T0_out);
}

// vrati relativnu vlhkost v percentach
float hts221_get_humidity(void)
{
	uint8_t data[2];

	hts221_read_reg(data, 2, HTS221_HUMIDITY_OUT_L_ADDR);
	int16_t h_out = (int16_t)((data[1] << 8) | data[0]);

	// linearna interpolacia medzi kalibracnymi bodmi
	float humidity = H0_rH + (float)(h_out - H0_T0_out) * (H1_rH - H0_rH) / (float)(H1_T0_out - H0_T0_out);

	// datasheet: hodnoty mimo rozsahu treba orezat softverovo
	if(humidity < 0.0f)
	{
		humidity = 0.0f;
	}
	if(humidity > 100.0f)
	{
		humidity = 100.0f;
	}

	return humidity;
}
