/**************************************************************************//**
 * @file     FlashPrg.c
 * @brief    Flash Programming Functions adapted for New Device Flash
 * @version  V1.0.0
 * @date     10. January 2018
 ******************************************************************************/
/*
 * Copyright (c) 2010-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "..\FlashOS.H"        // FlashOS Structures

#define ENABLE_FLASH_ALG_DEBUG  1

#define DEBUG_INIT        0
#define DEBUG_ERASE       0
#define DEBUG_ERASE_CHIP  0
#define DEBUG_PROGRAM     0
#define DEBUG_VERIFY      0

#if ENABLE_FLASH_ALG_DEBUG
#include "string.h"
#include "printf.h"
#define PRINTF(...)         printf(__VA_ARGS__)
#include "usart.h"
#define DEBUG_UART          &huart2

__weak void _putchar(char character)
{
 HAL_UART_Transmit(DEBUG_UART, (uint8_t *)&character, 1, 0xFFFF);
}
#else
#define PRINTF(...)
__weak void _putchar(char character)
{

}
#endif

#define USE_SFUD_LIB        1
#define SPI_FLASH_MEM_ADDR  0x90000000
#if USE_SFUD_LIB
#include "sfud.h"
#include "spi.h"

#define SPI_HANDLE          &hspi6
static sfud_flash_t sfud_dev0 = NULL;
uint8_t aux_buf[4096];
#else

#endif
/*
   Mandatory Flash Programming Functions (Called by FlashOS):
                int Init        (unsigned long adr,   // Initialize Flash
                                 unsigned long clk,
                                 unsigned long fnc);
                int UnInit      (unsigned long fnc);  // De-initialize Flash
                int EraseSector (unsigned long adr);  // Erase Sector Function
                int ProgramPage (unsigned long adr,   // Program Page Function
                                 unsigned long sz,
                                 unsigned char *buf);

   Optional  Flash Programming Functions (Called by FlashOS):
                int BlankCheck  (unsigned long adr,   // Blank Check
                                 unsigned long sz,
                                 unsigned char pat);
                int EraseChip   (void);               // Erase complete Device
      unsigned long Verify      (unsigned long adr,   // Verify Function
                                 unsigned long sz,
                                 unsigned char *buf);

       - BlanckCheck  is necessary if Flash space is not mapped into CPU memory space
       - Verify       is necessary if Flash space is not mapped into CPU memory space
       - if EraseChip is not provided than EraseSector for all sectors is called
*/


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init(unsigned long adr, unsigned long clk, unsigned long fnc) {

  /* Add your Code */
  extern int Xmain(void);

  /* 硬件初始化 */
  Xmain();

#if USE_SFUD_LIB && DEBUG_INIT == 0
  HAL_UART_DeInit(DEBUG_UART);
  HAL_SPI_DeInit(SPI_HANDLE);
  HAL_UART_Init(DEBUG_UART);
  HAL_SPI_Init(SPI_HANDLE);
  /* 初始化Flash驱动 */
  if(sfud_init() != SFUD_SUCCESS)
  {
    return 1;
  }

  /* 初始化驱动 */
  sfud_dev0 = NULL;
  sfud_dev0 = sfud_get_device(SFUD_GD25Q64C_DEVICE0_INDEX);
  if(NULL == sfud_dev0)
  {
    return 1;
  }
#else

#endif
  PRINTF("Extern Flash Init %u.\r\n", fnc);
  return (0);                                  // Finished without Errors
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit(unsigned long fnc) {

  /* Add your Code */
  PRINTF("UnInit %u.\r\n", fnc);
  return (0);                                  // Finished without Errors
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip(void) {
  /* Add your Code */
#if USE_SFUD_LIB && DEBUG_ERASE_CHIP == 0
  HAL_UART_DeInit(DEBUG_UART);
  HAL_SPI_DeInit(SPI_HANDLE);
  HAL_UART_Init(DEBUG_UART);
  HAL_SPI_Init(SPI_HANDLE);
  if(sfud_dev0 == NULL)
  {
    return 1;
  }
  uint8_t status;
  sfud_read_status(sfud_dev0, &status);

  if(sfud_erase(sfud_dev0, 512 * 1024 * 3 + 0, 2 * 1024 * 1024) != SFUD_SUCCESS)
  {
    return 1;
  }
#else

#endif
  PRINTF("EraseChip.\r\n");
  return (0);                                  // Finished without Errors
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector(unsigned long adr) {
  /* Add your Code */
  adr -= SPI_FLASH_MEM_ADDR;
#if USE_SFUD_LIB && DEBUG_ERASE == 0
  HAL_UART_DeInit(DEBUG_UART);
  HAL_SPI_DeInit(SPI_HANDLE);
  HAL_UART_Init(DEBUG_UART);
  HAL_SPI_Init(SPI_HANDLE);
  if(sfud_dev0 == NULL)
  {
    return 1;
  }
  uint8_t status;
  sfud_read_status(sfud_dev0, &status);

  if(sfud_erase(sfud_dev0, 512 * 1024 * 3 + adr, 4096) != SFUD_SUCCESS)
  {
    return 1;
  }
#else

#endif
  PRINTF("EraseSector 0x%08X.\r\n", adr);
  return (0);                                  // Finished without Errors
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage(unsigned long adr, unsigned long sz, unsigned char *buf) {
  /* Add your Code */
  adr -= SPI_FLASH_MEM_ADDR;
#if USE_SFUD_LIB && DEBUG_PROGRAM == 0
  HAL_UART_DeInit(DEBUG_UART);
  HAL_SPI_DeInit(SPI_HANDLE);
  HAL_UART_Init(DEBUG_UART);
  HAL_SPI_Init(SPI_HANDLE);
  if(sfud_dev0 == NULL)
  {
    return 1;
  }
  uint8_t status;
  sfud_read_status(sfud_dev0, &status);

  if(sfud_write(sfud_dev0,  512 * 1024 * 3 + adr, sz, buf) != SFUD_SUCCESS)
  {
    return 1;
  }
#else

#endif
  PRINTF("ProgramPage 0x%08X Size %u.\r\n", adr, sz);
  return (0);                                  // Finished without Errors
}

/**
* @brief 校验
*
* @param adr 起始地址
* @param sz 数据大小
* @param buf 要校验的数据缓冲地址
* @return unsigned long 尾部数据地址
*/
unsigned long Verify(unsigned long adr, unsigned long sz, unsigned char *buf)
{
 adr -= SPI_FLASH_MEM_ADDR;
#if USE_SFUD_LIB && DEBUG_VERIFY == 0
  HAL_UART_DeInit(DEBUG_UART);
  HAL_SPI_DeInit(SPI_HANDLE);
  HAL_UART_Init(DEBUG_UART);
  HAL_SPI_Init(SPI_HANDLE);
  if(sfud_dev0 == NULL)
  {
    return 1;
  }
  uint8_t status;
  sfud_read_status(sfud_dev0, &status);

  if(sfud_read(sfud_dev0, 512 * 1024 * 3 + adr, sz, aux_buf) != SFUD_SUCCESS)
  {
    return 1;
  }

  for(int i = 0; i < sz; i++)
  {
    if(aux_buf[i] != buf[i])
    {
      return (adr + i);              /* 校验失败 */
    }
  }
#else

#endif
 PRINTF("Verify 0x%08X Size %u.\r\n", adr, sz);
 adr += SPI_FLASH_MEM_ADDR;
 return (adr + sz);                 /* 校验成功 */
}

/**
 * @brief Blank Check Checks if Memory is Blank
 *
 * @param adr Block Start Address
 * @param sz Block Size (in bytes)
 * @param pat Block Pattern
 * @return int 0 - OK,  1 - Failed (需要擦除）
 */
int BlankCheck(unsigned long adr, unsigned long sz, unsigned char pat)
{
  // adr -= SPI_FLASH_MEM_ADDR;
  PRINTF("BlankCheck 0x%08X Size %u Pat %hhu.\r\n", adr, sz, pat);
  /* 强制擦除 */
  return 1;
}
