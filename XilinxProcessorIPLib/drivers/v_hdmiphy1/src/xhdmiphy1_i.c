/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_i.c
 *
 * Contains generic APIs that are locally called or used within the
 * HDMIPHY driver.
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
 * 1.1   ku   17/05/20 Adding uniquification to avoid clash with vphy
 * 1.1   ku   23/05/20 Corrected XHdmiphy1_Ch2Ids to set correct value
 *                     for Id1
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


/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will set the channel IDs to correspond with the supplied
* channel ID based on the protocol. HDMI uses 3 channels; This ID
* translation is done to allow other functions to operate iteratively over
* multiple channels.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId is the channel ID used to determine the indices.
* @param	Id0 is a pointer to the start channel ID to set.
* @param	Id1 is a pointer to the end channel ID to set.
*
* @return	None.
*
* @note		The contents of Id0 and Id1 will be set according to ChId.
*
******************************************************************************/
void XHdmiphy1_Ch2Ids(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
		u8 *Id0, u8 *Id1)
{
	u8 Channels = 4;

	if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		*Id0 = XHDMIPHY1_CHANNEL_ID_CH1;
		if ((XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) ||
			(XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX))) {
			if ((InstancePtr->Config.TxProtocol == XHDMIPHY1_PROTOCOL_HDMI21) ||
					(InstancePtr->Config.RxProtocol == XHDMIPHY1_PROTOCOL_HDMI21)) {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH4;
			}
			else if ((InstancePtr->Config.TxProtocol == XHDMIPHY1_PROTOCOL_HDMI) &&
					(InstancePtr->Config.UseGtAsTxTmdsClk == TRUE)){
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH4;
			}
			else {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH3;
			}
		}
		else {
			Channels = ((InstancePtr->Config.TxChannels >=
				InstancePtr->Config.RxChannels) ?
				InstancePtr->Config.TxChannels :
				InstancePtr->Config.RxChannels);

			if (Channels == 1) {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH1;
			}
			else if (Channels == 2) {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH2;
			}
			else if (Channels == 3) {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH3;
			}
			else {
				*Id1 = XHDMIPHY1_CHANNEL_ID_CH4;
			}
		}
	}
	else if (ChId == XHDMIPHY1_CHANNEL_ID_CMNA) {
		*Id0 = XHDMIPHY1_CHANNEL_ID_CMN0;
		if ((InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) ||
		    (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4)) {
			*Id1 = XHDMIPHY1_CHANNEL_ID_CMN1;
		}
		else {
			*Id1 = XHDMIPHY1_CHANNEL_ID_CMN0;
		}
	}
	else {
		*Id0 = *Id1 = ChId;
	}
}

