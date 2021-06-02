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
 * File: $Id: portevent_m.c v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "port.h"
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
/* ----------------------- Variables ----------------------------------------*/
/* Definitions for myBinarySem01 */
const osSemaphoreAttr_t xMasterRunRes_attributes = {
    .name = "xMasterRunRes"};
static SemaphoreHandle_t xMasterRunRes;
static osEventFlagsId_t xMasterOsEvent;
/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortEventInit(void)
{
    xMasterOsEvent = osEventFlagsNew(NULL);
    if (xMasterOsEvent == NULL)
    {
        MODBUS_DEBUG("xMBPortEventInit ERR=%d!\r\n", xMasterOsEvent);
    }else{
      MODBUS_DEBUG("xMBPortEventInit success!\r\n");
    }
    return TRUE;
}

BOOL xMBMasterPortEventPost(eMBMasterEventType eEvent)
{
    xEventGroupSetBits(xMasterOsEvent, eEvent);
    return TRUE;
}

BOOL xMBMasterPortEventGet(eMBMasterEventType *eEvent)
{
    uint32_t recvedEvent;
    /* waiting forever OS event */
    recvedEvent = xEventGroupWaitBits(xMasterOsEvent, /* 事件对象句柄 */
                                      EV_MASTER_READY | EV_MASTER_FRAME_RECEIVED | EV_MASTER_EXECUTE |
                                          EV_MASTER_FRAME_SENT | EV_MASTER_ERROR_PROCESS, /* 接收任务感兴趣的事件 */
                                      pdTRUE,                                             /* 退出时清除事件位 */
                                      pdFALSE,                                            /* 满足感兴趣的某一件事件 */
                                      portMAX_DELAY);                                     /* 指定超时事件,无限等待 */
    /* the enum type couldn't convert to int type */
    switch (recvedEvent)
    {
    case EV_MASTER_READY:
        *eEvent = EV_MASTER_READY;
        break;
    case EV_MASTER_FRAME_RECEIVED:
        *eEvent = EV_MASTER_FRAME_RECEIVED;
        break;
    case EV_MASTER_EXECUTE:
        *eEvent = EV_MASTER_EXECUTE;
        break;
    case EV_MASTER_FRAME_SENT:
        *eEvent = EV_MASTER_FRAME_SENT;
        break;
    case EV_MASTER_ERROR_PROCESS:
        *eEvent = EV_MASTER_ERROR_PROCESS;
        break;
    }
    return TRUE;
}
/**
 * This function is initialize the OS resource for modbus master.
 * Note:The resource is define by OS.If you not use OS this function can be empty.
 *
 */
void vMBMasterOsResInit(void)
{
    //rt_sem_init(&xMasterRunRes, "master res", 0x01, RT_IPC_FLAG_PRIO);
    xMasterRunRes = osSemaphoreNew(1, 1, &xMasterRunRes_attributes);
}

/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
BOOL xMBMasterRunResTake(LONG lTimeOut)
{
    /*If waiting time is -1 .It will wait forever */
    //return rt_sem_take(&xMasterRunRes, lTimeOut) ? FALSE : TRUE;
    if (lTimeOut == -1)
    {
        return osSemaphoreAcquire(xMasterRunRes, lTimeOut) ? FALSE : TRUE;
    }

    return osSemaphoreAcquire(xMasterRunRes, lTimeOut) ? FALSE : TRUE;
}

/**
 * This function is release Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be empty.
 *
 */
void vMBMasterRunResRelease(void)
{
    /* release resource */
    //rt_sem_release(&xMasterRunRes);
    osSemaphoreRelease(xMasterRunRes);
}

/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                    USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    osEventFlagsSet(xMasterOsEvent, EV_MASTER_ERROR_RESPOND_TIMEOUT);
    /* You can add your code under here. */
}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                 USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    //rt_event_send(&xMasterOsEvent, EV_MASTER_ERROR_RECEIVE_DATA);
    osEventFlagsSet(xMasterOsEvent, EV_MASTER_ERROR_RECEIVE_DATA);
    /* You can add your code under here. */
}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(UCHAR ucDestAddress, const UCHAR *pucPDUData,
                                     USHORT ucPDULength)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    //rt_event_send(&xMasterOsEvent, EV_MASTER_ERROR_EXECUTE_FUNCTION);
    osEventFlagsSet(xMasterOsEvent, EV_MASTER_ERROR_EXECUTE_FUNCTION);
    /* You can add your code under here. */
}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestScuuess(void)
{
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    //rt_event_send(&xMasterOsEvent, EV_MASTER_PROCESS_SUCESS);
    osEventFlagsSet(xMasterOsEvent, EV_MASTER_PROCESS_SUCESS);
    /* You can add your code under here. */
}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish(void)
{
    eMBMasterReqErrCode eErrStatus = MB_MRE_NO_ERR;
    uint32_t recvedEvent;
    /* waiting for OS event */

    osEventFlagsWait(xMasterOsEvent, /* 事件对象句柄 */
                     EV_MASTER_PROCESS_SUCESS | EV_MASTER_ERROR_RESPOND_TIMEOUT | EV_MASTER_ERROR_RECEIVE_DATA |
                         EV_MASTER_ERROR_EXECUTE_FUNCTION,
                     /* 接收任务感兴趣的事件 */
                     osFlagsWaitAny, /* 退出时清除事件位 */
                                     /* 满足感兴趣的某一件事件 */
                     portMAX_DELAY); /* 指定超时事件,无限等待 */
    /* the enum type couldn't convert to int type */
    switch (recvedEvent)
    {
    case EV_MASTER_PROCESS_SUCESS:
        break;
    case EV_MASTER_ERROR_RESPOND_TIMEOUT:
    {
        eErrStatus = MB_MRE_TIMEDOUT;
        break;
    }
    case EV_MASTER_ERROR_RECEIVE_DATA:
    {
        eErrStatus = MB_MRE_REV_DATA;
        break;
    }
    case EV_MASTER_ERROR_EXECUTE_FUNCTION:
    {
        eErrStatus = MB_MRE_EXE_FUN;
        break;
    }
    }
    return eErrStatus;
}

#endif
