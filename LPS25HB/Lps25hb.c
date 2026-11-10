/**
 ******************************************************************************
 * @file    lps25hb.c
 * @brief   Driver barometrického senzora LPS25HB
 ******************************************************************************
 */
#include "lps25hb.h"
#include <math.h>

/* Callbacky pre prístup k I2C */
static lps25hb_read_cb_t  i2c_read_cb  = 0;
static lps25hb_write_cb_t i2c_write_cb = 0;

/* ---------------------------------------------------------------------------
 * Interné funkcie pre čítanie/zápis registrov
 * ------------------------------------------------------------------------ */

/* Čítanie registrov. Pri viac ako jednom bajte sa zapne auto-inkrement adresy. */
static uint8_t read_regs(uint8_t reg_addr, uint8_t *data, uint8_t len)
{
  if (i2c_read_cb == 0)
  {
    return LPS25HB_ERROR;
  }

  if (len > 1U)
  {
    reg_addr |= LPS25HB_AUTO_INCREMENT;
  }

  return (i2c_read_cb(LPS25HB_I2C_ADDR, reg_addr, data, len) == 0U) ? LPS25HB_OK : LPS25HB_ERROR;
}

/* Zápis registrov. Pri viac ako jednom bajte sa zapne auto-inkrement adresy. */
static uint8_t write_regs(uint8_t reg_addr, const uint8_t *data, uint8_t len)
{
  if (i2c_write_cb == 0)
  {
    return LPS25HB_ERROR;
  }

  if (len > 1U)
  {
    reg_addr |= LPS25HB_AUTO_INCREMENT;
  }

  return (i2c_write_cb(LPS25HB_I2C_ADDR, reg_addr, data, len) == 0U) ? LPS25HB_OK : LPS25HB_ERROR;
}

/* ---------------------------------------------------------------------------
 * Verejné funkcie
 * ------------------------------------------------------------------------ */

void lps25hb_register_callbacks(lps25hb_read_cb_t read_cb, lps25hb_write_cb_t write_cb)
{
  i2c_read_cb  = read_cb;
  i2c_write_cb = write_cb;
}

uint8_t lps25hb_init(void)
{
  uint8_t who_am_i = 0;
  uint8_t ctrl1;

  /* 1. Overenie pripojenia senzora */
  if (read_regs(LPS25HB_WHO_AM_I_REG, &who_am_i, 1U) != LPS25HB_OK)
  {
    return LPS25HB_ERROR;
  }
  if (who_am_i != LPS25HB_WHO_AM_I_VALUE)
  {
    return LPS25HB_ERROR_ID;
  }

  /* 2. Konfigurácia: aktívny režim, výstupná frekvencia 12.5 Hz,
   *    BDU (výstupné registre sa neaktualizujú, kým sa neprečítajú všetky bajty hodnoty) */
  ctrl1 = LPS25HB_CTRL1_PD_ACTIVE | LPS25HB_CTRL1_ODR_12_5HZ | LPS25HB_CTRL1_BDU;
  if (write_regs(LPS25HB_CTRL_REG1, &ctrl1, 1U) != LPS25HB_OK)
  {
    return LPS25HB_ERROR;
  }

  return LPS25HB_OK;
}

uint8_t lps25hb_read_pressure(int32_t *pressure_x100)
{
  uint8_t raw_bytes[3];

  if (pressure_x100 == 0)
  {
    return LPS25HB_ERROR;
  }

  /* PRESS_OUT_XL, PRESS_OUT_L a PRESS_OUT_H sa čítajú naraz (multi-byte) */
  if (read_regs(LPS25HB_PRESS_OUT_XL, raw_bytes, 3U) != LPS25HB_OK)
  {
    return LPS25HB_ERROR;
  }

  /* 24-bitová hodnota v doplnkovom kóde */
  int32_t raw = (int32_t)(((uint32_t)raw_bytes[2] << 16) |
                          ((uint32_t)raw_bytes[1] << 8)  |
                           (uint32_t)raw_bytes[0]);
  if ((raw & 0x00800000) != 0)
  {
    raw -= 0x01000000;
  }

  /* tlak [hPa] = raw / 4096, v stotinách hPa: raw * 100 / 4096 = raw * 25 / 1024 */
  *pressure_x100 = (raw * 25) / 1024;
  return LPS25HB_OK;
}

int32_t lps25hb_calc_relative_altitude_cm(int32_t pressure_x100, int32_t ref_pressure_x100)
{
  if ((pressure_x100 <= 0) || (ref_pressure_x100 <= 0))
  {
    return 0;
  }

  /* Barometrická rovnica: h = 44330 * (1 - (p / p0)^(1/5.255))   [m]
   * Ako p0 sa použije referenčný tlak na zemi, výsledkom je relatívna výška. */
  float ratio  = (float)pressure_x100 / (float)ref_pressure_x100;
  float height = 44330.0f * (1.0f - powf(ratio, 0.190295f));

  float height_cm = height * 100.0f;
  return (int32_t)((height_cm >= 0.0f) ? (height_cm + 0.5f) : (height_cm - 0.5f));
}
