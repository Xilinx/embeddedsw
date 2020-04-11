/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_gtye4.c
 *
 * Contains a minimal set of functions for the XHdmiphy1 driver that allow
 * access to all of the Video PHY core's functionality. See xhdmiphy1.h for a
 * detailed description of the driver.
 *
 * @note    None.
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

#include "xparameters.h"
#include "xhdmiphy1_gt.h"
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
#include "xstatus.h"

/**************************** Function Prototypes *****************************/

u32 XHdmiphy1_Gtye5RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_Gtye5TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);

/************************** Constant Definitions ******************************/
/* PLL operating ranges. */
#define XHDMIPHY1_LCPLL_MIN        		8000000000LL
#define XHDMIPHY1_LCPLL_MAX        		16375000000LL
#define XHDMIPHY1_LCPLL_MIN_REFCLK 		120000000LL
#define XHDMIPHY1_RPLL_MIN         		4000000000LL
#define XHDMIPHY1_RPLL_MAX         		8000000000LL
#define XHDMIPHY1_RPLL_MIN_REFCLK  		120000000LL
#define XHDMIPHY1_HDMI_GTYE5_DRU_LRATE  3000000000U


const XHdmiphy1_GtConfig Gtye5Config = {
/*  .CfgSetCdr = XHdmiphy1_Gtye4CfgSetCdr,
    .CheckPllOpRange = XHdmiphy1_Gtye4CheckPllOpRange,
    .OutDivChReconfig = XHdmiphy1_Gtye4OutDivChReconfig,
    .ClkChReconfig = XHdmiphy1_Gtye4ClkChReconfig,
    .ClkCmnReconfig = XHdmiphy1_Gtye4ClkCmnReconfig, */
    .RxChReconfig = XHdmiphy1_Gtye5RxChReconfig,
    .TxChReconfig = XHdmiphy1_Gtye5TxChReconfig,

    .CpllDivs = {
        .M = 0,
        .N1 = 0,
        .N2 = 0,
        .D = 0,
    },
    .QpllDivs = {
        .M = 0,
        .N1 = 0,
        .N2 = 0,
        .D = 0,
    },
};

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function calculates the LCPLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if calculated LCPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiLcpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status = XST_SUCCESS;
	u32 *RefClkPtr;
	u8 TmdsClockRatio = 0;
	u8 IsHdmi21 = 0;
	u64 LineRate = 0;

	/* Suppress Warning Messages */
	ChId = ChId;


	/* Pre-calculation. */
	if (Dir == XHDMIPHY1_DIR_RX) {
		RefClkPtr = &InstancePtr->HdmiRxRefClkHz;
		IsHdmi21 = InstancePtr->RxHdmi21Cfg.IsEnabled;
		TmdsClockRatio = InstancePtr->HdmiRxTmdsClockRatio;

		/* Calculate Line Rate */
		if (IsHdmi21) {
			LineRate = InstancePtr->RxHdmi21Cfg.LineRate;
		}
		else {
			LineRate = (u64)(*RefClkPtr) * ((TmdsClockRatio ? 40 : 10));
		}

		/* Disable DRU */
		InstancePtr->HdmiRxDruIsEnabled = 0;

		/* Enable DRU based on incoming REFCLK */
		if ((!IsHdmi21) && (!TmdsClockRatio) &&
			(InstancePtr->HdmiRxRefClkHz < XHDMIPHY1_LCPLL_MIN_REFCLK)) {
			if (InstancePtr->Config.DruIsPresent) {
	            /* Check DRU frequency */
				if (XHdmiphy1_DruGetRefClkFreqHz(InstancePtr) == XST_FAILURE) {
					XHdmiphy1_LogWrite(InstancePtr,
							XHDMIPHY1_LOG_EVT_DRU_CLK_ERR, 1);
					XHdmiphy1_ErrorHandler(InstancePtr);
					return (XST_FAILURE);
				}

				InstancePtr->HdmiRxDruIsEnabled = 1;
				LineRate = XHDMIPHY1_HDMI_GTYE5_DRU_LRATE;
			}
			else {
				XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_NO_DRU, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}
		}
	}
	else {
		RefClkPtr = &InstancePtr->HdmiTxRefClkHz;
		IsHdmi21 = InstancePtr->TxHdmi21Cfg.IsEnabled;
		InstancePtr->HdmiTxSampleRate = 1;

		if (!IsHdmi21) {
			/* Determine if HDMI 2.0 Mode */
			if (*RefClkPtr >= 340000000) {
				TmdsClockRatio = 1;
				(*RefClkPtr) = (*RefClkPtr) / 4;
			}
			/* Check for x1 Over Sampling Mode*/
			else if (*RefClkPtr >= 124990000) {
				InstancePtr->HdmiTxSampleRate = 1;
			}
			/* Check for x2 Over Sampling Mode*/
			else if (*RefClkPtr >= 99000000) {
				InstancePtr->HdmiTxSampleRate = 2;
				(*RefClkPtr) = (*RefClkPtr) * 2;
			}
			/* Check for x3 Over Sampling Mode*/
			else if (*RefClkPtr >= 59400000) {
				InstancePtr->HdmiTxSampleRate = 3;
				(*RefClkPtr) = (*RefClkPtr) * 3;
			}
			/* Check for x5 Over Sampling Mode*/
			else if (*RefClkPtr < 59400000) {
				InstancePtr->HdmiTxSampleRate = 5;
				(*RefClkPtr) = (*RefClkPtr) * 5;
			}
		}

		/* Calculate Line Rate */
		if (IsHdmi21) {
			LineRate = InstancePtr->TxHdmi21Cfg.LineRate;
		}
		else {
			LineRate = (u64)(*RefClkPtr) * ((TmdsClockRatio ? 40 : 10));
		}

	}

	/* Check for DRU mode */
	if ((Dir == XHDMIPHY1_DIR_RX) &&
		(InstancePtr->HdmiRxDruIsEnabled)) {
		InstancePtr->Quads[0].Lcpll.LineRateCfg = 0;
	}
	/* Check for HDMI 1.4/2.0 GT LineRate Config */
	else if (!IsHdmi21) {
		/* HDMI 1.4 */
		if (!TmdsClockRatio) {
			if ((119990000 <= (*RefClkPtr)) &&
					((*RefClkPtr) <= 204687500)) {
				InstancePtr->Quads[0].Lcpll.LineRateCfg = 1;
			}
			else if ((204687500 <= (*RefClkPtr)) &&
						/* 297 MHz + 0.5% + 10 KHz error */
						((*RefClkPtr) <= 298500000)) {
				InstancePtr->Quads[0].Lcpll.LineRateCfg = 2;
			}
			else{
				Status = XST_FAILURE;
			}
		}
		/* HDMI 2.0 */
		else {
			if ((84570000 <= (*RefClkPtr)) &&
						((*RefClkPtr) <= 102343750)) {
				InstancePtr->Quads[0].Lcpll.LineRateCfg = 3;
			}
			else if ((102343750 <= (*RefClkPtr)) &&
						((*RefClkPtr) <= 149500000)) {
				InstancePtr->Quads[0].Lcpll.LineRateCfg = 4;
			}
			else{
				Status = XST_FAILURE;
			}
		}
	}
	/* Check for HDMI 2.1 GT LineRate Config */
	else if (IsHdmi21) {
		if (LineRate == 3000000000) {
			InstancePtr->Quads[0].Lcpll.LineRateCfg = 5;
		}
		else if (LineRate == 6000000000) {
			InstancePtr->Quads[0].Lcpll.LineRateCfg = 6;
		}
		else if (LineRate == 8000000000) {
			InstancePtr->Quads[0].Lcpll.LineRateCfg = 7;
		}
		else if (LineRate == 10000000000) {
			InstancePtr->Quads[0].Lcpll.LineRateCfg = 8;
		}
		else if (LineRate == 12000000000) {
			InstancePtr->Quads[0].Lcpll.LineRateCfg = 9;
		}
		else {
			Status = XST_FAILURE;
		}
	}

	/* Update Line Rate Value */
	XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
			/* LCPLL is CMN0 */
			XHDMIPHY1_CHANNEL_ID_CMN0, LineRate);

	if (Status == XST_FAILURE) {
		XHdmiphy1_LogWrite(InstancePtr,
				XHDMIPHY1_LOG_EVT_GT_LCPLL_CFG_ERR, 1);
		XHdmiphy1_ErrorHandler(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function calculates the RPLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
* @param	Dir is an indicator for RX or TX.
*
* @return
*		- XST_SUCCESS if calculated RPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiRpllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir)
{
	u32 Status = XST_SUCCESS;
	u32 *RefClkPtr;
	u8 TmdsClockRatio = 0;
	u8 IsHdmi21 = 0;
	u64 LineRate = 0;

	/* Suppress Warning Messages */
	ChId = ChId;


	/* Pre-calculation. */
	if (Dir == XHDMIPHY1_DIR_RX) {
		RefClkPtr = &InstancePtr->HdmiRxRefClkHz;
		IsHdmi21 = InstancePtr->RxHdmi21Cfg.IsEnabled;
		TmdsClockRatio = InstancePtr->HdmiRxTmdsClockRatio;

		/* Calculate Line Rate */
		if (IsHdmi21) {
			LineRate = InstancePtr->RxHdmi21Cfg.LineRate;
		}
		else {
			LineRate = (u64)(*RefClkPtr) * ((TmdsClockRatio ? 40 : 10));
		}

		/* Disable DRU */
		InstancePtr->HdmiRxDruIsEnabled = 0;

		/* Enable DRU based on incoming REFCLK */
		if ((!IsHdmi21) && (!TmdsClockRatio) &&
			(InstancePtr->HdmiRxRefClkHz < XHDMIPHY1_RPLL_MIN_REFCLK)) {
			if (InstancePtr->Config.DruIsPresent) {
	            /* Check DRU frequency */
				if (XHdmiphy1_DruGetRefClkFreqHz(InstancePtr) == XST_FAILURE) {
					XHdmiphy1_LogWrite(InstancePtr,
							XHDMIPHY1_LOG_EVT_DRU_CLK_ERR, 1);
					XHdmiphy1_ErrorHandler(InstancePtr);
					return (XST_FAILURE);
				}

				InstancePtr->HdmiRxDruIsEnabled = 1;
				LineRate = XHDMIPHY1_HDMI_GTYE5_DRU_LRATE;
			}
			else {
				XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_NO_DRU, 1);
				XHdmiphy1_ErrorHandler(InstancePtr);
				return (XST_FAILURE);
			}
		}
	}
	else {
		RefClkPtr = &InstancePtr->HdmiTxRefClkHz;
		IsHdmi21 = InstancePtr->TxHdmi21Cfg.IsEnabled;
		InstancePtr->HdmiTxSampleRate = 1;

		if (!IsHdmi21) {
			/* Determine if HDMI 2.0 Mode */
			if (*RefClkPtr >= 340000000) {
				TmdsClockRatio = 1;
				(*RefClkPtr) = (*RefClkPtr) / 4;
			}
			/* Check for x1 Over Sampling Mode*/
			else if (*RefClkPtr >= 124990000) {
				InstancePtr->HdmiTxSampleRate = 1;
			}
			/* Check for x2 Over Sampling Mode*/
			else if (*RefClkPtr >= 99000000) {
				InstancePtr->HdmiTxSampleRate = 2;
				(*RefClkPtr) = (*RefClkPtr) * 2;
			}
			/* Check for x3 Over Sampling Mode*/
			else if (*RefClkPtr >= 59400000) {
				InstancePtr->HdmiTxSampleRate = 3;
				(*RefClkPtr) = (*RefClkPtr) * 3;
			}
			/* Check for x5 Over Sampling Mode*/
			else if (*RefClkPtr < 59400000) {
				InstancePtr->HdmiTxSampleRate = 5;
				(*RefClkPtr) = (*RefClkPtr) * 5;
			}
		}

		/* Calculate Line Rate */
		if (IsHdmi21) {
			LineRate = InstancePtr->TxHdmi21Cfg.LineRate;
		}
		else {
			LineRate = (u64)(*RefClkPtr) * ((TmdsClockRatio ? 40 : 10));
		}

	}

	/* Check for DRU mode */
	if ((Dir == XHDMIPHY1_DIR_RX) &&
		(InstancePtr->HdmiRxDruIsEnabled)) {
		InstancePtr->Quads[0].Rpll.LineRateCfg = 0;
	}
	/* Check for HDMI 1.4/2.0 GT LineRate Config */
	else if (!IsHdmi21) {
		/* HDMI 1.4 */
		if (!TmdsClockRatio) {
			if ((119990000 <= (*RefClkPtr)) &&
					((*RefClkPtr) <= 200000000)) {
				InstancePtr->Quads[0].Rpll.LineRateCfg = 1;
			}
			else if ((200000000 <= (*RefClkPtr)) &&
						/* 297 MHz + 0.5% + 10 KHz error */
						((*RefClkPtr) <= 298500000)) {
				InstancePtr->Quads[0].Rpll.LineRateCfg = 2;
			}
			else{
				Status = XST_FAILURE;
			}
		}
		/* HDMI 2.0 */
		else {
			if ((84570000 <= (*RefClkPtr)) &&
						((*RefClkPtr) <= 100000000)) {
				InstancePtr->Quads[0].Rpll.LineRateCfg = 3;
			}
			else if ((100000000 <= (*RefClkPtr)) &&
						((*RefClkPtr) <= 149500000)) {
				InstancePtr->Quads[0].Rpll.LineRateCfg = 4;
			}
			else{
				Status = XST_FAILURE;
			}
		}
	}
	/* Check for HDMI 2.1 GT LineRate Config */
	else if (IsHdmi21) {
		if (LineRate == 3000000000) {
			InstancePtr->Quads[0].Rpll.LineRateCfg = 5;
		}
		else if (LineRate == 6000000000) {
			InstancePtr->Quads[0].Rpll.LineRateCfg = 6;
		}
		else if (LineRate == 8000000000) {
			InstancePtr->Quads[0].Rpll.LineRateCfg = 7;
		}
		else if (LineRate == 10000000000) {
			InstancePtr->Quads[0].Rpll.LineRateCfg = 8;
		}
		else if (LineRate == 12000000000) {
			InstancePtr->Quads[0].Rpll.LineRateCfg = 9;
		}
		else {
			Status = XST_FAILURE;
		}
	}

	/* Update Line Rate Value */
	XHdmiphy1_CfgLineRate(InstancePtr, QuadId,
			/* RPLL is CMN1 */
			XHDMIPHY1_CHANNEL_ID_CMN1, LineRate);

	if (Status == XST_FAILURE) {
		XHdmiphy1_LogWrite(InstancePtr,
				XHDMIPHY1_LOG_EVT_GT_RPLL_CFG_ERR, 1);
		XHdmiphy1_ErrorHandler(InstancePtr);
	}

	return Status;
}

