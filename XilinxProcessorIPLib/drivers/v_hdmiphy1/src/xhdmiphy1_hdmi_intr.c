/*******************************************************************************
 *
 * Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xhdmiphy1_hdmi_intr.c
 *
 * This file contains video PHY functionality specific to the HDMI protocol
 * related to interrupts.
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
#include "xstatus.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"
#include "xhdmiphy1_hdmi.h"
#include "xhdmiphy1_gt.h"

/************************** Function Prototypes ******************************/

extern void XHdmiphy1_Ch2Ids(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
        u8 *Id0, u8 *Id1);

static void XHdmiphy1_HdmiGtHandler(XHdmiphy1 *InstancePtr);
static void XHdmiphy1_ClkDetHandler(XHdmiphy1 *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
* This function installs an HDMI callback function for the specified handler
* type
*
* @param    InstancePtr is a pointer to the XHdmiphy1 instance.
* @param    HandlerType is the interrupt handler type which specifies which
*       interrupt event to attach the callback for.
* @param    CallbackFunc is the address to the callback function.
* @param    CallbackRef is the user data item that will be passed to the
*       callback function when it is invoked.
*
* @return   None.
*
* @note     None.
*
*******************************************************************************/
void XHdmiphy1_SetHdmiCallback(XHdmiphy1 *InstancePtr,
        XHdmiphy1_HdmiHandlerType HandlerType,
        void *CallbackFunc, void *CallbackRef)
{
    /* Verify arguments. */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid((HandlerType == XHDMIPHY1_HDMI_HANDLER_TXINIT) ||
            (HandlerType == XHDMIPHY1_HDMI_HANDLER_TXREADY) ||
            (HandlerType == XHDMIPHY1_HDMI_HANDLER_RXINIT) ||
            (HandlerType == XHDMIPHY1_HDMI_HANDLER_RXREADY));
    Xil_AssertVoid(CallbackFunc != NULL);
    Xil_AssertVoid(CallbackRef != NULL);

    switch (HandlerType) {
    case XHDMIPHY1_HDMI_HANDLER_TXINIT:
        InstancePtr->HdmiTxInitCallback = (XHdmiphy1_Callback)CallbackFunc;
        InstancePtr->HdmiTxInitRef = CallbackRef;
        break;

    case XHDMIPHY1_HDMI_HANDLER_TXREADY:
        InstancePtr->HdmiTxReadyCallback = (XHdmiphy1_Callback)CallbackFunc;
        InstancePtr->HdmiTxReadyRef = CallbackRef;
        break;

    case XHDMIPHY1_HDMI_HANDLER_RXINIT:
        InstancePtr->HdmiRxInitCallback = (XHdmiphy1_Callback)CallbackFunc;
        InstancePtr->HdmiRxInitRef = CallbackRef;
        break;

    case XHDMIPHY1_HDMI_HANDLER_RXREADY:
        InstancePtr->HdmiRxReadyCallback = (XHdmiphy1_Callback)CallbackFunc;
        InstancePtr->HdmiRxReadyRef = CallbackRef;
        break;

    default:
        break;
    }
}

/*****************************************************************************/
/**
* This function sets the appropriate HDMI interupt handlers.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiIntrHandlerCallbackInit(XHdmiphy1 *InstancePtr)
{
    /* GT Interrupts */
    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_TXRESET_DONE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_RXRESET_DONE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_CPLL_LOCK,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_QPLL_LOCK,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_TXALIGN_DONE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_QPLL1_LOCK,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);
#else
    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_LCPLL_LOCK,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_RPLL_LOCK,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);
    XHdmiphy1_SetIntrHandler(InstancePtr,
		XHDMIPHY1_INTR_HANDLER_TYPE_TX_GPO_RISING_EDGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
		XHDMIPHY1_INTR_HANDLER_TYPE_RX_GPO_RISING_EDGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);
