/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_IPI_H_
#define XPM_IPI_H_

#include "xpm_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAYLOAD_ARG_CNT			(8U)
#define RESPONSE_ARG_CNT		(8U)

#define PM_IPI_TIMEOUT			(~0U)

XStatus XPm_IpiSend(u32 IpiMask, u32 *Payload);
XStatus XPm_IpiReadStatus(u32 IpiMask);
XStatus XPm_IpiPollForAck(u32 IpiMask, u32 TimeOutCount);
XStatus XPm_IpiRead(u32 IpiMask, u32 (*Response)[RESPONSE_ARG_CNT]);

#ifdef __cplusplus
}
#endif

#endif /* XPM_IPI_H_ */
