/*******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
 *                     Changed EffectiveAddr datatype in XVphy_CfgInitialize
 *                       to UINTPTR
 *                     Used usleep API instead of MB_Sleep API
 *                     Fixed Null pointer dereference in XVphy_IBufDsEnable
 *                     Suppressed warning messages due to unused arguments
 * 1.4   gm   29/11/16 Moved internally used APIs to xvphy_i.c/h
 *                     Added preprocessor directives for sw footprint reduction
 *                     Fixed c++ compiler warnings
 * 1.6   gm   12/06/17 Changed FAILURE return value of XVphy_DrpRead to 0xDEAD
 *                     Added XVphy_DrpRd, XVphy_SetErrorCallback,
 *                        XVphy_SetPllLayoutErrorCallback and
 *                        XVphy_RegisterDebug APIs
 *                     Added filter in XVphy_MmcmStart to prevent MMCM from
 *                        starting when divider values are invalid
 * 1.7   gm   13/09/17 Added GTYE4 support
 *                     Updated XVphy_MmcmStart to be able to support a system
 *                        with active HDMI and DP
 *                     Added xvphy_mmcme2/3/4.c drivers to move MMCM
 *                        configuration from RTL to SW
 *                     Moved XVphy_MmcmWriteParameters to xvphy_mmcme2/3/4.c
 *                     Added XVphy_SetPolarity, XVphy_SetPrbsSel and
 *                        XVphy_TxPrbsForceError APIs
 * 1.8   gm   05/14/18 Removed XVphy_DrpWrite and XVphy_DrpRead APIs
 *            23/07/18 Added APIs XVphy_SetTxVoltageSwing and
 *                       XVphy_SetTxPreEmphasis from xvphy_i.c/h
 *                     Added XVphy_SetTxPostCursor API
 * 1.9   gm   14/05/18 Added XVphy_SetRxLpm from xvphy_i.c/.h
 * 1.10  ssh  16/08/22 Added support for multi gt (GTHE4 and GTYE4)
 * 1.11  ssh  04/02/24 Added support for DP2.1
 *
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/
#include "xparameters.h"
#include "xstatus.h"
#include "xvphy.h"
#include "xvphy_i.h"
#include "xvphy_hdmi.h"
#include "sleep.h"
#include "xvphy_gt.h"

/**************************** Function Prototypes *****************************/
static u32 XVphy_DrpAccess(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u16 Addr, u16 *Val);

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
		UINTPTR EffectiveAddr)
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

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	InstancePtr->GtAdaptor = &Gtxe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE2)
	InstancePtr->GtAdaptor = &Gthe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
	InstancePtr->GtAdaptor = &Gtpe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE3)
	InstancePtr->GtAdaptor = &Gthe3Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE4) || (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4)
	if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE4) {
		InstancePtr->GtAdaptor = &Gthe4Config;
	} else if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTYE4) {
		InstancePtr->GtAdaptor = &Gtye4Config;
	}
