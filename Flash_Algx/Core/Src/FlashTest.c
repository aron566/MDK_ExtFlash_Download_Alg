/**
 *  @file FlashTest.c
 *
 *  @date 2022年10月12日 12:08:34 星期三
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v1.0.0
 */
/** Includes -----------------------------------------------------------------*/
#include <string.h>
/* Private includes ----------------------------------------------------------*/
#include "RTE_Components.h"
#include CMSIS_device_header
#include "FlashOS.h"
#include "printf.h"
#include "usart.h"

/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///< use C compiler
extern "C" {
#endif
/** Private macros -----------------------------------------------------------*/
#define DEBUG_UART          &huart2
/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
extern struct FlashDevice const FlashDevice;
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/
#include "sfud.h"

#define SPI_FLASH_MEM_ADDR  0//0x09000000
static sfud_flash_t sfud_dev0 = NULL;

/**
 ******************************************************************
 * @brief   Flash初始化
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
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/
void _putchar(char character)
{
 HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&character, 1, 0xFFFF);
}

void stop_on_error(uint32_t cond) {
    if (cond) {
        __BKPT(0x1);
        while(1) {}
    }
}

#define MEMORY_SECTOR_SIZE 1024
static uint8_t test_buf[MEMORY_SECTOR_SIZE];

volatile int ret;
int main(void) {
  /* 擦除 */
  ret = Init(FlashDevice.DevAdr, 0, 1);
  stop_on_error(ret);
  ret = BlankCheck(FlashDevice.DevAdr, MEMORY_SECTOR_SIZE, 4);
  if(1 == ret)
  {
    EraseSector(FlashDevice.DevAdr);
  }
  ret = UnInit(1);
  stop_on_error(ret);


  /* 编程 */
  ret = Init(FlashDevice.DevAdr, 0, 2);
  stop_on_error(ret);
  for (int i = 0; i < MEMORY_SECTOR_SIZE; i++) {
      test_buf[i] = i;
  }
  ret = ProgramPage(FlashDevice.DevAdr, MEMORY_SECTOR_SIZE, test_buf);
  stop_on_error(ret);
  for (int i = 0; i < MEMORY_SECTOR_SIZE; i++) {
      test_buf[i] = i + 1;
  }
  ret = ProgramPage(FlashDevice.DevAdr + MEMORY_SECTOR_SIZE, MEMORY_SECTOR_SIZE, test_buf);
  stop_on_error(ret);
  for (int i = 0; i < MEMORY_SECTOR_SIZE; i++) {
      test_buf[i] = i;
  }
  ret = UnInit(2);
  stop_on_error(ret);

  /* 校验 */
  ret = Init(FlashDevice.DevAdr, 0, 3);
  stop_on_error(ret);
  ret = Verify(FlashDevice.DevAdr, MEMORY_SECTOR_SIZE, test_buf);
  for (int i = 0; i < MEMORY_SECTOR_SIZE; i++) {
      test_buf[i] = i + 1;
  }
  ret = Verify(FlashDevice.DevAdr + MEMORY_SECTOR_SIZE, MEMORY_SECTOR_SIZE, test_buf);
  ret = UnInit(3);
  stop_on_error(ret);

  /* 初始化Flash驱动 */
  if(sfud_init() != SFUD_SUCCESS)
  {
    return 1;
  }

  /* 初始化驱动 */
  if(flash0_init() != 0)
  {
    return 1;
  }

  printf("Init OK.\r\n");

  flash0_erase(0, 4 * 1024);

  char result = 0;
  for(int i = 0; i < 1024; i++)
  {
    test_buf[i] = i;
  }

  /* write */
  flash0_write(0, test_buf, 1024);
  uint8_t ret[1024] = {0};
  /* read */
  flash0_read(0, ret, 1024);
  /* compare */
  for(uint16_t i = 0; i < 1024; i++)
  {
    if(test_buf[i] != ret[i])
    {
      printf("Data[%hu] Verify Error.\r\n", i);
      result = 1;
    }
  }
  if(result == 0)
  {
    printf("Data Buf 1 Verify OK.\r\n");
  }
  result = 0;
  for(int i = 0; i < 1024; i++)
  {
    test_buf[i] = i + 1;
  }

  /* write */
  flash0_write(1024, test_buf, 1024);
  /* read */
  flash0_read(1024, ret, 1024);
  /* compare */
  for(uint16_t i = 0; i < 1024; i++)
  {
    if(test_buf[i] != ret[i])
    {
      printf("Data[%hu] Verify Error.\r\n", i);
      result = 1;
    }
  }
  if(result == 0)
  {
    printf("Data Buf 2 Verify OK.\r\n");
  }
  uint32_t time = 0;
  while (1) {
    printf("%u\r\n", time++);
    HAL_Delay(1000);
  }
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
