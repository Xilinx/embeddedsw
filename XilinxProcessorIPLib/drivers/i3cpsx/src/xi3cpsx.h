/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XI3CPSX_H       /* prevent circular inclusions */
#define XI3CPSX_H

/*****************************************************************************/
/**
*
* @file xi3cpsx.h
* @addtogroup i3cpsx Overview
* @{
* @brief Implements the PS I3C driver for master and slave transfers.
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
*      is initialized by the software and this call is not needed.
*
*    - XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*      configuration structure provided by the caller. If running in a
*      system with address translation, the provided virtual memory base
*      address replaces the physical address in the configuration
*      structure.
*
* <b>Multiple Masters</b>
*
* <b>Multiple Slaves</b>
*
* Multiple slaves are supported by selecting them with unique addresses.
*
* <b>Polled Mode Operation</b>
*
* This driver supports polled mode transfers.
*
* <b>Interrupts</b>
*
* The application must connect the interrupt handler of the driver,
* XI3cPsx_InterruptHandler to an interrupt system such that it is called
* when an interrupt occurs. This function does not save and restore the
* processor context such that the application must provide this processing.
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
* 1.7  vlt  12/30/25 Update Doxygen comments to include SDT flow details.
*      vlt  01/27/26 Fixed codespell issues.
*      vlt  03/15/26 Updated BaseAddress type from u32 to UINTPTR
*                    to support 64-bit addressing.
* 1.8  vn   07/30/26 Fix Doxygen warnings
* 1.8  tb   09/22/26 Align Doxygen comments with AMD documentation standards
* </pre>
*
******************************************************************************/

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
#define XI3CPSX_EVENT_TXOVR_OR_RXUNF		0x0006U  /**< Receive Buffer Underflow or Transmit buffer Overflow */
#define XI3CPSX_EVENT_TRANSF_ABORT		0x0008U  /**< Transfer abort */
#define XI3CPSX_EVENT_SLV_WR_NACK		0x0009U  /**< I2C Slave write data NACK */
#define XI3CPSX_EVENT_PEC			0x000CU  /**< PEC */
#define XI3CPSX_EVENT_TRANSF_ERR		0x000DU  /**< Transfer error */

/** @} */

/** @name IBI (In-Band Interrupt) constants
 * @{
 */
#define XI3CPSX_I3C_HOT_JOIN_ADDR	0x02U	/**< Reserved I3C Hot-Join address */
#define XI3CPSX_I3C_BCR_IBI_PAYLOAD	0x04U	/**< BCR[2]: target sends data with IBI */
#define XI3CPSX_IBI_MAX_PAYLOAD		255U	/**< Max IBI payload bytes (DATA_LEN is 8-bit: 0..255) */
#define XI3CPSX_SLV_IBI_MAX_PAYLOAD	5U	/**< Max target-generated IBI payload: MDB + 4 data bytes */
/** @} */

/** @name Role constants
 *
 * These constants are used to pass into the device setup routines to
 * set up the device according to transfer direction.
 * @{
 */
#define SENDING_ROLE		1  /**< Transfer direction is sending */
#define RECVING_ROLE		0  /**< Transfer direction is receiving */
/** @} */

#define XI3CPSX_MAX_TRANSFER_SIZE	((u32)255U - (u32)3U) /**< Max transfer size */
/* Division rounded to integer greater than actual value */
/** Ceiling division helper. */
#define XI3CPSX_CEIL_DIV(a, b)        ((a + b - 1) / b)

/** Converts a word count to a byte count. */
#define XI3CPSX_WORD_TO_BYTES(N)		(N * 4)
#define XI3CPSX_DATA_LEN			0xFFFFU		/**< Data length */
#define XI3CPSX_TRANSFER_ERROR			0xF0000000U	/**< Error */
#define XI3CPSX_TIMEOUT_COUNTER         	2000000U 	/**< Wait for 2 sec in worst case */

/**
 * @brief Defines the maximum number of Device Address Table (DAT) entries.
 *
 * Maximum number of entries in the Device Address Table (DAT) supported by the
 * controller and tracked by the driver.
 */
#define XI3CPSX_MAX_DAT_ENTRIES		11U

