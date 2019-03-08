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
 * @file xhdmiphy1_gt.h
 *
 * The Xilinx HDMI PHY (HDMIPHY) driver. This driver supports the
 * Xilinx HDMI PHY IP core.
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
 * </pre>
 *
 * @addtogroup xhdmiphy1_v1_0
 * @{
*******************************************************************************/

#ifndef XHDMIPHY1_GT_H_
/* Prevent circular inclusions by using protection macros. */
#define XHDMIPHY1_GT_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/

#include "xhdmiphy1.h"
#include "xhdmiphy1_i.h"
#include "xil_assert.h"

/****************************** Type Definitions ******************************/

typedef struct {
	const u8 *M;
	const u8 *N1;
	const u8 *N2;
	const u8 *D;
} XHdmiphy1_GtPllDivs;

typedef struct XHdmiphy1_GtConfigS {
	u32 (*CfgSetCdr)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId);
	u32 (*CheckPllOpRange)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId, u64);
	u32 (*OutDivChReconfig)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId,
			XHdmiphy1_DirectionType);
	u32 (*ClkChReconfig)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId);
	u32 (*ClkCmnReconfig)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId);
	u32 (*RxChReconfig)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId);
	u32 (*TxChReconfig)(XHdmiphy1 *, u8, XHdmiphy1_ChannelId);

	XHdmiphy1_GtPllDivs CpllDivs;
	XHdmiphy1_GtPllDivs QpllDivs;
} XHdmiphy1_GtConfig;

/******************* Macros (Inline Functions) Definitions ********************/
#ifdef __cplusplus
u32 XHdmiphy1_CfgSetCdr(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_CheckPllOpRange(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, u64 PllClkOutFreqHz);
u32 XHdmiphy1_OutDivChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
		XHdmiphy1_ChannelId ChId, XHdmiphy1_DirectionType Dir);
u32 XHdmiphy1_ClkChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_ClkCmnReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_RxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
u32 XHdmiphy1_TxChReconfig(XHdmiphy1 *InstancePtr, u8 QuadId,
        XHdmiphy1_ChannelId ChId);
#else
#define XHdmiphy1_CfgSetCdr(Ip, ...) \
		((Ip)->GtAdaptor->CfgSetCdr(Ip, __VA_ARGS__))
#define XHdmiphy1_CheckPllOpRange(Ip, ...) \
		((Ip)->GtAdaptor->CheckPllOpRange(Ip, __VA_ARGS__))
#define XHdmiphy1_OutDivChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->OutDivChReconfig(Ip, __VA_ARGS__))
#define XHdmiphy1_ClkChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->ClkChReconfig(Ip, __VA_ARGS__))
#define XHdmiphy1_ClkCmnReconfig(Ip, ...) \
		((Ip)->GtAdaptor->ClkCmnReconfig(Ip, __VA_ARGS__))
#define XHdmiphy1_RxChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->RxChReconfig(Ip, __VA_ARGS__))
#define XHdmiphy1_TxChReconfig(Ip, ...) \
		((Ip)->GtAdaptor->TxChReconfig(Ip, __VA_ARGS__))
#endif

/*************************** Variable Declarations ****************************/

#if (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE3)
extern const XHdmiphy1_GtConfig Gthe3Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTHE4)
extern const XHdmiphy1_GtConfig Gthe4Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE4)
extern const XHdmiphy1_GtConfig Gtye4Config;
#elif (XPAR_HDMIPHY1_0_TRANSCEIVER == XHDMIPHY1_GTYE5)
extern const XHdmiphy1_GtConfig Gtye5Config;
#endif

#ifdef __cplusplus
}
#endif

#endif /* XHDMIPHY1_GT_H_ */
/** @} */
