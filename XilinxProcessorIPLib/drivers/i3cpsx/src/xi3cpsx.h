/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file I3cPsx.h
* @addtogroup Overview
* @{
* @details
*
* This is an implementation of I3C driver in the PS block. The device can
* be either a master or a slave on the I3C bus. This implementation supports
* both interrupt mode transfer and polled mode transfer.
*
* The higher level software must implement a higher layer protocol to inform
* the slave what to send to the master.
*
* <b>Initialization & Configuration</b>
*
* The XI3cPsx_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed by
* various operating systems, the driver instance can be initialized in the
* following way:
*
*    - XI3cPsx_LookupConfig(DeviceId) - Use the device identifier to find
*      the static configuration structure defined in XI3cPsx_g.c. This is
*      setup by the tools. For some operating systems the config structure
*      will be initialized by the software and this call is not needed.
*
*    - XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*      configuration structure provided by the caller. If running in a
*      system with address translation, the provided virtual memory base
*      address replaces the physical address in the configuration
*      structure.
*
* <b>Multiple Masters</b>
*
*
* <b>Multiple Slaves</b>
*
* Multiple slaves are supported by selecting them with unique addresses.
*
*
* <b>Polled Mode Operation</b>
*
* This driver supports polled mode transfers.
*
* <b>Interrupts</b>
*
* The user must connect the interrupt handler of the driver,
* XI3cPsx_InterruptHandler to an interrupt system such that it will be called
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
* 1.00  sd  06/10/22 First release
* 1.3  sd   11/17/23 Added support for system device-tree flow
* 1.4  gm   10/07/24 Added functions for Enable, Resume, read response
* 		     and set threshold for Tx, Rx and command.
* 		     Update data type of Send and Recv byte counts.
* </pre>
*
******************************************************************************/

#ifndef XI3CPSX_H       /* prevent circular inclusions */
#define XI3CPSX_H       /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"
#include "xplatform_info.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/** @} */

/** @name Callback events
 *
 * These constants specify the handler events that are passed to an application
 * event handler from the driver.  These constants are bit masks such that
 * more than one event can be passed to the handler.
 *
 * @{
 */
#define XI3CPSX_EVENT_CRC		0x0001U  /**< CRC Event*/
#define XI3CPSX_EVENT_PARITY		0x0002U  /**< Parity Event*/
#define XI3CPSX_EVENT_FRAME		0x0003U  /**< Frame Errors */
#define XI3CPSX_EVENT_IBA_NACK		0x0004U  /**< IBA NACK */
#define XI3CPSX_EVENT_ADDRESS_NACK		0x0005U  /**< Address NACK */
#define XI3CPSX_EVENT_TXOVR_OR_RXUNF		0x0006U  /**< Recieve Buffer Underflow or Transmit buffer Overflow */
#define XI3CPSX_EVENT_TRANSF_ABORT		0x0008U  /**< Transfer abort */
#define XI3CPSX_EVENT_SLV_WR_NACK		0x0009U  /**< I2C Slave write data NACK */
#define XI3CPSX_EVENT_PEC			0x000CU  /**< PEC */

/** @} */

/** name Role constants
 *
 * These constants are used to pass into the device setup routines to
 * set up the device according to transfer direction.
 */
#define SENDING_ROLE		1  /**< Transfer direction is sending */
#define RECVING_ROLE		0  /**< Transfer direction is receiving */


#define XI3CPSX_MAX_TRANSFER_SIZE	((u32)255U - (u32)3U) /**< Max transfer size */
/* Division rounded to integer greater than actual value */
#define XI3CPSX_CEIL_DIV(a, b)        ((a + b - 1) / b)

#define XI3CPSX_WORD_TO_BYTES(N)		(N * 4)
#define XI3CPSX_DATA_LEN			0x00FFU		/**< Data length */
#define XI3CPSX_TRANSFER_ERROR			0xF0000000U	/**< Error */
#define XI3CPSX_TIMEOUT_COUNTER         	2000000U 	/**< Wait for 2 sec in worst case */
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
typedef void (*XI3cPsx_IntrHandler) (void *CallBackRef, u32 StatusEvent);

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

	u32 InputClockHz; /**< Input clock frequency */
	u32 DeviceCount; /**< No of I3C devices */
} XI3cPsx_Config;

typedef struct {
	u32 TransCmd;
	u32 TransArg;
	void *RxBuf;
	u8 Error;
} XI3cPsx_Cmd;

/**
 * This typedef contains FIFO depth information.
 */
typedef struct {
	u8 Cmd_FD;
	u8 Data_FD;
} XI3cPsx_Master_Caps;

