/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcert_genx509cert.c
*
* This file contains the implementation of the interface functions for creating
* X.509 certificate for DevIK and DevAK public key.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   har  01/09/2023 Initial release
* 1.1   am   08/18/2023 Renamed error codes which starts with XOCP with XCERT
* 1.2   har  10/31/2023 Add support to use DevIk subject as DevAk Issuer if DevIk user config is
*			available
*       kpt  10/26/2023 Add support to run KAT
*       har  12/08/2023 Add support for Subject Alternative Name field
* 1.2   am   01/31/2024 Moved entire file under PLM_OCP_KEY_MNGMT macro
*       kpt  02/21/2024 Add support for DME extension
*       har  03/25/2024 Fix calculation of hash for serial
* 1.3   har  05/02/2024 Added doxygen grouping and tags
*			Fixed doxygen warnings
*       har  06/07/2024 Added support to store and get user config for key index
*	kal  07/24/2024 Code refactoring updates for versal_2ve_2vm
*       har  08/08/2024 Added TCB Info extension in DevIk CSR
*       har  08/23/2024 Removed HwType field in Extended Key usage extension for Versal Gen2 devices
*       har  09/17/2024 Fixed doxygen warnings
*       kpt  11/19/2024 Add UTF8 encoding support for version field
* 1.4   har  02/27/2025 Use SHA1 to calculate hash of uncompressed public key for SKI/AKI
*       tvp  05/16/2025 Use SHA3 for Versal_2vp
*       tvp  06/05/2025 Remove use of UEID and TCB Info extension for Versal_2vp
*       tvp  09/18/2025 Remove use of DME extension for Versal_2vp
* 1.6   vm   12/18/2025 Add validation for certificate size
*       rpu  01/17/2026 Add Layer field to TCB Info extension
*       rmv  01/30/2026 Renamed OCP keymanagement macro
*       tbk  05/03/2026 Add validation for xcert remaining buffer before writing field
*
* </pre>
* @note
*
******************************************************************************/
/**
 * @addtogroup xcert_apis XilCert APIs
 * @{
 */
/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_NATIVE_KEY_MGMT
#include "xsecure_ellipticplat.h"
#include "xcert_sha.h"
#include "xcert_genx509cert.h"
#include "xcert_createfield.h"
#include "xplmi.h"
#include "xplmi_status.h"
#include "xplmi_tamper.h"
#include "xsecure_init.h"
#include "xplmi_dma.h"
#include "xsecure_kat.h"

/************************** Constant Definitions *****************************/
/** @name Object IDs
 * @{
 */
 /**< Object IDs used in X.509 Certificate and Certificate Signing Request */
#ifdef VERSAL_2VP
static const u8 Oid_SignAlgo[]		= {0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U, 0x03U, 0x0BU};
#else
static const u8 Oid_SignAlgo[]		= {0x06U, 0x08U, 0x2AU, 0x86U, 0x48U, 0xCEU, 0x3DU, 0x04U, 0x03U, 0x03U};
#endif
static const u8 Oid_EcPublicKey[]	= {0x06U, 0x07U, 0x2AU, 0x86U, 0x48U, 0xCEU, 0x3DU, 0x02U, 0x01U};
static const u8 Oid_P384[]		= {0x06U, 0x05U, 0x2BU, 0x81U, 0x04U,0x00U, 0x22U};
static const u8 Oid_SubKeyIdentifier[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x0EU};
static const u8 Oid_AuthKeyIdentifier[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x23U};
#ifndef VERSAL_2VP
static const u8 Oid_TcbInfoExtn[]	= {0x06U, 0x06U, 0x67U, 0x81U, 0x05U, 0x05U, 0x04U, 0x01U};
static const u8 Oid_UeidExtn[]		= {0x06U, 0x06U, 0x67U, 0x81U, 0x05U, 0x05U, 0x04U, 0x04U};
#endif
static const u8 Oid_KeyUsageExtn[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x0FU};
static const u8 Oid_EkuExtn[]		= {0x06U, 0x03U, 0x55U, 0x1DU, 0x25U};
static const u8 Oid_EkuClientAuth[]	= {0x06U, 0x08U, 0x2BU, 0x06U, 0x01U, 0x05U, 0x05U, 0x07U, 0x03U, 0x02U};
#ifndef VERSAL_2VE_2VM
static const u8 Oid_EkuHwType[]		= {0x06U, 0x0BU, 0x2BU, 0x06U, 0x01U, 0x04U, 0x01U, 0x82U, 0x37U, 0x66U, 0x01U, 0x0CU, 0x01U};
#endif
static const u8 Oid_BasicConstraintExtn[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x13U};
static const u8 Oid_ExtnRequest[]	= {0x06U, 0x09U, 0x2AU, 0x86U, 0x48U, 0x86U, 0xF7U, 0x0DU, 0x01U, 0x09U, 0x0EU};
#ifndef VERSAL_2VP
static const u8 Oid_Sha3_384[]		= {0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U, 0x02U, 0x09U};
#endif
#ifndef VERSAL_2VP
static const u8 Oid_DmeExtn[] = {0x06U, 0x0AU, 0x2BU, 0x06U, 0x01U, 0x04U, 0x01U, 0x82U, 0x37U, 0x66U, 0x03U, 0x01U};
static const u8 Oid_DmeStructExtn[] = {0x06U, 0x0BU, 0x2BU, 0x06U, 0x01U, 0x04U, 0x01U, 0x82U, 0x37U, 0x66U, 0x03U, 0x02U, 0x02};
#endif
/** @} */

/************************** Macro Definitions *****************************/
#define XCERT_RTCA_SPK_ID_ADDR				(0xF2014364U)
			/**< Address of the SPK ID stored in RTCA */
#define XCERT_RTCA_PLM_VERSION_ADDR			(0xF2014320U)
			/**< Address of the PLM version stored in RTCA */

#define XCERT_PMC_SUBSYSTEM_ID				(0x1C000001U)
			/**< PMC Subsystem ID*/

#define XCERT_SERIAL_FIELD_LEN				(22U)
			/**< Length of Serial Field */
#define XCERT_BIT7_MASK 				(0x80U)
			/**< Mask to get bit 7*/
#define XCERT_SIGN_AVAILABLE				(0x3U)
			/**< Signature available in SignStore */
#define XCERT_BYTE_MASK					(0xFFU)
			/**< Mask to get byte */
#define XCERT_MAX_CERT_SUPPORT				(5U)
	/**< Number of supported certificates is 4 -> 1 DevIK certificate and 4 DevAK certificates */
#define XCERT_SUB_KEY_ID_VAL_LEN			(20U)
			/**< Length of value of Subject Key ID */
#define XCERT_AUTH_KEY_ID_VAL_LEN			(20U)
			/**< Length of value of Authority Key ID */
#define XCERT_UNCOMPRESSED_PUB_KEY			(0x04U)
			/**< To indicate uncompressed public key */
#define XCERT_MAX_LEN_OF_KEYUSAGE_VAL			(2U)
			/**< Maximum length of value of key usage */
#define XCERT_LEN_OF_BYTE_IN_BITS			(8U)
			/**< Length of byte in bits */
#define XCERT_AUTH_KEY_ID_OPTIONAL_PARAM		(0x80U)
			/**< Optional parameter in Authority Key Identifier field*/
#define XOCP_APP_VERSION_MAX_LENGTH			(64U)
							/**< Maximum length of app version in bytes */
#define XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN	(XCERT_ECC_P384_PUBLIC_KEY_LEN + 1U)
							/**< Length of uncompressed ECC public key */

#define XCERT_MAX_CERT_SIZE			(2000U) /**< Maximum certificate size */

/** @name Optional parameter tags
 * @{
 */
 /**< Tags for optional parameters */
#define XCERT_OPTIONAL_PARAM_0_CONSTRUCTED_TAG			(0xA0U)
#define XCERT_OPTIONAL_PARAM_3_CONSTRUCTED_TAG			(0xA3U)
#define XCERT_OPTIONAL_PARAM_6_CONSTRUCTED_TAG			(0xA6U)

#define XCERT_OPTIONAL_PARAM_2_PRIMITIVE_TAG	(0x82U)
#define XCERT_OPTIONAL_PARAM_3_PRIMITIVE_TAG	(0x83U)
#define XCERT_OPTIONAL_PARAM_4_PRIMITIVE_TAG	(0x84U)

/** @} */

/** @name DNA
 * @{
 */
 /**< Macros related to Address of DNA and length of DNA in words and bytes */
#define XCERT_DNA_0_ADDRESS				(0xF1250020U)
#define XCERT_DNA_LEN_IN_WORDS				(4U)
#define XCERT_DNA_LEN_IN_BYTES				(XCERT_DNA_LEN_IN_WORDS * XCERT_WORD_LEN)
/** @} */

#ifndef VERSAL_2VP
#define XCERT_DME_PUB_KEY_X_0	    (0xF1115400U)	/**< DME public key X address */
#define XCERT_DME_PUB_KEY_Y_0       (0xF1115430U)   /**< DME public key Y address */
#endif

