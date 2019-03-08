/*******************************************************************************
 *
 * Copyright (C) 2015 - 2016 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xhdmiphy1.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xstatus.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"
#include "xhdmiphy1_hdmi.h"
#include "sleep.h"
#include "xhdmiphy1_gt.h"

/**************************** Function Prototypes *****************************/
static u32 XHdmiphy1_DrpAccess(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u16 Addr, u16 *Val);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves the configuration for this Video PHY instance and
 * fills in the InstancePtr->Config structure.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param	ConfigPtr is a pointer to the configuration structure that will
 *		be used to copy the settings from.
 * @param	EffectiveAddr is the device base address in the virtual memory
 *		space. If the address translation is not used, then the physical
 *		address is passed.
 *
 * @return	None.
 *
 * @note	Unexpected errors may occur if the address mapping is changed
 *		after this function is invoked.
 *
*******************************************************************************/
void XHdmiphy1_CfgInitialize(XHdmiphy1 *InstancePtr,
		XHdmiphy1_Config *ConfigPtr,
		UINTPTR EffectiveAddr)
{
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	u8 Sel;
#endif

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);
	Xil_AssertVoid(EffectiveAddr != 0x0);

	(void)memset((void *)InstancePtr, 0, sizeof(XHdmiphy1));
	InstancePtr->IsReady = 0;

	InstancePtr->Config = *ConfigPtr;
	InstancePtr->Config.BaseAddr = EffectiveAddr;

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE3)
	InstancePtr->GtAdaptor = &Gthe3Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
	InstancePtr->GtAdaptor = &Gthe4Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	InstancePtr->GtAdaptor = &Gtye4Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
	InstancePtr->GtAdaptor = &Gtye5Config;
#endif

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	const XHdmiphy1_SysClkDataSelType SysClkCfg[7][2] = {
		{(XHdmiphy1_SysClkDataSelType)0,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)1,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)2,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)3,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)4,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_PLL0_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)5,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_PLL1_OUTCLK},
		{(XHdmiphy1_SysClkDataSelType)6,
            XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
	};
	for (Sel = 0; Sel < 7; Sel++) {
		if (InstancePtr->Config.TxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.TxSysPllClkSel = SysClkCfg[Sel][1];
		}
		if (InstancePtr->Config.RxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.RxSysPllClkSel = SysClkCfg[Sel][1];
		}
	}

	InstancePtr->Config.TxRefClkSel = (XHdmiphy1_PllRefClkSelType)
			(InstancePtr->Config.TxRefClkSel +
					XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.RxRefClkSel = (XHdmiphy1_PllRefClkSelType)
			(InstancePtr->Config.RxRefClkSel +
					XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.TxFrlRefClkSel = (XHdmiphy1_PllRefClkSelType)
			(InstancePtr->Config.TxFrlRefClkSel +
					XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.RxFrlRefClkSel = (XHdmiphy1_PllRefClkSelType)
			(InstancePtr->Config.RxFrlRefClkSel +
					XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.DruRefClkSel = (XHdmiphy1_PllRefClkSelType)
			(InstancePtr->Config.DruRefClkSel +
					XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0);
#endif

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
/*****************************************************************************/
/**
* This function will initialize the PLL selection for a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	QpllRefClkSel is the QPLL reference clock selection for the
*		quad.
* @param	CpllRefClkSel is the CPLL reference clock selection for the
*		quad.
* @param	TxPllSelect is the reference clock selection for the quad's
*		TX PLL dividers.
* @param	RxPllSelect is the reference clock selection for the quad's
*		RX PLL dividers.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_PllInitialize(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId,
		XHdmiphy1_PllRefClkSelType QpllRefClkSel,
		XHdmiphy1_PllRefClkSelType CpllRefClkSel,
		XHdmiphy1_PllType TxPllSelect, XHdmiphy1_PllType RxPllSelect)
{
	/* Suppress Warning Messages */
	ChId = ChId;

	/* Set configuration in software. */
    XHdmiphy1_CfgPllRefClkSel(InstancePtr, QuadId,
            XHDMIPHY1_CHANNEL_ID_CMNA, QpllRefClkSel);
    XHdmiphy1_CfgPllRefClkSel(InstancePtr, QuadId,
            XHDMIPHY1_CHANNEL_ID_CHA, CpllRefClkSel);
	XHdmiphy1_CfgSysClkDataSel(InstancePtr, QuadId, XHDMIPHY1_DIR_TX,
			Pll2SysClkData(TxPllSelect));
	XHdmiphy1_CfgSysClkDataSel(InstancePtr, QuadId, XHDMIPHY1_DIR_RX,
			Pll2SysClkData(RxPllSelect));
	XHdmiphy1_CfgSysClkOutSel(InstancePtr, QuadId, XHDMIPHY1_DIR_TX,
			Pll2SysClkOut(TxPllSelect));
	XHdmiphy1_CfgSysClkOutSel(InstancePtr, QuadId, XHDMIPHY1_DIR_RX,
			Pll2SysClkOut(RxPllSelect));

	/* Write configuration to hardware at once. */
	XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, QuadId);

	return XST_SUCCESS;
}
#endif

/******************************************************************************/
/*
* This function installs a custom delay/sleep function to be used by the
* XHdmiphy1 driver.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item (microseconds to delay) that
*		will be passed to the custom sleep/delay function when it is
*		invoked.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XHdmiphy1_SetUserTimerHandler(XHdmiphy1 *InstancePtr,
		XHdmiphy1_TimerHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->UserTimerWaitUs = CallbackFunc;
	InstancePtr->UserTimerPtr = CallbackRef;
}

/******************************************************************************/
/**
* This function is the delay/sleep function for the XHdmiphy1 driver. For the
* Zynq family, there exists native sleep functionality. For MicroBlaze however,
* there does not exist such functionality. In the MicroBlaze case, the default
* method for delaying is to use a predetermined amount of loop iterations. This
* method is prone to inaccuracy and dependent on system configuration; for
* greater accuracy, the user may supply their own delay/sleep handler, pointed
* to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
* hardware timer is used.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 instance.
* @param	MicroSeconds is the number of microseconds to delay/sleep for.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XHdmiphy1_WaitUs(XHdmiphy1 *InstancePtr, u32 MicroSeconds)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (MicroSeconds == 0) {
		return;
	}

	if (InstancePtr->UserTimerWaitUs != NULL) {
		/* Use the timer handler specified by the user for better
		 * accuracy. */
		InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
	}
	else {
	    /* Wait the requested amount of time. */
	    usleep(MicroSeconds);
	}
}