/*****************************************************************************/
/**
* This function will set the current RX/TX configuration over DRP.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_DirReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status = XST_SUCCESS;
	u8 Id, Id0, Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		if (Dir == XHDMIPHY1_DIR_TX) {
			Status |= XHdmiphy1_TxChReconfig(InstancePtr, QuadId,
						(XHdmiphy1_ChannelId)Id);
		}
		else {
			Status |= XHdmiphy1_RxChReconfig(InstancePtr, QuadId,
						(XHdmiphy1_ChannelId)Id);
		}
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			break;
		}
	}

	XHdmiphy1_LogWrite(InstancePtr, (Dir == XHDMIPHY1_DIR_TX) ?
	XHDMIPHY1_LOG_EVT_GT_TX_RECONFIG : XHDMIPHY1_LOG_EVT_GT_RX_RECONFIG, 1);

	return Status;
}


#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
/*****************************************************************************/
/**
* This function writes the current software configuration for the reference
* clock selections to hardware for the specified quad on all channels.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
*
* @return
*		- XST_SUCCESS.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_WriteCfgRefClkSelReg(XHdmiphy1 *InstancePtr, u8 QuadId)
{
	u32 RegVal = 0;
	XHdmiphy1_Channel *ChPtr;
	XHdmiphy1_GtType GtType = InstancePtr->Config.XcvrType;

	/* Point to the first channel since settings apply to all channels. */
	ChPtr = &InstancePtr->Quads[QuadId].Ch1;

	/* PllRefClkSel. */
	/* - QPLL0. */
	RegVal &= ~XHDMIPHY1_REF_CLK_SEL_QPLL0_MASK;
	RegVal = InstancePtr->Quads[QuadId].Cmn0.PllRefClkSel;
	/* - CPLL. */
	RegVal &= ~XHDMIPHY1_REF_CLK_SEL_CPLL_MASK;
	RegVal |= (ChPtr->CpllRefClkSel << XHDMIPHY1_REF_CLK_SEL_CPLL_SHIFT);
	if ((GtType == XHDMIPHY1_GT_TYPE_GTHE4) ||
            (GtType == XHDMIPHY1_GT_TYPE_GTYE4)) {
		/* - QPLL1. */
		RegVal &= ~XHDMIPHY1_REF_CLK_SEL_QPLL1_MASK;
		RegVal |= (InstancePtr->Quads[QuadId].Cmn1.PllRefClkSel <<
				XHDMIPHY1_REF_CLK_SEL_QPLL1_SHIFT);
	}

	/* SysClkDataSel. PLLCLKSEL */
	RegVal &= ~XHDMIPHY1_REF_CLK_SEL_SYSCLKSEL_MASK;
	/* - TXSYSCLKSEL[0]. TXPLLCLKSEL*/
	RegVal |= (ChPtr->TxDataRefClkSel <<
		XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_SHIFT(GtType)) &
		XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_MASK(GtType);
	/* - RXSYSCLKSEL[0]. RXPLLCLKSEL*/
	RegVal |= (ChPtr->RxDataRefClkSel <<
		XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_SHIFT(GtType)) &
		XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_MASK(GtType);

	/* SysClkOutSel. */
	/* - TXSYSCLKSEL[1]. */
	RegVal |= (ChPtr->TxOutRefClkSel <<
		XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_SHIFT(GtType)) &
		XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_MASK(GtType);
	/* - RXSYSCLKSEL[1]. */
	RegVal |= (ChPtr->RxOutRefClkSel <<
		XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_SHIFT(GtType)) &
		XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_MASK(GtType);

	/* Write to hardware. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
			 XHDMIPHY1_REF_CLK_SEL_REG,RegVal);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Configure the PLL reference clock selection for the specified channel(s).
* This is applied to both direction to the software configuration only.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	SysClkDataSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_CfgPllRefClkSel(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_PllRefClkSelType RefClkSel)
{
	u8 Id, Id0, Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].PllRefClkSel =
					RefClkSel;
	}
}

/*****************************************************************************/
/**
* Configure the SYSCLKDATA reference clock selection for the direction. Same
* configuration applies to all channels in the quad. This is applied to the
* software configuration only.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	SysClkDataSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_CfgSysClkDataSel(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir,
		XHdmiphy1_SysClkDataSelType SysClkDataSel)
{
	XHdmiphy1_Channel *ChPtr;
	u8 Id, Id0, Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	/* Select in software - same for all channels. */
	for (Id = Id0; Id <= Id1; Id++) {
		ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)];
		ChPtr->DataRefClkSel[Dir] = SysClkDataSel;
	}
}

/*****************************************************************************/
/**
* Configure the SYSCLKOUT reference clock selection for the direction. Same
* configuration applies to all channels in the quad. This is applied to the
* software configuration only.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	SysClkOutSel is the reference clock selection to configure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_CfgSysClkOutSel(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir,
		XHdmiphy1_SysClkOutSelType SysClkOutSel)
{
	XHdmiphy1_Channel *ChPtr;
	u8 Id, Id0, Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	/* Select in software - same for all channels. */
	for (Id = Id0; Id <= Id1; Id++) {
		ChPtr = &InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)];
		ChPtr->OutRefClkSel[Dir] = SysClkOutSel;
	}
}
#endif

/*****************************************************************************/
/**
* Obtain the reconfiguration channel ID for given PLL type
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	PllType is the PLL type being used by the channel.
*
* @return	The Channel ID to be used for reconfiguration
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_ChannelId XHdmiphy1_GetRcfgChId(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, XHdmiphy1_PllType PllType)
{
	XHdmiphy1_ChannelId ChId;

	/* Suppress Warning Messages */
	InstancePtr = InstancePtr;
	QuadId = QuadId;
	Dir = Dir;

	/* Determine which channel(s) to operate on. */
	switch (PllType) {
	case XHDMIPHY1_PLL_TYPE_QPLL:
	case XHDMIPHY1_PLL_TYPE_QPLL0:
	case XHDMIPHY1_PLL_TYPE_LCPLL:
		ChId = XHDMIPHY1_CHANNEL_ID_CMN0;
		break;
	case XHDMIPHY1_PLL_TYPE_QPLL1:
	case XHDMIPHY1_PLL_TYPE_RPLL:
		ChId = XHDMIPHY1_CHANNEL_ID_CMN1;
		break;
	default:
		ChId = XHDMIPHY1_CHANNEL_ID_CHA;
		break;
	}

	return ChId;
}

