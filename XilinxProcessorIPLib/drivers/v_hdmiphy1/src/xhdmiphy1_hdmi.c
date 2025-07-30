/*******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_hdmi.c
 *
 * This file contains video PHY functionality specific to the HDMI protocol.
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
 * 1.1   ku   24/07/20 Program MMCM params based on max line rate
 *                     configured in IP GUI
 * 1.2   ssh  02/02/23 Added API for Clock Detector Accuracy Range
 * 1.3   ssh  17/07/23 Added support for MMCM/PLL Clock Primitive
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"
#include "xhdmiphy1_hdmi.h"

/****************************** Type Definitions ******************************/

typedef struct {
	u64 DruLineRate;
	u16 PllScale;
	u32 Qpll0RefClkMin;
	u32 Qpll1RefClkMin;
	u32 CpllRefClkMin;
	u16 TxMmcmScale;
	u64 TxMmcmFvcoMin;
	u64 TxMmcmFvcoMax;
	u16 RxMmcmScale;
	u64 RxMmcmFvcoMin;
	u64 RxMmcmFvcoMax;
} XHdmiphy1_GtHdmiChars;

/**************************** Function Prototypes *****************************/

extern void XHdmiphy1_Ch2Ids(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
		u8 *Id0, u8 *Id1);
static const XHdmiphy1_GtHdmiChars *GetGtHdmiPtr(XHdmiphy1 *InstancePtr);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
static void XHdmiphy1_HdmiSetSystemClockSelection(XHdmiphy1 *InstancePtr,
                u8 QuadId);
#endif
static void XHdmiphy1_MmcmParam(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir);
/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function initializes the Video PHY for HDMI.
 *
 * @param	InstancePtr is a pointer to the XHdmiphy1 instance.
 * @param	CfgPtr is a pointer to the configuration structure that will
 *		        be used to copy the settings from.
* @param	QuadId is the GT quad ID to operate on.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XHdmiphy1_Hdmi_CfgInitialize(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_Config *CfgPtr)
{
	u8 Id, Id0, Id1;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Init done. */
	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_INIT, 0);

	/* Setup the instance. */
	XHdmiphy1_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddr);

	/* Set default. */
	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
			XHDMIPHY1_GT_STATE_IDLE;
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
			XHDMIPHY1_GT_STATE_IDLE;
		/* Initialize Transceiver Width values */
		if (InstancePtr->Config.TransceiverWidth == 2) {
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				TxDataWidth = 20;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				TxIntDataWidth = 2;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				RxDataWidth = 20;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				RxIntDataWidth = 2;
		}
		else {
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				TxDataWidth = 40;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				TxIntDataWidth = 4;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				RxDataWidth = 40;
			InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
				RxIntDataWidth = 4;
		}
	}

	/* Interrupt Disable. */
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_QPLL0_LOCK);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK);
#else
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE);
#endif
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT);
	XHdmiphy1_IntrDisable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT);

	/* Setup HDMI interrupt handler callback*/
	XHdmiphy1_HdmiIntrHandlerCallbackInit(InstancePtr);

	/* Configure clock detector. */
	XHdmiphy1_ClkDetEnable(InstancePtr, FALSE);
	XHdmiphy1_ClkDetSetFreqTimeout(InstancePtr,
                InstancePtr->Config.AxiLiteClkFreq);
	XHdmiphy1_ClkDetSetFreqLockThreshold(InstancePtr, 40);
	XHdmiphy1_ClkDetAccuracyRange(InstancePtr, 8);

	/* Start capturing logs. */
	XHdmiphy1_LogReset(InstancePtr);
	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_INIT, 0);

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_HdmiSetSystemClockSelection(InstancePtr, QuadId);

	/* Indicate of QPLL is present in design */
	if ((XHdmiphy1_IsTxUsingQpll(InstancePtr,
			QuadId, XHDMIPHY1_CHANNEL_ID_CH1) &&
			(XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX))) ||
		(XHdmiphy1_IsRxUsingQpll(InstancePtr,
				QuadId, XHDMIPHY1_CHANNEL_ID_CH1) &&
			(XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))) {
		InstancePtr->HdmiIsQpllPresent = TRUE;
	} else {
		InstancePtr->HdmiIsQpllPresent = FALSE;
	}

	if ((InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) ||
	    (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4)) {
		XHdmiphy1_SetBufgGtDiv(InstancePtr, XHDMIPHY1_DIR_TX, 1);
		XHdmiphy1_SetBufgGtDiv(InstancePtr, XHDMIPHY1_DIR_RX, 1);
	}
    XHdmiphy1_PowerDownGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CMNA,
            TRUE);
    XHdmiphy1_PowerDownGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
            TRUE);
	XHdmiphy1_ResetGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
			XHDMIPHY1_DIR_RX, TRUE);
	XHdmiphy1_ResetGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
			XHDMIPHY1_DIR_TX, TRUE);
#endif
	XHdmiphy1_MmcmReset(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, TRUE);
	XHdmiphy1_MmcmReset(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, TRUE);
	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		XHdmiphy1_IBufDsEnable(InstancePtr, QuadId, XHDMIPHY1_DIR_TX, (FALSE));
	}
	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		XHdmiphy1_IBufDsEnable(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, (FALSE));
	}

	/* DRU Settings. */
	if (InstancePtr->Config.DruIsPresent) {
		XHdmiphy1_IBufDsEnable(InstancePtr, QuadId, XHDMIPHY1_DIR_RX, TRUE);
		XHdmiphy1_DruReset(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, TRUE);
		XHdmiphy1_DruEnable(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);
	}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	/* Set to DFE */
	XHdmiphy1_SetRxLpm(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_RX, 0);
#endif

	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		XHdmiphy1_SetTxVoltageSwing(InstancePtr, QuadId,
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
			(XHdmiphy1_ChannelId)Id, 0xB);
#else
			(XHdmiphy1_ChannelId)Id, 0x1F);
#endif
		XHdmiphy1_SetTxPreEmphasis(InstancePtr, QuadId,
            (XHdmiphy1_ChannelId)Id, 0x4);
		XHdmiphy1_SetTxPostCursor(InstancePtr, QuadId,
            (XHdmiphy1_ChannelId)Id, 0x4);
	}

	/* Clear Interrupt Register */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_INTR_STS_REG,
			0xFFFFFFFF);

	/* Interrupt Enable. */
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_QPLL0_LOCK);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK);
#else
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE);
#endif
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT);
	XHdmiphy1_IntrEnable(InstancePtr,
			XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE);
	XHdmiphy1_ClkDetEnable(InstancePtr, TRUE);

	/* Set the flag to indicate the driver is. */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/* Init done. */
	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_INIT, 1);

	return XST_SUCCESS;
}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
/*****************************************************************************/
/**
* This function Sets the System Clock Selection
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdmiphy1_HdmiSetSystemClockSelection(XHdmiphy1 *InstancePtr,
                u8 QuadId)
{
	XHdmiphy1_PllType XHdmiphy1_QPllType;

     XHdmiphy1_QPllType = XHDMIPHY1_PLL_TYPE_QPLL0;

    /* Set system clock selections */
    if (InstancePtr->Config.TxSysPllClkSel ==
            InstancePtr->Config.RxSysPllClkSel) {
        if (InstancePtr->Config.RxSysPllClkSel ==
                XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK) {
            XHdmiphy1_PllInitialize(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CHA,
                    InstancePtr->Config.RxRefClkSel,
                    InstancePtr->Config.RxRefClkSel,
                    XHDMIPHY1_PLL_TYPE_CPLL,
                    XHDMIPHY1_PLL_TYPE_CPLL);
        }
        else {
            XHdmiphy1_PllInitialize(InstancePtr, QuadId,
                    XHDMIPHY1_CHANNEL_ID_CMN0,
                    InstancePtr->Config.RxRefClkSel,
                    InstancePtr->Config.RxRefClkSel,
                    XHdmiphy1_QPllType,
                    XHdmiphy1_QPllType);
        }
    }
    else if (InstancePtr->Config.TxSysPllClkSel ==
            XHDMIPHY1_SYSCLKSELDATA_TYPE_CPLL_OUTCLK) {
        XHdmiphy1_PllInitialize(InstancePtr, QuadId,
                XHDMIPHY1_CHANNEL_ID_CHA,
                InstancePtr->Config.RxRefClkSel,
                InstancePtr->Config.TxRefClkSel,
                XHDMIPHY1_PLL_TYPE_CPLL,
                XHdmiphy1_QPllType);
    }
    else {
        XHdmiphy1_PllInitialize(InstancePtr, QuadId,
                XHDMIPHY1_CHANNEL_ID_CMN0,
                InstancePtr->Config.TxRefClkSel,
                InstancePtr->Config.RxRefClkSel,
                XHdmiphy1_QPllType,
                XHDMIPHY1_PLL_TYPE_CPLL);
    }
}

/*****************************************************************************/
/**
* This function Updates the HDMIPHY clocking.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	TxSysPllClkSel is the SYSCLKDATA selection for TX.
* @param	RxSysPllClkSel is the SYSCLKDATA selection for RX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_HdmiUpdateClockSelection(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_SysClkDataSelType TxSysPllClkSel,
		XHdmiphy1_SysClkDataSelType RxSysPllClkSel)
{
	u8 Id, Id0, Id1;

	/* Reset PLL */
	XHdmiphy1_ResetGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
			XHDMIPHY1_DIR_RX, TRUE);
	XHdmiphy1_ResetGtPll(InstancePtr, QuadId, XHDMIPHY1_CHANNEL_ID_CHA,
			XHDMIPHY1_DIR_TX, TRUE);

	/* Set default. */
	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
			XHDMIPHY1_GT_STATE_IDLE;
		InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
			XHDMIPHY1_GT_STATE_IDLE;
	}

	/* Update Hdmiphy Clocking */
	InstancePtr->Config.TxSysPllClkSel = TxSysPllClkSel;
	InstancePtr->Config.RxSysPllClkSel = RxSysPllClkSel;
	XHdmiphy1_HdmiSetSystemClockSelection(InstancePtr, QuadId);
}

/*****************************************************************************/
/**
* This function resets the GT TX alignment module.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId is the channel ID to operate on.
* @param	Reset specifies TRUE/FALSE value to either assert or deassert
*		reset on the TX alignment module, respectively.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_TxAlignReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset)
{
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Read TX align register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_TX_BUFFER_BYPASS_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		MaskVal |= XHDMIPHY1_TX_BUFFER_BYPASS_TXPHDLYRESET_MASK(Id);
	}

	/* Write new value to BUFG_GT register. */
	if (Reset) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
        XHDMIPHY1_TX_BUFFER_BYPASS_REG, RegVal);
}

/*****************************************************************************/
/**
* This function resets the GT TX alignment module.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId is the channel ID to operate on.
* @param	Start specifies TRUE/FALSE value to either start or ttop the TX
*		alignment module, respectively.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_TxAlignStart(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Start)
{
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Read TX align register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_TX_BUFFER_BYPASS_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		MaskVal |= XHDMIPHY1_TX_BUFFER_BYPASS_TXPHALIGN_MASK(Id);
	}

	/* Write new value to BUFG_GT register. */
	if (Start) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
        XHDMIPHY1_TX_BUFFER_BYPASS_REG, RegVal);
}
#endif

