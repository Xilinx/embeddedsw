/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xspdif.c
 * @addtogroup spdif Overview
 * @{
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar   01/25/18  Initial release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xspdif.h"
#include "xspdif_chsts.h"
#include "xspdif_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSPDIF_SOFT_RESET_REGISTER_VALUE		 0X0A
//!< Soft Reset Register value to reset

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function initializes the XSpdif.
 * This function must be called prior to using the core.
 * Initialization of the XSpdif includes
 * setting up the instance data, and ensuring the hardware is in a quiescent
 * state.
 *
 * @param  InstancePtr is a pointer to the XSpdif instance.
 * @param  CfgPtr points to the configuration structure associated with
 *         the XSpdif.
 * @param  EffectiveAddr is the base address of the device. If address
 *         translation is being used, then this parameter must reflect the
 *         virtual base address. Otherwise, the physical address should be
 *         used.
 *
 * @return
 *   - XST_SUCCESS : if successful.
 *   - XST_FAILURE : if version mismatched.
 *
 * @note None.
 *
 ******************************************************************************/
int XSpdif_CfgInitialize(XSpdif *InstancePtr,
		XSpdif_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	u32 Status;
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Run the self test. */
	Status = XSpdif_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Disable the core */
	XSpdif_Enable(InstancePtr, FALSE);

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function enables/disables the XSpdif.
 *
 * @param  InstancePtr is a pointer to the XSpdif instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the XSpdif.
 *
 * @return None.
 *
 ******************************************************************************/
void XSpdif_Enable(XSpdif *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET);

	if (Enable) {
		RegValue |= XSPDIF_CORE_ENABLE_MASK;
		InstancePtr->IsStarted = (XIL_COMPONENT_IS_STARTED);
	} else {
		RegValue &= ~XSPDIF_CORE_ENABLE_MASK;
		InstancePtr->IsStarted = 0;
	}

	XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET,
			RegValue);
}

/*****************************************************************************/
/**
 * This function is used to soft reset the interrupt registers
 *
 * @param  InstancePtr is a pointer to the XSPDIF instance.
 * @return None.
 *
 ******************************************************************************/
void XSpdif_SoftReset(XSpdif *InstancePtr)

{
	XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
			XSPDIF_SOFT_RESET_REGISTER_OFFSET,
		       	XSPDIF_SOFT_RESET_REGISTER_VALUE);
}

/*****************************************************************************/
/**
 * This function resets the Fifo
 *
 * @param  InstancePtr is a pointer to the XSpdif instance.
 * @return None.
 *
 ******************************************************************************/
void XSpdif_ResetFifo(XSpdif *InstancePtr)

{
	u32 Val;
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET);

	RegValue |= XSPDIF_CORE_ENABLE_MASK;
	Val = (RegValue & (~XSPDIF_CORE_ENABLE_MASK));
	XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET, RegValue);
	XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET, Val);

}
/*****************************************************************************/
/**
 * This function sets the clock configuration bits.
 *
 * @param  InstancePtr is a pointer to the Spdif instance.
 * @param  Clk_DivNum is the clock division number.
 *         Clk_DivNum value can be only 4,8,16,24,32,48,or 64.
 *
 * @return -none
 *
 ******************************************************************************/