/*****************************************************************************/
/**
* This function will obtian the IP version.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	The IP version of the Video PHY core.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_GetVersion(XHdmiphy1 *InstancePtr)
{
	return XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_VERSION_REG);
}

/*****************************************************************************/
/**
* Configure the channel's line rate. This is a software only configuration and
* this value is used in the PLL calculator.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	LineRate is the line rate to configure software.
*
* @return
*		- XST_SUCCESS if the reference clock type is valid.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_CfgLineRate(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u64 LineRateHz)
{
	u8 Id;
	u8 Id0;
	u8 Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].LineRateHz =
								LineRateHz;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Obtain the channel's PLL reference clock selection.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The PLL type being used by the channel.
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_PllType XHdmiphy1_GetPllType(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId)
{
	XHdmiphy1_PllType PllType;
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	XHdmiphy1_SysClkDataSelType SysClkDataSel;
	XHdmiphy1_SysClkOutSelType SysClkOutSel;

	SysClkDataSel = XHdmiphy1_GetSysClkDataSel(InstancePtr, QuadId, Dir, ChId);
	SysClkOutSel = XHdmiphy1_GetSysClkOutSel(InstancePtr, QuadId, Dir, ChId);

	/* The sysclk data and output reference clocks should match. */

	if ((SysClkDataSel == XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK) &&
	(SysClkOutSel == XHDMIPHY1_SYSCLKSELOUT_TYPE_CPLL_REFCLK)) {
		PllType = XHDMIPHY1_PLL_TYPE_CPLL;
	}
	else if ((SysClkDataSel == XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) &&
	(SysClkOutSel == XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL_REFCLK)) {
		PllType = XHDMIPHY1_PLL_TYPE_QPLL;
	}
	else if ((SysClkDataSel == XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK) &&
	(SysClkOutSel == XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL0_REFCLK)) {
		PllType = XHDMIPHY1_PLL_TYPE_QPLL0;
	}
	else if ((SysClkDataSel == XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK) &&
	(SysClkOutSel == XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL1_REFCLK)) {
		PllType = XHDMIPHY1_PLL_TYPE_QPLL1;
	}
	else {
		PllType = XHDMIPHY1_PLL_TYPE_UNKNOWN;
	}
