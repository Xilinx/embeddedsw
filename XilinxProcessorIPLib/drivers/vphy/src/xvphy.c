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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
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
 * @file xvphy.c
 *
 * Contains a minimal set of functions for the XVphy driver that allow access
 * to all of the Video PHY core's functionality. See xvphy.h for a detailed
 * description of the driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als, 10/19/15 Initial release.
 *       gm
 * 1.1   gm   02/01/16 Added GTPE2 and GTHE4 support
 *                     Added more events to XVphy_LogEvent definitions.
 *                     Added TxBufferBypass in XVphy_Config structure.
 *                     Added XVphy_SetDefaultPpc and XVphy_SetPpc functions.
 *       als           Added XVphy_GetLineRateHz function.
 *       gm   20/04/16 Added XVphy_GetRcfgChId function
 * 1.2   gm            Added HdmiFastSwitch in XVphy_Config
 *                     Fixed bug in XVphy_IsPllLocked function
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include <string.h>
#include "xstatus.h"
#include "xvphy.h"
#include "xvphy_hdmi.h"
#if defined(__MICROBLAZE__)
#include "microblaze_sleep.h"
#elif defined(__arm__)
#include "sleep.h"
#endif
#include "xvphy_gt.h"

/**************************** Function Prototypes *****************************/

extern void XVphy_Ch2Ids(XVphy *InstancePtr, XVphy_ChannelId ChId,
		u8 *Id0, u8 *Id1);
static void XVphy_SelQuad(XVphy *InstancePtr, u8 QuadId);
static u32 XVphy_MmcmWriteParameters(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir);
static inline XVphy_SysClkDataSelType Pll2SysClkData(XVphy_PllType PllSelect);
static inline XVphy_SysClkOutSelType Pll2SysClkOut(XVphy_PllType PllSelect);
static u32 XVphy_DrpAccess(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u16 Addr, u16 *Val);
static u32 XVphy_PllCalculator(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir,
		u32 PllClkInFreqHz);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves the configuration for this Video PHY instance and
 * fills in the InstancePtr->Config structure.
 *
 * @param	InstancePtr is a pointer to the XVphy instance.
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
void XVphy_CfgInitialize(XVphy *InstancePtr, XVphy_Config *ConfigPtr,
		u32 EffectiveAddr)
{
	u8 Sel;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ConfigPtr != NULL);
	Xil_AssertVoid(EffectiveAddr != 0x0);

	(void)memset((void *)InstancePtr, 0, sizeof(XVphy));
	InstancePtr->IsReady = 0;

	InstancePtr->Config = *ConfigPtr;
	InstancePtr->Config.BaseAddr = EffectiveAddr;

	if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTXE2) {
		InstancePtr->GtAdaptor = &Gtxe2Config;
	}
	else if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE2) {
		InstancePtr->GtAdaptor = &Gthe2Config;
	}
	else if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) {
		InstancePtr->GtAdaptor = &Gtpe2Config;
	}
	else if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE3) {
		InstancePtr->GtAdaptor = &Gthe3Config;
	}
	else {
		InstancePtr->GtAdaptor = &Gthe4Config;
	}

	const XVphy_SysClkDataSelType SysClkCfg[7][2] = {
		{0, XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK},
		{1, XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
		{2, XVPHY_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK},
		{3, XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK},
		{4, XVPHY_SYSCLKSELDATA_TYPE_PLL0_OUTCLK},
		{5, XVPHY_SYSCLKSELDATA_TYPE_PLL1_OUTCLK},
		{6, XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
	};
	for (Sel = 0; Sel < 7; Sel++) {
		if (InstancePtr->Config.TxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.TxSysPllClkSel = SysClkCfg[Sel][1];
		}
		if (InstancePtr->Config.RxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.RxSysPllClkSel = SysClkCfg[Sel][1];
		}
	}

	InstancePtr->Config.TxRefClkSel += XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0;
	InstancePtr->Config.RxRefClkSel += XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0;
	InstancePtr->Config.DruRefClkSel += XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0;

	/* Correct RefClkSel offsets for GTPE2 EAST and WEST RefClks */
	if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) {
		if (InstancePtr->Config.TxRefClkSel > 6) {
			InstancePtr->Config.TxRefClkSel -= 4;
		}
		if (InstancePtr->Config.RxRefClkSel > 6) {
			InstancePtr->Config.RxRefClkSel -= 4;
		}
		if (InstancePtr->Config.DruRefClkSel > 6) {
			InstancePtr->Config.DruRefClkSel -= 4;
		}
	}

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
}

/*****************************************************************************/
/**
* This function will initialize the PLL selection for a given channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	QpllRefClkSel is the QPLL reference clock selection for the
*		quad.
*		- In GTP, this is used to hold PLL0 refclk selection.
* @param	CpllRefClkSel is the CPLL reference clock selection for the
*		quad.
*		- In GTP, this is used to hold PLL1 refclk selection.
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
u32 XVphy_PllInitialize(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_PllRefClkSelType QpllRefClkSel,
		XVphy_PllRefClkSelType CpllRefClkSel,
		XVphy_PllType TxPllSelect, XVphy_PllType RxPllSelect)
{
	XVphy_SelQuad(InstancePtr, QuadId);

	/* Set configuration in software. */
	if (InstancePtr->Config.XcvrType != XVPHY_GT_TYPE_GTPE2) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMNA, QpllRefClkSel);
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CHA, CpllRefClkSel);
	}
	/* GTP. */
	else {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMN0,	QpllRefClkSel);
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMN1, CpllRefClkSel);
	}
	XVphy_CfgSysClkDataSel(InstancePtr, QuadId, XVPHY_DIR_TX,
			Pll2SysClkData(TxPllSelect));
	XVphy_CfgSysClkDataSel(InstancePtr, QuadId, XVPHY_DIR_RX,
			Pll2SysClkData(RxPllSelect));
	XVphy_CfgSysClkOutSel(InstancePtr, QuadId, XVPHY_DIR_TX,
			Pll2SysClkOut(TxPllSelect));
	XVphy_CfgSysClkOutSel(InstancePtr, QuadId, XVPHY_DIR_RX,
			Pll2SysClkOut(RxPllSelect));

	/* Write configuration to hardware at once. */
	XVphy_WriteCfgRefClkSelReg(InstancePtr, QuadId);

	return XST_SUCCESS;
}

/******************************************************************************/
/*
* This function installs a custom delay/sleep function to be used by the XVphy
* driver.
*
* @param	InstancePtr is a pointer to the XVphy instance.
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
void XVphy_SetUserTimerHandler(XVphy *InstancePtr,
		XVphy_TimerHandler CallbackFunc, void *CallbackRef)
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
* This function is the delay/sleep function for the XVphy driver. For the Zynq
* family, there exists native sleep functionality. For MicroBlaze however,
* there does not exist such functionality. In the MicroBlaze case, the default
* method for delaying is to use a predetermined amount of loop iterations. This
* method is prone to inaccuracy and dependent on system configuration; for
* greater accuracy, the user may supply their own delay/sleep handler, pointed
* to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
* hardware timer is used.
*
* @param	InstancePtr is a pointer to the XVphy instance.
* @param	MicroSeconds is the number of microseconds to delay/sleep for.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XVphy_WaitUs(XVphy *InstancePtr, u32 MicroSeconds)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (MicroSeconds == 0) {
		return;
	}

#if defined(__MICROBLAZE__)
	if (InstancePtr->UserTimerWaitUs != NULL) {
		/* Use the timer handler specified by the user for better
		 * accuracy. */
		InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
	}
	else {
		/* MicroBlaze sleep only has millisecond accuracy. Round up. */
		u32 MilliSeconds = (MicroSeconds + 999) / 1000;
		MB_Sleep(MilliSeconds);
	}
#elif defined(__arm__)
	/* Wait the requested amount of time. */
	usleep(MicroSeconds);
#endif
}