void XSpdif_SetClkConfig(XSpdif *InstancePtr, u8 Clk_DivNum)
{
	u8 NumToBits;
	Xil_AssertVoid(InstancePtr != NULL);
	/* Check for Clock Division Value */
	switch (Clk_DivNum) {
	          case XSPDIF_CLK_4 :
			  NumToBits = 0;
	          break;

	          case XSPDIF_CLK_8 :
			  NumToBits = 1;
	          break;

	          case XSPDIF_CLK_16 :
			  NumToBits = 2;
	          break;

	          case XSPDIF_CLK_24 :
			  NumToBits = 3;
	          break;

	          case XSPDIF_CLK_32 :
			  NumToBits = 4;
	          break;

	          case XSPDIF_CLK_48 :
			  NumToBits = 5;
	          break;

	          case XSPDIF_CLK_64 :
			  NumToBits = 6;
	          break;

	          default :
			  NumToBits = 0;
			  xil_printf("Clk_DivNum value can be only 4,8,16,24,32,48,or 64.\r\n");
	          break;
	}
	u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET);

	RegValue &= ~XSPDIF_CLOCK_CONFIG_BITS_MASK;

	RegValue |= (NumToBits << XSPDIF_CLOCK_CONFIG_BITS_SHIFT);
	XSpdif_WriteReg(InstancePtr->Config.BaseAddress,
			XSPDIF_CONTROL_REGISTER_OFFSET, RegValue);
	}
/*****************************************************************************/
/**
* This function calculates the Sampling Frequency (Fs) and returns it's value.
*
* @param  InstancePtr is a pointer to the XSpdif instance.
* @param  AudClk is the audio clock frequency value in Hz.
*
* @return - Returns the sampling frequency value in Hz.
*
******************************************************************************/
u32 XSpdif_GetFs(XSpdif *InstancePtr, u32 AudClk)
{
	u32 Fs;
	u32 RegValue = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			XSPDIF_STATUS_REGISTER_OFFSET);
	RegValue &= XSPDIF_SAMPLE_CLOCK_COUNT_MASK;
	Fs = AudClk/(RegValue*64);
	return Fs;
}
/*****************************************************************************/
/**
 * This function reads all the Channel Status registers and writes to a buffer
 *
 * @param  InstancePtr is a pointer to the XSpdif instance.
 * @param  ChStatBuf is a pointer to a buffer.
 *
 * @return None.
 *
 ******************************************************************************/
void XSpdif_Rx_GetChStat(XSpdif *InstancePtr, u8 *ChStatBuf)
{
	int RegOffset = XSPDIF_CHANNEL_STATUS_REGISTER0_OFFSET;
	u32 *pBuf32 = (u32 *) ChStatBuf;
	u8 NumBytes = 24;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChStatBuf != NULL);

	for (int i = 0; i < NumBytes; i += 4) {
		*pBuf32 = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			       	RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/*****************************************************************************/
/**
 * This function reads the Channel A user data and writes to a buffer
 *
 * @param  InstancePtr is a pointer to the XSPDIF instance.
 * @param  ChA_UserDataBuf is a pointer to a buffer.
 *
 * @return None.
 *
 ******************************************************************************/
void XSpdif_Rx_GetChA_UserData(XSpdif *InstancePtr, u8 *ChA_UserDataBuf)
{
	int RegOffset = XSPDIF_CHANNEL_A_USER_DATA_REGISTER0_OFFSET;
	u32 *pBuf32 = (u32 *) ChA_UserDataBuf;
	u8 NumBytes = 24;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChA_UserDataBuf != NULL);

	for (int i = 0; i < NumBytes; i += 4) {
		*pBuf32 = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
			       	RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/*****************************************************************************/
/**
 * This function reads the Channel B user data and writes to a buffer
 *
 * @param  InstancePtr is a pointer to the XSPDIF instance.
 * @param  ChB_UserDataBuf is a pointer to a buffer.
 *
 * @return None.
 *
 ******************************************************************************/
void XSpdif_Rx_GetChB_UserData(XSpdif *InstancePtr, u8 *ChB_UserDataBuf)
{
	int RegOffset = XSPDIF_CHANNEL_B_USER_DATA_REGISTER0_OFFSET;
	u32 *pBuf32 = (u32 *) ChB_UserDataBuf;
	u8 NumBytes = 24;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChB_UserDataBuf != NULL);

	for (int i = 0; i < NumBytes; i += 4) {
		*pBuf32 = XSpdif_ReadReg(InstancePtr->Config.BaseAddress,
				RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/** @} */