/*****************************************************************************/
/**
* This function will check the status of a PLL lock on the specified channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_IsPllLocked(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u32 RegVal;
	u32 MaskVal;
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_PllType TxPllType;
	XHdmiphy1_PllType RxPllType;
#endif

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (ChId == XHDMIPHY1_CHANNEL_ID_CMN0) {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_QPLL0_MASK;
#else
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_LCPLL_MASK;
#endif
	}
	else if (ChId == XHDMIPHY1_CHANNEL_ID_CMN1) {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_QPLL1_MASK;
#else
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_RPLL_MASK;
	}
	else {
		/* This will result to XST_FAILURE */
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_CPLL_ALL_MASK;
#endif
	}
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	else if (ChId == XHDMIPHY1_CHANNEL_ID_CMNA) {
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_QPLL0_MASK |
			  XHDMIPHY1_PLL_LOCK_STATUS_QPLL1_MASK;
	}
	else if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
		TxPllType = XHdmiphy1_GetPllType(InstancePtr, 0,
				XHDMIPHY1_DIR_TX, XHDMIPHY1_CHANNEL_ID_CH1);
		RxPllType = XHdmiphy1_GetPllType(InstancePtr, 0,
				XHDMIPHY1_DIR_RX, XHDMIPHY1_CHANNEL_ID_CH1);
		if (RxPllType == XHDMIPHY1_PLL_TYPE_CPLL &&
			XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
			MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_CPLL_HDMI_MASK;
		}
		else if (TxPllType == XHDMIPHY1_PLL_TYPE_CPLL &&
			XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
			MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_CPLL_HDMI_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_CPLL_ALL_MASK;
		}
	}
	else {
		MaskVal = XHDMIPHY1_PLL_LOCK_STATUS_CPLL_MASK(ChId);
	}
#endif
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_PLL_LOCK_STATUS_REG);

	if ((RegVal & MaskVal) == MaskVal) {
		return XST_SUCCESS;
	}

	return XST_FAILURE;
}


#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
/*****************************************************************************/
/**
* Obtain the current reference clock frequency for the quad based on the
* reference clock type.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	RefClkType is the type to obtain the clock selection for.
*
* @return	The current reference clock frequency for the quad for the
*		specified type selection.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_GetQuadRefClkFreq(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_PllRefClkSelType RefClkType)
{
	u32 FreqHz;

	u8 RefClkIndex = RefClkType - XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTREFCLK0;

	FreqHz = (RefClkType > XHDMIPHY1_PLL_REFCLKSEL_TYPE_GTGREFCLK) ? 0 :
		InstancePtr->Quads[QuadId].RefClkHz[RefClkIndex];

	return FreqHz;
}

/*****************************************************************************/
/**
* Obtain the current [RT]XSYSCLKSEL[0] configuration.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The current [RT]XSYSCLKSEL[0] selection.
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_SysClkDataSelType XHdmiphy1_GetSysClkDataSel(XHdmiphy1 *InstancePtr,
        u8 QuadId, XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId)
{
	u32 Sel;
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;
	ChId = ChId;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
						XHDMIPHY1_REF_CLK_SEL_REG);

	if (Dir == XHDMIPHY1_DIR_TX) {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_DATA_SHIFT(
				InstancePtr->Config.XcvrType);
	}
	else {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_DATA_SHIFT(
				InstancePtr->Config.XcvrType);
	}

	return (XHdmiphy1_SysClkDataSelType) Sel;
}

/*****************************************************************************/
/**
* Obtain the current [RT]XSYSCLKSEL[1] configuration.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	ChId is the channel ID which to operate on.
*
* @return	The current [RT]XSYSCLKSEL[1] selection.
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_SysClkOutSelType XHdmiphy1_GetSysClkOutSel(XHdmiphy1 *InstancePtr,
        u8 QuadId, XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId)
{
	u32 Sel;
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;
	ChId = ChId;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_REF_CLK_SEL_REG);

	if (Dir == XHDMIPHY1_DIR_TX) {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XHDMIPHY1_REF_CLK_SEL_TXSYSCLKSEL_OUT_SHIFT(
				InstancePtr->Config.XcvrType);
	}
	else {
		/* Synchronize software configuration to hardware. */
		Sel = RegVal & XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_MASK(
				InstancePtr->Config.XcvrType);
		Sel >>= XHDMIPHY1_REF_CLK_SEL_RXSYSCLKSEL_OUT_SHIFT(
				InstancePtr->Config.XcvrType);
	}

	return (XHdmiphy1_SysClkOutSelType)Sel;
}