#endif

	const XVphy_SysClkDataSelType SysClkCfg[7][2] = {
		{(XVphy_SysClkDataSelType)0, XVPHY_SYSCLKSELDATA_TYPE_CPLL_OUTCLK},
		{(XVphy_SysClkDataSelType)1, XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
		{(XVphy_SysClkDataSelType)2, XVPHY_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK},
		{(XVphy_SysClkDataSelType)3, XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK},
		{(XVphy_SysClkDataSelType)4, XVPHY_SYSCLKSELDATA_TYPE_PLL0_OUTCLK},
		{(XVphy_SysClkDataSelType)5, XVPHY_SYSCLKSELDATA_TYPE_PLL1_OUTCLK},
		{(XVphy_SysClkDataSelType)6, XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK},
	};
	for (Sel = 0; Sel < 7; Sel++) {
		if (InstancePtr->Config.TxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.TxSysPllClkSel = SysClkCfg[Sel][1];
		}
		if (InstancePtr->Config.RxSysPllClkSel == SysClkCfg[Sel][0]) {
			InstancePtr->Config.RxSysPllClkSel = SysClkCfg[Sel][1];
		}
	}

	InstancePtr->Config.TxRefClkSel = (XVphy_PllRefClkSelType)
			(InstancePtr->Config.TxRefClkSel +
					XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.RxRefClkSel = (XVphy_PllRefClkSelType)
			(InstancePtr->Config.RxRefClkSel +
					XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0);
	InstancePtr->Config.DruRefClkSel = (XVphy_PllRefClkSelType)
			(InstancePtr->Config.DruRefClkSel +
					XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0);

	/* Correct RefClkSel offsets for GTPE2 EAST and WEST RefClks */
	if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTPE2) {
		if (InstancePtr->Config.TxRefClkSel > 6) {
			InstancePtr->Config.TxRefClkSel = (XVphy_PllRefClkSelType)
					(InstancePtr->Config.TxRefClkSel - 4);
		}
		if (InstancePtr->Config.RxRefClkSel > 6) {
			InstancePtr->Config.RxRefClkSel = (XVphy_PllRefClkSelType)
					(InstancePtr->Config.RxRefClkSel - 4);
		}
		if (InstancePtr->Config.DruRefClkSel > 6) {
			InstancePtr->Config.DruRefClkSel = (XVphy_PllRefClkSelType)
					(InstancePtr->Config.DruRefClkSel - 4);
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
	/* Suppress Warning Messages */
	ChId = ChId;

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

#if defined (XPAR_XDP_NUM_INSTANCES)
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
#endif

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

#if defined (XPAR_XDP_NUM_INSTANCES)
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
#endif

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
	/* For GTHE2, GTHE3, GTHE4, GTYE4, and GTXE2. */
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

#if defined (XPAR_XDP_NUM_INSTANCES)
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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
#endif

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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
    usleep (1000);
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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
* This function will set/clear the TX/RX polarity bit.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_SetPolarity(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Polarity)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal =  XVPHY_TX_CONTROL_TXPOLARITY_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_TX_CONTROL_TXPOLARITY_MASK(ChId);
		}
	}
	else {
		RegOffset = XVPHY_RX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_RX_CONTROL_RXPOLARITY_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_RX_CONTROL_RXPOLARITY_MASK(ChId);
		}
	}

	/* Read TX|RX Control Register */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Clear Polarity Register bits */
	RegVal &= ~MaskVal;

	if (Polarity) {
		RegVal |= MaskVal;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the TX/RXPRBSEL of the GT
*
* @param	InstancePtr is a pointer to the XVphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID which to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Pattern is the pattern XVphy_PrbsPattern
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XVphy_SetPrbsSel(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, XVphy_PrbsPattern Pattern)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;
	u32 PrbsEnc = 0x0;
	u8 Id, Id0, Id1;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XVPHY_DIR_TX) {
		RegOffset = XVPHY_TX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal =  XVPHY_TX_CONTROL_TXPRBSSEL_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_TX_CONTROL_TXPRBSSEL_MASK(ChId);
		}
		PrbsEnc = (u32) (((Pattern & 0x8) << 1) | (Pattern & 0x7));
	}
	else {
		RegOffset = XVPHY_RX_CONTROL_REG;
		if (ChId == XVPHY_CHANNEL_ID_CHA) {
			MaskVal = XVPHY_RX_CONTROL_RXPRBSSEL_ALL_MASK;
		}
		else {
			MaskVal = XVPHY_RX_CONTROL_RXPRBSSEL_MASK(ChId);
		}
		PrbsEnc = (u32) (Pattern & 0xF);
	}

	/* Read TX|RX Control Register */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	/* Mask out PRBS Register bits */
	RegVal &= ~MaskVal;

	XVphy_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		RegVal |= PrbsEnc << ((Dir == XVPHY_DIR_TX) ?
					XVPHY_TX_CONTROL_TXPRBSSEL_SHIFT(Id) :
					XVPHY_RX_CONTROL_RXPRBSSEL_SHIFT(Id));
	}

	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will set the TX/RXPRBSEL of the GT
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_TxPrbsForceError(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 ForceErr)
{
	u32 RegVal;
	u32 MaskVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (ChId == XVPHY_CHANNEL_ID_CHA) {
		MaskVal =  XVPHY_TX_CONTROL_TXPRBSFORCEERR_ALL_MASK;
	}
	else {
		MaskVal = XVPHY_TX_CONTROL_TXPRBSFORCEERR_MASK(ChId);
	}

	/* Read TX|RX Control Register */
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
					XVPHY_TX_CONTROL_REG);

	/* Clear Polarity Register bits */
	RegVal &= ~MaskVal;

	if (ForceErr) {
		RegVal |= MaskVal;
	}
	XVphy_WriteReg(InstancePtr->Config.BaseAddr,
					XVPHY_TX_CONTROL_REG, RegVal);

	return XST_SUCCESS;
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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
void XVphy_SetTxPreEmphasis(XVphy *InstancePtr, u8 QuadId,
        XVphy_ChannelId ChId, u8 Pe)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
