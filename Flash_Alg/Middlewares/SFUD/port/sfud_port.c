/**
 *  @file sfud_port.c.c
 *
 *  @date 2021-03-17
 *
 *  @author aron566
 *
 *  @copyright 爱谛科技研究院.
 *
 *  @brief 外部Flash驱动接口
 *
 *  @details 1、
 *
 *  @version V1.0
 */
/** Includes -----------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
#include <sfud.h>
#include <stdarg.h>
#include "main.h"
#if USE_FREERTOS /**< 使用FreeRTOS操作系统下的信号量，则操作flash需在线程下执行，且变量不可超出线程栈大小！！！ */
#include "cmsis_os.h"
osSemaphoreId SPI2TaskBinarySem01Handle;
#endif
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///<use C compiler
extern "C" {
#endif
/** Private typedef ----------------------------------------------------------*/
/* 同SPI总线下的多Flash操作CS接口 */
typedef struct
{
  void (*Enable_CS_Gpio)(void);
  void (*Disable_CS_Gpio)(void);
}SPI_FLASH_CS_GPIO_CTL_Typedef_t;

/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef hspi6;
/** Private macros -----------------------------------------------------------*/
#define NORFLASH_SPI_HANDLE       &hspi6  /**< NorFalsh通讯句柄 */
#define USE_SPI_DMA_MODE          0       /**< 使用DMA通讯模式 */
#define USE_DMA_BUF_ADDR          1       /**< 使用DMA缓冲区地址 */
#define NORFLASH_SPI_BLOCK_TIME   1000U   /**< 阻塞通讯时间 */
#define USE_SINGLE_BYTE_SEND      0       /**< 非DMA通讯模式下单字节发送模式使能开关 */

/** Private function prototypes ----------------------------------------------*/
void sfud_log_debug(const char *file, const long line, const char *format, ...);
bool SFUD_Get_DMA_Transfer_State(void);
static void HAL_CS_GPIO_DISABLE(void);
static void HAL_Flash0_CS_GPIO_ENABLE(void);
static void HAL_Flash1_CS_GPIO_ENABLE(void);

/** Private variables --------------------------------------------------------*/
/* 调试打印输出 */
#ifdef SFUD_DEBUG_MODE
  static char log_buf[256];
#endif

static volatile bool TX_Can_Disable_Cs_Flag = false;
#if USE_FREERTOS
  osSemaphoreId SPI2TaskBinarySem01Handle;
#endif
#if USE_SPI_DMA_MODE
  #if USE_DMA_BUF_ADDR
    #if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
      extern const uint32_t USE_DMA_BUF_SPACE$$Base;
      extern const uint32_t USE_DMA_BUF_SPACE$$Limit;
      static const uint32_t *DMA_Buf_Addr_Start = (&USE_DMA_BUF_SPACE$$Base);
      static const uint32_t *DMA_Buf_Addr_End   = (&USE_DMA_BUF_SPACE$$Limit);
    #elif defined(__ICCARM__) || defined(__ICCRX__)/**< IAR方式 */
      #pragma section="USE_DMA_BUF_SPACE"
      static const uint32_t DMA_Buf_Addr_Start = (__section_begin("USE_DMA_BUF_SPACE"));
      static const uint32_t DMA_Buf_Addr_End = (__section_end("USE_DMA_BUF_SPACE"));
    #elif defined(__GNUC__)
        extern const uint32_t _USE_DMA_BUF_SPACE_start;
        extern const uint32_t _USE_DMA_BUF_SPACE_end;
        static const uint32_t *DMA_Buf_Addr_Start = (&_USE_DMA_BUF_SPACE_start);
        static const uint32_t *DMA_Buf_Addr_End = (&_USE_DMA_BUF_SPACE_end);
    #else
      #error not supported compiler, please use command table mode
    #endif
  #else
    static const uint32_t DMA_Buf_Addr_Start = 0;
    static const uint32_t DMA_Buf_Addr_End = 0xFFFFFFFF;
  #endif
#endif

/* 各flash片选操作表 */
static SPI_FLASH_CS_GPIO_CTL_Typedef_t SPI_Cs_Ctl_Table[2];

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/**
 * @brief 初始化CS控制接口
 *
 */
static void HAL_CS_GPIO_CTL_Init(void)
{
  SPI_Cs_Ctl_Table[SFUD_GD25Q64C_DEVICE0_INDEX].Enable_CS_Gpio = HAL_Flash0_CS_GPIO_ENABLE;
  SPI_Cs_Ctl_Table[SFUD_GD25Q64C_DEVICE0_INDEX].Disable_CS_Gpio = HAL_CS_GPIO_DISABLE;
  SPI_Cs_Ctl_Table[SFUD_GD25Q64C_DEVICE1_INDEX].Enable_CS_Gpio = HAL_Flash1_CS_GPIO_ENABLE;
  SPI_Cs_Ctl_Table[SFUD_GD25Q64C_DEVICE1_INDEX].Disable_CS_Gpio = HAL_CS_GPIO_DISABLE;
}

/**
  ******************************************************************
  * @brief   失能Flash0、1
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static void HAL_CS_GPIO_DISABLE(void)
{
  HAL_GPIO_WritePin(SPI6_CS_FLASH0_GPIO_Port, SPI6_CS_FLASH0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI6_CS_FLASH1_GPIO_Port, SPI6_CS_FLASH1_Pin, GPIO_PIN_SET);
}

/**
  ******************************************************************
  * @brief   使能Flash0
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static void HAL_Flash0_CS_GPIO_ENABLE(void)
{
  HAL_GPIO_WritePin(SPI6_CS_FLASH0_GPIO_Port, SPI6_CS_FLASH0_Pin, GPIO_PIN_RESET);
}

/**
  ******************************************************************
  * @brief   使能Flash
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static void HAL_Flash1_CS_GPIO_ENABLE(void)
{
  HAL_GPIO_WritePin(SPI6_CS_FLASH1_GPIO_Port, SPI6_CS_FLASH1_Pin, GPIO_PIN_RESET);
}

#if USE_SINGLE_BYTE_SEND
/**
  ******************************************************************
  * @brief   SPI发送一个字节
  * @param   [in]byte
  * @retval  None
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static void HAL_SPI_Send_Byte(uint8_t byte)
{
  HAL_SPI_Transmit(NORFLASH_SPI_HANDLE, &byte, 1, NORFLASH_SPI_BLOCK_TIME);
}
#endif

/**
  ******************************************************************
  * @brief   SPI DMA发送多个字节
  * @param   [in]pData 发送缓冲区
  * @param   [in]Size 发送长度
  * @retval  SFUD_SUCCESS 成功
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
//static sfud_err HAL_SPI_DMA_Send_Bytes(uint8_t *pData, uint16_t Size)
//{
//  if(HAL_OK != HAL_SPI_Transmit_DMA(NORFLASH_SPI_HANDLE, pData, Size))
//  {
//    return SFUD_ERR_READ;
//  }
//  return SFUD_SUCCESS;
//}

#if USE_SINGLE_BYTE_SEND
/**
  ******************************************************************
  * @brief   SPI接收一个字节
  * @param   [in]None
  * @retval  字节
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static uint8_t HAL_SPI_Receive_Byte(void)
{
  uint8_t data = 0;
  HAL_SPI_Receive(NORFLASH_SPI_HANDLE, &data, 1, NORFLASH_SPI_BLOCK_TIME);
  return data;
}
#endif

/**
  ******************************************************************
  * @brief   SPI DMA接收多个字节
  * @param   [in]pData 接收缓冲区
  * @param   [in]Size 接收长度
  * @retval  SFUD_SUCCESS 成功
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
//static sfud_err HAL_SPI_DMA_Receive_Bytes(uint8_t *pData, uint16_t Size)
//{
//  if(HAL_OK != HAL_SPI_Receive_DMA(NORFLASH_SPI_HANDLE, pData, Size))
//  {
//    return SFUD_ERR_READ;
//  }
//  return SFUD_SUCCESS;
//}

/**
  ******************************************************************
  * @brief   SPI DMA写读接口
  * @param   [in]pTxData 待发送数据
  * @param   [in]pRxData 接收数据缓冲区
  * @retval  SFUD_SUCCESS 成功
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
//static sfud_err HAL_SPI_DMA_Write_Read(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
//{
//  /* select the flash: chip select low */
//  HAL_CS_GPIO_ENABLE();
//
//  if(HAL_OK != HAL_SPI_TransmitReceive_DMA(NORFLASH_SPI_HANDLE, pTxData, pRxData, Size))
//  {
//    return SFUD_ERR_WRITE;
//  }
//  return SFUD_SUCCESS;
//}

/**
  ******************************************************************
  * @brief   SPI多字节发送
  * @param   [in]spi sfud spi初始化数据
  * @param   [in]pData 发送区
  * @param   [in]Size 发送字节数
  * @param   [in]CS_Ctl 为true片选控制允许
  * @retval  SFUD_SUCCESS 成功发送
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static sfud_err HAL_Flash_Multi_Write(const sfud_spi *spi, const uint8_t *pData, uint16_t Size, bool CS_Ctl)
{
  /* select the flash: chip select low */
  SPI_FLASH_CS_GPIO_CTL_Typedef_t *Cs_Opt = (SPI_FLASH_CS_GPIO_CTL_Typedef_t *)spi->user_data;
  Cs_Opt->Enable_CS_Gpio();

#if USE_SPI_DMA_MODE
  /* when this buf in dma buf. */
  if((uint32_t)pData >= (uint32_t)DMA_Buf_Addr_Start && (uint32_t)pData < (uint32_t)DMA_Buf_Addr_End)
  {
    TX_Can_Disable_Cs_Flag = CS_Ctl;
    return HAL_SPI_DMA_Send_Bytes((uint8_t *)pData, Size);
  }
#endif

  /* while there is data to be written on the flash */
#if USE_SINGLE_BYTE_SEND
  while(Size--){
      /* send the current byte */
      HAL_SPI_Send_Byte(*pData);
      /* point on the next byte to be written */
      pData++;
  }
#else
  HAL_SPI_Transmit(NORFLASH_SPI_HANDLE, (uint8_t *)pData, Size, NORFLASH_SPI_BLOCK_TIME);
#endif

  /* 片选控制 */
  if(CS_Ctl == true)
  {
    HAL_CS_GPIO_DISABLE();
  }
  return SFUD_SUCCESS;
}