#define XI3CPSX_DCT_ENTRY_WORDS		4U	/**< Words per Device Characteristics Table (DCT) entry */
#define XI3CPSX_DCT_ENTRY_BYTES		(XI3CPSX_DCT_ENTRY_WORDS * 4U)	/**< Bytes per DCT entry; spacing between consecutive entries */

/**
 * @brief Computes the number of elements in an array at compile time.
 *
 * @note
 * C-style signature
 *	size_t XI3CPSX_ARRAY_SIZE(ElementType Array[])
 */
#define XI3CPSX_ARRAY_SIZE(Array)	(sizeof(Array) / sizeof((Array)[0U]))

/**************************** Type Definitions *******************************/

/**
* @brief Defines a callback for I3C interrupt events.
*
* The handler data type allows the application to define a callback function to
* respond to interrupt events in the system. This function is executed
* in interrupt context, so amount of processing should be minimized.
*
* @param[in] CallBackRef The callback reference passed in by the upper layer
*            when setting the callback functions, and passed back to the upper
*            layer when the callback is invoked. Its type is not important to
*            the driver, so it is a void pointer.
* @param[in] StatusEvent Indicates one or more status events that occurred.
*/
typedef void (*XI3cPsx_IntrHandler) (void *CallBackRef, u32 StatusEvent);

/**
* @brief Defines a callback for In-Band Interrupt (SIR) events.
*
* The IBI handler data type allows the application to define a callback function
* to respond to In-Band Interrupt (SIR) events received from a target. This
* function is executed in interrupt context, so the amount of processing
* should be minimized.
*
* @param[in] CallBackRef The callback reference passed in by the upper layer
*            when setting the IBI handler, and passed back to the upper layer
*            when the callback is invoked.
* @param[in] DevAddr The dynamic address of the target that raised the IBI.
* @param[in] Payload Pointer to the mandatory-data-byte/payload bytes drained
*            from the IBI queue (valid for PayloadLen bytes). Only valid for the
*            duration of the callback.
* @param[in] PayloadLen The number of valid payload bytes.
*/
typedef void (*XI3cPsx_IbiHandler) (void *CallBackRef, u32 DevAddr,
				    u8 *Payload, u32 PayloadLen);

/**
* @brief Defines a callback for Hot-Join requests.
*
* The Hot-Join handler data type allows the application to define a callback
* function the driver invokes (from interrupt context) to flag a Hot-Join
* request. The joining target is NOT yet enrolled when this is called; the
* application must run enrolment (ENTDAA) via XI3cPsx_HotJoinDaa() from
* thread context.
*
* @param[in] CallBackRef The callback reference passed in by the upper layer
*            when setting the Hot-Join handler, and passed back to the upper
*            layer when the callback is invoked.
*/
typedef void (*XI3cPsx_HjHandler) (void *CallBackRef);

/**
 * @brief Holds configuration information for the device.
 *
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;     /**< Unique ID  of device */
#else
	char *Name;       /**< Name of the device */
#endif
	UINTPTR BaseAddress;  /**< Base address of the device */
#ifdef SDT
	/**
	 * Bits[11:0] interrupt ID; bits[15:12] trigger type and level flags.
	 */
	u16 IntrId;
	/**
	 * Bit[0] interrupt parent type; bit[64/32:1] parent base address.
	 */
	UINTPTR IntrParent;
#endif

	u32 InputClockHz; /**< Input clock frequency */
	u32 DeviceCount; /**< No of I3C devices */
} XI3cPsx_Config;

/**
 * @brief Holds transaction command and arguments.
 *
 * This typedef contains Transaction command and arguments.
 */
typedef struct {
	u32 TransCmd;      /**< Transaction command code */
	u32 TransArg;      /**< Argument associated with the transaction */
	void *RxBuf;       /**< Pointer to receive buffer for incoming data */
	u8 Error;          /**< Error status for the transaction */
} XI3cPsx_Cmd;

/**
 * @brief Holds FIFO depth information.
 *
 * This typedef contains FIFO depth information.
 */
typedef struct {
	u8 Cmd_FD;	/**< Command FIFO depth */
	u8 Data_FD;	/**< Data FIFO depth */
} XI3cPsx_Master_Caps;

