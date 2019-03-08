/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF PLRCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEPLNT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_fabric.h
*
* This is the file which contains code related to Fabric.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPMCFW_FABRIC_H
#define XPMCFW_FABRIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
#include "xcframe.h"
#include "xcfupmc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* TRIM Types */
#define XPMCFW_FABRIC_TRIM_VGG		(0x1U)
#define XPMCFW_FABRIC_TRIM_CRAM		(0x2U)
#define XPMCFW_FABRIC_TRIM_BRAM		(0x3U)
#define XPMCFW_FABRIC_TRIM_URAM		(0x4U)

/************************** Function Prototypes ******************************/
XStatus XPmcFw_CfuInit();
XStatus XPmcFw_CframeInit();
XStatus XPmcFw_FabricInit();
XStatus XPmcFw_FabricPrepare();
XStatus XPmcFw_FabricStartSeq();
XStatus XPmcFw_FabricEndSeq();
void XPmcFw_ApplyTrim(u32 TrimType);
void XPmcFw_BramUramRepair();
void XPmcFw_FabricEnable();
XStatus XPmcFw_FabricClean();
XStatus XPmcFw_BisrLaguna();
XStatus XPmcFw_ReadFabricData(u32 *CfiReadPtr, u32 CfiLen);
XStatus XPmcFw_CheckFabricErr();
void XPmcFw_FabricGlblSeqInit();
#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_FABRIC_H */