/*****************************************************************************/
/**
* This function will initialize the clocking for a given channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_ClkInitialize(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir)
{
	u32 Status;

	Status = XVphy_ClkCalcParams(InstancePtr, QuadId, ChId, Dir, 0);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XVphy_ClkReconfig(InstancePtr, QuadId, ChId);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XVphy_OutDivReconfig(InstancePtr, QuadId, ChId, Dir);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XVphy_DirReconfig(InstancePtr, QuadId, ChId, Dir);

	return Status;
}

/*****************************************************************************/
/**
* This function will obtian the IP version.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	The IP version of the Video PHY core.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_GetVersion(XVphy *InstancePtr)
{
	return XVphy_ReadReg(InstancePtr->Config.BaseAddr, XVPHY_VERSION_REG);
}

/*****************************************************************************/
/**
* This function will enable or disable the LPM logic in the Video PHY core.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
void XVphy_SetRxLpm(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Enable)
{
	u32 RegVal;
	u32 MaskVal;

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
							XVPHY_RX_EQ_CDR_REG);

	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal = XVPHY_RX_CONTROL_RXLPMEN_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_RX_CONTROL_RXLPMEN_MASK(ChId);
	}

	if (Enable) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_RX_EQ_CDR_REG,
									RegVal);
}

/*****************************************************************************/
/**
* This function will set the TX voltage swing value for a given channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Vs is the voltage swing value to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_SetTxVoltageSwing(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 Vs)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	if ((ChId == XVPHY_CHANNEL_ID_CH1) || (ChId == XVPHY_CHANNEL_ID_CH2)) {
		RegOffset = XVPHY_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XVPHY_TX_DRIVER_CH34_REG;
	}
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XVPHY_TX_DRIVER_TXDIFFCTRL_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Vs << XVPHY_TX_DRIVER_TXDIFFCTRL_SHIFT(ChId));
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will set the TX pre-emphasis value for a given channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Pe is the pre-emphasis value to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_SetTxPreEmphasis(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Pe)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	if ((ChId == XVPHY_CHANNEL_ID_CH1) || (ChId == XVPHY_CHANNEL_ID_CH2)) {
		RegOffset = XVPHY_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XVPHY_TX_DRIVER_CH34_REG;
	}
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XVPHY_TX_DRIVER_TXPRECURSOR_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Pe << XVPHY_TX_DRIVER_TXPRECURSOR_SHIFT(ChId));
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function writes the current software configuration for the reference
* clock selections to hardware for the specified quad on all channels.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_WriteCfgRefClkSelReg(XVphy *InstancePtr, u8 QuadId)
{
	u32 RegVal = 0;
	XVphy_Channel *ChPtr;
	XVphy_GtType GtType = InstancePtr->Config.XcvrType;

	XVphy_SelQuad(InstancePtr, QuadId);

	/* Point to the first channel since settings apply to all channels. */
	ChPtr = &InstancePtr->Quads[QuadId].Ch1;

	/* PllRefClkSel. */
	/* - QPLL0. */
	RegVal &= ~XVPHY_REF_CLK_SEL_QPLL0_MASK;
	RegVal = InstancePtr->Quads[QuadId].Cmn0.PllRefClkSel;
	/* - CPLL. */
	RegVal &= ~XVPHY_REF_CLK_SEL_CPLL_MASK;
	RegVal |= (ChPtr->CpllRefClkSel << XVPHY_REF_CLK_SEL_CPLL_SHIFT);
	if ((GtType == XVPHY_GT_TYPE_GTHE3) ||
            (GtType == XVPHY_GT_TYPE_GTHE4) ||
            (GtType == XVPHY_GT_TYPE_GTPE2)) {
		/* - QPLL1. */
		RegVal &= ~XVPHY_REF_CLK_SEL_QPLL1_MASK;
		RegVal |= (InstancePtr->Quads[QuadId].Cmn1.PllRefClkSel <<
				XVPHY_REF_CLK_SEL_QPLL1_SHIFT);
	}

	/* SysClkDataSel. PLLCLKSEL */
	RegVal &= ~XVPHY_REF_CLK_SEL_SYSCLKSEL_MASK;
	/* - TXSYSCLKSEL[0]. TXPLLCLKSEL*/
	RegVal |= (ChPtr->TxDataRefClkSel <<
		XVPHY_REF_CLK_SEL_TXSYSCLKSEL_DATA_SHIFT(GtType)) &
		XVPHY_REF_CLK_SEL_TXSYSCLKSEL_DATA_MASK(GtType);
	/* - RXSYSCLKSEL[0]. RXPLLCLKSEL*/
	RegVal |= (ChPtr->RxDataRefClkSel <<
		XVPHY_REF_CLK_SEL_RXSYSCLKSEL_DATA_SHIFT(GtType)) &
		XVPHY_REF_CLK_SEL_RXSYSCLKSEL_DATA_MASK(GtType);

	/* SysClkOutSel. */
	/* - TXSYSCLKSEL[1]. */
	RegVal |= (ChPtr->TxOutRefClkSel <<
		XVPHY_REF_CLK_SEL_TXSYSCLKSEL_OUT_SHIFT(GtType)) &
		XVPHY_REF_CLK_SEL_TXSYSCLKSEL_OUT_MASK(GtType);
	/* - RXSYSCLKSEL[1]. */
	RegVal |= (ChPtr->RxOutRefClkSel <<
		XVPHY_REF_CLK_SEL_RXSYSCLKSEL_OUT_SHIFT(GtType)) &
		XVPHY_REF_CLK_SEL_RXSYSCLKSEL_OUT_MASK(GtType);

	/* Write to hardware. */
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_REF_CLK_SEL_REG,
			RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Configure the channel's line rate. This is a software only configuration and
* this value is used in the PLL calculator.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_CfgLineRate(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u64 LineRateHz)
{
	u8 Id;
	u8 Id0;
	u8 Id1;

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(Id)].LineRateHz =
								LineRateHz;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Configure the quad's reference clock frequency. This is a software only
* configuration and this value is used in the PLL calculator.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	RefClkType is the reference clock type to operate on.
* @param	FreqHz is the reference clock frequency to configure software.
*
* @return
*		- XST_SUCCESS if the reference clock type is valid.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_CfgQuadRefClkFreq(XVphy *InstancePtr, u8 QuadId,
		XVphy_PllRefClkSelType RefClkType, u32 FreqHz)
{
	u8 RefClkIndex = RefClkType - XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0;

	if (RefClkType > XVPHY_PLL_REFCLKSEL_TYPE_GTGREFCLK) {
		return XST_FAILURE;
	}
	InstancePtr->Quads[QuadId].RefClkHz[RefClkIndex] = FreqHz;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Configure the PLL reference clock selection for the specified channel(s).
* This is applied to both direction to the software configuration only.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	SysClkDataSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_CfgPllRefClkSel(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_PllRefClkSelType RefClkSel)
{
	u8 Id, Id0, Id1;

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(Id)].PllRefClkSel =
					RefClkSel;
	}
}

/*****************************************************************************/
/**
* Configure the SYSCLKDATA reference clock selection for the direction. Same
* configuration applies to all channels in the quad. This is applied to the
* software configuration only.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	SysClkDataSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_CfgSysClkDataSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_SysClkDataSelType SysClkDataSel)
{
	XVphy_Channel *ChPtr;
	u8 Id, Id0, Id1;

	XVphy_Ch2Ids(InstancePtr, XVPHY_CHANNEL_ID_CHA, &Id0, &Id1);
	/* Select in software - same for all channels. */
	for (Id = Id0; Id <= Id1; Id++) {
		ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(Id)];
		ChPtr->DataRefClkSel[Dir] = SysClkDataSel;
	}
}