/*****************************************************************************/
/**
* This function enables the HDMIPHY's detector peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the clock detector respectively.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetEnable(XHdmiphy1 *InstancePtr, u8 Enable)
{
	u32 RegVal;

	/* Read clkdet ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_CTRL_REG);

	/* Write new value to clkdet ctrl register. */
	if (Enable) {
		RegVal |= XHDMIPHY1_CLKDET_CTRL_RUN_MASK;
	}
	else {
		RegVal &= ~XHDMIPHY1_CLKDET_CTRL_RUN_MASK;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_CLKDET_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function clears the clock detector TX/RX timer.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetTimerClear(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir)
{
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	/* Read the clock detector control register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_CTRL_REG);

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegVal |= XHDMIPHY1_CLKDET_CTRL_TX_TMR_CLR_MASK;
	}
	else {
		RegVal |= XHDMIPHY1_CLKDET_CTRL_RX_TMR_CLR_MASK;
	}

	/* Write new value to clkdet ctrl register. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_CLKDET_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function resets clock detector TX/RX frequency.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetFreqReset(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir)
{
	u32 RegVal;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	/* Read clkdet ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_CTRL_REG);

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegVal |= XHDMIPHY1_CLKDET_CTRL_TX_FREQ_RST_MASK;
	}
	else {
		RegVal |= XHDMIPHY1_CLKDET_CTRL_RX_FREQ_RST_MASK;
	}

	/* Write new value to clkdet ctrl register. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_CLKDET_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function sets the clock detector frequency lock counter threshold value.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ThresholdVal is the threshold value to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetSetFreqLockThreshold(XHdmiphy1 *InstancePtr,
        u16 ThresholdVal)
{
	u32 RegVal;

	/* Read clkdet ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_CTRL_REG);
	RegVal &= ~XHDMIPHY1_CLKDET_CTRL_RX_FREQ_RST_MASK;

	/* Update with new threshold. */
	RegVal |= (ThresholdVal << XHDMIPHY1_CLKDET_CTRL_FREQ_LOCK_THRESH_SHIFT);

	/* Write new value to clkdet ctrl register. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_CLKDET_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function sets the clock detector accuracy range value.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ThresholdVal is the threshold value to be set.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetAccuracyRange(XHdmiphy1 *InstancePtr,
        u16 ThresholdVal)
{
	u32 RegVal;

	/* Read clkdet ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_CTRL_REG);
	RegVal &= ~XHDMIPHY1_CLKDET_CTRL_ACCURACY_RANGE_MASK;

	/* Update with new threshold. */
	RegVal |= (ThresholdVal << XHDMIPHY1_CLKDET_CTRL_ACCURACY_RANGE_SHIFT);

	/* Write new value to clkdet ctrl register. */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_CLKDET_CTRL_REG,
			RegVal);
}
/*****************************************************************************/
/**
* This function checks clock detector RX/TX frequency zero indicator bit.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Dir is an indicator for RX or TX.
*
* @return	- TRUE if zero frequency.
*		- FALSE otherwise, if non-zero frequency.
*
* @note		None.
*
******************************************************************************/
u8 XHdmiphy1_ClkDetCheckFreqZero(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir)
{
	u32 MaskVal = 0;
	u32 RegVal;

	if (Dir == XHDMIPHY1_DIR_TX) {
		MaskVal = XHDMIPHY1_CLKDET_STAT_TX_FREQ_ZERO_MASK;
	}
	else {
		MaskVal = XHDMIPHY1_CLKDET_STAT_RX_FREQ_ZERO_MASK;
	}

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_DRU_STAT_REG);
	RegVal &= MaskVal;

	if (RegVal) {
		return (TRUE);
	}

	return (FALSE);
}

/*****************************************************************************/
/**
* This function sets clock detector frequency lock counter threshold value.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	TimeoutVal is the timeout value and is normally the system clock
*		frequency.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetSetFreqTimeout(XHdmiphy1 *InstancePtr, u32 TimeoutVal)
{
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_FREQ_TMR_TO_REG, TimeoutVal);
}

/*****************************************************************************/
/**
* This function loads the timer to TX/RX in the clock detector.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
* @param	TimeoutVal is the timeout value to store in the clock detector.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_ClkDetTimerLoad(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, u32 TimeoutVal)
{
	u32 RegOffset;

	/* Suppress Warning Messages */
	QuadId = QuadId;

	if (Dir == XHDMIPHY1_DIR_TX) {
		RegOffset = XHDMIPHY1_CLKDET_TMR_TX_REG;
	}
	else {
		RegOffset = XHDMIPHY1_CLKDET_TMR_RX_REG;
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, TimeoutVal);
}

/*****************************************************************************/
/**
* This function returns the frequency of the RX/TX reference clock as
* measured by the clock detector peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Dir is an indicator for RX or TX.
*
* @return	The measured frequency of the RX/TX reference clock.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_ClkDetGetRefClkFreqHz(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir)
{
	u32 RegOffset;

	if (Dir == XHDMIPHY1_DIR_TX) {
		if (InstancePtr->TxHdmi21Cfg.IsEnabled == 1) {
			if (InstancePtr->Config.TxFrlRefClkSel ==
					InstancePtr->Config.TxRefClkSel) {
				RegOffset = XHDMIPHY1_CLKDET_FREQ_TX_REG;
			} else {
				RegOffset = XHDMIPHY1_CLKDET_FREQ_TX_FRL_REG;
			}
		} else {
			RegOffset = XHDMIPHY1_CLKDET_FREQ_TX_REG;
		}
	}
	else {
		if (InstancePtr->RxHdmi21Cfg.IsEnabled == 1) {
			if (InstancePtr->Config.RxFrlRefClkSel ==
					InstancePtr->Config.RxRefClkSel) {
				RegOffset = XHDMIPHY1_CLKDET_FREQ_RX_REG;
			} else {
				RegOffset = XHDMIPHY1_CLKDET_FREQ_RX_FRL_REG;
			}
		} else {
			RegOffset = XHDMIPHY1_CLKDET_FREQ_RX_REG;
		}
	}

	return XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, RegOffset);
}

/*****************************************************************************/
/**
* This function returns the frequency of the DRU reference clock as measured by
* the clock detector peripheral.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	The measured frequency of the DRU reference clock.
*
* @note		The design must have a DRU for this function to return a valid
*		value.
*
******************************************************************************/
u32 XHdmiphy1_DruGetRefClkFreqHz(XHdmiphy1 *InstancePtr)
{
	u32 DruFreqHz = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_CLKDET_FREQ_DRU_REG);



	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) {
		if (DruFreqHz > XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK_MAX){
			return XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK;
		}
		else if (DruFreqHz > XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2_MAX){
			return XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK2;
		}
		else if (DruFreqHz > XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3_MAX){
			return XHDMIPHY1_HDMI_GTHE4_DRU_REFCLK3;
		}
	}
	else if (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4) {
		if (DruFreqHz > XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK_MAX){
			return XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK;
		}
		else if (DruFreqHz > XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2_MAX){
			return XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK2;
		}
		else if (DruFreqHz > XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3_MIN &&
				DruFreqHz < XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3_MAX){
			return XHDMIPHY1_HDMI_GTYE4_DRU_REFCLK3;
		}
	}
	else {
		if (InstancePtr->versal_2ve_2vm == 1) {
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYP_DRU_REFCLK_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYP_DRU_REFCLK_MAX){
				return XHDMIPHY1_HDMI_GTYP_DRU_REFCLK;
			}
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1_MAX){
				return XHDMIPHY1_HDMI_GTYP_DRU_REFCLK1;
			}
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2_MAX){
				return XHDMIPHY1_HDMI_GTYP_DRU_REFCLK2;
			}

		} else {
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK_MAX){
				return XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK;
			}
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1_MAX){
				return XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK1;
			}
			if (DruFreqHz > XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MIN &&
					DruFreqHz < XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2_MAX){
				return XHDMIPHY1_HDMI_GTYE5_DRU_REFCLK2;
			}
		}
	}

	/* Return Failure */
	return XST_FAILURE;
}

/*****************************************************************************/
/**
* This function resets the DRU in the HDMIPHY.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId is the channel ID to operate on.
* @param	Reset specifies TRUE/FALSE value to either enable or disable
*		the DRU respectively.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_DruReset(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Reset)
{
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Read DRU ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_DRU_CTRL_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		MaskVal |= XHDMIPHY1_DRU_CTRL_RST_MASK(Id);
	}

	/* Write DRU ctrl register. */
	if (Reset) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_DRU_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function enabled/disables the DRU in the HDMIPHY.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId is the channel ID to operate on.
* @param	Enable specifies TRUE/FALSE value to either enable or disable
*		the DRU, respectively.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_DruEnable(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 Enable)
{
	u32 RegVal;
	u32 MaskVal = 0;
	u8 Id, Id0, Id1;

	/* Read DRU ctrl register. */
	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_DRU_CTRL_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		MaskVal |= XHDMIPHY1_DRU_CTRL_EN_MASK(Id);
	}

	/* Write DRU ctrl register. */
	if (Enable) {
		RegVal |= MaskVal;
	}
	else {
		RegVal &= ~MaskVal;
	}
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_DRU_CTRL_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function gets the DRU version
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
*
* @return	None.
*
******************************************************************************/
u16 XHdmiphy1_DruGetVersion(XHdmiphy1 *InstancePtr)
{
	u32 RegVal;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_DRU_STAT_REG);
	RegVal &= XHDMIPHY1_DRU_STAT_VERSION_MASK;
	RegVal >>= XHDMIPHY1_DRU_STAT_VERSION_SHIFT;

	return ((u16)RegVal);
}

/*****************************************************************************/
/**
* This function sets the DRU center frequency.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	ChId specifies the channel ID.
* @param	CenterFreqHz is the frequency value to set.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_DruSetCenterFreqHz(XHdmiphy1 *InstancePtr,
        XHdmiphy1_ChannelId ChId, u64 CenterFreqHz)
{
	u32 CenterFreqL;
	u32 CenterFreqH;
	u32 RegOffset;
	u8 Id, Id0, Id1;

	/* Split the 64-bit input into 2 32-bit values. */
	CenterFreqL = (u32)CenterFreqHz;
	CenterFreqHz >>= 32;
	CenterFreqHz &= XHDMIPHY1_DRU_CFREQ_H_MASK;;
	CenterFreqH = (u32)CenterFreqHz;

	XHdmiphy1_Ch2Ids(InstancePtr, ChId, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		RegOffset = XHDMIPHY1_DRU_CFREQ_L_REG(Id);
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset,
				CenterFreqL);

		RegOffset = XHDMIPHY1_DRU_CFREQ_H_REG(Id);
		XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, RegOffset,
				CenterFreqH);
	}
}

/*****************************************************************************/
/**
* This function calculates the center frequency value for the DRU.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return	The calculated DRU Center frequency value.
*
* @note		According to XAPP1240:
*			Center_f = fDIN * (2^32)/fdruclk
*		The DRU clock is derived from the measured reference clock and
*		the current QPLL settings.
*
******************************************************************************/
u64 XHdmiphy1_DruCalcCenterFreqHz(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId)
{
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_Channel *ChPtr;
	u64 DruRefClk;
#endif
	u64 ClkDetRefClk;
	u64 DataRate;
	u64 FDin;
	u64 FDruClk;

	ClkDetRefClk = XHdmiphy1_ClkDetGetRefClkFreqHz(InstancePtr,
                        XHDMIPHY1_DIR_RX);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	DruRefClk = XHdmiphy1_DruGetRefClkFreqHz(InstancePtr);

	/* Take the master channel (channel 1). */
	ChPtr = &InstancePtr->Quads[QuadId].Ch1;

	if ((ChId == XHDMIPHY1_CHANNEL_ID_CMN0) ||
			(ChId == XHDMIPHY1_CHANNEL_ID_CMN1)) {
		FDruClk = (DruRefClk * InstancePtr->Quads[QuadId].Plls[
			          XHDMIPHY1_CH2IDX(ChId)].PllParams.NFbDiv) /
			      (InstancePtr->Quads[QuadId].Plls[
			          XHDMIPHY1_CH2IDX(ChId)].PllParams.MRefClkDiv *
			          (ChPtr->RxOutDiv * 20));
	}
	else {
		FDruClk = (DruRefClk * ChPtr->PllParams.N1FbDiv *
			ChPtr->PllParams.N2FbDiv * 2) /
			(ChPtr->PllParams.MRefClkDiv * ChPtr->RxOutDiv * 20);
	}
#else
	/* Suppress Warning */
	QuadId = QuadId;
	ChId = ChId;

	FDruClk = (u64) (XHDMIPHY1_HDMI_GTYE5_DRU_LRATE / 20);
#endif
	DataRate = 10 * ClkDetRefClk;
	FDin = DataRate * ((u64)1 << 32);

	/* Check for divide by zero. */
	if (FDin && FDruClk) {
		return (FDin / FDruClk);
	}
	return 0;
}