/*****************************************************************************/
/**
* This function will reset and enable the Video PHY's user core logic.
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
u32 XHdmiphy1_GtUserRdyEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold)
{
	u32 RegVal;
	u32 MaskVal;
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_TX_INIT_REG;

		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal = XHDMIPHY1_TX_INIT_USERRDY_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_TX_INIT_USERRDY_MASK(ChId);
		}
	}
	else {
		RegOffset = XHDMIPHY1_RX_INIT_REG;
		if (ChId == XHDMIPHY1_CHANNEL_ID_CHA) {
			MaskVal = XHDMIPHY1_RX_INIT_USERRDY_ALL_MASK;
		}
		else {
			MaskVal = XHDMIPHY1_RX_INIT_USERRDY_MASK(ChId);
		}
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	/* Assert reset. */
	RegVal |= MaskVal;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~MaskVal;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
					RegOffset, RegVal);
	}

	return XST_SUCCESS;
}
#else
/*****************************************************************************/
/**
* This function will set the TXRATE or RXRATE port to select the GT Wizard
* configuration
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetGtLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 RegVal;
	u32 MaskVal;
	u32 ShiftVal;
	u32 RegOffset;
	u16 LRCfgVal;
	XHdmiphy1_PllType PllType;

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, Dir,
    			XHDMIPHY1_CHANNEL_ID_CH1);

    /* Extract Line Rate Config Value */
    if (PllType == XHDMIPHY1_PLL_TYPE_LCPLL) {
    	LRCfgVal = InstancePtr->Quads[QuadId].Lcpll.LineRateCfg;
    }
    else { /* RPLL */
    	LRCfgVal = InstancePtr->Quads[QuadId].Rpll.LineRateCfg;
    }

	if (Dir == XHDMIPHY1_DIR_TX) {
		if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
			(ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
			RegOffset = XHDMIPHY1_TX_RATE_CH12_REG;
		}
		else {
			RegOffset = XHDMIPHY1_TX_RATE_CH34_REG;
		}
		MaskVal = XHDMIPHY1_TX_RATE_MASK(ChId);
		ShiftVal = XHDMIPHY1_TX_RATE_SHIFT(ChId);
	}
	else{
		if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
			(ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
			RegOffset = XHDMIPHY1_RX_RATE_CH12_REG;
		}
		else {
			RegOffset = XHDMIPHY1_RX_RATE_CH34_REG;
		}
		MaskVal = XHDMIPHY1_RX_RATE_MASK(ChId);
		ShiftVal = XHDMIPHY1_RX_RATE_SHIFT(ChId);
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	RegVal &= ~MaskVal;
	RegVal |= (LRCfgVal << ShiftVal);
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will get the TXRATE or RXRATE port to select the GT Wizard
* configuration
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u16 XHdmiphy1_GetGtLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 RegVal;
	u32 MaskVal;
	u32 ShiftVal;
	u32 RegOffset;
	u16 LRCfgVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
			(ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
			RegOffset = XHDMIPHY1_TX_RATE_CH12_REG;
		}
		else {
			RegOffset = XHDMIPHY1_TX_RATE_CH34_REG;
		}
		MaskVal = XHDMIPHY1_TX_RATE_MASK(ChId);
		ShiftVal = XHDMIPHY1_TX_RATE_SHIFT(ChId);
	}
	else{
		if ((ChId == XHDMIPHY1_CHANNEL_ID_CH1) ||
			(ChId == XHDMIPHY1_CHANNEL_ID_CH2)) {
			RegOffset = XHDMIPHY1_RX_RATE_CH12_REG;
		}
		else {
			RegOffset = XHDMIPHY1_RX_RATE_CH34_REG;
		}
		MaskVal = XHDMIPHY1_RX_RATE_MASK(ChId);
		ShiftVal = XHDMIPHY1_RX_RATE_SHIFT(ChId);
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	RegVal &= MaskVal;
	LRCfgVal = (u16)(RegVal >> ShiftVal);

	return LRCfgVal;
}

/*****************************************************************************/
/**
* This function will set the GPI ports to the GT Wizard
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Set Set=TRUE; Clear=FALSE
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_SetGpi(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Set)
{
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Suppress Warning Messages */
	QuadId = QuadId;

    /* Read GPI Register*/
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_GT_DBG_GPI_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		if (Dir == XHDMIPHY1_DIR_TX) {
			MaskVal |=  XHDMIPHY1_TX_GPI_MASK(Id);
		}
		else {
			MaskVal |=  XHDMIPHY1_RX_GPI_MASK(Id);
		}
	}

	/* Construct Register Value */
	if (Set) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}

	/* Write GPI Register*/
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_GT_DBG_GPI_REG, RegVal);
}

/*****************************************************************************/
/**
* This function will get the GPI ports value from the GT Wizard
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	Value.
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_GetGpo(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;
	ChId = ChId;

    /* Read GPI Register*/
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_GT_DBG_GPO_REG);

	if (Dir == XHDMIPHY1_DIR_TX) {
		return ((RegVal &
					XHDMIPHY1_TX_GPO_MASK_ALL(InstancePtr->Config.TxChannels))
						>> XHDMIPHY1_TX_GPO_SHIFT);
	}
	else {
		return ((RegVal &
					XHDMIPHY1_RX_GPO_MASK_ALL(InstancePtr->Config.RxChannels))
						>> XHDMIPHY1_RX_GPO_SHIFT);
	}
}

