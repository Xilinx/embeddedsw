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
 * @file xvphy_i.h
 *
 * Contains generic APIs that are locally called or used within the
 * VPHY driver.
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
 * 1.0   gm   11/09/16 Initial release.
 * 1.4   gm   11/24/16 Made debug log optional (can be disabled via makefile)
 * 1.5   gm   02/05/17 Added XVphy_CfgCpllCalPeriodandTol API for US+ devices
 * 1.6   gm   06/08/17 Added XVphy_MmcmLocked, XVphy_ErrorHandler and
 *                       XVphy_PllLayoutErrorHandler APIs
 * 1.7   gm   13/09/17 Removed XVphy_MmcmWriteParameters API
 * </pre>
 *
 * @addtogroup xvphy_v1_7
 * @{
*******************************************************************************/

#ifndef XVPHY_I_H_
/* Prevent circular inclusions by using protection macros. */
#define XVPHY_I_H_

/******************************* Include Files ********************************/

#include "xil_assert.h"
#include "xvphy.h"
#include "xvphy_hw.h"
#include "xvidc.h"
#include "xvphy_dp.h"

/****************************** Type Definitions ******************************/


/**************************** Function Prototypes *****************************/


void XVphy_Ch2Ids(XVphy *InstancePtr, XVphy_ChannelId ChId,
		u8 *Id0, u8 *Id1);
XVphy_SysClkDataSelType Pll2SysClkData(XVphy_PllType PllSelect);
XVphy_SysClkOutSelType Pll2SysClkOut(XVphy_PllType PllSelect);
u32 XVphy_PllCalculator(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir,
		u32 PllClkInFreqHz);

/* xvphy.c: Voltage swing and preemphasis. */
void XVphy_SetRxLpm(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Enable);
void XVphy_SetTxVoltageSwing(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, u8 Vs);
void XVphy_SetTxPreEmphasis(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Pe);

/* xvphy.c: Channel configuration functions - setters. */
u32 XVphy_WriteCfgRefClkSelReg(XVphy *InstancePtr, u8 QuadId);
void XVphy_CfgPllRefClkSel(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_PllRefClkSelType RefClkSel);
void XVphy_CfgSysClkDataSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_SysClkDataSelType SysClkDataSel);
void XVphy_CfgSysClkOutSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_SysClkOutSelType SysClkOutSel);

u32 XVphy_ClkCalcParams(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u32 PllClkInFreqHz);
u32 XVphy_OutDivReconfig(XVphy *InstancePtr, u8 QuadId,
				XVphy_ChannelId ChId, XVphy_DirectionType Dir);
u32 XVphy_DirReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir);
u32 XVphy_ClkReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);

/* xvphy.c: Channel configuration functions - getters. */
XVphy_ChannelId XVphy_GetRcfgChId(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_PllType PllType);
u32 XVphy_GetQuadRefClkFreq(XVphy *InstancePtr, u8 QuadId,
		XVphy_PllRefClkSelType RefClkType);
XVphy_SysClkDataSelType XVphy_GetSysClkDataSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_ChannelId ChId);
XVphy_SysClkOutSelType XVphy_GetSysClkOutSel(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, XVphy_ChannelId ChId);
u32 XVphy_IsPllLocked(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_GtUserRdyEnable(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		XVphy_DirectionType Dir, u8 Hold);
u32 XVphy_CfgCpllCalPeriodandTol(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir, u32 FreeRunClkFreq);


/* xvphy.c: GT/MMCM DRP access. */
u32 XVphy_MmcmWriteParameters(XVphy *InstancePtr, u8 QuadId,
							XVphy_DirectionType Dir);
void XVphy_MmcmReset(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir,
		u8 Hold);
void XVphy_MmcmLockedMaskEnable(XVphy *InstancePtr, u8 QuadId,
		XVphy_DirectionType Dir, u8 Enable);
u8 XVphy_MmcmLocked(XVphy *InstancePtr, u8 QuadId, XVphy_DirectionType Dir);
void XVphy_SetBufgGtDiv(XVphy *InstancePtr, XVphy_DirectionType Dir, u8 Div);
/* xvphy.c Miscellaneous control. */
u32 XVphy_PowerDownGtPll(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId,
		u8 Hold);

/* xvphy_intr.c: Interrupt handling functions. */
void XVphy_SetIntrHandler(XVphy *InstancePtr, XVphy_IntrHandlerType HandlerType,
		XVphy_IntrHandler CallbackFunc, void *CallbackRef);
void XVphy_IntrEnable(XVphy *InstancePtr, XVphy_IntrHandlerType Intr);
void XVphy_IntrDisable(XVphy *InstancePtr, XVphy_IntrHandlerType Intr);
void XVphy_CfgErrIntr(XVphy *InstancePtr, XVphy_ErrType ErrIrq, u8 Set);

u64 XVphy_GetPllVcoFreqHz(XVphy *InstancePtr, u8 QuadId,
		XVphy_ChannelId ChId, XVphy_DirectionType Dir);

void XVphy_ErrorHandler(XVphy *InstancePtr);
#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
void XVphy_PllLayoutErrorHandler(XVphy *InstancePtr);
#endif

/******************* Macros (Inline Functions) Definitions ********************/

#endif /* XVPHY_I_H_ */
/** @} */