/*****************************************************************************/
/**
* This function sets the GT RX CDR and Equalization for DRU mode.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	Enable enables the DRU logic (when 1), or disables (when 0).
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_HdmiGtDruModeEnable(XHdmiphy1 *InstancePtr, u8 Enable)
{
	u32 RegVal;
	u32 RegMask = 0;
	u8 Id, Id0, Id1;

	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_DRU_EN, Enable);

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
			XHDMIPHY1_RX_EQ_CDR_REG);

	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
	for (Id = Id0; Id <= Id1; Id++) {
		RegMask |= XHDMIPHY1_RX_STATUS_RXCDRHOLD_MASK(Id) |
			XHDMIPHY1_RX_STATUS_RXOSOVRDEN_MASK(Id) |
			XHDMIPHY1_RX_STATUS_RXLPMLFKLOVRDEN_MASK(Id) |
			XHDMIPHY1_RX_STATUS_RXLPMHFOVRDEN_MASK(Id);
	}

	if (Enable) {
		RegVal |= RegMask;
	}
	else {
		RegVal &= ~RegMask;
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_RX_EQ_CDR_REG,
			RegVal);
}

/*****************************************************************************/
/**
* This function calculates the HDMI MMCM parameters.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
* @param	Ppc specifies the total number of pixels per clock.
*		- 1 = XVIDC_PPC_1
*		- 2 = XVIDC_PPC_2
*		- 4 = XVIDC_PPC_4
* @param	Bpc specifies the color depth/bits per color component.
*		- 6 = XVIDC_BPC_6
*		- 8 = XVIDC_BPC_8
*		- 10 = XVIDC_BPC_10
*		- 12 = XVIDC_BPC_12
*		- 16 = XVIDC_BPC_16
*
* @return
*		- XST_SUCCESS if calculated PLL parameters updated successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiCfgCalcMmcmParam(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
		XVidC_PixelsPerClock Ppc, XVidC_ColorDepth Bpc)
{
	u32 RefClk;
	u16 Div;
	u16 Mult;
	u16 MultDiv;
	u8 Valid;
	u64 LineRate = 0;
	XHdmiphy1_Mmcm *MmcmPtr;
	XHdmiphy1_PllType PllType;
	u64 clokoutdiv_dpll_lo;
	u64 clokoutdiv_dpll_high;
	u64 div_dpll_lo;
	u64 div_dpll_high;





	/* Suppress Warning Messages */
	ChId = ChId;

	/* Get line rate. */
	PllType = XHdmiphy1_GetPllType(InstancePtr, 0, Dir,
			XHDMIPHY1_CHANNEL_ID_CH1);

	switch (PllType) {
		case XHDMIPHY1_PLL_TYPE_QPLL:
		case XHDMIPHY1_PLL_TYPE_QPLL0:
		case XHDMIPHY1_PLL_TYPE_LCPLL:
			LineRate = InstancePtr->Quads[QuadId].Cmn0.LineRateHz;
			break;
		case XHDMIPHY1_PLL_TYPE_QPLL1:
		case XHDMIPHY1_PLL_TYPE_RPLL:
			LineRate = InstancePtr->Quads[QuadId].Cmn1.LineRateHz;
			break;
		default:
			LineRate = InstancePtr->Quads[QuadId].Ch1.LineRateHz;
			break;
	}

	if (((LineRate / 1000000) > 2970) && (Ppc == XVIDC_PPC_1)) {
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_1PPC_ERR, 1);
		XHdmiphy1_ErrorHandler(InstancePtr);
		return (XST_FAILURE);
	}

	Div = 1;

	do {
		if (Dir == XHDMIPHY1_DIR_RX) {
			RefClk = InstancePtr->HdmiRxRefClkHz;
			MmcmPtr= &InstancePtr->Quads[QuadId].RxMmcm;

			RefClk = RefClk / (GetGtHdmiPtr(InstancePtr))->RxMmcmScale;
			Mult = (GetGtHdmiPtr(InstancePtr))->RxMmcmFvcoMax * Div / RefClk;
		}
		else {
			RefClk = InstancePtr->HdmiTxRefClkHz;
			MmcmPtr= &InstancePtr->Quads[QuadId].TxMmcm;

			RefClk = RefClk / (GetGtHdmiPtr(InstancePtr))->TxMmcmScale;
			Mult = (GetGtHdmiPtr(InstancePtr))->TxMmcmFvcoMax * Div / RefClk;
		}

		/* Return if RefClk is below valid range */
		if (RefClk < 20000000) {
			return (XST_FAILURE);
		}

		/* In case of 4 pixels per clock, the M must be a multiple of four. */
		if (Ppc == XVIDC_PPC_4) {
			Mult = Mult / 4;
			Mult = Mult * 4;
		}
		/* Else the M must be a multiple of two. */
		else if (Ppc == XVIDC_PPC_2) {
			Mult = Mult / 2;
			Mult = Mult * 2;
		}

		if (((Dir == XHDMIPHY1_DIR_RX) &&
				(InstancePtr->Config.RxClkPrimitive == 0)) ||
				((Dir == XHDMIPHY1_DIR_TX) &&
						(InstancePtr->Config.TxClkPrimitive == 0))) {

		Valid = (FALSE);
		do {
            MultDiv = Mult / Div;
			MmcmPtr->ClkFbOutMult = Mult;
			MmcmPtr->DivClkDivide = Div;

			if (InstancePtr->Config.TransceiverWidth == 4) {
				/* Link clock: TMDS clock ratio 1/40. */
				if ((LineRate / 1000000) >= 3400) {
					if ((Dir == XHDMIPHY1_DIR_TX) &&
							(((LineRate / 1000000) / InstancePtr->
									HdmiTxSampleRate) < 3400)) {
						MmcmPtr->ClkOut0Div = MultDiv * 4;
					}
					else {
						MmcmPtr->ClkOut0Div = MultDiv;
					}
				}
				/* Link clock: TMDS clock ratio 1/10. */
				else {
					MmcmPtr->ClkOut0Div = MultDiv * 4;
				}
			}
			else { /* 2 Byte Mode */
				/* Link clock: TMDS clock ratio 1/40. */
				if ((LineRate / 1000000) >= 3400) {
					if ((Dir == XHDMIPHY1_DIR_TX) &&
							(((LineRate / 1000000) / InstancePtr->
									HdmiTxSampleRate) < 3400)) {
						MmcmPtr->ClkOut0Div = MultDiv * 2;
					}
					else {
						MmcmPtr->ClkOut0Div = MultDiv / 2;
					}
				}
				/* Link clock: TMDS clock ratio 1/10. */
				else {
					MmcmPtr->ClkOut0Div = MultDiv * 2;
				}
			}

			/* TMDS Clock */
			MmcmPtr->ClkOut1Div = MultDiv * ((Dir == XHDMIPHY1_DIR_TX) ?
					(InstancePtr->HdmiTxSampleRate) : 1);

			/* Video clock. */
			MmcmPtr->ClkOut2Div = 0;

			switch (Bpc) {
			case XVIDC_BPC_10:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 5 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					/* The clock ratio is 2.5 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by two (2 * 2.5 = 5)
						to get an integer number */
					if ((MultDiv % 2) == 0) {
						MmcmPtr->ClkOut2Div = (MultDiv * 5 / 2 *
							((Dir == XHDMIPHY1_DIR_TX)?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				/* Single pixel. */
				else {
					/* The clock ratio is 1.25 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by four (4 * 1.25 = 5)
						to get an integer number */
					if ((MultDiv % 4) == 0) {
						MmcmPtr->ClkOut2Div = (MultDiv * 5 / 4 *
							((Dir == XHDMIPHY1_DIR_TX) ?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				break;
			case XVIDC_BPC_12:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 6 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 3 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					/* The clock ratio is 1.5 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by two (2 * 1.5 = 3)
						to get an integer number */
					if ((MultDiv % 2) == 0) {
						MmcmPtr->ClkOut2Div = (MultDiv * 3 / 2 *
							((Dir == XHDMIPHY1_DIR_TX) ?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				break;
			case XVIDC_BPC_16 :
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 8 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 4 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					MmcmPtr->ClkOut2Div = (MultDiv * 2 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				break;
			case XVIDC_BPC_8:
			default:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 4 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut2Div = (MultDiv * 2 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					MmcmPtr->ClkOut2Div = (MultDiv *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				break;
			}

			/* Only do this when the ClkOut2Div has been set */
			if (MmcmPtr->ClkOut2Div) {
				if (Dir == XHDMIPHY1_DIR_RX) {
					/* Correct divider value if TMDS clock ratio is 1/40. */
					if (InstancePtr->HdmiRxTmdsClockRatio) {
						if ((MmcmPtr->ClkOut2Div % 4) == 0) {
							MmcmPtr->ClkOut2Div = MmcmPtr->ClkOut2Div / 4;
						}
						/* Not divisible by 4: repeat loop with a lower
						 * multiply value. */
						else {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
							MmcmPtr->ClkOut2Div = 255;
#else
							MmcmPtr->ClkOut2Div = 65535;
#endif
						}
					}
				}
				/* TX. */
				else if ((((LineRate / 1000000) >= 3400) &&
							(InstancePtr->HdmiTxSampleRate == 1)) ||
						 (((LineRate / 1000000) / InstancePtr->
								HdmiTxSampleRate) >= 3400)) {
					if ((MmcmPtr->ClkOut2Div % 4) == 0) {
						MmcmPtr->ClkOut2Div = MmcmPtr->ClkOut2Div / 4;
					}
					/* Not divisible by 4: repeat loop with a lower
					 * multiply value. */
					else {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
						MmcmPtr->ClkOut2Div = 255;
#else
						MmcmPtr->ClkOut2Div = 65535;
#endif
					}
				}
			}

			/* Check values. */
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
			if ((MmcmPtr->ClkOut0Div > 0) && (MmcmPtr->ClkOut0Div <= 128) &&
				(MmcmPtr->ClkOut1Div > 0) && (MmcmPtr->ClkOut1Div <= 128) &&
				(MmcmPtr->ClkOut2Div > 0) && (MmcmPtr->ClkOut2Div <= 128)) {
#else

			if (Dir == XHDMIPHY1_DIR_RX) {
				if (InstancePtr->Config.RxClkPrimitive == 1 ){
					clokoutdiv_dpll_lo = 1;
					clokoutdiv_dpll_high = 400;
				}
				else{
					clokoutdiv_dpll_lo = 0;
					clokoutdiv_dpll_high = 512;
				}
			}
			else{
				if (InstancePtr->Config.TxClkPrimitive == 1 ){
					clokoutdiv_dpll_lo = 1;
					clokoutdiv_dpll_high = 400;
				}
				else{
					clokoutdiv_dpll_lo = 0;
					clokoutdiv_dpll_high = 512;
				}
			}

			if ((MmcmPtr->ClkOut0Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut0Div <= clokoutdiv_dpll_high) &&
			(MmcmPtr->ClkOut1Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut1Div <= clokoutdiv_dpll_high) &&
				(MmcmPtr->ClkOut2Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut2Div <= clokoutdiv_dpll_high)) {
#endif
				Valid = (TRUE);
			}
			else {
				/* 4 pixels per clock. */
				if (Ppc == (XVIDC_PPC_4)) {
					/* Decrease Mult value. */
					Mult -= 4;
				}
				/* 2 pixels per clock. */
				else if (Ppc == (XVIDC_PPC_2)) {
					/* Decrease M value. */
					Mult -= 2;
				}
				/* 1 pixel per clock */
				else {
					/* Decrease M value */
					Mult -= 1;
				}
			}
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))
		} while (!Valid && (Mult > 3) && (Mult < 432));
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4 || \
     XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
		} while (!Valid && (Mult > 0) && (Mult < 129));
#else
		} while (!Valid && (Mult > 0) && (Mult < 65));
#endif

	} else {
		Valid = (FALSE);
		do {
            MultDiv = Mult / Div;
			MmcmPtr->ClkFbOutMult = Mult;
			MmcmPtr->DivClkDivide = Div;

			if (InstancePtr->Config.TransceiverWidth == 4) {
				/* Link clock: TMDS clock ratio 1/40. */
				if ((LineRate / 1000000) >= 3400) {
					if ((Dir == XHDMIPHY1_DIR_TX) &&
							(((LineRate / 1000000) / InstancePtr->
									HdmiTxSampleRate) < 3400)) {
						MmcmPtr->ClkOut2Div = MultDiv * 4;
					}
					else {
						MmcmPtr->ClkOut2Div = MultDiv;
					}
				}
				/* Link clock: TMDS clock ratio 1/10. */
				else {
					MmcmPtr->ClkOut2Div = MultDiv * 4;
				}
			}
			else { /* 2 Byte Mode */
				/* Link clock: TMDS clock ratio 1/40. */
				if ((LineRate / 1000000) >= 3400) {
					if ((Dir == XHDMIPHY1_DIR_TX) &&
							(((LineRate / 1000000) / InstancePtr->
									HdmiTxSampleRate) < 3400)) {
						MmcmPtr->ClkOut2Div = MultDiv * 2;
					}
					else {
						MmcmPtr->ClkOut2Div = MultDiv / 2;
					}
				}
				/* Link clock: TMDS clock ratio 1/10. */
				else {
					MmcmPtr->ClkOut2Div = MultDiv * 2;
				}
			}

			/* TMDS Clock */
			MmcmPtr->ClkOut0Div = MultDiv * ((Dir == XHDMIPHY1_DIR_TX) ?
					(InstancePtr->HdmiTxSampleRate) : 1);

			/* Video clock. */
			MmcmPtr->ClkOut1Div = 0;

			switch (Bpc) {
			case XVIDC_BPC_10:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 5 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					/* The clock ratio is 2.5 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by two (2 * 2.5 = 5)
						to get an integer number */
					if ((MultDiv % 2) == 0) {
						MmcmPtr->ClkOut1Div = (MultDiv * 5 / 2 *
							((Dir == XHDMIPHY1_DIR_TX)?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				/* Single pixel. */
				else {
					/* The clock ratio is 1.25 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by four (4 * 1.25 = 5)
						to get an integer number */
					if ((MultDiv % 4) == 0) {
						MmcmPtr->ClkOut1Div = (MultDiv * 5 / 4 *
							((Dir == XHDMIPHY1_DIR_TX) ?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				break;
			case XVIDC_BPC_12:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 6 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 3 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					/* The clock ratio is 1.5 */
					/* The PLL only supports integer values */
					/* The MultDiv must be dividable by two (2 * 1.5 = 3)
						to get an integer number */
					if ((MultDiv % 2) == 0) {
						MmcmPtr->ClkOut1Div = (MultDiv * 3 / 2 *
							((Dir == XHDMIPHY1_DIR_TX) ?
							(InstancePtr->HdmiTxSampleRate) : 1));
					}
				}
				break;
			case XVIDC_BPC_16 :
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 8 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 4 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					MmcmPtr->ClkOut1Div = (MultDiv * 2 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				break;
			case XVIDC_BPC_8:
			default:
				/* Quad pixel. */
				if (Ppc == (XVIDC_PPC_4)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 4 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Dual pixel. */
				else if (Ppc == (XVIDC_PPC_2)) {
					MmcmPtr->ClkOut1Div = (MultDiv * 2 *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				/* Single pixel. */
				else {
					MmcmPtr->ClkOut1Div = (MultDiv *
						((Dir == XHDMIPHY1_DIR_TX) ?
						(InstancePtr->HdmiTxSampleRate) : 1));
				}
				break;
			}

			/* Only do this when the ClkOut2Div has been set */
			if (MmcmPtr->ClkOut1Div) {
				if (Dir == XHDMIPHY1_DIR_RX) {
					/* Correct divider value if TMDS clock ratio is 1/40. */
					if (InstancePtr->HdmiRxTmdsClockRatio) {
						if ((MmcmPtr->ClkOut1Div % 4) == 0) {
							MmcmPtr->ClkOut1Div = MmcmPtr->ClkOut1Div / 4;
						}
						/* Not divisible by 4: repeat loop with a lower
						 * multiply value. */
						else {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
							MmcmPtr->ClkOut1Div = 255;
#else
							MmcmPtr->ClkOut2Div = 65535;
#endif
						}
					}
				}
				/* TX. */
				else if ((((LineRate / 1000000) >= 3400) &&
							(InstancePtr->HdmiTxSampleRate == 1)) ||
						 (((LineRate / 1000000) / InstancePtr->
								HdmiTxSampleRate) >= 3400)) {
					if ((MmcmPtr->ClkOut1Div % 4) == 0) {
						MmcmPtr->ClkOut1Div = MmcmPtr->ClkOut1Div / 4;
					}
					/* Not divisible by 4: repeat loop with a lower
					 * multiply value. */
					else {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
						MmcmPtr->ClkOut1Div = 255;
#else
						MmcmPtr->ClkOut2Div = 65535;
#endif
					}
				}
			}

			/* Check values. */
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
			if ((MmcmPtr->ClkOut0Div > 0) && (MmcmPtr->ClkOut0Div <= 128) &&
				(MmcmPtr->ClkOut1Div > 0) && (MmcmPtr->ClkOut1Div <= 128) &&
				(MmcmPtr->ClkOut2Div > 0) && (MmcmPtr->ClkOut2Div <= 128)) {
#else

				if (Dir == XHDMIPHY1_DIR_RX) {
					if (InstancePtr->Config.RxClkPrimitive == 1 ){
						clokoutdiv_dpll_lo = 1;
						clokoutdiv_dpll_high = 400;
					}
					else{
						clokoutdiv_dpll_lo = 0;
						clokoutdiv_dpll_high = 512;
					}
				}
				else{
					if (InstancePtr->Config.TxClkPrimitive == 1 ){
						clokoutdiv_dpll_lo = 1;
						clokoutdiv_dpll_high = 400;
					}
					else{
						clokoutdiv_dpll_lo = 0;
						clokoutdiv_dpll_high = 512;
					}
				}


				if ((MmcmPtr->ClkOut0Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut0Div <= clokoutdiv_dpll_high) &&
					(MmcmPtr->ClkOut1Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut1Div <= clokoutdiv_dpll_high) &&
					(MmcmPtr->ClkOut2Div > clokoutdiv_dpll_lo) && (MmcmPtr->ClkOut2Div <= clokoutdiv_dpll_high)) {

#endif
				Valid = (TRUE);
			}
			else {
				/* 4 pixels per clock. */
				if (Ppc == (XVIDC_PPC_4)) {
					/* Decrease Mult value. */
					Mult -= 4;
				}
				/* 2 pixels per clock. */
				else if (Ppc == (XVIDC_PPC_2)) {
					/* Decrease M value. */
					Mult -= 2;
				}
				/* 1 pixel per clock */
				else {
					/* Decrease M value */
					Mult -= 1;
				}
			}
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))
		} while (!Valid && (Mult > 3) && (Mult < 432));
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4 || \
     XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
		} while (!Valid && (Mult > 0) && (Mult < 129));
#else
		} while (!Valid && (Mult > 0) && (Mult < 65));
#endif

	}
		/* Increment divider */
		Div++;
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))

		if (Dir == XHDMIPHY1_DIR_RX) {
			if (InstancePtr->Config.RxClkPrimitive == 1 ){
				div_dpll_lo = 0;
				div_dpll_high = 21;
			}
			else{
				div_dpll_lo = 0;
				div_dpll_high = 124;
			}
		}
		else{
			if (InstancePtr->Config.TxClkPrimitive == 1 ){
				div_dpll_lo = 0;
				div_dpll_high = 21;
			}
			else{
				div_dpll_lo = 0;
				div_dpll_high = 124;
			}
		}

		}while (!Valid && (Div > div_dpll_lo) && (Div < div_dpll_high));
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4 || \
     XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	} while (!Valid && (Div > 0) && (Div < 107));
#else
	} while (!Valid && (Div > 0) && (Div < 20));
#endif

	if (Valid) {
		return (XST_SUCCESS);
	}
	else {
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_MMCM_ERR, 1);
		XHdmiphy1_ErrorHandler(InstancePtr);
		return (XST_FAILURE);
	}
}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
/*****************************************************************************/
/**
* This function calculates the QPLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if calculated QPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiQpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status;
	u64 RefClk = 0;
	u32 *RefClkPtr;
	u64 TxLineRate = 0;
	u8 Id, Id0, Id1;

	u8 SRArray[] = {1, 3, 5};
	u8 SRIndex;
	u8 SRValue;

	/* Suppress Warning Messages */
	ChId = ChId;

	XHdmiphy1_SysClkDataSelType SysClkDataSel =
        (XHdmiphy1_SysClkDataSelType) 0;
	XHdmiphy1_SysClkOutSelType SysClkOutSel =
        (XHdmiphy1_SysClkOutSelType) 0;
	XHdmiphy1_ChannelId ActiveCmnId =
        XHDMIPHY1_CHANNEL_ID_CMN0;

	u32 QpllRefClk;
	u32 QpllClkMin = 0;

	/* Determine QPLL reference clock from the first (master) channel. */
	if (Dir == XHDMIPHY1_DIR_RX) {
		QpllRefClk = InstancePtr->HdmiRxRefClkHz;
		RefClkPtr = &InstancePtr->HdmiRxRefClkHz;
	}
	else {
		QpllRefClk = InstancePtr->HdmiTxRefClkHz;
		RefClkPtr = &InstancePtr->HdmiTxRefClkHz;
	}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
	/* Determine which QPLL to use. */
	if (((102343750 <= QpllRefClk) && (QpllRefClk <= 122500000)) ||
		((204687500 <= QpllRefClk) && (QpllRefClk <= 245000000)) ||
		((409375000 <= QpllRefClk) && (QpllRefClk <= 490000000))) {
		SysClkDataSel = XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK;
		SysClkOutSel = XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL1_REFCLK;
		ActiveCmnId = XHDMIPHY1_CHANNEL_ID_CMN1;
		QpllClkMin = (u32) XHDMIPHY1_HDMI_GTHE4_QPLL1_REFCLK_MIN;
	}
	else {
		SysClkDataSel = XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK;
		SysClkOutSel = XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL0_REFCLK;
		ActiveCmnId = XHDMIPHY1_CHANNEL_ID_CMN0;
		QpllClkMin = (u32) XHDMIPHY1_HDMI_GTHE4_QPLL0_REFCLK_MIN;
	}
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	/* Determine which QPLL to use. */
	if (((102343750 <= QpllRefClk) && (QpllRefClk <= 122500000)) ||
		((204687500 <= QpllRefClk) && (QpllRefClk <= 245000000)) ||
		((409375000 <= QpllRefClk) && (QpllRefClk <= 490000000))) {
		SysClkDataSel = XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL1_OUTCLK;
		SysClkOutSel = XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL1_REFCLK;
		ActiveCmnId = XHDMIPHY1_CHANNEL_ID_CMN1;
		QpllClkMin = (u32) XHDMIPHY1_HDMI_GTYE4_QPLL1_REFCLK_MIN;
	}
	else {
		SysClkDataSel = XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK;
		SysClkOutSel = XHDMIPHY1_SYSCLKSELOUT_TYPE_QPLL0_REFCLK;
		ActiveCmnId = XHDMIPHY1_CHANNEL_ID_CMN0;
		QpllClkMin = (u32) XHDMIPHY1_HDMI_GTYE4_QPLL0_REFCLK_MIN;
	}
#endif

	/* Update QPLL clock selections. */
	XHdmiphy1_CfgSysClkDataSel(InstancePtr, QuadId, Dir, SysClkDataSel);
	XHdmiphy1_CfgSysClkOutSel(InstancePtr, QuadId, Dir, SysClkOutSel);

	/* RX is using QPLL. */
	if (Dir == XHDMIPHY1_DIR_RX) {
		/* Check if the reference clock is not below the minimum QPLL
		 * input frequency. */
		if (QpllRefClk >= QpllClkMin) {
			RefClk = QpllRefClk;

			/* Scaled line rate. */
			if (InstancePtr->RxHdmi21Cfg.IsEnabled == 0) {
				if (InstancePtr->HdmiRxTmdsClockRatio) {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						XHDMIPHY1_CHANNEL_ID_CMNA, (RefClk * 40));
				}
				else {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						XHDMIPHY1_CHANNEL_ID_CMNA, (RefClk * 10));
				}
			} else { /* InstancePtr->RxHdmi21Cfg.IsEnabled == 1 */
				XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						XHDMIPHY1_CHANNEL_ID_CMNA,
						InstancePtr->RxHdmi21Cfg.LineRate);
			}

			/* Clear DRU is enabled flag. */
			InstancePtr->HdmiRxDruIsEnabled = 0;

			/* Set RX data width. */
			XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
					&Id0, &Id1);
			for (Id = Id0; Id <= Id1; Id++) {
				if (InstancePtr->Config.TransceiverWidth == 2) {
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxDataWidth = 20;
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxIntDataWidth = 2;
				}
				else {
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxDataWidth = 40;
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxIntDataWidth = 4;
				}
			}

		}
		/* The reference clock is below the minimum frequency thus
		 * select the DRU. */
		else if (InstancePtr->Config.DruIsPresent) {
			RefClk = XHdmiphy1_DruGetRefClkFreqHz(InstancePtr);
            /* Check DRU frequency */
			if (RefClk == XST_FAILURE) {
				XHdmiphy1_LogWrite(InstancePtr,
						XHDMIPHY1_LOG_EVT_DRU_CLK_ERR, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}


			/* Round input frequency to 10 kHz. */
			RefClk = (RefClk+5000) / 10000;
			RefClk = RefClk * 10000;

			/* Set the DRU to operate at a linerate of 2.5 Gbps. */
			XHdmiphy1_CfgLineRate(InstancePtr,
				QuadId, XHDMIPHY1_CHANNEL_ID_CMNA,
				(GetGtHdmiPtr(InstancePtr))->DruLineRate);

			/* Set DRU is enabled flag. */
			InstancePtr->HdmiRxDruIsEnabled = 1;

			/* Set RX data width to 40 and 4 bytes. */
			XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
					&Id0, &Id1);
			for (Id = Id0; Id <= Id1; Id++) {
				InstancePtr->Quads[QuadId].Plls[
					XHDMIPHY1_CH2IDX(Id)].RxDataWidth = 20;
				InstancePtr->Quads[QuadId].Plls[
					XHDMIPHY1_CH2IDX(Id)].RxIntDataWidth = 2;
			}
		}
		else {
			XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_NO_DRU, 1);
			XHdmiphy1_ErrorHandler(InstancePtr);
			return (XST_FAILURE);
		}
	}

	/* TX is using QPLL. */
	else {
		/* Set default TX sample rate. */
		InstancePtr->HdmiTxSampleRate = 1;

		if (InstancePtr->TxHdmi21Cfg.IsEnabled == 0) {
			/* Update TX line rates. */
			XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
                XHDMIPHY1_CHANNEL_ID_CMNA, (u64)((*RefClkPtr) * 10));
			TxLineRate = (*RefClkPtr) / 100000;;

			/* Check if the linerate is above the 340 Mcsc. */
			if ((TxLineRate) >= 3400) {
				(*RefClkPtr) = (*RefClkPtr) / 4;
			}
		} else { /* InstancePtr->TxHdmi21Cfg.IsEnabled == 1 */
			XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
					XHDMIPHY1_CHANNEL_ID_CMNA,
					InstancePtr->TxHdmi21Cfg.LineRate);
		}
	}

	/* Calculate QPLL values. */
	for (SRIndex = 0; SRIndex < sizeof(SRArray); SRIndex++) {
		/* Only use oversampling when then TX is using the QPLL. */
		if (Dir == XHDMIPHY1_DIR_TX) {
			SRValue = SRArray[SRIndex];

			if (InstancePtr->TxHdmi21Cfg.IsEnabled == 0) {
				/* TX reference clock is below the minimum QPLL clock
				 * input frequency. */
				if ((*RefClkPtr) < QpllClkMin) {
					RefClk = ((*RefClkPtr) * SRValue);

					/* Calculate scaled line rate. */
					if (TxLineRate >= 3400) {
						XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
							XHDMIPHY1_CHANNEL_ID_CMNA,
							(u64)(RefClk * 40));
					}
					else {
						XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
							XHDMIPHY1_CHANNEL_ID_CMNA,
							(u64)(RefClk * 10));
					}
				}
				/* TX reference clock is in QPLL clock input range.
				 * In this case don't increase the reference clock, but
				 * increase the line rate. */
				else {
					RefClk = (*RefClkPtr);

					/* Calculate scaled line rate. */
					if (TxLineRate >= 3400) {
						XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
							XHDMIPHY1_CHANNEL_ID_CMNA,
							(u64)(RefClk * 40 *SRValue));
					}

					else {
						XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
							XHDMIPHY1_CHANNEL_ID_CMNA,
							(u64)(RefClk * 10 *SRValue));
					}
				}
			}
			else { /* InstancePtr->TxHdmi21Cfg.IsEnabled == 1 */
				RefClk = (*RefClkPtr);
			}
		}
		/* For all other reference clocks force sample rate to one. */
		else {
			SRValue = 1;
		}

		Status = XHdmiphy1_ClkCalcParams(InstancePtr, QuadId, ActiveCmnId,
						Dir, RefClk);
		if (Status == (XST_SUCCESS)) {
			/* Only execute when the TX is using the QPLL. */
			if (Dir == XHDMIPHY1_DIR_TX) {
				/* Set TX sample rate. */
				InstancePtr->HdmiTxSampleRate = SRValue;

				/* Update reference clock only when the
				 * reference clock is below the minimum QPLL
				 * input frequency. */
				if ((*RefClkPtr) < QpllClkMin) {
					(*RefClkPtr) = (*RefClkPtr) * SRValue;
				}
				else if (SRValue > 1) {
					XHdmiphy1_LogWrite(InstancePtr,
							XHDMIPHY1_LOG_EVT_GT_QPLL_CFG_ERR, 1);
					XHdmiphy1_ErrorHandler(InstancePtr);
					return (XST_FAILURE);
				}
			}

			/* Check Userclock Frequency */
            /* (300 MHz + 0.5%) + 10 KHz (Clkdet accuracy) */
			if (301500000 <
					(XHdmiphy1_GetLineRateHz(InstancePtr, QuadId,
							ActiveCmnId) /
					(InstancePtr->Config.TransceiverWidth * 10))) {
				XHdmiphy1_LogWrite(InstancePtr,
						XHDMIPHY1_LOG_EVT_USRCLK_ERR, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}

			return (XST_SUCCESS);
		}
	}
	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_GT_QPLL_CFG_ERR, 1);
	XHdmiphy1_ErrorHandler(InstancePtr);
	return (XST_FAILURE);
}