/*****************************************************************************/
/**
* This function will set the (TX|RX) MSTRESET port of the GT
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Reset Set=TRUE; Clear=FALSE
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_GtMstReset(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Reset)
{
	u32 RegOffsetCtrl;
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		MaskVal |=  XHDMIPHY1_TXRX_MSTRESET_MASK(Id);
	}

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffsetCtrl = XHDMIPHY1_TX_INIT_REG;
	}
	else {
		RegOffsetCtrl = XHDMIPHY1_RX_INIT_REG;
	}

	/* Read TX|RX INIT Register*/
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				RegOffsetCtrl);

	/* Construct Register Value */
	if (Reset) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}

	/* Write TX|RX INIT Register*/
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
			RegOffsetCtrl, RegVal);
}

/*****************************************************************************/
/**
* This function will will check the current CFG setting and compare
* it with the next CFG value
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Reset Set=TRUE; Clear=FALSE
*
* @return	TRUE if Current and Next CFG are the same
*           FALSE if Current and Next CFG are different
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_CheckLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u16 CurrCfgVal, LRCfgVal;
	XHdmiphy1_PllType PllType;

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, Dir,
			XHDMIPHY1_CHANNEL_ID_CH1);

    /* Extract Line Rate Config Value */
    if (PllType == XHDMIPHY1_PLL_TYPE_LCPLL) {
	LRCfgVal = InstancePtr->Quads[QuadId].Lcpll.LineRateCfg;
    }
    else { /* RPLL */
	LRCfgVal = InstancePtr->Quads[QuadId].Rpll.LineRateCfg;
    }

    /* Get current line rate configuration value */
	CurrCfgVal = XHdmiphy1_GetGtLineRateCfg(InstancePtr, QuadId, ChId, Dir);

	if (CurrCfgVal != LRCfgVal) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}
#endif

/*****************************************************************************/
/**
* This function will reset the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
void XHdmiphy1_MmcmReset(XHdmiphy1 *InstancePtr, u8 QuadId,
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

	/* Assert reset. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XHDMIPHY1_MMCM_USRCLK_CTRL_RST_MASK;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Hold) {
		/* De-assert reset. */
		RegVal &= ~XHDMIPHY1_MMCM_USRCLK_CTRL_RST_MASK;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function will reset the mixed-mode clock manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
void XHdmiphy1_MmcmLockedMaskEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, u8 Enable)
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

	/* Assert reset. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);
	RegVal |= XHDMIPHY1_MMCM_USRCLK_CTRL_LOCKED_MASK_MASK;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl, RegVal);

	if (!Enable) {
		/* De-assert reset. */
		RegVal &= ~XHDMIPHY1_MMCM_USRCLK_CTRL_LOCKED_MASK_MASK;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl,
									RegVal);
	}
}

/*****************************************************************************/
/**
* This function will get the lock status of the mixed-mode clock
* manager (MMCM) core.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.

*
* @return	TRUE if Locked else FALSE.
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_MmcmLocked(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir)
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

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			RegOffsetCtrl) & XHDMIPHY1_MMCM_USRCLK_CTRL_LOCKED_MASK;

	return (RegVal ? TRUE : FALSE);

}

/*****************************************************************************/
/**
* This function will set the CLKINSEL port of the MMCM
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
* @param	Sel CLKINSEL value
* 				0 - CLKIN1
* 				1 - CLKIN2
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_MmcmSetClkinsel(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, XHdmiphy1_MmcmClkinsel Sel)
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

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffsetCtrl);

	if (Sel == XHDMIPHY1_MMCM_CLKINSEL_CLKIN2) {
		RegVal &= ~XHDMIPHY1_MMCM_USRCLK_CTRL_CLKINSEL_MASK;
	}
	else {
		RegVal |= XHDMIPHY1_MMCM_USRCLK_CTRL_CLKINSEL_MASK;
	}

	 XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
				RegOffsetCtrl, RegVal);

}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
/*****************************************************************************/
/**
* This function obtains the divider value of the BUFG_GT peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Dir is an indicator for TX or RX
* @param	Div 3-bit divider value
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_SetBufgGtDiv(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir, u8 Div)
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


	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_BUFGGT_TXUSRCLK_REG;
	}
	else {
		RegOffset = XHDMIPHY1_BUFGGT_RXUSRCLK_REG;
	}

	/* Read BUFG_GT register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
	RegVal &= ~XHDMIPHY1_BUFGGT_XXUSRCLK_DIV_MASK;

	/* Shift divider value to correct position. */
	Divider <<= XHDMIPHY1_BUFGGT_XXUSRCLK_DIV_SHIFT;
	Divider &= XHDMIPHY1_BUFGGT_XXUSRCLK_DIV_MASK;
	RegVal |= Divider;

	/* Write new value to BUFG_GT ctrl register. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/*****************************************************************************/
