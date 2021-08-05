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
 * File: $Id: portevent.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "port.h"

/* ----------------------- Variables ----------------------------------------*/
static EventGroupHandle_t xSlaveOsEvent;
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortEventInit(void) {
  xSlaveOsEvent = xEventGroupCreate();
  if (xSlaveOsEvent != NULL) {
    MODBUS_DEBUG("xMBPortEventInit Success!\r\n");
  } else {
    MODBUS_DEBUG("xMBPortEventInit Faild !\r\n");
    return FALSE;
  }
  return TRUE;
}

BOOL xMBPortEventPost(eMBEventType eEvent) {
  BaseType_t flag;
  MODBUS_DEBUG("Post eEvent=%d!\r\n", eEvent);
  if (xSlaveOsEvent != NULL) {
    if (IS_IRQ()) {
      xEventGroupSetBitsFromISR(xSlaveOsEvent, eEvent, &flag);
    } else {
      xEventGroupSetBits(xSlaveOsEvent, eEvent);
    }
  }
  return TRUE;
}

BOOL xMBPortEventGet(eMBEventType *eEvent) {
  uint32_t recvedEvent;
  /* waiting forever OS event */
  recvedEvent = xEventGroupWaitBits(xSlaveOsEvent,
                                    EV_READY | EV_FRAME_RECEIVED | EV_EXECUTE |
                                        EV_FRAME_SENT, /* 接收任务感兴趣的事件
                                                        */
                                    pdTRUE,  /* 退出时清除事件?? */
                                    pdFALSE, /* 满足感兴趣的所有事?? */
                                    portMAX_DELAY); /* 指定超时事件,无限等待 */
  switch (recvedEvent) {
  case EV_READY:
    *eEvent = EV_READY;
    break;
  case EV_FRAME_RECEIVED:
    *eEvent = EV_FRAME_RECEIVED;
    break;
  case EV_EXECUTE:
    *eEvent = EV_EXECUTE;
    break;
  case EV_FRAME_SENT:
    *eEvent = EV_FRAME_SENT;
    break;
  }
  return TRUE;
}
