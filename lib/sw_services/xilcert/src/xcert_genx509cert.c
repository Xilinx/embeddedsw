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
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"

#include "xil_util.h"
#ifndef PLM_ECDSA_EXCLUDE
#include "xsecure_elliptic.h"
#include "xsecure_ellipticplat.h"
#endif
#include "xsecure_sha384.h"
#include "xsecure_utils.h"
#include "xcert_genx509cert.h"
#include "xcert_createfield.h"
#include "xplmi.h"
#include "xplmi_plat.h"
#include "xplmi_status.h"
#include "xplmi_tamper.h"
#include "xsecure_plat_kat.h"

/************************** Constant Definitions *****************************/
#define XCERT_OID_SIGN_ALGO				"06082A8648CE3D040303"
#define XCERT_OID_EC_PUBLIC_KEY				"06072A8648CE3D0201"
#define XCERT_OID_P384					"06052B81040022"

#define XCERT_HASH_SIZE_IN_BYTES			(48U)
#define XCERT_SERIAL_FIELD_LEN				(22U)
#define XCERT_BIT7_MASK 				(0x80)
#define XCERT_MAX_CERT_SUPPORT				(4U)
#define XCERT_LOWER_NIBBLE_MASK				(0xFU)
	/**< Number of supported certificates is 4 -> 1 DevIK certificate and 3 DevAK certificates */

/************************** Function Prototypes ******************************/
static XCert_UserCfg *XCert_GetUserCfgDB(void);
static u32* XCert_GetNumOfEntriesInUserCfgDB(void);
static int XCert_IsBufferNonZero(u8* Buffer, int BufferLen);
static int XCert_GetUserCfg(u32 SubsystemId, XCert_UserCfg **UserCfg);
static void XCert_GenVersionField(u8* TBSCertBuf, u32 *VersionLen);
static void XCert_GenSerialField(u8* TBSCertBuf, u8* Serial, u32 *SerialLen);
static int XCert_GenSignAlgoField(u8* CertBuf, u32 *SignAlgoLen);
static void XCert_GenIssuerField(u8* TBSCertBuf, u8* Issuer, const u32 IssuerValLen, u32 *IssuerLen);
static void XCert_GenValidityField(u8* TBSCertBuf, u8* Validity, const u32 ValidityValLen, u32 *ValidityLen);
static void XCert_GenSubjectField(u8* TBSCertBuf, u8* Subject, const u32 SubjectValLen, u32 *SubjectLen);
#ifndef PLM_ECDSA_EXCLUDE
static int XCert_GenPubKeyAlgIdentifierField(u8* TBSCertBuf, u32 *Len);
static int XCert_GenPublicKeyInfoField(u8* TBSCertBuf, u8* SubjectPublicKey,u32 *PubKeyInfoLen);
static void XCert_GenSignField(u8* X509CertBuf, u8* Signature, u32 *SignLen);
#endif
static int XCert_GenTBSCertificate(u8* X509CertBuf, XCert_Config* Cfg, u32 *DataLen);
static void XCert_GetData(const u32 Size, const u8 *Src, const u64 DstAddr);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the UserCfg database.
 *
 * @return
 *	-	Pointer to the first entry in UserCfg database
 *
 * @note	UserCfgDB is used to store the user configurable fields of
 * 		X.509 certificate for different subsystems.
 *		Each entry in the DB wil have following fields:
 *		------------------------------------------------------
 *		| Subsystem Id  |  Issuer   |  Subject  |  Validity  |
 *		------------------------------------------------------
 *
 ******************************************************************************/
static XCert_UserCfg *XCert_GetUserCfgDB(void)
{
	static XCert_UserCfg UsrCfgDB[XCERT_MAX_CERT_SUPPORT] = {0U};

	return &UsrCfgDB[0];
}

/*****************************************************************************/
/**
 * @brief	This function provides the pointer to the NumOfEntriesInUserCfgDB
*		which indicates the total number of subsystems for which
*		user configuration is stored in UsrCfgDB.
 *
 * @return
 *	-	Pointer to the NumOfEntriesInUserCfgDB
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
 *		XST_SUCCESS - If buffer is non-empty
 *		XST_FAILURE - If buffer is empty
 *
 ******************************************************************************/