/*****************************************************************************/
/**
* Configure the SYSCLKOUT reference clock selection for the direction. Same
* configuration applies to all channels in the quad. This is applied to the
* software configuration only.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	SysClkOutSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_CfgSysClkOutSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_SysClkOutSelType SysClkOutSel)
{
	XVphy_Channel *ChPtr;
	u8 Id, Id0, Id1;

	XVphy_Ch2Ids(InstancePtr, XVPHY_CHANNEL_ID_CHA, &Id0, &Id1);
	/* Select in software - same for all channels. */
	for (Id = Id0; Id <= Id1; Id++) {
		ChPtr = &InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(Id)];
		ChPtr->OutRefClkSel[Dir] = SysClkOutSel;
	}
}

/*****************************************************************************/
/**
* Obtain the channel's PLL reference clock selection.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The PLL type being used by the channel.
*
* @note		None.
*
******************************************************************************/
XVphy_PllType XVphy_GetPllType(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_ChannelId ChId)
{
	XVphy_SysClkDataSelType SysClkDataSel;
	XVphy_SysClkOutSelType SysClkOutSel;
	XVphy_PllType PllType;

	SysClkDataSel = XVphy_GetSysClkDataSel(InstancePtr, QuadId, Dir, ChId);
	SysClkOutSel = XVphy_GetSysClkOutSel(InstancePtr, QuadId, Dir, ChId);

	/* The sysclk data and output reference clocks should match. */

	if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_CPLL_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_CPLL;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL0_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL0;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL1_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL1;
	}
	else {
		PllType = XVPHY_PLL_TYPE_UNKNOWN;
	}
	/* For GTHE2, GTHE3, GTHE4, and GTXE2. */
	if (InstancePtr->Config.XcvrType != XVPHY_GT_TYPE_GTPE2) {
		return PllType;
	}

	if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_PLL0_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_PLL0_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_PLL0;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_PLL1_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_PLL1_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_PLL1;
	}
	else {
		PllType = XVPHY_PLL_TYPE_UNKNOWN;
	}
	/* For GTPE2. */
	return PllType;
}

/*****************************************************************************/
/**
* Obtain the reconfiguration channel ID for given PLL type
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	PllType is the PLL type being used by the channel.
*
* @return	The Channel ID to be used for reconfiguration
*
* @note		None.
*
******************************************************************************/
XVphy_ChannelId XVphy_GetRcfgChId(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_PllType PllType)
{
	XVphy_ChannelId ChId;

	/* Determine which channel(s) to operate on. */
	switch (PllType) {
	case XVPHY_PLL_TYPE_QPLL:
	case XVPHY_PLL_TYPE_QPLL0:
	case XVPHY_PLL_TYPE_PLL0:
		ChId = XVPHY_CHANNEL_ID_CMN0;
		break;
	case XVPHY_PLL_TYPE_QPLL1:
	case XVPHY_PLL_TYPE_PLL1:
		ChId = XVPHY_CHANNEL_ID_CMN1;
		break;
	default:
		ChId = XVPHY_CHANNEL_ID_CHA;
		break;
	}

	return ChId;
}

/*****************************************************************************/
/**
* Obtain the current reference clock frequency for the quad based on the
* reference clock type.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	RefClkType is the type to obtain the clock selection for.
*
* @return	The current reference clock frequency for the quad for the
*		specified type selection.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_GetQuadRefClkFreq(XVphy *InstancePtr, u8 QuadId,
		XVphy_PllRefClkSelType RefClkType)
{
	u32 FreqHz;

	u8 RefClkIndex = RefClkType - XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0;

	FreqHz = (RefClkType > XVPHY_PLL_REFCLKSEL_TYPE_GTGREFCLK) ? 0 :
		InstancePtr->Quads[QuadId].RefClkHz[RefClkIndex];

	return FreqHz;
}

/*****************************************************************************/
/**
* Obtain the current PLL reference clock selection.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
*
* @return	The current PLL reference clock selection.
*
* @note		None.
*
******************************************************************************/
XVphy_PllRefClkSelType XVphy_GetPllRefClkSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId)
{
	u32 Sel;
	u32 RegVal;

	Xil_AssertNonvoid((XVPHY_CHANNEL_ID_CH1 <= ChId) &&
			(ChId <= XVPHY_CHANNEL_ID_CMN1));

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
							XVPHY_REF_CLK_SEL_REG);

	/* Synchronize software configuration to hardware. */
	if (ChId == XVPHY_CHANNEL_ID_CMN0) {
		Sel = RegVal & XVPHY_REF_CLK_SEL_QPLL0_MASK;
	}
	else if (ChId == XVPHY_CHANNEL_ID_CMN1) {
		Sel = RegVal & XVPHY_REF_CLK_SEL_QPLL1_MASK;
		Sel >>= XVPHY_REF_CLK_SEL_QPLL1_SHIFT;
	}
	else {
		Sel = RegVal & XVPHY_REF_CLK_SEL_CPLL_MASK;
		Sel >>= XVPHY_REF_CLK_SEL_CPLL_SHIFT;
	}

	return Sel;
}

/*****************************************************************************/
/**
* Obtain the current [RT]XSYSCLKSEL[0] configuration.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The current [RT]XSYSCLKSEL[0] selection.
*
* @note		None.
*
******************************************************************************/
XVphy_SysClkDataSelType XVphy_GetSysClkDataSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_ChannelId ChId)
{
	u32 Sel;
	u32 RegVal;

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
							XVPHY_REF_CLK_SEL_REG);

	if (Dir == XVPHY_DIR_TX) {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XVPHY_REF_CLK_SEL_TXSYSCLKSEL_DATA_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XVPHY_REF_CLK_SEL_TXSYSCLKSEL_DATA_SHIFT(
				InstancePtr->Config.XcvrType);
	}
	else {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XVPHY_REF_CLK_SEL_RXSYSCLKSEL_DATA_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XVPHY_REF_CLK_SEL_RXSYSCLKSEL_DATA_SHIFT(
				InstancePtr->Config.XcvrType);
	}

	return Sel;
}

/*****************************************************************************/
/**
* Obtain the current [RT]XSYSCLKSEL[1] configuration.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The current [RT]XSYSCLKSEL[1] selection.
*
* @note		None.
*
******************************************************************************/
XVphy_SysClkOutSelType XVphy_GetSysClkOutSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_ChannelId ChId)
{
	u32 Sel;
	u32 RegVal;

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
			XVPHY_REF_CLK_SEL_REG);

	if (Dir == XVPHY_DIR_TX) {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XVPHY_REF_CLK_SEL_TXSYSCLKSEL_OUT_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XVPHY_REF_CLK_SEL_TXSYSCLKSEL_OUT_SHIFT(
				InstancePtr->Config.XcvrType);
	}
	else {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XVPHY_REF_CLK_SEL_RXSYSCLKSEL_OUT_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XVPHY_REF_CLK_SEL_RXSYSCLKSEL_OUT_SHIFT(
				InstancePtr->Config.XcvrType);
	}

	return Sel;
}

/*****************************************************************************/
/**
* This function will return the line rate in Hz for a given channel / quad.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to check.
* @param	ChId is the channel ID for which to retrieve the line rate.
*
* @return	The line rate in Hz.
*
* @note		None.
*
******************************************************************************/
u64 XVphy_GetLineRateHz(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((XVPHY_CHANNEL_ID_CH1 <= ChId) &&
			(ChId <= XVPHY_CHANNEL_ID_CMN1));

	return InstancePtr->Quads[QuadId].Plls[ChId -
		XVPHY_CHANNEL_ID_CH1].LineRateHz;
}

