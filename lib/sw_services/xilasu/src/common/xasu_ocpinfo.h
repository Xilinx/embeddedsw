/**************************************************************************************************
* Copyright (c) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocpinfo.h
 *
 * This file contains the OCP definitions which are common across the
 * client and server
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/16/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasu_common_defs Common Defs
* @{
*/
#ifndef XASU_OCPINFO_H_
#define XASU_OCPINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_util.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/************************************ Constant Definitions ***************************************/
/* OCP module command IDs */
#define XASU_OCP_GET_DEVIK_X509_CERT_CMD_ID	(0U)	/**< Command ID for DevIk x509 certificate
							generation */
#define XASU_OCP_GET_DEVAK_X509_CERT_CMD_ID	(1U)	/**< Command ID for DevAk x509 certificate
							generation */
#define XASU_OCP_GET_DEVIK_CSR_X509_CERT_CMD_ID	(2U)	/**< Command ID for DevIk CSR x509
							certificate generation */
#define XASU_OCP_DEVAK_ATTESTATION_CMD_ID	(3U)	/**< Command ID for DevAk attestation */
#define XASU_OCP_DME_CHALLENGE_REQ_CMD_ID	(4U)	/**< Command ID for DME challenge request */
#define XASU_OCP_DME_PVT_KEYS_ENCRYPT_CMD_ID	(5U)	/**< Command ID for DME private keys encryption */

#define XASU_OCP_DME_DEVICE_ID_SIZE_IN_WORDS	(12U)	/**< Device ID size in words */
#define XASU_OCP_DME_NONCE_SIZE_IN_WORDS	(8U)	/**< Nonce size in words */
#define XASU_OCP_DME_MEASUREMENT_SIZE_IN_WORDS	(12U)	/**< Measurement size in words */
#define XASU_OCP_DME_DEVICE_ID_SIZE_IN_BYTES	((XASU_OCP_DME_DEVICE_ID_SIZE_IN_WORDS) << 2U)
							/**< Device ID size in bytes */
#define XASU_OCP_DME_NONCE_SIZE_IN_BYTES	((XASU_OCP_DME_NONCE_SIZE_IN_WORDS) << 2U)
							/**< Nonce size in bytes */
#define XASU_OCP_DME_MEASUREMENT_SIZE_IN_BYTES	((XASU_OCP_DME_MEASUREMENT_SIZE_IN_WORDS) << 2U)
							/**< Measurement size in bytes */
#define XASU_OCP_ECC_P384_SIZE_IN_WORDS		(12U)	/**< P-384 size in words */
#define XASU_OCP_DME_KEY_SIZE_IN_BYTES		(48U)	/**< DME key size in bytes */
#define XASU_OCP_DME_IV_SIZE_IN_BYTES		(12U)	/**< DME Iv size in bytes */
#define XASU_OCP_DME_USER_KEY_0_ID		(0U)	/**< DME User Key 0 ID */
#define XASU_OCP_DME_USER_KEY_1_ID		(1U)	/**< DME User Key 1 ID */
#define XASU_OCP_DME_USER_KEY_2_ID		(2U)	/**< DME User Key 2 ID */

/************************************** Type Definitions *****************************************/
/**
 * This enum contains list of device key types.
 */
enum {
	XOCP_DEVIK = 0,		/**< Device identity key */
	XOCP_DEVAK,		/**< Device attestation key */
	XOCP_MAX_DEVICE_KEY,	/**< Maximum support device key */
};

/**
 * This structure contains OCP params info.
 */
typedef struct {
	u64 CertBufAddr;	/**< Certificate buffer address */
	u32 CertBufLen;		/**< Certificate buffer length */
	u32 CertActualSize;	/**< Certificate actual size */
	u32 DevKeySel;		/**< Device key type */
} XAsu_OcpCertParams;

/**
 * This structure contains information related to DevAk attestation.
 */
typedef struct {
	u64 DataAddr;		/**< Address of the data */
	u64 SignatureAddr;	/**< Address of the signature */
	u32 DataLen;		/**< Length of the data */
	u32 SignatureBufLen;	/**< Length of the signature buffer */
} XAsu_OcpDevAkAttest;

/**
 * This structure contains information related to DME challenge parameters buffers.
 */
typedef struct {
	u32 DeviceId[XASU_OCP_DME_DEVICE_ID_SIZE_IN_WORDS]; /**< Device ID buffer */
	u32 Nonce[XASU_OCP_DME_NONCE_SIZE_IN_WORDS]; /**< Nonce buffer */
	u32 Measurement[XASU_OCP_DME_MEASUREMENT_SIZE_IN_WORDS]; /**< Measurement buffer */
} XAsu_OcpDme;

/** DME response */
typedef struct {
	XAsu_OcpDme Dme;					/**< DME */
	u32 DmeSignatureR[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Signature comp R */
	u32 DmeSignatureS[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Signature comp S */
} XAsu_OcpDmeResponse;

/** This structure contains information related to DME challenge parameters. */
typedef struct {
	u64 NonceAddr;	/**< Address of the Nonce buffer */
	u64 OcpDmeResponseAddr;	/**< Address of the DME response structure */
} XAsu_OcpDmeParams;

/**
 * This structure contains information related to DME private key encryption.
 */
typedef struct {
	u64 DmePvtKeyAddr; /**< Address of the DME Private Key */
	u64 DmeEncPvtKeyAddr; /**< Address of the DME Encrypted Private Key */
	u32 DmeKeyId; /**< ID of the DME Key either 3/4/5 */
	u32 Reserved; /**< Reserved */
} XAsu_OcpDmeKeyEncrypt;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_OCPINFO_H_ */
/** @} */
