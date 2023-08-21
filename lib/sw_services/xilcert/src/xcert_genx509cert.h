/******************************************************************************
* Copyright (c) 2023, Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023, Advanced Micro Devices, Inc.  All rights reserved.
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
#include "xparameters.h"
#include "xplmi_debug.h"
#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_elliptic.h"
#endif
/**************************** Type Definitions *******************************/

/**************************** Constant Definitions *******************************/
#define XCERT_ISSUER_MAX_SIZE					(600U)
	/**< Max length of the DER encoded Issuer field received from CDO */
#define XCERT_SUBJECT_MAX_SIZE					(600U)
	/**< Max length of the DER encoded Subject field received from CDO */
#define XCERT_VALIDITY_MAX_SIZE					(40U)
	/**< Max length of the DER encoded Validity field received from CDO */
#define XCERT_HASH_SIZE_IN_BYTES				(48U)
					/**< Length of hash in bytes */
#define XCert_Printf						XPlmi_Printf
				/**< Alias of XPlmi_Printf to be used in XilCert*/
#define XCERT_ECC_P384_PUBLIC_KEY_LEN				(96U)
					/**< Length of ECC P-384 Public Key */

/**************************** Type Definitions *******************************/
typedef enum {
	XCERT_ISSUER = 0U,	/**< 0U */
	XCERT_SUBJECT,		/**< 1U */
	XCERT_VALIDITY		/**< 2U */
} XCert_UserCfgFields;

typedef struct {
	u8 Sign[XCERT_ECC_P384_PUBLIC_KEY_LEN]; /**< Signature of TBS certificate */
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES];	/**< Hash of the TBS certificate */
	u8 IsSignAvailable;			/**< Flag to check if signature is available */
} XCert_SignStore;

typedef struct {
	u8 Issuer[XCERT_ISSUER_MAX_SIZE];	/**< DER encoded value of Issuer */
	u32 IssuerLen;		/**< Length of DER encoded Issuer field */
	u8 Subject[XCERT_SUBJECT_MAX_SIZE];	/**< DER encoded value of Subject */
	u32 SubjectLen;		/**< Length of DER encoded Subject field */
	u8 Validity[XCERT_VALIDITY_MAX_SIZE];	/**< DER encoded value of Validity */
	u32 ValidityLen;	/**< Length of DER encoded Validity field */
} XCert_UserCfg;

typedef struct {
	u32 SubsystemId;	/**< Subsystem Id */
	XCert_UserCfg UserCfg;	/**< User configuration */
	XCert_SignStore SignStore; /**< Signature store */
} XCert_InfoStore;

typedef struct {
	u32 IsSelfSigned;	/**< Flag to check if self-signed certificate */
	u32 IsCsr;		/**< Flag to check if Certificate Signing Request */
	u8* SubjectPublicKey;	/**< Subject Public Key */
	u8* IssuerPrvtKey;	/**< Issuer Private Key */
	u8* IssuerPublicKey;	/**< Issuer Public Key */
	u8* FwHash;		/**< Firmware Hash */
}XCert_AppCfg;

typedef struct {
	u32 SubSystemId;	/**< Subsystem ID */
	XCert_UserCfg *UserCfg;		/**< Configuration from User */
	XCert_AppCfg AppCfg;	/**< Configuration from application */
}XCert_Config;

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
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType, u8* Val, u32 Len);

#endif
#ifdef __cplusplus
}
#endif  /* XCERT_GENX509CERT_H */
/* @} */
