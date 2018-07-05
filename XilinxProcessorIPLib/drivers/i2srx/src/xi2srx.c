/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2srx.c
 * @addtogroup i2srx_v1_0
 * @{
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18  Initial release.
 * 1.1   kar    04/02/18  Changed Channel Status clear API to clear all regs.
 * 2.0   kar    09/28/18  Added new API to enable justification.
 *                        Added new API to select left/right justification.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx.h"
#include "xi2srx_chsts.h"
#include "xi2srx_hw.h"
#include "xi2srx_debug.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XI2S_RX_CLK_MASK (0xFF)

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function initializes the XI2s Receiver.
 * This function must be called prior to using the core.
 * Initialization of the XI2s Receiver includes
 * setting up the instance data, and ensuring the hardware is in a quiescent
 * state.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  CfgPtr points to the configuration structure associated with
 *         the XI2s Receiver.
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
 *****************************************************************************/
int XI2s_Rx_CfgInitialize(XI2s_Rx *InstancePtr,
		XI2srx_Config *CfgPtr,
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

	/* Run the selftest. */
	Status = XI2s_Rx_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Disable the core */
	XI2s_Rx_Enable(InstancePtr, FALSE);

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function enables/disables the XI2s Receiver.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the XI2s Receiver.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_Enable(XI2s_Rx *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET);

	if (Enable) {
		RegValue |= XI2S_RX_REG_CTRL_EN_MASK;
		InstancePtr->IsStarted = (XIL_COMPONENT_IS_STARTED);
	} else {
		RegValue &= ~XI2S_RX_REG_CTRL_EN_MASK;
		InstancePtr->IsStarted = 0;
	}

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET, RegValue);
}
/*****************************************************************************/
/**
 * This function requests the XI2s Receiver to latch
 * the AES Channel Status bits from the registers.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_LatchAesChannelStatus(XI2s_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET);

	RegValue |= XI2S_RX_REG_CTRL_LATCH_CHSTS_MASK;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET, RegValue);
}
/*****************************************************************************/
/**
 * This function enables the specified interrupt of the XI2s Receiver.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  Mask is a bit mask of the interrupts to be enabled.
 *
 * @return None.
 *
 * @see XI2srx_HW for the available interrupt masks.
 *
 *****************************************************************************/
void XI2s_Rx_IntrEnable(XI2s_Rx *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_IRQCTRL_OFFSET);

	RegValue |= Mask;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_IRQCTRL_OFFSET, RegValue);
}
/*****************************************************************************/
/**
 * This function disables the specified interrupt of the XI2s Receiver.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  Mask is a bit mask of the interrupts to be disabled.
 *
 * @return None.
 *
 * @see XI2s_Receiver_HW for the available interrupt masks.
 *
 *****************************************************************************/
void XI2s_Rx_IntrDisable(XI2s_Rx *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_IRQCTRL_OFFSET);
	RegValue &= ~Mask;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_IRQCTRL_OFFSET, RegValue);
}
/****************************************************************************/
/**
 * This function sets the input source for the specified AXI-Stream channel pair
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  ChID specifies the AXI-Stream channel pair
 *   - 0 : AXI-Stream channel 0 and 1
 *   - 1 : AXI-Stream channel 2 and 3
 *   - 2 : AXI-Stream channel 4 and 5
 *   - 3 : AXI-Stream channel 6 and 7
 * @param  InputSource specifies the input source
 *
 * @return
 *   - XST_SUCCESS : if successful.
 *   - XST_FAILURE : if the AXI-Stream channel pair is invalid.
 *
 *****************************************************************************/
