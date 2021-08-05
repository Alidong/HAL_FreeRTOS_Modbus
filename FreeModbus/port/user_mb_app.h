#ifndef USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"

/* -----------------------Slave Defines -------------------------------------*/
#define S_DISCRETE_INPUT_START 0
#define S_DISCRETE_INPUT_NDISCRETES 64
#define S_COIL_START 0
#define S_COIL_NCOILS 64
#define S_REG_INPUT_START 0
#define S_REG_INPUT_NREGS 100
#define S_REG_HOLDING_START 0
#define S_REG_HOLDING_NREGS 100
/* salve mode: holding register's all address */
#define S_HD_RESERVE 0
#define S_HD_CPU_USAGE_MAJOR 1
#define S_HD_CPU_USAGE_MINOR 2
/* salve mode: input register's all address */
#define S_IN_RESERVE 0
/* salve mode: coil's all address */
#define S_CO_RESERVE 0
/* salve mode: discrete's all address */
#define S_DI_RESERVE 0
typedef struct _slave_data
{
    UCHAR *ucSDiscInBuf;
    UCHAR *ucSCoilBuf;
    USHORT *usSRegInBuf;
    USHORT *usSRegHoldBuf;
} Slave_data;
extern Slave_data Slave_station;

/* -----------------------Master Defines -------------------------------------*/
#define M_DISCRETE_INPUT_START 0
#define M_DISCRETE_INPUT_NDISCRETES 64
#define M_COIL_START 0
#define M_COIL_NCOILS 64
#define M_REG_INPUT_START 0
#define M_REG_INPUT_NREGS 100
#define M_REG_HOLDING_START 0
#define M_REG_HOLDING_NREGS 100
/* master mode: holding register's all address */
#define M_HD_RESERVE 0
/* master mode: input register's all address */
#define M_IN_RESERVE 0
/* master mode: coil's all address */
#define M_CO_RESERVE 0
/* master mode: discrete's all address */
#define M_DI_RESERVE 0
typedef struct _master_data
{
#if M_DISCRETE_INPUT_NDISCRETES % 8
    UCHAR(*ucMDiscInBuf)
    [M_DISCRETE_INPUT_NDISCRETES / 8 + 1];
#else
    UCHAR(*ucMDiscInBuf)
    [M_DISCRETE_INPUT_NDISCRETES / 8];
#endif
#if M_COIL_NCOILS % 8
    UCHAR(*ucMCoilBuf)
    [M_COIL_NCOILS / 8 + 1];
#else
    UCHAR(*ucMCoilBuf)
    [M_COIL_NCOILS / 8];
#endif

    USHORT(*usMRegInBuf)
    [M_REG_INPUT_NREGS];
    USHORT(*usMRegHoldBuf)
    [M_REG_HOLDING_NREGS];
} Master_data;
extern Master_data Master_station;
#endif