/*****************************************************************************/
/**
* This function calculates the TX PLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if calculated QPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiTxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u32 Status;
	XHdmiphy1_PllType PllType;

	/* Suppress Warning Messages */
	QuadId = QuadId;

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
        XHDMIPHY1_CHANNEL_ID_CH1);

    if (PllType == XHDMIPHY1_PLL_TYPE_LCPLL) {
    	Status = XHdmiphy1_HdmiLcpllParam(InstancePtr, 0, ChId,
    				XHDMIPHY1_DIR_TX);
    }
    else { /* RPLL */
    	Status = XHdmiphy1_HdmiRpllParam(InstancePtr, 0, ChId,
    				XHDMIPHY1_DIR_TX);
    }

    return Status;
}

/*****************************************************************************/
/**
* This function calculates the RX PLL parameters.
*
* @param	InstancePtr is a pointer to the HDMI GT core instance.
* @param	QuadId is the GT quad ID to operate on.
* @param	ChId is the channel ID to operate on.
*
* @return
*		- XST_SUCCESS if calculated QPLL parameters updated
*		  successfully.
*		- XST_FAILURE if parameters not updated.
*
* @note		None.
*
******************************************************************************/
u32 XHdmiphy1_HdmiRxPllParam(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
    u32 Status;
	XHdmiphy1_PllType PllType;

	/* Suppress Warning Messages */
	QuadId = QuadId;

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
        XHDMIPHY1_CHANNEL_ID_CH1);

    if (PllType == XHDMIPHY1_PLL_TYPE_LCPLL) {
    	Status = XHdmiphy1_HdmiLcpllParam(InstancePtr, 0, ChId,
    				XHDMIPHY1_DIR_RX);
    }
    else { /* RPLL */
    	Status = XHdmiphy1_HdmiRpllParam(InstancePtr, 0, ChId,
    				XHDMIPHY1_DIR_RX);
    }

	return Status;
}