/**
  ******************************************************************
  * @brief   SPI多字节接收
  * @param   [in]spi sfud spi初始化数据
  * @param   [in]pbuffer 接收区
  * @param   [in]Size 接收大小
  * @param   [in]CS_Ctl 为true片选控制允许
  * @retval  SFUD_SUCCESS 成功接收
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static sfud_err HAL_Flash_Multi_Read(const sfud_spi *spi, uint8_t* pData, uint16_t Size, bool CS_Ctl)
{
  /* select the flash: chip slect low */
  SPI_FLASH_CS_GPIO_CTL_Typedef_t *Cs_Opt = (SPI_FLASH_CS_GPIO_CTL_Typedef_t *)spi->user_data;
  Cs_Opt->Enable_CS_Gpio();

#if USE_SPI_DMA_MODE
  /* when this buf in dma buf. */
  if((uint32_t)pData >= (uint32_t)DMA_Buf_Addr_Start && (uint32_t)pData < (uint32_t)DMA_Buf_Addr_End)
  {
    return HAL_SPI_DMA_Receive_Bytes(pData, Size);
  }
#endif

  /* while there is data to be read */
#if USE_SINGLE_BYTE_SEND
  while(Size--){
      /* read a byte from the flash */
      *pData = HAL_SPI_Receive_Byte();
      /* point to the next location where the byte read will be saved */
      pData++;
  }
