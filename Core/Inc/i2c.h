/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* Návratové hodnoty funkcií I2C komunikácie */
#define I2C_OK      0U
#define I2C_ERROR   1U

/* USER CODE END Private defines */

void MX_I2C1_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * citanie z registra slave zariadenia.
 * len == 1  -> single byte čítanie
 * len  > 1  -> multi byte čítanie*
 * @param slave_addr
 * @param reg_addr
 *
 */
uint8_t i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

/**
 * Zápis do registra slave zariadenia.
 * len == 1  -> single byte zápis
 * len  > 1  -> multi byte zápis v jednej transakcii
 */
uint8_t i2c_write(uint8_t slave_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len);

/* Pohodlné obalové funkcie pre single byte prenos */
uint8_t i2c_read_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data);
uint8_t i2c_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

