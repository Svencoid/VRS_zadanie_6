/**
 ******************************************************************************
 * @file    hts221.c
 * @brief   Driver senzora vlhkosti a teploty HTS221
 ******************************************************************************
 */
#include "hts221.h"

/* Callbacky pre prístup k I2C */
static hts221_read_cb_t  i2c_read_cb  = 0;
static hts221_write_cb_t i2c_write_cb = 0;

/* Kalibračné koeficienty vyčítané zo senzora pri inicializácii */
typedef struct
{
  float   t0_degc;
  float   t1_degc;
  float   h0_rh;
  float   h1_rh;
  int16_t t0_out;
  int16_t t1_out;
  int16_t h0_t0_out;
  int16_t h1_t0_out;
} hts221_calib_t;

static hts221_calib_t calib;

/* ---------------------------------------------------------------------------
 * Interné funkcie pre čítanie/zápis registrov
 * ------------------------------------------------------------------------ */

/* Čítanie registrov. Pri viac ako jednom bajte sa zapne auto-inkrement adresy. */
static uint8_t read_regs(uint8_t reg_addr, uint8_t *data, uint8_t len)
{
  if (i2c_read_cb == 0)
  {
    return HTS221_ERROR;
  }

  if (len > 1U)
  {
    reg_addr |= HTS221_AUTO_INCREMENT;
  }

  return (i2c_read_cb(HTS221_I2C_ADDR, reg_addr, data, len) == 0U) ? HTS221_OK : HTS221_ERROR;
}

/* Zápis registrov. Pri viac ako jednom bajte sa zapne auto-inkrement adresy. */
static uint8_t write_regs(uint8_t reg_addr, const uint8_t *data, uint8_t len)
{
  if (i2c_write_cb == 0)
  {
    return HTS221_ERROR;
  }

  if (len > 1U)
  {
    reg_addr |= HTS221_AUTO_INCREMENT;
  }

  return (i2c_write_cb(HTS221_I2C_ADDR, reg_addr, data, len) == 0U) ? HTS221_OK : HTS221_ERROR;
}

/* Zaokrúhlenie hodnoty * 10 na celé číslo (symetricky pre kladné aj záporné) */
static int32_t round_x10(float value)
{
  float scaled = value * 10.0f;
  return (int32_t)((scaled >= 0.0f) ? (scaled + 0.5f) : (scaled - 0.5f));
}

/* ---------------------------------------------------------------------------
 * Verejné funkcie
 * ------------------------------------------------------------------------ */

void hts221_register_callbacks(hts221_read_cb_t read_cb, hts221_write_cb_t write_cb)
{
  i2c_read_cb  = read_cb;
  i2c_write_cb = write_cb;
}

