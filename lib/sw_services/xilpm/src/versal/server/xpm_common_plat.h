/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

#ifdef XPLMI_IPI_DEVICE_ID
#define PSM_IPI_INT_MASK		XPAR_XIPIPS_TARGET_PSV_PSM_0_CH0_MASK
#else
#define PSM_IPI_INT_MASK		(0U)
#endif /* XPLMI_IPI_DEVICE_ID */

/* Hack: These will increase code size.  Define them as needed. */
#define xSELF_TEST
#define xSELF_TEST_DEVICE_REQUEST
#define xSELF_TEST_PIN_API
#define xSELF_TEST_RESET_API
#define xSELF_TEST_CLOCK_API
#define xDEBUG_REG_IO

#define TRUE	1U
#define FALSE	0U

#define PmChkRegMask32(ADDR, MASK, VAL, STATUS)				\
	do {									\
		if (((u32)(VAL) & (u32)(MASK)) != ((u32)(XPm_In32((ADDR))) & (u32)(MASK))) {	\
			(STATUS) = XPM_REG_WRITE_FAILED;			\
		}								\
	} while (XPM_FALSE_COND)								\

#define PmChkRegOut32(ADDR, VAL, STATUS)					\
	do {									\
		if ((u32)(VAL) != XPm_In32((ADDR))) {				\
			(STATUS) = XPM_REG_WRITE_FAILED;			\
		}								\
	} while (XPM_FALSE_COND)								\


/* Payload Packets */
#define XPM_PACK_PAYLOAD(Payload, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)	\
	do {								\
		Payload[0] = (u32)Arg0;					\
		Payload[1] = (u32)Arg1;					\
		Payload[2] = (u32)Arg2;					\
		Payload[3] = (u32)Arg3;					\
		Payload[4] = (u32)Arg4;					\
		Payload[5] = (u32)Arg5;					\
		PmDbg("%s(%x, %x, %x, %x, %x)\r\n", __func__, Arg1, Arg2, Arg3, Arg4, Arg5);	\
	} while (XPM_FALSE_COND)

#define XPM_MODULE_ID			(0x02UL)

#define XPM_HEADER(len, ApiId)		((len << 16U) | (XPM_MODULE_ID << 8U) | ((u32)ApiId))

#define XPM_HEADER_SET_CMDTYPE(Payload, SecurityFlag)		\
	(Payload[0] = ((Payload[0] & ~0xFF000000UL) | ((SecurityFlag & 0x1UL) << 24U)))

#define XPM_PACK_PAYLOAD0(Payload, ApiId) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(0UL, ApiId), 0, 0, 0, 0, 0)
#define XPM_PACK_PAYLOAD1(Payload, ApiId, Arg1) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(1UL, ApiId), Arg1, 0, 0, 0, 0)
#define XPM_PACK_PAYLOAD2(Payload, ApiId, Arg1, Arg2) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(2UL, ApiId), Arg1, Arg2, 0, 0, 0)
#define XPM_PACK_PAYLOAD3(Payload, ApiId, Arg1, Arg2, Arg3) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(3UL, ApiId), Arg1, Arg2, Arg3, 0, 0)
#define XPM_PACK_PAYLOAD4(Payload, ApiId, Arg1, Arg2, Arg3, Arg4) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(4UL, ApiId), Arg1, Arg2, Arg3, Arg4, 0)
#define XPM_PACK_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	XPM_PACK_PAYLOAD(Payload, XPM_HEADER(5UL, ApiId), Arg1, Arg2, Arg3, Arg4, Arg5)

#define PM_CHECK_MASK(v, m) ((v & m) == m)

/**
 * Get a value at each bits to tell if skipping house cleaning function for given block
 * BLOCK are one of these(can be found under xpm_regs.h):
 *		LPD, FPD, NPD, AIE, CPM, PLD, VDU, GT, HNIC, DDRMC, BFR, SDFEC, ILKN, PMC.
 *
 */