/*****************************************************************************/
/**
* This function will wait for a PMA reset done on the specified channel(s) or
* time out.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the PMA reset has finalized.
*		- XST_FAILURE otherwise; waiting for the reset done timed out.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_WaitForPmaResetDone(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;
	u8 Retry = 0;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_INIT_STATUS_REG;
	}
	else {
		RegOffset = XVPHY_RX_INIT_STATUS_REG;
	}
	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal = XVPHY_TXRX_INIT_STATUS_PMARESETDONE_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_TXRX_INIT_STATUS_PMARESETDONE_MASK(ChId);
	}
	do {
		RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
		if (!(RegVal & MaskVal)){
			XVphy_WaitUs(InstancePtr, 1000);
			Retry++;
		}
	} while ((!(RegVal & MaskVal)) && (Retry < 15));

	if (Retry == 15){
		return XST_FAILURE;
	}
	else {
		return XST_SUCCESS;
	}
}

/*****************************************************************************/
/**
* This function will wait for a reset done on the specified channel(s) or time
* out.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the reset has finalized.
*		- XST_FAILURE otherwise; waiting for the reset done timed out.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_WaitForResetDone(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;
	u8 Retry = 0;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_INIT_STATUS_REG;
	}
	else {
		RegOffset = XVPHY_RX_INIT_STATUS_REG;
	}
	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal = XVPHY_TXRX_INIT_STATUS_RESETDONE_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_TXRX_INIT_STATUS_RESETDONE_MASK(ChId);
	}
	do {
		RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
		if (!(RegVal & MaskVal)){
			XVphy_WaitUs(InstancePtr, 1000);
			Retry++;
		}
	} while ((!(RegVal & MaskVal)) && (Retry < 15));

	if (Retry == 15){
		return XST_FAILURE;
	}
	else {
		return XST_SUCCESS;
	}
}

/*****************************************************************************/
/**
* This function will wait for a PLL lock on the specified channel(s) or time
* out.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
*
* @return
*		- XST_SUCCESS if the PLL(s) have locked.
*		- XST_FAILURE otherwise; waiting for the lock timed out.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_WaitForPllLock(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	u32 Status = XST_FAILURE;
	u8 Retry = 0;

	do {
		XVphy_WaitUs(InstancePtr, 1000);
		Status = XVphy_IsPllLocked(InstancePtr, QuadId, ChId);
		Retry++;
	} while ((Status != XST_SUCCESS) && (Retry < 15));

	return Status;
}

/*****************************************************************************/
/**
* This function will check the status of a PLL lock on the specified channel.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
*
* @return
*		- XST_SUCCESS if the specified PLL is locked.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_IsPllLocked(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	u32 RegVal;
	u32 MaskVal;
	XVphy_PllType TxPllType;
	XVphy_PllType RxPllType;

	if (ChId == XVPHY_CHANNEL_ID_CMN0) {
		MaskVal = XVPHY_PLL_LOCK_STATUS_QPLL0_MASK;
	}
	else if (ChId == XVPHY_CHANNEL_ID_CMN1) {
		MaskVal = XVPHY_PLL_LOCK_STATUS_QPLL1_MASK;
	}
	else if (ChId == XVPHY_CHANNEL_ID_CMNA) {
		MaskVal = XVPHY_PLL_LOCK_STATUS_QPLL0_MASK |
			  XVPHY_PLL_LOCK_STATUS_QPLL1_MASK;
	}
	else if (ChId == XVPHY_CHANNEL_ID_CHA) {
		TxPllType = XVphy_GetPllType(InstancePtr, 0, XVPHY_DIR_TX,
				XVPHY_CHANNEL_ID_CH1);
		RxPllType = XVphy_GetPllType(InstancePtr, 0, XVPHY_DIR_RX,
				XVPHY_CHANNEL_ID_CH1);
		if (RxPllType == XVPHY_PLL_TYPE_CPLL &&
				InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI) {
			MaskVal = XVPHY_PLL_LOCK_STATUS_CPLL_HDMI_MASK;
		}
		else if (TxPllType == XVPHY_PLL_TYPE_CPLL &&
				InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) {
			MaskVal = XVPHY_PLL_LOCK_STATUS_CPLL_HDMI_MASK;
		}
		else {
			MaskVal = XVPHY_PLL_LOCK_STATUS_CPLL_ALL_MASK;
		}
	}
	else {
		MaskVal = XVPHY_PLL_LOCK_STATUS_CPLL_MASK(ChId);
	}
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
			XVPHY_PLL_LOCK_STATUS_REG);

	if ((RegVal & MaskVal) == MaskVal) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;
}

/*****************************************************************************/
/**
* This function will reset the GT's PLL logic.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_ResetGtPll(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_INIT_REG;
	}
	else {
		RegOffset = XVPHY_RX_INIT_REG;
	}
	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal = XVPHY_TXRX_INIT_PLLGTRESET_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_TXRX_INIT_PLLGTRESET_MASK(ChId);
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will reset the GT's TX/RX logic.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_ResetGtTxRx(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_INIT_REG;
	}
	else {
		RegOffset = XVPHY_RX_INIT_REG;
	}
	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal =  XVPHY_TXRX_INIT_GTRESET_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_TXRX_INIT_GTRESET_MASK(ChId);
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will reset and enable the Video PHY's user core logic.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_GtUserRdyEnable(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_INIT_REG;

		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_TX_INIT_USERRDY_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_TX_INIT_USERRDY_MASK(ChId);
		}
	}
	else {
		RegOffset = XVPHY_RX_INIT_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_RX_INIT_USERRDY_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_RX_INIT_USERRDY_MASK(ChId);
		}
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will reset the specified GT quad.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if a valid PLL was specified.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_ResetGt(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir)
{
	XVphy_SysClkDataSelType SysClkDataSel;
	XVphy_SysClkOutSelType SysClkOutSel;
	XVphy_PllType PllType;

	/* All channels are using the same settings. */
	SysClkDataSel = XVphy_GetSysClkDataSel(InstancePtr, QuadId, Dir,
							XVPHY_CHANNEL_ID_CH1);
	SysClkOutSel = XVphy_GetSysClkOutSel(InstancePtr, QuadId, Dir,
							XVPHY_CHANNEL_ID_CH1);

	/* The sysclk data and output reference clocks should match. */
	if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_CPLL_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_CPLL;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL0_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL0;
	}
	else if ((SysClkDataSel == XVPHY_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK) &&
	(SysClkOutSel == XVPHY_SYSCLKSELOUT_TYPE_QPLL1_REFCLK)) {
		PllType = XVPHY_PLL_TYPE_QPLL1;
	}
	else {
		return XST_FAILURE;
	}

	XVphy_ResetGtPll(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA,
					(PllType == XVPHY_PLL_TYPE_CPLL) ?
					XVPHY_DIR_RX : XVPHY_DIR_TX, 0);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will initiate a write DRP transaction. It is a wrapper around
* XVphy_DrpAccess.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID on which to direct the DRP access.
* @param	Dir is an indicator for write (TX) or read (RX).
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
u32 XVphy_DrpWrite(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u16 Addr, u16 Val)
{
	return XVphy_DrpAccess(InstancePtr, QuadId, ChId,
			XVPHY_DIR_TX, /* Write. */
			Addr, &Val);
}

/*****************************************************************************/
/**
* This function will initiate a read DRP transaction. It is a wrapper around
* XVphy_DrpAccess.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID on which to direct the DRP access.
* @param	Dir is an indicator for write (TX) or read (RX).
* @param	Addr is the DRP address to issue the DRP access to.
*
* @return
*		- XST_SUCCESS if the DRP access was successful.
*		- XST_FAILURE otherwise, if the busy bit did not go low, or if
*		  the ready bit did not go high.
*
* @note		None.
*
******************************************************************************/
u16 XVphy_DrpRead(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId, u16 Addr)
{
	u32 Status;
	u16 Val;

	Status = XVphy_DrpAccess(InstancePtr, QuadId, ChId,
			XVPHY_DIR_RX, /* Read. */
			Addr, &Val);

	return (Status == XST_SUCCESS) ? Val : 0;
}