/**
 * @brief Holds command information.
 *
 * This typedef contains Command information.
 */
struct CmdInfo {
	u8 Cmd;		/**< Command type */
	u16 SlaveAddr;	/**< Slave address */
	u8 *TxBuff;	/**< Pointer to transmit buffer */
	u8 *RxBuff;	/**< Pointer to receive buffer */
	u8 TxLen;	/**< Transmit length in bytes */
	u8 RxLen;	/**< Receive length in bytes */
};
/**
 * @brief Holds per-instance I3C driver state.
 *
 * The application must allocate a variable of this type for each I3C
 * device in the system. A pointer to a variable of this type is then passed
 * to the driver API functions.
 */
typedef struct {
	XI3cPsx_Config Config;	/**< Configuration structure */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 Options;		/**< Options set in the device */
	u32 Addr[XI3CPSX_MAX_DAT_ENTRIES]; /**< Dynamic Addresses to be assigned */
	u32 DatStartAddr;	/**< DAT base, read from DEVICE_ADDR_TABLE_POINTER */
	u32 MaxDevs;		/**< DAT depth, capped at XI3CPSX_MAX_DAT_ENTRIES */

	XI3cPsx_Master_Caps Caps; /**< Cmd, Data fifo depths */
	u8 *SendBufferPtr;	/**< Pointer to send buffer */
	u8 *RecvBufferPtr;	/**< Pointer to recv buffer */
	u16 SendByteCount;	/**< Number of bytes still expected to send */
	u16 RecvByteCount;	/**< Number of bytes still expected to receive */
	u16 CurrByteCount;	/**< No. of bytes received in the last transfer */
	u8 Error;	/**< Error status of last transfer, set by the ISR */
	u8 LastDatIndex;	/**< DAT index of the most recently issued command */
	u32 LastTransCmd;	/**< Command word of the most recently issued command */
	volatile u8 OvfPending;	/**< Target latched an overflow; cleared with
				     GETSTATUS before the next transfer */
	volatile u32 RespDone;	/**< Set by the master ISR after it consumes a command response */

	s32 UpdateTxSize;	/**< If tx size register has to be updated */
	s32 IsSend;		/**< Whether master is sending or receiving */
	s32 IsRepeatedStart;	/**< Indicates if user set repeated start */
	s32 Is10BitAddr;	/**< Indicates if user set 10 bit address */

	XI3cPsx_IntrHandler StatusHandler;  /**< Event handler function */
	void *CallBackRef;	/**< Callback reference for event handler */

	u32 SclkHz;	/**< SCL clock frequency */

	u8 Bcr[XI3CPSX_MAX_DAT_ENTRIES];  /**< Per-device BCR captured during DAA */
	u32 SirRejectMask;	/**< Shadow of IBI_SIR_REQ_REJECT (1 = reject) */
	XI3cPsx_IbiHandler IbiHandler;	/**< IBI (SIR) event handler function */
	void *IbiCallBackRef;	/**< Callback reference for the IBI handler */
	u8 IbiData[XI3CPSX_IBI_MAX_PAYLOAD]; /**< Scratch buffer for IBI payload */

	XI3cPsx_HjHandler HjHandler;	/**< Hot-Join notification handler function */
	void *HjCallBackRef;	/**< Callback reference for the Hot-Join handler */
	volatile u32 HotJoinPending;	/**< Set by the ISR on a Hot-Join; the application enrols the target via XI3cPsx_HotJoinDaa() */
} XI3cPsx;

/************************** Variable Definitions *****************************/
extern XI3cPsx_Config XI3cPsx_ConfigTable[];	/**< Configuration table */

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* @brief Returns the register offset of a Device Address Table (DAT) entry.
*
* Entries are one word wide and the table base comes from
* DEVICE_ADDR_TABLE_POINTER.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Index The DAT entry index.
*
* @return Register offset of the requested DAT entry.
*
*****************************************************************************/
static inline u32 XI3cPsx_DatEntryOffset(const XI3cPsx *InstancePtr, u32 Index)
{
	return InstancePtr->DatStartAddr + (Index * 4U);
}

