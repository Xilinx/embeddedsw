/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef AMD_XSPI_CONTROL_H
#define AMD_XSPI_CONTROL_H

/*****************************************************************************/
/**
*
* @file amd_xspi_control.h
* @brief Declares the low-level functions of the AXI XSPI driver.
*
* The amd_xspi_control.h header file for the low-level functions of the AXI
* XSPI driver. These functions are used internally by the application-facing
* APIs and are not intended to be called from an application.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* 1.0   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup amd_xspi Overview
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "amd_xspi.h"

/**************************** Type Definitions *******************************/

/**
 * @brief Holds an instruction for Lite mode SPI transactions.
 *
 * This maps to the 32-bit instruction format in the Transmit Register.
 */
typedef struct {
	u8 CmdAddrBytes;	/**< Number of command+address bytes (encoded
				 * 0-3). */
	u16 DataBytes;		/**< Number of read/write data bytes (0-512). */
	u8 IsRead;		/**< 1=Read, 0=Write. */
	u8 DummyCycles;		/**< Number of dummy/latency cycles (0-63). */
	u8 IoFormat;		/**< IO format (for Dual/Quad modes). */
	u8 FrameFormat;		/**< Frame format (0-3). */
} AmdXspi_Instruction;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief Enables the AXI XSPI core.
*
* The core is left enabled for as long as the instance is ready. So the driver
* uses this only at initialization, and to restore the core after a register
* change that requires it to be quiescent.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
******************************************************************************/
static inline void AmdXspi_Enable(const AmdXspi *InstancePtr)
{
	u32 ControlReg;

	ControlReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_CONTROL_OFFSET);
	ControlReg &= ~AMD_XSPI_CR_SOFT_RESET_MASK;
	ControlReg |= AMD_XSPI_CR_ENABLE_MASK;

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET, ControlReg);
}

/*****************************************************************************/
/**
*
* @brief Disables the AXI XSPI core.
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
******************************************************************************/
static inline void AmdXspi_Disable(const AmdXspi *InstancePtr)
{
	u32 ControlReg;

	ControlReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_CONTROL_OFFSET);
	ControlReg &= ~AMD_XSPI_CR_ENABLE_MASK;

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET, ControlReg);
}

/************************** Function Prototypes ******************************/

/* Core state functions (amd_xspi_hw.c) */
void AmdXspi_Reset(AmdXspi *InstancePtr);

/* Lite mode transaction functions (amd_xspi_control.c) */
s32 AmdXspi_LiteRead(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg);
s32 AmdXspi_LiteWrite(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg);

/* Message queue functions (amd_xspi_control.c) */
void AmdXspi_StartMessage(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg);
void AmdXspi_FinishMessageQueue(AmdXspi *InstancePtr, u32 StatusEvent,
				     u32 ByteCount);

/* Data movement functions (amd_xspi_xfer.c) */
u8 AmdXspi_EncodeCmdAddrBytes(u32 CmdAddrBytes);
void AmdXspi_SendInstruction(AmdXspi *InstancePtr,
			   const AmdXspi_Instruction *InstrPtr);
void AmdXspi_SendCmdAddr(AmdXspi *InstancePtr, u8 *CmdAddrBuf, u32 NumBytes);
s32 AmdXspi_SendData(AmdXspi *InstancePtr, u8 *DataBuf, u32 NumBytes);
s32 AmdXspi_RecvData(AmdXspi *InstancePtr, u8 *DataBuf, u32 NumBytes);
void AmdXspi_FillTxFifo(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg);
void AmdXspi_DrainRxFifo(AmdXspi *InstancePtr, const AmdXspi_Msg *Msg,
			 u8 WaitForAll);

/* Register level functions (amd_xspi_hw.c) */
void AmdXspi_PrepareTransfer(AmdXspi *InstancePtr);
#ifdef __cplusplus
}
#endif


/** @} */
#endif /* AMD_XSPI_CONTROL_H */