/**
* This function will power down the specified GT PLL.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_PowerDownGtPll(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Hold)
{
	u32 MaskVal = 0;
	u32 RegVal;
	u8 Id, Id0, Id1;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (XHDMIPHY1_ISCH(ChId)) {
		XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	}
	else {
		/* When powering down a QPLL, power down for all channels. */
		XHdmiphy1_Ch2Ids(InstancePtr,
				XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	}
	for (Id = Id0; Id <= Id1; Id++) {
		if (ChId == XHDMIPHY1_CHANNEL_ID_CMN0) {
			MaskVal |= XHDMIPHY1_POWERDOWN_CONTROL_QPLL0PD_MASK(Id);
		}
		else if (ChId == XHDMIPHY1_CHANNEL_ID_CMN1) {
			MaskVal |= XHDMIPHY1_POWERDOWN_CONTROL_QPLL1PD_MASK(Id);
		}
		else if (ChId == XHDMIPHY1_CHANNEL_ID_CMNA) {
			MaskVal |= XHDMIPHY1_POWERDOWN_CONTROL_QPLL0PD_MASK(Id) |
				   XHDMIPHY1_POWERDOWN_CONTROL_QPLL1PD_MASK(Id);
		}
		else {
			MaskVal |= XHDMIPHY1_POWERDOWN_CONTROL_CPLLPD_MASK(Id);
		}
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_POWERDOWN_CONTROL_REG);
	RegVal |= MaskVal;
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_POWERDOWN_CONTROL_REG, RegVal);

	if (!Hold) {
		RegVal &= ~MaskVal;
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_POWERDOWN_CONTROL_REG, RegVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will try to find the necessary PLL divisor values to produce
* the configured line rate given the specified PLL input frequency. This will
* be done for all channels specified by ChId.
* This function is a wrapper for XHdmiphy1_PllCalculator.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_ClkCalcParams(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u32 PllClkInFreqHz)
{
	u32 Status = XST_SUCCESS;
	u8 Id, Id0, Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		Status = XHdmiphy1_PllCalculator(InstancePtr, QuadId,
				(XHdmiphy1_ChannelId)Id, Dir, PllClkInFreqHz);
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
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_OutDivReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status;
	u8 Id;
	u8 Id0;
	u8 Id1;

	if (!XHDMIPHY1_ISCH(ChId)) {
		ChId = XHDMIPHY1_CHANNEL_ID_CHA;
	}

	XHdmiphy1_LogWrite(InstancePtr, (Dir == XHDMIPHY1_DIR_TX) ?
	XHDMIPHY1_LOG_EVT_GT_TX_RECONFIG : XHDMIPHY1_LOG_EVT_GT_RX_RECONFIG, 0);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		Status = XHdmiphy1_OutDivChReconfig(InstancePtr, QuadId,
					(XHdmiphy1_ChannelId)Id, Dir);
		if (Status != XST_SUCCESS) {
			break;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will set the current clocking settings for each channel to
* hardware based on the configuration stored in the driver's instance.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_ClkReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u32 Status = XST_SUCCESS;
	u8 Id;
	u8 Id0;
	u8 Id1;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		if (XHDMIPHY1_ISCH(Id)) {
			Status |= XHdmiphy1_ClkChReconfig(InstancePtr, QuadId,
						(XHdmiphy1_ChannelId)Id);
		}
		else if (XHDMIPHY1_ISCMN(ChId)) {
			if (((XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) ||
				(XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX))) &&
			    (InstancePtr->HdmiIsQpllPresent == FALSE)) {
				XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_NO_QPLL_ERR,
                    1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				Status = XST_FAILURE;
				return Status;
			}
			Status |= XHdmiphy1_ClkCmnReconfig(InstancePtr, QuadId,
						(XHdmiphy1_ChannelId)Id);
		}
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			return Status;
		}
	}

	if (XHDMIPHY1_ISCH(ChId)) {
		XHdmiphy1_LogWrite(InstancePtr,
			XHDMIPHY1_LOG_EVT_CPLL_RECONFIG, 1);
	}
	else if (XHDMIPHY1_ISCMN(ChId)) {
		XHdmiphy1_LogWrite(InstancePtr,
			XHDMIPHY1_LOG_EVT_QPLL_RECONFIG, 1);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function will translate from XHdmiphy1_PllType to
* XHdmiphy1_SysClkDataSelType.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	The reference clock type based on the PLL selection.
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_SysClkDataSelType XHdmiphy1_Pll2SysClkData(XHdmiphy1_PllType PllSelect)
{
	return	(PllSelect == XHDMIPHY1_PLL_TYPE_CPLL) ?
			XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK :
		(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL) ?
			XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK :
		(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL0) ?
			XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK :
		/*(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL1) ?*/
			XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK;
}

/*****************************************************************************/
/**
* This function will translate from XHdmiphy1_PllType to
* XHdmiphy1_SysClkOutSelType.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	The reference clock type based on the PLL selection.
*
* @note		None.
*
******************************************************************************/
XHdmiphy1_SysClkOutSelType XHdmiphy1_Pll2SysClkOut(XHdmiphy1_PllType PllSelect)
{
	return	(PllSelect == XHDMIPHY1_PLL_TYPE_CPLL) ?
			XHDMIPHY1_SYSCLKSELOUT_TYPE_CPLL_REFCLK :
		(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL) ?
			XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL_REFCLK :
		(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL0) ?
			XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL0_REFCLK :
		/*(PllSelect == XHDMIPHY1_PLL_TYPE_QPLL1) ?*/
			XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL1_REFCLK;
}

/*****************************************************************************/
/**
* This function will try to find the necessary PLL divisor values to produce
* the configured line rate given the specified PLL input frequency.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
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
u32 XHdmiphy1_PllCalculator(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
		u32 PllClkInFreqHz)
{
	u32 Status;
	u64 PllClkOutFreqHz;
	u64 CalcLineRateFreqHz;
	u8 Id, Id0, Id1;
	u64 PllClkInFreqHzIn = PllClkInFreqHz;
	XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
		Plls[XHDMIPHY1_CH2IDX(ChId)];

	if (!PllClkInFreqHzIn) {
		PllClkInFreqHzIn = XHdmiphy1_GetQuadRefClkFreq(InstancePtr,
					QuadId,
					PllPtr->PllRefClkSel);
	}

	/* Select PLL value table offsets. */
	const XHdmiphy1_GtPllDivs *GtPllDivs;
	if (XHDMIPHY1_ISCH(ChId)) {
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
		Status = XHdmiphy1_CheckPllOpRange(InstancePtr, QuadId, ChId,
				PllClkOutFreqHz);
		if (Status != XST_SUCCESS) {
			continue;
		}

		if ((XHDMIPHY1_ISCH(ChId))) {
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

	if (XHDMIPHY1_ISCMN(ChId)) {
		/* Same divisor value for all channels if using a QPLL. */
		ChId = XHDMIPHY1_CHANNEL_ID_CHA;
	}

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].OutDiv[Dir] =
			*D;
		if (Dir == XHDMIPHY1_DIR_RX) {
			XHdmiphy1_CfgSetCdr(InstancePtr,\
				QuadId, (XHdmiphy1_ChannelId)Id);
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function calculates the PLL VCO operating frequency.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for TX or RX.
*
* @return	PLL VCO frequency in Hz
*
* @note		None.
*
******************************************************************************/
u64 XHdmiphy1_GetPllVcoFreqHz(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u64 PllxVcoRateHz;
	u64 PllRefClkHz;
	XHdmiphy1_Channel *PllPtr = &InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)];

	if (Dir == XHDMIPHY1_DIR_TX) {
		if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
			PllRefClkHz = InstancePtr->HdmiTxRefClkHz;
		}
		else {
			PllRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr,
							QuadId,
							PllPtr->PllRefClkSel);
		}
	}
	else {
		if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
			if (InstancePtr->HdmiRxDruIsEnabled) {
				PllRefClkHz =
				XHdmiphy1_DruGetRefClkFreqHz(InstancePtr);
			}
			else {
				PllRefClkHz = InstancePtr->HdmiRxRefClkHz;
			}
		}
		else {
			PllRefClkHz = XHdmiphy1_GetQuadRefClkFreq(InstancePtr,
							QuadId,
							PllPtr->PllRefClkSel);
		}
	}

	PllxVcoRateHz = (u64)(PllRefClkHz *
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
						PllParams.N1FbDiv *
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
						PllParams.N2FbDiv) /
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(ChId)].
						PllParams.MRefClkDiv;

	return PllxVcoRateHz;
}