#else
	/* Suppress Warning Messages */
	QuadId = QuadId;
	ChId = ChId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		PllType = (XHdmiphy1_PllType) InstancePtr->Config.TxSysPllClkSel-2;
	}
	else if (Dir == XHDMIPHY1_DIR_RX) {
		PllType = (XHdmiphy1_PllType) InstancePtr->Config.RxSysPllClkSel-2;
	}
	else {
		PllType = XHDMIPHY1_PLL_TYPE_UNKNOWN;
	}
#endif

	return PllType;
}

/*****************************************************************************/
/**
* This function will return the line rate in Hz for a given channel / quad.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to check.
* @param	ChId is the channel ID for which to retrieve the line rate.
*
* @return	The line rate in Hz.
*
* @note		None.
*
******************************************************************************/
u64 XHdmiphy1_GetLineRateHz(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((ChId < XHDMIPHY1_CHANNEL_ID_CMNA));

	XHdmiphy1_ChannelId Ch_Id = ChId;

	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		Ch_Id = XHDMIPHY1_CHANNEL_ID_CH1;
	} else if (ChId == XHDMIPHY1_CHANNEL_ID_CMNA) {
		Ch_Id = XHDMIPHY1_CHANNEL_ID_CMN0;
	}

	return InstancePtr->Quads[QuadId].Plls[Ch_Id -
		XHDMIPHY1_CHANNEL_ID_CH1].LineRateHz;
}

/*****************************************************************************/
/**
* This function will reset the GT's PLL logic.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Hold is an indicator whether to "hold" the reset if set to 1.
*		If set to 0: reset, then enable.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_ResetGtPll(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_TX_INIT_REG;
	}
	else {
		RegOffset = XHDMIPHY1_RX_INIT_REG;
	}
	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		MaskVal = XHDMIPHY1_TXRX_INIT_PLLGTRESET_ALL_MASK;
	}
	else {
		MaskVal = XHDMIPHY1_TXRX_INIT_PLLGTRESET_MASK(ChId);
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will reset the GT's TX/RX logic.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Hold is an indicator whether to "hold" the reset if set to 1.
*		If set to 0: reset, then enable.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_ResetGtTxRx(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_TX_INIT_REG;
	}
	else {
		RegOffset = XHDMIPHY1_RX_INIT_REG;
	}
	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		MaskVal =  XHDMIPHY1_TXRX_INIT_GTRESET_ALL_MASK;
	}
	else {
		MaskVal = XHDMIPHY1_TXRX_INIT_GTRESET_MASK(ChId);
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set/clear the TX/RX polarity bit.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Polarity 0-Not inverted 1-Inverted
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_SetPolarity(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Polarity)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_TX_CONTROL_REG;
		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal =  XHDMIPHY1_TX_CONTROL_TXPOLARITY_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_TX_CONTROL_TXPOLARITY_MASK(ChId);
		}
	}
	else {
		RegOffset = XHDMIPHY1_RX_CONTROL_REG;
		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal = XHDMIPHY1_RX_CONTROL_RXPOLARITY_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_RX_CONTROL_RXPOLARITY_MASK(ChId);
		}
	}

	/* Read TX|RX Control Register */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Clear Polarity Register bits */
	RegVal &= ~MaskVal;

	if (Polarity) {
		RegVal |= MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the TX/RXPRBSEL of the GT
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Pattern is the pattern XHdmiphy1_PrbsPattern
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_SetPrbsSel(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        XHdmiphy1_PrbsPattern Pattern)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;
	u32 PrbsEnc = 0x0;
	u8 Id, Id0, Id1;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_TX_CONTROL_REG;
		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal =  XHDMIPHY1_TX_CONTROL_TXPRBSSEL_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_TX_CONTROL_TXPRBSSEL_MASK(ChId);
		}
		PrbsEnc = (u32) (((Pattern & 0x8) << 1) | (Pattern & 0x7));
	}
	else {
		RegOffset = XHDMIPHY1_RX_CONTROL_REG;
		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal = XHDMIPHY1_RX_CONTROL_RXPRBSSEL_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_RX_CONTROL_RXPRBSSEL_MASK(ChId);
		}
		PrbsEnc = (u32) (Pattern & 0xF);
	}

	/* Read TX|RX Control Register */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Mask out PRBS Register bits */
	RegVal &= ~MaskVal;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		RegVal |= PrbsEnc << ((Dir == XHDMIPHY1_DIR_TX) ?
					XHDMIPHY1_TX_CONTROL_TXPRBSSEL_SHIFT(Id) :
					XHDMIPHY1_RX_CONTROL_RXPRBSSEL_SHIFT(Id));
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the TX/RXPRBSEL of the GT
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ForceErr 0-No Error 1-Force Error
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_TxPrbsForceError(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, u8 ForceErr)
{
	u32 RegVal;
	u32 MaskVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		MaskVal =  XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_ALL_MASK;
	}
	else {
		MaskVal = XHDMIPHY1_TX_CONTROL_TXPRBSFORCEERR_MASK(ChId);
	}

	/* Read TX|RX Control Register */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_TX_CONTROL_REG);

	/* Clear Polarity Register bits */
	RegVal &= ~MaskVal;

	if (ForceErr) {
		RegVal |= MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_TX_CONTROL_REG, RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the TX voltage swing value for a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Vs is the voltage swing value to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetTxVoltageSwing(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, u8 Vs)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
        (ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH34_REG;
	}
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XHDMIPHY1_TX_DRIVER_TXDIFFCTRL_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Vs << XHDMIPHY1_TX_DRIVER_TXDIFFCTRL_SHIFT(ChId));
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will set the TX pre-emphasis value for a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Pe is the pre-emphasis value to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetTxPreEmphasis(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Pe)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
        (ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH34_REG;
	}
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XHDMIPHY1_TX_DRIVER_TXPRECURSOR_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Pe << XHDMIPHY1_TX_DRIVER_TXPRECURSOR_SHIFT(ChId));
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will set the TX post-curosr value for a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Pe is the pre-emphasis value to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetTxPostCursor(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Pc)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
        (ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XHDMIPHY1_TX_DRIVER_CH34_REG;
	}
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Pc << XHDMIPHY1_TX_DRIVER_TXPOSTCURSOR_SHIFT(ChId));
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will enable or disable the LPM logic in the Video PHY core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Enable will enable (if 1) or disable (if 0) the LPM logic.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetRxLpm(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Enable)
{
	u32 RegVal;
	u32 MaskVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;
	Dir = Dir;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
							XHDMIPHY1_RX_EQ_CDR_REG);

	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		MaskVal = XHDMIPHY1_RX_CONTROL_RXLPMEN_ALL_MASK;
	}
	else {
		MaskVal = XHDMIPHY1_RX_CONTROL_RXLPMEN_MASK(ChId);
	}

	if (Enable) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_RX_EQ_CDR_REG,
									RegVal);
}