struct CmdInfo {
	u8 Cmd;
	u16 SlaveAddr;
	u8 *TxBuff;
	u8 *RxBuff;
	u8 TxLen;
	u8 RxLen;
};
/**
 * The XI3cPsx driver instance data. The user is required to allocate a
 * variable of this type for each IIC device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XI3cPsx_Config Config;	/**< Configuration structure */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 Options;		/**< Options set in the device */
	u32 Addr[20]; /**< Dynamic Addresses to be assigned */

	XI3cPsx_Master_Caps Caps; /**< Cmd, Data fifo depths */
	u8 *SendBufferPtr;	/**< Pointer to send buffer */
	u8 *RecvBufferPtr;	/**< Pointer to recv buffer */
	u16 SendByteCount;	/**< Number of bytes still expected to send */
	u16 RecvByteCount;	/**< Number of bytes still expected to receive */
	u16 CurrByteCount;	/**< No. of bytes expected in current transfer */
	u8 Error;

	s32 UpdateTxSize;	/**< If tx size register has to be updated */
	s32 IsSend;		/**< Whether master is sending or receiving */
	s32 IsRepeatedStart;	/**< Indicates if user set repeated start */
	s32 Is10BitAddr;	/**< Indicates if user set 10 bit address */

	XI3cPsx_IntrHandler StatusHandler;  /**< Event handler function */
	void *CallBackRef;	/**< Callback reference for event handler */
} XI3cPsx;

/************************** Variable Definitions *****************************/
extern XI3cPsx_Config XI3cPsx_ConfigTable[];	/**< Configuration table */

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* Place one byte into the transmit FIFO.
*
* @param	InstancePtr is the instance of IIC
*
* @return	None.
*
* @note		C-Style signature:
*		void XI3cPsx_SendByte(XI3cPsx *InstancePtr)
*
*****************************************************************************/
#define XI3cPsx_SendByte(InstancePtr)					\
	{									\
		u8 Data;							\
		Data = *((InstancePtr)->SendBufferPtr);				\
		XI3cPsx_Out32((InstancePtr)->Config.BaseAddress			\
			      + (u32)(XI3cPsx_DATA_OFFSET), 			\
			      (u32)(Data));			\
		(InstancePtr)->SendBufferPtr += 1;				\
		(InstancePtr)->SendByteCount -= 1;\
	}

/****************************************************************************/
/**
*
* Receive one byte from FIFO.
*
* @param	InstancePtr is the instance of IIC
*
* @return	None.
*
* @note		C-Style signature:
*		u8 XI3cPsx_RecvByte(XI3cPsx *InstancePtr)
*
*****************************************************************************/
#define XI3cPsx_RecvByte(InstancePtr)					\
	{									\
		u8 *Data, Value;						\
		Value = (u8)(XI3cPsx_In32((InstancePtr)->Config.BaseAddress	\
					  + (u32)XI3cPsx_DATA_OFFSET));  			\
		Data = &Value;							\
		*(InstancePtr)->RecvBufferPtr = *Data;				\
		(InstancePtr)->RecvBufferPtr += 1;				\
		(InstancePtr)->RecvByteCount --; 				\
	}

/*****************************************************************************/
/**
* @brief
* This function enables the controller
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
* @note
*
****************************************************************************/
static inline void XI3cPsx_Enable(XI3cPsx *InstancePtr)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	Reg = Reg | XI3CPSX_DEVICE_CTRL_ENABLE_MASK;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, Reg);
	(void)XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
}

/*****************************************************************************/
/**
* @brief
* This function enables the controller
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return	None.
*
* @note
*
****************************************************************************/
static inline void XI3cPsx_Resume(XI3cPsx *InstancePtr)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
	Reg = Reg | (XI3CPSX_DEVICE_CTRL_ENABLE_MASK | XI3CPSX_DEVICE_CTRL_RESUME_MASK);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL, Reg);
	(void)XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DEVICE_CTRL);
}

/*****************************************************************************/
/**
* @brief
* This function reads the response.
*
* It waits for the response from slave and reads the response.
*
* @param	InstancePtr is a pointer to the XI3cPsx instance.
*
* @return
*		- Response code on sucess.
*		- XST_TIMEOUT on timeout.
*
* @note
*
****************************************************************************/
static inline s32 XI3cPsx_GetResponse(XI3cPsx *InstancePtr)
{
	s32 Status;
	u32 ResponseData;

	Status = (int)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) +
				       XI3CPSX_INTR_STATUS),
				       XI3CPSX_INTR_STATUS_RESP_READY_STS_MASK,
				       XI3CPSX_INTR_STATUS_RESP_READY_STS_MASK,
				       XI3CPSX_TIMEOUT_COUNTER);

	if (Status != XST_SUCCESS) {
#ifdef DEBUG
		xil_printf("Response error: XST_TIMEOUT\n");
#endif
		return XST_TIMEOUT;
	}

	ResponseData = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				       XI3CPSX_RESPONSE_QUEUE_PORT);

	/*
	 * Return response code
	 */
	return  (ResponseData & XI3CPSX_RESPONSE_ERR_STS_MASK) >> XI3CPSX_RESPONSE_ERR_STS_SHIFT;
}