/*****************************************************************************/
/**
* This function will configure the channel's RX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye5RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u8 CfgValComp;

	/* Compare the current and next CFG values */
	CfgValComp = XHdmiphy1_CheckLineRateCfg(InstancePtr, QuadId, ChId,
					XHDMIPHY1_DIR_RX);

	if (!CfgValComp) {
		/* If CFG values are different */
		XHdmiphy1_SetGtLineRateCfg(InstancePtr, QuadId, ChId,
					XHDMIPHY1_DIR_RX);
	}
	else {
		/* Toggle RX Master Reset */
		XHdmiphy1_GtMstReset(InstancePtr, QuadId, ChId, XHDMIPHY1_DIR_RX,
					TRUE);
		XHdmiphy1_GtMstReset(InstancePtr, QuadId, ChId, XHDMIPHY1_DIR_RX,
					FALSE);
	}

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function will configure the channel's TX settings.
*
* @param    InstancePtr is a pointer to the XHdmiphy1 core instance.
* @param    QuadId is the GT quad ID to operate on.
* @param    ChId is the channel ID to operate on.
*
* @return
*       - XST_SUCCESS if the configuration was successful.
*       - XST_FAILURE otherwise.
*
* @note     None.
*
******************************************************************************/
u32 XHdmiphy1_Gtye5TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId)
{
	u8 CfgValComp;

	/* Compare the current and next CFG values */
	CfgValComp = XHdmiphy1_CheckLineRateCfg(InstancePtr, QuadId, ChId,
					XHDMIPHY1_DIR_TX);

	if (!CfgValComp) {
		/* If CFG values are different */
		XHdmiphy1_SetGtLineRateCfg(InstancePtr, QuadId, ChId,
					XHDMIPHY1_DIR_TX);
	}
	else {
		/* Toggle RX Master Reset */
		XHdmiphy1_GtMstReset(InstancePtr, QuadId, ChId, XHDMIPHY1_DIR_TX,
					TRUE);
		XHdmiphy1_GtMstReset(InstancePtr, QuadId, ChId, XHDMIPHY1_DIR_TX,
					FALSE);
	}

    return XST_SUCCESS;
}

#endif