/*****************************************************************************/
/**
* This function will initiate a write DRP transaction. It is a wrapper around
* XHdmiphy1_DrpAccess.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID on which to direct the DRP access.
* @param	Addr is the DRP address to issue the DRP access to.
* @param	Val is the value to write to the DRP address.
*
* @return
*		- XST_SUCCESS if the DRP access was successful.
*		- XST_FAILURE otherwise, if the busy bit did not go low, or if
*		  the ready bit did not go high.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_DrpWr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u16 Addr, u16 Val)
{
	return XHdmiphy1_DrpAccess(InstancePtr, QuadId, ChId,
			XHDMIPHY1_DIR_TX, /* Write. */
			Addr, &Val);
}

/*****************************************************************************/
/**
* This function will initiate a read DRP transaction. It is a wrapper around
* XHdmiphy1_DrpAccess.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID on which to direct the DRP access.
* @param	Addr is the DRP address to issue the DRP access to.
* @param	RetVal is the DRP read_value returned implicitly.
*
* @return
*		- XST_SUCCESS if the DRP access was successful.
*		- XST_FAILURE otherwise, if the busy bit did not go low, or if
*		  the ready bit did not go high.
*
* @note		None.
*
******************************************************************************/
u16 XHdmiphy1_DrpRd(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u16 Addr, u16 *RetVal)
{
	u32 Status;
	u16 Val;

	Status = XHdmiphy1_DrpAccess(InstancePtr, QuadId, ChId,
			XHDMIPHY1_DIR_RX, /* Read. */
			Addr, &Val);

    *RetVal = Val;

	return Status;
}

