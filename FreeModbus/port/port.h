/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: port.h ,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#ifndef _PORT_H
#define _PORT_H
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "main.h"
#include "mbconfig.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>
#define INLINE
#define PR_BEGIN_EXTERN_C extern "C" {
#define PR_END_EXTERN_C }
/*IF DEBUG*/
#define DEBUG 1

#if DEBUG == 1
#define MODBUS_DEBUG(fmt, args...)                                             \
  fprintf(stderr, "  MODBUS_DEBUG(%s:%d):  \t" fmt, __func__, __LINE__, ##args)
#elif DEBUG == 0
#define MODBUS_DEBUG(fmt, args...)                                             \
  do {                                                                         \
  } while (0)
#endif

#define ENTER_CRITICAL_SECTION() EnterCriticalSection()
#define EXIT_CRITICAL_SECTION() ExitCriticalSection()
typedef struct _serial_fifo {
  /* software fifo */
  volatile uint8_t *buffer;
  volatile uint16_t put_index, get_index;
} Serial_fifo;
#define FIFO_SIZE_MAX 265
typedef bool BOOL;

typedef unsigned char UCHAR;
typedef char CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define SLAVE_RS485_TX_MODE HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET)
#define SLAVE_RS485_RX_MODE HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET)

#define MASTER_RS485_TX_MODE HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET)
#define MASTER_RS485_RX_MODE                                                   \
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET)

#define USING_UART3
#define USING_UART2
void EnterCriticalSection(void);
void ExitCriticalSection(void);
void Put_in_fifo(Serial_fifo *buff, uint8_t *putdata, int length);
int Get_from_fifo(Serial_fifo *buff, uint8_t *getdata, int length);
extern __inline bool IS_IRQ(void);
#endif
