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
 * File: $Id: port.c,v 1.60 2015/02/01 9:18:05 Armink $
 */

/* ----------------------- System includes --------------------------------*/

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
/* ----------------------- Variables ----------------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
void EnterCriticalSection(void) { taskENTER_CRITICAL(); }

void ExitCriticalSection(void) { taskEXIT_CRITICAL(); }
/*put  bytes in buff*/
void Put_in_fifo(Serial_fifo *buff, uint8_t *putdata, int length) {

  portDISABLE_INTERRUPTS();
  while (length--) {
    buff->buffer[buff->put_index] = *putdata;
    buff->put_index += 1;
    if (buff->put_index >= FIFO_SIZE_MAX)
      buff->put_index = 0;
    /* if the next position is read index, discard this 'read char' */
    if (buff->put_index == buff->get_index) {
      buff->get_index += 1;
      if (buff->get_index >= FIFO_SIZE_MAX)
        buff->get_index = 0;
    }
  }
  portENABLE_INTERRUPTS();
}
/*get  bytes from buff*/
int Get_from_fifo(Serial_fifo *buff, uint8_t *getdata, int length) {
  int size = length;
  /* read from software FIFO */
  while (length) {
    int ch;
    /* disable interrupt */
    portDISABLE_INTERRUPTS();
    if (buff->get_index != buff->put_index) {
      ch = buff->buffer[buff->get_index];
      buff->get_index += 1;
      if (buff->get_index >= FIFO_SIZE_MAX)
        buff->get_index = 0;
    } else {
      /* no data, enable interrupt and break out */
      portENABLE_INTERRUPTS();
      break;
    }
    *getdata = ch & 0xff;
    getdata++;
    length--;
    /* enable interrupt */
    portENABLE_INTERRUPTS();
  }
  return size - length;
}
/*判断是否进入在中断中*/
#ifndef IS_IRQ()
extern __asm uint32_t vPortGetIPSR(void); //调用FreeRTOS API
__inline bool IS_IRQ(void) //使用内联函数提高速度
{
  if (vPortGetIPSR()) {
    return TRUE;
  }
  return FALSE;
}
#endif // MACRO