static int XCert_IsBufferNonZero(u8* Buffer, int BufferLen)
{
	int Status = XST_FAILURE;
	int Sum = 0;

	for (int i = 0; i < BufferLen; i++) {
		Sum |= Buffer[i];
	}

	if (Sum == 0) {
		Status = XST_FAILURE;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function finds the provided Subsystem ID in UserCfg DB and
 *		returns the pointer to the corresponding entry in DB
 *		if all the other fields are valid.
 *
 * @param	SubsystemID - Subsystem ID for which user configuration is requested
 * @param	UserCfg - Pointer to the entry in DB for the provided Subsystem ID
 *
 * @return
 *		XST_SUCCESS - If subsystem ID is found and other fields are valid
 *		Error Code - Upon any failure
 *
 ******************************************************************************/
static int XCert_GetUserCfg(u32 SubsystemId, XCert_UserCfg **UserCfg)
{
	int Status = XST_FAILURE;
	XCert_UserCfg *CertUsrCfgDB = XCert_GetUserCfgDB();
	u32 *NumOfEntriesInUserCfgDB = XCert_GetNumOfEntriesInUserCfgDB();
	u32 Idx;

	/**
	 * Search for given Subsystem ID in the UserCfg DB
	 */
	for (Idx = 0; Idx < *NumOfEntriesInUserCfgDB; Idx++) {
		if (CertUsrCfgDB[Idx].SubsystemId == SubsystemId) {
			/**
			 * If Subsystem ID is found then check that Subject,
			 * Issuer and Validity for that Subsystem ID is non-zero.
			 */
			Status = XCert_IsBufferNonZero(CertUsrCfgDB[Idx].Issuer,
				CertUsrCfgDB[Idx].IssuerLen);
			if (Status != XST_SUCCESS) {
				Status = XOCP_ERR_X509_INVALID_USER_CFG;
				goto END;
			}

			Status = XCert_IsBufferNonZero(CertUsrCfgDB[Idx].Subject,
				CertUsrCfgDB[Idx].SubjectLen);
			if (Status != XST_SUCCESS) {
				Status = XOCP_ERR_X509_INVALID_USER_CFG;
				goto END;
			}

			Status = XCert_IsBufferNonZero(CertUsrCfgDB[Idx].Validity,
				CertUsrCfgDB[Idx].ValidityLen);
			if (Status != XST_SUCCESS) {
				Status = XOCP_ERR_X509_INVALID_USER_CFG;
				goto END;
			}
			*UserCfg = &CertUsrCfgDB[Idx];
			Status = XST_SUCCESS;
			goto END;
		}
	}

	if (Idx == *NumOfEntriesInUserCfgDB) {
		Status = XOCP_ERR_X509_USR_CFG_NOT_FOUND;
		goto END;
	}

END:
	return Status;
}

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
 *		- XST_SUCCESS - If whole operation is success
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XCert_StoreCertUserInput(u32 SubSystemId, XCert_UserCfgFields FieldType, u8* Val, u32 Len)
{
	int Status = XST_FAILURE;
	u32 IdxToBeUpdated;
	u8 IsSubsystemIdPresent = FALSE;
	XCert_UserCfg *CertUsrCfgDB = XCert_GetUserCfgDB();
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
		if (CertUsrCfgDB[Idx].SubsystemId == SubSystemId) {
			IdxToBeUpdated = Idx;
			IsSubsystemIdPresent = TRUE;
			break;
		}
	}

	if (IsSubsystemIdPresent == FALSE) {
		IdxToBeUpdated = *NumOfEntriesInUserCfgDB;
		if (IdxToBeUpdated >= XCERT_MAX_CERT_SUPPORT) {
			Status = XOCP_ERR_X509_USER_CFG_STORE_LIMIT_CROSSED;
			goto END;
		}
		CertUsrCfgDB[IdxToBeUpdated].SubsystemId = SubSystemId;
		*NumOfEntriesInUserCfgDB = (*NumOfEntriesInUserCfgDB) + 1;
	}

	if (FieldType == XCERT_ISSUER) {
		XSecure_MemCpy(CertUsrCfgDB[IdxToBeUpdated].Issuer, Val, Len);
		CertUsrCfgDB[IdxToBeUpdated].IssuerLen = Len;
	}
	else if (FieldType == XCERT_SUBJECT) {
		XSecure_MemCpy(CertUsrCfgDB[IdxToBeUpdated].Subject, Val, Len);
		CertUsrCfgDB[IdxToBeUpdated].SubjectLen = Len;
	}
	else {
		XSecure_MemCpy(CertUsrCfgDB[IdxToBeUpdated].Validity, Val, Len);
		CertUsrCfgDB[IdxToBeUpdated].ValidityLen = Len;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates the X.509 Certificate.
 *
 * @param	X509CertBuf is the pointer to the X.509 Certificate buffer
 * @param	MaxCertSize is the maximum size of the X.509 Certificate buffer
 * @param	X509CertSize is the size of X.509 Certificate in bytes
 * @param	Cfg is structure which includes configuration for the X.509 Certificate.
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
	u32 TBSCertLen = 0U;
	u32 SignAlgoLen;
#ifndef PLM_ECDSA_EXCLUDE
	u32 SignLen;
	u8 Sign[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 SignTmp[XSECURE_ECC_P384_SIZE_IN_BYTES * 2U] = {0U};
	u8 Hash[XCERT_HASH_SIZE_IN_BYTES] = {0U};
#endif
	u8 HashTmp[XCERT_HASH_SIZE_IN_BYTES] = {0U};
	(void)MaxCertSize;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_GetUserCfg(Cfg->SubSystemId, &(Cfg->UserCfg));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate TBS certificate field
	 */
	Status = XCert_GenTBSCertificate(Curr, Cfg, &TBSCertLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + TBSCertLen;
	}

	/**
	 * Generate Sign Algorithm field
	 */
	Status = XCert_GenSignAlgoField(Curr, &SignAlgoLen);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_GEN_SIGN_ALGO_FIELD;
		goto END;
	}
	else {
		Curr = Curr + SignAlgoLen;
	}

	/**
	 * Calcualte SHA2 Digest of the TBS certificate
	 */
	Status = XSecure_Sha384Digest(Start, TBSCertLen, HashTmp);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_GEN_TBSCERT_DIGEST;
		goto END;
	}
#ifndef PLM_ECDSA_EXCLUDE

	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES, (u64)(UINTPTR)Hash,
					(u64)(UINTPTR)HashTmp);
	/**
	 * Calculate signature of the TBS certificate using the private key
	 */
	Status = XSecure_EllipticGenEphemeralNSign(XSECURE_ECC_NIST_P384, (const u8 *)Hash, sizeof(Hash),
				Cfg->AppCfg.PrvtKey, SignTmp);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_CALC_SIGN;
		goto END;
	}

	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
				(u64)(UINTPTR)Sign, (u64)(UINTPTR)SignTmp);
	XSecure_FixEndiannessNCopy(XSECURE_ECC_P384_SIZE_IN_BYTES,
			(u64)(UINTPTR)(Sign + XSECURE_ECC_P384_SIZE_IN_BYTES),
			(u64)(UINTPTR)(SignTmp + XSECURE_ECC_P384_SIZE_IN_BYTES));
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
	Status = XCert_UpdateEncodedLength(SequenceLenIdx, Curr - SequenceValIdx, SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);
	*X509CertSize = Curr - Start;

	XCert_GetData(*X509CertSize, (u8 *)X509CertBuf, X509CertAddr);

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
 *		XilCert library supports X.509 V3 certificates.
 *
 ******************************************************************************/
