/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef _XPM_CLIENT_IPI_H_
#define _XPM_CLIENT_IPI_H_

#include "xpm_client_common.h"

#define PAYLOAD_ARG_CNT			(6U)	/* 1 for API ID + 5 for API arguments */
#define RESPONSE_ARG_CNT		(4U)	/* 1 for status + 3 for values */

#define PM_IPI_TIMEOUT			(~0)

#define TARGET_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSU_PMC_0_CH0_MASK

XStatus XPm_IpiSend(struct XPm_Proc *const Proc, u32 *Payload);
XStatus Xpm_IpiReadBuff32(struct XPm_Proc *const Proc, u32 *Val1,
			  u32 *Val2, u32 *Val3);

#endif /* _XPM_CLIENT_IPI_H_ */
