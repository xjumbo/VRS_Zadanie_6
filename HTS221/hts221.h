/*
 * hts221.h
 *
 * Kniznica pre snimac vlhkosti a teploty HTS221
 */

#ifndef HTS221_H_
#define HTS221_H_

#include <stdint.h>

/* I2C adresa (8-bitova, 7-bitova adresa 0x5F posunuta o 1 bit) */
#define HTS221_I2C_ADDRESS			0xBE

/* WHO_AM_I */
#define HTS221_WHO_AM_I_ADDR		0x0F
#define HTS221_WHO_AM_I_VALUE		0xBC

/* konfiguracne registre */
#define HTS221_AV_CONF_ADDR			0x10
#define HTS221_CTRL_REG1_ADDR		0x20
#define HTS221_CTRL_REG2_ADDR		0x21
#define HTS221_CTRL_REG3_ADDR		0x22
#define HTS221_STATUS_REG_ADDR		0x27

/* vystupne registre */
#define HTS221_HUMIDITY_OUT_L_ADDR	0x28
#define HTS221_HUMIDITY_OUT_H_ADDR	0x29
#define HTS221_TEMP_OUT_L_ADDR		0x2A
#define HTS221_TEMP_OUT_H_ADDR		0x2B

/* kalibracne registre */
#define HTS221_H0_RH_X2_ADDR		0x30
#define HTS221_H1_RH_X2_ADDR		0x31
#define HTS221_T0_DEGC_X8_ADDR		0x32
#define HTS221_T1_DEGC_X8_ADDR		0x33
#define HTS221_T1_T0_MSB_ADDR		0x35
#define HTS221_H0_T0_OUT_L_ADDR		0x36
#define HTS221_H1_T0_OUT_L_ADDR		0x3A
#define HTS221_T0_OUT_L_ADDR		0x3C
#define HTS221_T1_OUT_L_ADDR		0x3E

/* viacbajtovy prenos: nastaveny MSB v adrese registra (auto-inkrement) */
#define HTS221_AUTO_INCREMENT		0x80

/* hodnoty a bity registrov */
#define HTS221_CTRL_REG1_PD			0x80	/* zapnutie senzora */
#define HTS221_CTRL_REG1_BDU		0x04	/* block data update */
#define HTS221_CTRL_REG1_ODR_12_5HZ	0x03	/* frekvencia merania 12.5 Hz */
#define HTS221_STATUS_T_DA			0x01	/* nove data teploty */
#define HTS221_STATUS_H_DA			0x02	/* nove data vlhkosti */

/* callback funkcie pre i2c komunikaciu (registruju sa z main) */
typedef void (*hts221_read_cb_t)(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr);
typedef void (*hts221_write_cb_t)(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr);

/* verejne funkcie */
void hts221_register_callbacks(hts221_read_cb_t read_cb, hts221_write_cb_t write_cb);
uint8_t hts221_init(void);
void hts221_read_reg(uint8_t *data, uint8_t length, uint8_t reg);
void hts221_write_reg(uint8_t *data, uint8_t length, uint8_t reg);

#endif /* HTS221_H_ */