/*****************************************************************************/
/**
* This function calculates the CPLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if calculated CPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiCpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status;
	u64 RefClk = 0;
	u32 *RefClkPtr;
	u32 TxLineRate = 0;
	XHdmiphy1_ChannelId ChannelId = XHDMIPHY1_CHANNEL_ID_CHA;
	u8 Id, Id0, Id1;

	u8 SRArray[] = {1, 3, 5};
	u8 SRIndex;
	u8 SRValue;

	/* Suppress Warning Messages */
	ChId = ChId;

	/* TX is using CPLL. */
	if (Dir == XHDMIPHY1_DIR_TX) {

		/* Set default TX sample rate. */
		InstancePtr->HdmiTxSampleRate = 1;

		/* Set line rate.  */
		RefClkPtr = &InstancePtr->HdmiTxRefClkHz;
		if (InstancePtr->TxHdmi21Cfg.IsEnabled == 0) {
			XHdmiphy1_CfgLineRate(InstancePtr, QuadId, ChannelId,
					(u64)((*RefClkPtr) * 10));
			TxLineRate = (*RefClkPtr)  / 100000;

			/* Check if the line rate is above the 340 Mcsc. */
			if (TxLineRate >= 3400) {
				(*RefClkPtr) = (*RefClkPtr) / 4;
			}
		} else { /* InstancePtr->TxHdmi21Cfg.IsEnabled == 1 */
			XHdmiphy1_CfgLineRate(InstancePtr, QuadId, ChannelId,
					(u64)(InstancePtr->TxHdmi21Cfg.LineRate));
			TxLineRate = InstancePtr->TxHdmi21Cfg.LineRate / 100000;
		}
	}
	/* RX is using CPLL. */
	else {
		RefClkPtr = &InstancePtr->HdmiRxRefClkHz;

		/* Check if the reference clock is not below the minimum CPLL
		 * input frequency. */
		if ((*RefClkPtr) >=
				(GetGtHdmiPtr(InstancePtr))->CpllRefClkMin) {
			RefClk = (*RefClkPtr);

			/* Scaled linerate */
			if (InstancePtr->RxHdmi21Cfg.IsEnabled == 0) {
				if (InstancePtr->HdmiRxTmdsClockRatio) {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						ChannelId, (RefClk * 40));
				}
				else {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						ChannelId, (RefClk * 10));
				}
			} else { /* InstancePtr->RxHdmi21Cfg.IsEnabled == 1 */
				XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
					ChannelId, InstancePtr->RxHdmi21Cfg.LineRate);
			}

			/* Clear DRU is enabled flag. */
			InstancePtr->HdmiRxDruIsEnabled = 0;

			/* Set RX data width. */
			XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
					&Id0, &Id1);
			for (Id = Id0; Id <= Id1; Id++) {
				if (InstancePtr->Config.TransceiverWidth == 2) {
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxDataWidth = 20;
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxIntDataWidth = 2;
				}
				else {
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxDataWidth = 40;
					InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(Id)].
						RxIntDataWidth = 4;
				}
			}

		}
		/* The reference clock is below the minimum frequency thus
		 * select the DRU. */
		else {
			if (InstancePtr->Config.DruIsPresent) {
				/* Return config not found error when TMDS ratio is 1/40 */
                if (InstancePtr->HdmiRxTmdsClockRatio) {
                    XHdmiphy1_LogWrite(InstancePtr,
                        XHDMIPHY1_LOG_EVT_GT_CPLL_CFG_ERR, 1);
                    XHdmiphy1_ErrorHandler(InstancePtr);
                    return (XST_FAILURE);
                }

                RefClk = XHdmiphy1_DruGetRefClkFreqHz(InstancePtr);
                /* Check DRU frequency */
			if (RefClk == XST_FAILURE) {
				XHdmiphy1_LogWrite(InstancePtr,
						XHDMIPHY1_LOG_EVT_DRU_CLK_ERR, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}

				/* Round input frequency to 10 kHz. */
				RefClk = (RefClk+5000) / 10000;
				RefClk = RefClk * 10000;

				/* Set the DRU to operate at a linerate of
				 * 2.5 Gbps. */
				XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						ChannelId,
						(GetGtHdmiPtr(InstancePtr))->
						DruLineRate);

				/* Set DRU is enabled flag. */
				InstancePtr->HdmiRxDruIsEnabled = 1;

				/* Set RX data width. */
				XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
						&Id0, &Id1);
				for (Id = Id0; Id <= Id1; Id++) {
					InstancePtr->Quads[QuadId].Plls[
						XHDMIPHY1_CH2IDX(Id)].
						RxDataWidth = 20;
					InstancePtr->Quads[QuadId].Plls[
						XHDMIPHY1_CH2IDX(Id)].
						RxIntDataWidth = 2;
				}

				if (TxLineRate > (((GetGtHdmiPtr(InstancePtr))
						->DruLineRate) / 1000000)) {
					XHdmiphy1_LogWrite(InstancePtr,
							XHDMIPHY1_LOG_EVT_VD_NOT_SPRTD_ERR, 1);
                    XHdmiphy1_ErrorHandler(InstancePtr);
                    return (XST_FAILURE);
				}
			}
			else {
				/* Return config not found error when TMDS ratio is 1/40 */
                if (InstancePtr->HdmiRxTmdsClockRatio) {
                    XHdmiphy1_LogWrite(InstancePtr,
                        XHDMIPHY1_LOG_EVT_GT_CPLL_CFG_ERR, 1);
                    XHdmiphy1_ErrorHandler(InstancePtr);
                }
                else {
					XHdmiphy1_LogWrite(InstancePtr,
							XHDMIPHY1_LOG_EVT_NO_DRU, 1);
					XHdmiphy1_ErrorHandler(InstancePtr);
                }
                return (XST_FAILURE);
			}
		}
	}

	/* Try different sample rates. */
	for (SRIndex = 0; SRIndex < sizeof(SRArray); SRIndex++) {
		/* Only use oversampling when then TX is using the CPLL. */
		if (Dir == XHDMIPHY1_DIR_TX) {
			SRValue = SRArray[SRIndex];

			if (InstancePtr->TxHdmi21Cfg.IsEnabled == 0) {
				/* Multiply the reference clock with the sample rate
				 * value. */
				RefClk = ((*RefClkPtr) * SRValue);

				/* Calculate scaled line rate. */
				if (TxLineRate >= 3400) {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						ChannelId, (RefClk * 40));
				}
				else {
					XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
						ChannelId, (RefClk * 10));
				}
			}
			else { /* InstancePtr->TxHdmi21Cfg.IsEnabled == 1 */
				RefClk = (*RefClkPtr);
			}
		}
		/* For all other reference clocks force sample rate to one. */
		else {
			SRValue = 1;
		}

		Status = XHdmiphy1_ClkCalcParams(InstancePtr, QuadId,
					ChannelId, Dir, RefClk);
		if (Status == (XST_SUCCESS)) {
			/* Only execute when the TX is using the QPLL. */
			if (Dir == XHDMIPHY1_DIR_TX) {
				InstancePtr->HdmiTxSampleRate = SRValue;
				(*RefClkPtr) = (*RefClkPtr) * SRValue;
			}

			/* Check Userclock Frequency */
            /* (300 MHz + 0.5%) + 10 KHz (Clkdet accuracy) */
			if (301500000 <	(XHdmiphy1_GetLineRateHz(InstancePtr, QuadId,
							XHDMIPHY1_CHANNEL_ID_CH1) /
					(InstancePtr->Config.TransceiverWidth * 10))) {
				XHdmiphy1_LogWrite(InstancePtr,
						XHDMIPHY1_LOG_EVT_USRCLK_ERR, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}

			return (XST_SUCCESS);
		}
	}

	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_GT_CPLL_CFG_ERR, 1);
	XHdmiphy1_ErrorHandler(InstancePtr);
	return (XST_FAILURE);
}
#endif