uint8_t hts221_init(void)
{
  uint8_t who_am_i = 0;
  uint8_t cal[HTS221_CALIB_LEN];
  uint8_t ctrl1;

  /* 1. Overenie pripojenia senzora */
  if (read_regs(HTS221_WHO_AM_I_REG, &who_am_i, 1U) != HTS221_OK)
  {
    return HTS221_ERROR;
  }
  if (who_am_i != HTS221_WHO_AM_I_VALUE)
  {
    return HTS221_ERROR_ID;
  }

  /* 2. Kalibračné koeficienty - registre 0x30 až 0x3F naraz (multi-byte čítanie) */
  if (read_regs(HTS221_CALIB_START, cal, HTS221_CALIB_LEN) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  /* cal[0] = H0_rH_x2 (0x30)        cal[1] = H1_rH_x2 (0x31)
   * cal[2] = T0_degC_x8 (0x32)      cal[3] = T1_degC_x8 (0x33)
   * cal[5] = T1/T0 msb (0x35)
   * cal[6..7]   = H0_T0_OUT (0x36, 0x37)
   * cal[10..11] = H1_T0_OUT (0x3A, 0x3B)
   * cal[12..13] = T0_OUT (0x3C, 0x3D)
   * cal[14..15] = T1_OUT (0x3E, 0x3F) */
  uint16_t t0_x8 = (uint16_t)(((uint16_t)(cal[5] & 0x03U) << 8) | cal[2]);
  uint16_t t1_x8 = (uint16_t)(((uint16_t)(cal[5] & 0x0CU) << 6) | cal[3]);

  calib.h0_rh     = (float)cal[0] / 2.0f;
  calib.h1_rh     = (float)cal[1] / 2.0f;
  calib.t0_degc   = (float)t0_x8 / 8.0f;
  calib.t1_degc   = (float)t1_x8 / 8.0f;
  calib.h0_t0_out = (int16_t)(((uint16_t)cal[7]  << 8) | cal[6]);
  calib.h1_t0_out = (int16_t)(((uint16_t)cal[11] << 8) | cal[10]);
  calib.t0_out    = (int16_t)(((uint16_t)cal[13] << 8) | cal[12]);
  calib.t1_out    = (int16_t)(((uint16_t)cal[15] << 8) | cal[14]);

  /* 3. Konfigurácia: aktívny režim, BDU (výstupné registre sa neaktualizujú,
   *    kým sa neprečítajú obe polovice hodnoty), výstupná frekvencia 12.5 Hz */
  ctrl1 = HTS221_CTRL1_PD_ACTIVE | HTS221_CTRL1_BDU | HTS221_CTRL1_ODR_12_5HZ;
  if (write_regs(HTS221_CTRL_REG1, &ctrl1, 1U) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  return HTS221_OK;
}

uint8_t hts221_read_temperature(int16_t *temperature_x10)
{
  uint8_t raw_bytes[2];

  if (temperature_x10 == 0)
  {
    return HTS221_ERROR;
  }

  /* TEMP_OUT_L a TEMP_OUT_H sa čítajú naraz (multi-byte) */
  if (read_regs(HTS221_TEMP_OUT_L, raw_bytes, 2U) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  int16_t raw = (int16_t)(((uint16_t)raw_bytes[1] << 8) | raw_bytes[0]);

  int32_t span = (int32_t)calib.t1_out - (int32_t)calib.t0_out;
  if (span == 0)
  {
    return HTS221_ERROR;
  }

  /* Lineárna interpolácia medzi kalibračnými bodmi T0 a T1 */
  float temperature = calib.t0_degc +
      ((float)((int32_t)raw - calib.t0_out) * (calib.t1_degc - calib.t0_degc)) / (float)span;

  *temperature_x10 = (int16_t)round_x10(temperature);
  return HTS221_OK;
}

uint8_t hts221_read_humidity(uint16_t *humidity_x10)
{
  uint8_t raw_bytes[2];

  if (humidity_x10 == 0)
  {
    return HTS221_ERROR;
  }

  /* HUMIDITY_OUT_L a HUMIDITY_OUT_H sa čítajú naraz (multi-byte) */
  if (read_regs(HTS221_HUMIDITY_OUT_L, raw_bytes, 2U) != HTS221_OK)
  {
    return HTS221_ERROR;
  }

  int16_t raw = (int16_t)(((uint16_t)raw_bytes[1] << 8) | raw_bytes[0]);

  int32_t span = (int32_t)calib.h1_t0_out - (int32_t)calib.h0_t0_out;
  if (span == 0)
  {
    return HTS221_ERROR;
  }

  /* Lineárna interpolácia medzi kalibračnými bodmi H0 a H1 */
  float humidity = calib.h0_rh +
      ((float)((int32_t)raw - calib.h0_t0_out) * (calib.h1_rh - calib.h0_rh)) / (float)span;

  /* Fyzikálne rozumný rozsah 0 až 100 % */
  if (humidity < 0.0f)   { humidity = 0.0f; }
  if (humidity > 100.0f) { humidity = 100.0f; }

  *humidity_x10 = (uint16_t)round_x10(humidity);
  return HTS221_OK;
}