/*****************************************************************************/
/**
* This function will power down the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Hold is an indicator whether to "hold" the power down if set
*		to 1. If set to 0: power down, then power back up.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_MmcmPowerDown(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u8 Hold)
{
	u32 RegOffsetCtrl;
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffsetCtrl = XHDMIPHY1_MMCM_TXUSRCLK_CTRL_REG;
	}
	else {
		RegOffsetCtrl = XHDMIPHY1_MMCM_RXUSRCLK_CTRL_REG;
	}

	/* Power down. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XHDMIPHY1_MMCM_USRCLK_CTRL_PWRDWN_MASK;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Hold) {
		/* Power up. */
		RegVal &= ~XHDMIPHY1_MMCM_USRCLK_CTRL_PWRDWN_MASK;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function will start the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_MmcmStart(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir)
{
	XHdmiphy1_Mmcm *MmcmPtr;

	if (Dir == XHDMIPHY1_DIR_RX) {
		MmcmPtr= &InstancePtr->Quads[QuadId].RxMmcm;
		//TODO: MAGS Remove once confirm that VCLK is from MMCM
//		if (InstancePtr->RxHdmi21Cfg.IsEnabled == TRUE) {
//			return;
//		}
	}
	else {
		MmcmPtr= &InstancePtr->Quads[QuadId].TxMmcm;
		//TODO: MAGS Remove once confirm that VCLK is from MMCM
//		if (InstancePtr->TxHdmi21Cfg.IsEnabled == TRUE) {
//			return;
//		}
	}

	/* Check values if valid */
	if (!((MmcmPtr->ClkOut0Div > 0) && (MmcmPtr->ClkOut0Div <= 128) &&
		  (MmcmPtr->ClkOut1Div > 0) && (MmcmPtr->ClkOut1Div <= 128) &&
		  (MmcmPtr->ClkOut2Div > 0) && (MmcmPtr->ClkOut2Div <= 128))) {
		return;
	}

	/* Assert MMCM reset. */
	XHdmiphy1_MmcmReset(InstancePtr, QuadId, Dir, TRUE);

	/* Configure MMCM. */
	XHdmiphy1_MmcmWriteParameters(InstancePtr, QuadId, Dir);

	/* Release MMCM reset. */
	XHdmiphy1_MmcmReset(InstancePtr, QuadId, Dir, FALSE);

	/* Unmask the MMCM Lock */
	XHdmiphy1_MmcmLockedMaskEnable(InstancePtr, 0, Dir, FALSE);

	XHdmiphy1_LogWrite(InstancePtr, (Dir == XHDMIPHY1_DIR_TX) ?
		XHDMIPHY1_LOG_EVT_TXPLL_RECONFIG : XHDMIPHY1_LOG_EVT_RXPLL_RECONFIG, 1);
}