* This function will set the TX post-curosr value for a given channel.
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
void XVphy_SetTxPostCursor(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Pc)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if ((ChId == XVPHY_CHANNEL_ID_CH1) || (ChId == XVPHY_CHANNEL_ID_CH2)) {
		RegOffset = XVPHY_TX_DRIVER_CH12_REG;
	}
	else {
		RegOffset = XVPHY_TX_DRIVER_CH34_REG;
	}
	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);

	MaskVal = XVPHY_TX_DRIVER_TXPOSTCURSOR_MASK(ChId);
	RegVal &= ~MaskVal;
	RegVal |= (Pc << XVPHY_TX_DRIVER_TXPOSTCURSOR_SHIFT(ChId));
	XVphy_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
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

	/* Suppress Warning Messages */
	QuadId = QuadId;
	Dir = Dir;

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
* This function will initiate a write DRP transaction. It is a wrapper around
* XVphy_DrpAccess.
*
* @param	InstancePtr is a pointer to the XVphy core instance.
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
u32 XVphy_DrpWr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
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
u16 XVphy_DrpRd(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
        u16 Addr, u16 *RetVal)
{
	u32 Status;
	u16 Val;

	Status = XVphy_DrpAccess(InstancePtr, QuadId, ChId,
			XVPHY_DIR_RX, /* Read. */
			Addr, &Val);

    *RetVal = Val;

	return Status;
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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
#if defined (XPAR_XDP_NUM_INSTANCES)
#if defined (XPAR_XV_HDMITX_0_DEVICE_ID) || defined (XPAR_XV_HDMIRX_0_DEVICE_ID) || \
	     defined (XPAR_XV_HDMITX_0_BASEADDR) || defined (XPAR_XV_HDMIRX_0_BASEADDR)

	if (InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_DP ||
		InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP) {
#endif
	/* Enable MMCM. */
	XVphy_MmcmPowerDown(InstancePtr, QuadId, Dir, FALSE);

	XVphy_WaitUs(InstancePtr, 10000);

	/* Toggle MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, TRUE);

	/* Configure MMCM. */
	XVphy_MmcmWriteParameters(InstancePtr, QuadId, Dir);

	/* Toggle MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, FALSE);
#endif
#if defined (XPAR_XV_HDMITX_0_DEVICE_ID) || defined (XPAR_XV_HDMIRX_0_DEVICE_ID) || \
	     defined (XPAR_XV_HDMITX_0_BASEADDR) || defined (XPAR_XV_HDMIRX_0_BASEADDR)
#if defined (XPAR_XDP_NUM_INSTANCES)
	} else if (XVphy_IsHDMI(InstancePtr, XVPHY_DIR_TX) ||
			   XVphy_IsHDMI(InstancePtr, XVPHY_DIR_RX)) {
#endif
	XVphy_Mmcm *MmcmPtr;

	if (Dir == XVPHY_DIR_RX) {
		MmcmPtr= &InstancePtr->Quads[QuadId].RxMmcm;
	}
	else {
		MmcmPtr= &InstancePtr->Quads[QuadId].TxMmcm;
	}

	/* Check values if valid */
	if (!((MmcmPtr->ClkOut0Div > 0) && (MmcmPtr->ClkOut0Div <= 128) &&
		  (MmcmPtr->ClkOut1Div > 0) && (MmcmPtr->ClkOut1Div <= 128) &&
		  (MmcmPtr->ClkOut2Div > 0) && (MmcmPtr->ClkOut2Div <= 128))) {
		return;
	}

	/* Assert MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, TRUE);

	/* Configure MMCM. */
	XVphy_MmcmWriteParameters(InstancePtr, QuadId, Dir);

	/* Release MMCM reset. */
	XVphy_MmcmReset(InstancePtr, QuadId, Dir, FALSE);

	/* Unmask the MMCM Lock */
	XVphy_MmcmLockedMaskEnable(InstancePtr, 0, Dir, FALSE);
#if defined (XPAR_XDP_NUM_INSTANCES)
	}
#endif
#endif

	XVphy_LogWrite(InstancePtr, (Dir == XVPHY_DIR_TX) ?
		XVPHY_LOG_EVT_TXPLL_RECONFIG : XVPHY_LOG_EVT_RXPLL_RECONFIG, 1);
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
	XVphy_PllRefClkSelType *TypePtr, *DruTypePtr, DruTypeDummy;
	u32 RegAddr = XVPHY_IBUFDS_GTXX_CTRL_REG;
	u32 RegVal;
	u32 MaskVal = 0;
	DruTypeDummy = XVPHY_PLL_REFCLKSEL_TYPE_GTGREFCLK;
	DruTypePtr = &DruTypeDummy;

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
			((InstancePtr->Config.DruIsPresent) &&
			(*DruTypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0))) {
		MaskVal = XVPHY_IBUFDS_GTXX_CTRL_GTREFCLK0_CEB_MASK;
	}
	else if ((*TypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1) ||
			((InstancePtr->Config.DruIsPresent) &&
			(*DruTypePtr == XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1))) {
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

#if defined (XPAR_XDP_NUM_INSTANCES)
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

	/* Suppress Warning Messages */
	QuadId = QuadId;

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
* This function sets up the PHY for DP2.1
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
void XVphy_SetupDP21Phy (XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Rate, XVphy_PllRefClkSelType RefClkSel,
		XVphy_PllType PllSelect)
{
	u32 RegVal;
	u32 Status = XST_SUCCESS;

	RegVal = XVphy_ReadReg(InstancePtr->Config.BaseAddr,
			XVPHY_CLKDET_CTRL_REG);

    if ((Rate == 0x1E) ||
		  (Rate == 0x14) ||
		  (Rate == 0x0A) ||
		  (Rate == 0x06) ) {
          //write '1' to [bit 31] of x200; DP1.4 select /40 from MMCM
	if (!Dir) {
          XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_CLKDET_CTRL_REG,
			  RegVal | 0x80000000);
          XVphy_SetRxLpm(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
			1);
	} else {
            XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_CLKDET_CTRL_REG,
			  RegVal | 0x40000000);
	}
      } else {
          //write '0' to bit[31] x200; DP2.0 select /32 from GT
	if (!Dir) {
          XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_CLKDET_CTRL_REG,
			  RegVal & 0x7FFFFFFF);
          XVphy_SetRxLpm(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
			0);
	} else {
		if (Rate == 0x02) {
			for (ChId = 1; ChId <= 4; ChId++) {
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_ADAPT_CFG1, 0xFB1C);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL1_CFG_0, 0x4040);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL1_CFG_1, 0x1040);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL2_CFG_0, 0x4040);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL2_CFG_1, 0x0040);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXDRV_FREQBAND, 0x8285);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG1, 0xAA00);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG2, 0xAA00);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG3, 0xAA00);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPHDLY_CFG1, 0xE);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPI_CFG0, 0x3000);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPI_CFG1, 0x0000);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TX_PI_BIASSET, 0x224);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CH_HSPMUX, 0x6060);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXOUT_DIV, 0x1E8);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_RTX_BUF_CML_CTRL, 0x6);
				Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TX_CLK25_DIV, 0xA806);
			}
		} else {
			for (ChId = 1; ChId <= 4; ChId++) {
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_ADAPT_CFG1, 0xF81C);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL1_CFG_0, 0xC0C0);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL1_CFG_1, 0x10C0);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL2_CFG_0, 0xC0C0);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CKCAL2_CFG_1, 0x80C0);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXDRV_FREQBAND, 0x8085);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG1, 0x6C00);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG2, 0x6C00);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXFE_CFG3, 0x6C00);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPHDLY_CFG1, 0xF);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPI_CFG0, 0x100);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXPI_CFG1, 0x1000);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TX_PI_BIASSET, 0x222);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_CH_HSPMUX, 0x4020);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TXOUT_DIV, 0x3E8);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_RTX_BUF_CML_CTRL, 0x4);
			Status |= XVphy_DrpWr(InstancePtr, QuadId, ChId, XVPHY_GTYE4_CHANNEL_TX_CLK25_DIV, 0x8806);
		}
        }
            XVphy_WriteReg(InstancePtr->Config.BaseAddr, XVPHY_CLKDET_CTRL_REG,
			  RegVal & 0xBFFFFFFF);
	}
      }

	if (!Dir) {
		InstancePtr->Quads[QuadId].RxMmcm.dp20rate = Rate;
        XVphy_MmcmWriteParameters (InstancePtr, 0, XVPHY_DIR_RX);
        XVphy_MmcmReset (InstancePtr, 0, XVPHY_DIR_RX, TRUE);
        if (PllSelect == XVPHY_PLL_TYPE_CPLL) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CHA, RefClkSel);
        } else if (PllSelect == XVPHY_PLL_TYPE_QPLL0) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMN0, RefClkSel);
        } else if (PllSelect == XVPHY_PLL_TYPE_QPLL1) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMN1, RefClkSel);
        }
	} else {
		InstancePtr->Quads[QuadId].TxMmcm.dp20rate = Rate;
		XVphy_MmcmWriteParameters (InstancePtr, 0, XVPHY_DIR_TX);
		XVphy_MmcmReset (InstancePtr, 0, XVPHY_DIR_TX, TRUE);
        if (PllSelect == XVPHY_PLL_TYPE_CPLL) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CHA, RefClkSel);
        } else if (PllSelect == XVPHY_PLL_TYPE_QPLL0) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMNA, RefClkSel);
        } else if (PllSelect == XVPHY_PLL_TYPE_QPLL1) {
		XVphy_CfgPllRefClkSel(InstancePtr, QuadId,
				XVPHY_CHANNEL_ID_CMN1, RefClkSel);
        }
	}
	XVphy_WriteCfgRefClkSelReg(InstancePtr, QuadId);

	//forcing CPLL for RX, QPLL for TX as of now for 10G
	if (!Dir) {
		if (PllSelect == XVPHY_PLL_TYPE_CPLL) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX);
		} else if (PllSelect == XVPHY_PLL_TYPE_QPLL0) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN0, XVPHY_DIR_RX);
		} else if (PllSelect == XVPHY_PLL_TYPE_QPLL1) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1, XVPHY_DIR_RX);
		}
	} else {
		if (PllSelect == XVPHY_PLL_TYPE_CPLL) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX);
		} else if (PllSelect == XVPHY_PLL_TYPE_QPLL0) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN0, XVPHY_DIR_TX);
		} else if (PllSelect == XVPHY_PLL_TYPE_QPLL1) {
			XVphy_ClkInitialize(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1, XVPHY_DIR_TX);
		}
	}
}


