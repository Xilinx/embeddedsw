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
 * @file xvphy_dp.c
 *
 * This file contains video PHY functionality specific to the DisplayPort
 * protocol.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   vkd  10/19/15 Initial release.
 * 1.2   gm   08/26/16 Suppressed warning messages due to unused arguments
 * 1.4   gm   29/11/16 Added preprocessor directives for sw footprint reduction
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xparameters.h"
#if defined (XPAR_XDP_0_DEVICE_ID)
#include "xstatus.h"
#include "xvphy_dp.h"
#include "xvphy.h"
#include "xvphy_i.h"
#include "string.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function initializes the Video PHY for DisplayPort.
 *
 * @param	InstancePtr is a pointer to the XVphy instance.
 * @param	CfgPtr is a pointer to the configuration structure that will
 *		be used to copy the settings from.
 * @param	QuadId is the GT quad ID to operate on.
 * @param	CpllRefClkSel is the CPLL reference clock selection for the
 *		quad.
 * @param	QpllRefClkSel is the QPLL reference clock selection for the
 *		quad.
 * @param	TxPllSelect is the reference clock selection for the quad's
 *		TX PLL dividers.
 * @param	RxPllSelect is the reference clock selection for the quad's
 *		RX PLL dividers.
 * @param	LinkRate is the line rate to set for the quad's channels.
 *
 * @return
 *		- XST_SUCCESS.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XVphy_DpInitialize(XVphy *InstancePtr, XVphy_Config *CfgPtr, u8 QuadId,
		XVphy_PllRefClkSelType CpllRefClkSel,
		XVphy_PllRefClkSelType QpllRefClkSel,
		XVphy_PllType TxPllSelect, XVphy_PllType RxPllSelect,
		u8 LinkRate)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Suppress Warning Messages */
	LinkRate = LinkRate;

	/* Setup the instance */
	(void)memset((void *)InstancePtr, 0, sizeof(XVphy));
	XVphy_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddr);

	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_TXRESET_DONE);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_RXRESET_DONE);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_CPLL_LOCK);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_QPLL_LOCK);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_QPLL1_LOCK);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_TXALIGN_DONE);
	XVphy_IntrDisable(InstancePtr,
			XVPHY_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE);
	XVphy_IntrDisable(InstancePtr,
			XVPHY_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT);
	XVphy_IntrDisable(InstancePtr, XVPHY_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT);

	XVphy_LogReset(InstancePtr);

	XVphy_LogWrite(InstancePtr, (XVPHY_LOG_EVT_INIT), 0);

	InstancePtr->Quads[QuadId].Plls[0].TxState = (XVPHY_GT_STATE_IDLE);

	InstancePtr->Quads[QuadId].Plls[0].RxState = (XVPHY_GT_STATE_IDLE);

	switch (InstancePtr->Config.XcvrType) {
	case XVPHY_GT_TYPE_GTPE2:
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0,
			XVPHY_DP_REF_CLK_FREQ_HZ_135);
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1,
			XVPHY_DP_REF_CLK_FREQ_HZ_135);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH1,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH2,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH3,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH4,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
	 break;
	case XVPHY_GT_TYPE_GTXE2:
	case XVPHY_GT_TYPE_GTHE2:
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0,
			XVPHY_DP_REF_CLK_FREQ_HZ_135);
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1,
			XVPHY_DP_REF_CLK_FREQ_HZ_135);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH1,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH2,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH3,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH4,
			XVPHY_GTXE2_DIFF_SWING_DP_L1);
		break;
	case XVPHY_GT_TYPE_GTHE3:
	case XVPHY_GT_TYPE_GTHE4:
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK0,
			XVPHY_DP_REF_CLK_FREQ_HZ_162);
		XVphy_CfgQuadRefClkFreq(InstancePtr, QuadId,
			XVPHY_PLL_REFCLKSEL_TYPE_GTREFCLK1,
			XVPHY_DP_REF_CLK_FREQ_HZ_162);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH1, XVPHY_GTHE3_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH2, XVPHY_GTHE3_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH3, XVPHY_GTHE3_DIFF_SWING_DP_L1);
		XVphy_SetTxVoltageSwing(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH4, XVPHY_GTHE3_DIFF_SWING_DP_L1);
		XVphy_SetTxPreEmphasis(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH1, XVPHY_GTHE3_PREEMP_DP_L0);
		XVphy_SetTxPreEmphasis(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH2, XVPHY_GTHE3_PREEMP_DP_L0);
		XVphy_SetTxPreEmphasis(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH3, XVPHY_GTHE3_PREEMP_DP_L0);
		XVphy_SetTxPreEmphasis(InstancePtr, QuadId,
			XVPHY_CHANNEL_ID_CH4, XVPHY_GTHE3_PREEMP_DP_L0);
		break;
	default:
		break;
	}

	XVphy_Set8b10b(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,
			1);
	XVphy_Set8b10b(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
			1);
	XVphy_SetRxLpm(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,
			1);

	XVphy_PllInitialize(InstancePtr, QuadId, XVPHY_CHANNEL_ID_CHA,
			QpllRefClkSel, CpllRefClkSel, TxPllSelect, RxPllSelect);

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	XVphy_LogWrite(InstancePtr, (XVPHY_LOG_EVT_INIT), 1);

	return (XST_SUCCESS);

}

