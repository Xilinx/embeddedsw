/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file I3c.h
* @addtogroup Overview
* @{
* @details
*
* This is an implementation of I3C driver in the block. The device can
* be either a master or a slave on the I3C bus. This implementation supports
* both interrupt mode transfer and polled mode transfer.
*
* The higher level software must implement a higher layer protocol to inform
* the slave what to send to the master.
*
* <b>Initialization & Configuration</b>
*
* The XI3c_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed by
* various operating systems, the driver instance can be initialized in the
* following way:
*
*    - XI3c_LookupConfig(DeviceId) - Use the device identifier to find
*      the static configuration structure defined in XI3c_g.c. This is
*      setup by the tools. For some operating systems the config structure
*      will be initialized by the software and this call is not needed.
*
*    - XI3c_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*      configuration structure provided by the caller. If running in a
*      system with address translation, the provided virtual memory base
*      address replaces the physical address in the configuration
*      structure.
*
* Multiple slaves are supported by selecting them with unique addresses.
*
* <b>Polled Mode Operation</b>
*
* This driver supports polled mode transfers.
*
* <b>Interrupts</b>
*
* The user must connect the interrupt handler of the driver,
* XI3c_InterruptHandler to an interrupt system such that it will be called
* when an interrupt occurs. This function does not save and restore the
* processor context such that the user must provide this processing.
*
* The driver handles the following interrupts:
* - Transfer complete
* - More Data
* - Error
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------
* 1.00  gm  02/09/24 First release
* 1.1   gm  10/07/24 Added XI3c_GetRevisionNumber() for reading revision
* 		      number.
* </pre>
*
******************************************************************************/

#ifndef XI3C_H       /* prevent circular inclusions */
#define XI3C_H       /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xi3c_hw.h"
#include "xplatform_info.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

#define XI3C_MAXDAACOUNT		108
#define XI3C_MAXDATA_LENGTH		4095
#define XI3C_SLAVEINFO_READ_BYTECOUNT	9
#define TIMEOUT_COUNTER			2000000U /**< Wait for 2 sec in worst case */
#define XI3C_BROADCAST_ADDRESS		0x7E
#define WORD_TO_BYTE			4
#define XI3C_CEIL_DIV(x, y)		(((x) + (y) - 1) / (y))

/* Broadcast commands */
#define XI3C_CCC_BRDCAST_ENEC		0x0
#define XI3C_CCC_BRDCAST_DISEC		0x1
#define XI3C_CCC_BRDCAST_ENTAS		0x2
#define XI3C_CCC_BRDCAST_ENTAS1		0x3
#define XI3C_CCC_BRDCAST_ENTAS2		0x4
#define XI3C_CCC_BRDCAST_ENTAS3		0x5
#define XI3C_CCC_BRDCAST_RSTDAA		0x6
#define XI3C_CCC_BRDCAST_ENTDAA		0x7
#define XI3C_CCC_BRDCAST_DEFTGTS	0x8
#define XI3C_CCC_BRDCAST_SETMWL		0x9
#define XI3C_CCC_BRDCAST_SETMRL		0xa
#define XI3C_CCC_BRDCAST_ENTTM		0xb
#define XI3C_CCC_BRDCAST_SETBUSCON	0xc

#define XI3C_CCC_BRDCAST_ENDXFER	0x12

#define XI3C_CCC_BRDCAST_SETAASA	0x29

#define XI3C_CCC_BRDCAST_DEVCTRL	0x62

/* Unicast commands */
#define XI3C_CCC_ENEC			0x80
#define XI3C_CCC_DISEC			0x81
#define XI3C_CCC_ENTAS0			0x82
#define XI3C_CCC_ENTAS1			0x83
#define XI3C_CCC_ENTAS2			0x84
#define XI3C_CCC_ENTAS3			0x85
#define XI3C_CCC_SETDASA		0x87
#define XI3C_CCC_SETNEWDA		0x88
#define XI3C_CCC_SETMWL			0x89
#define XI3C_CCC_SETMRL			0x8a
#define XI3C_CCC_GETMWL			0x8b
#define XI3C_CCC_GETMRL			0x8c
#define XI3C_CCC_GETPID			0x8d
#define XI3C_CCC_GETBCR			0x8e
#define XI3C_CCC_GETDCR			0x8f
#define XI3C_CCC_GETSTATUS		0x90

/*****************************************************************************/
/**
* Checks whether the I3C bus is busy.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*               s32 XI3c_BusIsBusy(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_BusIsBusy(BaseAddress)					     \
	(XI3c_ReadReg((BaseAddress), XI3C_SR_OFFSET) & XI3C_SR_BUS_BUSY_MASK)

/*
 * Clock configurations
 */

/*****************************************************************************/
/**
*
* @brief
* Sets scl high time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is scl high time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetSclHighTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetSclHighTime(InstancePtr, Val)				\
	XI3c_WriteReg(InstancePtr->Config.BaseAddress,			\
		      XI3C_SCL_HIGH_TIME_OFFSET,			\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets scl high time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetSclHighTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetSclHighTime(InstancePtr)				     \
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			     \
		       XI3C_SCL_HIGH_TIME_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets scl low time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is scl low time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetSclLowTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetSclLowTime(InstancePtr, Val)				\
	XI3c_WriteReg(InstancePtr->Config.BaseAddress,			\
		      XI3C_SCL_LOW_TIME_OFFSET,				\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets scl low time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetSclLowTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetSclLowTime(InstancePtr)					    \
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			    \
		       XI3C_SCL_LOW_TIME_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets sda hold time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is sda hold time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetSdaHoldTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetSdaHoldTime(InstancePtr, Val)				\
	XI3c_WriteReg(InstancePtr->Config.BaseAddress,			\
		      XI3C_SDA_HOLD_TIME_OFFSET,			\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets sda hold time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetSdaHoldTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetSdaHoldTime(InstancePtr)				     \
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			     \
		       XI3C_SDA_HOLD_TIME_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets bus idle time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is bus idle time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetBusIdleTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetBusIdleTime(InstancePtr, Val)				    \
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_BUS_IDLE_OFFSET,\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets bus idle time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetBusIdleTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetBusIdleTime(InstancePtr)				\
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			\
		       XI3C_BUS_IDLE_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets Tsu Start time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is Tsu Start time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetTsuStartTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetTsuStartTime(InstancePtr, Val)				     \
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_TSU_START_OFFSET,\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets Tsu Start time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetTsuStartTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetTsuStartTime(InstancePtr)				 \
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			 \
		       XI3C_TSU_START_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets Thd Start time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is Thd Start time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetThdStartTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetThdStartTime(InstancePtr, Val)				     \
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_THD_START_OFFSET,\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* This function gets Thd Start time.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetThdStartTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetThdStartTime(InstancePtr)				\
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			\
		       XI3C_THD_START_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets Tsu Stop time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is Tsu Stop time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetTsuStopTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetTsuStopTime(InstancePtr, Val)				    \
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_TSU_STOP_OFFSET,\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets Tsu Stop time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetTsuStopTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetTsuStopTime(InstancePtr)				\
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			\
		       XI3C_TSU_STOP_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets Scl open drain high time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is Scl open drain high time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetSclOdHighTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetSclOdHighTime(InstancePtr, Val)				\
	XI3c_WriteReg(InstancePtr->Config.BaseAddress,			\
		      XI3C_OD_SCL_HIGH_TIME_OFFSET,			\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets Scl open drain high time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetSclOdHighTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetSclOdHighTime(InstancePtr)				 \
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			 \
		       XI3C_OD_SCL_HIGH_TIME_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Sets Scl open drain low time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Val is Scl open drain low time value to be set.
*
* @return       None.
*
* @note         Caller need to update other timing parameters if required.
*		C-style signature:
*		void XI3c_SetSclOdLowTime(XI3c *InstancePtr, u32 Val)
*
******************************************************************************/
#define XI3c_SetSclOdLowTime(InstancePtr, Val)				\
	XI3c_WriteReg(InstancePtr->Config.BaseAddress,			\
		      XI3C_OD_SCL_LOW_TIME_OFFSET,			\
		      (Val & XI3C_18BITS_MASK))

/*****************************************************************************/
/**
*
* @brief
* Gets Scl open drain low time of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetSclOdLowTime(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetSclOdLowTime(InstancePtr)				\
	((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			\
		       XI3C_OD_SCL_LOW_TIME_OFFSET)) & XI3C_18BITS_MASK)

/*****************************************************************************/
/**
*
* @brief
* Gets Core Revision number of I3C.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         C-style signature:
*		u32 XI3c_GetRevisionNumber(XI3c *InstancePtr)
*
******************************************************************************/
#define XI3c_GetRevisionNumber(InstancePtr)					\
	(((XI3c_ReadReg(InstancePtr->Config.BaseAddress,			\
		       XI3C_VERSION_OFFSET)) & XI3C_CORE_REVISION_NUM_MASK)	\
		       >> XI3C_CORE_REVISION_NUM_SHIFT)

/**************************** Type Definitions *******************************/

/**
* The handler data type allows the user to define a callback function to
* respond to interrupt events in the system. This function is executed
* in interrupt context, so amount of processing should be minimized.
*
* @param	CallBackRef is the callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked. Its type is
*		not important to the driver, so it is a void pointer.
* @param	StatusEvent indicates one or more status events that occurred.
*/
typedef void (*XI3c_IntrHandler) (u32 StatusEvent);

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;     /**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;  /**< Base address of the device */
#ifdef SDT
	u16 IntrId;		/**< Bits[11:0] Interrupt-id Bits[15:12]
				 * trigger type and level flags */
	UINTPTR IntrParent;	/**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address */
#endif
	u32 InputClockHz;	/**< Input clock frequency */
	u8 RwFifoDepth;		/**< Read and write fifo depth */
	u8 WrThreshold;		/**< Write fifo programmable threshold byte count */
	u8 DeviceCount;		/**< Number of devices connected */
	u8 IbiCapable;		/**< IBI Capability */
	u8 HjCapable;		/**< Hot Join Capability */
} XI3c_Config;