u16 XVphy_DP21PhyReset (XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir)
{
	u32 Status;
	u32 Retry;

	u8 PllSelect = XVphy_GetPllType (InstancePtr, QuadId, Dir, ChId);

	if (!Dir) {
	XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_RX,(TRUE));
	if (InstancePtr->Config.DpRxProtocol == 0) {
		XVphy_ResetGtPll(InstancePtr, 0, ChId,
				XVPHY_DIR_RX,(TRUE));
		XVphy_ResetGtPll(InstancePtr, 0, ChId,
				XVPHY_DIR_RX, (FALSE));
	} else if (InstancePtr->Config.DpRxProtocol == 1) {
		XVphy_ResetGtPll(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DIR_RX,(TRUE));
		XVphy_ResetGtPll(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DIR_RX, (FALSE));
	}
	XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_RX, (FALSE));
	Status = XVphy_WaitForResetDone(InstancePtr, 0, ChId,
			XVPHY_DIR_RX);
	Status |= XVphy_WaitForPllLock(InstancePtr, 0, ChId);

	XVphy_MmcmReset (InstancePtr, 0, XVPHY_DIR_RX, FALSE);
	Retry = 0;
	while (!XVphy_MmcmLocked(InstancePtr, 0, XVPHY_DIR_RX)) {
		Retry++;
		if (Retry > 2000) {
			Status = XST_FAILURE;
			break;
		}
	}
	} else {
		if (InstancePtr->Config.DpTxProtocol == 0) {
			XVphy_WriteReg(InstancePtr->Config.BaseAddr,
					XVPHY_PLL_RESET_REG,
					(XVPHY_PLL_RESET_QPLL0_MASK |
							XVPHY_PLL_RESET_QPLL1_MASK)); // 0x06
		} else if  (InstancePtr->Config.DpTxProtocol == 1) {
			if (PllSelect == XVPHY_PLL_TYPE_CPLL) {
				XVphy_WriteReg(InstancePtr->Config.BaseAddr,
						XVPHY_PLL_RESET_REG,
						XVPHY_PLL_RESET_CPLL_MASK); // 0x06
			} else if (PllSelect == XVPHY_PLL_TYPE_QPLL0) {
				XVphy_WriteReg(InstancePtr->Config.BaseAddr,
						XVPHY_PLL_RESET_REG,
						XVPHY_PLL_RESET_QPLL0_MASK); // 0x06
			} else if (PllSelect == XVPHY_PLL_TYPE_QPLL1) {
				XVphy_WriteReg(InstancePtr->Config.BaseAddr,
						XVPHY_PLL_RESET_REG,
						XVPHY_PLL_RESET_QPLL1_MASK); // 0x06
			}
		}
		XVphy_WriteReg(InstancePtr->Config.BaseAddr,
				XVPHY_PLL_RESET_REG, 0x0);

//		XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_TX,(TRUE));
		XVphy_ResetGtPll(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DIR_TX,(TRUE));
		XVphy_ResetGtPll(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				XVPHY_DIR_TX, (FALSE));
//		XVphy_BufgGtReset(InstancePtr, XVPHY_DIR_TX, (FALSE));
		Status = XVphy_WaitForResetDone(InstancePtr, 0, ChId,
				XVPHY_DIR_TX);
		Status |= XVphy_WaitForPllLock(InstancePtr, 0, ChId);

		XVphy_MmcmReset (InstancePtr, 0, XVPHY_DIR_TX, FALSE);
		Retry = 0;
		while (!XVphy_MmcmLocked(InstancePtr, 0, XVPHY_DIR_TX)) {
			Retry++;
			if (Retry > 2000) {
				Status |= XST_FAILURE;
				break;
			}
		}

//		XVphy_WriteReg(InstancePtr->Config.BaseAddr,
//				XVPHY_PLL_RESET_REG,
//				(XVPHY_PLL_RESET_QPLL0_MASK |
//				 XVPHY_PLL_RESET_QPLL1_MASK)); // 0x06
//		XVphy_WriteReg(InstancePtr->Config.BaseAddr,
//				XVPHY_PLL_RESET_REG, 0x0);
//
//		XVphy_ResetGtPll(InstancePtr, QuadId,
//				XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));
//
//		Status = XVphy_WaitForPmaResetDone(InstancePtr, 0,
//				ChId, XVPHY_DIR_TX);
//
//		Status |= XVphy_WaitForPllLock(InstancePtr, 0, ChId);
//
//		Status |= XVphy_WaitForResetDone(InstancePtr, 0,
//				ChId, XVPHY_DIR_TX);
//
//		XVphy_MmcmReset (InstancePtr, 0, XVPHY_DIR_TX, FALSE);
//		Retry = 0;
//		while (!XVphy_MmcmLocked(InstancePtr, 0, XVPHY_DIR_TX)) {
//			Retry++;
//			if (Retry > 2000) {
//				Status = XST_FAILURE;
//				break;
//			}
//		}

	}
	return Status;
}