#endif

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_TX_MMCM_LOCK_CHANGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_RX_MMCM_LOCK_CHANGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_HdmiGtHandler, InstancePtr);

    /* Clock Detector Interrupts */
    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_TX_CLKDET_FREQ_CHANGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_ClkDetHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_RX_CLKDET_FREQ_CHANGE,
            (XHdmiphy1_IntrHandler)XHdmiphy1_ClkDetHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_TX_TMR_TIMEOUT,
            (XHdmiphy1_IntrHandler)XHdmiphy1_ClkDetHandler, InstancePtr);

    XHdmiphy1_SetIntrHandler(InstancePtr,
            XHDMIPHY1_INTR_HANDLER_TYPE_RX_TMR_TIMEOUT,
            (XHdmiphy1_IntrHandler)XHdmiphy1_ClkDetHandler, InstancePtr);
}

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
/*****************************************************************************/
/**
* This function is the handler for events triggered by TX GPO Rising Edge.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiTxGpoRisingEdgeHandler(XHdmiphy1 *InstancePtr)
{
    u8 Id, Id0, Id1;

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TXGPO_RE, 1);

    /* De-assert GPI port. */
    XHdmiphy1_SetGpi(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_TX, FALSE);

    /* Start TX MMCM. */
    XHdmiphy1_MmcmStart(InstancePtr, 0, XHDMIPHY1_DIR_TX);

	/* Configure TXRATE Port */
    XHdmiphy1_DirReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_TX);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_LOCK;
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by RX GPO Rising Edge.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiRxGpoRisingEdgeHandler(XHdmiphy1 *InstancePtr)
{
    u8 Id, Id0, Id1;

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RXGPO_RE, 1);

    /* De-assert GPI port. */
    XHdmiphy1_SetGpi(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_RX, FALSE);

    /* Configure RXRATE Port */
    XHdmiphy1_DirReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_RX);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
            XHDMIPHY1_GT_STATE_LOCK;
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by LCPLL lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiLcpllLockHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType TxPllType;
    u8 Id, Id0, Id1;
    XHdmiphy1_ChannelId ChId;

    /* Determine PLL type. */
    TxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
        XHDMIPHY1_CHANNEL_ID_CH1);

    /* Determine which channel(s) to operate on. */
	ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
				XHDMIPHY1_DIR_NONE, XHDMIPHY1_PLL_TYPE_LCPLL);

	if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
		/* Log, lock */
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_LCPLL_LOCK, 1);

        XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0,
                &Id1);
        for (Id = Id0; Id <= Id1; Id++) {
		if (TxPllType == XHDMIPHY1_PLL_TYPE_LCPLL) {
				InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
					TxState = XHDMIPHY1_GT_STATE_RESET;
		}
		else {
				InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
					RxState = XHDMIPHY1_GT_STATE_RESET;
		}
        }
	}
	else {
		/* Log, Lost lock */
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_LCPLL_LOCK, 0);
	}

}

