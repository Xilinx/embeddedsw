/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_common.h
* @addtogroup xil_ocpapis DME APIs
* @{
*
* @cond xocp_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   vns  06/27/2022 Initial release
* 1.1   am   01/10/2023 Added XOCP_DME_DEVICE_ID_SIZE_BYTES macro for dme
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XOCP_COMMON_H
#define XOCP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"

/************************** Constant Definitions *****************************/
#define XOCP_PCR_SIZE_WORDS			(12U)
#define XOCP_PCR_SIZE_BYTES			(48U)

#define XOCP_DME_DEVICE_ID_SIZE_WORDS		(12U)
#define XOCP_DME_DEVICE_ID_SIZE_BYTES		(XOCP_DME_DEVICE_ID_SIZE_WORDS << 2U)

#define XOCP_DME_NONCE_SIZE_WORDS		(8U)
#define XOCP_DME_NONCE_SIZE_BYTES		(XOCP_DME_NONCE_SIZE_WORDS << 2U)

#define XOCP_DME_MEASURE_SIZE_WORDS		(12U)
#define XOCP_DME_MEASURE_SIZE_BYTES		(XOCP_DME_MEASURE_SIZE_WORDS << 2U)

#define XOCP_ECC_P384_SIZE_WORDS		(12U)
#define XOCP_ECC_P384_SIZE_BYTES		(48U)
#define XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES	(96U)

#define XOCP_MAX_NUM_OF_HWPCR_EVENTS		(32U)
#define XOCP_SHA3_LEN_IN_BYTES          	(48U)

/**************************** Type Definitions *******************************/

/*
 * Hardware PCR selection
 */
typedef enum {
	XOCP_PCR_0 = 0, /**< PCR 0 */
	XOCP_PCR_1,	/**< PCR 1 */
	XOCP_PCR_2,	/**< PCR 2 */
	XOCP_PCR_3,	/**< PCR 3 */
	XOCP_PCR_4,	/**< PCR 4 */
	XOCP_PCR_5,	/**< PCR 5 */
	XOCP_PCR_6,	/**< PCR 6 */
	XOCP_PCR_7	/**< PCR 7 */
} XOcp_RomHwPcr;

/*
 * DME
 */
typedef struct {
	u32 DeviceID[XOCP_DME_DEVICE_ID_SIZE_WORDS];	/**< Device ID */
	u32 Nonce[XOCP_DME_NONCE_SIZE_WORDS];		/**< Nonce */
	u32 Measurement[XOCP_DME_MEASURE_SIZE_WORDS];	/**< Measurement */
} XOcp_Dme;

/*
 * DME response
 */
typedef struct {
	XOcp_Dme Dme;									/**< DME */
	u32 DmeSignatureR[XOCP_ECC_P384_SIZE_WORDS];	/**< Signature comp R */
	u32 DmeSignatureS[XOCP_ECC_P384_SIZE_WORDS];	/**< Signature comp S */
} XOcp_DmeResponse;

typedef struct {
	u8 PcrNo;
	u8 Hash[XOCP_SHA3_LEN_IN_BYTES];
	u8 PcrValue[XOCP_SHA3_LEN_IN_BYTES];
} XOcp_HwPcrEvent;

typedef struct {
	XOcp_HwPcrEvent Buffer[XOCP_MAX_NUM_OF_HWPCR_EVENTS];
	u32 HeadIndex;
	u32 TailIndex;
	u32 OverFlowFlag;
} XOcp_HwPcrLog;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_COMMON_H */
/* @} */
