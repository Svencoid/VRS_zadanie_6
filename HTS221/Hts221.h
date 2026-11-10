/**
 ******************************************************************************
 * @file    hts221.h
 * @brief   Driver senzora vlhkosti a teploty I2C komunikacia
 ******************************************************************************
 */
#ifndef HTS221_H_
#define HTS221_H_

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * I2C adresa (7-bitová)
 * ------------------------------------------------------------------------ */
#define HTS221_I2C_ADDR             0x5FU

/* priznak v adrese registra, ktorý zapne auto-inkrement adresy pri multi-byte prenose */
#define HTS221_AUTO_INCREMENT       0x80U

/* ---------------------------------------------------------------------------
 * Adresy registrov
 * ------------------------------------------------------------------------ */
#define HTS221_WHO_AM_I_REG         0x0FU
#define HTS221_AV_CONF_REG          0x10U
#define HTS221_CTRL_REG1            0x20U
#define HTS221_CTRL_REG2            0x21U
#define HTS221_CTRL_REG3            0x22U
#define HTS221_STATUS_REG           0x27U
#define HTS221_HUMIDITY_OUT_L       0x28U
#define HTS221_HUMIDITY_OUT_H       0x29U
#define HTS221_TEMP_OUT_L           0x2AU
#define HTS221_TEMP_OUT_H           0x2BU

/* Kalibračné registre čítajú sa naraz jedným multi-byte čítaním */
#define HTS221_CALIB_START          0x30U
#define HTS221_CALIB_LEN            16U

/* ---------------------------------------------------------------------------
 * Hodnoty registrov
 * ------------------------------------------------------------------------ */
#define HTS221_WHO_AM_I_VALUE       0xBCU

#define HTS221_CTRL1_PD_ACTIVE      0x80U
#define HTS221_CTRL1_BDU            0x04U
#define HTS221_CTRL1_ODR_ONE_SHOT   0x00U
#define HTS221_CTRL1_ODR_1HZ        0x01U
#define HTS221_CTRL1_ODR_7HZ        0x02U
#define HTS221_CTRL1_ODR_12_5HZ     0x03U

/* STATUS_REG: bit 1 = H_DA vlhkost, bit 0 = T_DA teplota */
#define HTS221_STATUS_T_DA          0x01U
#define HTS221_STATUS_H_DA          0x02U

/* ---------------------------------------------------------------------------
 * Navratove kody
 * ------------------------------------------------------------------------ */
#define HTS221_OK                   ((uint8_t)0U)
#define HTS221_ERROR                ((uint8_t)1U)   /* chyba komunikacie */
#define HTS221_ERROR_ID             ((uint8_t)2U)   /* WHO_AM_I  */

/* ---------------------------------------------------------------------------
 * Callbacky pre prístup k I2C (0 = OK, nenulová hodnota = chyba)
 * ------------------------------------------------------------------------ */
typedef uint8_t (*hts221_read_cb_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
typedef uint8_t (*hts221_write_cb_t)(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len);

/**
 * Registrácia funkcií pre čítanie a zápis cez I2C. */
void hts221_register_callbacks(hts221_read_cb_t read_cb, hts221_write_cb_t write_cb);

uint8_t hts221_init(void);

uint8_t hts221_read_temperature(int16_t *temperature_x10);

uint8_t hts221_read_humidity(uint16_t *humidity_x10);

#endif /* HTS221_H_ */
