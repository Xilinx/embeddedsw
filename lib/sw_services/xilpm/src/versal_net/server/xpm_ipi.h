/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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

#define PSM_API_DIRECT_PWR_DWN			(1U)
#define PSM_API_DIRECT_PWR_UP			(2U)
#define PSM_API_FPD_HOUSECLEAN			(3U)
#define PM_IPI_TIMEOUT			(~0U)

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
#define PSM_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSXL_PSM_0_CH0_MASK
#else
#define PSM_IPI_INT_MASK		(0U)
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */

XStatus XPm_IpiSend(u32 IpiMask, u32 *Payload);
XStatus XPm_IpiReadStatus(u32 IpiMask);
#ifdef __cplusplus
}
#endif

#endif /* XPM_IPI_H_ */