/*****************************************************************************/
/**
* This function sets the MMCM programming values based on Max Rate
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return       None
*
* @note         Based on the MAX Rate set in the GUI, the MMCM values are
*               selected.
*               3  -> Max Rate selected in GUI is 3G
*               6  -> Max Rate selected in GUI is 6G
*               8  -> Max Rate selected in GUI is 8G
*               10 -> Max Rate selected in GUI is 10G
*               12 -> Max Rate selected in GUI in 12G
*
******************************************************************************/
static void XHdmiphy1_MmcmParam(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir)
{
	u8 MaxRate;
	u8 NumChannels;
	XHdmiphy1_Mmcm *MmcmPtr;

	if (Dir == XHDMIPHY1_DIR_RX) {
		MmcmPtr = &InstancePtr->Quads[QuadId].RxMmcm;
		MaxRate = InstancePtr->Config.RxMaxRate;
		NumChannels = InstancePtr->Config.RxChannels;
	}
	else {
		MmcmPtr = &InstancePtr->Quads[QuadId].TxMmcm;
		MaxRate = InstancePtr->Config.TxMaxRate;
		NumChannels = InstancePtr->Config.TxChannels;
	}

	switch (MaxRate) {
	case 3:
		MmcmPtr->ClkFbOutMult =
			XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT3x3;
		MmcmPtr->DivClkDivide =
			XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK3x3;
		MmcmPtr->ClkOut0Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV3x3;
		MmcmPtr->ClkOut1Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV3x3;
		MmcmPtr->ClkOut2Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV3x3;
		break;
	case 6:
		if (NumChannels == 3) {
			MmcmPtr->ClkFbOutMult =
				XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6x3;
			MmcmPtr->DivClkDivide =
				XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6x3;
			MmcmPtr->ClkOut0Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6x3;
			MmcmPtr->ClkOut1Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6x3;
			MmcmPtr->ClkOut2Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6x3;
		} else {
			MmcmPtr->ClkFbOutMult =
				XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT6;
			MmcmPtr->DivClkDivide =
				XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK6;
			MmcmPtr->ClkOut0Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV6;
			MmcmPtr->ClkOut1Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV6;
			MmcmPtr->ClkOut2Div =
				XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV6;
		}
		break;
	case 8:
		MmcmPtr->ClkFbOutMult =
			XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT8;
		MmcmPtr->DivClkDivide =
			XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK8;
		MmcmPtr->ClkOut0Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV8;
		MmcmPtr->ClkOut1Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV8;
		MmcmPtr->ClkOut2Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV8;
		break;
	case 10:
		MmcmPtr->ClkFbOutMult =
			XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT10;
		MmcmPtr->DivClkDivide =
			XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK10;
		MmcmPtr->ClkOut0Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV10;
		MmcmPtr->ClkOut1Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV10;
		MmcmPtr->ClkOut2Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV10;
		break;
	case 12:
		MmcmPtr->ClkFbOutMult =
			XHDMIPHY1_FRL_VIDCLK_MMCM_FBOUTMULT;
		MmcmPtr->DivClkDivide =
			XHDMIPHY1_FRL_VIDCLK_MMCM_DIVCLK;
		MmcmPtr->ClkOut0Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT0DIV;
		MmcmPtr->ClkOut1Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT1DIV;
		MmcmPtr->ClkOut2Div =
			XHDMIPHY1_FRL_VIDCLK_MMCM_CLKOUT2DIV;
		break;
	default:
		break;
	}
}

