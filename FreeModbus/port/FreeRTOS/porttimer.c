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
 * File: $Id: porttimer.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static TimerHandle_t timer;
static void prvvTIMERExpiredISR(void);
static void timer_timeout_ind(TIM_HandleTypeDef *xTimer);
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timerout50us)
{
    /*
    Freertos can't create timer in isr!
    So,I use hardware timer here! ！Freq=1Mhz
    */
    timer = xTimerCreate("Slave timer",
                         (50 * usTim1Timerout50us) / (1000 * 1000 / configTICK_RATE_HZ) + 1,
                         pdFALSE, (void *)2, timer_timeout_ind);
    return TRUE;
}

void vMBPortTimersEnable()
{
    MODBUS_DEBUG("Start slave timer!\r\n");
    if (IS_IRQ())
    {
        xTimerStartFromISR((TimerHandle_t)timer, 0);
    }
    else
    {
        xTimerStart((TimerHandle_t)timer, 0);
    }
}

void vMBPortTimersDisable()
{
    MODBUS_DEBUG("Stop timer!\r\n");
    if (IS_IRQ())
    {
        xTimerStopFromISR((TimerHandle_t)timer, 0);
    }
    else
    {
        xTimerStop((TimerHandle_t)timer, 0);
    }
}

void prvvTIMERExpiredISR(void)
{
    (void)pxMBPortCBTimerExpired();
}
static void timer_timeout_ind(TIM_HandleTypeDef *xTimer)
{
    MODBUS_DEBUG("Timer callback!\r\n");
    prvvTIMERExpiredISR();
}
