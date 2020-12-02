/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2stx.c
 * @addtogroup i2stx_v2_2
 * @{
 *
 * Contains a minimal set of functions for the i2s_transmitter driver
 * that allow access to all of the i2s transmitter core's functionality.
 * See xi2stx.h for a detailed description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 1.1   kar    04/02/18 Changed channel status clear API to clear all regs.
 * 2.0   kar    09/28/18 Added new API to enable justification.
 *                       Added new API to select left/right justification.
 * 2.1   pg     01/30/20 Added 32bit_LR support is added to calculate i2s clock
 *                       for 32bit mode.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2stx.h"
#include "xi2stx_chsts.h"
#include "xi2stx_hw.h"
#include "xi2stx_debug.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XI2S_TX_CLK_MASK (0xFF)
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function initializes the I2S Transmitter.
 * This function must be called prior to using the core.
 * Initialization of the I2S Transmitter includes setting up the
 * instance data, and ensuring the hardware is in a quiescent state.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  CfgPtr points to the configuration structure associated with
 *         the I2s Transmitter.
 * @param  EffectiveAddr is the base address of the device. If address
 *         translation is being used, then this parameter must reflect the
 *         virtual base address. Otherwise, the physical address should be
 *         used.
 *
 * @return
 *   - XST_SUCCESS : if successful.
 *   - XST_FAILURE : otherwise.
 *
 * @note None.
 *
 ****************************************************************************/
int XI2s_Tx_CfgInitialize(XI2s_Tx *InstancePtr,
		XI2stx_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)0x0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Run the self test. */
	Status = XI2s_Tx_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Disable the core */
	XI2s_Tx_Enable(InstancePtr, FALSE);

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function enables/disables the I2s Transmitter.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the I2s Transmitter.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Tx_Enable(XI2s_Tx *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET);

	if (Enable) {
		RegValue |= XI2S_TX_REG_CTRL_EN_MASK;
		InstancePtr->IsStarted = (XIL_COMPONENT_IS_STARTED);
	} else {
		RegValue &= ~XI2S_TX_REG_CTRL_EN_MASK;
		InstancePtr->IsStarted = 0;
	}

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET,
			RegValue);
}
/*****************************************************************************/
/**
 * This function enables the specified interrupt of the I2s Transmitter.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  Mask is a bit mask of the interrupts to be enabled.
 *
 * @return None.
 *
 * @see XI2stx_hw.h for the available interrupt masks.
 *
 *****************************************************************************/
void XI2s_Tx_IntrEnable(XI2s_Tx *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_IRQCTRL_OFFSET);

	RegValue |= Mask;

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_IRQCTRL_OFFSET,
			RegValue);
}
/*****************************************************************************/
/**
 * This function disables the specified interrupt of the I2s Transmitter.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  Mask is a bit mask of the interrupts to be disabled.
 *
 * @return None.
 *
 * @see XI2stx_HW for the available interrupt masks.
 *
 *****************************************************************************/
void XI2s_Tx_IntrDisable(XI2s_Tx *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_IRQCTRL_OFFSET);

	RegValue &= ~Mask;

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_IRQCTRL_OFFSET,
			RegValue);
}
/*****************************************************************************/
/**
 * This function sets the input source for the specified I2s channel.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  ChID specifies the I2s channel
 * @param  InputSource specifies the input source
 *
 * @return
 *   - XST_SUCCESS : if successful.
 *   - XST_FAILURE : if the I2s channel is invalid.
 *
 *****************************************************************************/