typedef struct {
	u8 CmdType;		/**< Cmd type: 0 - Legacy I2C, 1 - SDR */
	u8 NoRepeatedStart;	/**< Repeated start or stop on completion */
	u8 Pec;			/**< Parity Error Check */
	u8 SlaveAddr;		/**< Slave device address */
	u8 Rw;			/**< Read - 1 or Write - 0 */
	u16 ByteCount;		/**< No of bytes to send/recv */
	u8 Tid;			/**< Transaction ID */
} XI3c_Cmd;

/**
 * The XI3c slave data. XI3c driver reads from slave devices during DAA
 * and fills these values.
 */
typedef struct {
	u8 DynaAddr;	/**< Dynamic Address */
	u64 Id;		/**< Slave Id */
	u8 Bcr;		/**< Bus Characteristic Register */
	u8 Dcr;		/**< Device Characteristic Register */
} XI3c_SlaveInfo;

/**
 * The XI3c driver instance data. The user is required to allocate a
 * variable of this type for each I3C device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XI3c_Config Config;	/**< Configuration structure */
	u32 IsReady;		/**< Device is initialized and ready */
	u8 *SendBufferPtr;	/**< Pointer to send buffer */
	u8 *RecvBufferPtr;	/**< Pointer to recv buffer */
	u16 SendByteCount;	/**< Number of bytes still expected to send */
	u16 RecvByteCount;	/**< Number of bytes still expected to receive */
	u8 Error;		/**< Error value */
	u8 CurDeviceCount;		/**< Current number of devices on the bus */
	XI3c_IntrHandler StatusHandler;	/**< Event handler function */
	XI3c_SlaveInfo XI3c_SlaveInfoTable[XI3C_MAXDAACOUNT]; /**< Slave info table */
} XI3c;