/*****************************************************************************/
/**
* This function will reset the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
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
void XVphy_MmcmReset(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir,
		u8 Hold)
{
	u32 RegOffsetCtrl;
	u32 RegVal;

	XVphy_SelQuad(InstancePtr, QuadId);

	if (Dir == XVPHY_DIR_TX) {
		RegOffsetCtrl = XVPHY_MMCM_TXUSRCLK_CTRL_REG;
	}
	else {
		RegOffsetCtrl = XVPHY_MMCM_RXUSRCLK_CTRL_REG;
	}

	/* Assert reset. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XVPHY_MMCM_USRCLK_CTRL_RST_MASK;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~XVPHY_MMCM_USRCLK_CTRL_RST_MASK;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function will power down the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
void XVphy_MmcmPowerDown(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir,
		u8 Hold)
{
	u32 RegOffsetCtrl;
	u32 RegVal;

	XVphy_SelQuad(InstancePtr, QuadId);

	if (Dir == XVPHY_DIR_TX) {
		RegOffsetCtrl = XVPHY_MMCM_TXUSRCLK_CTRL_REG;
	}
	else {
		RegOffsetCtrl = XVPHY_MMCM_RXUSRCLK_CTRL_REG;
	}

	/* Power down. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XVPHY_MMCM_USRCLK_CTRL_PWRDWN_MASK;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Hold) {
		/* Power up. */
		RegVal &= ~XVPHY_MMCM_USRCLK_CTRL_PWRDWN_MASK;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function will start the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_MmcmStart(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir)
{
	u32 Status;
	u8 Retry;

	if ((Dir == XVPHY_DIR_TX &&
			InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) ||
		(Dir == XVPHY_DIR_RX &&
			InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI)) {
		XVphy_HdmiMmcmStart(InstancePtr, QuadId, Dir);
		return;
	}

	/* Enable MMCM. */
	XVphy_MmcmPowerDown(InstancePtr, QuadId, Dir, FALSE);

	XVphy_WaitUs(InstancePtr, 10000);

	/* Toggle MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, FALSE);

	XVphy_WaitUs(InstancePtr, 10000);

	/* Configure MMCM. */
	Retry = 0;
	do {
		XVphy_WaitUs(InstancePtr, 10000);
		Status = XVphy_MmcmWriteParameters(InstancePtr, QuadId, Dir);
		Retry++;
	} while ((Status != XST_SUCCESS) && (Retry < 3));

	XVphy_WaitUs(InstancePtr, 10000);

	/* Toggle MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, FALSE);

	XVphy_LogWrite(InstancePtr, (Dir == XVPHY_DIR_TX) ?
		XVPHY_LOG_EVT_TXPLL_RECONFIG : XVPHY_LOG_EVT_RXPLL_RECONFIG, 1);
}

/*****************************************************************************/
/**
* This function will reset the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Enable is an indicator whether to "Enable" the locked mask
*		if set to 1. If set to 0: reset, then disable.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_MmcmLockedMaskEnable(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, u8 Enable)
{
	u32 RegOffsetCtrl;
	u32 RegVal;

	XVphy_SelQuad(InstancePtr, QuadId);

	if (Dir == XVPHY_DIR_TX) {
		RegOffsetCtrl = XVPHY_MMCM_TXUSRCLK_CTRL_REG;
	}
	else {
		RegOffsetCtrl = XVPHY_MMCM_RXUSRCLK_CTRL_REG;
	}

	/* Assert reset. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XVPHY_MMCM_USRCLK_CTRL_LOCKED_MASK_MASK;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Enable) {
		/* De-assert reset. */
		RegVal &= ~XVPHY_MMCM_USRCLK_CTRL_LOCKED_MASK_MASK;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function resets the BUFG_GT peripheral.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	Dir is an indicator for TX or RX
* @param	Reset specifies TRUE/FALSE value to either assert or deassert
*		reset on the BUFG_GT, respectively.
*
* @return	None.
*
******************************************************************************/
void XVphy_BufgGtReset(XVphy *InstancePtr, XVphy_DirectionType Dir, u8 Reset)
{
	u32 RegVal;
	u32 RegOffset;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_BUFGGT_TXUSRCLK_REG;
	}
	else {
		RegOffset = XVPHY_BUFGGT_RXUSRCLK_REG;
	}

	/* Read BUFG_GT register. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Write new value to BUFG_GT register. */
	if (Reset) {
		RegVal |= XVPHY_BUFGGT_XXUSRCLK_CLR_MASK;
	}
	else {
		RegVal &= ~XVPHY_BUFGGT_XXUSRCLK_CLR_MASK;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function obtains the divider value of the BUFG_GT peripheral.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	Dir is an indicator for TX or RX
* @param	Div 3-bit divider value
*
* @return	None.
*
******************************************************************************/
void XVphy_SetBufgGtDiv(XVphy *InstancePtr, XVphy_DirectionType Dir, u8 Div)
{
	u32 RegVal;
	u32 RegOffset;
	u8 Divider = Div;

	if (Divider == 0) {
		Divider = 1;
	}
	else {
		Divider = Divider - 1;
	}


	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_BUFGGT_TXUSRCLK_REG;
	}
	else {
		RegOffset = XVPHY_BUFGGT_RXUSRCLK_REG;
	}

	/* Read BUFG_GT register. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	RegVal &= ~XVPHY_BUFGGT_XXUSRCLK_DIV_MASK;

	/* Shift divider value to correct position. */
	Divider <<= XVPHY_BUFGGT_XXUSRCLK_DIV_SHIFT;
	Divider &= XVPHY_BUFGGT_XXUSRCLK_DIV_MASK;
	RegVal |= Divider;

	/* Write new value to BUFG_GT ctrl register. */
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function enables the TX or RX IBUFDS peripheral.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	Dir is an indicator for TX or RX.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the IBUFDS, respectively.
*
* @return	None.
*
******************************************************************************/
void XVphy_IBufDsEnable(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir,
		u8 Enable)
{
	XVphy_PllRefClkSelType *TypePtr, *DruTypePtr;
	u32 RegAddr = XVPHY_IBUFDS_GTXX_CTRL_REG;
	u32 RegVal;
	u32 MaskVal = 0;
	DruTypePtr = NULL;

	if (Dir == XVPHY_DIR_TX) {
		TypePtr = &InstancePtr->Config.TxRefClkSel;
	}
	else {
		TypePtr = &InstancePtr->Config.RxRefClkSel;
		if (InstancePtr->Config.DruIsPresent) {
			DruTypePtr = &InstancePtr->Config.DruRefClkSel;
		}
	}

	if ((*TypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0) ||
			(*DruTypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0)) {
		MaskVal = XVPHY_IBUFDS_GTXX_CTRL_GTREFCLK0_CEB_MASK;
	}
	else if ((*TypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1) ||
			(*DruTypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1)) {
		MaskVal = XVPHY_IBUFDS_GTXX_CTRL_GTREFCLK1_CEB_MASK;
	}
	else {
		if (Dir == XVPHY_DIR_TX) {
			RegAddr = XVPHY_MISC_TXUSRCLK_REG;
		}
		else {
			RegAddr = XVPHY_MISC_RXUSRCLK_REG;
		}
		MaskVal = XVPHY_MISC_XXUSRCLK_REFCLK_CEB_MASK;
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegAddr);

	if (Enable) {
		RegVal &= ~MaskVal;
	}
	else {
		RegVal |= MaskVal;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegAddr, RegVal);
}

/*****************************************************************************/
/**
* This function enables the TX or RX CLKOUT1 OBUFTDS peripheral.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	Dir is an indicator for TX or RX.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the OBUFTDS, respectively.
*
* @return	None.
*
******************************************************************************/
void XVphy_Clkout1OBufTdsEnable(XVphy *InstancePtr, XVphy_DirectionType Dir,
		u8 Enable)
{
	u32 RegVal;
	u32 RegOffset;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_MISC_TXUSRCLK_REG;
	}
	else {
		RegOffset = XVPHY_MISC_RXUSRCLK_REG;
	}

	/* Read XXUSRCLK MISC register. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Write new value to XXUSRCLK MISC register. */
	if (Enable) {
		RegVal |= XVPHY_MISC_XXUSRCLK_CKOUT1_OEN_MASK;
	}
	else {
		RegVal &= ~XVPHY_MISC_XXUSRCLK_CKOUT1_OEN_MASK;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will set 8b10b encoding for the specified GT PLL.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Enable is an indicator to enable/disable 8b10b encoding.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
void XVphy_Set8b10b(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Enable)
{
	u32 RegOffset;
	u32 MaskVal;
	u32 RegVal;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_TX_CONTROL_TX8B10BEN_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_TX_CONTROL_TX8B10BEN_MASK(ChId);
		}
	}
	else {
		RegOffset = XVPHY_RX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_RX_CONTROL_RX8B10BEN_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_RX_CONTROL_RX8B10BEN_MASK(ChId);
		}
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	if (Enable) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will power down the specified GT PLL.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to power down the PLL for.
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
u32 XVphy_PowerDownGtPll(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Hold)
{
	u32 MaskVal = 0;
	u32 RegVal;
	u8 Id, Id0, Id1;

	if (XVPHY_ISCH(ChId)) {
		XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	}
	else {
		/* When powering down a QPLL, power down for all channels. */
		XVphy_Ch2Ids(InstancePtr, XVPHY_CHANNEL_ID_CHA, &Id0, &Id1);
	}
	for (Id = Id0; Id <= Id1; Id++) {
		if (ChId == XVPHY_CHANNEL_ID_CMN0) {
			MaskVal |= XVPHY_POWERDOWN_CONTROL_QPLL0PD_MASK(Id);
		}
		else if (ChId == XVPHY_CHANNEL_ID_CMN1) {
			MaskVal |= XVPHY_POWERDOWN_CONTROL_QPLL1PD_MASK(Id);
		}
		else if (ChId == XVPHY_CHANNEL_ID_CMNA) {
			MaskVal |= XVPHY_POWERDOWN_CONTROL_QPLL0PD_MASK(Id) |
				   XVPHY_POWERDOWN_CONTROL_QPLL1PD_MASK(Id);
		}
		else {
			MaskVal |= XVPHY_POWERDOWN_CONTROL_CPLLPD_MASK(Id);
		}
	}

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
					XVPHY_POWERDOWN_CONTROL_REG);
	RegVal |= MaskVal;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
					XVPHY_POWERDOWN_CONTROL_REG, RegVal);

	if (!Hold) {
		RegVal &= ~MaskVal;
		XVphy_WriteReg(InstancePtr->Config.BaseAddr,
					XVPHY_POWERDOWN_CONTROL_REG, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function returns true when the RX and TX are bonded and are running
* from the same (RX) reference clock.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	TRUE if the RX and TX are using the same PLL, FALSE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_IsBonded(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	XVphy_SysClkDataSelType RxSysClkDataSel;
	XVphy_SysClkOutSelType RxSysClkOutSel;
	XVphy_SysClkDataSelType TxSysClkDataSel;
	XVphy_SysClkOutSelType TxSysClkOutSel;

	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		ChId = XVPHY_CHANNEL_ID_CH1;
	}

	RxSysClkDataSel = XVphy_GetSysClkDataSel(InstancePtr, QuadId,
							XVPHY_DIR_RX, ChId);
	RxSysClkOutSel = XVphy_GetSysClkOutSel(InstancePtr, QuadId,
							XVPHY_DIR_RX, ChId);
	TxSysClkDataSel = XVphy_GetSysClkDataSel(InstancePtr, QuadId,
							XVPHY_DIR_TX, ChId);
	TxSysClkOutSel = XVphy_GetSysClkOutSel(InstancePtr, QuadId,
							XVPHY_DIR_TX, ChId);

	if ((RxSysClkDataSel == TxSysClkDataSel) &&
					(RxSysClkOutSel == TxSysClkOutSel)) {
		return TRUE;
	}

	return FALSE;
}

/*****************************************************************************/
/**
* This function will try to find the necessary PLL divisor values to produce
* the configured line rate given the specified PLL input frequency. This will
* be done for all channels specified by ChId.
* This function is a wrapper for XVphy_PllCalculator.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to calculate the PLL values for.
* @param	ChId is the channel ID to calculate the PLL values for.
* @param	Dir is an indicator for TX or RX.
* @param	PllClkInFreqHz is the PLL input frequency on which to base the
*		calculations on. A value of 0 indicates to use the currently
*		configured quad PLL reference clock. A non-zero value indicates
*		to ignore what is currently configured in SW, and use a custom
*		frequency instead.
*
* @return
*		- XST_SUCCESS if valid PLL values were found to satisfy the
*		  constraints.
*		- XST_FAILURE otherwise.
*
* @note		If successful, the channel's PllParams structure will be
*		modified with the valid PLL parameters.
*
******************************************************************************/
u32 XVphy_ClkCalcParams(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u32 PllClkInFreqHz)
{
	u32 Status = XST_SUCCESS;
	u8 Id, Id0, Id1;

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		Status = XVphy_PllCalculator(InstancePtr, QuadId, Id, Dir,
				PllClkInFreqHz);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will set the current output divider configuration over DRP.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID for which to write the settings for.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_OutDivReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir)
{
	u32 Status;
	u8 Id;
	u8 Id0;
	u8 Id1;

	if (!XVPHY_ISCH(ChId)) {
		ChId = XVPHY_CHANNEL_ID_CHA;
	}

	XVphy_LogWrite(InstancePtr, (Dir == XVPHY_DIR_TX) ?
		XVPHY_LOG_EVT_GT_TX_RECONFIG : XVPHY_LOG_EVT_GT_RX_RECONFIG, 0);

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		Status = XVphy_OutDivChReconfig(InstancePtr, QuadId, Id, Dir);
		if (Status != XST_SUCCESS) {
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will set the current RX/TX configuration over DRP.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID for which to write the settings for.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_DirReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir)
{
	u32 Status = XST_SUCCESS;
	u8 Id, Id0, Id1;

	if ((InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE2) &&
			(Dir == XVPHY_DIR_TX)) {
		return XST_SUCCESS;
	}

    if ((InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) &&
		((InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_DP) ||
		 (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP))) {
               ChId = XVPHY_CHANNEL_ID_CHA;
    }

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		if (Dir == XVPHY_DIR_TX) {
			Status = XVphy_TxPllRefClkDiv1Reconfig(InstancePtr,
					QuadId, Id);
		}
		else {
			Status = XVphy_RxChReconfig(InstancePtr, QuadId, Id);
		}
		if (Status != XST_SUCCESS) {
			break;
		}
	}

	XVphy_LogWrite(InstancePtr, (Dir == XVPHY_DIR_TX) ?
		XVPHY_LOG_EVT_GT_TX_RECONFIG : XVPHY_LOG_EVT_GT_RX_RECONFIG, 1);

	return Status;
}

/*****************************************************************************/
/**
* This function will set the current clocking settings for each channel to
* hardware based on the configuration stored in the driver's instance.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID for which to write the settings for.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_ClkReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	u32 Status;
	u8 Id;
	u8 Id0;
	u8 Id1;

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		if (XVPHY_ISCH(Id)) {
			Status = XVphy_ClkChReconfig(InstancePtr, QuadId, Id);
		}
		else if (XVPHY_ISCMN(ChId)) {
			Status = XVphy_ClkCmnReconfig(InstancePtr, QuadId, Id);
		}
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	if (XVPHY_ISCH(Id)) {
		XVphy_LogWrite(InstancePtr, XVPHY_LOG_EVT_CPLL_RECONFIG, 1);
	}
	else if (XVPHY_ISCMN(ChId) &&
			(InstancePtr->Config.XcvrType != XVPHY_GT_TYPE_GTPE2)) {
		XVphy_LogWrite(InstancePtr, XVPHY_LOG_EVT_QPLL_RECONFIG, 1);
	}
	else if (XVPHY_ISCMN(ChId)) { /* GTPE2. */
		XVphy_LogWrite(InstancePtr, (ChId == XVPHY_CHANNEL_ID_CMN0) ?
			XVPHY_LOG_EVT_PLL0_RECONFIG :
			XVPHY_LOG_EVT_PLL1_RECONFIG, 1);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will set the channel IDs to correspond with the supplied
* channel ID based on the protocol. HDMI uses 3 channels; DP uses 4. This ID
* translation is done to allow other functions to operate iteratively over
* multiple channels.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	ChId is the channel ID used to determine the indices.
* @param	Id0 is a pointer to the start channel ID to set.
* @param	Id1 is a pointer to the end channel ID to set.
*
* @return	None.
*
* @note		The contents of Id0 and Id1 will be set according to ChId.
*
******************************************************************************/
void XVphy_Ch2Ids(XVphy *InstancePtr, XVphy_ChannelId ChId,
		u8 *Id0, u8 *Id1)
{
	u8 Channels = 4;

	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		*Id0 = XVPHY_CHANNEL_ID_CH1;
		if ((InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) ||
			(InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI)) {
			*Id1 = XVPHY_CHANNEL_ID_CH3;
		}
		else {
			Channels = ((InstancePtr->Config.TxChannels >=
							InstancePtr->Config.RxChannels) ?
									InstancePtr->Config.TxChannels :
									InstancePtr->Config.RxChannels);

			if (Channels == 1) {
				*Id1 = XVPHY_CHANNEL_ID_CH1;
			}
			else if (Channels == 2) {
				*Id1 = XVPHY_CHANNEL_ID_CH2;
			}
			else if (Channels == 3) {
				*Id1 = XVPHY_CHANNEL_ID_CH3;
			}
			else {
				*Id1 = XVPHY_CHANNEL_ID_CH4;
			}
		}
	}
	else if (ChId == XVPHY_CHANNEL_ID_CMNA) {
		*Id0 = XVPHY_CHANNEL_ID_CMN0;
		if ((InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE3) ||
		    (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE4)) {
			*Id1 = XVPHY_CHANNEL_ID_CMN1;
		}
		else {
			*Id1 = XVPHY_CHANNEL_ID_CMN0;
		}
	}
	else {
		*Id0 = *Id1 = ChId;
	}
}

/*****************************************************************************/
/**
* This function will set the Video PHY IP to operate on the specified GT quad.
* All Video PHY future accesses will operate on the specified quad until this
* a different quad is set.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XVphy_SelQuad(XVphy *InstancePtr, u8 QuadId)
{
	u32 RegVal;

	RegVal = (QuadId << XVPHY_BANK_SELECT_RX_SHIFT) | QuadId;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_BANK_SELECT_REG,
									RegVal);
}

/*****************************************************************************/
/**
* This function will write the mixed-mode clock manager (MMCM) values currently
* stored in the driver's instance structure to hardware .
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return
*		- XST_SUCCESS if the MMCM write was successful.
*		- XST_FAILURE otherwise, if the configuration success bit did
*		  not go low.
*
* @note		None.
*
******************************************************************************/
static u32 XVphy_MmcmWriteParameters(XVphy *InstancePtr, u8 QuadId,
							XVphy_DirectionType Dir)
{
	u32 RegOffsetCtrl;
	u32 RegOffsetClk;
	u32 RegVal;
	XVphy_Mmcm *MmcmParams;
	u8 Retry;

	XVphy_SelQuad(InstancePtr, QuadId);

	if (Dir == XVPHY_DIR_TX) {
		RegOffsetCtrl = XVPHY_MMCM_TXUSRCLK_CTRL_REG;
		RegOffsetClk = XVPHY_MMCM_TXUSRCLK_REG1;
	}
	else {
		RegOffsetCtrl = XVPHY_MMCM_RXUSRCLK_CTRL_REG;
		RegOffsetClk = XVPHY_MMCM_RXUSRCLK_REG1;
	}
	MmcmParams = &InstancePtr->Quads[QuadId].Mmcm[Dir];

	/* Check Parameters if has been Initialized */
	if (!MmcmParams->DivClkDivide && !MmcmParams->ClkFbOutMult &&
			!MmcmParams->ClkFbOutFrac && !MmcmParams->ClkOut0Frac &&
			!MmcmParams->ClkOut0Div && !MmcmParams->ClkOut1Div &&
			!MmcmParams->ClkOut2Div) {
		return XST_FAILURE;
	}

	/* MMCM_[TX|RX]USRCLK_REG1 */
	RegVal = MmcmParams->DivClkDivide;
	RegVal |= (MmcmParams->ClkFbOutMult <<
				XVPHY_MMCM_USRCLK_REG1_CLKFBOUT_MULT_SHIFT);
	RegVal |= (MmcmParams->ClkFbOutFrac <<
				XVPHY_MMCM_USRCLK_REG1_CLKFBOUT_FRAC_SHIFT);
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetClk, RegVal);

	/* MMCM_[TX|RX]USRCLK_REG2 */
	RegOffsetClk += 4;
	RegVal = MmcmParams->ClkOut0Div;
	RegVal |= (MmcmParams->ClkOut0Frac <<
				XVPHY_MMCM_USRCLK_REG2_CLKOUT0_FRAC_SHIFT);
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetClk, RegVal);

	/* MMCM_[TX|RX]USRCLK_REG3 */
	RegOffsetClk += 4;
	RegVal = MmcmParams->ClkOut1Div;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetClk, RegVal);

	/* MMCM_[TX|RX]USRCLK_REG4 */
	RegOffsetClk += 4;
	RegVal = MmcmParams->ClkOut2Div;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetClk, RegVal);

	/* Update the MMCM. */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XVPHY_MMCM_USRCLK_CTRL_CFG_NEW_MASK;
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	/* Wait until the MMCM indicates configuration has succeeded. */
	Retry = 0;
	do {
		XVphy_WaitUs(InstancePtr, 1000);
		RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
								RegOffsetCtrl);
		if (Retry > 15) {
			return XST_FAILURE;
		}
		Retry++;
	} while (!(RegVal & XVPHY_MMCM_USRCLK_CTRL_CFG_SUCCESS_MASK));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will translate from XVphy_PllType to XVphy_SysClkDataSelType.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	The reference clock type based on the PLL selection.