/*****************************************************************************/
/**
* This function is the handler for events triggered by RPLL lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiRpllLockHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType TxPllType;
    u8 Id, Id0, Id1;
    XHdmiphy1_ChannelId ChId;

    /* Determine PLL type. */
    TxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
        XHDMIPHY1_CHANNEL_ID_CH1);

	/* Determine which channel(s) to operate on. */
	ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
				XHDMIPHY1_DIR_NONE, XHDMIPHY1_PLL_TYPE_RPLL);

	if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
		/* Log, lock */
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RPLL_LOCK, 1);

        XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0,
                &Id1);
        for (Id = Id0; Id <= Id1; Id++) {
		if (TxPllType == XHDMIPHY1_PLL_TYPE_RPLL) {
				InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
					TxState = XHDMIPHY1_GT_STATE_RESET;
		}
		else {
				InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
					RxState = XHDMIPHY1_GT_STATE_RESET;
		}
        }
	}
	else {
		/* Log, Lost lock */
		XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RPLL_LOCK, 0);
	}

}
#else
/*****************************************************************************/
/**
* This function is the handler for events triggered by QPLL lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiQpllLockHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType TxPllType;
    XHdmiphy1_PllType RxPllType;
    u8 Id, Id0, Id1;
    XHdmiphy1_ChannelId ChId;

    /* Determine PLL type. */
    TxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
        XHDMIPHY1_CHANNEL_ID_CH1);
    RxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
        XHDMIPHY1_CHANNEL_ID_CH1);

    /* RX is using QPLL. */
    if ((RxPllType == XHDMIPHY1_PLL_TYPE_QPLL) ||
            (RxPllType == XHDMIPHY1_PLL_TYPE_QPLL0) ||
            (RxPllType == XHDMIPHY1_PLL_TYPE_QPLL1)) {

        /* Determine which channel(s) to operate on. */
        ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
                    XHDMIPHY1_DIR_RX, RxPllType);

        if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
            /* Log, lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_QPLL_LOCK, 1);

            /* GT RX reset. */
            XHdmiphy1_ResetGtTxRx(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
                    XHDMIPHY1_DIR_RX, FALSE);

            XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0,
                    &Id1);
            for (Id = Id0; Id <= Id1; Id++) {
                InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
                    RxState = XHDMIPHY1_GT_STATE_RESET;
            }
        }
        else {
            /* Log, Lost lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_QPLL_LOCK, 0);
        }
    }
    /* TX is using QPLL. */
    else {
        /* Determine which channel(s) to operate on. */
        ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
                    XHDMIPHY1_DIR_TX, TxPllType);

        if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
            /* Log, lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_QPLL_LOCK, 1);

            /* GT TX reset. */
            XHdmiphy1_ResetGtTxRx(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
                    XHDMIPHY1_DIR_TX, FALSE);

            XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0,
                    &Id1);
            for (Id = Id0; Id <= Id1; Id++) {
                InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
                    TxState = XHDMIPHY1_GT_STATE_RESET;
            }
        }
        else {
            /* Log, Lost lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_QPLL_LOCK, 0);
        }
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by CPLL lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiCpllLockHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType TxPllType;
    XHdmiphy1_PllType RxPllType;
    u8 Id, Id0, Id1;
    XHdmiphy1_ChannelId ChId;

    /* Determine PLL type. */
    TxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
            XHDMIPHY1_CHANNEL_ID_CH1);
    RxPllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
            XHDMIPHY1_CHANNEL_ID_CH1);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);

    /* RX is using CPLL. */
    if (RxPllType == XHDMIPHY1_PLL_TYPE_CPLL) {
        /* Determine which channel(s) to operate on. */
        ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
                    XHDMIPHY1_DIR_RX, RxPllType);

        if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
            /* Log, lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_CPLL_LOCK, 1);
            /* GT RX reset. */
            XHdmiphy1_ResetGtTxRx(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
                    XHDMIPHY1_DIR_RX, FALSE);

            for (Id = Id0; Id <= Id1; Id++) {
                InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
                    RxState = XHDMIPHY1_GT_STATE_RESET;
            }
        }
        else {
            /* Log, Lost lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_CPLL_LOCK, 0);
        }
    }
    /* TX is using CPLL. */
    else {
        /* Determine which channel(s) to operate on. */
        ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0,
                    XHDMIPHY1_DIR_TX, TxPllType);

        if (XHdmiphy1_IsPllLocked(InstancePtr, 0, ChId) == XST_SUCCESS) {
            /* Log, lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_CPLL_LOCK, 1);

            /* GT TX reset. */
            XHdmiphy1_ResetGtTxRx(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
                    XHDMIPHY1_DIR_TX, FALSE);

            for (Id = Id0; Id <= Id1; Id++) {
                InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].
                    TxState = XHDMIPHY1_GT_STATE_RESET;
            }
        }
        else {
            /* Log, Lost lock */
            XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_CPLL_LOCK, 0);
        }
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by GT TX alignment done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiGtTxAlignDoneLockHandler(XHdmiphy1 *InstancePtr)
{
    u8 Id, Id0, Id1;

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TX_ALIGN, 1);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_READY;
    }

    /* TX ready callback. */
    if (InstancePtr->HdmiTxReadyCallback) {
        InstancePtr->HdmiTxReadyCallback(InstancePtr->HdmiTxReadyRef);
    }
}
#endif

/*****************************************************************************/
/**
* This function is the handler for events triggered by GT TX reset lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiGtTxResetDoneLockHandler(XHdmiphy1 *InstancePtr)
{
    u8 Id, Id0, Id1;
	XHdmiphy1_PllType PllType;
    XHdmiphy1_ChannelId ChId;

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TX_RST_DONE, 0);

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
                       XHDMIPHY1_CHANNEL_ID_CH1);
    /* Determine which channel(s) to operate on. */
    ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0, XHDMIPHY1_DIR_TX,
                PllType);

    /* Set TX TMDS Clock Pattern Generator */
    if ((InstancePtr->Config.UseGtAsTxTmdsClk == TRUE) &&
         ((InstancePtr->TxHdmi21Cfg.IsEnabled == 0) ||
         (InstancePtr->TxHdmi21Cfg.IsEnabled == 1 &&
                InstancePtr->TxHdmi21Cfg.NChannels == 3))) {
         XHdmiphy1_PatgenSetRatio(InstancePtr, 0,
                       (u64)((XHdmiphy1_GetLineRateHz(InstancePtr,
                                    0, ChId)) / 1000000));
         XHdmiphy1_PatgenEnable(InstancePtr, 0, TRUE);
    } else {
         XHdmiphy1_PatgenEnable(InstancePtr, 0, FALSE);
    }

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    if ((InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE3) ||
            (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) ||
            (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4)) {
        XHdmiphy1_TxAlignReset(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, TRUE);
        XHdmiphy1_TxAlignReset(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);
    }

    /* GT alignment. */
    XHdmiphy1_TxAlignStart(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, TRUE);
    XHdmiphy1_TxAlignStart(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_ALIGN;
    }