/******************************************************************************/
/**
* This function prints Vphy debug information on STDIO/Uart console.
*
* @param	InstancePtr is a pointer to the Vphy core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVphy_DpDebugInfo(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId)
{
	XVphy_Channel *ChPtr;
	XVphy_ChannelId CmnId = XVPHY_CHANNEL_ID_CMN0;
	u8 UsesQpll0 = 0;
	u8 CpllDVal;
	u8 QpllDVal;

	ChPtr = &InstancePtr->Quads[QuadId].Plls[0];

	if (XVphy_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
		xil_printf("TX => CPLL / ");
	}
	else if (ChPtr->TxDataRefClkSel ==
			XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK) {
		UsesQpll0 = (TRUE);
		CmnId = XVPHY_CHANNEL_ID_CMN0;
		xil_printf("TX => QPLL1, ");
	}
	else if (ChPtr->TxDataRefClkSel ==
			XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK) {
		UsesQpll0 = (FALSE);
		CmnId = XVPHY_CHANNEL_ID_CMN;
		xil_printf("TX => QPLL, ");
	}
	else {
		UsesQpll0 = (FALSE);
		CmnId = XVPHY_CHANNEL_ID_CMN1;
		xil_printf("TX => QPLL0, ");
	}

	if (XVphy_IsRxUsingCpll(InstancePtr, QuadId, ChId)) {
		xil_printf("RX => CPLL\n\r");
	}
	else if (ChPtr->RxDataRefClkSel ==
			XVPHY_SYSCLKSELDATA_TYPE_QPLL0_OUTCLK) {
		UsesQpll0 = (TRUE);
		CmnId = XVPHY_CHANNEL_ID_CMN0;
		xil_printf("RX => QPLL0, \n\r");
	}
	else if (ChPtr->RxDataRefClkSel ==
			XVPHY_SYSCLKSELDATA_TYPE_QPLL_OUTCLK){
		UsesQpll0 = (FALSE);
		CmnId = XVPHY_CHANNEL_ID_CMN;
		xil_printf("RX => QPLL, \n\r");
	}
	else {
		UsesQpll0 = (FALSE);
		CmnId = XVPHY_CHANNEL_ID_CMN1;
		xil_printf("RX => QPLL1, \n\r");
	}

	xil_printf("RX state: ");
	switch (InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].RxState) {
	case (XVPHY_GT_STATE_IDLE):
		xil_printf("idle\n\r");
		break;
	case (XVPHY_GT_STATE_LOCK):
		if (XVphy_IsRxUsingCpll(InstancePtr, QuadId, ChId)) {
			xil_printf("CPLL lock\n\r");
		}
		else {
			xil_printf("QPLL%d lock\n\r", (UsesQpll0 ? 0 : 1));
		}
		break;
	case (XVPHY_GT_STATE_RESET):
		xil_printf("GT reset\n\r");
		break;
	case (XVPHY_GT_STATE_READY):
		xil_printf("ready\n\r");
		break;
	default:
		xil_printf("unknown\n\r");
		break;
	}

	xil_printf("TX state: ");
	switch (InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(ChId)].TxState) {
	case (XVPHY_GT_STATE_IDLE):
		xil_printf("idle\n\r");
		break;
	case (XVPHY_GT_STATE_LOCK):
		if (XVphy_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
			xil_printf("CPLL lock\n\r");
		}
		else {
			xil_printf("QPLL%d lock\n\r", (UsesQpll0 ? 0 : 1));
		}
		break;
	case (XVPHY_GT_STATE_RESET):
		xil_printf("GT reset\n\r");
		break;
	case (XVPHY_GT_STATE_ALIGN):
		xil_printf("align\n\r");
		break;
	case (XVPHY_GT_STATE_READY):
		xil_printf("ready\n\r");
		break;
	default:
		xil_printf("unknown\n\r");
		break;
	}

	if (XVphy_IsTxUsingCpll(InstancePtr, QuadId, ChId)) {
		QpllDVal = ChPtr->RxOutDiv;
		CpllDVal = ChPtr->TxOutDiv;
	}
	else {
		CpllDVal = ChPtr->RxOutDiv;
		QpllDVal = ChPtr->TxOutDiv;
	}

	xil_printf("\n\r");
	xil_printf("QPLL settings\n\r");
	xil_printf("-------------\n\r");
	xil_printf("M : %d - N : %d - D : %d\n\r",
		InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].
			PllParams.MRefClkDiv,
		InstancePtr->Quads[QuadId].Plls[XVPHY_CH2IDX(CmnId)].
			PllParams.NFbDiv,
		QpllDVal);
	xil_printf("\n\r");

	xil_printf("CPLL settings\n\r");
	xil_printf("-------------\n\r");
	xil_printf("M : %d - N1 : %d - N2 : %d - D : %d\n\r",
			ChPtr->PllParams.MRefClkDiv,
			ChPtr->PllParams.N1FbDiv,
			ChPtr->PllParams.N2FbDiv,
			CpllDVal);
	xil_printf("\n\r");

	print(" \n\r");
}
#endif
