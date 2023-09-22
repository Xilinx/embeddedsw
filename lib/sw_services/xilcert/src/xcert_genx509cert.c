/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"

#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_ellipticplat.h"
#endif
#include "xsecure_sha384.h"
#include "xcert_genx509cert.h"
#include "xcert_createfield.h"
#include "xplmi.h"
#include "xplmi_status.h"
#include "xplmi_tamper.h"
#include "xsecure_plat_kat.h"
#include "xsecure_init.h"
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/
/** @name Object IDs
 * @{
 */
 /**< Object IDs used in X.509 Certificate and Certificate Signing Request */
static const u8 Oid_SignAlgo[]		= {0x06U, 0x08U, 0x2AU, 0x86U, 0x48U, 0xCEU, 0x3DU, 0x04U, 0x03U, 0x03U};
#ifndef PLM_ECDSA_EXCLUDE
static const u8 Oid_EcPublicKey[]	= {0x06U, 0x07U, 0x2AU, 0x86U, 0x48U, 0xCEU, 0x3DU, 0x02U, 0x01U};
static const u8 Oid_P384[]		= {0x06U, 0x05U, 0x2BU, 0x81U, 0x04U,0x00U, 0x22U};
#endif
static const u8 Oid_SubKeyIdentifier[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x0EU};
static const u8 Oid_AuthKeyIdentifier[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x23U};
static const u8 Oid_TcbInfoExtn[]	= {0x06U, 0x06U, 0x67U, 0x81U, 0x05U, 0x05U, 0x04U, 0x01U};
static const u8 Oid_UeidExtn[]		= {0x06U, 0x06U, 0x67U, 0x81U, 0x05U, 0x05U, 0x04U, 0x04U};
static const u8 Oid_KeyUsageExtn[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x0FU};
static const u8 Oid_EkuExtn[]		= {0x06U, 0x03U, 0x55U, 0x1DU, 0x25U};
static const u8 Oid_EkuClientAuth[]	= {0x06U, 0x08U, 0x2BU, 0x06U, 0x01U, 0x05U, 0x05U, 0x07U, 0x03U, 0x02U};
static const u8 Oid_EkuHwType[]		= {0x06U, 0x0BU, 0x2BU, 0x06U, 0x01U, 0x04U, 0x01U, 0x82U, 0x37U, 0x66U, 0x01U, 0x0CU, 0x01U};
static const u8 Oid_BasicConstraintExtn[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x13U};
static const u8 Oid_ExtnRequest[]	= {0x06U, 0x09U, 0x2AU, 0x86U, 0x48U, 0x86U, 0xF7U, 0x0DU, 0x01U, 0x09U, 0x0EU};
static const u8 Oid_Sha3_384[]		= {0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U, 0x04U, 0x02U, 0x09U};
/** @} */

#define XCERT_SERIAL_FIELD_LEN				(22U)
			/**< Length of Serial Field */
#define XCERT_BIT7_MASK 				(0x80U)
			/**< Mask to get bit 7*/
#define XCERT_LOWER_NIBBLE_MASK				(0xFU)
			/**< Mask to get lower nibble */
#define XCERT_SIGN_AVAILABLE				(0x3U)
			/**< Signature available in SignStore */
#define XCERT_BYTE_MASK					(0xFFU)
			/**< Mask to get byte */
#define XCERT_MAX_CERT_SUPPORT				(4U)
	/**< Number of supported certificates is 4 -> 1 DevIK certificate and 3 DevAK certificates */
#define XCERT_SUB_KEY_ID_VAL_LEN			(20U)
			/**< Length of value of Subject Key ID */
#define XCERT_AUTH_KEY_ID_VAL_LEN			(20U)
			/**< Length of value of Authority Key ID */
#define XCERT_UNCOMPRESSED_PUB_KEY			(0x04U)
			/**< To indicate uncompressed public key */
#define XCERT_MAX_LEN_OF_KEYUSAGE_VAL			(2U)
			/**< Maximum length of value of key usage */

#define XCERT_WORD_LEN					(0x04U)
			/**< Length of word in bytes */
#define XCERT_LEN_OF_BYTE_IN_BITS			(8U)
			/**< Length of byte in bits */
#define XCERT_AUTH_KEY_ID_OPTIONAL_PARAM		(0x80U)
		/**< Optional parameter in Authority Key Identifier field*/

/** @name Optional parameter tags
 * @{
 */
 /**< Tags for optional parameters */
#define XCERT_OPTIONAL_PARAM_0_TAG			(0xA0U)
#define XCERT_OPTIONAL_PARAM_3_TAG			(0xA3U)
#define XCERT_OPTIONAL_PARAM_6_TAG			(0xA6U)
/** @} */

/** @name DNA
 * @{
 */
 /**< Macros related to Address of DNA and length of DNA in words and bytes */
#define XCERT_DNA_0_ADDRESS				(0xF1250020U)
#define XCERT_DNA_LEN_IN_WORDS				(4U)
#define XCERT_DNA_LEN_IN_BYTES				(XCERT_DNA_LEN_IN_WORDS * XCERT_WORD_LEN)
/** @} */

