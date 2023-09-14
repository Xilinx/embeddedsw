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
*       am   08/18/2023 Added XOcp_OcpErrorStatus enum
*       am   09/04/2023 Added XOCP_DICE_CDI_SEED_ZERO enum
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
#define XOCP_PCR_SIZE_WORDS			(12U) /**< PCR size in words */
#define XOCP_PCR_SIZE_BYTES			(48U) /**< PCR size in bytes */

#define XOCP_DME_DEVICE_ID_SIZE_WORDS		(12U) /**< Device id size in words */
#define XOCP_DME_DEVICE_ID_SIZE_BYTES		(XOCP_DME_DEVICE_ID_SIZE_WORDS << 2U) /**< Device id size in bytes */

#define XOCP_DME_NONCE_SIZE_WORDS		(8U) /**< Nonce size in words */
#define XOCP_DME_NONCE_SIZE_BYTES		(XOCP_DME_NONCE_SIZE_WORDS << 2U) /**< Nonce size in bytes */

#define XOCP_DME_MEASURE_SIZE_WORDS		(12U) /**< Measurement size in words */
#define XOCP_DME_MEASURE_SIZE_BYTES		(XOCP_DME_MEASURE_SIZE_WORDS << 2U) /**< Measurement size in bytes */

#define XOCP_ECC_P384_SIZE_WORDS		(12U) /**< Curve P384 size in words */
#define XOCP_ECC_P384_SIZE_BYTES		(48U) /**< Curve P384 size in bytes */
#define XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES	(96U) /**< Size of P384 public key in bytes */

#define XOCP_MAX_NUM_OF_HWPCR_EVENTS		(32U) /**< Maximum number of hardware pcr events */
#define XOCP_SHA3_LEN_IN_BYTES          	(48U) /**< Lenght of sha3 hash in bytes */
#define XOCP_MAX_NUM_OF_SWPCRS			(0x40U) /**< Maximum number of software pcrs */
#define XOCP_NUM_OF_SWPCRS			(0x8U) /**< Number of software pcrs */
#define XOCP_EVENT_ID_NUM_OF_BYTES		(4U) /**< Number of bytes of pcr event ID*/
#define XOCP_VERSION_NUM_OF_BYTES		(1U) /**< Number of bytes of ocp version */

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
	XOcp_HwPcrEvent Buffer[XOCP_MAX_NUM_OF_HWPCR_EVENTS]; /**< Stores hardware pcr events */
	XOcp_HwPcrLogInfo LogInfo; /**< Log information of hardware pcr */
	u32 HeadIndex; /**< Starting index of hardware pcr event */
	u32 TailIndex; /**< Last index of hardware pcr event */
} XOcp_HwPcrLog;
/*
 * SW PCR extend params
 */
typedef struct {
	u32 PcrNum;		/**< SW PCR number */
	u32 MeasurementIdx;	/**< Measurement index */
	u32 DataSize;		/**< Data size */
	u32 OverWrite;		/**< Digest to overwrite or not */
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
	u32 DataLength;                                 /**< Data length */
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
	u64 CertAddr;		/**< Address of Certificate */
	u64 ActualLenAddr;	/**< Address of Actual Length of certificate */
	u32 CertSize;		/**< Size of certificate */
	XOcp_DevKey DevKeySel;	/**< Select Device key - DevIK/ DevAK */
	u32 IsCsr;		/**< Flag for Certificate Signing Request */
} XOcp_X509Cert;

typedef struct {
	u64 HashAddr; /**< Address of the hash */
	u64 SignatureAddr; /**< Address of the signature */
	u32 Reserved; /**<  Reserved this field is not been used */
	u32 HashLen; /**< Length of the hash */
} XOcp_Attest;

typedef enum {
	XOCP_PCR_ERR_PCR_SELECT	= 0x02, /**< 0x02 Error in PCR selection */
	XOCP_PCR_ERR_NOT_COMPLETED,	/**< 0x03 PCR operation not completed */
	XOCP_PCR_ERR_OPERATION,		/**< 0x04 PCR operation error */
	XOCP_PCR_ERR_IN_UPDATE_LOG,	/**< 0x05 PCR log update error */
	XOCP_PCR_ERR_IN_GET_PCR,	/**< 0x06 Error in GetPcr */
	XOCP_PCR_ERR_IN_GET_PCR_LOG,	/**< 0x07 Error in GetPcrLog*/
	XOCP_PCR_ERR_INVALID_LOG_READ_REQUEST,
					/**< 0x08 PCR log read request is invalid */
	XOCP_PCR_ERR_MEASURE_IDX_SELECT,/**< 0x09 SwPcr measurement index is invalid */
	XOCP_PCR_ERR_SWPCR_CONFIG_NOT_RECEIVED, /**< 0x0A SwPcr configuration is not done */
	XOCP_PCR_ERR_INSUFFICIENT_BUF_MEM, /**< 0x0B Pcr insufficient buffer size provided */
	XOCP_PCR_ERR_SWPCR_DUP_EXTEND, /**< 0x0C Duplicate Pcr extend request received */

	XOCP_DICE_CDI_PARITY_ERROR = 0x20,	/**< 0x20 CDI parity error */
	XOCP_DME_ERR,		/**< 0x21 DME signing error */
	XOCP_DME_ROM_ERROR,		/**< 0x22 DME error in ROM */
	XOCP_ERR_DEVIK_NOT_READY,	/**< 0x23 DEVIK key not ready */
	XOCP_ERR_DEVAK_NOT_READY,	/**< 0x24 DEVAK key not ready */
	XOCP_ERR_INVALID_DEVAK_REQ,	/**< 0x25 Error when there is a invalid DEVAK request */
	XOCP_DICE_CDI_SEED_ZERO,	/**< 0x26 DICE CDI Seed is zero */
	XOCP_ERR_GLITCH_DETECTED,	/**< 0x27 Error glitch detected */
}XOcp_OcpErrorStatus;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_COMMON_H */
/* @} */
