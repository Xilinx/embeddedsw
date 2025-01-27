/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_IPI_H_
#define XPM_IPI_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XPLMI_IPI_DEVICE_ID
#define PAYLOAD_ARG_CNT		XIPIPSU_MAX_MSG_LEN
#define RESPONSE_ARG_CNT	XIPIPSU_MAX_MSG_LEN
#else
#define PAYLOAD_ARG_CNT		(8U)
#define RESPONSE_ARG_CNT	(8U)
#endif
#define PM_IPI_TIMEOUT			(~0U)

XStatus XPm_IpiSend(u32 IpiMask, u32 *Payload);
XStatus XPm_IpiReadStatus(u32 IpiMask);
XStatus XPm_IpiPollForAck(u32 IpiMask, u32 TimeOutCount);
XStatus XPm_IpiRead(u32 IpiMask, u32 (*Response)[RESPONSE_ARG_CNT]);

#ifdef __cplusplus
}
#endif

#endif /* XPM_IPI_H_ */