/****************************************************************************/
/**
*
* @brief Places one byte into the transmit FIFO.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
*
* @note
* C-style signature
*	void XI3cPsx_SendByte(XI3cPsx *InstancePtr)
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
* @brief Receives one byte from FIFO.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
*
* @note
* C-style signature
*	void XI3cPsx_RecvByte(XI3cPsx *InstancePtr)
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
* @brief Computes the I3C odd-parity bit for a 7-bit dynamic address.
*
* Per the I3C spec and the DEV_ADDR_TABLE_LOC*n*.DEV_DYNAMIC_ADDR field,
* bit[7] of the 8-bit encoded value must be the odd parity of bits[6:0],
* that is, parity = ~XOR(addr[6:0]).  The controller transmits this 8-bit value
* verbatim during ENTDAA, so a raw address write with bit[7]=0 produces
* wrong parity for any 7-bit address that has an even number of 1-bits
* (for example 0x09, 0x0A, 0x0C, 0x0F, 0x11, 0x12, ...) and the slave NACKs the
* address assignment, hanging the controller.
*
* @param[in] Addr The 7-bit dynamic address (only bits[6:0] are used).
*
* @return 0 or 1 - the odd-parity bit to place in bit[7].
*
****************************************************************************/
static inline u8 XI3cPsx_OddParity7(u8 Addr)
{
	Addr = (u8)((Addr & XI3CPSX_4BITS_MASK) ^ ((Addr >> 4) & XI3CPSX_4BITS_MASK));
	Addr = (u8)((Addr & XI3CPSX_2BITS_MASK) ^ ((Addr >> 2) & XI3CPSX_2BITS_MASK));
	Addr = (u8)((Addr & XI3CPSX_1BIT_MASK) ^ ((Addr >> 1) & XI3CPSX_1BIT_MASK));

	return (u8)(!(Addr & XI3CPSX_1BIT_MASK));
}

/*****************************************************************************/
/**
* @brief Enables the I3C controller.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
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
* @brief Resumes the I3C controller.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
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
* @brief Reads the response.
*
* It waits for the response from slave and reads the response.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
*
* @return
*        - Response code on success.
*        - XST_TIMEOUT on timeout.
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
		/* No response word, so the last transfer's length is not valid. */
		InstancePtr->CurrByteCount = 0U;
		return XST_TIMEOUT;
	}

	ResponseData = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress,
				       XI3CPSX_RESPONSE_QUEUE_PORT);

	/* Record the number of bytes the response reports for this transfer. */
	InstancePtr->CurrByteCount = (u16)(ResponseData & XI3CPSX_DATA_LEN);

	/*
	 * Return response code
	 */
	return  (ResponseData & XI3CPSX_RESPONSE_ERR_STS_MASK) >> XI3CPSX_RESPONSE_ERR_STS_SHIFT;
}

/*****************************************************************************/
/**
* @brief Sets the response-buffer interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The response-buffer threshold value.
*
****************************************************************************/
static inline void XI3cPsx_SetRespThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_QUEUE_THLD_CTRL_RESP_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, Reg);
}

/*****************************************************************************/
/**
* @brief Sets the command-queue empty interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The command-empty threshold value.
*
****************************************************************************/
static inline void XI3cPsx_SetCmdEmptyThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_QUEUE_THLD_CTRL_CMD_EMPTY_BUF_THLD_MASK);
	Reg = Reg | Val;
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_QUEUE_THLD_CTRL, Reg);
}

/*****************************************************************************/
/**
* @brief Sets the receive-buffer interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The receive-buffer threshold value.
*
****************************************************************************/
static inline void XI3cPsx_SetRxThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

/*****************************************************************************/
/**
* @brief Sets the transmit-buffer interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The transmit-empty threshold value.
*
****************************************************************************/
static inline void XI3cPsx_SetTxThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_TX_EMPTY_BUF_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

