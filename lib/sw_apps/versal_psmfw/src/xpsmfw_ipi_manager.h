/******************************************************************************
* Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpsmfw_ipi_manager.h
*
* This file contains definitions for IPI manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_IPI_MANAGER_H_
#define XPSMFW_IPI_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xpsmfw_default.h"

#ifdef XPAR_PSV_IPI_PSM_DEVICE_ID
#include "xipipsu.h"

/* Instance of IPI Driver */
//extern XIpiPsu *IpiInstPtr;

#define XPSMFW_IPI_MASK_COUNT 	XIPIPSU_MAX_TARGETS
extern u32 IpiMaskList[XPSMFW_IPI_MASK_COUNT];

#define XPSMFW_IPI_MAX_MSG_LEN XIPIPSU_MAX_MSG_LEN

#endif /* XPAR_PSV_IPI_PSM_DEVICE_ID */

#define XPSMFW_IPI_TIMEOUT	(~0U)
#define PAYLOAD_ARG_CNT		(8U)
#define RESPONSE_ARG_CNT	(8U)

#ifdef XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define IPI_PSM_IER_PMC_MASK	  XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#else
#define IPI_PSM_IER_PMC_MASK 0U
#endif

/**
 * Initialize the IPI driver instance
 * This should be called in the core init
 */
s32 XPsmfw_IpiManagerInit(void);

int XPsmFw_DispatchIpiHandler(u32 SrcMask);

XStatus XPsmFw_IpiSend(u32 IpiMask, u32 *Payload);

XStatus XPsmFw_IpiSendResponse(u32 IpiMask, u32 *Payload);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_IPI_MANAGER_H_ */
