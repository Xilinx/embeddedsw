/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_genx509cert.h
*
* This file contains the functions to create different fields of the X.509
* certificate.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  01/09/2023 Initial release
* 1.1   am   08/18/2023 Added XCert_ErrorStatus enum
* 1.2   har  12/08/2023 Add support for Subject Alternative Name field
*       am   01/31/2024 Moved entire file under PLM_OCP_KEY_MNGMT macro
*       kpt  02/21/2024 Add support for DME extension
* 1.3   har  05/02/2024 Fixed doxygen warnings
*       har  06/07/2023 Added support for Key index
*
*
* </pre>
*
*
******************************************************************************/

#ifndef XCERT_GENX509CERT_H
#define XCERT_GENX509CERT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MNGMT
#include "xplmi_debug.h"
#include "xsecure_elliptic.h"

/**************************** Type Definitions *******************************/

/**************************** Constant Definitions *******************************/
#define XCERT_ISSUER_MAX_SIZE					(600U)
	/**< Max length of the DER encoded Issuer field received from CDO */
#define XCERT_SUBJECT_MAX_SIZE					(600U)
	/**< Max length of the DER encoded Subject field received from CDO */
#define XCERT_VALIDITY_MAX_SIZE					(40U)
	/**< Max length of the DER encoded Validity field received from CDO */
#define XCERT_SUB_ALT_NAME_MAX_SIZE				(90U)
	/**< Max length of the DER encoded Subject Alternative Name field received from CDO */
#define XCERT_HASH_SIZE_IN_BYTES				(48U)
					/**< Length of hash in bytes */
#define XCert_Printf						XPlmi_Printf
				/**< Alias of XPlmi_Printf to be used in XilCert*/
#define XCERT_ECC_P384_PUBLIC_KEY_LEN				(96U)
					/**< Length of ECC P-384 Public Key */

#define XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES      (48U)
				/**< Length og ECC P-384 Public Key in bytes */

#define XCERT_ECC_P384_SIZE_WORDS					(12U)
	/**< Length of ECC P-384 Key in words */

#define XCERT_DME_DEVICE_ID_SIZE_WORDS				(12U)
	/**< Length of DME device id size in words */

#define XCERT_DME_NONCE_SIZE_WORDS					(8U)
	/**< Length of nonce in words */

#define XCERT_DME_MEASURE_SIZE_WORDS				(12U)
	/**< Length of DME measurement in words */

/**************************** Type Definitions *******************************/
/**
 * This typedef contains enumeration of the fields of X.509 certificate
 * which are configured by user
 */
typedef enum {
	XCERT_ISSUER = 0U,	/**< 0 - Issuer */
	XCERT_SUBJECT,		/**< 1 - Subject */
	XCERT_VALIDITY, 	/**< 2 - Validity */
	XCERT_SUBALTNAME	/**< 3 - Subject Alternative Name */
} XCert_UserCfgFields;

/**
 * This typedef contains information related to storing
 * the signature of X.509 certificate
 */
typedef struct {
	u8 Sign[XCERT_ECC_P384_PUBLIC_KEY_LEN]; /**< Signature of TBS certificate */
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES];	/**< Hash of the TBS certificate */
	u8 IsSignAvailable;			/**< Flag to check if signature is available */
} XCert_SignStore;

/**
 * This typedef contains information of the fields of X.509 certificate
 * which is provided by user
 */
typedef struct {
	u8 Issuer[XCERT_ISSUER_MAX_SIZE];	/**< DER encoded value of Issuer */
	u32 IssuerLen;		/**< Length of DER encoded Issuer field */
	u8 Subject[XCERT_SUBJECT_MAX_SIZE];	/**< DER encoded value of Subject */
	u32 SubjectLen;		/**< Length of DER encoded Subject field */
	u8 Validity[XCERT_VALIDITY_MAX_SIZE];	/**< DER encoded value of Validity */
	u32 ValidityLen;	/**< Length of DER encoded Validity field */
	u32 IsSubAltNameAvailable;	/**< Flag to indicate if Subject Alt Name is given by user */
	u8 SubAltName[XCERT_SUB_ALT_NAME_MAX_SIZE];	/**< DER encoded value of Subject Alt Name */
	u32 SubAltNameLen;	/**< Length of DER encoded Subject Alt Name */
} XCert_UserCfg;

