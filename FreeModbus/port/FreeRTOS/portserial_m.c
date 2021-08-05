/*
 * FreeModbus Libary: RT-Thread Port
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
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master
 * Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/
static volatile uint8_t rx_buff[FIFO_SIZE_MAX];
static Serial_fifo Master_serial_rx_fifo;
/* software simulation serial transmit IRQ handler thread stack */
/* software simulation serial transmit IRQ handler thread */
static TaskHandle_t thread_serial_soft_trans_irq = NULL; /* 创建任务句柄 */
/* serial event */
static EventGroupHandle_t event_serial = NULL;
/* modbus master serial device */
static UART_HandleTypeDef *serial;

/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START (1 << 0)

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
static void serial_soft_trans_irq(void *parameter);
static void Master_TxCpltCallback(struct __UART_HandleTypeDef *huart);
static void Master_RxCpltCallback(struct __UART_HandleTypeDef *huart);
static int stm32_getc(void);
static int stm32_putc(CHAR c);
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
                             eMBParity eParity) {
  /**
   * set 485 mode receive and transmit control IO
   * @note MODBUS_MASTER_RT_CONTROL_PIN_INDEX need be defined by user
   */

  /* set serial name */
  if (ucPORT == 1) {
#if defined(USING_UART1)
    extern UART_HandleTypeDef huart1;
    serial = &huart1;
    MODBUS_DEBUG("Slave using uart1!\r\n");
#endif
  } else if (ucPORT == 2) {
#if defined(USING_UART2)
    extern UART_HandleTypeDef huart2;
    serial = &huart2;
    MODBUS_DEBUG("Master using uart2!\r\n");

#endif
  } else if (ucPORT == 3) {
#if defined(USING_UART3)
    extern UART_HandleTypeDef huart3;
    serial = &huart3;
    MODBUS_DEBUG("Master using uart3!\r\n");
#endif
  }
  /* set serial configure parameter */
  /* set serial configure */
  serial->Init.BaudRate = ulBaudRate;
  serial->Init.StopBits = UART_STOPBITS_1;
  switch (eParity) {
  case MB_PAR_NONE: {
    serial->Init.WordLength = UART_WORDLENGTH_8B;
    serial->Init.Parity = UART_PARITY_NONE;
    break;
  }
  case MB_PAR_ODD: {
    serial->Init.WordLength = UART_WORDLENGTH_9B;
    serial->Init.Parity = UART_PARITY_ODD;
    break;
  }
  case MB_PAR_EVEN: {
    serial->Init.WordLength = UART_WORDLENGTH_9B;
    serial->Init.Parity = UART_PARITY_EVEN;
    break;
  }
  }
  if (HAL_UART_Init(serial) != HAL_OK) {
    Error_Handler();
  }
  __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
  __HAL_UART_DISABLE_IT(serial, UART_IT_TC);
  /*registe recieve callback*/
  HAL_UART_RegisterCallback(serial, HAL_UART_RX_COMPLETE_CB_ID,
                            Master_RxCpltCallback);
  /* software initialize */
  Master_serial_rx_fifo.buffer = rx_buff;
  Master_serial_rx_fifo.get_index = 0;
  Master_serial_rx_fifo.put_index = 0;
  /* software initialize */

  /* 创建主站发送任�?? */
  event_serial = xEventGroupCreate();
  if (NULL != event_serial) {
    MODBUS_DEBUG("Master create event_serial success! event_serial=%d\r\n",
                 event_serial);
  } else {
    MODBUS_DEBUG("Master create event_serial Failed!\r\n");
  }
  BaseType_t xReturn = pdPASS;
  xReturn =
      xTaskCreate((TaskFunction_t)serial_soft_trans_irq, /* 任务入口函数 */
                  (const char *)"master trans",          /* 任务名字 */
                  (uint16_t)128,                         /* 任务栈大�?? */
                  (void *)NULL,    /* 任务入口函数参数 */
                  (UBaseType_t)12, /* 任务的优先级 */
                  NULL);           /*任务控制块指�?? */
  if (xReturn == pdPASS) {
    MODBUS_DEBUG("xTaskCreate Master trans success\r\n");
  } else {
    MODBUS_DEBUG("xTaskCreate Master trans faild!\r\n");
    return FALSE;
  }

  return TRUE;
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable) {
  /*这一步不能省略，需要提前清理掉标志位，否则接收会有问题*/
  __HAL_UART_CLEAR_FLAG(serial,UART_FLAG_RXNE);
   __HAL_UART_CLEAR_FLAG(serial,UART_FLAG_TC);
  if (xRxEnable == pdTRUE) {
    MODBUS_DEBUG("RS485_RX_MODE\r\n");
   /* switch 485 to receive mode */
    MASTER_RS485_RX_MODE;
    /* enable RX interrupt */
    __HAL_UART_ENABLE_IT(serial, UART_IT_RXNE);

  } else {
    MODBUS_DEBUG("RS485_TX_MODE\r\n");
    /* switch 485 to transmit mode */
    MASTER_RS485_TX_MODE;
    /* disable RX interrupt */
    __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
  }
  if (xTxEnable == pdTRUE) {
    /* start serial transmit */
    xEventGroupSetBits(event_serial, EVENT_SERIAL_TRANS_START);
  } else {
    xEventGroupClearBits(event_serial, EVENT_SERIAL_TRANS_START);
  }
}

void vMBMasterPortClose(void) {
  __HAL_UART_DISABLE(serial);
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte) {
  stm32_putc(ucByte);
  return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR *pucByte) {
  Get_from_fifo(&Master_serial_rx_fifo, (uint8_t *)pucByte, 1);
  return TRUE;
}

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void) { pxMBMasterFrameCBTransmitterEmpty(); }

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void) { pxMBMasterFrameCBByteReceived(); }

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void serial_soft_trans_irq(void *parameter) {

  while (1) {
    /* waiting for serial transmit start */
    xEventGroupWaitBits(event_serial,             /* 事件对象句柄 */
                        EVENT_SERIAL_TRANS_START, /* 接收任务感兴趣的事件 */
                        pdFALSE,        /* 退出时清除事件�?? */
                        pdFALSE,        /* 满足感兴趣的所有事�?? */
                        portMAX_DELAY); /* 指定超时事件,无限等待 */
    /* execute modbus callback */
    prvvUARTTxReadyISR();
  }
}

/**
 * @brief  Rx Transfer completed callbacks.
 * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
 *                the configuration information for the specified UART module.
 * @retval None
 */
static void Master_RxCpltCallback(struct __UART_HandleTypeDef *huart) {
  int ch = -1;
  /*UART RX非空中断调用，并获取一帧数�??*/
  while (1) {
    ch = stm32_getc();
    if (ch == -1)
      break;
    Put_in_fifo(&Master_serial_rx_fifo, (uint8_t *)&ch, 1);
  }
  prvvUARTRxISR();
}
/*UART发送一个数据*/
static int stm32_putc(CHAR c) {
  serial->Instance->DR = c;
  while (!(serial->Instance->SR & UART_FLAG_TC))
    ;
  return TRUE;
}
/*UART接收一个数据*/
static int stm32_getc(void) {
  int ch;
  ch = -1;
  if (serial->Instance->SR & UART_FLAG_RXNE) {
    ch = serial->Instance->DR & 0xff;
  }
  return ch;
}
#endif