#else
    /* Unmask RESET DONE */
    if ((XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr, 0x120) & 0x200) != 0) {

	/* Deassert TX LNKRDY MASK */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_TX_INIT_REG,
			(XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_TX_INIT_REG) & ~0x10101010));
    }

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
		XHDMIPHY1_GT_STATE_READY;
    }

    /* TX ready callback. */
    if (InstancePtr->HdmiTxReadyCallback) {
        InstancePtr->HdmiTxReadyCallback(InstancePtr->HdmiTxReadyRef);
    }
#endif
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by GT RX reset lock done.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiGtRxResetDoneLockHandler(XHdmiphy1 *InstancePtr)
{
    u8 Id, Id0, Id1;

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RX_RST_DONE, 0);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
            XHDMIPHY1_GT_STATE_READY;
    }

    /* Unmask RESET DONE */
	/* Deassert RX LNKRDY MASK */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_RX_INIT_REG,
			(XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_RX_INIT_REG) & ~0x10101010));

    /* If DRU is use/d, release its reset. */
    if (InstancePtr->HdmiRxDruIsEnabled) {
        XHdmiphy1_DruReset(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);
    }

    /* RX ready callback. */
    if (InstancePtr->HdmiRxReadyCallback) {
        InstancePtr->HdmiRxReadyCallback(InstancePtr->HdmiRxReadyRef);
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by a change in TX frequency
* as detected by the HDMI clock detector logic.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiTxClkDetFreqChangeHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType PllType;
    u8 Id, Id0, Id1;

    if (InstancePtr->TxHdmi21Cfg.IsEnabled) {
        if (InstancePtr->Config.TxRefClkSel !=
                InstancePtr->Config.TxFrlRefClkSel) {
            return;
        }
    }
    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TX_FREQ, 0);

	/* Set TX TMDS Clock Pattern Generator */
	if ((InstancePtr->Config.UseGtAsTxTmdsClk == TRUE) &&
		((InstancePtr->TxHdmi21Cfg.IsEnabled == 0) ||
		 (InstancePtr->TxHdmi21Cfg.IsEnabled == 1 &&
			InstancePtr->TxHdmi21Cfg.NChannels == 3))) {
		XHdmiphy1_PatgenEnable(InstancePtr, 0, FALSE);
	}

	/* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
            XHDMIPHY1_CHANNEL_ID_CH1);

	/* If the TX frequency has changed, the PLL is always disabled. */
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHdmiphy1_PowerDownGtPll(InstancePtr, 0,
        (PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
        XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA, TRUE);

    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_TX, TRUE);
#else
	/* Suppress Warning Messages */
    PllType = PllType;

    /* Mask RESET DONE */
	/* Deassert TX LNKRDY MASK */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_TX_INIT_REG,
			(XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_TX_INIT_REG) | 0x10101010));
#endif
    /* Mask the MMCM Lock */
    XHdmiphy1_MmcmLockedMaskEnable(InstancePtr, 0, XHDMIPHY1_DIR_TX, TRUE);

    /* Disable TX MMCM. */
    /* XHdmiphy1_MmcmPowerDown(InstancePtr, 0, XHDMIPHY1_DIR_TX, TRUE); */

    /* Clear TX timer. */
    XHdmiphy1_ClkDetTimerClear(InstancePtr, 0, XHDMIPHY1_DIR_TX);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    /* Clear GT alignment. */
    XHdmiphy1_TxAlignStart(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);
#endif

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_IDLE;
    }

    /* If there is no reference clock, load TX timer in usec. */
    if (XHdmiphy1_ClkDetGetRefClkFreqHz(InstancePtr, XHDMIPHY1_DIR_TX)) {
        XHdmiphy1_ClkDetTimerLoad(InstancePtr, 0, XHDMIPHY1_DIR_TX,
                        InstancePtr->Config.AxiLiteClkFreq/1000);
    }

    /* Callback to re-initialize. */
    if (InstancePtr->HdmiTxInitCallback) {
        InstancePtr->HdmiTxInitCallback(InstancePtr->HdmiTxInitRef);
    }
}