/*****************************************************************************/
/**
* This function update/set the HDMI TX parameter.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Ppc is the pixels per clock to set.
* @param	Bpc is the bits per color to set.
* @param	ColorFormat is the color format to set.
*
* @return
*		- XST_SUCCESS if TX parameters set/updated.
*		- XST_FAILURE if low resolution video not supported.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_SetHdmiTxParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId,
		XVidC_PixelsPerClock Ppc, XVidC_ColorDepth Bpc,
		XVidC_ColorFormat ColorFormat)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Ppc == (XVIDC_PPC_1)) || (Ppc == (XVIDC_PPC_2)) ||
			(Ppc == (XVIDC_PPC_4)));
	Xil_AssertNonvoid((Bpc == (XVIDC_BPC_8)) || (Bpc == (XVIDC_BPC_10)) ||
			(Bpc == (XVIDC_BPC_12)) || (Bpc == (XVIDC_BPC_16)));
	Xil_AssertNonvoid((ColorFormat == (XVIDC_CSF_RGB)) ||
			(ColorFormat == (XVIDC_CSF_YCRCB_444)) ||
			(ColorFormat == (XVIDC_CSF_YCRCB_422)) ||
			(ColorFormat == (XVIDC_CSF_YCRCB_420)));

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	/* Only calculate the QPLL/CPLL parameters when the GT TX and RX are not
	 * coupled. */
    if (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
        Status = XHdmiphy1_HdmiCpllParam(InstancePtr, QuadId, ChId,
                XHDMIPHY1_DIR_TX);
    }
    else {
        Status = XHdmiphy1_HdmiQpllParam(InstancePtr, QuadId, ChId,
                XHDMIPHY1_DIR_TX);
        /* Update SysClk and PLL Clk registers immediately. */
        XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, QuadId);
    }
#else
    Status = XHdmiphy1_HdmiTxPllParam(InstancePtr, QuadId, ChId);
#endif

    if (Status == XST_FAILURE) {
        return Status;
    }

	/* Is HDMITXSS PPC match with HDMIPHY PPC? */
	if (Ppc == InstancePtr->Config.Ppc) {
		Status = (XST_SUCCESS);
	}
	else {
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_PPC_MSMTCH_ERR, 1);
		XHdmiphy1_ErrorHandler(InstancePtr);
		Status = (XST_FAILURE);
	}
	if (Status == (XST_SUCCESS)) {
		/* HDMI 2.1 */
		if (InstancePtr->TxHdmi21Cfg.IsEnabled) {
			XHdmiphy1_MmcmParam(InstancePtr, QuadId, XHDMIPHY1_DIR_TX);
			return Status;
		}

		/* Calculate TXPLL parameters.
		 * In HDMI the colordepth in YUV422 is always 12 bits,
		 * although on the link itself it is being transmitted as
		 * 8-bits. Therefore if the colorspace is YUV422, then force the
		 * colordepth to 8 bits. */
		if (ColorFormat == XVIDC_CSF_YCRCB_422) {
			Status = XHdmiphy1_HdmiCfgCalcMmcmParam(InstancePtr, QuadId,
                        ChId, XHDMIPHY1_DIR_TX, Ppc, XVIDC_BPC_8);
		}
		/* Other colorspaces. */
		else {
			Status = XHdmiphy1_HdmiCfgCalcMmcmParam(InstancePtr, QuadId,
                        ChId, XHDMIPHY1_DIR_TX, Ppc, Bpc);
		}
	}
	else {
		Status = (XST_FAILURE);
	}
	return Status;
}

/*****************************************************************************/
/**
* This function update/set the HDMI RX parameter.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if RX parameters set/updated.
*		- XST_FAILURE if low resolution video not supported.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_SetHdmiRxParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	XHdmiphy1_ChannelId ChanId = ChId;
	XHdmiphy1_PllType PllType;
	u32 Status;
	u64 DruCenterFreq;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	if (XHdmiphy1_IsRxUsingCpll(InstancePtr, QuadId, ChId)) {
		Status = XHdmiphy1_HdmiCpllParam(InstancePtr, QuadId, ChId,
                    XHDMIPHY1_DIR_RX);
	}
	else {
        Status = XHdmiphy1_HdmiQpllParam(InstancePtr, QuadId, ChId,
                    XHDMIPHY1_DIR_RX);
		/* Update SysClk and PLL Clk registers immediately */
		XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, QuadId);
	}
#else
        Status = XHdmiphy1_HdmiRxPllParam(InstancePtr, QuadId, ChId);
#endif

	if (InstancePtr->HdmiRxDruIsEnabled) {
		/* Determine PLL type. */
		PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
                    XHDMIPHY1_CHANNEL_ID_CH1);
		/* Update the ChId */
		ChanId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
                    XHDMIPHY1_DIR_RX, PllType);

		DruCenterFreq = XHdmiphy1_DruCalcCenterFreqHz(InstancePtr, QuadId,
					ChanId);
		XHdmiphy1_DruSetCenterFreqHz(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
						DruCenterFreq);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function enables or disables the Pattern Generator for the GT Channel 4
* when it isused to generate the TX TMDS Clock.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Enable TRUE/FALSE
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_PatgenEnable(XHdmiphy1 *InstancePtr, u8 QuadId, u8 Enable)
{
	u32 RegVal;

	/* Suppress Warning Messages */
    QuadId = QuadId;

	RegVal = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
							XHDMIPHY1_PATGEN_CTRL_REG);

	if (Enable) {
		RegVal |= XHDMIPHY1_PATGEN_CTRL_ENABLE_MASK;
	}
	else {
		RegVal &= ~XHDMIPHY1_PATGEN_CTRL_ENABLE_MASK;
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
						XHDMIPHY1_PATGEN_CTRL_REG, RegVal);
}

/*****************************************************************************/
/**
* This function sets the Pattern Generator for the GT Channel 4 when it is
* used to generate the TX TMDS Clock.
*
* @param	InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	TxLineRate in Mbps.
*
* @return	None.
*
******************************************************************************/
void XHdmiphy1_PatgenSetRatio(XHdmiphy1 *InstancePtr, u8 QuadId,
        u64 TxLineRate)
{
	u32 RegVal;

	/* Suppress Warning Messages */
    QuadId = QuadId;

	RegVal = (XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_PATGEN_CTRL_REG)
				& ~XHDMIPHY1_PATGEN_CTRL_RATIO_MASK);

	if ((TxLineRate >= 3400) && (InstancePtr->HdmiTxSampleRate == 1)) {
		RegVal |= XHDMIPHY1_Patgen_Ratio_40 &
                        XHDMIPHY1_PATGEN_CTRL_RATIO_MASK;
	}
	else {
		RegVal |= InstancePtr->HdmiTxSampleRate &
						XHDMIPHY1_PATGEN_CTRL_RATIO_MASK;
	}

	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr,
						XHDMIPHY1_PATGEN_CTRL_REG, RegVal);
}