#endif

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

	/* Suppress Warning Messages */
	QuadId = QuadId;

	/* Determine which DRP registers to use based on channel. */
	if (XVPHY_ISCMN(ChId)) {
		RegOffsetCtrl = XVPHY_DRP_CONTROL_COMMON_REG;
		RegOffsetSts = XVPHY_DRP_STATUS_COMMON_REG;
	}
	else if (XVPHY_ISTXMMCM(ChId)) {
		RegOffsetCtrl = XVPHY_DRP_CONTROL_TXMMCM_REG;
		RegOffsetSts = XVPHY_DRP_STATUS_TXMMCM_REG;
	}
	else if (XVPHY_ISRXMMCM(ChId)) {
		RegOffsetCtrl = XVPHY_DRP_CONTROL_RXMMCM_REG;
		RegOffsetSts = XVPHY_DRP_STATUS_RXMMCM_REG;
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

/******************************************************************************/
/**
* This function installs a callback function for the VPHY error conditions
*
* @param	InstancePtr is a pointer to the XVPhy instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note		The XVphy_ErrorHandler API calls the registered function in
* 			  ErrorCallback and passes two arguments: 1) CallbackRef
* 			  2) Error Type as defined by XVphy_ErrType.
*
* 			Sample Function Call:
* 				CallbackFunc(CallbackRef, XVphy_ErrType);
*
*******************************************************************************/
void XVphy_SetErrorCallback(XVphy *InstancePtr,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->ErrorCallback = (XVphy_ErrorCallback)CallbackFunc;
	InstancePtr->ErrorRef = CallbackRef;
}

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
/******************************************************************************/
/**
* This function installs a callback function for the VPHY PLL Layout error
* conditions
*
* @param	InstancePtr is a pointer to the XVPhy instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item that will be passed to the
*		callback function when it is invoked.
*
* @return	None.
*
* @note		The XVphy_ErrorHandler API calls the registered function in
* 			  ErrorCallback and passes two arguments: 1) CallbackRef
* 			  2) Error Type as defined by XVphy_ErrType.
*
* 			Sample Function Call:
* 				CallbackFunc(CallbackRef, XVphy_ErrType);
*
*******************************************************************************/
void XVphy_SetPllLayoutErrorCallback(XVphy *InstancePtr,
		void *CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->PllLayoutErrorCallback = (XVphy_ErrorCallback)CallbackFunc;
	InstancePtr->PllLayoutErrorRef = CallbackRef;
}
#endif

/*****************************************************************************/
/**
* This function prints out Video PHY register and GT Channel and Common
* DRP register contents.
*
* @param	InstancePtr is a pointer to the Vphy core instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_RegisterDebug(XVphy *InstancePtr)
{
	u32 RegOffset;
	u16 DrpAddr, MaxDrpAddr;
	u16 DrpVal, ChId;
	u8  MaxChannels;

	xil_printf("\r\nVPHY Registers\r\n");
	xil_printf("-----------------\r\n");
	xil_printf("Offset   |  Value\r\n");
	xil_printf("-----------------\r\n");
	for (RegOffset = 0; RegOffset <= 0x334; ) {
		xil_printf("0x%04x      0x%08x\r\n",RegOffset,
		XVphy_ReadReg(InstancePtr->Config.BaseAddr, RegOffset));
		RegOffset += 4;
	}

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	MaxDrpAddr = 0x0044;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE2)
	MaxDrpAddr = 0x0047;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
	MaxDrpAddr = 0x002D;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE3)
	MaxDrpAddr = 0x00B0;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE4) || (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4)
	MaxDrpAddr = 0x00B0;
#endif

	xil_printf("\r\nVPHY GT COMMON DRP Registers\r\n");
	xil_printf("----------------------------\r\n");
	if ((InstancePtr->HdmiIsQpllPresent == TRUE) ||
			(InstancePtr->Config.TxProtocol == XVPHY_PROTOCOL_DP) ||
			(InstancePtr->Config.RxProtocol == XVPHY_PROTOCOL_DP)) {
		xil_printf("Offset   |  Value\r\n");
		xil_printf("----------------------------\r\n");
		for (DrpAddr = 0x0000; DrpAddr <= MaxDrpAddr; DrpAddr++) {
			XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN0,
					DrpAddr, &DrpVal);
			xil_printf("0x%04x      0x%04x\r\n",DrpAddr, DrpVal);
		}
	} else {
		xil_printf("No QPLL in this VPHY Instance\r\n");
	}

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
	MaxDrpAddr = 0x015C;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE2)
	MaxDrpAddr = 0x015E;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
	MaxDrpAddr = 0x00AD;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE3)
	MaxDrpAddr = 0x015F;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE4) || (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4)
	if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTHE4) {
		MaxDrpAddr = 0x025F;
	} else if (InstancePtr->Config.XcvrType == XVPHY_GT_TYPE_GTYE4) {
		MaxDrpAddr = 0x028C;
	}
#endif
	/* Get Max number of channels in VPHY */
	MaxChannels = (InstancePtr->Config.RxChannels >
					InstancePtr->Config.TxChannels) ?
					InstancePtr->Config.RxChannels :
					InstancePtr->Config.TxChannels;

	for (ChId = 1; ChId <= MaxChannels; ChId++) {
	xil_printf("\r\nVPHY GT CHANNEL %d DRP Registers\r\n", ChId);
	xil_printf("-------------------------------\r\n");
	xil_printf("Offset   |  Value\r\n");
	xil_printf("-------------------------------\r\n");
		for (DrpAddr = 0x0000; DrpAddr <= MaxDrpAddr; DrpAddr++) {
			XVphy_DrpRd(InstancePtr, 0, ChId, DrpAddr, &DrpVal);
			xil_printf("0x%04x      0x%04x\r\n",DrpAddr, DrpVal);
		}
	}
}