static void XCert_GenVersionField(u8* TBSCertBuf, u32 *VersionLen)
{
	u8* Curr = TBSCertBuf;

	*(Curr++) = XCERT_ASN1_TAG_INTEGER;
	*(Curr++) = XCERT_LEN_OF_VALUE_OF_VERSION;
	*(Curr++) = XCERT_VERSION_VALUE_V3;

	*VersionLen = Curr - TBSCertBuf;
}

/*****************************************************************************/
/**
 * @brief	This function creates the Serial field of the TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Serial field shall be added.
 * @param	SerialLen is the length of the Serial field
 *
 * @note	CertificateSerialNumber  ::=  INTEGER
 *		The length of the serial must not be more than 20 bytes.
 *		The value of the serial is determined by calculating the
 *		SHA2 hash of the fields in the TBS Certificate except the Version
 *		and Serial Number fields. 20 bytes from LSB of the calculated
 *		hash is updated as the Serial Number
 *
 ******************************************************************************/
static void XCert_GenSerialField(u8* TBSCertBuf, u8* DataHash, u32 *SerialLen)
{
	u8 Serial[XCERT_LEN_OF_VALUE_OF_SERIAL] = {0U};
	u32 LenToBeCopied;

	/**
	 * The value of serial field must be 20 bytes. If the most significant
	 * bit in the first byte of Serial is set, then the value shall be
	 * prepended with 0x00 after DER encoding.
	 * So if MSB is set then 00 followed by 19 bytes of hash wil be the serial
	 * value. If not set then 20 bytes of hash will be used as serial value.
	 */
	if ((*DataHash & XCERT_BIT7_MASK) == XCERT_BIT7_MASK) {
		LenToBeCopied = XCERT_LEN_OF_VALUE_OF_SERIAL - 1U;
	}
	else {
		LenToBeCopied = XCERT_LEN_OF_VALUE_OF_SERIAL;
	}

	XSecure_MemCpy64((u64)(UINTPTR)Serial, (u64)(UINTPTR)DataHash,
			LenToBeCopied);

	XCert_CreateInteger(TBSCertBuf, Serial, LenToBeCopied, SerialLen);
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

	Status = XCert_CreateRawDataFromStr(Curr, XCERT_OID_SIGN_ALGO, &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + OidLen;
	}

	*(Curr++) = XCERT_ASN1_TAG_NULL;
	*(Curr++) = XCERT_NULL_VALUE;

	*SequenceLenIdx = Curr - SequenceValIdx;
	*SignAlgoLen = Curr - CertBuf;

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
static void XCert_GenIssuerField(u8* TBSCertBuf, u8* Issuer, const u32 IssuerValLen, u32 *IssuerLen)
{
	XCert_CreateRawDataFromByteArray(TBSCertBuf, Issuer, IssuerValLen, IssuerLen);
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
static void XCert_GenValidityField(u8* TBSCertBuf, u8* Validity, const u32 ValidityValLen, u32 *ValidityLen)
{
	XCert_CreateRawDataFromByteArray(TBSCertBuf, Validity, ValidityValLen, ValidityLen);
}

/*****************************************************************************/
/**
 * @brief	This function creates the Subject field in TBS Certificate.
 *
 * @param	TBSCertBuf is the pointer in the TBS Certificate buffer where
 *		the Subject field shall be added.
 * @param	Subject is the DER encoded value of the Subject field
 * @param	IssuerValLen is the length of the DER encoded value
 * @param	SubjectLen is the length of the Subject field
 *
 * @note	This function expects the user to provide the Subject field in DER
 *		encoded format and it will be updated in the TBS Certificate buffer.
 *
 ******************************************************************************/
static void XCert_GenSubjectField(u8* TBSCertBuf, u8* Subject, const u32 SubjectValLen, u32 *SubjectLen)
{
	XCert_CreateRawDataFromByteArray(TBSCertBuf, Subject, SubjectValLen, SubjectLen);
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

	Status = XCert_CreateRawDataFromStr(Curr, XCERT_OID_EC_PUBLIC_KEY, &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + OidLen;
	}

	Status = XCert_CreateRawDataFromStr(Curr, XCERT_OID_P384, &OidLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + OidLen;
	}

	*SequenceLenIdx = Curr - SequenceValIdx;
	*Len = Curr - TBSCertBuf;

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
 * @note	SubjectPublicKeyInfo  ::=  SEQUENCE  {
			algorithm            AlgorithmIdentifier,
			subjectPublicKey     BIT STRING  }
 *
 ******************************************************************************/
static int XCert_GenPublicKeyInfoField(u8* TBSCertBuf, u8* SubjectPublicKey, u32 *PubKeyInfoLen)
{
	int Status = XST_FAILURE;
	u32 KeyLen = XSECURE_ECC_P384_SIZE_IN_BYTES + XSECURE_ECC_P384_SIZE_IN_BYTES;
	u8* Curr = TBSCertBuf;
	u8* SequenceLenIdx;
	u8* SequenceValIdx;
	u32 Len;

	*(Curr++) = XCERT_ASN1_TAG_SEQUENCE;
	SequenceLenIdx = Curr++;
	SequenceValIdx = Curr;

	Status = XCert_GenPubKeyAlgIdentifierField(Curr, &Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	XCert_CreateBitString(Curr, SubjectPublicKey, KeyLen, &Len);
	Curr = Curr + Len;

	*SequenceLenIdx = Curr - SequenceValIdx;
	*PubKeyInfoLen = Curr - TBSCertBuf;

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function creates the TBS(To Be Signed) Certificate.
 *
 * @param	TBSCertBuf is the pointer to the TBS Certificate buffer
 * @param	Cfg is structure which includes configuration for the TBS Certificate.
 * @param	TBSCertLen is the length of the TBS Certificate
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

	/**
	 * Generate Version field
	 */
	XCert_GenVersionField(Curr, &Len);
	Curr = Curr + Len;

	/**
	 * Store the start index for the Serial field. Once all the remaining
	 * fields are populated then the SHA2 hash is calculated for the
	 * remaining fields in the TBS certificate and Serial is 20 bytes from
	 * LSB of the calculated hash. Hence we need to store the start index of Serial
	 * so that it can be updated later
	 */
	SerialStartIdx = Curr;

	/**
	 * Generate Signature Algorithm field
	 */
	Status = XCert_GenSignAlgoField(Curr, &Len);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_GEN_TBSCERT_SIGN_ALGO_FIELD;
		goto END;
	}
	else {
		Curr = Curr + Len;
	}

	/**
	 * Generate Issuer field
	 */
	XCert_GenIssuerField(Curr, Cfg->UserCfg->Issuer, Cfg->UserCfg->IssuerLen, &Len);
	Curr = Curr + Len;

	/**
	 * Generate Validity field
	 */
	XCert_GenValidityField(Curr, Cfg->UserCfg->Validity, Cfg->UserCfg->ValidityLen, &Len);
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
		Status = XOCP_ERR_X509_GEN_TBSCERT_PUB_KEY_INFO_FIELD;
		goto END;
	}
	else {
		Curr = Curr + Len;
	}
#else
	Status = XOCP_ECDSA_NOT_ENABLED_ERR;
	goto END;
#endif

	if (XPlmi_IsKatRan(XPLMI_SECURE_SHA384_KAT_MASK) != TRUE) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XOCP_ERR_KAT_FAILED, Status, StatusTmp, XSecure_Sha384Kat);
		if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
			goto END;
		}
		XPlmi_SetKatMask(XPLMI_SECURE_SHA384_KAT_MASK);
	}

	/**
	 * Calculate SHA2 Hash for all fields in the TBS certificate except Version and Serial
	 * Please note that currently SerialStartIdx points to the field after Serial.
	 * Hence this is the start pointer for calcualting the hash.
	 */
	Status = XSecure_Sha384Digest((u8* )SerialStartIdx, Curr - SerialStartIdx, Hash);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * The fields after Serial have to be moved to make space for updating the Serial field
	 * Since the length of value of Serial field is 20 bytes and total length of Serial field
	 * (including tyag, length and value) is 22 bytes. So the remaining fields
	 * are moved by 22 bytes
	 */
	Status = Xil_SMemMove(SerialStartIdx + XCERT_SERIAL_FIELD_LEN, Curr - SerialStartIdx,
		SerialStartIdx, Curr - SerialStartIdx, Curr - SerialStartIdx);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Generate Serial field
	 */
	XCert_GenSerialField(SerialStartIdx, Hash, &Len);
	Curr = Curr + XCERT_SERIAL_FIELD_LEN;

	/**
	 * Update the encoded length in the TBS certificate SEQUENCE
	 */
	Status =  XCert_UpdateEncodedLength(SequenceLenIdx, Curr - SequenceValIdx, SequenceValIdx);
	if (Status != XST_SUCCESS) {
		Status = XOCP_ERR_X509_UPDATE_ENCODED_LEN;
		goto END;
	}
	Curr = Curr + ((*SequenceLenIdx) & XCERT_LOWER_NIBBLE_MASK);

	*TBSCertLen = Curr - Start;

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
static void XCert_GenSignField(u8* X509CertBuf, u8* Signature, u32 *SignLen)
{
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

	XCert_CreateInteger(Curr, Signature, XSECURE_ECC_P384_SIZE_IN_BYTES, &Len);
	Curr = Curr + Len;

	XCert_CreateInteger(Curr, Signature + XSECURE_ECC_P384_SIZE_IN_BYTES,
		XSECURE_ECC_P384_SIZE_IN_BYTES, &Len);
	Curr = Curr + Len;

	*SequenceLenIdx = Curr - SequenceValIdx;
	*BitStrLenIdx = Curr - BitStrValIdx;
	*SignLen = Curr - X509CertBuf;
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
static void XCert_GetData(const u32 Size, const u8 *Src, const u64 DstAddr)
{
	u32 Index = 0U;

	for (Index = 0U; Index < Size; Index++) {
		XSecure_OutByte64((DstAddr + Index), Src[Index]);
	}
}