static inline void XI3cPsx_SetRespThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, Reg);
}

static inline void XI3cPsx_SetCmdEmptyThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_MASK);
	Reg = Reg | Val;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, Reg);
}

static inline void XI3cPsx_SetRxThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

static inline void XI3cPsx_SetTxThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

static inline void XI3cPsx_SetRxStartThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

static inline void XI3cPsx_SetTxStartThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_START_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

/************************** Function Prototypes ******************************/

/*
 * Function for configuration lookup, in XI3cPsx_sinit.c
 */
#ifndef SDT
XI3cPsx_Config *XI3cPsx_LookupConfig(u16 DeviceId);
#else
XI3cPsx_Config *XI3cPsx_LookupConfig(u32 BaseAddress);
#endif

/*
 * Functions for general setup, in XI3cPsx.c
 */
s32 XI3cPsx_CfgInitialize(XI3cPsx *InstancePtr, XI3cPsx_Config *ConfigPtr,
			  u32 EffectiveAddr);

void XI3cPsx_Abort(XI3cPsx *InstancePtr);
void XI3cPsx_Resume(XI3cPsx *InstancePtr);
void XI3cPsx_Reset(XI3cPsx *InstancePtr);
void XI3cPsx_ResetFifos(XI3cPsx *InstancePtr);

s32 XI3cPsx_BusIsBusy(XI3cPsx *InstancePtr);
s32 XI3cPsx_TransmitFifoFill(XI3cPsx *InstancePtr);
void XI3cPsx_WrCmdFifo(XI3cPsx *InstancePtr, XI3cPsx_Cmd *Cmd);
void XI3cPsx_WrTxFifo(XI3cPsx *InstancePtr, u32 *TxBuf, u16 TxLen);
void XI3cPsx_RdRxFifo(XI3cPsx *InstancePtr, u32 *RxBuf, u16 RxLen);
s32 XI3cPsx_SendTransferCmd(XI3cPsx *InstancePtr, struct CmdInfo *CmdCCC);
s32 XI3cPsx_SendAddrAssignCmd(XI3cPsx *InstancePtr, struct CmdInfo *CmdCCC);
s32 XI3cPsx_BusInit(XI3cPsx *InstancePtr);
#ifdef DEBUG
void XI3cPsx_PrintDCT(XI3cPsx *InstancePtr);
#endif

/*
 * Functions for interrupts, in XI3cPsx_intr.c
 */
void XI3cPsx_SetStatusHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			      XI3cPsx_IntrHandler FunctionPtr);

/*
 * Functions for device as master, in XI3cPsx_master.c
 */
s32 XI3cPsx_MasterSend(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
		       XI3cPsx_Cmd Cmd);
s32 XI3cPsx_MasterRecv(XI3cPsx *InstancePtr, u8 *MsgPtr,
		       s32 ByteCount, XI3cPsx_Cmd *Cmds);
s32 XI3cPsx_MasterSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
			     XI3cPsx_Cmd Cmd);
s32 XI3cPsx_MasterRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
			     XI3cPsx_Cmd *Cmd);
void XI3cPsx_MasterInterruptHandler(XI3cPsx *InstancePtr);

/*
 * Functions for device as slave, in XI3cPsx_slave.c
 */
void XI3cPsx_SetupSlave(XI3cPsx *InstancePtr, u16 SlaveAddr);
void XI3cPsx_SlaveSend(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount);
void XI3cPsx_SlaveRecv(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount);
s32 XI3cPsx_SlaveSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
			    XI3cPsx_Cmd Cmds);
s32 XI3cPsx_SlaveRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr);
void XI3cPsx_SlaveInterruptHandler(XI3cPsx *InstancePtr);

/*
 * Functions for selftest, in XI3cPsx_selftest.c
 */
s32 XI3cPsx_SelfTest(XI3cPsx *InstancePtr);

/*
 * Functions for setting and getting data rate, in XI3cPsx_options.c
 */
s32 XI3cPsx_SetOptions(XI3cPsx *InstancePtr, u32 Options);
s32 XI3cPsx_ClearOptions(XI3cPsx *InstancePtr, u32 Options);
u32 XI3cPsx_GetOptions(XI3cPsx *InstancePtr);

s32 XI3cPsx_SetSClk(XI3cPsx *InstancePtr);
u32 XI3cPsx_GetSClk(XI3cPsx *InstancePtr);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