#define XCert_In32					(XPlmi_In32)
			/**< Alias of XPlmi_In32 to be used in XilCert*/

/** @name Layer in TCB info extension
 * @{
 */
 /**< Macros related to Layer sub-field in TCB info extension */
#define XCERT_LAYER_0				(0U)	/**< DevIK certificate */
#define XCERT_LAYER_1				(1U)	/**< DevAk certificate */

/************************** Type Definitions ******************************/
typedef enum {
	XCERT_DIGITALSIGNATURE,	/**< Digital Signature */
	XCERT_NONREPUDIATION,	/**< Non Repudiation */
	XCERT_KEYENCIPHERMENT,	/**< Key Encipherment */
	XCERT_DATAENCIPHERMENT,	/**< Data Encipherment */
	XCERT_KEYAGREEMENT,	/**< Key Agreement */
	XCERT_KEYCERTSIGN,	/**< Key Certificate Sign */
	XCERT_CRLSIGN,		/**< CRL Sign */
	XCERT_ENCIPHERONLY,	/**< Encipher Only */
	XCERT_DECIPHERONLY	/**< Decipher Only */
} XCert_KeyUsageOption;

/************************** Function Prototypes ******************************/
static XCert_InfoStore *XCert_GetCertDB(void);
static u32 *XCert_GetNumOfEntriesInUserCfgDB(void);
static int XCert_IsBufferNonZero(const u8 *Buffer, u32 BufferLen);
static int XCert_GetUserCfg(u32 SubsystemId, u32 KeyIndex, XCert_UserCfg **UserCfg);
static int XCert_UpdateUserCfg(XCert_Config *Cfg);
static int XCert_GenVersionField(const XCert_Config *Cfg);
static int XCert_GenSerialField(u8 *TBSCertBuf, u8 *DataHash);
static int XCert_GenSignAlgoField(void);
static inline int XCert_GenIssuerField(const u8 *Issuer, const u32 IssuerValLen);
static inline int XCert_GenValidityField(const u8 *Validity, const u32 ValidityValLen);
static inline int XCert_GenSubjectField(const u8 *Subject, const u32 SubjectValLen);
static int XCert_GenPubKeyAlgIdentifierField(void);
static int XCert_GenPublicKeyInfoField(const u8 *SubjectPublicKey);
static int XCert_GenSignField(const u8 *Signature);
static int XCert_GetSignStored(u32 SubsystemId, XCert_SignStore **SignStore);
static int XCert_GenTBSCertificate(XCert_Config *Cfg);
static void XCert_CopyCertificate(const u32 Size, const u8 *Src, const u64 DstAddr);
static int XCert_GenSubjectKeyIdentifierField(const u8 *SubjectPublicKey);
static int XCert_GenAuthorityKeyIdentifierField(const u8 *IssuerPublicKey);
#ifndef VERSAL_2VP
static int XCert_GenTcbInfoExtnField(XCert_Config *Cfg);
static int XCert_GenUeidExnField(void);
#endif
static void XCert_UpdateKeyUsageVal(u8 *KeyUsageVal, XCert_KeyUsageOption KeyUsageOption);
static int XCert_GenKeyUsageField(const XCert_Config *Cfg);
static int XCert_GenExtKeyUsageField(const XCert_Config *Cfg);
static inline int XCert_GenSubAltNameField(const u8 *SubAltName, const u32 SubAltNameValLen);
static int XCert_GenX509v3ExtensionsField(XCert_Config *Cfg);
static int XCert_GenBasicConstraintsExtnField(void);
static int XCert_GenCsrExtensions(XCert_Config *Cfg);
static int XCert_GenCertReqInfo(XCert_Config *Cfg);
#ifndef VERSAL_2VP
static int XCert_GenDmeExtnField(u32 *Len, const XCert_DmeResponse * DmeResp);
static int XCert_GenDmePublicKeyAndStructExtnField(u32 *Len, const XCert_DmeChallenge *Dme);
#endif
#if (!defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP))
static int XCert_GenLayerField(XCert_Config* Cfg);
static int XCert_GenFwVersionField(XCert_Config *Cfg);
static int XCert_GenSecurityVersionField(void);
#endif


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the static variable SpkId
 *
 * @return
 *		 - Pointer to the static variable SPK ID.
 *
 *****************************************************************************/
u32 *XCert_GetSpkId(void)
{
	static u32 SpkId;

	return &SpkId;
}

/*****************************************************************************/
/**
 * @brief	This function creates the X.509 Certificate/Certificate Signing Request(CSR)
 *
 * @param	X509CertAddr	Address of the X.509 Certificate buffer
 * @param	MaxCertSize	Maximum size of the X.509 Certificate buffer
 * @param	X509CertSize	Size of X.509 Certificate in bytes
 * @param	Cfg		Pointer to structure which includes configuration for the X.509 Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated X.509 certificate/CSR
 *		 - XST_INVALID_PARAM  Invalid function arguments
 *		 - XCERT_ERR_X509_KAT_FAILED  Failure of SHA384 KAT
 *		 - XCERT_ERR_X509_GEN_TBSCERT_DIGEST  Failure in SHA 384 digest calculation for TBS certificate
 *		 - XCERT_ERR_X509_GET_SIGN  Failure in getting stored signature
 *		 - XSECURE_KAT_MAJOR_ERROR  Failure in Sign Generate KAT
 *		 - XCERT_ERR_X509_CALC_SIGN  Failure in generating ephemeral key and signature
 *		 - XCERT_ERR_X509_UPDATE_ENCODED_LEN  Failure in updating encoded length
 *		 - XCERT_ERR_X509_INSUFFICIENT_MEMORY  Insufficient buffer provided for X.509 cert.
 *		 - XST_FAILURE  In case of failure
 *
 * @note	Certificate  ::=  SEQUENCE  {
 *			tbsCertificate       TBSCertificate,
 *			signatureAlgorithm   AlgorithmIdentifier,
 *			signatureValue       BIT STRING  }
 *
 ******************************************************************************/
int XCert_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32 *X509CertSize, XCert_Config *Cfg)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u32 TbsCertDataLen = 0U;
	u8 X509CertBuf[XCERT_MAX_CERT_SIZE];
	int HashCmpStatus = XST_FAILURE;
	u8 Sign[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 SignTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	XCert_SignStore *SignStore = NULL;
	u8 HashTmp[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	u8 *TbsCertStart;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	if (Cfg == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Zero-initialize the certificate buffer before use */
	Status = Xil_SMemSet(X509CertBuf, XCERT_MAX_CERT_SIZE, 0U, XCERT_MAX_CERT_SIZE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	X509CertInfo->Buff = &X509CertBuf[0];
	X509CertInfo->CurrOffset = X509CertInfo->Buff;
	X509CertInfo->RemainingBytes = XCERT_MAX_CERT_SIZE;

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;	}

	Status = XCert_UpdateUserCfg(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	TbsCertStart = X509CertInfo->CurrOffset;
	/**
	 * SHA 384 is used to calculate hash for Serial field and to calculate hash
	 * for signature calculation. So run KAT for SHA384 before use
	 */

	if (XPlmi_IsKatRan(XOCP_SECURE_SHA_KAT_MASK) != (u32)TRUE) {
		Status = XCert_ShaKat();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Cfg->AppCfg.IsCsr == TRUE) {
		Status = XCert_GenCertReqInfo(Cfg);
	}
	else {
		Status = XCert_GenTBSCertificate(Cfg);
	}

	TbsCertDataLen = XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, TbsCertStart);

	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Generate Sign Algorithm field
	 */
	Status = XCert_GenSignAlgoField();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Calculate SHA 384 Digest of the TBS certificate
	 */
	Status = XCert_ShaDigest(TbsCertStart, TbsCertDataLen, HashTmp);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_DIGEST;
		goto END;
	}
	/**
	 * Get the TBS certificate signature stored in Cert DB
	 */
	Status = XCert_GetSignStored(Cfg->SubSystemId, &SignStore);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GET_SIGN;
		goto END;
	}
	/**
	 * If the Signature is available, compare the hash stored with
	 * the hash calculated to make sure nothing is changed.
	 * if hash matches, copy the stored signature else generate
	 * the signature again.
	 */
	if (SignStore->IsSignAvailable == XCERT_SIGN_AVAILABLE) {
		HashCmpStatus = Xil_SMemCmp((void *)HashTmp, sizeof(HashTmp),
				(void *)SignStore->Hash, sizeof(SignStore->Hash),
				sizeof(HashTmp));
		if (HashCmpStatus == XST_SUCCESS) {
			Status = Xil_SMemCpy((void *)Sign, sizeof(Sign),
				(void *)SignStore->Sign, sizeof(SignStore->Sign),
				sizeof(Sign));
			if (Status != XST_SUCCESS) {
				goto END;
			}
		}
	}

	/**
	 * If the Signature is not available or Hash comparison from above fails,
	 * regenerate the signature.
	 */
	if ((SignStore->IsSignAvailable != XCERT_SIGN_AVAILABLE) || (HashCmpStatus != XST_SUCCESS)) {
		if (XPlmi_IsKatRan(XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK) != TRUE) {
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XSECURE_KAT_MAJOR_ERROR, Status, StatusTmp,
				XSecure_EllipticSignGenerateKat, XSECURE_ECC_PRIME);
			if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
				goto END;
			}
			XPlmi_SetKatMask(XPLMI_SECURE_ECC_SIGN_GEN_SHA3_384_KAT_MASK);
		}

		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)Hash,
					(u64)(UINTPTR)HashTmp);
		/**
		 * Calculate signature of the TBS certificate using the private key
		 */
		Status = XSecure_EllipticGenEphemeralNSign(XSECURE_ECC_NIST_P384, (const u8 *)Hash, sizeof(Hash),
				Cfg->AppCfg.IssuerPrvtKey, SignTmp);
		if (Status != XST_SUCCESS) {
			Status = (int)XCERT_ERR_X509_CALC_SIGN;
			goto END;
		}
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				(u64)(UINTPTR)Sign, (u64)(UINTPTR)SignTmp);
		XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				(u64)(UINTPTR)(Sign + XSECURE_ECC_P384_SIZE_IN_BYTES),
				(u64)(UINTPTR)(SignTmp + XSECURE_ECC_P384_SIZE_IN_BYTES));
		/**
		 * Copy the generated signature and hash into SignStore
		 */
		Status = Xil_SMemCpy(SignStore->Hash, sizeof(SignStore->Hash),
				HashTmp, sizeof(HashTmp), sizeof(HashTmp));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = Xil_SMemCpy(SignStore->Sign, sizeof(SignStore->Sign),
				Sign, sizeof(Sign), sizeof(Sign));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		SignStore->IsSignAvailable = XCERT_SIGN_AVAILABLE;
	}
	/**
	 * Generate Signature field
	 */
	Status = XCert_GenSignField(Sign);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Update the encoded length in the X.509 certificate SEQUENCE
	 */
	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
		XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
		SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}

	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	/**
	 * Validate the certificate size and copy the certificate to user buffer
	 */
	if ((u32)(X509CertInfo->CurrOffset - X509CertInfo->Buff) <= MaxCertSize) {
		*X509CertSize = (u32)(X509CertInfo->CurrOffset - X509CertInfo->Buff);
		XCert_CopyCertificate(*X509CertSize, (u8 *)X509CertInfo->Buff, X509CertAddr);
	} else {
		Status = (int)XCERT_ERR_X509_INSUFFICIENT_MEMORY;
	}