/*****************************************************************************/
/**
* This function returns the number of active reference clock sources
* based in the CFG
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	No of active REFCLK sources
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_GetRefClkSourcesCount(XHdmiphy1 *InstancePtr)
{
	u8 RefClkNum = 0;
	u8 RefClkNumMax = 5;
	XHdmiphy1_PllRefClkSelType RefClkSel[RefClkNumMax];
	XHdmiphy1_PllRefClkSelType RefClkSelTemp[RefClkNumMax];
	u8 i, j, Match;

	/* TxRefClkSel */
	RefClkSel[0] = (InstancePtr->Config.TxProtocol !=
                        XHDMIPHY1_PROTOCOL_NONE) ?
                            InstancePtr->Config.TxRefClkSel : 99;
	/* RxRefClkSel */
	RefClkSel[1] = (InstancePtr->Config.RxProtocol !=
                        XHDMIPHY1_PROTOCOL_NONE) ?
                            InstancePtr->Config.RxRefClkSel : 99;
	/* DruRefClkSel */
	RefClkSel[2] = (InstancePtr->Config.DruIsPresent) ?
                            InstancePtr->Config.DruRefClkSel : 99;
	/* TxFrlRefClkSel */
	RefClkSel[3] = (InstancePtr->Config.TxProtocol ==
                        XHDMIPHY1_PROTOCOL_HDMI21) ?
                            InstancePtr->Config.TxFrlRefClkSel : 99;
	/* RxFrlRefClkSel */
	RefClkSel[4] = (InstancePtr->Config.RxProtocol ==
                        XHDMIPHY1_PROTOCOL_HDMI21) ?
                            InstancePtr->Config.RxFrlRefClkSel : 99;

	/* Initialize Unique RefClk holder */
	for (i=0; i<RefClkNumMax; i++) {
		RefClkSelTemp[i] = 99;
	}

	i = 0;
	do {
		if (RefClkSel[i] != 99) {
			Match = 0;
			j = 0;
			/* Check if RefClkSel is already in Unique Holder array */
			do {
				if (RefClkSelTemp[j] == RefClkSel[i]) {
					Match |= 1;
				}
				j++;
			} while (j<RefClkNum);

			/* Register in Unique Holder if new RefClk is detected */
			if (Match == 0) {
				RefClkSelTemp[RefClkNum] = RefClkSel[i];
				/* Increment RefClk counter */
				RefClkNum++;
			}
		}
		i++;
	} while (i<RefClkNumMax);

	return RefClkNum;
}
#endif