#define XCert_In32					(XPlmi_In32)
			/**< Alias of XPlmi_In32 to be used in XilCert*/

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
static u32* XCert_GetNumOfEntriesInUserCfgDB(void);
static int XCert_IsBufferNonZero(u8* Buffer, u32 BufferLen);
static int XCert_GetUserCfg(u32 SubsystemId, XCert_UserCfg **UserCfg);
static void XCert_GenVersionField(u8* TBSCertBuf, XCert_Config *Cfg, u32 *VersionLen);
static int XCert_GenSerialField(u8* TBSCertBuf, u8* DataHash, u32 *SerialLen);
static int XCert_GenSignAlgoField(u8* CertBuf, u32 *SignAlgoLen);
static inline int XCert_GenIssuerField(u8* TBSCertBuf, u8* Issuer, const u32 IssuerValLen, u32 *IssuerLen);
static inline int XCert_GenValidityField(u8* TBSCertBuf, u8* Validity, const u32 ValidityValLen, u32 *ValidityLen);
static inline int XCert_GenSubjectField(u8* TBSCertBuf, u8* Subject, const u32 SubjectValLen, u32 *SubjectLen);
#ifndef PLM_ECDSA_EXCLUDE
static int XCert_GenPubKeyAlgIdentifierField(u8* TBSCertBuf, u32 *Len);
static int XCert_GenPublicKeyInfoField(u8* TBSCertBuf, u8* SubjectPublicKey,u32 *PubKeyInfoLen);
static int XCert_GenSignField(u8* X509CertBuf, u8* Signature, u32 *SignLen);
static int XCert_GetSignStored(u32 SubsystemId, XCert_SignStore **SignStore);
#endif
static int XCert_GenTBSCertificate(u8* TBSCertBuf, XCert_Config* Cfg, u32 *TBSCertLen);
static void XCert_CopyCertificate(const u32 Size, const u8 *Src, const u64 DstAddr);
static int XCert_GenSubjectKeyIdentifierField(u8* TBSCertBuf, u8* SubjectPublicKey, u32 *SubjectKeyIdentifierLen);
static int XCert_GenAuthorityKeyIdentifierField(u8* TBSCertBuf, u8* IssuerPublicKey, u32 *AuthorityKeyIdentifierLen);
static int XCert_GenTcbInfoExtnField(u8* TBSCertBuf, XCert_Config* Cfg, u32 *TcbInfoExtnLen);
static int XCert_GenUeidExnField(u8* TBSCertBuf, u32 *UeidExnLen);
static void XCert_UpdateKeyUsageVal(u8* KeyUsageVal, XCert_KeyUsageOption KeyUsageOption);
static int XCert_GenKeyUsageField(u8* TBSCertBuf, XCert_Config* Cfg, u32 *KeyUsageExtnLen);
static int XCert_GenExtKeyUsageField(u8* TBSCertBuf,  XCert_Config* Cfg, u32 *EkuLen);
static int XCert_GenX509v3ExtensionsField(u8* TBSCertBuf,  XCert_Config* Cfg, u32 *ExtensionsLen);
static int XCert_GenBasicConstraintsExtnField(u8* CertReqInfoBuf, u32 *Len);
static int XCert_GenCsrExtensions(u8* CertReqInfoBuf, XCert_Config* Cfg, u32 *ExtensionsLen);
static int XCert_GenCertReqInfo(u8* CertReqInfoBuf, XCert_Config* Cfg, u32 *CertReqInfoLen);

/************************** Function Definitions *****************************/
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
static u32* XCert_GetNumOfEntriesInUserCfgDB(void)
{
	static u32 NumOfEntriesInUserCfgDB = 0U;

	return &NumOfEntriesInUserCfgDB;
}

/*****************************************************************************/
/**
 * @brief	This function checks if all the bytes in the provided buffer are
 *		zero.
 *
 * @param	Buffer - Pointer to the buffer
 * @param	BufferLen - Length of the buffer
 *
 * @return
 *		 - XST_SUCCESS  If buffer is non-empty
 *		 - XST_FAILURE  If buffer is empty
 *
 ******************************************************************************/
static int XCert_IsBufferNonZero(u8* Buffer, u32 BufferLen)
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
 * @brief	This function finds the provided Subsystem ID in UserCfg DB and
 *		returns the pointer to the corresponding entry in DB
 *		if all the other fields are valid.
 *
 * @param	SubsystemId - Subsystem ID for which user configuration is requested
 * @param	UserCfg - Pointer to the entry in DB for the provided Subsystem ID
 *
 * @return
 *		 - XST_SUCCESS  If subsystem ID is found and other fields are valid
 *		 - Error Code  Upon any failure
 *
 ******************************************************************************/