/*****************************************************************************/
/**
* This function enables the TX or RX IBUFDS peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Dir is an indicator for TX or RX.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the IBUFDS, respectively.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_IBufDsEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u8 Enable)
{
	XHdmiphy1_PllRefClkSelType *TypePtr, *DruTypePtr, DruTypeDummy;
	u32 RegAddr = XHDMIPHY1_IBUFDS_GTXX_CTRL_REG;
	u32 RegVal;
	u32 MaskVal = 0;
	DruTypeDummy = XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTGREFCLK;
	DruTypePtr = &DruTypeDummy;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		TypePtr = &InstancePtr->Config.TxRefClkSel;
	}
	else {
		TypePtr = &InstancePtr->Config.RxRefClkSel;
		if (InstancePtr->Config.DruIsPresent) {
			DruTypePtr = &InstancePtr->Config.DruRefClkSel;
		}
	}

	if ((*TypePtr == XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0) ||
			((InstancePtr->Config.DruIsPresent) &&
			(*DruTypePtr == XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0))) {
		MaskVal = XHDMIPHY1_IBUFDS_GTXX_CTRL_GTREFCLK0_CEB_MASK;
	}
	else if ((*TypePtr == XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK1) ||
			((InstancePtr->Config.DruIsPresent) &&
			(*DruTypePtr == XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK1))) {
		MaskVal = XHDMIPHY1_IBUFDS_GTXX_CTRL_GTREFCLK1_CEB_MASK;
	}
	else {
		if (Dir == XHDMIPHY1_DIR_TX) {
			RegAddr = XHDMIPHY1_MISC_TXUSRCLK_REG;
		}
		else {
			RegAddr = XHDMIPHY1_MISC_RXUSRCLK_REG;
		}
		MaskVal = XHDMIPHY1_MISC_XXUSRCLK_REFCLK_CEB_MASK;
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegAddr);

	if (Enable) {
		RegVal &= ~MaskVal;
	}
	else {
		RegVal |= MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegAddr, RegVal);
}

/*****************************************************************************/
/**
* This function enables the TX or RX CLKOUT1 OBUFTDS peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Dir is an indicator for TX or RX.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the OBUFTDS, respectively.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_Clkout1OBufTdsEnable(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir, u8 Enable)
{
	u32 RegVal;
	u32 RegOffset;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_MISC_TXUSRCLK_REG;
	}
	else {
		RegOffset = XHDMIPHY1_MISC_RXUSRCLK_REG;
	}

	/* Read XXUSRCLK MISC register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Write new value to XXUSRCLK MISC register. */
	if (Enable) {
		RegVal |= XHDMIPHY1_MISC_XXUSRCLK_CKOUT1_OEN_MASK;
	}
	else {
		RegVal &= ~XHDMIPHY1_MISC_XXUSRCLK_CKOUT1_OEN_MASK;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will initiate a DRP transaction (either read or write).
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID on which to direct the DRP access.
* @param	Dir is an indicator for write (TX) or read (RX).
* @param	Addr is the DRP address to issue the DRP access to.
* @param	Val is a pointer to the data value. In write mode, this pointer
*		will hold the value to write. In read mode, this pointer will
*		be populated with the read value.
*
* @return
*		- XST_SUCCESS if the DRP access was successful.
*		- XST_FAILURE otherwise, if the busy bit did not go low, or if
*		  the ready bit did not go high.
*
* @note		In read mode (Dir == XHDMIPHY1_DIR_RX), the data pointed to by Val
*		will be populated with the u16 value that was read._
*
******************************************************************************/
static u32 XHdmiphy1_DrpAccess(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u16 Addr, u16 *Val)
{
	u32 RegOffsetCtrl;
	u32 RegOffsetSts;
	u32 RegVal;
	u8 Retry;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	/* Determine which DRP registers to use based on channel. */
	if (XHDMIPHY1_ISCMN(ChId)) {
		RegOffsetCtrl = XHDMIPHY1_DRP_CONTROL_COMMON_REG;
		RegOffsetSts = XHDMIPHY1_DRP_STATUS_COMMON_REG;
	}
	else if (XHDMIPHY1_ISTXMMCM(ChId)) {
		RegOffsetCtrl = XHDMIPHY1_DRP_CONTROL_TXMMCM_REG;
		RegOffsetSts = XHDMIPHY1_DRP_STATUS_TXMMCM_REG;
	}
	else if (XHDMIPHY1_ISRXMMCM(ChId)) {
		RegOffsetCtrl = XHDMIPHY1_DRP_CONTROL_RXMMCM_REG;
		RegOffsetSts = XHDMIPHY1_DRP_STATUS_RXMMCM_REG;
	}
	else {
		RegOffsetCtrl = XHDMIPHY1_DRP_CONTROL_CH1_REG +
			(4 * XHDMIPHY1_CH2IDX(ChId));
		RegOffsetSts = XHDMIPHY1_DRP_STATUS_CH1_REG +
			(4 * (XHDMIPHY1_CH2IDX(ChId)));
	}

	/* Wait until the DRP status indicates that it is not busy.*/
	Retry = 0;
	do {
		RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
								RegOffsetSts);
		if (Retry > 150) {
			return XST_FAILURE;
		}
		Retry++;
	} while (RegVal & XHDMIPHY1_DRP_STATUS_DRPBUSY_MASK);

	/* Write the command to the channel's DRP. */
	RegVal = (Addr & XHDMIPHY1_DRP_CONTROL_DRPADDR_MASK);
	RegVal |= XHDMIPHY1_DRP_CONTROL_DRPEN_MASK;
	if (Dir == XHDMIPHY1_DIR_TX) {
		/* Enable write. */
		RegVal |= XHDMIPHY1_DRP_CONTROL_DRPWE_MASK;
		RegVal |= ((*Val << XHDMIPHY1_DRP_CONTROL_DRPDI_SHIFT) &
						XHDMIPHY1_DRP_CONTROL_DRPDI_MASK);
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	/* Wait until the DRP status indicates ready.*/
	Retry = 0;
	do {
		RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
								RegOffsetSts);
		if (Retry > 150) {
			return XST_FAILURE;
		}
		Retry++;
	} while (!(RegVal & XHDMIPHY1_DRP_STATUS_DRPRDY_MASK));

	if (Dir == XHDMIPHY1_DIR_RX) {
		/* Mask non-data out for read. */
		RegVal &= XHDMIPHY1_DRP_STATUS_DRPO_MASK;
		/* Populate Val with read contents. */
		*Val = RegVal;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function installs a callback function for the HDMIPHY error conditions
*
* @param	InstancePtr is a pointer to the XHdmiphy1 instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note		The XHdmiphy1_ErrorHandler API calls the registered function in
* 			  ErrorCallback and passes two arguments: 1) CallbackRef
* 			  2) Error Type as defined by XHdmiphy1_ErrType.
*
* 			Sample Function Call:
* 				CallbackFunc(CallbackRef, XHdmiphy1_ErrType);
*
******************************************************************************/
void XHdmiphy1_SetErrorCallback(XHdmiphy1 *InstancePtr,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->ErrorCallback = (XHdmiphy1_ErrorCallback)CallbackFunc;
	InstancePtr->ErrorRef = CallbackRef;
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the LogWrite
* API:
*
* @param    InstancePtr is a pointer to the XHdmiphy1 instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_SetLogCallback(XHdmiphy1 *InstancePtr,
		u64 *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->LogWriteCallback = (XHdmiphy1_LogCallback)CallbackFunc;
	InstancePtr->LogWriteRef = CallbackRef;
}

/*****************************************************************************/
/**
* This function prints out Video PHY register and GT Channel and Common
* DRP register contents.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_RegisterDebug(XHdmiphy1 *InstancePtr)
{
	u32 RegOffset;
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	u16 DrpAddr, MaxDrpAddr;
	u16 DrpVal, ChId;
	u8  MaxChannels;
#endif

	xil_printf("\r\nHDMIPHY Registers\r\n");
	xil_printf("-----------------\r\n");
	xil_printf("Offset   |  Value\r\n");
	xil_printf("-----------------\r\n");
	for (RegOffset = 0; RegOffset <= 0x334; ) {
		xil_printf("0x%04x      0x%08x\r\n",RegOffset,
		XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset));
		RegOffset += 4;
	}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE3)
	MaxDrpAddr = 0x00B0;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
	MaxDrpAddr = 0x00B0;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	MaxDrpAddr = 0x00B0;
#endif

	xil_printf("\r\nHDMIPHY GT COMMON DRP Registers\r\n");
	xil_printf("----------------------------\r\n");
	if (InstancePtr->HdmiIsQpllPresent == TRUE) {
		xil_printf("Offset   |  Value\r\n");
		xil_printf("----------------------------\r\n");
		for (DrpAddr = 0x0000; DrpAddr <= MaxDrpAddr; DrpAddr++) {
			XHdmiphy1_DrpRd(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CMN0,
					DrpAddr, &DrpVal);
			xil_printf("0x%04x      0x%04x\r\n",DrpAddr, DrpVal);
		}
	} else {
		xil_printf("No QPLL in this HDMIPHY Instance\r\n");
	}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE3)
	MaxDrpAddr = 0x015F;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
	MaxDrpAddr = 0x025F;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	MaxDrpAddr = 0x028C;
#endif
	/* Get Max number of channels in HDMIPHY */
	MaxChannels = (InstancePtr->Config.RxChannels >
					InstancePtr->Config.TxChannels) ?
					InstancePtr->Config.RxChannels :
					InstancePtr->Config.TxChannels;

	for (ChId = 1; ChId <= MaxChannels; ChId++) {
	xil_printf("\r\nHDMIPHY GT CHANNEL %d DRP Registers\r\n", ChId);
	xil_printf("-------------------------------\r\n");
	xil_printf("Offset   |  Value\r\n");
	xil_printf("-------------------------------\r\n");
		for (DrpAddr = 0x0000; DrpAddr <= MaxDrpAddr; DrpAddr++) {
			XHdmiphy1_DrpRd(InstancePtr, 0, ChId, DrpAddr, &DrpVal);
			xil_printf("0x%04x      0x%04x\r\n",DrpAddr, DrpVal);
		}
	}
#endif
}
