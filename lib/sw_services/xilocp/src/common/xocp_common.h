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
* 1.2   kpt  06/02/2023 Added XOcp_HwPcrLogInfo structure
*       kal  06/02/2023 Added SW PCR related structures and macros
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
#define XOCP_MAX_NUM_OF_SWPCRS			(0x40U)
#define XOCP_NUM_OF_SWPCRS			(0x8U)
#define XOCP_PDI_TYPE_FULL			(1U)
#define XOCP_PDI_TYPE_PARTIAL			(2U)
#define XOCP_PDI_TYPE_RESTORE			(3U)
#define XOCP_EVENT_ID_NUM_OF_BYTES		(4U)
#define XOCP_VERSION_NUM_OF_BYTES		(1U)

/**************************** Type Definitions *******************************/

typedef enum {
	XOCP_DEVIK = 0, /**< Device Identity key */
	XOCP_DEVAK		/**< Device attestation key */
}XOcp_DevKey;

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
} XOcp_HwPcr;

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

/*
 * HW PCR Event
 */
typedef struct {
	u8 PcrNo;					/**< HW PCR number */
	u8 Hash[XOCP_SHA3_LEN_IN_BYTES];		/**< Hash to be extended */
	u8 PcrValue[XOCP_SHA3_LEN_IN_BYTES];		/**< PCR value after extension */
} XOcp_HwPcrEvent;

/*
 * HW PCR Log
 */
typedef struct {
	u32 RemainingHwPcrEvents;         /**< Number of HWPCR log events */
	u32 TotalHwPcrLogEvents;          /**< Total number of HWPCR log events */
	u32 OverflowCntSinceLastRd;       /**< Overflow count since last read */
	u32 HwPcrEventsRead;              /**< Number of events read in current request */
} XOcp_HwPcrLogInfo;

typedef struct {
	XOcp_HwPcrEvent Buffer[XOCP_MAX_NUM_OF_HWPCR_EVENTS];
	XOcp_HwPcrLogInfo LogInfo;
	u32 HeadIndex;
	u32 TailIndex;
} XOcp_HwPcrLog;
/*
 * SW PCR extend params
 */
typedef struct {
	u32 PcrNum;		/**< SW PCR number */
	u32 MeasurementIdx;	/**< Measurement index */
	u32 DataSize;		/**< Data size */
	u32 PdiType;		/**< Pdi type full/partial/restore */
	u64 DataAddr;		/**< Address of the data to be extended */
} XOcp_SwPcrExtendParams;

/*
 * SW PCR Data read params
 */
typedef struct {
	u32 PcrNum;					/**< SW PCR number */
	u32 MeasurementIdx;				/**< Measurement Index */
	u32 DataStartIdx;				/**< Data Start Index to read */
	u32 BufSize;					/**< Buffer Size */
	u64 BufAddr;                                    /**< User provided Buffer Address */
	u32 ReturnedBytes;				/**< Returned bytes */
} XOcp_SwPcrReadData;

/*
 * SW PCR Measurement
 */
typedef struct {
	u32 EventId;					/**< Event Id */
	u32 Version;					/**< Version */
	u8 HashOfData[XOCP_PCR_SIZE_BYTES];		/**< Hash of the data */
	u8 MeasuredData[XOCP_PCR_SIZE_BYTES];		/**< PCR measurement with N-1 digest */
} XOcp_PcrMeasurement;

/*
 * SW PCR Log InParams
 */
typedef struct {
	u32 PcrNum;					/**< SW PCR number */
	u32 LogSize;					/**< User provided buffer size */
	u64 PcrLogAddr;                                 /**< User provided buffer address */
	u32 DigestCount;				/**< Extended digest count*/
} XOcp_SwPcrLogReadData;

typedef struct {
	u64 CertAddr;
	u64 ActualLenAddr;
	u32 CertSize;
	XOcp_DevKey DevKeySel;
} XOcp_X509Cert;

typedef struct {
	u64 HashAddr;
	u64 SignatureAddr;
	u32 Reserved;
	u32 HashLen;
} XOcp_Attest;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_COMMON_H */
/* @} */
