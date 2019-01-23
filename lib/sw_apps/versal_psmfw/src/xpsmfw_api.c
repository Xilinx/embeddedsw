/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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

#include "xpsmfw_api.h"
#include "xpsmfw_ipi_manager.h"
#include "xpsmfw_power.h"

#define PACK_PAYLOAD(Payload, Arg0, Arg1)	\
	Payload[0] = (u32)Arg0;		\
	Payload[1] = (u32)Arg1;         \
	XPsmFw_Printf(DEBUG_PRINT_ALWAYS, "%s(%x)\r\n", __func__, Arg1);

#define LIBPM_MODULE_ID			(0x06)
#define HEADER(len, ApiId)		((len << 16) | (LIBPM_MODULE_ID << 8) | (ApiId))

#define PACK_PAYLOAD0(Payload, ApiId)	\
	PACK_PAYLOAD(Payload, HEADER(0, ApiId), NULL)
#define PACK_PAYLOAD1(Payload, ApiId, Arg1)	\
	PACK_PAYLOAD(Payload, HEADER(1, ApiId), Arg1)

/****************************************************************************/
/**
 * @brief	Process IPI commands
 *
 * @param Payload	API ID and call arguments
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_ProcessIpi(u32 *Payload)
{
	XStatus Status = XST_FAILURE;
	u32 ApiId = Payload[0];
	u32 DeviceId = Payload[1];

	switch (ApiId) {
		case PSM_API_DIRECT_PWR_DWN:
			Status = XPsmFw_DirectPwrDwn(DeviceId);
			break;
		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Send power down event to PMC
 *
 * @param DevId	Device ID of powering down processor
 *
 * @return	XST_SUCCESS or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_PowerDownEvent(u32 DevId)
{
	XStatus Status;
	u32 Payload[PAYLOAD_ARG_CNT];

	PACK_PAYLOAD1(Payload, PM_PWR_DWN_EVENT, DevId);

	Status = XPsmFw_IpiSend(IPI_PSM_IER_PMC_MASK, Payload);

	return Status;
}