*
* @note		None.
*
******************************************************************************/
static inline XVphy_SysClkDataSelType Pll2SysClkData(XVphy_PllType PllSelect)
{
	return	(PllSelect == XVPHY_PLL_TYPE_CPLL) ?
			XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL) ?
			XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL0) ?
			XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL1) ?
			XVPHY_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK :
		(PllSelect == XVPHY_PLL_TYPE_PLL0) ?
			XVPHY_SYSCLKSELDATA_TYPE_PLL0_OUTCLK :
		XVPHY_SYSCLKSELDATA_TYPE_PLL1_OUTCLK;
}

/*****************************************************************************/
/**
* This function will translate from XVphy_PllType to XVphy_SysClkOutSelType.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
*
* @return	The reference clock type based on the PLL selection.
*
* @note		None.
*
******************************************************************************/
static inline XVphy_SysClkOutSelType Pll2SysClkOut(XVphy_PllType PllSelect)
{
	return	(PllSelect == XVPHY_PLL_TYPE_CPLL) ?
			XVPHY_SYSCLKSELOUT_TYPE_CPLL_REFCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL) ?
			XVPHY_SYSCLKSELOUT_TYPE_QPLL_REFCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL0) ?
			XVPHY_SYSCLKSELOUT_TYPE_QPLL0_REFCLK :
		(PllSelect == XVPHY_PLL_TYPE_QPLL1) ?
			XVPHY_SYSCLKSELOUT_TYPE_QPLL1_REFCLK :
		(PllSelect == XVPHY_PLL_TYPE_PLL0) ?
			XVPHY_SYSCLKSELOUT_TYPE_PLL0_REFCLK :
		XVPHY_SYSCLKSELOUT_TYPE_PLL1_REFCLK;
}