/*****************************************************************************/
/**
* This function will configure the HDMIPHY to HDMI 2.0 mode
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if TX parameters set/updated.
*		- XST_FAILURE if low resolution video not supported.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_Hdmi20Config(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir)
{
	XHdmiphy1_PllType PllType;
	u32 Status = XST_SUCCESS;

	/* Suppress Warning Messages */
    QuadId = QuadId;

	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TMDS_RECONFIG, Dir);

	/* Determine PLL type. */
	PllType = XHdmiphy1_GetPllType(InstancePtr, 0, Dir,
                XHDMIPHY1_CHANNEL_ID_CH1);

	/* Update HDMI Configurations */
	if (Dir == XHDMIPHY1_DIR_TX) {
		InstancePtr->TxHdmi21Cfg.LineRate = 0;
		InstancePtr->TxHdmi21Cfg.NChannels = 0;
		InstancePtr->TxHdmi21Cfg.IsEnabled = FALSE;
	} else {
		InstancePtr->RxHdmi21Cfg.LineRate = 0;
		InstancePtr->RxHdmi21Cfg.NChannels = 0;
		InstancePtr->RxHdmi21Cfg.IsEnabled = FALSE;
	}

	XHdmiphy1_MmcmSetClkinsel(InstancePtr, QuadId, Dir,
			XHDMIPHY1_MMCM_CLKINSEL_CLKIN1);

	/* Disable Clock Detector Interrupts */
	if (Dir == XHDMIPHY1_DIR_TX) {
		XHdmiphy1_IntrEnable(InstancePtr,
				XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE);
	} else {
		XHdmiphy1_IntrEnable(InstancePtr,
				XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE);
	}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	/* Update reference clock election. */
	XHdmiphy1_CfgPllRefClkSel(InstancePtr, 0,
		((PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
			XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA),
		((Dir == XHDMIPHY1_DIR_TX) ?
			InstancePtr->Config.TxRefClkSel :
			InstancePtr->Config.RxRefClkSel));

	/* Update RefClk selection. */
	XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, 0);
#else
	/* Suppress Warning Messages */
	PllType = PllType;
#endif

	return Status;
}

/*****************************************************************************/
/**
* This function will configure the FRL REF CLK for HDMI 2.1 operation
*
*
* @return
*		- FRL Ref Clk value
*
* @note		None.
*
******************************************************************************/
u32 Xhdmiphy1_RefClkValue() {

	u32 frl_refclk;
	char SpeedGrade[5] = XPAR_HDMIPHY1_0_SPEEDGRADE_STR;
	char CompVal[5] = "-1";
	char *SpeedGradePtr = &SpeedGrade[0];
	char *CompValPtr = &CompVal[0];

	if (strncmp(SpeedGradePtr, CompValPtr, 2) == 0) {
		frl_refclk = 200000000U;
	} else {
		frl_refclk = 400000000U;
	}

	return frl_refclk;
}

/*****************************************************************************/
/**
* This function will configure the GT for HDMI 2.1 operation
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	Dir is an indicator for RX or TX.
* @param	LineRate is the line rate for HDMI 2.1 operation.
* @param	NChannels is the number of channels for HDMI 2.1 operation.
*
* @return
*		- XST_SUCCESS if TX parameters set/updated.
*		- XST_FAILURE if low resolution video not supported.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_Hdmi21Config(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u64 LineRate, u8 NChannels)
{
	XHdmiphy1_PllType PllType = XHDMIPHY1_PLL_TYPE_UNKNOWN;
	u32 Status = XST_SUCCESS;
	u32 Hdmi21_frl_refclk;

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4) || \
	(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	char SpeedGrade[5] = XPAR_HDMIPHY1_0_SPEEDGRADE_STR;
	char CompVal[5] = "-1";
	char *SpeedGradePtr = &SpeedGrade[0];
	char *CompValPtr = &CompVal[0];


	/* Determine PLL type. */
	PllType = XHdmiphy1_GetPllType(InstancePtr, 0, Dir,
                XHDMIPHY1_CHANNEL_ID_CH1);

	/* Look for -1 Parts. Max Line rate is 8Gbps */
	if (strncmp(SpeedGradePtr, CompValPtr, 2) == 0) {
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
		if (PllType == (XHDMIPHY1_PLL_TYPE_QPLL0 || XHDMIPHY1_PLL_TYPE_QPLL1)) {
		if (LineRate > 10e9) {
			XHdmiphy1_LogWrite(InstancePtr,
                XHDMIPHY1_LOG_EVT_SPDGRDE_ERR, Dir);
			XHdmiphy1_ErrorHandler(InstancePtr);
			return XST_FAILURE;
		}
		} else {
		if (LineRate > 8e9) {
			XHdmiphy1_LogWrite(InstancePtr,
                XHDMIPHY1_LOG_EVT_SPDGRDE_ERR, Dir);
			XHdmiphy1_ErrorHandler(InstancePtr);
			return XST_FAILURE;
		}
		}
	}
#endif
#endif

	XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_FRL_RECONFIG, Dir);

	/* Disable Clock Detector Interrupts */
	if (Dir == XHDMIPHY1_DIR_TX) {
		if (InstancePtr->Config.TxRefClkSel !=
				InstancePtr->Config.TxFrlRefClkSel) {
			XHdmiphy1_IntrDisable(InstancePtr,
					XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE);
		}
		/* Enable 4th Channel output */
		XHdmiphy1_Clkout1OBufTdsEnable(InstancePtr, XHDMIPHY1_DIR_TX, (TRUE));
	} else {
		if (InstancePtr->Config.RxRefClkSel !=
						InstancePtr->Config.RxFrlRefClkSel) {
			XHdmiphy1_IntrDisable(InstancePtr,
					XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE);
		}
	}

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	/* Update reference clock election. */
	XHdmiphy1_CfgPllRefClkSel(InstancePtr, 0,
		((PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
			XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA),
		((Dir == XHDMIPHY1_DIR_TX) ?
			InstancePtr->Config.TxFrlRefClkSel :
			InstancePtr->Config.RxFrlRefClkSel));

	/* Update RefClk selection. */
	XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, 0);