#else
  HAL_SPI_Receive(NORFLASH_SPI_HANDLE, pData, Size, NORFLASH_SPI_BLOCK_TIME);
#endif

  /* 片选控制 */
  if(CS_Ctl == true)
  {
    HAL_CS_GPIO_DISABLE();
  }
  return SFUD_SUCCESS;
}

/**
  ******************************************************************
  * @brief   SPI信号量初始化
  * @param   [in]None
  * @retval  None
  * @author  aron566
  * @version v1.0
  * @date    2021-05-14
  ******************************************************************
  */
static void SPI_Lock_Init(void)
{
#if USE_FREERTOS_SYSTEM
  SPI2TaskBinarySem01Handle = osSemaphoreNew(1, 1, NULL);
#endif
}
/* lock SPI bus */
static void SPI_lock(const struct __sfud_spi *spi)
{
#if USE_FREERTOS_SYSTEM
  osSemaphoreAcquire(SPI2TaskBinarySem01Handle, 0);
#endif
}
/* unlock SPI bus */
static void SPI_unlock(const struct __sfud_spi *spi)
{
#if USE_FREERTOS_SYSTEM
  osSemaphoreRelease(SPI2TaskBinarySem01Handle);
#endif
}
/* SPI Delay */
static void SPI_Delay(void)
{
#if USE_FREERTOS_SYSTEM
   osDelay(1);
#else
  HAL_Delay(1);
#endif
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size)
{
  /* 等操作完成 */
  while(SFUD_Get_DMA_Transfer_State() == false);

  /**
   * add your spi write and read code
   */
  if(write_size)
  {
    SFUD_ASSERT(write_buf);
    if(SFUD_SUCCESS != HAL_Flash_Multi_Write(spi, write_buf, write_size, (read_size > 0)?false:true))
    {
      return SFUD_ERR_WRITE;
    }
  }

  if(read_size)
  {
    SFUD_ASSERT(read_buf);
    if(SFUD_SUCCESS != HAL_Flash_Multi_Read(spi, read_buf, read_size, true))
    {
      return SFUD_ERR_READ;
    }
  }

  return SFUD_SUCCESS;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size)
{
  sfud_err result = SFUD_SUCCESS;

  /**
   * add your qspi read flash data code
   */

  return result;
}