/************************** Variable Definitions *****************************/
extern XI3c_Config XI3c_ConfigTable[];	/**< Configuration table */

extern u8 XI3C_DynaAddrList[];

/************************** Function Prototypes ******************************/

/*
 * Function for configuration lookup, in XI3c_sinit.c
 */
#ifndef SDT
XI3c_Config *XI3c_LookupConfig(u16 DeviceId);
#else
XI3c_Config *XI3c_LookupConfig(u32 BaseAddress);
#endif

/*
 * Functions for general setup, in XI3c.c
 */
s32 XI3c_CfgInitialize(XI3c *InstancePtr, XI3c_Config *ConfigPtr,
			  u32 EffectiveAddr);
void XI3c_FillCmdFifo(XI3c *InstancePtr, XI3c_Cmd *Cmd);
void XI3c_WriteTxFifo(XI3c *InstancePtr);
void XI3c_ReadRxFifo(XI3c *InstancePtr);
s32 XI3c_SendTransferCmd(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 Data);
s32 XI3c_DynaAddrAssign(XI3c *InstancePtr, u8 DynaAddr[], u8 DevCount);
s32 XI3c_SetSClk(XI3c *InstancePtr, u32 SclkHz, u8 Mode);
void XI3C_BusInit(XI3c *InstancePtr);

/*
 * Functions for interrupts, in XI3c_master.c
 */
void XI3c_SetStatusHandler(XI3c *InstancePtr, XI3c_IntrHandler FunctionPtr);

/*
 * Functions for device as master, in XI3c_master.c
 */

s32 XI3c_MasterSend(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount);
s32 XI3c_MasterRecv(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount);
s32 XI3c_MasterSendPolled(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount);
s32 XI3c_MasterRecvPolled(XI3c *InstancePtr, XI3c_Cmd *Cmd, u8 *MsgPtr, u16 ByteCount);
void XI3c_MasterInterruptHandler(XI3c *InstancePtr);
s32 XI3c_IbiRecv(XI3c *InstancePtr, u8 *MsgPtr);
s32 XI3c_IbiRecvPolled(XI3c *InstancePtr, u8 *MsgPtr);