int XI2s_Tx_SetChMux(XI2s_Tx *InstancePtr, XI2s_Tx_ChannelId ChID,
		XI2s_Tx_ChMuxInput InputSource)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (ChID > XI2S_TX_NUM_CHANNELS)
		return XST_FAILURE;

	int RegValue = 0;
	int RegOffset = XI2S_TX_CH01_OFFSET + (ChID * 4);

	switch (InputSource) {
		case XI2S_TX_CHMUX_AXIS_01:
			RegValue = InputSource;
			break;

		case XI2S_TX_CHMUX_AXIS_23:
			RegValue = InputSource;
			break;

		case XI2S_TX_CHMUX_AXIS_45:
			RegValue = InputSource;
			break;

		case XI2S_TX_CHMUX_AXIS_67:
			RegValue = InputSource;
			break;

		case XI2S_TX_CHMUX_WAVEGEN:
			RegValue = InputSource;
			break;

		default: /* Disabled */
			RegValue = 0;
			break;
	}

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			RegOffset, RegValue);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function calculates the SCLK Output divider value of the
 * I2S timing generator.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  MClk is the frequency of the MClk.
 * @param  Fs is the sampling frequency of the system.
 * Divider value for the SCLK generation, MCLK/SCLK = SCLKOUT_DIV x 2
 * i.e. MCLK = 384xFs, SCLK = 48xFs
 * (2x24bits) ->
 * SCLKOUT_DIV = MCLK/SCLK/2 = 4
 * Valid values are 1 through 15.
 *
 * @return - XST_FAILURE if SCLK Output divider is not calculated to be a
 *           positive integer.
 *         - XST_SUCCESS, otherwise.
 *
 *****************************************************************************/
u32 XI2s_Tx_SetSclkOutDiv(XI2s_Tx *InstancePtr, u32 MClk, u32 Fs)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((MClk > 0) && (MClk > Fs));
	Xil_AssertNonvoid((Fs > 0) && (Fs < MClk));
	u32 SClk;
	u8 SClkOut_Div;

	/* Sclk should be multiplied by 32, when 32BitLR parameter is enabled */
	if (InstancePtr->Config.Is32BitLR)
	{
		SClk = (2 * Fs * 32);
	} else {
		SClk = (2 * Fs * InstancePtr->Config.DWidth);
	}

	SClkOut_Div = (MClk/SClk);
	SClkOut_Div /= 2;

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
		XI2S_TX_TMR_CTRL_OFFSET, SClkOut_Div & XI2S_TX_CLK_MASK);
	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function gets the captured AES Channel Status bits.
 *
 * @param  InstancePtr is a pointer to the I2s Transmitter instance.
 * @param  AesChStatusBuf is a pointer to a buffer that is used for writing
 *         the AES Channel Status bits, this needs to be allocated by
 *         user application
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Tx_GetAesChStatus(XI2s_Tx *InstancePtr,
		u8 *AesChStatusBuf)
{
	int RegOffset = XI2S_TX_AES_CHSTS0_OFFSET;
	u32 *pBuf32 = (u32 *)AesChStatusBuf;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AesChStatusBuf != NULL);

	for (int i = 0; i < 24; i += 4) {
		*pBuf32 = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
				RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/*****************************************************************************/
/**
 *
 * This function clears the captured AES Channel Status bits. This will clear
 * all the 6 channel status registers.
 *
 * @param InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Tx_ClrAesChStatRegs(XI2s_Tx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
			(XI2S_TX_AES_CHSTS0_OFFSET), (u32)0);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_TX_AES_CHSTS1_OFFSET), (u32)0);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_TX_AES_CHSTS2_OFFSET), (u32)0);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_TX_AES_CHSTS3_OFFSET), (u32)0);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_TX_AES_CHSTS4_OFFSET), (u32)0);
	XI2s_Tx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_TX_AES_CHSTS5_OFFSET), (u32)0);
}
/*****************************************************************************/
/**
 * This function enables/disables the justification.
 *
 * @param  InstancePtr is a pointer to the XI2s Transmitter instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the justification.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Tx_JustifyEnable(XI2s_Tx *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET);

	if (Enable)
		RegValue |= XI2S_TX_REG_CTRL_JFE_MASK;
	else
		RegValue &= ~XI2S_TX_REG_CTRL_JFE_MASK;

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET, RegValue);
}
/*****************************************************************************/
/**
 * This function is to enable right/left justification.
 *
 * @param  InstancePtr is a pointer to the XI2s Transmitter instance.
 * @param  Justify is a enum to select the left or right justfication.
 *    - XI2S_TX_JUSTIFY_LEFT : Left justication
 *    - XI2S_TX_JUSTIFY_RIGHT : Right justification
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Tx_Justify(XI2s_Tx *InstancePtr, XI2s_Tx_Justification Justify)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Tx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET);

	if (Justify)
		RegValue |= XI2S_TX_REG_CTRL_LORJF_MASK;
	else
		RegValue &= ~XI2S_TX_REG_CTRL_LORJF_MASK;

	XI2s_Tx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_TX_CORE_CTRL_OFFSET, RegValue);
}
/** @} */