/*****************************************************************************/
/**
* This function is the handler for events triggered by a change in RX frequency
* as detected by the HDMI clock detector logic.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiRxClkDetFreqChangeHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_PllType PllType;
    u32 RxRefClkHz;
    u8 Id, Id0, Id1;

    if (InstancePtr->RxHdmi21Cfg.IsEnabled) {
        if (InstancePtr->Config.RxRefClkSel !=
                InstancePtr->Config.RxFrlRefClkSel) {
            return;
        }
    }

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RX_FREQ, 0);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
            XHDMIPHY1_GT_STATE_IDLE;
    }

    if (!InstancePtr->RxHdmi21Cfg.IsEnabled) {
        /* Mask the MMCM Lock */
        XHdmiphy1_MmcmLockedMaskEnable(InstancePtr, 0, XHDMIPHY1_DIR_RX,
			TRUE);
    }

    /* Determine PLL type and RX reference clock selection. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
            XHDMIPHY1_CHANNEL_ID_CH1);

    /* Fetch New RX Reference Clock Frequency */
    RxRefClkHz = XHdmiphy1_ClkDetGetRefClkFreqHz(InstancePtr,
                    XHDMIPHY1_DIR_RX);

    /* Round input frequency to 10 kHz. */
    RxRefClkHz = (RxRefClkHz+5000) / 10000;
    RxRefClkHz = RxRefClkHz * 10000;

    /* Store RX reference clock. */
    if (InstancePtr->RxHdmi21Cfg.IsEnabled) {
        InstancePtr->HdmiRxRefClkHz = XHDMIPHY1_HDMI21_FRL_REFCLK;
    }
    else {
        InstancePtr->HdmiRxRefClkHz = RxRefClkHz;
    }

    /* If the RX frequency has changed, the PLL is always disabled. */
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHdmiphy1_PowerDownGtPll(InstancePtr, 0,
        (PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
        XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA, TRUE);

    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_RX, TRUE);
#else
	/* Suppress Warning Messages */
    PllType = PllType;

    /* Mask RESET DONE */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_RX_INIT_REG,
			(XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_RX_INIT_REG) | 0x10101010));

#endif

    /* If DRU is present, disable it and assert reset. */
    if (InstancePtr->Config.DruIsPresent) {
        XHdmiphy1_DruReset(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, TRUE);
        XHdmiphy1_DruEnable(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, FALSE);
    }

    /* Clear RX timer. */
    XHdmiphy1_ClkDetTimerClear(InstancePtr, 0, XHDMIPHY1_DIR_RX);

    /* If there is reference clock, load RX timer in usec.
     * The reference clock should be larger than 25Mhz. We are using a 20Mhz
     * instead to keep some margin for errors. */
    if (RxRefClkHz > 20000000) {
        XHdmiphy1_ClkDetTimerLoad(InstancePtr, 0, XHDMIPHY1_DIR_RX,
                        InstancePtr->Config.AxiLiteClkFreq/1000);

        /* Callback to re-initialize. */
        if (InstancePtr->HdmiRxInitCallback) {
            InstancePtr->HdmiRxInitCallback(InstancePtr->HdmiRxInitRef);
        }
    }
}