/************************** Inline Function Definitions **********************/
/*****************************************************************************/
/**
*
* @brief
* Enables/Disables the I3C. To execute I3C transactions this has to  be
* Enabled. Any data transfer that is in progress is aborted when disabled and
* no further I3C transactions will happen till I3C enabled.
*
* @param        InstancePtr is a pointer to the XI3c instance.
* @param        Enable is flag indicates to enable or disable the I3C.
*
* @return       None.
*
* @note         None.
*
******************************************************************************/
static inline void XI3c_Enable(XI3c *InstancePtr,u8 Enable)
{
        u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

        Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET);
        Data &= ~XI3C_CR_EN_MASK;
        Data |= Enable;
        XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* @brief
* Abort allows controller to kill existing transcation.
* In response Abort request, Controller issues a stop condtion after existing
* transcation is finished. Abort bit is auto cleared after Abort is completed.
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XI3c_Abort(XI3c *InstancePtr)
{
        u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

        Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET);
        Data |= XI3C_CR_ABORT_MASK;
        XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* @brief
* Resume allows controller to resume the contoller from Abort/Error State.
* Resume bit will be auto cleared once controller resumes the operation.
* After Controller goes to Abort/Eroor State. User should handle the necesarry
* action to Abort/Error state (ex: Clearing FIFOs etc)
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XI3c_Resume(XI3c *InstancePtr)
{
        u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

        Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET);
        Data |= XI3C_CR_RESUME_MASK;
        XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* @brief
* Enable IBI capability
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XI3c_EnableIbi(XI3c *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET);
	Data |= XI3C_CR_IBI_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* @brief
* Enable Hot Join capability
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XI3c_EnableHotjoin(XI3c *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET);
	Data |= XI3C_CR_HJ_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_CR_OFFSET, Data);
}

/*****************************************************************************/
/**
*
* @brief
* Update the slave address and BCR register values of available device
* to the controller RAM.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	Slave device index of XI3c_SlaveInfoTable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XI3c_UpdateAddrBcr(XI3c *InstancePtr, u16 DevIndex)
{
	u32 AddrBcr;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	AddrBcr = InstancePtr->XI3c_SlaveInfoTable[DevIndex].DynaAddr & XI3C_7BITS_MASK;   /**< Dynamic address: 0 to 6 bits */
	AddrBcr |= (u32)(InstancePtr->XI3c_SlaveInfoTable[DevIndex].Bcr & XI3C_8BITS_MASK) << 8;/**< BCR: 8 to 15 bits */

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_TARGET_ADDR_BCR, AddrBcr);
}

/*****************************************************************************/
/**
*
* @brief
* Resets the I3C device. Reset must only be called after the driver has been
* initialized. The configuration of the device after reset is the same as its
* configuration after initialization.  Any data transfer that is in progress is
* aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the I3C device after the reset.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         None.
*
******************************************************************************/
static inline void XI3c_Reset(XI3c *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET);
	Data |= XI3C_SOFT_RESET_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET, Data);
	usleep(50);
	Data &= ~XI3C_SOFT_RESET_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET, Data);
	usleep(10);
}

/*****************************************************************************/
/**
*
* @brief
* Resets the fifos of I3C device. Reset must only be called after the driver has been
* initialized. Any data transfer that is in progress is aborted.
*
* @param        InstancePtr is a pointer to the XI3c instance.
*
* @return       None.
*
* @note         None.
*
******************************************************************************/
static inline void XI3c_ResetFifos(XI3c *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	Data = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET);
	Data |= XI3C_ALL_FIFOS_RESET_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET, Data);
	usleep(50);
	Data &= ~XI3C_ALL_FIFOS_RESET_MASK;
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_RESET_OFFSET, Data);
	usleep(10);
}

/*****************************************************************************/
/**
*
* @brief
* Calculates the Odd parity for the dynamic address.
*
* @param        Addr is a dynamic address.
*
* @return       - 0 if Addr has odd parity.
* 		- 1 if Addr has even parity.
*
* @note         None.
*
******************************************************************************/
static inline u8 XI3c_GetOddParity(u8 Addr)
{
	Addr = (Addr & XI3C_4BITS_MASK) ^ ((Addr >> 4) & XI3C_4BITS_MASK);
	Addr = (Addr & XI3C_2BITS_MASK) ^ ((Addr >> 2) & XI3C_2BITS_MASK);
	Addr = (Addr & XI3C_1BIT_MASK) ^ ((Addr >> 1) & XI3C_1BIT_MASK);

	return !(Addr & 1);
}

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
