/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xspdif.h
 * @addtogroup xspdif_v1_1
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18  Initial release.
 * </pre>
 *
 ******************************************************************************/

#ifndef XSPDIF_H
#define XSPDIF_H
/* Prevent circular inclusions by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xspdif_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/** @name Handler Types
* @{
*/
/**
* These constants specify different types of handlers and is used to
* differentiate interrupt requests from the XSpdif peripheral.
*/
typedef enum {
		XSPDIF_HANDLER_TX_OR_RX_FIFO_FULL = 0,
		//!< Transmitter or Receiver FIFO Full Handler
		XSPDIF_HANDLER_TX_OR_RX_FIFO_EMPTY,
		//!< Transmitter or Receiver FIFO Empty Handler
		XSPDIF_HANDLER_START_OF_BLOCK,  //!< Start of Block Handler
		XSPDIF_HANDLER_BMC_ERROR, //!< BMC error
		XSPDIF_HANDLER_PREAMBLE_ERROR, //!< Preamble error
		XSPDIF_NUM_HANDLERS //!< Number of handler types
} XSpdif_HandlerType;
/*@}*/

/**
* Callback function data type for handling interrupt requests
* from the XSpdif peripheral. The application using this driver is
* expected to define a handler of this type to support interrupt driven mode.
* The handler is called in an interrupt context such that minimal processing
* should be performed.
*
* @param CallBackRef is a callback reference passed in by the upper
*        layer when setting the callback functions, and passed back
*        to the upper layer when the callback is invoked.
*
* @return None
* @note None
*/
typedef void (*XSpdif_Callback)(void *CallbackRef);
/**
* @brief This typedef contains configuration information for the XSpdif.
*/
typedef struct {
		u32 DeviceId;		//!< DeviceId is the unique ID of XSpdif
		UINTPTR BaseAddress;
		//!< BaseAddress of the XSpdif Transmitter or Receiver
} XSpdif_Config;
/**
* @brief The XSpdif driver instance data.
*
* An instance must be allocated for each XSpdif core in use.
*/
typedef struct {
		u32 IsReady;
		//!< Core and the driver instance are initialized
		u32 IsStarted;
		//!< Core and the driver instance has started
		XSpdif_Config Config;    //!< Hardware Configuration
		/* Call backs */
		XSpdif_Callback TxOrRxFifoFullHandler;
		//!< Transmitter or Receiver Fifo Full Handler
		void *TxOrRxFifoFullHandlerRef;
		//!< Callback reference for Transmitter or
		//Receiver Fifo Full Handler
		XSpdif_Callback TxOrRxFifoEmptyHandler;
		//!< Transmitter or Receiver Fifo Empty Handler
		void *TxOrRxFifoEmptyHandlerRef;
		//!< Callback reference for Transmitter or
		//Receiver Fifo Empty Handler
		XSpdif_Callback StartOfBlockHandler; //!< Start of Block Handler
		void *StartOfBlockHandlerRef;
		//!< Callback reference for Start of Block Handler
		XSpdif_Callback BmcErrHandler; //!< Start of BMC Error Handler
		void *BmcErrHandlerRef;
		//!< Callback reference for BMC Error Handler
		XSpdif_Callback PreambleErrHandler;
		//!< Start of Preamble Error Handler
		void *PreambleErrHandlerRef;
		//!< Callback reference for Preamble Error Handler
} XSpdif;

/************************* Function Prototypes ******************************/

/* Initialization function in xspdif_sinit.c */
XSpdif_Config *XSpdif_LookupConfig(u16 DeviceId);
int XSpdif_Initialize(XSpdif *InstancePtr, u16 DeviceId);

/* Initialization and control functions in xspdif.c */
int XSpdif_CfgInitialize(XSpdif *InstancePtr,
XSpdif_Config *CfgPtr, UINTPTR EffectiveAddr);
void XSpdif_Enable(XSpdif *InstancePtr, u8 Enable);
void XSpdif_SetClkConfig(XSpdif *InstancePtr, u8 Clk_DivNum);
u32 XSpdif_GetFs(XSpdif *InstancePtr, u32 AudClk);
/* Function to soft reset the Spdif */
void XSpdif_SoftReset(XSpdif *InstancePtr);

/* Function to reset the Fifo */
void XSpdif_ResetFifo(XSpdif *InstancePtr);

/* Interrupt related functions */
/*****************************************************************************/
/**
* This function clears the specified interrupt of the XSpdif.
*
* @param InstancePtr is a pointer to the XSpdif core instance.
* @param Mask is a bit mask of the interrupts to be cleared.
* @see xspdif_hw.h file for the available interrupt masks.
*
* @return None.
*
******************************************************************************/

static inline void XSpdif_IntrClear(XSpdif *InstancePtr, u32 Mask)
{
		Xil_AssertVoid(InstancePtr != NULL);
		XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
				XSPDIF_INTERRUPT_STATUS_REGISTER_OFFSET, Mask);
}
/*****************************************************************************/
/**
* This function globally enables the final interrupt out to the system
*
* @param  InstancePtr is a pointer to the XSpdif instance.
*
* @return None.
*
******************************************************************************/
static inline void XSpdif_Global_IntEnable(XSpdif *InstancePtr)
{
		Xil_AssertVoid(InstancePtr != NULL);
		XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
				XSPDIF_GLOBAL_INTERRUPT_ENABLE_OFFSET,
				XSPDIF_GINTR_ENABLE_MASK);
}
/*****************************************************************************/
/**
* This function disables the specified interrupt of the XSpdif.
*
* @param  InstancePtr is a pointer to the XSpdif instance.
* @param  Mask is a bit mask of the interrupts to be disabled.
*
* @return None.
*
* @see xspdif_hw.h file for the available interrupt masks.
*
******************************************************************************/
static inline void XSpdif_IntrDisable(XSpdif *InstancePtr, u32 Mask)
{
		Xil_AssertVoid(InstancePtr != NULL);

		u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
				XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET);
		RegValue &= ~Mask;
		XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
				XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET,
				RegValue);
}
/*****************************************************************************/
/**
* This function enables the specified interrupt of the XSpdif.
*
* @param  InstancePtr is a pointer to the XSpdif instance.
* @param  Mask is a bit mask of the interrupts to be enabled.
*
* @return None.
*
* @see xspdf_hw.h file for the available interrupt masks.
*
******************************************************************************/
static inline void XSpdif_IntrEnable(XSpdif *InstancePtr, u32 Mask)
{
		Xil_AssertVoid(InstancePtr != NULL);
		u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
				XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET);
		RegValue |= Mask;
		XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
				XSPDIF_INTERRUPT_ENABLE_REGISTER_OFFSET,
				RegValue);
}
void XSpdif_IntrHandler(void *InstancePtr);

int XSpdif_SetHandler(XSpdif *InstancePtr, XSpdif_HandlerType HandlerType,
XSpdif_Callback FuncPtr, void *CallbackRef);

/* Channel status related function */
void XSpdif_Rx_GetChStat(XSpdif *InstancePtr, u8 *ChStatBuf);

/* User Data related functions */
void XSpdif_Rx_GetChA_UserData(XSpdif *InstancePtr, u8 *ChA_UserDataBuf);

void XSpdif_Rx_GetChB_UserData(XSpdif *InstancePtr, u8 *ChB_UserDataBuf);

int XSpdif_SelfTest(XSpdif *InstancePtr);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XSPDIF_H */
/** @} */

