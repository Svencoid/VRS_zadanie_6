/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* maximalny pocet iteracii. */
#define I2C_TIMEOUT_LOOPS   200000UL

/* Najviac bajtov v jednej transakcii (NBYTES je 8-bitové, pri zápise sa
 * k dátam pridáva ešte adresa registra). */
#define I2C_MAX_READ_LEN    255U
#define I2C_MAX_WRITE_LEN   254U

/**
 * Reset I2C periferie po chybe
 * Vypnutie PE bitu vynuluje stavový automat*/
static void i2c_abort(void)
{
  LL_I2C_Disable(I2C1);
  /* PE musi zostat v 0 aspon 3 hodinove cykly APB */
  for (volatile uint8_t i = 0; i < 8U; i++)
  {
    __NOP();
  }
  LL_I2C_Enable(I2C1);
}

/**
 * cakanie na splnenie podmienky s timeoutom. Pri NACK
 * sa I2C perifera resetuje a volajuca funkcia skoncí s I2C_ERROR.
 */
#define I2C_WAIT_FOR(cond)                                          \
  do {                                                              \
    uint32_t _timeout = I2C_TIMEOUT_LOOPS;                          \
    while (!(cond))                                                 \
    {                                                               \
      if (LL_I2C_IsActiveFlag_NACK(I2C1) || (--_timeout == 0U))     \
      {                                                             \
        i2c_abort();                                                \
        return I2C_ERROR;                                           \
      }                                                             \
    }                                                               \
  } while (0)

/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

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
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

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
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/* USER CODE BEGIN 1 */

uint8_t i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
  if ((data == 0) || (len == 0U))
  {
    return I2C_ERROR;
  }

  /* zbernica musi byt volna */
  I2C_WAIT_FOR(!LL_I2C_IsActiveFlag_BUSY(I2C1));

  /* 1. fáza: START + adresa slave (zápis) + adresa registra.
   * SOFTEND = po odvysielaní bajtu sa nevygeneruje STOP, ale čaká sa na
   * príznak TC, aby sa dal poslať "repeated start". */
  LL_I2C_HandleTransfer(I2C1, (uint32_t)slave_addr << 1, LL_I2C_ADDRSLAVE_7BIT,
                        1, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

  I2C_WAIT_FOR(LL_I2C_IsActiveFlag_TXIS(I2C1));
  LL_I2C_TransmitData8(I2C1, reg_addr);
  I2C_WAIT_FOR(LL_I2C_IsActiveFlag_TC(I2C1));

  /* 2. fáza: repeated START + adresa slave (čítanie) + prijatie len bajtov.
   * AUTOEND = po prijatí posledného bajtu sa automaticky vygeneruje STOP. */
  LL_I2C_HandleTransfer(I2C1, (uint32_t)slave_addr << 1, LL_I2C_ADDRSLAVE_7BIT,
                        len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

  for (uint8_t i = 0; i < len; i++)
  {
    I2C_WAIT_FOR(LL_I2C_IsActiveFlag_RXNE(I2C1));
    data[i] = LL_I2C_ReceiveData8(I2C1);
  }

  I2C_WAIT_FOR(LL_I2C_IsActiveFlag_STOP(I2C1));
  LL_I2C_ClearFlag_STOP(I2C1);

  return I2C_OK;
}

uint8_t i2c_write(uint8_t slave_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len)
{
  if ((data == 0) || (len == 0U) || (len > I2C_MAX_WRITE_LEN))
  {
    return I2C_ERROR;
  }

  I2C_WAIT_FOR(!LL_I2C_IsActiveFlag_BUSY(I2C1));

  /* Jedna transakcia: START + adresa + adresa registra + len dátových bajtov + STOP */
  LL_I2C_HandleTransfer(I2C1, (uint32_t)slave_addr << 1, LL_I2C_ADDRSLAVE_7BIT,
                        (uint32_t)len + 1U, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

  I2C_WAIT_FOR(LL_I2C_IsActiveFlag_TXIS(I2C1));
  LL_I2C_TransmitData8(I2C1, reg_addr);

  for (uint8_t i = 0; i < len; i++)
  {
    I2C_WAIT_FOR(LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, data[i]);
  }

  I2C_WAIT_FOR(LL_I2C_IsActiveFlag_STOP(I2C1));
  LL_I2C_ClearFlag_STOP(I2C1);

  return I2C_OK;
}

uint8_t i2c_read_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data)
{
  return i2c_read(slave_addr, reg_addr, data, 1U);
}

uint8_t i2c_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
  return i2c_write(slave_addr, reg_addr, &data, 1U);
}

/* USER CODE END 1 */