/*****************************************************************************/
/**
* This function is the handler for TX timer timeout events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiTxTimerTimeoutHandler(XHdmiphy1 *InstancePtr)
{
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    XHdmiphy1_ChannelId ChId;
    XHdmiphy1_PllType PllType;
#else
	u8 CfgValComp;
#endif
    u8 Id, Id0, Id1;

    if (!InstancePtr->TxHdmi21Cfg.IsEnabled) {
        XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TX_TMR, 1);
    }

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_TX,
            XHDMIPHY1_CHANNEL_ID_CH1);
    /* Determine which channel(s) to operate on. */
    ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0, XHDMIPHY1_DIR_TX, PllType);

    /* Start TX MMCM. */
    XHdmiphy1_MmcmStart(InstancePtr, 0, XHDMIPHY1_DIR_TX);

    /* Enable PLL. */
    XHdmiphy1_PowerDownGtPll(InstancePtr, 0,
        (PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
        XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA, FALSE);

    if (PllType != XHDMIPHY1_PLL_TYPE_CPLL) {
        /* Set QPLL Selection in PIO. */
        XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, 0);
    }

    XHdmiphy1_ClkReconfig(InstancePtr, 0, ChId);
    XHdmiphy1_OutDivReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_TX);
    if ((InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE3) ||
        (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) ||
        (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4)) {
        XHdmiphy1_SetBufgGtDiv(InstancePtr, XHDMIPHY1_DIR_TX,
                (PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
                InstancePtr->Quads[0].Plls[0].TxOutDiv :
                (InstancePtr->Quads[0].Plls[0].TxOutDiv != 16) ?
                        InstancePtr->Quads[0].Plls[0].TxOutDiv :
                        InstancePtr->Quads[0].Plls[0].TxOutDiv / 2);
    }

    XHdmiphy1_DirReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_TX);

    /* Assert PLL reset. */
    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_TX, TRUE);

    /* De-assert PLL reset. */
    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_TX, FALSE);

    if ((InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE3) ||
        (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTHE4) ||
        (InstancePtr->Config.XcvrType == XHDMIPHY1_GT_TYPE_GTYE4)) {
        /* Clear GT alignment. */
        XHdmiphy1_TxAlignStart(InstancePtr, ChId, FALSE);
    }

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_LOCK;
    }
#else
	/* Compare the current and next CFG values */
	CfgValComp = XHdmiphy1_CheckLineRateCfg(InstancePtr, 0,
					XHDMIPHY1_CHANNEL_ID_CH1, XHDMIPHY1_DIR_TX);

	if (!CfgValComp) {
		/* Assert GPI port. */
		XHdmiphy1_SetGpi(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_TX, TRUE);
	}
	else {
		XHdmiphy1_HdmiTxGpoRisingEdgeHandler(InstancePtr);
	}

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].TxState =
            XHDMIPHY1_GT_STATE_GPO_RE;
    }
#endif
}

/*****************************************************************************/
/**
* This function is the handler for RX timer timeout events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiRxTimerTimeoutHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_ChannelId ChId;
    XHdmiphy1_PllType PllType;
    u32 Status;
    u8 Id, Id0, Id1;
#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
	u8 CfgValComp;
#endif

    if (!InstancePtr->RxHdmi21Cfg.IsEnabled) {
        XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RX_TMR, 1);
    } else {
		if (InstancePtr->Config.RxRefClkSel ==
				InstancePtr->Config.RxFrlRefClkSel) {
			/* Set MMCM CLKINSEL to CLK1 */
			XHdmiphy1_MmcmSetClkinsel(InstancePtr, 0, XHDMIPHY1_DIR_RX,
					MMCM_CLKINSEL_CLKIN1);

			/* Start RX MMCM. */
			XHdmiphy1_MmcmStart(InstancePtr, 0, XHDMIPHY1_DIR_RX);
		}
    }

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
            XHDMIPHY1_CHANNEL_ID_CH1);
    /* Determine which channel(s) to operate on. */
    ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0, XHDMIPHY1_DIR_RX, PllType);

    XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);


    /* Set RX parameters. */
    Status = XHdmiphy1_SetHdmiRxParam(InstancePtr, 0, ChId);
    if (Status != XST_SUCCESS) {
        for (Id = Id0; Id <= Id1; Id++) {
            InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
                XHDMIPHY1_GT_STATE_IDLE;
        }

        return;
    }

    /* Enable DRU to set the clock muxes. */
    XHdmiphy1_DruEnable(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA,
            InstancePtr->HdmiRxDruIsEnabled);

    /* Update GT DRU mode. */
    XHdmiphy1_HdmiGtDruModeEnable(InstancePtr,
		InstancePtr->HdmiRxDruIsEnabled);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    /* Enable PLL. */
    XHdmiphy1_PowerDownGtPll(InstancePtr, 0,
        (PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
        XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA, FALSE);

    /* Update reference clock election. */
    if (InstancePtr->RxHdmi21Cfg.IsEnabled == FALSE) {
        XHdmiphy1_CfgPllRefClkSel(InstancePtr, 0,
            ((PllType == XHDMIPHY1_PLL_TYPE_CPLL) ?
                XHDMIPHY1_CHANNEL_ID_CHA : XHDMIPHY1_CHANNEL_ID_CMNA),
            ((InstancePtr->HdmiRxDruIsEnabled) ?
                InstancePtr->Config.DruRefClkSel :
                InstancePtr->Config.RxRefClkSel));
    }

    /* Update RefClk selection. */
    XHdmiphy1_WriteCfgRefClkSelReg(InstancePtr, 0);

    /* Determine PLL type. */
    PllType = XHdmiphy1_GetPllType(InstancePtr, 0, XHDMIPHY1_DIR_RX,
            XHDMIPHY1_CHANNEL_ID_CH1);
    /* Determine which channel(s) to operate on. */
    ChId = XHdmiphy1_GetRcfgChId(InstancePtr, 0, XHDMIPHY1_DIR_RX, PllType);

    XHdmiphy1_ClkReconfig(InstancePtr, 0, ChId);
    XHdmiphy1_OutDivReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_RX);

    XHdmiphy1_DirReconfig(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_RX);

    /* Assert RX PLL reset. */
    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
        XHDMIPHY1_DIR_RX, TRUE);

    /* De-assert RX PLL reset. */
    XHdmiphy1_ResetGtPll(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
            XHDMIPHY1_DIR_RX, FALSE);