#endif /* SFUD_USING_QSPI */
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

/**
  * @brief Tx and Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if(hspi == NORFLASH_SPI_HANDLE)
  {
    HAL_CS_GPIO_DISABLE();
  }
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxRxCpltCallback should be implemented in the user file
   */
}

/**
  * @brief Tx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_TxCpltCallback should be implemented in the user file
   */
  if(hspi == NORFLASH_SPI_HANDLE && TX_Can_Disable_Cs_Flag == true)
  {
    HAL_CS_GPIO_DISABLE();
  }
}

/**
  * @brief Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SPI_RxCpltCallback should be implemented in the user file
   */
  if(hspi == NORFLASH_SPI_HANDLE)
  {
    HAL_CS_GPIO_DISABLE();
  }
}

/**
  ******************************************************************
  * @brief   sfud接口初始化
  * @param   [in]flash 句柄
  * @retval  初始化状态
  * @author  aron566
  * @version v1.0
  * @date    2021-06-16
  ******************************************************************
  */
sfud_err sfud_spi_port_init(sfud_flash *flash)
{
  sfud_err result = SFUD_SUCCESS;

  /**
   * add your port spi bus and device object initialize code like this:
   * 1. rcc initialize
   * 2. gpio initialize
   * 3. spi device initialize
   * 4. flash->spi and flash->retry item initialize
   *    flash->spi.wr = spi_write_read; //Required
   *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
   *    flash->spi.lock = spi_lock;
   *    flash->spi.unlock = spi_unlock;
   *    flash->spi.user_data = &spix;
   *    flash->retry.delay = null;
   *    flash->retry.times = 10000; //Required
   */
  SPI_Lock_Init();
  HAL_CS_GPIO_CTL_Init();

  flash->spi.wr = spi_write_read;
  flash->spi.lock = SPI_lock;
  flash->spi.unlock = SPI_unlock;
  flash->spi.user_data = (void *)&SPI_Cs_Ctl_Table[flash->index];
  flash->retry.delay = SPI_Delay;
  flash->retry.times = 10000;
  return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...)
{
#ifdef SFUD_DEBUG_MODE
  va_list args;

  /* args point to the first variable parameter */
  va_start(args, format);
  printf("[SFUD](%s:%ld) ", file, line);
  /* must use vprintf to print */
  vsnprintf(log_buf, sizeof(log_buf), format, args);
  printf("%s\r\n", log_buf);
  va_end(args);
#endif
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...)
{
#ifdef SFUD_DEBUG_MODE
  va_list args;

  /* args point to the first variable parameter */
  va_start(args, format);
  printf("[SFUD]");
  /* must use vprintf to print */
  vsnprintf(log_buf, sizeof(log_buf), format, args);
  printf("%s\r\n", log_buf);
  va_end(args);
#endif
}

/**
  ******************************************************************
  * @brief   获取SFUD，DMA 传输状态
  * @param   [in]None.
  * @retval  true 传输成功
  * @author  aron566
  * @version v1.0
  * @date    2021-06-16
  ******************************************************************
  */
bool SFUD_Get_DMA_Transfer_State(void)
{
  return (HAL_SPI_STATE_READY == HAL_SPI_GetState(NORFLASH_SPI_HANDLE))?true:false;
}

#ifdef __cplusplus ///<end extern c
}
#endif
/******************************** End of file *********************************/
