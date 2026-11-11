/*
 * lps25hb.h
 *
 * Kniznica pre barometricky snimac tlaku LPS25HB
 */

#ifndef LPS25HB_H_
#define LPS25HB_H_

#include <stdint.h>


#define LPS25HB_I2C_ADDRESS_SA0_HIGH	0xBA
#define LPS25HB_I2C_ADDRESS_SA0_LOW		0xB8


#define LPS25HB_WHO_AM_I_ADDR		0x0F
#define LPS25HB_WHO_AM_I_VALUE		0xBD


#define LPS25HB_RES_CONF_ADDR		0x10
#define LPS25HB_CTRL_REG1_ADDR		0x20
#define LPS25HB_CTRL_REG2_ADDR		0x21
#define LPS25HB_CTRL_REG3_ADDR		0x22
#define LPS25HB_CTRL_REG4_ADDR		0x23
#define LPS25HB_STATUS_REG_ADDR		0x27


#define LPS25HB_PRESS_OUT_XL_ADDR	0x28
#define LPS25HB_PRESS_OUT_L_ADDR	0x29
#define LPS25HB_PRESS_OUT_H_ADDR	0x2A
#define LPS25HB_TEMP_OUT_L_ADDR		0x2B
#define LPS25HB_TEMP_OUT_H_ADDR		0x2C
#define LPS25HB_AUTO_INCREMENT		0x80

#define LPS25HB_CTRL_REG1_PD		0x80
#define LPS25HB_CTRL_REG1_ODR_12_5HZ	0x30
#define LPS25HB_CTRL_REG1_BDU		0x04
#define LPS25HB_STATUS_T_DA			0x01
#define LPS25HB_STATUS_P_DA			0x02


typedef void (*lps25hb_read_cb_t)(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr);
typedef void (*lps25hb_write_cb_t)(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr);

#endif /* LPS25HB_H_ */
