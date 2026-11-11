/**
  ******************************************************************************
  * File Name          : I2C.c
  * Description        : This file provides code for the configuration
  *                      of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
static uint8_t *i2c_rx_buf;
static volatile uint8_t i2c_rx_idx;
/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void)
{
  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 interrupt Init */
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(I2C1_EV_IRQn);

  /** I2C Initialization
  */
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x2000090E;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 2;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);

}

/* USER CODE BEGIN 1 */
#define I2C_TIMEOUT		100000

static uint8_t i2c_wait(uint32_t (*flag)(I2C_TypeDef *))
{
	uint32_t timeout = I2C_TIMEOUT;

	while(!flag(I2C1))
	{
		if(LL_I2C_IsActiveFlag_NACK(I2C1) || (timeout-- == 0))
		{
			return 1;
		}
	}
	return 0;
}

static void i2c_abort(void)
{
	LL_I2C_DisableIT_RX(I2C1);
	LL_I2C_Disable(I2C1);
	LL_I2C_Enable(I2C1);
}

void i2c_master_read(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr)
{

	for(uint8_t i = 0; i < length; i++)
	{
		data[i] = 0;
	}

	i2c_rx_buf = data;
	i2c_rx_idx = 0;
	LL_I2C_EnableIT_RX(I2C1);

	LL_I2C_HandleTransfer(I2C1, slave_addr, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
	if(i2c_wait(LL_I2C_IsActiveFlag_TXIS))
	{
		i2c_abort();
		return;
	}
	LL_I2C_TransmitData8(I2C1, reg);
	if(i2c_wait(LL_I2C_IsActiveFlag_TC))
	{
		i2c_abort();
		return;
	}

	LL_I2C_HandleTransfer(I2C1, slave_addr, LL_I2C_ADDRSLAVE_7BIT, length, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);
	if(i2c_wait(LL_I2C_IsActiveFlag_STOP))
	{
		i2c_abort();
		return;
	}

	LL_I2C_ClearFlag_STOP(I2C1);
	LL_I2C_DisableIT_RX(I2C1);
}

void i2c_master_write(uint8_t *data, uint8_t length, uint8_t reg, uint8_t slave_addr)
{
	LL_I2C_HandleTransfer(I2C1, slave_addr, LL_I2C_ADDRSLAVE_7BIT, length + 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
	if(i2c_wait(LL_I2C_IsActiveFlag_TXIS))
	{
		i2c_abort();
		return;
	}
	LL_I2C_TransmitData8(I2C1, reg);

	for(uint8_t i = 0; i < length; i++)
	{
		if(i2c_wait(LL_I2C_IsActiveFlag_TXIS))
		{
			i2c_abort();
			return;
		}
		LL_I2C_TransmitData8(I2C1, data[i]);
	}

	if(i2c_wait(LL_I2C_IsActiveFlag_STOP))
	{
		i2c_abort();
		return;
	}
	LL_I2C_ClearFlag_STOP(I2C1);
}

void i2c_rx_irq_handler(void)
{
	if(LL_I2C_IsActiveFlag_RXNE(I2C1))
	{
		i2c_rx_buf[i2c_rx_idx++] = LL_I2C_ReceiveData8(I2C1);
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