/*****************************************************************************/
/**
* @brief Sets the receive-start interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The receive-start threshold value.
*
****************************************************************************/
static inline void XI3cPsx_SetRxStartThreshold(XI3cPsx *InstancePtr, u32 Val)
{
	u32 Reg;

	Reg = XI3cPsx_ReadReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL);
	Reg = Reg & ~(XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_MASK);
	Reg = Reg | (Val << XI3CPSX_DATA_BUFFER_THLD_CTRL_RX_START_THLD_SHIFT);
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_DATA_BUFFER_THLD_CTRL, Reg);
}

/*****************************************************************************/
/**
* @brief Sets the transmit-start interrupt threshold.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
* @param[in] Val The transmit-start threshold value.
*
****************************************************************************/
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
XI3cPsx_Config *XI3cPsx_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * Functions for general setup, in XI3cPsx.c
 */
s32 XI3cPsx_CfgInitialize(XI3cPsx *InstancePtr, XI3cPsx_Config *ConfigPtr,
			  u32 EffectiveAddr);

/**
 * @brief Aborts an I3C transfer.
 *
 * @param[in] InstancePtr Pointer to the XI3cPsx instance.
 */
void XI3cPsx_Abort(XI3cPsx *InstancePtr);
void XI3cPsx_Resume(XI3cPsx *InstancePtr);
void XI3cPsx_Reset(XI3cPsx *InstancePtr);
void XI3cPsx_ResetFifos(XI3cPsx *InstancePtr);
void XI3cPsx_RecoverFromError(XI3cPsx *InstancePtr);

/**
 * @brief Checks whether the I3C bus is busy.
 *
 * @param[in] InstancePtr Pointer to the XI3cPsx instance.
 *
 * @return
 *        - TRUE if the bus is busy.
 *        - FALSE if the bus is not busy.
 */
s32 XI3cPsx_BusIsBusy(XI3cPsx *InstancePtr);
/**
 * @brief Fills the I3C transmit FIFO.
 *
 * @param[in] InstancePtr Pointer to the XI3cPsx instance.
 *
 * @return
 *        Remaining byte count for this instance.
 */
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
		       XI3cPsx_Cmd Cmds);
s32 XI3cPsx_MasterRecv(XI3cPsx *InstancePtr, u8 *MsgPtr,
		       s32 ByteCount, XI3cPsx_Cmd *Cmds);
s32 XI3cPsx_MasterSendPolled(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
			     XI3cPsx_Cmd Cmds);
s32 XI3cPsx_MasterRecvPolled(XI3cPsx *InstancePtr, u8 *MsgPtr, s32 ByteCount,
			     XI3cPsx_Cmd *Cmds);
void XI3cPsx_MasterInterruptHandler(XI3cPsx *InstancePtr);

/*
 * Functions for In-Band Interrupt (IBI/SIR) support, in xi3cpsx_master.c
 */
void XI3cPsx_SetIbiHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			   XI3cPsx_IbiHandler FunctionPtr);
s32 XI3cPsx_EnableIbi(XI3cPsx *InstancePtr, u32 DevIndex);
s32 XI3cPsx_DisableIbi(XI3cPsx *InstancePtr, u32 DevIndex);

/*
 * Functions for Hot-Join support, in xi3cpsx_master.c
 */
void XI3cPsx_SetHjHandler(XI3cPsx *InstancePtr, void *CallBackRef,
			  XI3cPsx_HjHandler FunctionPtr);
void XI3cPsx_EnableHotJoin(XI3cPsx *InstancePtr);
void XI3cPsx_DisableHotJoin(XI3cPsx *InstancePtr);
s32 XI3cPsx_HotJoinDaa(XI3cPsx *InstancePtr);

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
/**
 * @brief Runs a self-test on the I3C device.
 *
 * @param[in] InstancePtr Pointer to the XI3cPsx instance.
 *
 * @return
 *        - XST_SUCCESS if the test was successful.
 *        - XST_FAILURE if the test was not successful.
 */
s32 XI3cPsx_SelfTest(XI3cPsx *InstancePtr);

/*
 * Functions to get and set I3C Scl clock frequency
 */
s32 XI3cPsx_SetSClk(XI3cPsx *InstancePtr);
s32 XI3cPsx_SetSClkRate(XI3cPsx *InstancePtr, u32 SclkHz);
u32 XI3cPsx_GetSClk(const XI3cPsx *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
