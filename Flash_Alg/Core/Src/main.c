/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const uint16_t TEST_FLASH __attribute__((used)) __attribute__((section("USE_EXT_FLASH_2MB_BUF_SPACE"))) = 0x1010; 
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
  static uint32_t ticks = 0U;
  uint32_t i;
  for(i = (SystemCoreClock >> 14U); i > 0U; i--)
  {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;
  if(wait < HAL_MAX_DELAY)
  {
    wait += (uint32_t)(HAL_TICK_FREQ_DEFAULT);
  }
  while((HAL_GetTick() - tickstart) < wait)
  {
    __NOP();
  }
}
#include "stdio.h"
#define DEBUG_UART          &huart2
#if defined (__CC_ARM)
  #pragma import(__use_no_semihosting) // 确保没有�? C 库链接使用半主机的函�?
#endif
struct __FILE // 标准库需要的支持函数
{
	int handle;
};
/* FILE is typedef �? d in stdio.h. */
FILE __stdout;
void _ttywrch(int ch)
{
  ch = ch;
}

void _sys_exit(int x) //定义 _sys_exit() 以避免使用半主机模式
{
  (void)x;
}

/************************************************************
  * @brief   重定向c库函数printf到HAL_UART_Transmit
  * @param   [in]ch 输出字符.
  * @param   [in]f 文件指针
  * @return  字符
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&ch, 1, 0xFF);
  return ch;
}

/************************************************************
  * @brief   重定向c库函数getchar,scanf
  * @param   [in]f 文件指针
  * @return  字符
  * @author  aron566
  * @date    2020/3/15
  * @version v1.0
  * @note    @@
  ***********************************************************/
int fgetc(FILE *f)
{
  uint8_t ch = 0;
  while(HAL_UART_Receive(DEBUG_UART, (uint8_t *)&ch, 1, 0xFF) != HAL_OK);
  return ch;
}

#include "sfud.h"

#define SPI_FLASH_MEM_ADDR  0//0x09000000
static sfud_flash_t sfud_dev0 = NULL;
unsigned char aux_buf[4096];
/**
  ******************************************************************
  * @brief   Flash初始�?
  * @param   [in]None
  * @return  0成功
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int flash0_init(void)
{
#ifdef RT_USING_SFUD
  /* RT-Thread RTOS platform */
  sfud_dev0 = rt_sfud_flash_find_by_dev_name(FAL_USING_NOR_FLASH_DEV_NAME);
#else
  /* bare metal platform */
  sfud_dev0 = sfud_get_device(SFUD_GD25Q64C_DEVICE0_INDEX);
#endif

  if(NULL == sfud_dev0)
  {
    return -1;
  }

  return 0;
}

/**
  ******************************************************************
  * @brief   Flash读取
  * @param   [in]None
  * @return  读取大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int flash0_read(long offset, uint8_t *buf, size_t size)
{
  uint8_t status;
  // sfud_err ret;
  sfud_read_status(sfud_dev0, &status);
  // printf("fresh sfud_read Status: %d\r\n", ret);

  if(sfud_read(sfud_dev0, 512 * 1024 * 3 + offset, size, buf) != SFUD_SUCCESS)
  {
    return -1;
  }

  return size;
}

/**
  ******************************************************************
  * @brief   Flash写入
  * @param   [in]None
  * @return  写入大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int flash0_write(long offset, const uint8_t *buf, size_t size)
{
  uint8_t status;
  // sfud_err ret;
  sfud_read_status(sfud_dev0, &status);
  // printf("fresh sfud_write Status: %d\r\n", ret);

  if(sfud_write(sfud_dev0,  512 * 1024 * 3 + offset, size, buf) != SFUD_SUCCESS)
  {
    return -1;
  }

  return size;
}

/**
  ******************************************************************
  * @brief   Flash擦除
  * @param   [in]None
  * @return  擦除大小
  * @author  aron566
  * @version V1.0
  * @date    2021-05-14
  ******************************************************************
  */
static int flash0_erase(long offset, size_t size)
{
  uint8_t status;
  // sfud_err ret;
  sfud_read_status(sfud_dev0, &status);
  // printf("fresh sfud_erase Status: %d\r\n", ret);

  if(sfud_erase(sfud_dev0, 512 * 1024 * 3 + offset, size) != SFUD_SUCCESS)
  {
    return -1;
  }

  return size;
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
  HAL_Init();

  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_SPI6_Init();
  /* USER CODE BEGIN 2 */
  /* 初始化Flash驱动 */
  if(sfud_init() != SFUD_SUCCESS)
  {
    return 1;
  }

  /* 初始化Flash驱动 */
  if(flash0_init() != 0)
  {
    return 1;
  }

  printf("Init OK.\r\n");
  
  flash0_erase(128*1024, 128*1024);
  uint8_t num[10] = {0xFF, 0x05,0x04,0x03,0x02,0x01,0x08,0x33,0x22,0x11};
  /*write*/
  flash0_write(128*1024, num, 10);
  uint8_t ret[10] = {0};
  /*read*/
  flash0_read(128*1024, ret, 10);
  /*compare*/
  if(memcmp(num, ret, 10) ==  0)
  {
    printf("Data Verify OK.\r\n");
  }
  uint16_t test = 0;
  flash0_read(0, (uint8_t *)&test, 2);
  printf("test: 0x1010 =? 0x%04X\r\n", test);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 10;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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

#ifdef  USE_FULL_ASSERT
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