#else
	/* Compare the current and next CFG values */
	CfgValComp = XHdmiphy1_CheckLineRateCfg(InstancePtr, 0,
					XHDMIPHY1_CHANNEL_ID_CH1, XHDMIPHY1_DIR_RX);

	if (!CfgValComp) {
		/* Assert GPI port. */
		XHdmiphy1_SetGpi(InstancePtr, 0, XHDMIPHY1_CHANNEL_ID_CHA,
				XHDMIPHY1_DIR_RX, TRUE);
	}
	else {
		XHdmiphy1_HdmiRxGpoRisingEdgeHandler(InstancePtr);
	}

	XHdmiphy1_Ch2Ids(InstancePtr, XHDMIPHY1_CHANNEL_ID_CHA, &Id0, &Id1);
    for (Id = Id0; Id <= Id1; Id++) {
        InstancePtr->Quads[0].Plls[XHDMIPHY1_CH2IDX(Id)].RxState =
            XHDMIPHY1_GT_STATE_GPO_RE;
    }
#endif
}

/*****************************************************************************/
/**
* This function is the handler for TX MMCM Lock events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiTxMmcmLockHandler(XHdmiphy1 *InstancePtr)
{
    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_TXPLL_LOCK, 1);

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
    /* Unmask RESET DONE */
	/* Deassert TX LNKRDY MASK */
	XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_TX_INIT_REG,
			(XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
					XHDMIPHY1_TX_INIT_REG) & ~0x10101010));
#endif

}

/*****************************************************************************/
/**
* This function is the handler for RX MMCM Lock events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiRxMmcmLockHandler(XHdmiphy1 *InstancePtr)
{

    XHdmiphy1_LogWrite(InstancePtr, XHDMIPHY1_LOG_EVT_RXPLL_LOCK, 1);

}

/*****************************************************************************/
/**
* This function is the interrupt handler for the GT events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_HdmiGtHandler(XHdmiphy1 *InstancePtr)
{
    u32 Event;
    u32 EventMask;
    u32 EventAck;
    XHdmiphy1_GtState *TxStatePtr;
    XHdmiphy1_GtState *RxStatePtr;

    EventMask =
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
	XHDMIPHY1_INTR_QPLL0_LOCK_MASK | XHDMIPHY1_INTR_CPLL_LOCK_MASK |
		XHDMIPHY1_INTR_QPLL1_LOCK_MASK |
		XHDMIPHY1_INTR_TXALIGNDONE_MASK |
#else
		XHDMIPHY1_INTR_LCPLL_LOCK_MASK | XHDMIPHY1_INTR_RPLL_LOCK_MASK |
        XHDMIPHY1_INTR_TXGPO_RE_MASK | XHDMIPHY1_INTR_RXGPO_RE_MASK |
#endif
		XHDMIPHY1_INTR_TXRESETDONE_MASK | XHDMIPHY1_INTR_RXRESETDONE_MASK |
		XHDMIPHY1_INTR_TXMMCMUSRCLK_LOCK_MASK |
        XHDMIPHY1_INTR_RXMMCMUSRCLK_LOCK_MASK;

    u8 QuadId = 0;

    /* Read Interrupt Status register */
    Event = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_INTR_STS_REG);

    EventAck = EventMask & Event;

    /* Read States for Quad=0 Ch1 */
    TxStatePtr = &InstancePtr->Quads[QuadId].Ch1.TxState;
    RxStatePtr = &InstancePtr->Quads[QuadId].Ch1.RxState;

    if (Event & XHDMIPHY1_INTR_TXMMCMUSRCLK_LOCK_MASK) {
        XHdmiphy1_HdmiTxMmcmLockHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RXMMCMUSRCLK_LOCK_MASK) {
        XHdmiphy1_HdmiRxMmcmLockHandler(InstancePtr);
    }