int XI2s_Rx_SetChMux(XI2s_Rx *InstancePtr, XI2s_Rx_ChannelId ChID,
		XI2s_Rx_ChMuxInput InputSource)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (ChID > XI2S_RX_NUM_CHANNELS)
		return XST_FAILURE;

	int RegValue = 0;
	int RegOffset = XI2S_RX_CH01_OFFSET + (ChID * 4);

	switch (InputSource) {
		case XI2S_RX_CHMUX_XI2S_01:
			RegValue = InputSource;
			break;

		case XI2S_RX_CHMUX_XI2S_23:
			RegValue = InputSource;
			break;

		case XI2S_RX_CHMUX_XI2S_45:
			RegValue = InputSource;
			break;

		case XI2S_RX_CHMUX_XI2S_67:
			RegValue = InputSource;
			break;

		case XI2S_RX_CHMUX_WAVEGEN:
			RegValue = InputSource;
			break;

		default:
			RegValue = 0;
			break;
	}
	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			RegOffset, RegValue);
	return XST_SUCCESS;
}
/****************************************************************************/
/**
 * This function calculates the SCLK Output divider value of the
 * I2s timing generator.
 *
 * @param  InstancePtr is a pointer to the I2s Receiver instance.
 * @param  MClk is the frequency of the MClk.
 * @param  Fs is the sampling frequency of the system.
 * Divider value for the SCLK generation
 * MCLK/SCLK = SCLKOUT_DIV x 2
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
u32 XI2s_Rx_SetSclkOutDiv(XI2s_Rx *InstancePtr, u32 MClk, u32 Fs)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((MClk > 0) && (MClk > Fs));
	Xil_AssertNonvoid((Fs > 0) && (Fs < MClk));
	u32 SClk = (2 * InstancePtr->Config.DWidth) * Fs;
	u8 SClkOut_Div;

	SClkOut_Div = (MClk/SClk);
	SClkOut_Div /= 2;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
		XI2S_RX_TMR_CTRL_OFFSET, SClkOut_Div & XI2S_RX_CLK_MASK);

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function sets the AES Channel Status bits to insert.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  AesChStatusBuf is a pointer to a buffer containing the
 *         AES channel status bits.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_SetAesChStatus(XI2s_Rx *InstancePtr, u8 *AesChStatusBuf)
{
	int RegOffset = XI2S_RX_AES_CHSTS0_OFFSET;
	u32 *pBuf32 = (u32 *) AesChStatusBuf;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(AesChStatusBuf != NULL);

	for (int i = 0; i < 24; i += 4) {
		XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
				RegOffset, *pBuf32);
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
 * @param InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_ClrAesChStatRegs(XI2s_Rx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
			(XI2S_RX_AES_CHSTS0_OFFSET), (u32)0);
	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_AES_CHSTS1_OFFSET), (u32)0);
	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_AES_CHSTS2_OFFSET), (u32)0);
	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_AES_CHSTS3_OFFSET), (u32)0);
	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_AES_CHSTS4_OFFSET), (u32)0);
	XI2s_Rx_WriteReg((InstancePtr)->Config.BaseAddress,
				(XI2S_RX_AES_CHSTS5_OFFSET), (u32)0);
}
/*****************************************************************************/
/**
 * This function enables/disables the justification.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the justification.
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_JustifyEnable(XI2s_Rx *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET);

	if (Enable)
		RegValue |= XI2S_RX_REG_CTRL_JFE_MASK;
	else
		RegValue &= ~XI2S_RX_REG_CTRL_JFE_MASK;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET, RegValue);
}

/*****************************************************************************/
/**
 * This function is to enable right/left justification.
 *
 * @param  InstancePtr is a pointer to the XI2s Receiver instance.
 * @param  Justify is a enum to select the left or right justfication.
 *    - XI2S_RX_JUSTIFY_LEFT : Left justication
 *    - XI2S_RX_JUSTIFY_RIGHT : Right justification
 *
 * @return None.
 *
 *****************************************************************************/
void XI2s_Rx_Justify(XI2s_Rx *InstancePtr, XI2s_Rx_Justification Justify)
{
	Xil_AssertVoid(InstancePtr != NULL);

	u32 RegValue = XI2s_Rx_ReadReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET);

	if (Justify)
		RegValue |= XI2S_RX_REG_CTRL_LORJF_MASK;
	else
		RegValue &= ~XI2S_RX_REG_CTRL_LORJF_MASK;

	XI2s_Rx_WriteReg(InstancePtr->Config.BaseAddress,
			XI2S_RX_CORE_CTRL_OFFSET, RegValue);
}
/** @} */
