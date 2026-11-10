/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hts221.h"
#include "lps25hb.h"
#include "csv.h"
#include "stm32f3xx_ll_usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* perioda udajov do pc v ms */
#define SEND_PERIOD_MS          100U

/* pocet merani tlaku na zemi */
#define REF_PRESSURE_SAMPLES    10U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void usart2_send_string(const char *text);
static void sensors_fatal_error(const char *message);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* odosielanie textu cez USART2 a uvolnenie tx registra */
static void usart2_send_string(const char *text)
{
  while (*text != '\0')
  {
    while (!LL_USART_IsActiveFlag_TXE(USART2))
    {
    }
    LL_USART_TransmitData8(USART2, (uint8_t)*text);
    text++;
  }

  /* pockaj kym sa odosle posledný bajt */
  while (!LL_USART_IsActiveFlag_TC(USART2))
  {
  }
}

/* error handling */
static void sensors_fatal_error(const char *message)
{
  while (1)
  {
    usart2_send_string(message);
    LL_mDelay(1000);
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Registrácia callbackov,
   * komunikaciu dostanu ako ukazovatele z i2c.c */
  hts221_register_callbacks(i2c_read, i2c_write);
  lps25hb_register_callbacks(i2c_read, i2c_write);

  /* Inicializacia senzorov */
  uint8_t st = hts221_init();
  if (st != HTS221_OK)
  {
    sensors_fatal_error(st == HTS221_ERROR_ID ? "ERROR: HTS221 WHO_AM_I mismatch\r\n"
                                              : "ERROR: HTS221 I2C no response\r\n");
  }
  if (lps25hb_init() != LPS25HB_OK)
  {
    sensors_fatal_error("ERROR: LPS25HB init failed\r\n");
  }

  /* cas prvzch merani */
  LL_mDelay(500);

  /* referencny tlak na zemi.
   * vyska relativna k zaiatocnej polohe */
  int32_t ref_pressure_x100 = 0;
  {
    int32_t sum = 0;
    uint8_t valid = 0;
    for (uint8_t i = 0; i < REF_PRESSURE_SAMPLES; i++)
    {
      int32_t p;
      if (lps25hb_read_pressure(&p) == LPS25HB_OK)
      {
        sum += p;
        valid++;
      }
      LL_mDelay(100);
    }
    if (valid == 0U)
    {
      sensors_fatal_error("ERROR: LPS25HB read failed\r\n");
    }
    ref_pressure_x100 = sum / (int32_t)valid;
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    int16_t  temperature_x10;
    uint16_t humidity_x10;
    int32_t  pressure_x100;

    if ((hts221_read_temperature(&temperature_x10) == HTS221_OK) &&
        (hts221_read_humidity(&humidity_x10) == HTS221_OK) &&
        (lps25hb_read_pressure(&pressure_x100) == LPS25HB_OK))
    {
      /* post processing: relativny tlak */
      int32_t altitude_cm = lps25hb_calc_relative_altitude_cm(pressure_x100, ref_pressure_x100);

      char line[64];
      if (csv_format_line(line, sizeof(line), temperature_x10, humidity_x10,
                          pressure_x100, altitude_cm) > 0)
      {
        usart2_send_string(line);
      }
    }

    LL_mDelay(SEND_PERIOD_MS);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
