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
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */
#include "port.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Static variables ---------------------------------*/
/* software simulation serial transmit IRQ handler thread */
static TaskHandle_t thread_serial_soft_trans_irq = NULL;
/* serial event */
static osEventFlagsId_t event_serial;
/* modbus slave serial device */
static UART_HandleTypeDef *serial;
/*
 * Serial FIFO mode 
 */

static volatile uint8_t rx_buff[FIFO_SIZE_MAX];
static Serial_fifo Slave_serial_rx_fifo;
/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START (1 << 0)

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
static void serial_soft_trans_irq(void *parameter);
static void Slave_TxCpltCallback(struct __UART_HandleTypeDef *huart);
static void Slave_RxCpltCallback(struct __UART_HandleTypeDef *huart);
static int stm32_getc(void);
static int stm32_putc(CHAR c);
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
                       eMBParity eParity)
{
    /**
     * set 485 mode receive and transmit control IO
     * @note MODBUS_SLAVE_RT_CONTROL_PIN_INDEX need be defined by user
     */
    //rt_pin_mode(MODBUS_SLAVE_RT_CONTROL_PIN_INDEX, PIN_MODE_OUTPUT);

    /* set serial name */
    if (ucPORT == 1)
    {
#if defined(USING_UART1)
        extern UART_HandleTypeDef huart1;
        serial = &huart1;
        MODBUS_DEBUG("Slave using uart1!\r\n");
        
#endif
    }
    else if (ucPORT == 2)
    {
#if defined(USING_UART2)
        extern UART_HandleTypeDef huart2;
        serial = &huart2;
        MODBUS_DEBUG("Slave using uart2!\r\n");

#endif
    }
    else if (ucPORT == 3)
    {
#if defined(USING_UART3)
      extern UART_HandleTypeDef huart3;
        serial = &huart3;
        MODBUS_DEBUG("Slave using uart3!\r\n");
#endif
    }
    /* set serial configure */
    serial->Init.BaudRate = ulBaudRate;
    serial->Init.StopBits = UART_STOPBITS_1;
    switch (eParity)
    {
    case MB_PAR_NONE:
    {
        serial->Init.WordLength = UART_WORDLENGTH_8B;
        serial->Init.Parity = UART_PARITY_NONE;
        break;
    }
    case MB_PAR_ODD:
    {
        serial->Init.WordLength = UART_WORDLENGTH_9B;
        serial->Init.Parity = UART_PARITY_ODD;
        break;
    }
    case MB_PAR_EVEN:
    {
        serial->Init.WordLength = UART_WORDLENGTH_9B;
        serial->Init.Parity = UART_PARITY_EVEN;
        break;
    }
    }
    if (HAL_UART_Init(serial) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(serial, UART_IT_TC);
    /*registe recieve callback*/
    HAL_UART_RegisterCallback(serial, HAL_UART_RX_COMPLETE_CB_ID, Slave_RxCpltCallback);
    /* software initialize */
    Slave_serial_rx_fifo.buffer = rx_buff;
    Slave_serial_rx_fifo.get_index = 0;
    Slave_serial_rx_fifo.put_index = 0;

    /* 创建主站发送任务 */
    BaseType_t xReturn = xTaskCreate((TaskFunction_t)serial_soft_trans_irq,          /* 任务入口函数 */
                                     (const char *)"slave trans",                    /* 任务名字 */
                                     (uint16_t)512,                                  /* 任务栈大小 */
                                     (void *)NULL,                                   /* 任务入口函数参数 */
                                     (UBaseType_t)5,                                 /* 任务的优先级 */
                                     (TaskHandle_t *)&thread_serial_soft_trans_irq); /*任务控制块指针 */

    if (xReturn == pdPASS)
    {
        MODBUS_DEBUG("xTaskCreate slave trans success\r\n");
    }
    return TRUE;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        /* enable RX interrupt */
        __HAL_UART_ENABLE_IT(serial, UART_IT_RXNE);
        /* switch 485 to receive mode */
        MODBUS_DEBUG("RS485_RX_MODE\r\n");
        SLAVE_RS485_RX_MODE;
    }
    else
    {
        /* switch 485 to transmit mode */
        MODBUS_DEBUG("RS485_TX_MODE\r\n");
        SLAVE_RS485_TX_MODE;
        /* disable RX interrupt */
        __HAL_UART_DISABLE_IT(serial, UART_IT_RXNE);
    }
    if (xTxEnable)
    {
        /* start serial transmit */
        osEventFlagsSet(event_serial, EVENT_SERIAL_TRANS_START);
    }
    else
    {
        /* stop serial transmit */
        osEventFlagsClear(event_serial, EVENT_SERIAL_TRANS_START);
        printf("ms=%.2f,fps=%.2f\r\n",__HAL_TIM_GetCounter(&htim7)/100.f,1000.f/(__HAL_TIM_GetCounter(&htim7)/100.f));
    }
}

void vMBPortClose(void)
{
    __HAL_UART_DISABLE(serial);
}
/*Send a byte*/
BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    stm32_putc(ucByte);
    return TRUE;
}
/*Get a byte from fifo*/
BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    Get_from_fifo(&Slave_serial_rx_fifo, (uint8_t *)pucByte, 1);
    return TRUE;
}

/*
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

/*
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
    pxMBFrameCBByteReceived();
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
static void serial_soft_trans_irq(void *parameter)
{
    uint32_t recved_event;
    event_serial = osEventFlagsNew(NULL); //创建事件
    if (NULL != event_serial)
    {
        MODBUS_DEBUG("Slave event_serial Event creat success!\r\n");
    }
    else
    {
        MODBUS_DEBUG("Slave event_serial Event creat faild err=%d\r\n", event_serial);
    }
    while (1)
    {
        /* waiting for serial transmit start */
        recved_event = osEventFlagsWait(event_serial, EVENT_SERIAL_TRANS_START, osFlagsWaitAny | osFlagsNoClear, osWaitForever);
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
void Slave_RxCpltCallback(UART_HandleTypeDef *huart)
{
  __HAL_TIM_SET_COUNTER(&htim7,0);
    int ch = -1;
    /*UART RX非空中断调用，并获取一帧数据*/
    while (1)
    {
        ch = stm32_getc();
        if (ch == -1)
            break;
        Put_in_fifo(&Slave_serial_rx_fifo, (uint8_t *)&ch, 1);
    }
    prvvUARTRxISR();
}
/*UART发送一个数据*/
static int stm32_putc(CHAR c)
{
    serial->Instance->DR = c;
    while (!(serial->Instance->SR & UART_FLAG_TC))
        ;
    return TRUE;
}
/*UART接收一个数据*/
static int stm32_getc(void)
{
    int ch;
    ch = -1;
    if (serial->Instance->SR & UART_FLAG_RXNE)
    {
        ch = serial->Instance->DR & 0xff;
    }
    return ch;
}