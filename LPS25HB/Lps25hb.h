/**
 ******************************************************************************
 * @file    lps25hb.h
 * @brief   Driver barometrického senzora LPS25HB (komunikácia cez I2C)
 *
 * Driver nepozná konkrétnu perifériu MCU. Funkcie na čítanie/zápis cez I2C sa
 * mu odovzdajú ako callbacky (lps25hb_register_callbacks).
 ******************************************************************************
 */
#ifndef LPS25HB_H_
#define LPS25HB_H_

#include <stdint.h>

/* ---------------------------------------------------------------------------
 * I2C adresa (7-bitová). Na doske X-NUCLEO-IKS01A1 je to 0x5D (8-bitovo 0xBA).
 * Ak by senzor neodpovedal, skús 0x5C (SA0 pripojený na GND).
 * ------------------------------------------------------------------------ */
#define LPS25HB_I2C_ADDR            0x5DU

/* Príznak v adrese registra, ktorý zapne auto-inkrement adresy pri multi-byte prenose */
#define LPS25HB_AUTO_INCREMENT      0x80U

/* ---------------------------------------------------------------------------
 * Adresy registrov
 * ------------------------------------------------------------------------ */
#define LPS25HB_REF_P_XL_REG        0x15U
#define LPS25HB_WHO_AM_I_REG        0x0FU
#define LPS25HB_RES_CONF_REG        0x1AU
#define LPS25HB_CTRL_REG1           0x20U
#define LPS25HB_CTRL_REG2           0x21U
#define LPS25HB_CTRL_REG3           0x22U
#define LPS25HB_STATUS_REG          0x27U
#define LPS25HB_PRESS_OUT_XL        0x28U
#define LPS25HB_PRESS_OUT_L         0x29U
#define LPS25HB_PRESS_OUT_H         0x2AU
#define LPS25HB_TEMP_OUT_L          0x2BU
#define LPS25HB_TEMP_OUT_H          0x2CU

/* ---------------------------------------------------------------------------
 * Hodnoty registrov
 * ------------------------------------------------------------------------ */
#define LPS25HB_WHO_AM_I_VALUE      0xBDU

/* CTRL_REG1: bit 7 = PD (1 = aktívny), bity 6:4 = ODR, bit 2 = BDU */
#define LPS25HB_CTRL1_PD_ACTIVE     0x80U
#define LPS25HB_CTRL1_ODR_ONE_SHOT  0x00U
#define LPS25HB_CTRL1_ODR_1HZ       0x10U
#define LPS25HB_CTRL1_ODR_7HZ       0x20U
#define LPS25HB_CTRL1_ODR_12_5HZ    0x30U
#define LPS25HB_CTRL1_ODR_25HZ      0x40U
#define LPS25HB_CTRL1_BDU           0x04U

/* STATUS_REG: bit 1 = P_DA (tlak pripravený), bit 0 = T_DA (teplota pripravená) */
#define LPS25HB_STATUS_P_DA         0x02U

/* Citlivosť senzora: 4096 LSB = 1 hPa */
#define LPS25HB_LSB_PER_HPA         4096

/* ---------------------------------------------------------------------------
 * Návratové kódy
 * ------------------------------------------------------------------------ */
#define LPS25HB_OK                  ((uint8_t)0U)
#define LPS25HB_ERROR               ((uint8_t)1U)   /* chyba komunikácie / neregistrované callbacky */
#define LPS25HB_ERROR_ID            ((uint8_t)2U)   /* WHO_AM_I nesedí s dokumentáciou */

/* ---------------------------------------------------------------------------
 * Callbacky pre prístup k I2C (0 = OK, nenulová hodnota = chyba)
 * ------------------------------------------------------------------------ */
typedef uint8_t (*lps25hb_read_cb_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
typedef uint8_t (*lps25hb_write_cb_t)(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len);

/**
 * Registrácia funkcií pre čítanie a zápis cez I2C. Musí sa zavolať pred lps25hb_init().
 */
void lps25hb_register_callbacks(lps25hb_read_cb_t read_cb, lps25hb_write_cb_t write_cb);

/**
 * Overí pripojenie senzora (WHO_AM_I) a nastaví ho do aktívneho režimu
 * (ODR 12.5 Hz, BDU zapnutý).
 */
uint8_t lps25hb_init(void);

/**
 * Čítanie tlaku vzduchu. Výsledok je v stotinách hPa (napr. 101325 = 1013.25 hPa).
 */
uint8_t lps25hb_read_pressure(int32_t *pressure_x100);

/**
 * Post processing: výpočet relatívnej výšky z tlaku.
 *
 * @param pressure_x100      aktuálny tlak v stotinách hPa
 * @param ref_pressure_x100  referenčný tlak v stotinách hPa (tlak na zemi / pri štarte)
 * @return výška nad referenčnou úrovňou v centimetroch (napr. 832 = 8.32 m)
 */
int32_t lps25hb_calc_relative_altitude_cm(int32_t pressure_x100, int32_t ref_pressure_x100);

#endif /* LPS25HB_H_ */