static int XCert_GetUserCfg(u32 SubsystemId, XCert_UserCfg **UserCfg)
{
	int Status = XST_FAILURE;
	XCert_InfoStore *CertDB = XCert_GetCertDB();
	u32 *NumOfEntriesInUserCfgDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	/**
	 * Search for given Subsystem ID in the UserCfg DB
	 */
	for (Idx = 0; Idx < *NumOfEntriesInUserCfgDB; Idx++) {
		if (CertDB[Idx].SubsystemId == SubsystemId) {
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

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function finds the provided Subsystem ID in InfoStore DB and
 *		returns the pointer to the corresponding sign entry in DB.
 *
 * @param	SubsystemId - SubsystemId for which stored signature is requested
 * @param	SignStore - Pointer to the entry in DB for the provided Subsystem ID
 *
 * @return
 *		 - XST_SUCCESS  If subsystem ID is found
 *		 - Error Code  Upon any failure
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
#endif

/*****************************************************************************/
/**
 * @brief	This function stores the user provided value for the user configurable
 *		fields in the certificate as per the provided FieldType.
 *
 * @param	SubSystemId is the id of subsystem for which field data is provided
 * @param	FieldType is to identify the field for which input is provided
 * @param	Val is the value of the field provided by the user
 * @param	Len is the length of the value in bytes
 *
 * @return
 *		 - XST_SUCCESS  If whole operation is success
 *		 - XST_FAILURE  Upon any failure
 *
 ******************************************************************************/
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType, u8* Val, u32 Len)
{
	int Status = XST_FAILURE;
	u32 IdxToBeUpdated;
	u32 IsSubsystemIdPresent = FALSE;
	XCert_InfoStore *CertDB = XCert_GetCertDB();
	u32 *NumOfEntriesInUserCfgDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	if (FieldType > XCERT_VALIDITY) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (((FieldType == XCERT_VALIDITY) && (Len > XCERT_VALIDITY_MAX_SIZE)) ||
		((FieldType == XCERT_ISSUER) && (Len > XCERT_ISSUER_MAX_SIZE)) ||
		((FieldType == XCERT_SUBJECT) && (Len > XCERT_SUBJECT_MAX_SIZE))) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 * Look for the Subsystem Id. If it is there get the index and update the
	 * field of existing subsystem else increment index and add entry for
	 * new subsystem.
	*/
	for (Idx = 0; Idx < *NumOfEntriesInUserCfgDB; Idx++) {
		if (CertDB[Idx].SubsystemId == SubSystemId) {
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
	else {
		XSecure_MemCpy(CertDB[IdxToBeUpdated].UserCfg.Validity, Val, Len);
		CertDB[IdxToBeUpdated].UserCfg.ValidityLen = Len;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the X.509 Certificate/Certificate Signing Request(CSR)
 *
 * @param	X509CertAddr is the address of the X.509 Certificate buffer
 * @param	MaxCertSize is the maximum size of the X.509 Certificate buffer
 * @param	X509CertSize is the size of X.509 Certificate in bytes
 * @param	Cfg is structure which includes configuration for the X.509 Certificate.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated X.509 certificate/CSR
 *		 - Error code  In case of failure
 *
 * @note	Certificate  ::=  SEQUENCE  {
 *			tbsCertificate       TBSCertificate,
 *			signatureAlgorithm   AlgorithmIdentifier,
 *			signatureValue       BIT STRING  }
 *
 ******************************************************************************/
int XCert_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32* X509CertSize, XCert_Config *Cfg)
{
	int Status = XST_FAILURE;
	u8 X509CertBuf[1024];
	u8* Start = X509CertBuf;
	u8* Curr = Start;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 DataLen = 0U;
	u32 SignAlgoLen;
#ifndef PLM_ECDSA_EXCLUDE
	int HashCmpStatus = XST_FAILURE;
	u32 SignLen = 0U;
	u8 Sign[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 SignTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	XCert_SignStore *SignStore;
#endif
	u8 HashTmp[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	u8 *TbsCertStart;
	(void)MaxCertSize;

	if (Cfg == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_GetUserCfg(Cfg->SubSystemId, &(Cfg->UserCfg));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	TbsCertStart = Curr;
	if (Cfg->AppCfg.IsCsr == TRUE) {
		Status = XCert_GenCertReqInfo(Curr, Cfg, &DataLen);
	}
	else {
		Status = XCert_GenTBSCertificate(Curr, Cfg, &DataLen);
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + DataLen;
	}

	/**
	 * Generate Sign Algorithm field
	 */
	Status = XCert_GenSignAlgoField(Curr, &SignAlgoLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + SignAlgoLen;

	/**
	 * Calculate SHA2 Digest of the TBS certificate
	 */
	Status = XSecure_Sha384Digest(TbsCertStart, DataLen, HashTmp);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_DIGEST;
		goto END;
	}
#ifndef PLM_ECDSA_EXCLUDE

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
	XCert_GenSignField(Curr, Sign, &SignLen);
	Curr = Curr + SignLen;
#else
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
	goto END;
#endif

	/**
	 * Update the encoded length in the X.509 certificate SEQUENCE
	 */
	Status = XCert_UpdateEncodedLength(SequenceLenIdx, (u32)(Curr - SequenceValIdx), SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);
	*X509CertSize = Curr - Start;

	XCert_CopyCertificate(*X509CertSize, (u8 *)X509CertBuf, X509CertAddr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Version field of the TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Version field shall be added.
 * @param	VersionLen is the length of the Version field
 *
 * @note	Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
 *		This field describes the version of the encoded certificate.
 *		XilCert library supports X.509 V3 certificates. For Certificate
 *		Signing Request, the supported version is V1
 *
 ******************************************************************************/
static void XCert_GenVersionField(u8* TBSCertBuf, XCert_Config *Cfg, u32 *VersionLen)
{
	u8* Curr = TBSCertBuf;

	*(Curr++) = XCERT_ASN1_TAG_INTEGER;
	*(Curr++) = XCERT_LEN_OF_VALUE_OF_VERSION;

	if (Cfg->AppCfg.IsCsr != TRUE) {
		*(Curr++) = XCERT_VERSION_VALUE_V3;
	}
	else {
		*(Curr++) = XCERT_VERSION_VALUE_V1;
	}

	*VersionLen = (u32)(Curr - TBSCertBuf);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Serial field of the TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Serial field shall be added.
 * @param	SerialLen is the length of the Serial field
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Serial field
 *		 - Error code  In case of failure
 *
 * @note	CertificateSerialNumber  ::=  INTEGER
 *		The length of the serial must not be more than 20 bytes.
 *		The value of the serial is determined by calculating the
 *		SHA2 hash of the fields in the TBS Certificate except the Version
 *		and Serial Number fields. 20 bytes from LSB of the calculated
 *		hash is updated as the Serial Number
 *
 ******************************************************************************/
static int XCert_GenSerialField(u8* TBSCertBuf, u8* DataHash, u32 *SerialLen)
{
	int Status = XST_FAILURE;
	u8 Serial[XCERT_LEN_OF_VALUE_OF_SERIAL] = {0U};
	u32 LenToBeCopied;

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

	Status = XCert_CreateInteger(TBSCertBuf, Serial, LenToBeCopied, SerialLen);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Signature Algorithm field.
 *		This field in present in TBS Certificate as well as the
 *		X509 certificate.
 *
 * @param	CertBuf is the pointer in the Certificate buffer where
 *		the Signature Algorithm field shall be added.
 * @param	SignAlgoLen is the length of the SignAlgo field
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
static int XCert_GenSignAlgoField(u8* CertBuf, u32 *SignAlgoLen)
{
	int Status = XST_FAILURE;
	u8* Curr = CertBuf;
	u32 OidLen;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_SignAlgo, sizeof(Oid_SignAlgo), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	*(Curr++) = XCERT_ASN1_TAG_NULL;
	*(Curr++) = XCERT_NULL_VALUE;

	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*SignAlgoLen = (u32)(Curr - CertBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Issuer field in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Issuer field shall be added.
 * @param	Issuer is the DER encoded value of the Issuer field
 * @param	IssuerValLen is the length of the DER encoded value
 * @param	IssuerLen is the length of the Issuer field
 *
 * @note	This function expects the user to provide the Issuer field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenIssuerField(u8* TBSCertBuf, u8* Issuer, const u32 IssuerValLen, u32 *IssuerLen)
{
	int Status = XST_FAILURE;

	Status = XCert_CreateRawDataFromByteArray(TBSCertBuf, Issuer, IssuerValLen, IssuerLen);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Validity field in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Validity field shall be added.
 * @param	Validity is the DER encoded value of the Validity field
 * @param	ValidityValLen is the length of the DER encoded value
 * @param	ValidityLen is the length of the Validity field
 *
 * @note	This function expects the user to provide the Validity field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenValidityField(u8* TBSCertBuf, u8* Validity, const u32 ValidityValLen, u32 *ValidityLen)
{
	int Status = XST_FAILURE;

	Status = XCert_CreateRawDataFromByteArray(TBSCertBuf, Validity, ValidityValLen, ValidityLen);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Subject field in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Subject field shall be added.
 * @param	Subject is the DER encoded value of the Subject field
 * @param	SubjectValLen is the length of the DER encoded value
 * @param	SubjectLen is the length of the Subject field
 *
 * @note	This function expects the user to provide the Subject field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static inline int XCert_GenSubjectField(u8* TBSCertBuf, u8* Subject, const u32 SubjectValLen, u32 *SubjectLen)
{
	int Status = XST_FAILURE;

	Status = XCert_CreateRawDataFromByteArray(TBSCertBuf, Subject, SubjectValLen, SubjectLen);

	return Status;
}

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function creates the Public Key Algorithm Identifier sub-field.
 *		It is a part of Subject Public Key Info field present in
 *		TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Public Key Algorithm Identifier sub-field shall be added.
 * @param	Len is the length of the Public Key Algorithm Identifier sub-field.
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
static int XCert_GenPubKeyAlgIdentifierField(u8* TBSCertBuf, u32 *Len)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 OidLen;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_EcPublicKey, sizeof(Oid_EcPublicKey), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_P384, sizeof(Oid_P384), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*Len = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Public Key Info field present in
 *		TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Public Key Info field shall be added.
 * @param	SubjectPublicKey is the public key of the Subject for which the
  *		certificate is being created.
 * @param	PubKeyInfoLen is the length of the Public Key Info field.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Public Key Info field
 *		 - Error code  In case of failure
 *
 * @note	SubjectPublicKeyInfo  ::=  SEQUENCE  {
			algorithm            AlgorithmIdentifier,
			subjectPublicKey     BIT STRING  }
 *
 ******************************************************************************/
static int XCert_GenPublicKeyInfoField(u8* TBSCertBuf, u8* SubjectPublicKey, u32 *PubKeyInfoLen)
{
	int Status = XST_FAILURE;
	u32 KeyLen = XCERT_ECC_P384_PUBLIC_KEY_LEN;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 Len;
	u8 UncompressedPublicKey[XCERT_ECC_P384_PUBLIC_KEY_LEN + 1U] = {0U};

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_GenPubKeyAlgIdentifierField(Curr, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */
	UncompressedPublicKey[0U] = XCERT_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(UncompressedPublicKey + 1U, KeyLen + 1U, SubjectPublicKey, KeyLen, KeyLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XCert_CreateBitString(Curr, UncompressedPublicKey, KeyLen + 1U, TRUE, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*PubKeyInfoLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}
#endif

/******************************************************************************/
/**
 * @brief	This function creates the Subject Key Identifier field present in
 * 		TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Subject Key Identifier field shall be added.
 * @param	SubjectPublicKey is the public key whose hash will be used as
 * 		Subject Key Identifier
 * @param	SubjectKeyIdentifierLen is the length of the Subject Key Identifier field.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Subject Key Identifier field
 *		 - Error code  In case of failure
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
static int XCert_GenSubjectKeyIdentifierField(u8* TBSCertBuf, u8* SubjectPublicKey, u32 *SubjectKeyIdentifierLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u32 OidLen;
	u32 FieldLen;
	XSecure_Sha3 *ShaInstancePtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	XSecure_Sha3Hash Sha3Hash;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_SubKeyIdentifier, sizeof(Oid_SubKeyIdentifier),
		&OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	Status = XSecure_Sha3Initialize(ShaInstancePtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Digest(ShaInstancePtr, (UINTPTR)SubjectPublicKey, XCERT_ECC_P384_PUBLIC_KEY_LEN, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;

	Status = XCert_CreateOctetString(Curr, Sha3Hash.Hash, XCERT_SUB_KEY_ID_VAL_LEN, &FieldLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + FieldLen;

	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*SubjectKeyIdentifierLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the Authority Key Identifier field present in
 * 		TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Authority Key Identifier field shall be added.
 * @param	IssuerPublicKey is the public key whose hash will be used as
 * 		Authority Key Identifier
 * @param	AuthorityKeyIdentifierLen is the length of the Authority Key Identifier field.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Authority Key Identifier field
 *		 - Error code  In case of failure
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
static int XCert_GenAuthorityKeyIdentifierField(u8* TBSCertBuf, u8* IssuerPublicKey, u32 *AuthorityKeyIdentifierLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u8* KeyIdSequenceLenIdx;
	u8* KeyIdSequenceValIdx;
	u32 OidLen;
	u32 FieldLen;
	XSecure_Sha3 *ShaInstancePtr = XSecure_GetSha3Instance();
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);
	XSecure_Sha3Hash Sha3Hash;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_AuthKeyIdentifier, sizeof(Oid_AuthKeyIdentifier),
		&OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	Status = XSecure_Sha3Initialize(ShaInstancePtr, PmcDmaInstPtr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_Sha3Digest(ShaInstancePtr, (UINTPTR)IssuerPublicKey, XCERT_ECC_P384_PUBLIC_KEY_LEN, &Sha3Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;


	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	KeyIdSequenceLenIdx = Curr++;
	KeyIdSequenceValIdx = Curr;

	Status = XCert_CreateOctetString(Curr, Sha3Hash.Hash, XCERT_AUTH_KEY_ID_VAL_LEN, &FieldLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + FieldLen;

	/**
	 * 0x80 indicates that the SEQUENCE contains the optional parameter tagged
	 * as [0] in the AuthorityKeyIdentifier sequence
	 */
	*KeyIdSequenceValIdx = XCERT_AUTH_KEY_ID_OPTIONAL_PARAM;

	*KeyIdSequenceLenIdx = (u8)(Curr - KeyIdSequenceValIdx);
	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*AuthorityKeyIdentifierLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the TCB Info Extension(2.23.133.5.4.1)
 * 		field present in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the TCB Info Extension field shall be added.
 * @param	TcbInfoExtnLen is the length of the Authority Key Identifier field.
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
 * 		As per requirement, only fwids needs to be included in the extension.
 *		For DevIk certificates, the value of fwid shall be SHA3-384 hash of
 *		PLM and PMC CDO.
 *		For DevAk certificates, the value of fwid shall be SHA3-384 hash of
 *		the application.
 *
 ******************************************************************************/
static int XCert_GenTcbInfoExtnField(u8* TBSCertBuf, XCert_Config* Cfg, u32 *TcbInfoExtnLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u8* TcbInfoSequenceLenIdx;
	u8* TcbInfoSequenceValIdx;
	u8* OptionalTagLenIdx;
	u8* OptionalTagValIdx;
	u8* FwIdSequenceLenIdx;
	u8* FwIdSequenceValIdx;
	u32 OidLen;
	u32 FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_TcbInfoExtn, sizeof(Oid_TcbInfoExtn), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	TcbInfoSequenceLenIdx = Curr++;
	TcbInfoSequenceValIdx = Curr;

	*(Curr++) = XCERT_OPTIONAL_PARAM_6_TAG;
	OptionalTagLenIdx = Curr++;
	OptionalTagValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	FwIdSequenceLenIdx = Curr++;
	FwIdSequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_Sha3_384, sizeof(Oid_Sha3_384), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	Status = XCert_CreateOctetString(Curr, Cfg->AppCfg.FwHash, XCERT_HASH_SIZE_IN_BYTES, &FieldLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + FieldLen;

	*FwIdSequenceLenIdx = (u8)(Curr - FwIdSequenceValIdx);
	*OptionalTagLenIdx = (u8)(Curr - OptionalTagValIdx);
	*TcbInfoSequenceLenIdx = (u8)(Curr - TcbInfoSequenceValIdx);
	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*TcbInfoExtnLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the UEID extension(2.23.133.5.4.4) field
 * 		present in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the UEID extension field shall be added.
 * @param	UeidExnLen is the length of the UEID Extension field.
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
static int XCert_GenUeidExnField(u8* TBSCertBuf, u32 *UeidExnLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u8* UeidSequenceLenIdx;
	u8* UeidSequenceValIdx;
	u32 OidLen;
	u32 FieldLen;
	u32 Dna[XCERT_DNA_LEN_IN_WORDS] = {0U};
	u32 Offset;
	u32 Address;
	u32 Idx;
	u32 Val;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_UeidExtn, sizeof(Oid_UeidExtn), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;


	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	UeidSequenceLenIdx = Curr++;
	UeidSequenceValIdx = Curr;

	for (Idx = 0U; Idx < XCERT_DNA_LEN_IN_WORDS; Idx++) {
		Offset = XCERT_DNA_LEN_IN_WORDS - Idx - 1U;
		Address = XCERT_DNA_0_ADDRESS + (Offset * XCERT_WORD_LEN);
		Val = XCert_In32(Address);
		Dna[Idx] = Xil_EndianSwap32(Val);
	}

	Status = XCert_CreateOctetString(Curr, (u8*)Dna, XCERT_DNA_LEN_IN_BYTES ,&FieldLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + FieldLen;

	*UeidSequenceLenIdx = (u8)(Curr - UeidSequenceValIdx);
	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*UeidExnLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}


/******************************************************************************/
/**
 * @brief	This function updates the value of Key Usage extension field
 * 		present in TBS Certificate as per the KeyUsageOption.
 *
 * @param	KeyUsageVal is the pointer in the Key Usage value buffer where
 *		the Key Usage option shall be updated.
 * @param	KeyUsageOption is type of key usage which has to be updated
 *
 ******************************************************************************/
static void XCert_UpdateKeyUsageVal(u8* KeyUsageVal, XCert_KeyUsageOption KeyUsageOption)
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
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Key Usage extension field shall be added.
 * @param	Cfg is structure which includes configuration for the TBS Certificate.
 * @param	KeyUsageExtnLen is the length of the Key Usage Extension field.
 *
 * @note	id-ce-keyUsage OBJECT IDENTIFIER ::=  { id-ce 15 }
 *
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
static int XCert_GenKeyUsageField(u8* TBSCertBuf, XCert_Config* Cfg, u32 *KeyUsageExtnLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u32 OidLen;
	u32 FieldLen;
	u8 KeyUsageVal[XCERT_MAX_LEN_OF_KEYUSAGE_VAL] = {0U};
	u32 KeyUsageValLen;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_KeyUsageExtn, sizeof(Oid_KeyUsageExtn), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	XCert_CreateBoolean(Curr, (u8)TRUE, &FieldLen);
	Curr = Curr + FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;

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

	Status = XCert_CreateBitString(Curr, KeyUsageVal, KeyUsageValLen, FALSE, &FieldLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + FieldLen;

	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*KeyUsageExtnLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the Extended Key Usage extension field
 * 		present in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Extended Key Usage extension field shall be added.
 * @param	Cfg is structure which includes configuration for the TBS Certificate.
 * @param	EkuLen is the length of the Extended Key Usage Extension field.
 *
 * @note	id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 }
 *		ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
 *		KeyPurposeId ::= OBJECT IDENTIFIER
 *
 ******************************************************************************/
static int XCert_GenExtKeyUsageField(u8* TBSCertBuf, XCert_Config* Cfg, u32 *EkuLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u8* EkuSequenceLenIdx;
	u8* EkuSequenceValIdx;
	u32 OidLen;
	u32 FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_EkuExtn, sizeof(Oid_EkuExtn), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	XCert_CreateBoolean(Curr, (u8)TRUE, &FieldLen);
	Curr = Curr + FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;


	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	EkuSequenceLenIdx = Curr++;
	EkuSequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_EkuClientAuth, sizeof(Oid_EkuClientAuth), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	if (Cfg->AppCfg.IsCsr == TRUE) {
		Status = XCert_CreateRawDataFromByteArray(Curr, Oid_EkuHwType, sizeof(Oid_EkuHwType), &OidLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Curr = Curr + OidLen;
	}

	*EkuSequenceLenIdx = (u8)(Curr - EkuSequenceValIdx);
	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*EkuLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 * 		TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the X.509 V3 extensions field shall be added.
 * @param	Cfg is structure which includes configuration for the TBS Certificate.
 * @param	ExtensionsLen is the length of the X.509 V3 Extensions field.
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated X.509 V3 Extensions field
 *		 - Error code  In case of failure
 *
 * @note	Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
 *
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
static int XCert_GenX509v3ExtensionsField(u8* TBSCertBuf,  XCert_Config* Cfg, u32 *ExtensionsLen)
{
	int Status = XST_FAILURE;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OptionalTagLenIdx;
	u8* OptionalTagValIdx;
	u32 Len;

	*(Curr++) = XCERT_OPTIONAL_PARAM_3_TAG;
	OptionalTagLenIdx = Curr++;
	OptionalTagValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_GenSubjectKeyIdentifierField(Curr, Cfg->AppCfg.SubjectPublicKey, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	Status = XCert_GenAuthorityKeyIdentifierField(Curr, Cfg->AppCfg.IssuerPublicKey, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	Status = XCert_GenTcbInfoExtnField(Curr, Cfg, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	/**
	 * UEID entension (2.23.133.5.4.4) should be added for self-signed
	 * DevIK certificates only
	 */
	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		Status = XCert_GenUeidExnField(Curr, &Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Curr = Curr + Len;
	}

	Status =  XCert_GenKeyUsageField(Curr, Cfg, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	if (Cfg->AppCfg.IsSelfSigned == TRUE) {
		Status = XCert_GenExtKeyUsageField(Curr, Cfg, &Len);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Curr = Curr + Len;
	}

	Status =  XCert_UpdateEncodedLength(SequenceLenIdx, (u32)(Curr - SequenceValIdx), SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);

	Status =  XCert_UpdateEncodedLength(OptionalTagLenIdx, (u32)(Curr - OptionalTagValIdx), OptionalTagValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*OptionalTagLenIdx) & XCERT_LOWER_NIBBLE_MASK);

	*ExtensionsLen = (u32)(Curr - TBSCertBuf);

END:
	return Status;
}


/******************************************************************************/
/**
 * @brief	This function creates the Basic Constraints extension field
 *
 * @param	CertReqInfoBuf is the pointer in the buffer where
 *		the Basic Constraints extension field shall be added.
 * @param	Len is the length of the Basic Constraints Extension field.
 *
 * @note	This extension shall be part of the CSR only
 *
 ******************************************************************************/
static int XCert_GenBasicConstraintsExtnField(u8* CertReqInfoBuf, u32 *Len)
{
	int Status = XST_FAILURE;
	u8* Curr = CertReqInfoBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OctetStrLenIdx;
	u8* OctetStrValIdx;
	u8* BasicConstraintSequenceLenIdx;
	u8* BasicConstraintSequenceValIdx;
	u32 OidLen;
	u32 FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_BasicConstraintExtn,
		sizeof(Oid_BasicConstraintExtn), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	XCert_CreateBoolean(Curr, (u8)TRUE, &FieldLen);
	Curr = Curr + FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_OCTETSTRING;
	OctetStrLenIdx = Curr++;
	OctetStrValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	BasicConstraintSequenceLenIdx = Curr++;
	BasicConstraintSequenceValIdx = Curr;

	XCert_CreateBoolean(Curr, (u8)TRUE, &FieldLen);
	Curr = Curr + FieldLen;

	*(Curr++) = XCERT_ASN1_TAG_INTEGER;
	*(Curr++) = XCERT_LEN_OF_VALUE_OF_PATH_LEN_CONSTRAINT;
	*(Curr++) = XCERT_PATH_LEN_CONSTRAINT_VALUE_0x0;

	*BasicConstraintSequenceLenIdx = (u8)(Curr - BasicConstraintSequenceValIdx);
	*OctetStrLenIdx = (u8)(Curr - OctetStrValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*Len = (u32)(Curr - CertReqInfoBuf);

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 * 		TBS Certificate.
 *
 * @param	CertReqInfoBuf is the pointer in the Certificate Request Info
 * 		buffer where extensions field shall be added.
 * @param	Cfg is structure which includes configuration for the Certificate Request Info
 * @param	ExtensionsLen is the length of the Extensions field.
 *
 ******************************************************************************/
static int XCert_GenCsrExtensions(u8* CertReqInfoBuf, XCert_Config* Cfg, u32 *ExtensionsLen)
{
	int Status = XST_FAILURE;
	u8* Curr = CertReqInfoBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* OptionalTagLenIdx;
	u8* OptionalTagValIdx;
	u8* ExtnReqSeqLenIdx;
	u8* ExtnReqSeqValIdx;
	u8* SetLenIdx;
	u8* SetValIdx;
	u32 Len;
	u32 OidLen;

	*(Curr++) = XCERT_OPTIONAL_PARAM_0_TAG;
	OptionalTagLenIdx = Curr++;
	OptionalTagValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	ExtnReqSeqLenIdx = Curr++;
	ExtnReqSeqValIdx = Curr;

	Status = XCert_CreateRawDataFromByteArray(Curr, Oid_ExtnRequest, sizeof(Oid_ExtnRequest), &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + OidLen;

	*(Curr++) = XCERT_ASN1_TAG_SET;
	SetLenIdx = Curr++;
	SetValIdx = Curr;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status =  XCert_GenKeyUsageField(Curr, Cfg, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	Status = XCert_GenExtKeyUsageField(Curr, Cfg, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	XCert_GenUeidExnField(Curr, &Len);
	Curr = Curr + Len;

	Status = XCert_GenBasicConstraintsExtnField(Curr, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	*SetLenIdx = (u8)(Curr - SetValIdx);
	*ExtnReqSeqLenIdx = (u8)(Curr - ExtnReqSeqValIdx);
	*OptionalTagLenIdx = (u8)(Curr - OptionalTagValIdx);
	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*ExtensionsLen = (u32)(Curr - CertReqInfoBuf);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the TBS(To Be Signed) Certificate.
 *
 * @param	TBSCertBuf is the pointer to the TBS Certificate buffer
 * @param	Cfg is structure which includes configuration for the TBS Certificate.
 * @param	TBSCertLen is the length of the TBS Certificate
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated TBS Certificate
 *		 - Error code  In case of failure
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
static int XCert_GenTBSCertificate(u8* TBSCertBuf, XCert_Config* Cfg, u32 *TBSCertLen)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u8* Start = TBSCertBuf;
	u8* Curr  = Start;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u8* SerialStartIdx;
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	u32 Len;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	*(Curr++) = XCERT_OPTIONAL_PARAM_0_TAG;
	*(Curr++) = XCERT_LEN_OF_VERSION_FIELD;

	/**
	 * Generate Version field
	 */
	XCert_GenVersionField(Curr, Cfg, &Len);
	Curr = Curr + Len;

	/**
	 * Store the start index for the Serial field. Once all the remaining
	 * fields are populated then the SHA2 hash is calculated for the
	 * remaining fields in the TBS certificate and Serial is 20 bytes from
	 * LSB of the calculated hash. Hence we need to store the start index of Serial
	 * so that it can be updated later. The total length of the Serial field is 22 bytes.
	 */
	SerialStartIdx = Curr;
	Curr = Curr + XCERT_SERIAL_FIELD_LEN;

	/**
	 * Generate Signature Algorithm field
	 */
	Status = XCert_GenSignAlgoField(Curr, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	/**
	 * Generate Issuer field
	 */
	Status = XCert_GenIssuerField(Curr, Cfg->UserCfg->Issuer, Cfg->UserCfg->IssuerLen, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	/**
	 * Generate Validity field
	 */
	Status = XCert_GenValidityField(Curr, Cfg->UserCfg->Validity, Cfg->UserCfg->ValidityLen, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

	/**
	 * Generate Subject field
	 */
	Status = XCert_GenSubjectField(Curr, Cfg->UserCfg->Subject, Cfg->UserCfg->SubjectLen, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Curr = Curr + Len;

#ifndef PLM_ECDSA_EXCLUDE
	/**
	 * Generate Public Key Info field
	 */
	Status = XCert_GenPublicKeyInfoField(Curr, Cfg->AppCfg.SubjectPublicKey, &Len);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD;
		goto END;
	}
	else {
		Curr = Curr + Len;
	}
#else
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
	goto END;
#endif

	/**
	 * Generate X.509 V3 extensions field
	 *
	 */
	Status = XCert_GenX509v3ExtensionsField(Curr, Cfg, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA384_KAT_MASK) != (u8)TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XCERT_ERR_X509_KAT_FAILED, Status, StatusTmp, XSecure_Sha384Kat);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_SHA384_KAT_MASK);
	}

	/**
	 * Calculate SHA2 Hash for all fields in the TBS certificate except Version and Serial
	 * Please note that currently SerialStartIdx points to the field after Serial.
	 * Hence this is the start pointer for calculating the hash.
	 */
	Status = XSecure_Sha384Digest((u8* )SerialStartIdx, (u32)(Curr - SerialStartIdx), Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Serial field
	 */
	Status = XCert_GenSerialField(SerialStartIdx, Hash, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Update the encoded length in the TBS certificate SEQUENCE
	 */
	Status =  XCert_UpdateEncodedLength(SequenceLenIdx, (u32)(Curr - SequenceValIdx), SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);

	*TBSCertLen = (u32)(Curr - Start);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Certification Request Info.
 *
 * @param	CertReqInfoBuf is address of the buffer which stores the Certification Request Info
 * @param	Cfg is structure which includes configuration for the Certification Request Info
 * @param	CertReqInfoLen is the length of the Certification Request Info
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Certification Request Info
 *		 - Error code  In case of failure
 *
 ******************************************************************************/
static int XCert_GenCertReqInfo(u8* CertReqInfoBuf, XCert_Config* Cfg, u32 *CertReqInfoLen)
{
	int Status = XST_FAILURE;
	u8* Start = CertReqInfoBuf;
	u8* Curr  = Start;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 Len;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	/**
	 * Generate Version field
	 */
	XCert_GenVersionField(Curr, Cfg, &Len);
	Curr = Curr + Len;

	/**
	 * Generate Subject field
	 */
	XCert_GenSubjectField(Curr, Cfg->UserCfg->Subject, Cfg->UserCfg->SubjectLen, &Len);
	Curr = Curr + Len;

#ifndef PLM_ECDSA_EXCLUDE
	/**
	 * Generate Public Key Info field
	 */
	Status = XCert_GenPublicKeyInfoField(Curr, Cfg->AppCfg.SubjectPublicKey, &Len);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD;
		goto END;
	}
	else {
		Curr = Curr + Len;
	}
#else
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
	goto END;
#endif

	XCert_GenCsrExtensions(Curr, Cfg, &Len);
	Curr = Curr + Len;

	Status =  XCert_UpdateEncodedLength(SequenceLenIdx, (u32)(Curr - SequenceValIdx), SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = (int)XCERT_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);

	*CertReqInfoLen = (u32)(Curr - Start);


END:
	return Status;
}

#ifndef PLM_ECDSA_EXCLUDE
/*****************************************************************************/
/**
 * @brief	This function creates the Signature field in the X.509 certificate
 *
 * @param	X509CertBuf is the pointer to the X.509 Certificate buffer
 * @param	Signature is value of the Signature field in X.509 certificate
 * @param	SignLen is the length of the Signature field
 *
 * @return
 *		 - XST_SUCCESS  Successfully generated Sign field
 *		 - Error code  In case of failure
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
static int XCert_GenSignField(u8* X509CertBuf, u8* Signature, u32 *SignLen)
{
	int Status = XST_FAILURE;
	u8* Curr = X509CertBuf;
	u8* BitStrLenIdx;
	u8* BitStrValIdx;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 Len;

	*(Curr++) = XCERT_ASN1_TAG_BITSTRING;
	BitStrLenIdx = Curr++;
	BitStrValIdx = Curr++;
	*BitStrValIdx = 0x00;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_CreateInteger(Curr, Signature, XSECURE_ECC_P384_SIZE_IN_BYTES, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	Status = XCert_CreateInteger(Curr, Signature + XSECURE_ECC_P384_SIZE_IN_BYTES,
		XSECURE_ECC_P384_SIZE_IN_BYTES, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	*SequenceLenIdx = (u8)(Curr - SequenceValIdx);
	*BitStrLenIdx = (u8)(Curr - BitStrValIdx);
	*SignLen = (u32)(Curr - X509CertBuf);

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function copies data to 32/64 bit address from
 *		local buffer.
 *
 * @param	Size 	- Length of data in bytes
 * @param	Src     - Pointer to the source buffer
 * @param	DstAddr - Destination address
 *
 *****************************************************************************/
static void XCert_CopyCertificate(const u32 Size, const u8 *Src, const u64 DstAddr)
{
	u32 Index = 0U;

	for (Index = 0U; Index < Size; Index++) {
		XSecure_OutByte64((DstAddr + Index), Src[Index]);
	}
}