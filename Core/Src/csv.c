
#include "csv.h"
#include <stdio.h>

/* zapis cial s pevnymi desatinnymi miestami */
static int format_fixed(char *dst, size_t size, int32_t value, uint8_t decimals)
{
  uint32_t divisor = 1U;
  for (uint8_t i = 0; i < decimals; i++)
  {
    divisor *= 10U;
  }

  const char *sign = (value < 0) ? "-" : "";
  uint32_t magnitude = (value < 0) ? (uint32_t)(-value) : (uint32_t)value;

  if (decimals == 0U)
  {
    return snprintf(dst, size, "%s%lu", sign, (unsigned long)magnitude);
  }

  return snprintf(dst, size, "%s%lu.%0*lu", sign,
                  (unsigned long)(magnitude / divisor),
                  (int)decimals,
                  (unsigned long)(magnitude % divisor));
}

int csv_format_line(char *buffer, size_t size,
                    int32_t temperature_x10,
                    uint32_t humidity_x10,
                    int32_t pressure_x100,
                    int32_t altitude_cm)
{
  char temp_str[16];
  char hum_str[16];
  char press_str[16];
  char alt_str[16];

  /* Vlhkosť sa zobrazuje na celé percentá (zaokrúhlené) */
  int32_t humidity_percent = (int32_t)((humidity_x10 + 5U) / 10U);

  format_fixed(temp_str,  sizeof(temp_str),  temperature_x10, 1U);
  format_fixed(hum_str,   sizeof(hum_str),   humidity_percent, 0U);
  format_fixed(press_str, sizeof(press_str), pressure_x100,   2U);
  format_fixed(alt_str,   sizeof(alt_str),   altitude_cm,     2U);

  return snprintf(buffer, size, "%s,%s,%s,%s\r\n", temp_str, hum_str, press_str, alt_str);
}
