/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xhdmiphy1_i.h
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
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 *            dd/mm/yy
 * ----- ---- -------- -----------------------------------------------
 * 1.0   gm   10/12/18 Initial release.
 * 1.1   ku   17/05/20 Adding uniquification to avoid clash with vphy
 * </pre>
 *
 * @addtogroup xhdmiphy1_v2_4
 * @{
*******************************************************************************/

#ifndef XHDMIPHY1_I_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_I_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xhdmiphy1.h"
#include "xhdmiphy1_hw.h"
#include "xvidc.h"

/****************************** Type Definitions ******************************/


/**************************** Function Prototypes *****************************/


void XHdmiphy1_Ch2Ids(XHdmiphy1 *InstancePtr, XHdmiphy1_ChannelId ChId,
		u8 *Id0, u8 *Id1);
u32 XHdmiphy1_DirReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
XHdmiphy1_SysClkDataSelType XHdmiphy1_Pll2SysClkData(XHdmiphy1_PllType PllSelect);
XHdmiphy1_SysClkOutSelType XHdmiphy1_Pll2SysClkOut(XHdmiphy1_PllType PllSelect);
u32 XHdmiphy1_PllCalculator(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
		u32 PllClkInFreqHz);

/* xhdmiphy1.c: Channel configuration functions - setters. */
u32 XHdmiphy1_WriteCfgRefClkSelReg(XHdmiphy1 *InstancePtr, u8 QuadId);
void XHdmiphy1_CfgPllRefClkSel(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_PllRefClkSelType RefClkSel);
void XHdmiphy1_CfgSysClkDataSel(XHdmiphy1 *InstancePtr, u8 QuadId,
	XHdmiphy1_DirectionType Dir, XHdmiphy1_SysClkDataSelType SysClkDataSel);
void XHdmiphy1_CfgSysClkOutSel(XHdmiphy1 *InstancePtr, u8 QuadId,
	XHdmiphy1_DirectionType Dir, XHdmiphy1_SysClkOutSelType SysClkOutSel);

u32 XHdmiphy1_ClkCalcParams(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u32 PllClkInFreqHz);
u32 XHdmiphy1_OutDivReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_ClkReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
#endif

/* xhdmiphy1.c: Channel configuration functions - getters. */
XHdmiphy1_ChannelId XHdmiphy1_GetRcfgChId(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, XHdmiphy1_PllType PllType);
u32 XHdmiphy1_IsPllLocked(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
u32 XHdmiphy1_GetQuadRefClkFreq(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_PllRefClkSelType RefClkType);
XHdmiphy1_SysClkDataSelType XHdmiphy1_GetSysClkDataSel(XHdmiphy1 *InstancePtr,
        u8 QuadId, XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId);
XHdmiphy1_SysClkOutSelType XHdmiphy1_GetSysClkOutSel(XHdmiphy1 *InstancePtr,
        u8 QuadId, XHdmiphy1_DirectionType Dir, XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_GtUserRdyEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Hold);
u32 XHdmiphy1_CfgCpllCalPeriodandTol(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir,
        u32 FreeRunClkFreq);
#else
void XHdmiphy1_SetGtLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u16 XHdmiphy1_GetGtLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
void XHdmiphy1_SetGpi(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Set);
u8 XHdmiphy1_GetGpo(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
void XHdmiphy1_GtMstReset(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir, u8 Reset);
u8 XHdmiphy1_CheckLineRateCfg(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
#endif

/* xhdmiphy1.c: GT/MMCM DRP access. */
u32 XHdmiphy1_MmcmWriteParameters(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_MmcmReset(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir, u8 Hold);
void XHdmiphy1_MmcmLockedMaskEnable(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, u8 Enable);
u8 XHdmiphy1_MmcmLocked(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_DirectionType Dir);
void XHdmiphy1_MmcmSetClkinsel(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_DirectionType Dir, XHdmiphy1_MmcmClkinsel Sel);
#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
void XHdmiphy1_SetBufgGtDiv(XHdmiphy1 *InstancePtr,
        XHdmiphy1_DirectionType Dir, u8 Div);
/* xhdmiphy1.c Miscellaneous control. */
u32 XHdmiphy1_PowerDownGtPll(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId, u8 Hold);
#endif

/* xhdmiphy1_intr.c: Interrupt handling functions. */
void XHdmiphy1_SetIntrHandler(XHdmiphy1 *InstancePtr,
        XHdmiphy1_IntrHandlerType HandlerType,
	XHdmiphy1_IntrHandler CallbackFunc, void *CallbackRef);
void XHdmiphy1_IntrEnable(XHdmiphy1 *InstancePtr,
		XHdmiphy1_IntrHandlerType Intr);
void XHdmiphy1_IntrDisable(XHdmiphy1 *InstancePtr,
        XHdmiphy1_IntrHandlerType Intr);

#if ((XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYE5)||(XPAR_HDMIPHY1_0_TRANSCEIVER != XHDMIPHY1_GTYP))
u64 XHdmiphy1_GetPllVcoFreqHz(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u8 XHdmiphy1_GetRefClkSourcesCount(XHdmiphy1 *InstancePtr);
#endif

u8 XHdmiphy1_IsHDMI(XHdmiphy1 *InstancePtr, XHdmiphy1_DirectionType Dir);
void XHdmiphy1_HdmiTxTimerTimeoutHandler(XHdmiphy1 *InstancePtr);
void XHdmiphy1_HdmiRxTimerTimeoutHandler(XHdmiphy1 *InstancePtr);

void XHdmiphy1_ErrorHandler(XHdmiphy1 *InstancePtr);

/******************* Macros (Inline Functions) Definitions ********************/

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_I_H_ */
/** @} */