#endif

	Hdmi21_frl_refclk = Xhdmiphy1_RefClkValue();

	/* Update HDMI Configurations */
	if (Dir == XHDMIPHY1_DIR_TX) {
		InstancePtr->HdmiTxRefClkHz = Hdmi21_frl_refclk;
		InstancePtr->TxHdmi21Cfg.LineRate = LineRate;
		InstancePtr->TxHdmi21Cfg.NChannels = NChannels;
		InstancePtr->TxHdmi21Cfg.IsEnabled = TRUE;

		Status = XHdmiphy1_SetHdmiTxParam(InstancePtr, 0,
				((PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
				XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA),
				InstancePtr->Config.Ppc,
				XVIDC_BPC_8,
				XVIDC_CSF_RGB);

		/* Mask the MMCM Lock */
		XHdmiphy1_MmcmLockedMaskEnable(InstancePtr, 0, Dir, TRUE);

		if (InstancePtr->Config.TxRefClkSel ==
				InstancePtr->Config.TxFrlRefClkSel) {
			XHdmiphy1_MmcmSetClkinsel(InstancePtr, QuadId, Dir,
					XHDMIPHY1_MMCM_CLKINSEL_CLKIN1);
		} else {
			XHdmiphy1_MmcmSetClkinsel(InstancePtr, QuadId, Dir,
					XHDMIPHY1_MMCM_CLKINSEL_CLKIN2);
		}
		if (InstancePtr->Config.TxRefClkSel !=
				InstancePtr->Config.TxFrlRefClkSel) {
			XHdmiphy1_HdmiTxTimerTimeoutHandler(InstancePtr);
		}
	} else {
		InstancePtr->HdmiRxRefClkHz = Hdmi21_frl_refclk;
		InstancePtr->RxHdmi21Cfg.LineRate = LineRate;
		InstancePtr->RxHdmi21Cfg.NChannels = NChannels;
		InstancePtr->RxHdmi21Cfg.IsEnabled = TRUE;

		/* Set MMCM dividers for FRL mode */
		XHdmiphy1_MmcmParam(InstancePtr, QuadId, XHDMIPHY1_DIR_RX);

		/* Mask the MMCM Lock */
		XHdmiphy1_MmcmLockedMaskEnable(InstancePtr, 0, Dir, TRUE);

		if (InstancePtr->Config.RxRefClkSel !=
				InstancePtr->Config.RxFrlRefClkSel) {
			/* Set MMCM CLKINSEL to CLK2 */
			XHdmiphy1_MmcmSetClkinsel(InstancePtr, QuadId, Dir,
					XHDMIPHY1_MMCM_CLKINSEL_CLKIN2);

			/* Start RX MMCM. */
			XHdmiphy1_MmcmStart(InstancePtr, 0, XHDMIPHY1_DIR_RX);

			XHdmiphy1_HdmiRxTimerTimeoutHandler(InstancePtr);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
* This function prints Video PHY debug information related to HDMI.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XHdmiphy1_HdmiDebugInfo(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u32 RegValue;
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)&&(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
	XHdmiphy1_Channel *ChPtr;
	XHdmiphy1_ChannelId CmnId = XHDMIPHY1_CHANNEL_ID_CMN0;
	u8 CpllDVal;
	u8 QpllDVal;
	u8 UsesQpll0 = 0;

	ChPtr = &InstancePtr->Quads[QuadId].Plls[0];

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		if (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
			xil_printf("TX: CPLL\r\n");
		}
		else {
			if ((ChPtr->TxDataRefClkSel ==
					XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) ||
				(ChPtr->TxDataRefClkSel ==
					XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK)) {
				UsesQpll0 = (TRUE);
				CmnId = XHDMIPHY1_CHANNEL_ID_CMN0;
			}
			else {
				UsesQpll0 = (FALSE);
				CmnId = XHDMIPHY1_CHANNEL_ID_CMN1;
			}
			xil_printf("TX: QPLL%d\r\n", (UsesQpll0 ? 0 : 1));
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		if (XHdmiphy1_IsRxUsingCpll(InstancePtr, QuadId, ChId)) {
			xil_printf("RX: CPLL\r\n");
		}
		else {
			if ((ChPtr->RxDataRefClkSel ==
					XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) ||
				(ChPtr->RxDataRefClkSel ==
					XHDMIPHY1_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK)) {
				UsesQpll0 = (TRUE);
				CmnId = XHDMIPHY1_CHANNEL_ID_CMN0;
			}
			else {
				UsesQpll0 = (FALSE);
				CmnId = XHDMIPHY1_CHANNEL_ID_CMN1;
			}
			xil_printf("RX: QPLL%d\r\n", (UsesQpll0 ? 0 : 1));
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		xil_printf("TX state: ");
		switch (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].TxState) {
		case (XHDMIPHY1_GT_STATE_IDLE):
			xil_printf("idle\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_LOCK):
			if (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
				xil_printf("CPLL lock\r\n");
			}
			else {
				xil_printf("QPLL%d lock\r\n", (UsesQpll0 ? 0 : 1));
			}
			break;
		case (XHDMIPHY1_GT_STATE_RESET):
			xil_printf("GT reset\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_ALIGN):
			xil_printf("align\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_READY):
			xil_printf("ready\r\n");
			break;
		default:
			xil_printf("unknown\r\n");
			break;
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		xil_printf("RX state: ");
		switch (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].RxState) {
		case (XHDMIPHY1_GT_STATE_IDLE):
			xil_printf("idle\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_LOCK):
			if (XHdmiphy1_IsRxUsingCpll(InstancePtr, QuadId, ChId)) {
				xil_printf("CPLL lock\r\n");
			}
			else {
				xil_printf("QPLL%d lock\r\n", (UsesQpll0 ? 0 : 1));
			}
			break;
		case (XHDMIPHY1_GT_STATE_RESET):
			xil_printf("GT reset\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_READY):
			xil_printf("ready\r\n");
			break;
		default:
			xil_printf("unknown\r\n");
			break;
		}
	}

	if (XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
		QpllDVal = ChPtr->RxOutDiv;
		CpllDVal = ChPtr->TxOutDiv;
	}
	else {
		CpllDVal = ChPtr->RxOutDiv;
		QpllDVal = ChPtr->TxOutDiv;
	}

	xil_printf("\r\n");
    if ((XHdmiphy1_IsTxUsingQpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX))) ||
        (XHdmiphy1_IsRxUsingQpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))) {
        xil_printf("QPLL%d settings\r\n", (UsesQpll0 ? 0 : 1));
        xil_printf("-------------\r\n");
        xil_printf("M : %d - N : %d - D : %d\r\n",
            InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.MRefClkDiv,
            InstancePtr->Quads[QuadId].Plls[XHDMIPHY1_CH2IDX(CmnId)].
                PllParams.NFbDiv, QpllDVal);
        xil_printf("\r\n");
    }

    if ((XHdmiphy1_IsTxUsingCpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX))) ||
        (XHdmiphy1_IsRxUsingCpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))) {
        xil_printf("CPLL settings\r\n");
        xil_printf("-------------\r\n");
        xil_printf("M : %d - N1 : %d - N2 : %d - D : %d\r\n",
            ChPtr->PllParams.MRefClkDiv,
            ChPtr->PllParams.N1FbDiv, ChPtr->PllParams.N2FbDiv,
            CpllDVal);
        xil_printf("\r\n");
    }
#else

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		if (XHdmiphy1_IsTxUsingLcpll(InstancePtr, QuadId, ChId)) {
			xil_printf("TX: LCPLL\r\n");
		}
		else {
			xil_printf("TX: RPLL\r\n");
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		if (XHdmiphy1_IsRxUsingLcpll(InstancePtr, QuadId, ChId)) {
			xil_printf("RX: LCPLL\r\n");
		}
		else {
			xil_printf("RX: RPLL\r\n");
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		xil_printf("TX state: ");
		switch (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].TxState) {
		case (XHDMIPHY1_GT_STATE_IDLE):
			xil_printf("idle\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_GPO_RE):
			xil_printf("GPO Assert\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_LOCK):
			if (XHdmiphy1_IsTxUsingLcpll(InstancePtr, QuadId, ChId)) {
				xil_printf("LCPLL lock\r\n");
			}
			else {
				xil_printf("RPLL lock\r\n");
			}
			break;
		case (XHDMIPHY1_GT_STATE_RESET):
			xil_printf("GT reset\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_ALIGN):
			xil_printf("align\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_READY):
			xil_printf("ready\r\n");
			break;
		default:
			xil_printf("unknown\r\n");
			break;
		}
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		xil_printf("RX state: ");
		switch (InstancePtr->Quads[QuadId].
                    Plls[XHDMIPHY1_CH2IDX(ChId)].RxState) {
		case (XHDMIPHY1_GT_STATE_IDLE):
			xil_printf("idle\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_GPO_RE):
			xil_printf("GPO Assert\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_LOCK):
			if (XHdmiphy1_IsRxUsingLcpll(InstancePtr, QuadId, ChId)) {
				xil_printf("LCPLL lock\r\n");
			}
			else {
				xil_printf("RPLL lock\r\n");
			}
			break;
		case (XHDMIPHY1_GT_STATE_RESET):
			xil_printf("GT reset\r\n");
			break;
		case (XHDMIPHY1_GT_STATE_READY):
			xil_printf("ready\r\n");
			break;
		default:
			xil_printf("unknown\r\n");
			break;
		}
	}

	xil_printf("\r\n");
    if ((XHdmiphy1_IsTxUsingLcpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX))) ||
        (XHdmiphy1_IsRxUsingLcpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))) {
        xil_printf("LCPLL settings\r\n");
        xil_printf("-------------\r\n");
        xil_printf("LR CFG : %d\r\n",
            InstancePtr->Quads[QuadId].Lcpll.LineRateCfg);
        xil_printf("\r\n");
    }

    if ((XHdmiphy1_IsTxUsingRpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX))) ||
        (XHdmiphy1_IsRxUsingRpll(InstancePtr, QuadId, ChId) &&
            (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))) {
        xil_printf("RPLL settings\r\n");
        xil_printf("-------------\r\n");
        xil_printf("LR CFG : %d\r\n",
            InstancePtr->Quads[QuadId].Rpll.LineRateCfg);
        xil_printf("\r\n");
    }
#endif
	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)) {
		if (InstancePtr->Config.RxClkPrimitive == 0) {
			xil_printf("RX MMCM settings\r\n");
		} else {
			xil_printf("RX PLL settings\r\n");
		}
		xil_printf("-------------\r\n");
		xil_printf("Mult : %d - Div : %d - Clk0Div : %d - Clk1Div : %d - "
			   "Clk2Div : %d\r\n",
			InstancePtr->Quads[QuadId].RxMmcm.ClkFbOutMult,
			InstancePtr->Quads[QuadId].RxMmcm.DivClkDivide,
			InstancePtr->Quads[QuadId].RxMmcm.ClkOut0Div,
			InstancePtr->Quads[QuadId].RxMmcm.ClkOut1Div,
			InstancePtr->Quads[QuadId].RxMmcm.ClkOut2Div);
		xil_printf("\r\n");
	}

	if (XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_TX)) {
		if (InstancePtr->Config.TxClkPrimitive == 0) {
			xil_printf("TX MMCM settings\r\n");
		} else {
			xil_printf("TX PLL settings\r\n");
		}
		xil_printf("-------------\r\n");
		xil_printf("Mult : %d - Div : %d - Clk0Div : %d - Clk1Div : %d - "
			   "Clk2Div : %d\r\n",
			InstancePtr->Quads[QuadId].TxMmcm.ClkFbOutMult,
			InstancePtr->Quads[QuadId].TxMmcm.DivClkDivide,
			InstancePtr->Quads[QuadId].TxMmcm.ClkOut0Div,
			InstancePtr->Quads[QuadId].TxMmcm.ClkOut1Div,
			InstancePtr->Quads[QuadId].TxMmcm.ClkOut2Div);
		xil_printf("\r\n");
	}

	if ((InstancePtr->Config.DruIsPresent) &&
		(XHdmiphy1_IsHDMI(InstancePtr, XHDMIPHY1_DIR_RX)))	{
		xil_printf("DRU Settings\r\n");
		xil_printf("-------------\r\n");
		RegValue = XHdmiphy1_DruGetVersion(InstancePtr);
		xil_printf("Version  : %d\r\n", RegValue);

		if (InstancePtr->HdmiRxDruIsEnabled) {
			RegValue = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_DRU_GAIN_REG(ChId));

			xil_printf("G1       : %d\r\nG1_P     : %d\r\n"
				   "G2       : %d\r\n",
				((RegValue & XHDMIPHY1_DRU_GAIN_G1_MASK)),
				((RegValue & XHDMIPHY1_DRU_GAIN_G1_P_MASK) >>
					XHDMIPHY1_DRU_GAIN_G1_P_SHIFT),
				((RegValue & XHDMIPHY1_DRU_GAIN_G2_MASK) >>
					XHDMIPHY1_DRU_GAIN_G2_SHIFT));

			RegValue = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_DRU_CFREQ_H_REG(ChId));
			xil_printf("Center_F : %x", RegValue);

			RegValue = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
				XHDMIPHY1_DRU_CFREQ_L_REG(ChId));
			xil_printf("%x\r\n", RegValue);
		}
		else {
			xil_printf("DRU is disabled\r\n");
		}

		xil_printf(" \r\n");
	}
}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
static const XHdmiphy1_GtHdmiChars Gthe4HdmiChars = {
	.DruLineRate = XHDMIPHY1_HDMI_GTHE4_DRU_LRATE,
	.PllScale = XHDMIPHY1_HDMI_GTHE4_PLL_SCALE,
	.Qpll0RefClkMin = XHDMIPHY1_HDMI_GTHE4_QPLL0_REFCLK_MIN,
	.Qpll1RefClkMin = XHDMIPHY1_HDMI_GTHE4_QPLL1_REFCLK_MIN,
	.CpllRefClkMin = XHDMIPHY1_HDMI_GTHE4_CPLL_REFCLK_MIN,
	.TxMmcmScale = XHDMIPHY1_HDMI_GTHE4_TX_MMCM_SCALE,
	.TxMmcmFvcoMin = XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MIN,
	.TxMmcmFvcoMax = XHDMIPHY1_HDMI_GTHE4_TX_MMCM_FVCO_MAX,
	.RxMmcmScale = XHDMIPHY1_HDMI_GTHE4_RX_MMCM_SCALE,
	.RxMmcmFvcoMin = XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MIN,
	.RxMmcmFvcoMax = XHDMIPHY1_HDMI_GTHE4_RX_MMCM_FVCO_MAX,
};
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
static const XHdmiphy1_GtHdmiChars Gtye4HdmiChars = {
	.DruLineRate = XHDMIPHY1_HDMI_GTYE4_DRU_LRATE,
	.PllScale = XHDMIPHY1_HDMI_GTYE4_PLL_SCALE,
	.Qpll0RefClkMin = XHDMIPHY1_HDMI_GTYE4_QPLL0_REFCLK_MIN,
	.Qpll1RefClkMin = XHDMIPHY1_HDMI_GTYE4_QPLL1_REFCLK_MIN,
	.CpllRefClkMin = XHDMIPHY1_HDMI_GTYE4_CPLL_REFCLK_MIN,
	.TxMmcmScale = XHDMIPHY1_HDMI_GTYE4_TX_MMCM_SCALE,
	.TxMmcmFvcoMin = XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MIN,
	.TxMmcmFvcoMax = XHDMIPHY1_HDMI_GTYE4_TX_MMCM_FVCO_MAX,
	.RxMmcmScale = XHDMIPHY1_HDMI_GTYE4_RX_MMCM_SCALE,
	.RxMmcmFvcoMin = XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MIN,
	.RxMmcmFvcoMax = XHDMIPHY1_HDMI_GTYE4_RX_MMCM_FVCO_MAX,
};
#elif ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))
static const XHdmiphy1_GtHdmiChars Gtye5HdmiChars = {
	.DruLineRate = XHDMIPHY1_HDMI_GTYE5_DRU_LRATE,
	.PllScale = XHDMIPHY1_HDMI_GTYE5_PLL_SCALE,
	.Qpll0RefClkMin = XHDMIPHY1_HDMI_GTYE5_LCPLL_REFCLK_MIN,
	.Qpll1RefClkMin = XHDMIPHY1_HDMI_GTYE5_RPLL_REFCLK_MIN,
	.CpllRefClkMin = 0,
	.TxMmcmScale = XHDMIPHY1_HDMI_GTYE5_TX_MMCM_SCALE,
	.TxMmcmFvcoMin = XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MIN,
	.TxMmcmFvcoMax = XHDMIPHY1_HDMI_GTYE5_TX_MMCM_FVCO_MAX,
	.RxMmcmScale = XHDMIPHY1_HDMI_GTYE5_RX_MMCM_SCALE,
	.RxMmcmFvcoMin = XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MIN,
	.RxMmcmFvcoMax = XHDMIPHY1_HDMI_GTYE5_RX_MMCM_FVCO_MAX,
};
#endif

/*****************************************************************************/
/**
* This function returns a pointer to the HDMI parameters based on the GT type.
*
* @param	InstancePtr is a pointer to the Hdmiphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- A pointer to the HDMI GT characteristics.
*		- NULL if the GT type is unsupported.
*
* @note		None.
*
******************************************************************************/
static const XHdmiphy1_GtHdmiChars *GetGtHdmiPtr(XHdmiphy1 *InstancePtr)
{
	/* Suppress Warning Messages */
	InstancePtr = InstancePtr;

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
	return &Gthe4HdmiChars;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
	return &Gtye4HdmiChars;
#elif ((XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYP))
	return &Gtye5HdmiChars;
#endif

	return NULL;
}
