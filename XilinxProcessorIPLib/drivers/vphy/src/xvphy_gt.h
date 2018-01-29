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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
 * @file xvphy_gt.h
 *
 * The Xilinx Video PHY (VPHY) driver. This driver supports the Xilinx Video PHY
 * IP core.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  10/19/15 Initial release.
 * 1.1   gm   02/01/16 Added Gtpe2Config and Gtpe4Config variables.
 * 1.4   gm   29/11/16 Added preprocessor directives for sw footprint reduction
 *                     Changed TX reconfig hook from TxPllRefClkDiv1Reconfig to
 *                       TxChReconfig
 *                     Fixed c++ compiler warnings
 *                     Added xcvr adaptor functions for C++ compilations
 * 1.7   gm   13/09/17 Added GTYE4 support
 * </pre>
 *
 * @addtogroup xvphy_v1_7
 * @{
*******************************************************************************/

#ifndef XVPHY_GT_H_
/* Prevent circular inclusions by using protection macros. */
#define XVPHY_GT_H_

/******************************* Include Files ********************************/

#include "xvphy.h"
#include "xvphy_i.h"
#include "xil_assert.h"

/****************************** Type Definitions ******************************/

typedef struct {
	const u8 *M;
	const u8 *N1;
	const u8 *N2;
	const u8 *D;
} XVphy_GtPllDivs;

typedef struct XVphy_GtConfigS {
	u32 (*CfgSetCdr)(XVphy *, u8, XVphy_ChannelId);
	u32 (*CheckPllOpRange)(XVphy *, u8, XVphy_ChannelId, u64);
	u32 (*OutDivChReconfig)(XVphy *, u8, XVphy_ChannelId,
			XVphy_DirectionType);
	u32 (*ClkChReconfig)(XVphy *, u8, XVphy_ChannelId);
	u32 (*ClkCmnReconfig)(XVphy *, u8, XVphy_ChannelId);
	u32 (*RxChReconfig)(XVphy *, u8, XVphy_ChannelId);
	u32 (*TxChReconfig)(XVphy *, u8, XVphy_ChannelId);

	XVphy_GtPllDivs CpllDivs;
	XVphy_GtPllDivs QpllDivs;
} XVphy_GtConfig;

/******************* Macros (Inline Functions) Definitions ********************/
#ifdef __cplusplus
u32 XVphy_CfgSetCdr(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_CheckPllOpRange(XVphy *InstancePtr, u8 QuadId,
							XVphy_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XVphy_OutDivChReconfig(XVphy *InstancePtr, u8 QuadId,
							XVphy_ChannelId ChId, XVphy_DirectionType Dir);
u32 XVphy_ClkChReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_ClkCmnReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_RxChReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
u32 XVphy_TxChReconfig(XVphy *InstancePtr, u8 QuadId, XVphy_ChannelId ChId);
#else
#define XVphy_CfgSetCdr(Ip, ...) \
		((Ip)->GtAdaptor->CfgSetCdr(Ip, __VA_ARGS__))
#define XVphy_CheckPllOpRange(Ip, ...) \
		((Ip)->GtAdaptor->CheckPllOpRange(Ip, __VA_ARGS__))
#define XVphy_OutDivChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->OutDivChReconfig(Ip, __VA_ARGS__))
#define XVphy_ClkChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->ClkChReconfig(Ip, __VA_ARGS__))
#define XVphy_ClkCmnReconfig(Ip, ...) \
		((Ip)->GtAdaptor->ClkCmnReconfig(Ip, __VA_ARGS__))
#define XVphy_RxChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->RxChReconfig(Ip, __VA_ARGS__))
#define XVphy_TxChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->TxChReconfig(Ip, __VA_ARGS__))
#endif

/*************************** Variable Declarations ****************************/

#if (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTXE2)
extern const XVphy_GtConfig Gtxe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE2)
extern const XVphy_GtConfig Gthe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTPE2)
extern const XVphy_GtConfig Gtpe2Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE3)
extern const XVphy_GtConfig Gthe3Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTHE4)
extern const XVphy_GtConfig Gthe4Config;
#elif (XPAR_VPHY_0_TRANSCEIVER == XVPHY_GTYE4)
extern const XVphy_GtConfig Gtye4Config;
#endif

#endif /* XVPHY_GT_H_ */
/** @} */