#define PM_DISABLE_HOUSECLEAN_GET(BLOCK)( \
	(XPm_In32(PM_HOUSECLEAN_DISABLE_ ##BLOCK## _BASEADDR) & PM_HOUSECLEAN_DISABLE_ ##BLOCK## _MASK ) >> PM_HOUSECLEAN_DISABLE_ ##BLOCK## _OFFSET \
)
/**
 * FALSE if house clean of given BLOCK and FUNC is skipped
 * Note:  The condition is evaluated redundantly to avoid glitches from altering
 * branching decision.
 * BLOCK are one of these(can be found under xpm_regs.h):
 *		LPD, FPD, NPD, AIE, CPM, PLD, VDU, GT, HNIC, DDRMC, BFR, SDFEC, ILKN, PMC.
 * FUNC are one of these:
 *	SCAN, BISR, MBIST, LBIST, PLHC
 */
#define PM_HOUSECLEAN_CHECK(BLOCK, FUNC) (	\
	(!PM_CHECK_MASK(PM_DISABLE_HOUSECLEAN_GET(BLOCK), HOUSECLEAN_FUNC_DISABLE_ ##FUNC## _MASK)) ||	\
	(!PM_CHECK_MASK(PM_DISABLE_HOUSECLEAN_GET(BLOCK), HOUSECLEAN_FUNC_DISABLE_ ##FUNC## _MASK))		\
)

#define PLATFORM_VERSION_SILICON_ES1		(0x0U)
#define PLATFORM_VERSION_SILICON_ES2		(0x1U)

#define XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT		(18U)
#define XPM_PMC_TAP_IDCODE_DEV_SHIFT		(12U)
#define XPM_PMC_TAP_IDCODE_SBFMLY_S		((u32)2U << XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT)
#define XPM_PMC_TAP_IDCODE_SBFMLY_SV		((u32)3U << XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT)
#define XPM_PMC_TAP_IDCODE_SBFMLY_P_HS		((u32)1U << XPM_PMC_TAP_IDCODE_SBFMLY_SHIFT)
/* VM1802 */
#define PMC_TAP_IDCODE_DEV_VM1802		((u32)0x2AU << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VM1802	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VM1802)
/* VC1902 */
#define PMC_TAP_IDCODE_DEV_VC1902		((u32)0x28U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VC1902	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VC1902)
/* VM2152 */
#define PMC_TAP_IDCODE_DEV_VM2152		((u32)0x20U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VM2152	(XPM_PMC_TAP_IDCODE_SBFMLY_P_HS | PMC_TAP_IDCODE_DEV_VM2152)
/* VR16XX_PARENT */
#define PMC_TAP_IDCODE_DEV_VR16XX_PARENT		((u32)0x20U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VR16XX_PARENT	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VR16XX_PARENT)
/* VR1602 */
#define PMC_TAP_IDCODE_DEV_VR1602		((u32)0x25U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VR1602	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VR1602)
/* VR1652 */
#define PMC_TAP_IDCODE_DEV_VR1652		((u32)0x26U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VR1652	(XPM_PMC_TAP_IDCODE_SBFMLY_S | PMC_TAP_IDCODE_DEV_VR1652)
/* VP1902 */
#define PMC_TAP_IDCODE_DEV_VP1902		((u32)0x0U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VP1902	(XPM_PMC_TAP_IDCODE_SBFMLY_P_HS | PMC_TAP_IDCODE_DEV_VP1902)
/* VE2302 */
#define PMC_TAP_IDCODE_DEV_VE2302		((u32)0x8U << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VE2302	(XPM_PMC_TAP_IDCODE_SBFMLY_SV | PMC_TAP_IDCODE_DEV_VE2302)
/* VM1102 */
#define PMC_TAP_IDCODE_DEV_VM1102		((u32)0xAU << XPM_PMC_TAP_IDCODE_DEV_SHIFT)
#define PMC_TAP_IDCODE_DEV_SBFMLY_VM1102	(XPM_PMC_TAP_IDCODE_SBFMLY_SV | PMC_TAP_IDCODE_DEV_VM1102)

#define SLR_TYPE_MONOLITHIC_DEV			(0x7U)
#define SLR_TYPE_SSIT_DEV_MASTER_SLR		(0x6U)
#define SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_TOP	(0x5U)
#define SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_NTOP	(0x4U)
#define SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_TOP	(0x3U)
#define SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_NTOP	(0x2U)
#define SLR_TYPE_SSIT_DEV_SLAVE_3_SLR_TOP	(0x1U)
#define SLR_TYPE_INVALID			(0x0U)

#ifdef STDOUT_BASEADDRESS
#if (STDOUT_BASEADDRESS == 0xFF000000U)
#define NODE_UART PM_DEV_UART_0 /* Assign node ID with UART0 device ID */
#elif (STDOUT_BASEADDRESS == 0xFF010000U)
#define NODE_UART PM_DEV_UART_1 /* Assign node ID with UART1 device ID */
#endif
#endif

/** Pass through (Empty) macros, since there's no save and restore in versal platform */
#define SAVE_REGION(...) __VA_ARGS__

/* NPI PCSR related general functions */
XStatus XPm_PcsrWrite(u32 BaseAddress, u32 Mask, u32 Value);
u8 XPm_PlatGetSlrIndex(void);

/******************************************************************************
 * SSIT PLM to PLM communication related APIs and Macros
 *****************************************************************************/
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM

#define NODE_SLR_IDX_SHIFT	12U
#define NODE_SLR_IDX_MASK_BITS	0x3U

#define NODE_SLR_IDX_MASK	((u32)NODE_SLR_IDX_MASK_BITS << NODE_SLR_IDX_SHIFT)

/* Timeout for event completion (in microseconds) */
#define TIMEOUT_IOCTL_COMPL	(10000U)

u32 IsNodeOnSecondarySLR(u32 DeviceId, u32 *SlrIndex);
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

XStatus XPm_SsitForwardApi(XPm_ApiId ApiId, const u32 *ArgBuf, u32 NumArgs,
				const u32 CmdType, u32 *const Response);


#ifdef __cplusplus
}
#endif

#endif /* XPM_COMMON_PLAT_H_ */
