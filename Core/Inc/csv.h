/**
 ******************************************************************************
 * @file    csv.h
 * @brief   Formátovanie nameraných hodnôt do CSV riadku pre Serial Oscilloscope
 ******************************************************************************
 */
#ifndef CSV_H_
#define CSV_H_

#include <stdint.h>
#include <stddef.h>

/**
 * Zostaví jeden CSV riadok vo formáte "teplota,vlhkosť,tlak,výška\r\n":
 *   teplota [°C]   "xx.x"      (1 desatinné miesto)
 *   vlhkosť [%]    "xx"        (celé číslo)
 *   tlak [hPa]     "xxxx.xx"   (2 desatinné miesta)
 *   výška [m]      "xx.xx"     (2 desatinné miesta)
 *
 * Funkcia nepoužíva formátovanie float hodnôt (printf s float by v štandardnom
 * nastavení CubeIDE nefungovalo), všetko sa počíta na celých číslach.
 *
 * @param temperature_x10  teplota v desatinách °C
 * @param humidity_x10     vlhkosť v desatinách %
 * @param pressure_x100    tlak v stotinách hPa
 * @param altitude_cm      výška v centimetroch
 * @return počet zapísaných znakov (bez ukončovacej nuly), alebo -1 pri chybe
 */
int csv_format_line(char *buffer, size_t size,
                    int32_t temperature_x10,
                    uint32_t humidity_x10,
                    int32_t pressure_x100,
                    int32_t altitude_cm);

#endif /* CSV_H_ */