END:
	return Status;
}

/**
 * @cond xcert_internal
 * @{
 */
/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the X509 InfoStore database.
 *
 * @return
 *		 - Pointer to the first entry in InfoStore database
 *
 * @note	InfoStore DB is used to store the user configurable fields of
 * 		X.509 certificate, hash and signature of the TBS Ceritificate
 * 		for different subsystems.
 *		Each entry in the DB will have following fields:
 *		- Subsystem Id
 *		- Issuer
 *		- Subject
 *		- Validity
 *		- Signature
 *		- Hash
 *		- IsSignAvailable
 *
 ******************************************************************************/
static XCert_InfoStore *XCert_GetCertDB(void)
{
	static XCert_InfoStore CertDB[XCERT_MAX_CERT_SUPPORT] = {0U};

	return &CertDB[0];
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the NumOfEntriesInUserCfgDB
*		which indicates the total number of subsystems for which
*		user configuration is stored in UsrCfgDB.
 *
 * @return
 *		 - Pointer to the NumOfEntriesInUserCfgDB
 *
 ******************************************************************************/
static u32 *XCert_GetNumOfEntriesInUserCfgDB(void)
{
	static u32 NumOfEntriesInUserCfgDB = 0U;

	return &NumOfEntriesInUserCfgDB;
}

/*****************************************************************************/
/**
 * @brief	This function checks if all the bytes in the provided buffer are
 *		zero.
 *
 * @param	Buffer		Pointer to the buffer
 * @param	BufferLen	Length of the buffer
 *
 * @return
 *		 - XST_SUCCESS  If buffer is non-empty
 *		 - XST_FAILURE  If buffer is empty
 *
 ******************************************************************************/
static int XCert_IsBufferNonZero(const u8 *Buffer, u32 BufferLen)
{
	volatile int Status = XST_FAILURE;
	u32 Sum = 0;
	volatile u32 Idx;

	for (Idx = 0; Idx < BufferLen; Idx++) {
		Sum |= Buffer[Idx];
	}
	if (Idx != BufferLen) {
		goto END;
	}

	if (Sum != 0U) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function finds the provided Subsystem ID and Key Index in
 * 		Certificate DB and returns the pointer to the corresponding entry in DB
 *		if all the other fields are valid.
 *
 * @param	SubsystemId	Subsystem ID for which user configuration is requested
 * @param	KeyIndex	Index of the key for given subsystem ID
 * @param	UserCfg		Pointer to the entry in DB for the provided Subsystem ID
 *
 * @return
 *		 - XST_SUCCESS  If subsystem ID is found and other fields are valid
 *		 - XCERT_ERR_X509_INVALID_USER_CFG  User configuration for given subsystem ID is invalid
 *		 - XCERT_ERR_X509_USR_CFG_NOT_FOUND  User configuration for given subsystem ID is not found
 *		 - XST_FAILURE  Upon any failure
 *
 ******************************************************************************/
static int XCert_GetUserCfg(u32 SubsystemId, u32 KeyIndex, XCert_UserCfg **UserCfg)
{
	int Status = XST_FAILURE;
	XCert_InfoStore *CertDB = XCert_GetCertDB();
	u32 *NumOfEntriesInUserCfgDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	/**
	 * Search for given Subsystem ID and KeyIndex  in the UserCfg DB
	 */
	for (Idx = 0; Idx < *NumOfEntriesInUserCfgDB; Idx++) {
		if ((CertDB[Idx].SubsystemId == SubsystemId) && (CertDB[Idx].KeyIndex == KeyIndex)) {
			/**
			 * If Subsystem ID is found then check that Subject,
			 * Issuer and Validity for that Subsystem ID is non-zero.
			 */
			Status = XCert_IsBufferNonZero(CertDB[Idx].UserCfg.Issuer,
				CertDB[Idx].UserCfg.IssuerLen);
			if (Status != XST_SUCCESS) {
				Status = (int)XCERT_ERR_X509_INVALID_USER_CFG;
				goto END;
			}

			Status = XCert_IsBufferNonZero(CertDB[Idx].UserCfg.Subject,
				CertDB[Idx].UserCfg.SubjectLen);
			if (Status != XST_SUCCESS) {
				Status = (int)XCERT_ERR_X509_INVALID_USER_CFG;
				goto END;
			}

			Status = XCert_IsBufferNonZero(CertDB[Idx].UserCfg.Validity,
				CertDB[Idx].UserCfg.ValidityLen);
			if (Status != XST_SUCCESS) {
				Status = (int)XCERT_ERR_X509_INVALID_USER_CFG;
				goto END;
			}
			*UserCfg = &CertDB[Idx].UserCfg;
			Status = XST_SUCCESS;
			goto END;
		}
	}

	if (Idx == *NumOfEntriesInUserCfgDB) {
		Status = (int)XCERT_ERR_X509_USR_CFG_NOT_FOUND;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function updates the user configuration to be used for
 * 		generating X.509 certificate
 * 		For DevIK certificate and DevIK CSR, it gets the user
 * 		configuration from the DB
 * 		For DevAK certificates, it gets the user cfg from DB. If DevIK
 * 		user cfg is available in DB then DevIK Subject shall be used as
 * 		Issuer in DevAK certificate
 *
 * @param	Cfg	Pointer to the configuration to be used for generating certificate
 *
 * @return
 *		 - XST_SUCCESS  User Cfg is successfully updated
 *		 - XST_FAILURE  Upon any failure
 *
 ******************************************************************************/
static int XCert_UpdateUserCfg(XCert_Config* Cfg)
{
	int Status = XST_FAILURE;
	XCert_UserCfg* DevIKUserCfg;

	Status = XCert_GetUserCfg(Cfg->SubSystemId, Cfg->KeyIndex, &Cfg->UserCfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Cfg->AppCfg.IsSelfSigned == FALSE) {
		Status = XCert_GetUserCfg(XCERT_PMC_SUBSYSTEM_ID, 0U, &DevIKUserCfg);
		if (Status == XST_SUCCESS) {
			Status = Xil_SMemSet(Cfg->UserCfg->Issuer, XCERT_ISSUER_MAX_SIZE, 0U,
				Cfg->UserCfg->IssuerLen);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Cfg->UserCfg->IssuerLen = 0U;

			Status = Xil_SMemCpy(Cfg->UserCfg->Issuer, XCERT_ISSUER_MAX_SIZE,
				DevIKUserCfg->Subject, XCERT_SUBJECT_MAX_SIZE,
				DevIKUserCfg->SubjectLen);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			Cfg->UserCfg->IssuerLen = DevIKUserCfg->SubjectLen;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function finds the provided Subsystem ID in InfoStore DB and
 *		returns the pointer to the corresponding sign entry in DB.
 *
 * @param	SubsystemId	Subsystem ID for which stored signature is requested
 * @param	SignStore	Pointer to the entry in DB for the provided Subsystem ID
 *
 * @return
 *		 - XST_SUCCESS  If subsystem ID is found
 *		 - XST_FAILURE  Upon any failure
 *
 ******************************************************************************/
static int XCert_GetSignStored(u32 SubsystemId, XCert_SignStore **SignStore)
{
	int Status = XST_FAILURE;
	XCert_InfoStore *CertDB = XCert_GetCertDB();
	u32 *NumOfEntriesInCertDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	/**
	 * Search for given Subsystem ID in the CertDB
	 */
	for (Idx = 0; Idx < *NumOfEntriesInCertDB; Idx++) {
		if (CertDB[Idx].SubsystemId == SubsystemId) {
			*SignStore = &CertDB[Idx].SignStore;
			Status = XST_SUCCESS;
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the user provided value for the user configurable
 *		fields in the certificate as per the provided FieldType.
 *
 * @param	SubSystemId	Id of subsystem for which field data is provided
 * @param	FieldType	To identify the field for which input is provided
 * @param	Val		Value of the field provided by the user
 * @param	Len		Length of the value in bytes
 * @param	KeyIndex	Index of the key for given subsystem ID
 *
 * @return
 *		 - XST_SUCCESS  If whole operation is success
 *		 - XST_INVALID_PARAM  Failure due to invalid arguments
 *		 - XOCP_ERR_X509_USER_CFG_STORE_LIMIT_CROSSED  Exceed maximum limit of user configuration
 *		 - XST_FAILURE  Upon any failure
 *
 ******************************************************************************/
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType,
						u8 *Val, u32 Len, u32 KeyIndex)
{
	int Status = XST_FAILURE;
	u32 IdxToBeUpdated;
	u32 IsSubsystemIdPresent = FALSE;
	XCert_InfoStore *CertDB = XCert_GetCertDB();
	u32 *NumOfEntriesInUserCfgDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	if (FieldType > XCERT_SUBALTNAME) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (((FieldType == XCERT_VALIDITY) && (Len > XCERT_VALIDITY_MAX_SIZE)) ||
		((FieldType == XCERT_ISSUER) && (Len > XCERT_ISSUER_MAX_SIZE)) ||
		((FieldType == XCERT_SUBJECT) && (Len > XCERT_SUBJECT_MAX_SIZE)) ||
		((FieldType == XCERT_SUBALTNAME) && (Len > XCERT_SUB_ALT_NAME_MAX_SIZE))) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Look for the Subsystem Id. If it is there get the index and update the
	 * field of existing subsystem else increment index and add entry for
	 * new subsystem.
	*/
	for (Idx = 0; Idx < *NumOfEntriesInUserCfgDB; Idx++) {
		if ((CertDB[Idx].SubsystemId == SubSystemId) && (CertDB[Idx].KeyIndex == KeyIndex)) {
			IdxToBeUpdated = Idx;
			IsSubsystemIdPresent = TRUE;
			break;
		}
	}

	if (IsSubsystemIdPresent == (u8)FALSE) {
		IdxToBeUpdated = *NumOfEntriesInUserCfgDB;
		if (IdxToBeUpdated >= XCERT_MAX_CERT_SUPPORT) {
			Status = (int)XOCP_ERR_X509_USER_CFG_STORE_LIMIT_CROSSED;
			goto END;
		}
		CertDB[IdxToBeUpdated].SubsystemId = SubSystemId;
		CertDB[IdxToBeUpdated].KeyIndex = KeyIndex;
		*NumOfEntriesInUserCfgDB = (*NumOfEntriesInUserCfgDB) + 1U;
	}

	if (FieldType == XCERT_ISSUER) {
		XSecure_MemCpy(CertDB[IdxToBeUpdated].UserCfg.Issuer, Val, Len);
		CertDB[IdxToBeUpdated].UserCfg.IssuerLen = Len;
	}
	else if (FieldType == XCERT_SUBJECT) {
		XSecure_MemCpy(CertDB[IdxToBeUpdated].UserCfg.Subject, Val, Len);
		CertDB[IdxToBeUpdated].UserCfg.SubjectLen = Len;
	}
	else if (FieldType == XCERT_VALIDITY){
		XSecure_MemCpy(CertDB[IdxToBeUpdated].UserCfg.Validity, Val, Len);
		CertDB[IdxToBeUpdated].UserCfg.ValidityLen = Len;
	}
	else {
		XSecure_MemCpy(CertDB[IdxToBeUpdated].UserCfg.SubAltName, Val, Len);
		CertDB[IdxToBeUpdated].UserCfg.SubAltNameLen = Len;
		CertDB[IdxToBeUpdated].UserCfg.IsSubAltNameAvailable = TRUE;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Version field of the TBS Certificate.
 *
 * @param	Cfg	Pointer to structure which includes configuration for the TBS Certificate.
 *
 * @return
 *		- XST_SUCCESS  Successfully generated Version field
 *		- Error  In case of failure
 *
 * @note	Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
 *		This field describes the version of the encoded certificate.
 *		XilCert library supports X.509 V3 certificates. For Certificate
 *		Signing Request, the supported version is V1
 *
 ******************************************************************************/
static int XCert_GenVersionField(const XCert_Config *Cfg)
{
	int Status = XST_FAILURE;

	Status  = XCert_UpdateByteField(XCERT_ASN1_TAG_INTEGER);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status  = XCert_UpdateByteField(XCERT_LEN_OF_VALUE_OF_VERSION);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Cfg->AppCfg.IsCsr != TRUE) {
		Status  = XCert_UpdateByteField(XCERT_VERSION_VALUE_V3);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
	else {
		Status  = XCert_UpdateByteField(XCERT_VERSION_VALUE_V1);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Serial field of the TBS Certificate.
 *
 * @param	TBSCertBuf	Pointer in the TBS Certificate buffer where
 *		the Serial field shall be added.
 * @param	DataHash	Hash which is to be used as value in Serial field
 * @param	SerialLen	Length of the Serial field
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Serial field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	CertificateSerialNumber  ::=  INTEGER
 *		The length of the serial must not be more than 20 bytes.
 *		The value of the serial is determined by calculating the
 *		SHA2 hash of the fields in the TBS Certificate except the Version
 *		and Serial Number fields. 20 bytes from LSB of the calculated
 *		hash is updated as the Serial Number
 *
 ******************************************************************************/
static int XCert_GenSerialField(u8 *TBSCertBuf, u8 *DataHash)
{
	int Status = XST_FAILURE;
	u8 Serial[XCERT_LEN_OF_VALUE_OF_SERIAL] = {0U};
	u32 LenToBeCopied;
	u8 *TempDataBuff;
	u32 TempRemainingDataBuffLen;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	/**
	 * The value of serial field must be 20 bytes. If the most significant
	 * bit in the first byte of Serial is set, then the value shall be
	 * prepended with 0x00 after DER encoding.
	 * So if MSB is set then 00 followed by 19 bytes of hash will be the serial
	 * value. If not set then 20 bytes of hash will be used as serial value.
	 */
	if ((*DataHash & XCERT_BIT7_MASK) == XCERT_BIT7_MASK) {
		LenToBeCopied = XCERT_LEN_OF_VALUE_OF_SERIAL - 1U;
	}
	else {
		LenToBeCopied = XCERT_LEN_OF_VALUE_OF_SERIAL;
	}

	Status  = Xil_SMemCpy(Serial, XCERT_LEN_OF_VALUE_OF_SERIAL, DataHash,
		XCERT_LEN_OF_VALUE_OF_SERIAL, LenToBeCopied);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	TempDataBuff = X509CertInfo->CurrOffset;
	TempRemainingDataBuffLen = X509CertInfo->RemainingBytes;

	X509CertInfo->CurrOffset = TBSCertBuf;
	X509CertInfo->RemainingBytes = XCERT_LEN_OF_VALUE_OF_SERIAL +
					XCERT_SERIAL_DATA_HDR_LEN;

	Status = XCert_CreateInteger(Serial, LenToBeCopied);

	X509CertInfo->CurrOffset = TempDataBuff;
	X509CertInfo->RemainingBytes = TempRemainingDataBuffLen;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Signature Algorithm field. This field
 * 		is present in TBS Certificate as well as the X.509 certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully created Signature Algorithm field
 *		 - Error In case of failure
 *
 * @note	AlgorithmIdentifier  ::=  SEQUENCE  {
 *		algorithm		OBJECT IDENTIFIER,
 *		parameters		ANY DEFINED BY algorithm OPTIONAL}
 *
 *		This function supports only ECDSA with SHA-384 as
 *		 Signature Algorithm.
 *		The algorithm identifier for ECDSA with SHA-384 signature values is:
 *		ecdsa-with-SHA384 OBJECT IDENTIFIER ::= { iso(1) member-body(2)
 *		us(840) ansi-X9-62(10045) signatures(4) ecdsa-with-SHA2(3) 3 }
 *		The parameters field MUST be absent.
 *		Hence, the AlgorithmIdentifier shall be a SEQUENCE of
 *		one component: the OID ecdsa-with-SHA384.
 *		The parameter field must be replaced with NULL.
 *
 ******************************************************************************/
static int XCert_GenSignAlgoField(void)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_SignAlgo,
				sizeof(Oid_SignAlgo));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_ASN1_TAG_NULL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_NULL_VALUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Issuer field in TBS Certificate.
 *
 * @param	Issuer		DER encoded value of the Issuer field
 * @param	IssuerValLen	Length of the DER encoded value
 * @param	IssuerLen	Length of the Issuer field
 *
 * @return
 *		 - XST_SUCCESS  Successfully created Issuer field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	This function expects the user to provide the Issuer field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenIssuerField(const u8 *Issuer, const u32 IssuerValLen)
{
	return XCert_CreateRawDataFromByteArray(Issuer, IssuerValLen);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Validity field in TBS Certificate.
 *
 * @param	Validity	DER encoded value of the Validity field
 * @param	ValidityValLen	Length of the DER encoded value
 * @param	ValidityLen	Length of the Validity field
 *
 * @return
 *		 - XST_SUCCESS  Successfully created Validity field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	This function expects the user to provide the Validity field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenValidityField(const u8 *Validity, const u32 ValidityValLen)
{
	return XCert_CreateRawDataFromByteArray(Validity, ValidityValLen);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Subject field in TBS Certificate.
 *
 * @param	Subject		DER encoded value of the Subject field
 * @param	SubjectValLen	Length of the DER encoded value
 * @param	SubjectLen	Length of the Subject field
 *
 * @return
 *		 - XST_SUCCESS  Successfully created Subject field
 *		 - Error  In case of failure
 *
 * @note	This function expects the user to provide the Subject field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenSubjectField(const u8 *Subject, const u32 SubjectValLen)
{
	return XCert_CreateRawDataFromByteArray(Subject, SubjectValLen);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Public Key Algorithm Identifier sub-field. It
 *		is a part of Subject Public Key Info field present in
 *		TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Public Key Algorithm Identifier sub-field.
 *		 - Error  In case of failure
 *
 * @note	AlgorithmIdentifier  ::=  SEQUENCE  {
 *		algorithm		OBJECT IDENTIFIER,
 *		parameters		ANY DEFINED BY algorithm OPTIONAL}
 *
 *		This function supports only ECDSA P-384 key
 *		The algorithm identifier for ECDSA P-384 key is:
 *		{iso(1) member-body(2) us(840) ansi-x962(10045) keyType(2) ecPublicKey(1)}
 *		The parameter for ECDSA P-384 key is:
 *		secp384r1 OBJECT IDENTIFIER ::= {
 *		iso(1) identified-organization(3) certicom(132) curve(0) 34 }
 *
 *		Hence, the AlgorithmIdentifier shall be a SEQUENCE of
 *		two components: the OID id-ecPublicKey and OID secp384r1
 *
 ******************************************************************************/
static int XCert_GenPubKeyAlgIdentifierField(void)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_EcPublicKey, sizeof(Oid_EcPublicKey));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_P384, sizeof(Oid_P384));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Public Key Info field present in
 *		TBS Certificate.
 *
 * @param	SubjectPublicKey	Public key of the Subject for which the certificate is being
 * 		created.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Public Key Info field
 *		 - Error  In case of failure
 *
 * @note	SubjectPublicKeyInfo  ::=  SEQUENCE  {
			algorithm            AlgorithmIdentifier,
			subjectPublicKey     BIT STRING  }
 *
 ******************************************************************************/
static int XCert_GenPublicKeyInfoField(const u8 *SubjectPublicKey)
{
	int Status = XST_FAILURE;
	u32 KeyLen = XCERT_ECC_P384_PUBLIC_KEY_LEN;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 UncompressedPublicKey[XCERT_ECC_P384_PUBLIC_KEY_LEN + XCERT_BYTE_LEN] = {0U};
	const XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenPubKeyAlgIdentifierField();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */
	UncompressedPublicKey[0U] = XCERT_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], KeyLen + (1U),
							SubjectPublicKey, KeyLen, KeyLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBitString(UncompressedPublicKey, KeyLen + XCERT_BYTE_LEN, (u32)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the Subject Key Identifier field present in
 * 		TBS Certificate.
 *
 * @param	SubjectPublicKey	Public key whose hash will be used as Subject Key Identifier
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Subject Key Identifier field
 *		 - Error  In case of failure
 *
 * @note	SubjectKeyIdentifierExtension  ::=  SEQUENCE  {
 *		extnID      OBJECT IDENTIFIER,
 *		extnValue   OCTET STRING
 *		}
 *		To calculate value of SubjectKeyIdentifier field, hash is
 *		calculated on the Subject Public Key and 20 bytes from LSB of
 *		the hash is considered as the value for this field.
 *
 ******************************************************************************/
static int XCert_GenSubjectKeyIdentifierField(const u8 *SubjectPublicKey)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 UncompressedPublicKey[XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN];
#ifndef VERSAL_2VP
	u8 Hash[XSECURE_SHA1_HASH_SIZE];
#else
	XSecure_Sha3Hash Sha3Hash_Instance;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#endif
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */

	UncompressedPublicKey[0U] = XCERT_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
		SubjectPublicKey, XCERT_ECC_P384_PUBLIC_KEY_LEN, XCERT_ECC_P384_PUBLIC_KEY_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
			&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_SubKeyIdentifier,
					sizeof(Oid_SubKeyIdentifier));
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XSecure_Sha1Digest(UncompressedPublicKey, XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#else
	Status = XSecure_Sha3Digest(ShaInstPtr, (UINTPTR)UncompressedPublicKey,
				    XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, &Sha3Hash_Instance);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XCert_CreateOctetString(Hash, XCERT_SUB_KEY_ID_VAL_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#else
	Status = XCert_CreateOctetString(Sha3Hash_Instance.Hash, XCERT_SUB_KEY_ID_VAL_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the Authority Key Identifier field present in
 * 		TBS Certificate.
 *
 * @param	IssuerPublicKey		Public key whose hash will be
 *					used as Authority Key Identifier
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Authority Key Identifier field
 *		 - Error  In case of failure
 * @note
 * 		id-ce-authorityKeyIdentifier OBJECT IDENTIFIER ::=  { id-ce 35 }
 *
 *		AuthorityKeyIdentifier ::= SEQUENCE {
 *		keyIdentifier             [0] KeyIdentifier           OPTIONAL,
 *		authorityCertIssuer       [1] GeneralNames            OPTIONAL,
 *		authorityCertSerialNumber [2] CertificateSerialNumber OPTIONAL  }
 *
 *		KeyIdentifier ::= OCTET STRING
 *		To calculate value ofAuthorityKeyIdentifier field, hash is
 *		calculated on the Issuer Public Key and 20 bytes from LSB of
 *		the hash is considered as the value for this field.
 *
 ******************************************************************************/
static int XCert_GenAuthorityKeyIdentifierField(const u8 *IssuerPublicKey)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 *KeyIdSequenceLenIdx;
	u8 *KeyIdSequenceValIdx;
	u8 UncompressedPublicKey[XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN];
#ifndef VERSAL_2VP
	u8 Hash[XSECURE_SHA1_HASH_SIZE];
#else
	XSecure_Sha3Hash Sha3Hash_Instance;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
#endif
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */

	UncompressedPublicKey[0U] = XCERT_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
		IssuerPublicKey, XCERT_ECC_P384_PUBLIC_KEY_LEN, XCERT_ECC_P384_PUBLIC_KEY_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_AuthKeyIdentifier,
						sizeof(Oid_AuthKeyIdentifier));
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XSecure_Sha1Digest(UncompressedPublicKey, XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#else
	Status = XSecure_Sha3Digest(ShaInstPtr, (UINTPTR)UncompressedPublicKey,
				    XCERT_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, &Sha3Hash_Instance);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&KeyIdSequenceLenIdx, &KeyIdSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#ifndef VERSAL_2VP

	Status = XCert_CreateOctetString(Hash, XCERT_AUTH_KEY_ID_VAL_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#else
	Status = XCert_CreateOctetString(Sha3Hash_Instance.Hash, XCERT_SUB_KEY_ID_VAL_LEN);

	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif
	/**
	 * 0x80 indicates that the SEQUENCE contains the optional parameter tagged
	 * as [0] in the AuthorityKeyIdentifier sequence
	 */

	*KeyIdSequenceValIdx = XCERT_AUTH_KEY_ID_OPTIONAL_PARAM;
	*KeyIdSequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, KeyIdSequenceValIdx);
	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);



END:
	return Status;
}

#ifndef VERSAL_2VP
/******************************************************************************/
/**
 * @brief	This function creates the TCB Info Extension(2.23.133.5.4.1)
 * 		field present in TBS Certificate.
 *
 * @param	Cfg		Pointer to structure which includes configuration for the TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated TCB Info Extension field
 *		 - Error  In case of failure
 *
 * @note
 * 		tcg-dice-TcbInfo OBJECT IDENTIFIER ::= {tcg-dice 1}
 *		DiceTcbInfo ::== SEQUENCE {
 *			vendor [0] IMPLICIT UTF8String OPTIONAL,
 *			model [1] IMPLICIT UTF8String OPTIONAL,
 *			version [2] IMPLICIT UTF8String OPTIONAL,
 *			svn [3] IMPLICIT INTEGER OPTIONAL,
 *			layer [4] IMPLICIT INTEGER OPTIONAL,
 *			index [5] IMPLICIT INTEGER OPTIONAL,
 *			fwids [6] IMPLICIT FWIDLIST OPTIONAL,
 *			flags [7] IMPLICIT OperationalFlags OPTIONAL,
 *			vendorInfo [8] IMPLICIT OCTET STRING OPTIONAL,
 *			type [9] IMPLICIT OCTET STRING OPTIONAL
 *		}
 *		FWIDLIST ::== SEQUENCE SIZE (1..MAX) OF FWID
 *		FWID ::== SEQUENCE {
 *			hashAlg OBJECT IDENTIFIER,
 *			digest OCTET STRING
 *		}
 *
 * 		As per requirement, version, svn and fwids needs to be included in the extension.
 *		For DevIk certificates,
 *		- The value of version shall be the version of PLM
 *		- The value of svn shall be the SPK ID used during boot (assuming that the same SPK ID
 *		is used for all partitions)
 *		- The value of fwid shall be SHA3-384 hash of PLM and PMC CDO.
 *
 *		For DevAk certificates,
 *		- The value of version shall be provided via the user optional data in the PDI.
 *		- The value of svn shall be the SPK ID used during boot (assuming that the same SPK ID
 *		is used for all partitions)
 *		- The value of fwid shall be SHA3-384 hash of the application.
 *
 ******************************************************************************/
static int XCert_GenTcbInfoExtnField(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 *TcbInfoSequenceLenIdx;
	u8 *TcbInfoSequenceValIdx;
	u8 *OptionalTagLenIdx;
	u8 *OptionalTagValIdx;
	u8 *FwIdSequenceLenIdx;
	u8 *FwIdSequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_TcbInfoExtn, sizeof(Oid_TcbInfoExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&TcbInfoSequenceLenIdx, &TcbInfoSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VE_2VM
	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_2_PRIMITIVE_TAG,
					&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenFwVersionField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*OptionalTagLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OptionalTagValIdx);

	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_3_PRIMITIVE_TAG,
					&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenSecurityVersionField();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*OptionalTagLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OptionalTagValIdx);

	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_4_PRIMITIVE_TAG,
						&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenLayerField(Cfg);

	if (Status != XST_SUCCESS) {
		goto END;
	}

	*OptionalTagLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OptionalTagValIdx);

#endif
	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_6_CONSTRUCTED_TAG,
						&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
						&FwIdSequenceLenIdx, &FwIdSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_Sha3_384, sizeof(Oid_Sha3_384));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateOctetString(Cfg->AppCfg.FwHash, XCERT_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	*FwIdSequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, FwIdSequenceValIdx);
	*OptionalTagLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OptionalTagValIdx);

	Status = XCert_UpdateEncodedLength(TcbInfoSequenceLenIdx,
					XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset,
					TcbInfoSequenceValIdx), TcbInfoSequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*TcbInfoSequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
					(u32)(*TcbInfoSequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(OctetStrLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, OctetStrValIdx),
				(u8 *)OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*OctetStrLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*OctetStrLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				(u8 *)SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the UEID extension(2.23.133.5.4.4) field
 * 		present in TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated UEID Extension field
 *		 - Error  In case of failure
 *
 * @note	tcg-dice-Ueid OBJECT IDENTIFIER ::= {tcg-dice 4}
 *		TcgUeid ::== SEQUENCE {
 *			ueid OCTET STRING
 *		}
 *		The content of the UEID extension should contributes to the CDI
 *		which generated the Subject Key. Hence Device DNA is used as the
 *		value of this extension.
 *
 ******************************************************************************/
static int XCert_GenUeidExnField(void)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 *UeidSequenceLenIdx;
	u8 *UeidSequenceValIdx;
	u32 Dna[XCERT_DNA_LEN_IN_WORDS] = {0U};
	u32 Offset;
	u32 Address;
	u32 Idx;
	u32 Val;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_UeidExtn, sizeof(Oid_UeidExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&UeidSequenceLenIdx, &UeidSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	for (Idx = 0U; Idx < XCERT_DNA_LEN_IN_WORDS; Idx++) {
		Offset = XCERT_DNA_LEN_IN_WORDS - Idx - 1U;
		Address = XCERT_DNA_0_ADDRESS + (Offset * XCERT_WORD_LEN);
		Val = XCert_In32(Address);
		Dna[Idx] = Xil_EndianSwap32(Val);
	}

	Status = XCert_CreateOctetString((u8 *)Dna, XCERT_DNA_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*UeidSequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, UeidSequenceValIdx);
	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}
#endif


/******************************************************************************/
/**
 * @brief	This function updates the value of Key Usage extension field
 * 		present in TBS Certificate as per the KeyUsageOption.
 *
 * @param	KeyUsageVal	Pointer in the Key Usage value buffer where the Key Usage option
 * 		shall be updated.
 * @param	KeyUsageOption	Type of key usage which has to be updated
 *
 ******************************************************************************/
static void XCert_UpdateKeyUsageVal(u8 *KeyUsageVal, XCert_KeyUsageOption KeyUsageOption)
{
	u8 Idx = KeyUsageOption / XCERT_LEN_OF_BYTE_IN_BITS;
	u8 ShiftVal = (XCERT_LEN_OF_BYTE_IN_BITS * (Idx + 1U)) - (u8)KeyUsageOption - 1U;

	KeyUsageVal[Idx] |= 1U << ShiftVal;
}

/******************************************************************************/
/**
 * @brief	This function creates the Key Usage extension field present in
 * 		TBS Certificate.
 *
 * @param	Cfg		Pointer to structure which includes configuration for the TBS Certificate.
   *
 * @return
 *		 - XST_SUCCESS  Successfully generated Key Usage field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	id-ce-keyUsage OBJECT IDENTIFIER ::=  { id-ce 15 }
 *		KeyUsage ::= BIT STRING {
 *			digitalSignature        (0),
 *			nonRepudiation          (1),
 *			keyEncipherment         (2),
 *			dataEncipherment        (3),
 *			keyAgreement            (4),
 *			keyCertSign             (5),
 *			cRLSign                 (6),
 *			encipherOnly            (7),
 *			decipherOnly            (8)
 *		}
 *
 ******************************************************************************/
static int XCert_GenKeyUsageField(const XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 KeyUsageVal[XCERT_MAX_LEN_OF_KEYUSAGE_VAL] = {0U};
	u32 KeyUsageValLen;

	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE, &SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_KeyUsageExtn, sizeof(Oid_KeyUsageExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBoolean((u8)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		XCert_UpdateKeyUsageVal(KeyUsageVal, XCERT_KEYCERTSIGN);
	}
	else {
		XCert_UpdateKeyUsageVal(KeyUsageVal, XCERT_DIGITALSIGNATURE);
		XCert_UpdateKeyUsageVal(KeyUsageVal, XCERT_KEYAGREEMENT);
	}

	if ((KeyUsageVal[1U] & XCERT_BYTE_MASK) == 0U) {
		KeyUsageValLen = XCERT_MAX_LEN_OF_KEYUSAGE_VAL - 1U;
	}
	else {
		KeyUsageValLen = XCERT_MAX_LEN_OF_KEYUSAGE_VAL;
	}

	Status = XCert_CreateBitString(KeyUsageVal, KeyUsageValLen, (u32)FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the Extended Key Usage extension field
 * 		present in TBS Certificate.
 *
 * @param	Cfg	Pointer to structure which includes configuration for the TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Extended Key Usage field
 *		 - Error  In case of failure
 *
 * @note	id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 }
 *		ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
 *		KeyPurposeId ::= OBJECT IDENTIFIER
 *
 ******************************************************************************/
static int XCert_GenExtKeyUsageField(const XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 *EkuSequenceLenIdx;
	u8 *EkuSequenceValIdx;

#ifdef VERSAL_2VE_2VM
	(void)Cfg;
#endif
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_EkuExtn, sizeof(Oid_EkuExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBoolean((u8)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&EkuSequenceLenIdx, &EkuSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_EkuClientAuth, sizeof(Oid_EkuClientAuth));
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VE_2VM
	if (Cfg->AppCfg.IsCsr == TRUE) {
		Status = XCert_CreateRawDataFromByteArray(Oid_EkuHwType, sizeof(Oid_EkuHwType));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif

	*EkuSequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, EkuSequenceValIdx);
	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Subject Alternative Name extension field in TBS Certificate.
 *
 * @param	SubAltName		DER encoded value of the Subject Alternative Name extension field
 * @param	SubAltNameValLen	Length of the DER encoded value
 * @param	SubAltNameLen		Length of the Subject Alternative Name extension field
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Subject Alternative Name exetnsion field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	This function expects the user to provide the Subject Alternative Name extension
 *		field in DER encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenSubAltNameField(const u8 *SubAltName, const u32 SubAltNameValLen)
{
	return XCert_CreateRawDataFromByteArray(SubAltName, SubAltNameValLen);
}

/******************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 * 		TBS Certificate.
 *
 * @param	Cfg		structure which includes configuration for the TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated X.509 V3 Extensions field
 *		 - XCERT_ERR_X509_UPDATE_ENCODED_LEN  Failure in updating encoded length
 *		 - Error  In case of failure
 *
 * @note	Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
 *		Extension  ::=  SEQUENCE  {
 *		extnID      OBJECT IDENTIFIER,
 *		critical    BOOLEAN DEFAULT FALSE,
 *		extnValue   OCTET STRING
 *				-- contains the DER encoding of an ASN.1 value
 *				-- corresponding to the extension type identified
 *				-- by extnID
 *		}
 *
 *		This field is a SEQUENCE of the different extensions which should
 * 		be part of the Version 3 of the X.509 certificate.
 *
 ******************************************************************************/
static int XCert_GenX509v3ExtensionsField(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OptionalTagLenIdx;
	u8 *OptionalTagValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_3_CONSTRUCTED_TAG,
					&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenSubjectKeyIdentifierField(Cfg->AppCfg.SubjectPublicKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenAuthorityKeyIdentifierField(Cfg->AppCfg.IssuerPublicKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XCert_GenTcbInfoExtnField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * UEID extension (2.23.133.5.4.4) should be added for self-signed
	 * DevIK certificates only
	 */
	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		Status = XCert_GenUeidExnField();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif

	Status =  XCert_GenKeyUsageField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		Status = XCert_GenExtKeyUsageField(Cfg);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	if (Cfg->UserCfg->IsSubAltNameAvailable == TRUE) {
		Status = XCert_GenSubAltNameField(Cfg->UserCfg->SubAltName,
			Cfg->UserCfg->SubAltNameLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(OptionalTagLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, OptionalTagValIdx),
				OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*OptionalTagLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
					(u32)(*OptionalTagLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

END:
	return Status;
}


/******************************************************************************/
/**
 * @brief	This function creates the Basic Constraints extension field
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Basic Constraints Extension field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	This extension shall be part of the CSR only
 *
 ******************************************************************************/
static int XCert_GenBasicConstraintsExtnField(void)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u8 *BasicConstraintSequenceLenIdx;
	u8 *BasicConstraintSequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_BasicConstraintExtn,
							sizeof(Oid_BasicConstraintExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBoolean((u8)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&BasicConstraintSequenceLenIdx,
					&BasicConstraintSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBoolean((u8)TRUE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_ASN1_TAG_INTEGER);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_LEN_OF_VALUE_OF_PATH_LEN_CONSTRAINT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_PATH_LEN_CONSTRAINT_VALUE_0x0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*BasicConstraintSequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset,
								BasicConstraintSequenceValIdx);
	*OctetStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, OctetStrValIdx);
	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);

END:
	return Status;
}

#ifndef VERSAL_2VP
/******************************************************************************/
/**
 * @brief	This function creates the DME extension field
 *
 * @param	Len		Length of the DME Extension field.
 * @param	DmeResp		Pointer to structure which holds DME response
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated DME Extension field
 *		 - Error  In case of failure
 *
 * @note	DmeExtension ::= SEQUENCE {
 *		dmePublicKey 	 	SubjectPublicKeyInfo,
 *		dmeStructureFormat 	OBJECT IDENTIFIER,
 *		dmeStructure 	 	OCTET STRING,
 *		signatureAlgorithm 	AlgorithmIdentifier,
 *		signatureValue 	 	BIT STRING,
 *		}
 *		This extension shall be part of the CSR only and CSR will be generated
 *		only when DME response is generated.
 *
 ******************************************************************************/
static int XCert_GenDmeExtnField(u32 *Len, const XCert_DmeResponse *DmeResp)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *DmeSequenceLenIdx;
	u8 *DmeSequenceValIdx;
	u8 *OctetStrLenIdx;
	u8 *OctetStrValIdx;
	u32 DmeStructFieldLen;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();
	const u8 *CertReqInfoBuf = X509CertInfo->CurrOffset;

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_DmeExtn, sizeof(Oid_DmeExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_OCTETSTRING,
					&OctetStrLenIdx, &OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&DmeSequenceLenIdx, &DmeSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate DME structure extension field */
	Status = XCert_GenDmePublicKeyAndStructExtnField(&DmeStructFieldLen, &DmeResp->Dme);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Sign Algorithm field
	 */
	Status = XCert_GenSignAlgoField();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Signature field
	 */
	Status = XCert_GenSignField((const u8 *)DmeResp->DmeSignatureR);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateEncodedLength(DmeSequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, DmeSequenceValIdx),
				DmeSequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((*DmeSequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
					(u32)(*DmeSequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(OctetStrLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, OctetStrValIdx),
				OctetStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((*OctetStrLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*OctetStrLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	*Len = XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, CertReqInfoBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the DME public key and structure extension sub-field
 *
 * @param	Len		Length of the DME structure extension field.
 * @param	Dme		Pointer to XCert_DmeChallenge
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated DME public key and structure Extension sub-field
 *		 - Error  In case of failure
 *
 * @note	This extension shall be part of the CSR only
 *
 ******************************************************************************/
static int XCert_GenDmePublicKeyAndStructExtnField(u32 *Len, const XCert_DmeChallenge *Dme)
{
	int Status = XST_FAILURE;
	u8 DmePublicKey[XCERT_ECC_P384_PUBLIC_KEY_LEN] = {0U};
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();
	const u8 *CertReqInfoBuf = X509CertInfo->CurrOffset;

	/* Reverse endianness of DME public key */
	Status = Xil_SChangeEndiannessAndCpy(DmePublicKey, XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES,
				(const u8*)(UINTPTR)XCERT_DME_PUB_KEY_X_0, XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES,
				XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SChangeEndiannessAndCpy(&DmePublicKey[XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES],
				XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES, (const u8*)(UINTPTR)XCERT_DME_PUB_KEY_Y_0,
				XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES, XCERT_ECC_P384_PUBLIC_KEY_LEN_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate DME Public Key Info field
	 */
	Status = XCert_GenPublicKeyInfoField(DmePublicKey);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Generate DME struct extension */
	Status = XCert_CreateRawDataFromByteArray(Oid_DmeStructExtn, sizeof(Oid_DmeStructExtn));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateOctetString((const u8 *)(UINTPTR)Dme, sizeof(XCert_DmeChallenge));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*Len = XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, CertReqInfoBuf);

END:
	return Status;
}
#endif

/******************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 * 		Certificate request Info.
 *
 * @param	Cfg		Structure which includes configuration for the Certificate Request Info
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Extensions field for Certification Request Info
 *		 - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
static int XCert_GenCsrExtensions(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *OptionalTagLenIdx;
	u8 *OptionalTagValIdx;
	u8 *ExtnReqSeqLenIdx;
	u8 *ExtnReqSeqValIdx;
	u8 *SetLenIdx;
	u8 *SetValIdx;
	u32 Len;

	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_OPTIONAL_PARAM_0_CONSTRUCTED_TAG,
					&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&ExtnReqSeqLenIdx, &ExtnReqSeqValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateRawDataFromByteArray(Oid_ExtnRequest, sizeof(Oid_ExtnRequest));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SET,
					&SetLenIdx, &SetValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status =  XCert_GenKeyUsageField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenExtKeyUsageField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XCert_GenTcbInfoExtnField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_GenUeidExnField();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	Status = XCert_GenBasicConstraintsExtnField();
	if (Status != XST_SUCCESS) {
		goto END;
	}

#ifndef VERSAL_2VP
	Status = XCert_GenDmeExtnField(&Len, Cfg->AppCfg.DmeResp);
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(SetLenIdx,
					XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SetValIdx),
					SetValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((*SetLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)((*SetLenIdx) & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(ExtnReqSeqLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, ExtnReqSeqValIdx),
				ExtnReqSeqValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((*ExtnReqSeqLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
					(u32)(*ExtnReqSeqLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

	Status = XCert_UpdateEncodedLength(OptionalTagLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, OptionalTagValIdx),
				OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((*OptionalTagLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo,
					(u32)(*OptionalTagLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the TBS(To Be Signed) Certificate.
 *
 * @param	Cfg		Structure which includes configuration for the TBS Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated TBS Certificate
 *		 - XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD  Error in generating Public Key Info field
 *		 - XCERT_ERR_X509_UPDATE_ENCODED_LEN  Error in updating encoded length
 *		 - Error  In case of failure
 *
 * @note	TBSCertificate  ::=  SEQUENCE  {
 *			version         [0]  EXPLICIT Version DEFAULT v1,
 *			serialNumber         CertificateSerialNumber,
 *			signature            AlgorithmIdentifier,
 *			issuer               Name,
 *			validity             Validity,
 *			subject              Name,
 *			subjectPublicKeyInfo SubjectPublicKeyInfo,
 *		}
 *
 ******************************************************************************/
static int XCert_GenTBSCertificate(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *SerialStartIdx;
	u8 *SerialHashStartIdx;
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
			&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_OPTIONAL_PARAM_0_CONSTRUCTED_TAG);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(XCERT_LEN_OF_VERSION_FIELD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Version field
	 */
	Status = XCert_GenVersionField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Store the start index for the Serial field. Once all the remaining
	 * fields are populated then the SHA2 hash is calculated for the
	 * remaining fields in the TBS certificate and Serial is 20 bytes from
	 * LSB of the calculated hash. Hence we need to store the start index of Serial
	 * so that it can be updated later. The total length of the Serial field is 22 bytes.
	 */
	SerialStartIdx = X509CertInfo->CurrOffset;
	XCert_AdvanceOffset(X509CertInfo, (u32)XCERT_SERIAL_FIELD_LEN);
	SerialHashStartIdx = X509CertInfo->CurrOffset;
	/**
	 * Generate Signature Algorithm field
	 */
	Status = XCert_GenSignAlgoField();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Generate Issuer field
	 */
	Status = XCert_GenIssuerField(Cfg->UserCfg->Issuer, Cfg->UserCfg->IssuerLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Validity field
	 */
	Status = XCert_GenValidityField(Cfg->UserCfg->Validity, Cfg->UserCfg->ValidityLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Subject field
	 */
	Status = XCert_GenSubjectField(Cfg->UserCfg->Subject, Cfg->UserCfg->SubjectLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Public Key Info field
	 */

	Status = XCert_GenPublicKeyInfoField(Cfg->AppCfg.SubjectPublicKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD;
		goto END;
	}
	/**
	 * Generate X.509 V3 extensions field
	 *
	 */
	Status = XCert_GenX509v3ExtensionsField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Calculate Hash for all fields in the TBS certificate except Version and Serial
	 * Please note that currently SerialStartIdx points to the field after Serial.
	 * Hence this is the start pointer for calculating the hash.
	 */
	Status = XCert_ShaDigest(SerialHashStartIdx, XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset,
									SerialHashStartIdx), Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Serial field
	 */
	Status = XCert_GenSerialField(SerialStartIdx, Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Update the encoded length in the TBS certificate SEQUENCE
	 */

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Certification Request Info.
 *
 * @param	Cfg		Structure which includes configuration for the Certification Request Info
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Certification Request Info
 *		 - XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD  Error in generating Public Key Info field
 *		 - XCERT_ERR_X509_UPDATE_ENCODED_LEN  Error in updating encoded length
 *		 - Error  In case of failure
 *
 ******************************************************************************/
static int XCert_GenCertReqInfo(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE, &SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Generate Version field
	 */
	Status = XCert_GenVersionField(Cfg);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/**
	 * Generate Subject field
	 */
	Status = XCert_GenSubjectField(Cfg->UserCfg->Subject,
				Cfg->UserCfg->SubjectLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Public Key Info field
	 */
	Status = XCert_GenPublicKeyInfoField(Cfg->AppCfg.SubjectPublicKey);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD;
		goto END;
	}

	Status = XCert_GenCsrExtensions(Cfg);

	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateEncodedLength(SequenceLenIdx,
				XCERT_PTR_DIFF_U32(X509CertInfo->CurrOffset, SequenceValIdx),
				SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~XCERT_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		XCert_AdvanceOffset(X509CertInfo, (u32)(*SequenceLenIdx & XCERT_LOWER_NIBBLE_MASK));
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Signature field in the X.509 certificate
 *
 * @param	Signature	Value of the Signature field in X.509 certificate
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Sign field
 *		 - XST_FAILURE  In case of failure
 *
 * @note	Th signature value is encoded as a BIT STRING and included in the
 *		signature field. When signing, the ECDSA algorithm generates
 *		two values - r and s.
 *		To easily transfer these two values as one signature,
 *		they MUST be DER encoded using the following ASN.1 structure:
 *		Ecdsa-Sig-Value  ::=  SEQUENCE  {
 *			r     INTEGER,
 *			s     INTEGER  }
 *
 ******************************************************************************/
static int XCert_GenSignField(const u8 *Signature)
{
	int Status = XST_FAILURE;
	u8 *BitStrLenIdx;
	u8 *BitStrValIdx;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	XCert_X509CertInfo *X509CertInfo = XCert_GetX509CertInstance();

	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_BITSTRING,
					&BitStrLenIdx, &BitStrValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_UpdateByteField(0x00);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	Status = XCert_UpdateTLVField(XCERT_ASN1_TAG_SEQUENCE,
					&SequenceLenIdx, &SequenceValIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateInteger(Signature, XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}


	Status = XCert_CreateInteger((u8 *)(Signature + XSECURE_ECC_P384_SIZE_IN_BYTES),
		XSECURE_ECC_P384_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*SequenceLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, SequenceValIdx);
	*BitStrLenIdx = XCERT_PTR_DIFF_U8(X509CertInfo->CurrOffset, BitStrValIdx);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies data to 32/64 bit address from
 *		local buffer.
 *
 * @param	Size	Length of data in bytes
 * @param	Src	Pointer to the source buffer
 * @param	DstAddr Destination address
 *
 *****************************************************************************/
static void XCert_CopyCertificate(const u32 Size, const u8 *Src, const u64 DstAddr)
{
	u32 Index = 0U;

	for (Index = 0U; Index < Size; Index++) {
		XSecure_OutByte64((DstAddr + Index), Src[Index]);
	}
}

#if (!defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP))
/*****************************************************************************/
/**
 * @brief	This function creates the Version sub-field present in the TCB Info extension
 * 		of the TBS Certificate.
 *
 * @param	TBSCertBuf	Pointer in the TBS Certificate buffer where the FwVersion field shall be added.
 * @param	Cfg		Pointer to structure which includes configuration for the TBS Certificate.
 * @param	FwVersionLen	Length of the Version sub-field
 *
 * @return
 *              - XST_SUCCESS  Successfully generated Version sub-field
 *              - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
static int XCert_GenFwVersionField(XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u32 FwVersion;

	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		FwVersion = XCert_In32(XCERT_RTCA_PLM_VERSION_ADDR);
		Status = XCert_BuildPlmVersionAndCreateRawField(FwVersion);
	}
	else {
		/**
		 * In case app version is not provided, create Fw Version field with zero as a value
		*/
		if (Cfg->AppCfg.FwVersionLen == 0U) {
			Status = XCert_UpdateByteField(XCERT_NULL_VALUE);
		}
		else {
			Cfg->AppCfg.FwVersionLen = Xil_Strnlen((char*)Cfg->AppCfg.FwVersion, Cfg->AppCfg.FwVersionLen);
			Status = XCert_CreateRawDataFromByteArray(Cfg->AppCfg.FwVersion,
									Cfg->AppCfg.FwVersionLen);
			if(Status != XST_SUCCESS) {
				goto END;
			}
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Security Version sub-field present in the TCB Info extension
 * 		of the TBS Certificate.
 *
 * @param	SvnLen		Length of the Security Version sub-field
 *
 * @return
 *              - XST_SUCCESS  Successfully generated security version field
 *              - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
static int XCert_GenSecurityVersionField(void)
{
	u32 *Svn = XCert_GetSpkId();
	u32 IntegerVal;

	IntegerVal = Xil_EndianSwap32(*Svn);
	return XCert_CreateIntegerFieldFromByteArray((u8 *)&IntegerVal, XCERT_WORD_LEN);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Layer sub-field present in the TCB Info extension
 *		of the TBS Certificate.
 *
 * @param	Cfg		Pointer to structure which includes configuration for the TBS Certificate.
 *
 * @return
 *              - XST_SUCCESS  Successfully generated layer field
 *              - XST_FAILURE  In case of failure
 *
 ******************************************************************************/
static int XCert_GenLayerField(XCert_Config *Cfg)
{
	u32 Layer;
	u32 IntegerVal;

	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		Layer = XCERT_LAYER_0;
	}
	else {
		Layer = XCERT_LAYER_1;
	}

	IntegerVal = Xil_EndianSwap32(Layer);
	return XCert_CreateIntegerFieldFromByteArray((u8 *)&IntegerVal, XCERT_WORD_LEN);
}
#endif /* VERSAL_2VE_2VM */

#endif  /* PLM_OCP_NATIVE_KEY_MGMT */


/**
 * @}
 * @endcond
 */