/**
 * This typedef contains user configuration and information related to stored signature
 * for given Subsystem ID
 */
typedef struct {
	u32 SubsystemId;	/**< Subsystem Id */
	u32 KeyIndex;		/**< Key Index for given Subsystem Id */
	XCert_UserCfg UserCfg;	/**< User configuration */
	XCert_SignStore SignStore; /**< Signature store */
} XCert_InfoStore;

/**
 * This typedef contains the components of DME challenge
 */
typedef struct {
	u32 DeviceID[XCERT_DME_DEVICE_ID_SIZE_WORDS];	/**< Device ID */
	u32 Nonce[XCERT_DME_NONCE_SIZE_WORDS];		/**< Nonce */
	u32 Measurement[XCERT_DME_MEASURE_SIZE_WORDS];	/**< Measurement */
} XCert_DmeChallenge;

/**
 * This typedef contains the components of DME response which is DME challenge
 * and the signature generated using DME private key
 */
typedef struct {
	XCert_DmeChallenge Dme;                         /**< DME challenge */
	u32 DmeSignatureR[XCERT_ECC_P384_SIZE_WORDS];   /**< Signature comp R */
	u32 DmeSignatureS[XCERT_ECC_P384_SIZE_WORDS];   /**< Signature comp S */
} XCert_DmeResponse;

/**
 * This typedef contains the information required in the request to generate the
 * X.509 certificate
 */
typedef struct {
	u32 IsSelfSigned;	/**< Flag to check if self-signed certificate */
	u32 IsCsr;		/**< Flag to check if Certificate Signing Request */
	u8* SubjectPublicKey;	/**< Subject Public Key */
	u8* IssuerPrvtKey;	/**< Issuer Private Key */
	u8* IssuerPublicKey;	/**< Issuer Public Key */
	u8* FwHash;		/**< Firmware Hash */
	XCert_DmeResponse* DmeResp; /**< DME configuration */
}XCert_AppCfg;

/**
 * This typedef contains the configuration for X.509 certificate for given Subsystem ID
 */
typedef struct {
	u32 SubSystemId;	/**< Subsystem ID */
	u32 KeyIndex;		/**< Key Index */
	XCert_UserCfg *UserCfg;		/**< Configuration from User */
	XCert_AppCfg AppCfg;	/**< Configuration from application */
}XCert_Config;

/**
 * Error codes from request to generate X.509 certificate/ Certificate Signing Request
 */
typedef enum {
	XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD = 0x40,
		/**< 0x40 TBS certificate public key info field */
	XCERT_ERR_X509_GEN_TBSCERT_DIGEST,
		/**< 0x41 TBS certificate Digest */
	XCERT_ERR_X509_CALC_SIGN,
		/**< 0x42 Calculate sign */
	XCERT_ERR_X509_UPDATE_ENCODED_LEN,
		/**< 0x43 Update encoded length */
	XCERT_ERR_X509_INVALID_USER_CFG,
		/**< 0x44 Stored user configuration is all zeroes */
	XCERT_ERR_X509_USR_CFG_NOT_FOUND,
		/**< 0x45 User configuration not found for provided Subsystem Id*/
	XCERT_ERR_X509_GET_SIGN,
		/**< 0x46 Failed to get the signature stored */
	XCERT_ERR_X509_KAT_FAILED,
		/**< 0x47 KAT failure error */
}XCert_ErrorStatus;

/************************** Function Prototypes ******************************/
int XCert_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32* X509CertSize, XCert_Config *Cfg);
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType, u8* Val, u32 Len, u32 KeyIndex);


#ifdef __cplusplus
}
#endif
#endif  /* PLM_OCP_KEY_MNGMT */
#endif  /* XCERT_GENX509CERT_H */
/* @} */