/*****************************************************************************/
/**
* This function will initiate a DRP transaction (either read or write).
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
* @note		In read mode (Dir == XVPHY_DIR_RX), the data pointed to by Val
*		will be populated with the u16 value that was read._
*
******************************************************************************/
static u32 XVphy_DrpAccess(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u16 Addr, u16 *Val)
{
	u32 RegOffsetCtrl;
	u32 RegOffsetSts;
	u32 RegVal;
	u8 Retry;

	XVphy_SelQuad(InstancePtr, QuadId);

	/* Determine which DRP registers to use based on channel. */
	if (XVPHY_ISCMN(ChId)) {
		RegOffsetCtrl = XVPHY_DRP_CONTROL_COMMON_REG;
		RegOffsetSts = XVPHY_DRP_STATUS_COMMON_REG;
	}
	else {
		RegOffsetCtrl = XVPHY_DRP_CONTROL_CH1_REG +
			(4 * XVPHY_CH2IDX(ChId));
		RegOffsetSts = XVPHY_DRP_STATUS_CH1_REG +
			(4 * (XVPHY_CH2IDX(ChId)));
	}

    if ((InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) &&
		((InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_DP) ||
		 (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP))) {
               ChId = XVPHY_CHANNEL_ID_CHA;
		XVphy_WaitUs(InstancePtr, 3000);
	}

	/* Wait until the DRP status indicates that it is not busy.*/
	Retry = 0;
	do {
		RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
								RegOffsetSts);
		if (Retry > 150) {
			return XST_FAILURE;
		}
		Retry++;
	} while (RegVal & XVPHY_DRP_STATUS_DRPBUSY_MASK);

	/* Write the command to the channel's DRP. */
	RegVal = (Addr & XVPHY_DRP_CONTROL_DRPADDR_MASK);
	RegVal |= XVPHY_DRP_CONTROL_DRPEN_MASK;
	if (Dir == XVPHY_DIR_TX) {
		/* Enable write. */
		RegVal |= XVPHY_DRP_CONTROL_DRPWE_MASK;
		RegVal |= ((*Val << XVPHY_DRP_CONTROL_DRPDI_SHIFT) &
						XVPHY_DRP_CONTROL_DRPDI_MASK);
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	/* Wait until the DRP status indicates ready.*/
	Retry = 0;
	do {
		RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
								RegOffsetSts);
		if (Retry > 150) {
			return XST_FAILURE;
		}
		Retry++;
	} while (!(RegVal & XVPHY_DRP_STATUS_DRPRDY_MASK));

	if (Dir == XVPHY_DIR_RX) {
		/* Mask non-data out for read. */
		RegVal &= XVPHY_DRP_STATUS_DRPO_MASK;
		/* Populate Val with read contents. */
		*Val = RegVal;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will try to find the necessary PLL divisor values to produce
* the configured line rate given the specified PLL input frequency.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to calculate the PLL values for.
* @param	ChId is the channel ID to calculate the PLL values for.
* @param	Dir is an indicator for TX or RX.
* @param	PllClkInFreqHz is the PLL input frequency on which to base the
*		calculations on. A value of 0 indicates to use the currently
*		configured quad PLL reference clock. A non-zero value indicates
*		to ignore what is currently configured in SW, and use a custom
*		frequency instead.
*
* @return
*		- XST_SUCCESS if valid PLL values were found to satisfy the
*		  constraints.
*		- XST_FAILURE otherwise.
*
* @note		If successful, the channel's PllParams structure will be
*		modified with the valid PLL parameters.
*
******************************************************************************/
static u32 XVphy_PllCalculator(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir,
		u32 PllClkInFreqHz)
{
	u32 Status;
	u64 PllClkOutFreqHz;
	u64 CalcLineRateFreqHz;
	u8 Id, Id0, Id1;
	u64 PllClkInFreqHzIn = PllClkInFreqHz;
	XVphy_Channel *PllPtr = &InstancePtr->Quads[QuadId].
		Plls[XVPHY_CH2IDX(ChId)];

	if (!PllClkInFreqHzIn) {
		PllClkInFreqHzIn = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
					PllPtr->PllRefClkSel);
	}

	/* Select PLL value table offsets. */
	const XVphy_GtPllDivs *GtPllDivs;
	if (XVPHY_ISCH(ChId)) {
		GtPllDivs = &InstancePtr->GtAdaptor->CpllDivs;
	}
	else {
		GtPllDivs = &InstancePtr->GtAdaptor->QpllDivs;
	}

	const u8 *M, *N1, *N2, *D;
	for (N2 = GtPllDivs->N2; *N2 != 0; N2++) {
	for (N1 = GtPllDivs->N1; *N1 != 0; N1++) {
	for (M = GtPllDivs->M;   *M != 0;  M++) {
		PllClkOutFreqHz = (PllClkInFreqHzIn * *N1 * *N2) / *M;

		/* Test if the calculated PLL clock is in the VCO range. */
		Status = XVphy_CheckPllOpRange(InstancePtr, QuadId, ChId,
				PllClkOutFreqHz);
		if (Status != XST_SUCCESS) {
			continue;
		}

		if ((InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) ||
				(XVPHY_ISCH(ChId))) {
			PllClkOutFreqHz *= 2;
		}
		/* Apply TX/RX divisor. */
		for (D = GtPllDivs->D; *D != 0; D++) {
			CalcLineRateFreqHz = PllClkOutFreqHz / *D;
			if (CalcLineRateFreqHz == PllPtr->LineRateHz) {
				goto calc_done;
			}
		}
	}
	}
	}
	/* Calculation failed, don't change divisor settings. */
	return XST_FAILURE;

calc_done:
	/* Found the multiplier and divisor values for requested line rate. */
	PllPtr->PllParams.MRefClkDiv = *M;
	PllPtr->PllParams.NFbDiv = *N1;
	PllPtr->PllParams.N2FbDiv = *N2; /* Won't be used for QPLL.*/
	PllPtr->PllParams.IsLowerBand = 1; /* Won't be used for CPLL. */

	if (XVPHY_ISCMN(ChId)) {
		/* Same divisor value for all channels if using a QPLL. */
		ChId = XVPHY_CHANNEL_ID_CHA;
	}

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(Id)].OutDiv[Dir] =
			*D;
		if (Dir == XVPHY_DIR_RX) {
			XVphy_CfgSetCdr(InstancePtr, QuadId, Id);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the default in HDF.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_SetDefaultPpc(XVphy *InstancePtr, u8 QuadId)
{
	extern XVphy_Config XVphy_ConfigTable[XPAR_XVPHY_NUM_INSTANCES];

	InstancePtr->Config.Ppc = XVphy_ConfigTable[QuadId].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Ppc is the PPC to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_SetPpc(XVphy *InstancePtr, u8 QuadId, u8 Ppc)
{
	InstancePtr->Config.Ppc = Ppc;
}

/*****************************************************************************/
/**
* This function calculates the PLL VCO operating frequency.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	PLL VCO frequency in Hz
*
* @note		None.
*
******************************************************************************/
u64 XVphy_GetPllVcoFreqHz(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir)
{
	u64 PllxVcoRateHz;
	u64 PllRefClkHz;
	XVphy_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XVPHY_CH2IDX(ChId)];

	if (Dir == XVPHY_DIR_TX) {
		if (InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_HDMI) {
			PllRefClkHz = InstancePtr->HdmiTxRefClkHz;
		}
		else {
			PllRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
									PllPtr->PllRefClkSel);
		}
	}
	else {
		if (InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_HDMI) {
			if (InstancePtr->HdmiRxDruIsEnabled) {
				PllRefClkHz = XVphy_DruGetRefClkFreqHz(InstancePtr);
			}
			else {
				PllRefClkHz = InstancePtr->HdmiRxRefClkHz;
			}
		}
		else {
			PllRefClkHz = XVphy_GetQuadRefClkFreq(InstancePtr, QuadId,
									PllPtr->PllRefClkSel);
		}
	}

	PllxVcoRateHz = (PllRefClkHz *
				InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
								PllParams.N1FbDiv *
				InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
								PllParams.N2FbDiv) /
				InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].
								PllParams.MRefClkDiv;

	return PllxVcoRateHz;
}