#ifdef __cplusplus
/*****************************************************************************/
/**
* This function is a transceiver adaptor to set the clock and data recovery
* (CDR) values for a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	return InstancePtr->GtAdaptor->CfgSetCdr(InstancePtr, QuadId, ChId);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to check if a given PLL output
* frequency is within the operating range of the PLL for the GT type.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	PllClkOutFreqHz is the frequency to check.
*
* @return
*		- XST_SUCCESS if the frequency resides within the PLL's range.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
				XHdmiphy1_ChannelId ChId,
				u64 PllClkOutFreqHz)
{
	return InstancePtr->GtAdaptor->CheckPllOpRange(InstancePtr,
							QuadId, ChId,
							PllClkOutFreqHz);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to set the output divider logic for
* a given channel.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	return InstancePtr->GtAdaptor->OutDivChReconfig(InstancePtr, QuadId,
							ChId, Dir);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to configure the channel
* clock settings.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	return InstancePtr->GtAdaptor->ClkChReconfig(InstancePtr, QuadId, ChId);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to configure the common channel
* clock settings.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	CmnId is the common channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	return InstancePtr->GtAdaptor->ClkCmnReconfig(InstancePtr, QuadId, ChId);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to configure the channel's
* RX settings.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	return InstancePtr->GtAdaptor->RxChReconfig(InstancePtr, QuadId, ChId);
}

/*****************************************************************************/
/**
* This function is a transceiver adaptor to configure the channel's
* TX settings.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if the configuration was successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	return InstancePtr->GtAdaptor->TxChReconfig(InstancePtr, QuadId, ChId);
}
#endif

/*****************************************************************************/
/**
* This function checks if Instance is HDMI 2.0 or HDMI 2.1
*
* @param	InstancePtr is a pointer to the HDMIPHY instance.
* @param	Dir is an indicator for RX or TX.
*
* @return	TRUE if HDMI 2.0 or 2.1 else FALSE.
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_IsHDMI(XHdmiphy1 *InstancePtr, XHdmiphy1_DirectionType Dir)
{
	if (Dir == XHDMIPHY1_DIR_TX) {
		if ((InstancePtr->Config.TxProtocol == XHDMIPHY1_PROTOCOL_HDMI) ||
			(InstancePtr->Config.TxProtocol == XHDMIPHY1_PROTOCOL_HDMI21)) {
			return TRUE;
		}
	} else { /* Dir == XHDMIPHY1_DIR_RX */
		if ((InstancePtr->Config.RxProtocol == XHDMIPHY1_PROTOCOL_HDMI) ||
			(InstancePtr->Config.RxProtocol == XHDMIPHY1_PROTOCOL_HDMI21)) {
			return TRUE;
		}
	}

	return FALSE;
}

/*****************************************************************************/
/**
* This function is the error condition handler
*
* @param	InstancePtr is a pointer to the HDMIPHY instance.
* @param    ErrIrqType is the error type
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ErrorHandler(XHdmiphy1 *InstancePtr)
{
	if (InstancePtr->ErrorCallback != NULL) {
		InstancePtr->ErrorCallback(InstancePtr->ErrorRef);
	}
}
