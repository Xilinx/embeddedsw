/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_COMMON_PLAT_H_
#define XPM_COMMON_PLAT_H_

#include "xstatus.h"
#include "xil_io.h"
#include "xil_util.h"
#include "xpm_err.h"
#include "xplmi_debug.h"
#include "xpm_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PMC_TAP_VERSION_PLATFORM_MASK		(0x0F000000U)

/* @TODO: Added below macro as workaorund to compile for AIEPG2 platform
 * Since it doesn't have PSM, this macro should not be required for the platform
 * Fix this during code cleanup
 */
#ifdef VERSAL_AIEPG2
#ifndef XPAR_XIPIPS_TARGET_PSXL_PSM_0_CH0_MASK
#define	XPAR_XIPIPS_TARGET_PSXL_PSM_0_CH0_MASK 0U
#endif
#endif

#ifdef XPLMI_IPI_DEVICE_ID
	#ifdef XPAR_XIPIPS_TARGET_PSX_PSM_0_CH0_MASK
		#define PSM_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSX_PSM_0_CH0_MASK
	#else
		#define PSM_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSXL_PSM_0_CH0_MASK
	#endif
#else
	#define PSM_IPI_INT_MASK		(0U)
#endif /* XPLMI_IPI_DEVICE_ID */

#define XPM_POLL_TIMEOUT		(0X1000000U)

#ifdef STDOUT_BASEADDRESS
	#if (STDOUT_BASEADDRESS == 0xF1920000U)
		#define NODE_UART PM_DEV_UART_0 /* Assign node ID with UART0 device ID */
	#elif (STDOUT_BASEADDRESS == 0xF1930000U)
		#define NODE_UART PM_DEV_UART_1 /* Assign node ID with UART1 device ID */
	#endif
#endif

static inline u8 XPm_PlatGetSlrIndex(void)
{
	/* Non-SSIT device, must return 0 */
	return 0U;
}

static inline XStatus XPm_SsitForwardApi(XPm_ApiId ApiId, const u32 *ArgBuf,
						      u32 NumArgs, u32 CmdType,
						      u32 *const Response)
{
	(void)ApiId;
	(void)ArgBuf;
	(void)NumArgs;
	(void)CmdType;
	(void)Response;

	return XST_DEVICE_NOT_FOUND;
}

#define SAVE_REGION(...) \
union { \
struct { __VA_ARGS__ } save; \
struct { __VA_ARGS__};};

u32 XPm_GetByteBufferOffset(void);
u32 XPm_GetSavedByteBufferAddress(void);
u32 XPm_GetPrevByteBufferAddress(void);
u32 XPm_ConvertToSavedAddress(u32 InputAddress);
#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_H_ */