#if (XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)
    if ((Event & XHDMIPHY1_INTR_QPLL0_LOCK_MASK) ||
        (Event & XHDMIPHY1_INTR_QPLL1_LOCK_MASK)) {
        XHdmiphy1_HdmiQpllLockHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_CPLL_LOCK_MASK) {
        XHdmiphy1_HdmiCpllLockHandler(InstancePtr);
    }
    if ((Event & XHDMIPHY1_INTR_TXRESETDONE_MASK)
            && (*TxStatePtr == XHDMIPHY1_GT_STATE_RESET)) {
        XHdmiphy1_HdmiGtTxResetDoneLockHandler(InstancePtr);
    }
    if ((Event & XHDMIPHY1_INTR_TXALIGNDONE_MASK)
            && (*TxStatePtr == XHDMIPHY1_GT_STATE_ALIGN)) {
        XHdmiphy1_HdmiGtTxAlignDoneLockHandler(InstancePtr);
    }
    if ((Event & XHDMIPHY1_INTR_RXRESETDONE_MASK)
            && (*RxStatePtr == XHDMIPHY1_GT_STATE_RESET)) {
        XHdmiphy1_HdmiGtRxResetDoneLockHandler(InstancePtr);
    }
#else
    if (Event & XHDMIPHY1_INTR_TXGPO_RE_MASK) {
        XHdmiphy1_HdmiTxGpoRisingEdgeHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RXGPO_RE_MASK) {
	XHdmiphy1_HdmiRxGpoRisingEdgeHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_LCPLL_LOCK_MASK) {
        XHdmiphy1_HdmiLcpllLockHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RPLL_LOCK_MASK) {
        XHdmiphy1_HdmiRpllLockHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_TXRESETDONE_MASK) {
        XHdmiphy1_HdmiGtTxResetDoneLockHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RXRESETDONE_MASK) {
        XHdmiphy1_HdmiGtRxResetDoneLockHandler(InstancePtr);
    }

	/* Suppress Warning Messages */
    TxStatePtr = TxStatePtr;
    RxStatePtr = RxStatePtr;
#endif

    /* Clear event flags by writing to Interrupt Status register */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_INTR_STS_REG,
            EventAck);
}

/*****************************************************************************/
/**
* This function is the interrupt handler for the clock detector events.
*
* @param    InstancePtr is a pointer to the HDMIPHY instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdmiphy1_ClkDetHandler(XHdmiphy1 *InstancePtr)
{
    u32 Event;
    u32 EventMask;
    u32 EventAck;

    EventMask = XHDMIPHY1_INTR_TXCLKDETFREQCHANGE_MASK |
                XHDMIPHY1_INTR_RXCLKDETFREQCHANGE_MASK |
                XHDMIPHY1_INTR_TXTMRTIMEOUT_MASK |
                XHDMIPHY1_INTR_RXTMRTIMEOUT_MASK;

    /* Read Interrupt Status register */
    Event = XHdmiphy1_ReadReg(InstancePtr->Config.BaseAddr,
                XHDMIPHY1_INTR_STS_REG);

    EventAck = EventMask & Event;

    if (Event & XHDMIPHY1_INTR_TXCLKDETFREQCHANGE_MASK) {
        XHdmiphy1_HdmiTxClkDetFreqChangeHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RXCLKDETFREQCHANGE_MASK) {
        XHdmiphy1_HdmiRxClkDetFreqChangeHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_TXTMRTIMEOUT_MASK) {
        XHdmiphy1_HdmiTxTimerTimeoutHandler(InstancePtr);
    }
    if (Event & XHDMIPHY1_INTR_RXTMRTIMEOUT_MASK) {
        XHdmiphy1_HdmiRxTimerTimeoutHandler(InstancePtr);
    }

    /* Clear event flags by writing to Interrupt Status register */
    XHdmiphy1_WriteReg(InstancePtr->Config.BaseAddr, XHDMIPHY1_INTR_STS_REG,
            EventAck);
}
