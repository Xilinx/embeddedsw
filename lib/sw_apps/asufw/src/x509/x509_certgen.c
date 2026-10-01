/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file x509_certgen.c
*
* This file contains the implementation of the interface functions for creating
* X.509 certificate for DevIK and DevAK public keys.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------------------------
* 1.0   rmv  05/19/25 Initial release
*       rmv  08/06/25 Move X509_CERTIFICATE_MAX_SIZE_IN_BYTES macro to header file
*       rmv  09/08/25 Move parameter validation to exported function instead of static function
*       vm   02/04/26 Replace magic number with named constant
*       vm   02/02/26 Assigned proper suffix to integer constants to fix MISRA-C
*                     violation for Rule 7.2
*       rmv  08/04/26 Add support for basic constraints extension
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup x509_server_apis X.509 Server APIs
 * @{
 */
/************************************** Include Files ********************************************/
#include "x509_cert.h"
#include "xasu_eccinfo.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xil_sutil.h"

/********************************** Constant Definitions *****************************************/
/**< OIDs used in X.509 Certificate and Certificate Signing Request */
static const u8 Oid_SignAlgoSha3_384[] = {0x06U, 0x09U, 0x60U, 0x86U, 0x48U, 0x01U, 0x65U, 0x03U,
					  0x04U, 0x03U, 0x0BU};
static const u8 Oid_EccPublicKey[] = {0x2AU, 0x86U, 0x48U, 0xCEU, 0x3DU, 0x02U, 0x01U};
static const u8 Oid_P384[] = {0x06U, 0x05U, 0x2BU, 0x81U, 0x04U, 0x00U, 0x22U};

/************************************ Macro Definitions ******************************************/
#define X509_VERSION_FIELD_LEN					(0x3U)	/**< Length of Version
									field */
#define X509_VERSION_VAL_LEN					(0x01U)	/**< Length of Version
									field */
#define X509_VERSION_VALUE_V1					(0x00U)	/**< Value of version 1 */
#define X509_SERIAL_VAL_LEN					(0x14U)	/**< Length of Serial
									number field */
#define X509_SERIAL_FIELD_LEN					(22U)	/**< Length of Serial
									Field */
#define X509_SERIAL_FIRST_BYTE_FALLBACK				(0x40U)	/**< Non-zero, MSB-clear
									fallback for the first
									serial byte */
#define X509_SERIAL_FIRST_BYTE_INDEX				(0U)	/**< Index of the first
									byte of the serial value */
#define X509_AUTH_KEY_ID_VAL_LEN				(20U)	/**< Length of value of
									Authority Key ID */
#define X509_SUB_KEY_ID_VAL_LEN					(20U)	/**< Length of value of
									Subject Key ID */
#define X509_KEYUSAGE_VAL_LEN					(2U)	/**< Length of value of key
									usage */
#define X509_UNCOMPRESSED_PUB_KEY				(0x04U)	/**< To indicate
									uncompressed public key */
#define X509_LOWER_NIBBLE_MASK					(0xFU)	/**< Mask to get lower
									nibble */
#define X509_HASH_MAX_SIZE_IN_BYTES				(48U)	/**< Maximum length of
									hash */
#define X509_SIGNATURE_MAX_SIZE_IN_BYTES			(96U)	/**< Maximum length of
									signature */
#define X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN		(97U)	/**< ECC uncompressed public
									key length */
#define X509_ECDSA_SIG_R_S_SIZE_DIVISOR				(2U)	/**< Divisor to split ECDSA
									signature into R and S
									components */
#define X509_ONE_BYTE						(1U)	/**< Buffer validation for
									single byte access */
#define X509_TAG_LEN_VAL_MIN_SIZE				(3U)	/**< Minimum size of tag,
									length and value field */
#define X509_TAG_BYTE_FIELD_SIZE				(2U)	/**< Size of tag and byte
									field */
#define X509_ASN1_INTEGER_SIGN_BYTE_PREFIX			(0x00U)	/**< ASN.1 DER integer sign byte
									prefix for positive values with
									MSB set */
#define X509_ASN1_ZERO_UNUSED_BITS				(0x00U)	/**< Zero unused bits in BIT
									STRING */
#define X509_PATH_LEN_CONSTRAINT_VALUE				(0x00U)	/**< Value of path length
									constraint in basic
									constraints extension */
#define X509_ASN1_MAX_ENCODED_VAL_LEN				(0xFFFFU) /**< Maximum value length
									which can be DER encoded */
#define X509_ASN1_MAX_LEN_FIELD_SIZE				(3U)	/**< Number of bytes
									occupied by the longest
									supported DER encoded
									length field */
#define X509_UTC_TIME_LEN	\
	(X509_ASN1_TIME_WITH_ZERO_TIME_OFFSET(X509_ASN1_YEAR_LEN_UTC_BYTES))
						/**< Length of a time in the UTC form
						YYMMDDHHMMSSZ */
#define X509_GENERALIZED_TIME_LEN	\
	(X509_ASN1_TIME_WITH_ZERO_TIME_OFFSET(X509_ASN1_YEAR_LEN_GEN_BYTES))
						/**< Length of a time in the generalized
						form YYYYMMDDHHMMSSZ */
#define X509_ASN1_MIN_UTC_YEAR	\
	(X509_ASN1_YEAR_MIN_OFFSET + X509_ASN1_THRESHOLD_YEARS)
						/**< Earliest year which the two digit year
						of the UTC form can represent */
#define X509_ASN1_MIN_GENERALIZED_YEAR	\
	(X509_ASN1_YEAR_MAX_OFFSET + X509_ASN1_THRESHOLD_YEARS)
						/**< Earliest year which RFC 5280 requires
						to be encoded in the generalized form */
#define X509_ASN1_YEARS_PER_CENTURY			(100U)	/**< Number of years in a century */

/************************************ Type Definitions *******************************************/
/**
 * This structure holds certificate generation information.
 */
typedef struct {
	u8 *Buf;			/**< Pointer to the buffer storing the DER encoded data */
	u32 BufSize;			/**< Size of the buffer pointed by Buf */
	u32 Offset;			/**< Current offset of buffer */
	u32 SerialOffset;		/**< Offset of serial field */
} X509_CertGenInfo;

/********************************** Variable Definitions *****************************************/
/**< X.509 public key OID list */
static X509_OidPublicKeyDescriptor PubKeyOidList[X509_PUB_KEY_MAX] = {
	[X509_PUB_KEY_UNSUPPORTED] = {
		.PubKeyType = X509_PUB_KEY_UNSUPPORTED,
		.Oid = NULL,
		.OidLen = 0U,
	},
	[X509_PUB_KEY_ECC] = {
		.PubKeyType = X509_PUB_KEY_ECC,
		.Oid = Oid_EccPublicKey,
		.OidLen = (u8)XASUFW_ARRAY_SIZE(Oid_EccPublicKey),
	},
};

/**< X.509 algorithm parameter OID list */
static X509_AlgoEccParam ParamOidList[X509_ECC_CURVE_TYPE_MAX] = {
	[X509_ECC_CURVE_TYPE_384] = {
		.EccCurveType = X509_ECC_CURVE_TYPE_384,
		.ParamOid = Oid_P384,
		.ParamOidLen = (u8)XASUFW_ARRAY_SIZE(Oid_P384),
	},
};

/**< X.509 signature OID list */
static X509_SignatureOidDescriptor SignOidList[X509_SIGN_TYPE_MAX] = {
	[X509_SIGN_TYPE_ECC_SHA3_384] = {
		.SignType = X509_SIGN_TYPE_ECC_SHA3_384,
		.SignOid = Oid_SignAlgoSha3_384,
		.SignLen = (u8)XASUFW_ARRAY_SIZE(Oid_SignAlgoSha3_384),
	},
};

static X509_CertGenInfo CertInstance; /**< X.509 certificate generation instance used to hold
					certificate information */

/************************************ Function Prototypes ****************************************/
static inline s32 X509_ValidateBufferSpace(u32 RequiredSize);
static s32 X509_ValidateAndUpdateTlvField(u8 Asn1Tag, u8 **LenIdx, u8 **ValIdx);
static s32 X509_ValidateAndUpdateByteField(u8 Value);
static s32 X509_UpdateEncodedLength(u8 *LenIdx, u32 Len, u8 *ValIdx);
static X509_InitData *X509_GetInitData(void);
static u32 X509_Asn1GetFirstNonZeroByteOffset(const u8 *Data, u32 Cnt);
static s32 X509_Asn1CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen);
static s32 X509_Asn1CreateInteger(const u8 *IntegerVal, u32 IntegerLen);
static s32 X509_Asn1CreateRawDataFromByteArray(const u8 *RawData, const u32 LenOfRawDataVal);
static u8 X509_Asn1GetTrailingZeroesCount(u8 Data);
static s32 X509_Asn1CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull);
static s32 X509_Asn1CreateTlvField(u8 Asn1Tag, const u8 *Val, u32 ValLen);
static inline s32 X509_Asn1CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen);
static inline s32 X509_Asn1CreatePrintableString(const u8 *Val, u32 ValLen);
static s32 X509_Asn1CreateNameAttribute(const u8 *OidTlv, u32 OidTlvLen, const u8 *Val,
					u32 ValLen);
static s32 X509_Asn1CreateBoolean(const u32 BooleanVal);
static inline s32 X509_GenVersionField(u8 Version);
static s32 X509_GenSignAlgoField(void);
static inline s32 X509_GenIssuerField(const u8 *Issuer, const u32 IssuerValLen);
static inline s32 X509_GenValidityField(const u8 *Validity, const u32 ValidityValLen);
static inline s32 X509_GenSubjectField(const u8 *Subject, const u32 SubjectValLen);
static s32 X509_GenPubKeyAlgIdentifierField(const X509_SubjectPublicKeyInfo *PubKeyInfo);
static s32 X509_GenPublicKeyInfoField(const X509_SubjectPublicKeyInfo *PubKeyInfo);
static s32 X509_GenSubjectKeyIdentifierField(const u8 *SubjectPublicKey, u32 SubjectPubKeyLen,
					     const void *PlatformData);
static s32 X509_GenAuthorityKeyIdentifierField(const u8 *IssuerPublicKey, u32 IssuerPubKeyLen,
					       const void *PlatformData);
static void X509_UpdateKeyUsageVal(u8 *KeyUsageVal, u8 KeyUsageBitPos);
static inline void X509_UpdateKeyUsageValFromMask(u8 *KeyUsageVal, u32 KeyUsageMask);
static s32 X509_GenKeyUsageField(const X509_Config *Cfg);
static s32 X509_GenExtKeyUsageField(void);
static s32 X509_GenBasicConstraintsField(void);
static inline s32 X509_GenSubAltNameField(const u8 *SubAltName, const u32 SubAltNameValLen);
static inline void X509_AddTagField(u8 Asn1Tag);
static inline s32 X509_ValidateAndAddTagField(u8 Asn1Tag);
static s32 X509_ValidateAndAddTagByteFields(u8 Asn1Tag, u8 ByteValue);
static s32 X509_GenX509v3ExtensionsField(const X509_Config *Cfg);
static s32 X509_GenSerialField(const u8 *DataHash);
static s32 X509_GenTBSCertificate(const X509_Config *Cfg, u32 *TBSCertLen);
static s32 X509_GenSignField(const u8 *Signature, u32 SignLen);
static s32 X509_GenCsrExtensions(const X509_Config *Cfg);
static s32 X509_GenCertReqInfo(const X509_Config *Cfg, u32 *CsrLen);
static s32 X509_Asn1EncodeName(const XAsu_X509Name *Name, u8 *DerBuf, u32 DerBufSize,
			       u32 *DerLen);
static s32 X509_Asn1EncodeValidity(const XAsu_X509Validity *Validity, u8 *DerBuf,
				   u32 DerBufSize, u32 *DerLen);
static u32 X509_Asn1IsPrintableChar(u8 Char);
static s32 X509_ValidatePrintableString(const u8 *Str, u32 StrLen, u32 MaxStrLen);
static s32 X509_Asn1CreateCertTime(const XAsu_X509Time *Time);
static s32 X509_ValidateName(const XAsu_X509Name *Name);
static s32 X509_ValidateValidity(const XAsu_X509Validity *Validity);
static inline void X509_GetDateTime(const XAsu_X509Time *Time, X509_DateTime *DateTime);

/*************************************************************************************************/
/**
 * @brief	This function creates the X.509 Certificate/Certificate Signing Request(CSR).
 *
 * @param	X509CertAddr	Address of the buffer for storing the generated X.509 certificate.
 * @param	MaxCertSize	Maximum size of the input buffer used for storing X.509 certificate.
 * @param	X509CertSize	Pointer to store the actual size of the X.509 certificate copied to
 *				the given buffer.
 * @param	Cfg		Pointer to structure which includes configuration for the X.509
 *				certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if X.509 certificate or CSR is created successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_INVALID_PARAM, if parameter is invalid.
 *	- XASUFW_X509_UNSUPPORTED_ALGORITHM, if Public Key Algorithm is not supported.
 *	- XASUFW_X509_UNSUPPORTED_CURVE_TYPE, if curve type is not supported.
 *	- XASUFW_X509_CERT_GEN_FAIL, if certificate generation is failed.
 *	- XASUFW_X509_DIGEST_SIGN_CALL_BACK_NOT_REGISTERED, if digest or sign callback is not
 *	  registered.
 *	- XASUFW_X509_GENERATE_DIGEST_FAIL, if digest calculation is failed.
 *	- XASUFW_X509_GENERATE_SIGN_FAIL, if signature generation is failed.
 *	- XASUFW_X509_GEN_SIGN_ALGO_FIELD_FAIL, if signature algorithm field generation is failed.
 *	- XASUFW_X509_GEN_SIGN_FIELD_FAIL, if signature field generation is failed.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if update encoded length is failed.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if field length is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	Certificate  ::=  SEQUENCE  {
 *			tbsCertificate       TBSCertificate,
 *			signatureAlgorithm   AlgorithmIdentifier,
 *			signatureValue       BIT STRING
 *		}
 *
 * @note	The currently this API supports only 32-bit addresses. Data type for X509CertAddr is
 *		64-bit to ensure future compatibility with system that may require 64-bit
 *		addressing.
 *
 *************************************************************************************************/
s32 X509_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32 *X509CertSize,
			  const X509_Config *Cfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const X509_InitData *InitData = X509_GetInitData();
	u32 DataLen = 0U;
	u32 HashLen = 0U;
	u32 SignLen = 0U;
	u32 CertSize;
	const u8 *TBSCertStart = NULL;
	const u8 Hash[X509_HASH_MAX_SIZE_IN_BYTES] = {0U};
	const u8 Sign[X509_SIGNATURE_MAX_SIZE_IN_BYTES] = {0U};
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;

	/** Validate certificate address, size and config. */
	if ((X509CertAddr == 0U) || (X509CertSize == NULL) || (Cfg == NULL) ||
	    (MaxCertSize < X509_CERTIFICATE_MAX_SIZE_IN_BYTES)) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate X.509 configs. */
	if ((Cfg->UserCfg == NULL) || ((Cfg->IsCsr != XASU_TRUE) && (Cfg->IsCsr != XASU_FALSE)) ||
	    ((Cfg->IsSelfSigned != XASU_TRUE) && (Cfg->IsSelfSigned != XASU_FALSE)) ||
	    (Cfg->KeyUsage == 0U)) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate X.509 user configs. */
	if ((Cfg->UserCfg->IssuerLen == 0U) || (Cfg->UserCfg->SubjectLen == 0U) ||
	    (Cfg->UserCfg->ValidityLen == 0U)) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate X.509 user config field lengths against the buffers which hold them. */
	if ((Cfg->UserCfg->IssuerLen > X509_ISSUER_MAX_SIZE) ||
	    (Cfg->UserCfg->SubjectLen > X509_SUBJECT_MAX_SIZE) ||
	    (Cfg->UserCfg->ValidityLen > X509_VALIDITY_MAX_SIZE)) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	/** Validate Subject Alternative Name length, if it is provided by the user. */
	if ((Cfg->UserCfg->IsSubAltNameAvailable == XASU_TRUE) &&
	    (Cfg->UserCfg->SubAltNameLen > X509_SUB_ALT_NAME_MAX_SIZE)) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	/** Validate digest and signature callbacks. */
	if ((InitData->GenerateDigest == NULL) || (InitData->GenerateSignature == NULL)) {
		Status = XASUFW_X509_DIGEST_SIGN_CALL_BACK_NOT_REGISTERED;
		goto END;
	}

	/** Validate public key type. */
	if (Cfg->PubKeyInfo.PubKeyType != X509_PUB_KEY_ECC) {
		Status = XASUFW_X509_UNSUPPORTED_ALGORITHM;
		goto END;
	}

	/** Validate public key curve type. */
	if (Cfg->PubKeyInfo.EccCurveType != X509_ECC_CURVE_TYPE_384) {
		Status = XASUFW_X509_UNSUPPORTED_CURVE_TYPE;
		goto END;
	}

	/** Validate subject public key. */
	if ((Cfg->PubKeyInfo.SubjectPublicKey == NULL) || (Cfg->PubKeyInfo.SubjectPubKeyLen !=
	     XAsu_DoubleCurveLength(XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES))) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Initialize X.509 certificate generation instance. */
	CertInstance.Buf = (u8 *)(UINTPTR)X509CertAddr;
	CertInstance.BufSize = MaxCertSize;
	CertInstance.Offset = 0U;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	TBSCertStart = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate CSR or TBS(To Be Signed) depending on the request. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	if (Cfg->IsCsr == XASU_TRUE) {
		Status = X509_GenCertReqInfo(Cfg, &DataLen);
	} else {
		Status = X509_GenTBSCertificate(Cfg, &DataLen);
	}

	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CERT_GEN_FAIL);
		goto END;
	}

	/** Generate Signature Algorithm field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSignAlgoField();
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SIGN_ALGO_FIELD_FAIL);
		goto END;
	}

	/** Calculate digest for TBS certificate. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateDigest(TBSCertStart, DataLen, Hash,
					  (u32)X509_HASH_MAX_SIZE_IN_BYTES, &HashLen,
					  Cfg->PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GENERATE_DIGEST_FAIL);
		goto END;
	}

	/** Generate signature for calculated hash of TBS. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateSignature(Hash, HashLen, Sign,
					     X509_SIGNATURE_MAX_SIZE_IN_BYTES, &SignLen,
					     Cfg->IssuerPrvtKey, Cfg->PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GENERATE_SIGN_FAIL);
		goto END;
	}

	/** Generate Signature field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSignField(Sign, SignLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SIGN_FIELD_FAIL);
		goto END;
	}

	/** Update the encoded length in the X.509 certificate SEQUENCE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	CertSize = CertInstance.Offset;
	*X509CertSize = CertSize;

END:
	return Status;

}

/*************************************************************************************************/
/**
 * @brief	This function initialize the callbacks and data for the digest and signature
 *		calculation.
 *
 * @param	CfgData		Pointer to structure containing data required to calculate digest
 *		and signature calculation.
 *
 * @return
 *	- XASUFW_SUCCESS, in case of success.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PARAM, if parameter is NULL.
 *
 *************************************************************************************************/
s32 X509_Init(const X509_InitData *CfgData)
{
	s32 Status = XASUFW_FAILURE;
	X509_InitData *InitData = X509_GetInitData();

	/** Validate input parameter. */
	if (CfgData == NULL) {
		Status = XASUFW_X509_INVALID_PARAM;
		goto END;
	}

	/** Assign signature type config and callbacks for digest and signature generation */
	InitData->SignType = CfgData->SignType;
	InitData->GenerateDigest = CfgData->GenerateDigest;
	InitData->GenerateSignature = CfgData->GenerateSignature;

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function builds the DER encoded user configuration of an X.509 certificate
 *		from the user configuration.
 *
 * @param	CertFields	Pointer to the structure holding the user supplied certificate
 *				fields.
 * @param	UserCfg		Pointer to the structure to be filled with the DER encoded
 *				certificate fields.
 *
 * @return
 *	- XASUFW_SUCCESS, if the user configuration is built successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PARAM, if parameter is invalid.
 *	- XASUFW_X509_GEN_SUBJECT_FIELD_FAIL, if the subject is invalid or its DER encoding is
 *	  failed.
 *	- XASUFW_X509_GEN_ISSUER_FIELD_FAIL, if the issuer is invalid or its DER encoding is failed.
 *	- XASUFW_X509_GEN_VALIDITY_FIELD_FAIL, if the validity period is invalid or its DER encoding
 *	  is failed.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if zeroize memset is failed.
 *
 *************************************************************************************************/
s32 X509_BuildUserCfg(const XAsu_X509CertFields *CertFields, X509_UserCfg *UserCfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate input parameters. */
	if ((CertFields == NULL) || (UserCfg == NULL)) {
		Status = XASUFW_X509_INVALID_PARAM;
		goto END;
	}

	/** Clear first, so a rejected call leaves nothing from an earlier one. */
	Status = Xil_SMemSet(UserCfg, sizeof(X509_UserCfg), 0U, sizeof(X509_UserCfg));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Validate the subject name. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateName(&CertFields->Subject);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SUBJECT_FIELD_FAIL);
		goto END;
	}

	/** Validate the issuer name. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateName(&CertFields->Issuer);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_ISSUER_FIELD_FAIL);
		goto END;
	}

	/** Validate the validity period. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateValidity(&CertFields->Validity);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VALIDITY_FIELD_FAIL);
		goto END;
	}

	/** DER encode the subject name. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1EncodeName(&CertFields->Subject, UserCfg->Subject, X509_SUBJECT_MAX_SIZE,
				     &UserCfg->SubjectLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SUBJECT_FIELD_FAIL);
		goto END;
	}

	/** DER encode the issuer name. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1EncodeName(&CertFields->Issuer, UserCfg->Issuer, X509_ISSUER_MAX_SIZE,
				     &UserCfg->IssuerLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_ISSUER_FIELD_FAIL);
		goto END;
	}

	/** DER encode the validity period. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1EncodeValidity(&CertFields->Validity, UserCfg->Validity,
					 X509_VALIDITY_MAX_SIZE, &UserCfg->ValidityLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VALIDITY_FIELD_FAIL);
		goto END;
	}

	/** Set the subject alternate name availability flag to false. */
	UserCfg->IsSubAltNameAvailable = (u32)XASU_FALSE;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function encodes the Length field in the DER encoded value
 *		and updates in the provided pointer. In case the Length field
 *		requires more than one byte, it also shifts the value accordingly.
 *
 * @param	LenIdx	Pointer to the Length field of the encoded value.
 * @param	Len	Length of the Value field in bytes.
 * @param	ValIdx	Pointer to the Value field of the encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if updating encoded length is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_MEM_MOVE_FAIL, if memory move is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_UpdateEncodedLength(u8 *LenIdx, u32 Len, u8 *ValIdx)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 RemainBufSize = 0U;

	RemainBufSize = CertInstance.BufSize - CertInstance.Offset;

	if (Len <= X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*LenIdx = (u8)Len;
		Status = XASUFW_SUCCESS;
	} else if (Len <= X509_ASN1_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES) {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_1BYTE;
		Status = Xil_SMemMove(ValIdx + XASUFW_BUFFER_INDEX_ONE,
				      Len - XASUFW_VALUE_ONE + RemainBufSize, ValIdx, Len, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_MOVE_FAIL;
			goto END;
		}
		*(LenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)Len;
	} else {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_2BYTES;
		Status = Xil_SMemMove(ValIdx + XASUFW_BUFFER_INDEX_TWO,
				      Len - XASUFW_VALUE_TWO + RemainBufSize, ValIdx, Len, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_MOVE_FAIL;
			goto END;
		}
		*(LenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)((Len & X509_ASN1_BYTE1_MASK) >>
							   X509_ASN1_NO_OF_BITS_IN_BYTE);
		*(LenIdx + XASUFW_BUFFER_INDEX_TWO) = (u8)(Len & X509_ASN1_BYTE0_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates if there is required space available in the destination
 *		buffer of the DER encoding instance.
 *
 * @param	RequiredSize	Number of bytes required in the buffer.
 *
 * @return
 *	- XASUFW_SUCCESS, if sufficient buffer space is available in the destination buffer.
 *	- XASUFW_FAILURE, if sufficient buffer space is not available in the destination buffer.
 *
 *************************************************************************************************/
static inline s32 X509_ValidateBufferSpace(u32 RequiredSize)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	if ((CertInstance.Offset + RequiredSize) < CertInstance.BufSize) {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates available space in the destination buffer and updates ASN.1
 *		tag, length and value field.
 *
 * @param	Asn1Tag		ASN.1 tag to be added.
 * @param	LenIdx		Pointer to store the address of length index.
 * @param	ValIdx		Pointer to store the address of value index.
 *
 * @return
 *	- XASUFW_SUCCESS, if tag is added, buffer space is sufficient and pointers are set.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 X509_ValidateAndUpdateTlvField(u8 Asn1Tag, u8 **LenIdx, u8 **ValIdx)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate buffer space for tag, length and value fields. */
	Status = X509_ValidateBufferSpace(X509_TAG_LEN_VAL_MIN_SIZE);
	if (Status == XASUFW_SUCCESS) {
		/** Add ASN.1 tag field. */
		X509_AddTagField(Asn1Tag);

		/** Allocate buffer space for the length and value fields. */
		*LenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
		*ValIdx = &(CertInstance.Buf[CertInstance.Offset]);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates available space in the destination buffer and writes a
 *		single byte to the buffer.
 *
 * @param	Value	The byte value to write
 *
 * @return
 *	- XASUFW_SUCCESS, if sufficient buffer space is available and byte is written.
 *	- XASUFW_FAILURE, if sufficient buffer space is not available in the destination buffer.
 *
 *************************************************************************************************/
static s32 X509_ValidateAndUpdateByteField(u8 Value)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate buffer space for single byte. */
	Status = X509_ValidateBufferSpace(X509_ONE_BYTE);
	if (Status == XASUFW_SUCCESS) {
		CertInstance.Buf[CertInstance.Offset++] = Value;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides the pointer to the X.509 initial data.
 *
 * @return
 *	- Pointer to structure which stores initialized data.
 *
 *************************************************************************************************/
static X509_InitData *X509_GetInitData(void)
{
	static X509_InitData InitData;

	return &InitData;
}

/*************************************************************************************************/
/**
 * @brief	This function takes a byte array as input and returns offset of a
 *		starting non-zero byte.
 *
 * @param	Data	Input byte array.
 * @param	Cnt	Number of bytes in a given array.
 *
 * @return
 *	- Offset of first non-zero byte in a given array.
 *
 *************************************************************************************************/
static u32 X509_Asn1GetFirstNonZeroByteOffset(const u8 *Data, u32 Cnt)
{
	u32 Offset = 0U;

	while (Offset < Cnt) {
		if (Data[Offset] != 0U) {
			break;
		}
		Offset++;
	}

	return Offset;
}

/*************************************************************************************************/
/**
 * @brief	This function extracts each byte from the integer value and create
 *		DER encoded ASN.1 Integer.
 *
 * @param	IntegerVal	Pointer to the Value of the ASN.1 Integer.
 * @param	IntegerLen	Length of the integer.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 integer.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Offset = 0U;
	u32 RemainBufSize = 0U;

	Offset = X509_Asn1GetFirstNonZeroByteOffset(IntegerVal, IntegerLen - 1U);

	/**
	 * If the most significant bit in the first byte of IntegerVal is set,
	 * then the value must be prepended with 0x00.
	 */
	if ((IntegerVal[Offset] & X509_ASN1_BIT7_MASK) == X509_ASN1_BIT7_MASK) {
		/** Validate buffer space and write ASN.1 DER integer sign byte prefix. */
		Status = X509_ValidateAndUpdateByteField(X509_ASN1_INTEGER_SIGN_BYTE_PREFIX);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
			goto END;
		}
	}

	RemainBufSize = CertInstance.BufSize - CertInstance.Offset;

	/** Copy integer values. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), RemainBufSize,
			     &IntegerVal[Offset], IntegerLen - Offset, IntegerLen - Offset);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}
	CertInstance.Offset += (IntegerLen - Offset);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Integer.
 *
 * @param	IntegerVal	Value of the ASN.1 Integer.
 * @param	IntegerLen	Length of the value of the ASN.1 Integer.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 Integer.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_INTEGER_FIELD_FROM_ARRAY_FAIL, if create integer field is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateInteger(const u8 *IntegerVal, u32 IntegerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *IntegerLenIdx = NULL;
	u8 *IntegerValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_INTEGER, &IntegerLenIdx,
						&IntegerValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateIntegerFieldFromByteArray(IntegerVal, IntegerLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
						  XASUFW_X509_CREATE_INTEGER_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*IntegerLenIdx = (u8)(End - IntegerValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes DER encoded data as input in form of byte array
 *		and updates it in the provided buffer.
 *
 * @param	RawData		DER encoded value as byte array to be updated in buffer.
 * @param	LenOfRawDataVal	Length of DER encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if update is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateRawDataFromByteArray(const u8 *RawData, const u32 LenOfRawDataVal)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 RemainBufSize = 0U;

	RemainBufSize = CertInstance.BufSize - CertInstance.Offset;

	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), RemainBufSize, RawData,
			     LenOfRawDataVal, LenOfRawDataVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	CertInstance.Offset += LenOfRawDataVal;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function takes a byte of data as input and returns number of
 *		trailing zeroes in that byte.
 *
 * @param	Data	Input byte for which number of trailing zeroes need to be counted.
 *
 * @return
 *	- Number of trailing zeroes. In case the Data is 0 then number of trailing zeroes is 8.
 *
 *************************************************************************************************/
static u8 X509_Asn1GetTrailingZeroesCount(u8 Data)
{
	u8 Count = 0U;
	u8 Value = Data;

	if (Value != 0x0U) {
		while ((Value & 0x1U) == 0x0U) {
			Value = Value >> 0x1U;
			Count++;
		}
	} else {
		Count = X509_ASN1_NO_OF_BITS_IN_BYTE;
	}

	return Count;
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 BitString.
 *
 * @param	BitStringVal	Value of the ASN.1 BitString.
 * @param	BitStringLen	Length of the value of the ASN.1 BitString in bytes.
 * @param	IsLastByteFull	Flag to check if the last byte is full or not.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 BitString.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 NumofTrailingZeroes = 0U;
	u8 *BitStringLenIdx = NULL;
	u8 *BitStringValIdx = NULL;
	const u8 *End = NULL;
	u32 RemainBufSize = 0U;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_BIT_STRING, &BitStringLenIdx,
						&BitStringValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/**
	 * The first byte of the value of the BITSTRING is used to show the number
	 * of unused bits in the last byte of the BITSTRING.
	 */
	if (IsLastByteFull == XASU_FALSE) {
		NumofTrailingZeroes = X509_Asn1GetTrailingZeroesCount(*(BitStringVal +
								      BitStringLen - 1U));
	}

	/** Validate buffer space and write number of trailing zeroes. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateByteField(NumofTrailingZeroes);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	RemainBufSize = CertInstance.BufSize - CertInstance.Offset;

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), RemainBufSize,
			     BitStringVal, BitStringLen, BitStringLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}
	CertInstance.Offset += BitStringLen;

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*BitStringLenIdx = (u8)(End - BitStringValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates a DER encoded ASN.1 TLV(Tag, Length, Value) field.
 *
 * @param	Asn1Tag		ASN.1 tag of the TLV pair.
 * @param	Val		Value of the TLV pair.
 * @param	ValLen		Length of the value of the TLV pair in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 TLV pair.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if the value length cannot be DER encoded.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateTlvField(u8 Asn1Tag, const u8 *Val, u32 ValLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *LenIdx;

	/** Validate that the length can be encoded in the supported number of length bytes. */
	if (ValLen > X509_ASN1_MAX_ENCODED_VAL_LEN) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	/** Add ASN.1 tag field. */
	Status = X509_ValidateAndAddTagField(Asn1Tag);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space for the length field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateBufferSpace(X509_ASN1_MAX_LEN_FIELD_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	LenIdx = &(CertInstance.Buf[CertInstance.Offset++]);

	/**
	 * Add the length field. Lengths up to 127 use the short form and longer lengths use the
	 * long form with one or two length bytes.
	 */
	if (ValLen <= X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*LenIdx = (u8)ValLen;
	} else if (ValLen <= X509_ASN1_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES) {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_1BYTE;
		*(LenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)ValLen;
		CertInstance.Offset++;
	} else {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_2BYTES;
		*(LenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)((ValLen & X509_ASN1_BYTE1_MASK) >>
							   X509_ASN1_NO_OF_BITS_IN_BYTE);
		*(LenIdx + XASUFW_BUFFER_INDEX_TWO) = (u8)(ValLen & X509_ASN1_BYTE0_MASK);
		CertInstance.Offset += XASUFW_VALUE_TWO;
	}

	/** Add the value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Val, ValLen);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 OctetString.
 *
 * @param	OctetStringVal	Value of the ASN.1 OctetString.
 * @param	OctetStringLen	Length of the value of the ASN.1 OctetString.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 OctetString.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static inline s32 X509_Asn1CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen)
{
	return X509_Asn1CreateTlvField(X509_ASN1_TAG_OCTET_STRING, OctetStringVal, OctetStringLen);
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 PrintableString.
 *
 * @param	Val	Value of the ASN.1 PrintableString.
 * @param	ValLen	Length of the value of the ASN.1 PrintableString in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 PrintableString.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static inline s32 X509_Asn1CreatePrintableString(const u8 *Val, u32 ValLen)
{
	return X509_Asn1CreateTlvField(X509_ASN1_TAG_PRINTABLE_STRING, Val, ValLen);
}

/*************************************************************************************************/
/**
 * @brief	This function creates a DER encoded RelativeDistinguishedName holding a single
 *		attribute type and value pair.
 *
 * @param	OidTlv		Pointer to the DER encoded TLV pair of the attribute type OID.
 * @param	OidTlvLen	Length of the DER encoded TLV pair of the attribute type OID.
 * @param	Val		Pointer to the printable string attribute value.
 * @param	ValLen		Length of the printable string attribute value.
 *
 * @return
 *	- XASUFW_SUCCESS, if the attribute is created successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_CREATE_PRINTABLE_STRING_FAIL, if printable string creation is failed.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if updating encoded length is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 * @note	RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue
 *		AttributeTypeAndValue ::= SEQUENCE {
 *			type	AttributeType,
 *			value	AttributeValue
 *		}
 *
 *		The attribute value is encoded as a PrintableString.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateNameAttribute(const u8 *OidTlv, u32 OidTlvLen, const u8 *Val, u32 ValLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *SetLenIdx = NULL;
	u8 *SetValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;

	/** Validate buffer space and update tag, length and value field of the SET. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SET, &SetLenIdx, &SetValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field of the SEQUENCE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add the attribute type OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(OidTlv, OidTlvLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Add the attribute value. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreatePrintableString(Val, ValLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_PRINTABLE_STRING_FAIL);
		goto END;
	}

	/** Update the length of the AttributeTypeAndValue SEQUENCE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx), SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	/** Update the length of the RelativeDistinguishedName SET. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SetLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SetValIdx), SetValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SetLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SetLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Boolean.
 *
 * @param	BooleanVal	Can be TRUE or FALSE.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 Boolean.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *
 * @note	ASN.1 tag for Boolean is 0x01.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateBoolean(const u32 BooleanVal)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate buffer space for tag, length and value fields. */
	Status = X509_ValidateBufferSpace(X509_TAG_LEN_VAL_MIN_SIZE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add ASN.1 tag field. */
	X509_AddTagField(X509_ASN1_TAG_BOOLEAN);

	/** Write boolean length. */
	CertInstance.Buf[CertInstance.Offset++] = X509_ASN1_LEN_OF_VALUE_OF_BOOLEAN;

	/** Write boolean value. */
	if (BooleanVal == XASU_TRUE) {
		CertInstance.Buf[CertInstance.Offset++] = (u8)X509_ASN1_BOOLEAN_TRUE;
	} else {
		CertInstance.Buf[CertInstance.Offset++] = (u8)X509_ASN1_BOOLEAN_FALSE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Version field of the TBS Certificate.
 *
 * @param	Version		Value of version.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created version field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	Version ::= INTEGER { v1(0), v2(1), v3(2) }
 *		This field describes the version of the encoded certificate.
 *		For Certificate Signing Request, the supported version is v1.
 *
 *************************************************************************************************/
static inline s32 X509_GenVersionField(u8 Version)
{
	return X509_Asn1CreateInteger(&Version, X509_VERSION_VAL_LEN);
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Signature Algorithm field. This field
 *		is present in TBS certificate as well as the X.509 certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Signature Algorithm field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_UNSUPPORTED_SIGN_TYPE, if signature type is not supported.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	AlgorithmIdentifier ::= SEQUENCE {
 *			algorithm		OBJECT IDENTIFIER,
 *			parameters		ANY DEFINED BY algorithm OPTIONAL
 *		}
 *
 *		This function supports only ECDSA with SHA3-384 as Signature Algorithm.
 *		The algorithm identifier for ECDSA with SHA3-384 signature values is:
 *		id-ecdsa-with-sha3-384 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16)
 *		us(840) organization(1) gov(101) csor(3) nistAlgorithm(4) sigAlgs(3) 11 }
 *		As per RFC 5758 section 3.2, the parameters field MUST be absent for the ECDSA
 *		signature algorithms and the SHA-3 based ECDSA algorithm identifiers follow the
 *		same convention. Encoding an explicit NULL makes the certificate non-conformant
 *		and strict DER parsers reject it.
 *
 *		Hence, the AlgorithmIdentifier is a SEQUENCE of one component: the OID
 *		id-ecdsa-with-sha3-384.
 *
 *************************************************************************************************/
static s32 X509_GenSignAlgoField(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const X509_InitData *InitData = X509_GetInitData();
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Check whether signature type is supported or not. */
	if (SignOidList[InitData->SignType].SignType != X509_SIGN_TYPE_ECC_SHA3_384) {
		Status = XASUFW_X509_UNSUPPORTED_SIGN_TYPE;
		goto END;
	}

	/** Copy signature type OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(SignOidList[InitData->SignType].SignOid,
			(sizeof(SignOidList[InitData->SignType].SignOid[0]) *
			 SignOidList[InitData->SignType].SignLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*SequenceLenIdx = (u8)(End - SequenceValIdx);

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Issuer field in TBS Certificate.
 *
 * @param	Issuer		DER encoded value of the Issuer field.
 * @param	IssuerValLen	Length of the DER encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Issuer field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static inline s32 X509_GenIssuerField(const u8 *Issuer, const u32 IssuerValLen)
{
	return X509_Asn1CreateRawDataFromByteArray(Issuer, IssuerValLen);
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Validity field in TBS Certificate.
 *
 * @param	Validity	DER encoded value of the Validity field.
 * @param	ValidityValLen	Length of the DER encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Validity field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static inline s32 X509_GenValidityField(const u8 *Validity, const u32 ValidityValLen)
{
	return X509_Asn1CreateRawDataFromByteArray(Validity, ValidityValLen);
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Subject field in TBS Certificate.
 *
 * @param	Subject		DER encoded value of the Subject field.
 * @param	SubjectValLen	Length of the DER encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Subject field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static inline s32 X509_GenSubjectField(const u8 *Subject, const u32 SubjectValLen)
{
	return X509_Asn1CreateRawDataFromByteArray(Subject, SubjectValLen);
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Public Key Algorithm Identifier sub-field. It
 *		is a part of Subject Public Key Info field present in
 *		TBS Certificate.
 *
 * @param	PubKeyInfo	Pointer to structure containing public-key information.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Public Key Algorithm Identifier sub-field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	AlgorithmIdentifier ::= SEQUENCE {
 *			algorithm		OBJECT IDENTIFIER,
 *			parameters		ANY DEFINED BY algorithm OPTIONAL
 *		}
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
 *************************************************************************************************/
static s32 X509_GenPubKeyAlgIdentifierField(const X509_SubjectPublicKeyInfo *PubKeyInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;
	u8 EccCurveType = (u8)PubKeyInfo->EccCurveType;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add ASN.1 tag and OID length fields. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndAddTagByteFields(X509_ASN1_TAG_OID,
						  PubKeyOidList[PubKeyInfo->PubKeyType].OidLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Copy public key type OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(PubKeyOidList[PubKeyInfo->PubKeyType].Oid,
			(sizeof(PubKeyOidList[PubKeyInfo->PubKeyType].Oid[0]) *
				PubKeyOidList[PubKeyInfo->PubKeyType].OidLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Copy ECC curve type OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(ParamOidList[EccCurveType].ParamOid,
				(sizeof(ParamOidList[EccCurveType].ParamOid[0]) *
				ParamOidList[EccCurveType].ParamOidLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Public Key Info field present in
 *		TBS Certificate.
 *
 * @param	PubKeyInfo	Pointer to structure containing public-key information.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Public Key Info field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_GENERATE_PUB_KEY_ALGO_FIELD_FAIL, if public key algorithm generation is
 *	  failed.
 *	- XASUFW_X509_CREATE_BIT_STRING_FAIL, if bit string creation is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	SubjectPublicKeyInfo ::= SEQUENCE {
 *			algorithm	 AlgorithmIdentifier,
 *			subjectPublicKey BIT STRING
 *		}
 *
 *************************************************************************************************/
static s32 X509_GenPublicKeyInfoField(const X509_SubjectPublicKeyInfo *PubKeyInfo)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Generate public key algorithm identifier field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenPubKeyAlgIdentifierField(PubKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
						  XASUFW_X509_GENERATE_PUB_KEY_ALGO_FIELD_FAIL);
		goto END;
	}

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */
	UncompressedPublicKey[0U] = X509_UNCOMPRESSED_PUB_KEY;
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
			     PubKeyInfo->SubjectPublicKey, PubKeyInfo->SubjectPubKeyLen,
			     PubKeyInfo->SubjectPubKeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Create bit string for public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBitString(UncompressedPublicKey,
					  X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
					  XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_BIT_STRING_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Subject Key Identifier field present in
 *		TBS Certificate.
 *
 * @param	SubjectPublicKey	Public key whose hash will be used as Subject Key
 *		Identifier.
 * @param	SubjectPubKeyLen	Public key length.
 * @param	PlatformData		Pointer to platform related data.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Subject Key Identifier field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_CREATE_OCTET_STRING_FAIL, if octet string creation is failed.
 *	- XASUFW_X509_GENERATE_DIGEST_FAIL, if generate digest is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	SubjectKeyIdentifierExtension ::= SEQUENCE {
 *			extnID		OBJECT IDENTIFIER,
 *			extnValue	OCTET STRING
 *		}
 *		To calculate value of SubjectKeyIdentifier field, hash is
 *		calculated on the Subject Public Key and 20 bytes from LSB of
 *		the hash is considered as the value for this field.
 *
 *************************************************************************************************/
static s32 X509_GenSubjectKeyIdentifierField(const u8 *SubjectPublicKey, u32 SubjectPubKeyLen,
					     const void *PlatformData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 Oid_SubKeyIdentifier[]	= {0x06U, 0x03U, 0x55U, 0x1DU, 0x0EU};
	const X509_InitData *InitData = X509_GetInitData();
	u32 HashLen = 0U;
	const u8 Hash[X509_HASH_MAX_SIZE_IN_BYTES] = {0U};
	u8 *OctetStrLenIdx = NULL;
	u8 *OctetStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End = NULL;

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */
	UncompressedPublicKey[0U] = X509_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
			     SubjectPublicKey, SubjectPubKeyLen, SubjectPubKeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for subject key OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_SubKeyIdentifier,
						     sizeof(Oid_SubKeyIdentifier));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Calculate digest of subject public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateDigest(UncompressedPublicKey,
					  X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, Hash,
					  (u32)X509_HASH_MAX_SIZE_IN_BYTES, &HashLen,
					  PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GENERATE_DIGEST_FAIL);
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OCTET_STRING, &OctetStrLenIdx,
						&OctetStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create octet string field to store calculated hash. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateOctetString(Hash, X509_SUB_KEY_ID_VAL_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_OCTET_STRING_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*OctetStrLenIdx = (u8)(End - OctetStrValIdx);
	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Authority Key Identifier field present in
 *		TBS Certificate.
 *
 * @param	IssuerPublicKey		Public key whose hash will be used as Authority Key
 *					Identifier.
 * @param	IssuerPubKeyLen		Issuer public key length.
 * @param	PlatformData		Pointer to platform related data.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Authority Key Identifier field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_GENERATE_DIGEST_FAIL, if generate digest is failed.
 *	- XASUFW_X509_CREATE_OCTET_STRING_FAIL, if octet string creation is failed.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.

 * @note
 *		id-ce-authorityKeyIdentifier OBJECT IDENTIFIER ::= { id-ce 35 }
 *
 *		AuthorityKeyIdentifier ::= SEQUENCE {
 *			keyIdentifier			[0] KeyIdentifier	OPTIONAL,
 *			authorityCertIssuer		[1] GeneralNames	OPTIONAL,
 *			authorityCertSerialNumber	[2] CertificateSerialNumber	OPTIONAL
 *		}
 *
 *		KeyIdentifier ::= OCTET STRING
 *		To calculate value ofAuthorityKeyIdentifier field, hash is
 *		calculated on the Issuer Public Key and 20 bytes from LSB of
 *		the hash is considered as the value for this field.
 *
 *************************************************************************************************/
static s32 X509_GenAuthorityKeyIdentifierField(const u8 *IssuerPublicKey, u32 IssuerPubKeyLen,
					       const void *PlatformData)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 Oid_AuthKeyIdentifier[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x23U};
	const X509_InitData *InitData = X509_GetInitData();
	u32 HashLen = 0U;
	const u8 Hash[X509_HASH_MAX_SIZE_IN_BYTES] = {0U};
	u8 *KeyIdSequenceLenIdx = NULL;
	u8 *KeyIdSequenceValIdx = NULL;
	u8 *OctetStrLenIdx = NULL;
	u8 *OctetStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End = NULL;

	/**
	 * First byte of the Public key should be 0x04 to indicate that it is
	 * an uncompressed public key.
	 */
	UncompressedPublicKey[0U] = X509_UNCOMPRESSED_PUB_KEY;
	Status = Xil_SMemCpy(&UncompressedPublicKey[1U], X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN,
			     IssuerPublicKey, IssuerPubKeyLen, IssuerPubKeyLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for authority key OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_AuthKeyIdentifier,
						     sizeof(Oid_AuthKeyIdentifier));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Calculate digest of issuer public key. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateDigest(UncompressedPublicKey,
					  X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN, Hash,
					  (u32)X509_HASH_MAX_SIZE_IN_BYTES,
					  &HashLen, PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GENERATE_DIGEST_FAIL);
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OCTET_STRING, &OctetStrLenIdx,
						&OctetStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &KeyIdSequenceLenIdx,
						&KeyIdSequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create octet string field to store calculated hash. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateOctetString(Hash, X509_AUTH_KEY_ID_VAL_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_OCTET_STRING_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	/**
	 * 0x80 indicates that the SEQUENCE contains the optional parameter tagged
	 * as [0] in the AuthorityKeyIdentifier sequence.
	 */
	*KeyIdSequenceValIdx = X509_ASN1_TAG_CONTEXT_SPECIFIC;

	*KeyIdSequenceLenIdx = (u8)(End - KeyIdSequenceValIdx);
	*OctetStrLenIdx = (u8)(End - OctetStrValIdx);
	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the value of Key Usage extension field
 *		present in TBS Certificate as per the key usage bit position.
 *
 * @param	KeyUsageVal	Pointer in the Key Usage value buffer where the Key Usage bit
 *				shall be updated.
 * @param	KeyUsageBitPos	Position of the key usage bit within the BIT STRING.
 *
 *************************************************************************************************/
static void X509_UpdateKeyUsageVal(u8 *KeyUsageVal, u8 KeyUsageBitPos)
{
	u8 Idx = (KeyUsageBitPos / X509_ASN1_NO_OF_BITS_IN_BYTE);
	u8 ShiftVal = (X509_ASN1_NO_OF_BITS_IN_BYTE * (Idx + XASUFW_VALUE_ONE)) -
		      KeyUsageBitPos - XASUFW_VALUE_ONE;

	KeyUsageVal[Idx] |= 1U << ShiftVal;
}

/*************************************************************************************************/
/**
 * @brief	This function updates the value of the Key Usage extension field present in TBS
 *		Certificate from a mask of key usage options.
 *
 * @param	KeyUsageVal	Pointer in the Key Usage value buffer where the Key Usage options
 *				shall be updated.
 * @param	KeyUsageMask	Bit wise OR of the XASU_X509_KEY_USAGE_* masks.
 *
 *************************************************************************************************/
static inline void X509_UpdateKeyUsageValFromMask(u8 *KeyUsageVal, u32 KeyUsageMask)
{
	u32 Mask = XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE;
	u8 KeyUsageBitPos = 0U;

	/** Update the Key Usage value buffer with the Key Usage options. */
	while (Mask <= XASU_X509_KEY_USAGE_MAX_MASK) {
		if ((KeyUsageMask & Mask) != 0U) {
			X509_UpdateKeyUsageVal(KeyUsageVal, KeyUsageBitPos);
		}
		Mask <<= 1U;
		KeyUsageBitPos++;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Key Usage extension field present in
 *		TBS Certificate.
 *
 * @param	Cfg	Pointer to structure which includes configuration for the TBS Certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Key Usage field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_CREATE_BIT_STRING_FAIL, if bit string creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	id-ce-keyUsage OBJECT IDENTIFIER ::= { id-ce 15 }
 *		KeyUsage ::= BIT STRING {
 *			digitalSignature	(0),
 *			nonRepudiation		(1),
 *			keyEncipherment		(2),
 *			dataEncipherment	(3),
 *			keyAgreement		(4),
 *			keyCertSign		(5),
 *			cRLSign			(6),
 *			encipherOnly		(7),
 *			decipherOnly		(8)
 *		}
 *
**************************************************************************************************/
static s32 X509_GenKeyUsageField(const X509_Config *Cfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 Oid_KeyUsageExtn[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x0FU};
	u32 KeyUsageValLen = 0U;
	u8 KeyUsageVal[X509_KEYUSAGE_VAL_LEN] = {0U};
	u8 *OctetStrLenIdx = NULL;
	u8 *OctetStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for key usage extension OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_KeyUsageExtn,
						     sizeof(Oid_KeyUsageExtn));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBoolean(XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_BOOLEAN_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OCTET_STRING, &OctetStrLenIdx,
						&OctetStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	X509_UpdateKeyUsageValFromMask(KeyUsageVal, Cfg->KeyUsage);

	if ((KeyUsageVal[XASUFW_BUFFER_INDEX_ONE] & X509_ASN1_BYTE0_MASK) == 0U) {
		KeyUsageValLen = X509_KEYUSAGE_VAL_LEN - XASUFW_VALUE_ONE;
	} else {
		KeyUsageValLen = X509_KEYUSAGE_VAL_LEN;
	}

	/** Create BIT string field to store key usage value. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBitString(KeyUsageVal, KeyUsageValLen, XASU_FALSE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_BIT_STRING_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*OctetStrLenIdx = (u8)(End - OctetStrValIdx);
	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Extended Key Usage extension field
 *		present in TBS Certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Extended Key Usage field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 }
 *		ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
 *		KeyPurposeId ::= OBJECT IDENTIFIER
 *
 *************************************************************************************************/
static s32 X509_GenExtKeyUsageField(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 Oid_EkuExtn[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x25U};
	const u8 Oid_EkuClientAuth[] = {0x06U, 0x08U, 0x2BU, 0x06U, 0x01U, 0x05U, 0x05U, 0x07U,
					0x03U, 0x02U};
	u8 *EkuSequenceLenIdx = NULL;
	u8 *EkuSequenceValIdx = NULL;
	u8 *OctetStrLenIdx = NULL;
	u8 *OctetStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for extended key usage extension OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_EkuExtn, sizeof(Oid_EkuExtn));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBoolean(XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_BOOLEAN_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OCTET_STRING, &OctetStrLenIdx,
						&OctetStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &EkuSequenceLenIdx,
						&EkuSequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for extended key usage client authority OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_EkuClientAuth, sizeof(Oid_EkuClientAuth));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*EkuSequenceLenIdx = (u8)(End - EkuSequenceValIdx);
	*OctetStrLenIdx = (u8)(End - OctetStrValIdx);
	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the basic constraints extension field present in
 *		Certificate Request Info.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated basic constraints field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_CREATE_BOOLEAN_FAIL, if boolean creation is failed.
 *	- XASUFW_X509_CREATE_INTEGER_FIELD_FAIL, if integer field creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	id-ce-basicConstraints OBJECT IDENTIFIER ::= { id-ce 19 }
 *		BasicConstraints ::= SEQUENCE {
 *			CA			BOOLEAN DEFAULT FALSE,
 *			pathLenConstraint	INTEGER (0..MAX) OPTIONAL
 *		}
 *
 *************************************************************************************************/
static s32 X509_GenBasicConstraintsField(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 PathLenConstraint = X509_PATH_LEN_CONSTRAINT_VALUE;
	const u8 Oid_BasicConstraintsExtn[] = {0x06U, 0x03U, 0x55U, 0x1DU, 0x13U};
	u8 *BasicConstraintsSequenceLenIdx = NULL;
	u8 *BasicConstraintsSequenceValIdx = NULL;
	u8 *OctetStrLenIdx = NULL;
	u8 *OctetStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for basic constraints extension OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_BasicConstraintsExtn,
						     sizeof(Oid_BasicConstraintsExtn));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Mark the extension as critical. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBoolean(XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_BOOLEAN_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OCTET_STRING, &OctetStrLenIdx,
						&OctetStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE,
						&BasicConstraintsSequenceLenIdx,
						&BasicConstraintsSequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Assert the CA boolean to indicate that the subject is a Certification Authority. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateBoolean(XASU_TRUE);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_BOOLEAN_FAIL;
		goto END;
	}

	/** Create integer field to store the path length constraint. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger(&PathLenConstraint, X509_SINGLE_BYTE);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_INTEGER_FIELD_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*BasicConstraintsSequenceLenIdx = (u8)(End - BasicConstraintsSequenceValIdx);
	*OctetStrLenIdx = (u8)(End - OctetStrValIdx);
	*SequenceLenIdx = (u8)(End - SequenceValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Subject Alternative Name extension field in TBS
 *		Certificate.
 *
 * @param	SubAltName		DER encoded value of the Subject Alternative Name extension
 *		field.
 * @param	SubAltNameValLen	Length of the DER encoded value.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Subject Alternative Name extension field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static inline s32 X509_GenSubAltNameField(const u8 *SubAltName, const u32 SubAltNameValLen)
{
	return X509_Asn1CreateRawDataFromByteArray(SubAltName, SubAltNameValLen);
}

/*************************************************************************************************/
/**
 * @brief	This function adds ASN.1 tag field in X.509 Certificate.
 *
 * @param	Asn1Tag		ASN.1 tag to be added.
 *
 *************************************************************************************************/
static inline void X509_AddTagField(u8 Asn1Tag)
{
	CertInstance.Buf[CertInstance.Offset++] = Asn1Tag;
}

/*************************************************************************************************/
/**
 * @brief	This function validates buffer space and adds ASN.1 tag field in the destination
 *		buffer.
 *
 * @param	Asn1Tag		ASN.1 tag to be added.
 *
 * @return
 *	- XASUFW_SUCCESS, if tag field is added successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static inline s32 X509_ValidateAndAddTagField(u8 Asn1Tag)
{
	return X509_ValidateAndUpdateByteField(Asn1Tag);
}

/*************************************************************************************************/
/**
 * @brief	This function validates buffer space and adds ASN.1 tag and byte fields in the
 *		destination buffer.
 *
 * @param	Asn1Tag		ASN.1 tag to be added.
 * @param	ByteValue	Byte value to be added after the tag.
 *
 * @return
 *	- XASUFW_SUCCESS, if tag and byte fields are added successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 X509_ValidateAndAddTagByteFields(u8 Asn1Tag, u8 ByteValue)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate buffer space for tag and byte fields. */
	Status = X509_ValidateBufferSpace(X509_TAG_BYTE_FIELD_SIZE);
	if (Status == XASUFW_SUCCESS) {
		/** Add tag and byte fields. */
		X509_AddTagField(Asn1Tag);
		CertInstance.Buf[CertInstance.Offset++] = ByteValue;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 *		TBS Certificate.
 *
 * @param	Cfg	structure which includes configuration for the TBS Certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated X.509 v3 Extensions field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if updating encoded length is failed.
 *	- XASUFW_X509_GEN_SUB_KEY_IDENTIFIER_FIELD_FAIL, if subject key identifier field generation
 *	  is failed.
 *	- XASUFW_X509_GEN_AUTH_KEY_IDENTIFIER_FIELD_FAIL, if authority key identifier field
 *	  generation is failed.
 *	- XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL, if key usage field generation is failed.
 *	- XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL, if extended key usage field generation
 *	  is failed.
 *	- XASUFW_X509_GEN_SUB_ALT_NAME_FIELD_FAIL, if subject alternate name field generation
 *	  is failed.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	Extensions ::= SEQUENCE SIZE (1..MAX) OF Extension
 *		Extension ::= SEQUENCE {
 *			extnID		OBJECT IDENTIFIER,
 *			critical	BOOLEAN DEFAULT FALSE,
 *			extnValue	OCTET STRING
 *						-- contains the DER encoding of an ASN.1 value
 *						-- corresponding to the extension type identified
 *						-- by extnID
 *		}
 *
 *		This field is a SEQUENCE of the different extensions which should
 *		be part of the Version 3 of the X.509 certificate.
 *
 *************************************************************************************************/
static s32 X509_GenX509v3ExtensionsField(const X509_Config *Cfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *OptionalTagLenIdx = NULL;
	u8 *OptionalTagValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OPTIONAL_PARAM_3_CONSTRUCTED_TAG,
						&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Generate subject key identifier field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSubjectKeyIdentifierField(Cfg->PubKeyInfo.SubjectPublicKey,
						   Cfg->PubKeyInfo.SubjectPubKeyLen,
						   Cfg->PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_GEN_SUB_KEY_IDENTIFIER_FIELD_FAIL);
		goto END;
	}

	/** Generate authority key identifier field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenAuthorityKeyIdentifierField(Cfg->IssuerPublicKey,
			Cfg->IssuerPubKeyLen, Cfg->PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_GEN_AUTH_KEY_IDENTIFIER_FIELD_FAIL);
		goto END;
	}

	/**
	 * Generate the basic constraints field for a CA certificate (CA=TRUE, pathLenConstraint=0).
	 */
	if (Cfg->IsCaCert == XASU_TRUE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_GenBasicConstraintsField();
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status,
					XASUFW_X509_GEN_BASIC_CONSTRAINTS_FIELD_FAIL);
			goto END;
		}
	}

	/** Generate key usage field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenKeyUsageField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL;
		goto END;
	}

	/**
	 * Generate the extended key usage field for a self-signed identity certificate. A CA
	 * certificate (e.g. the UDE self-signed CA certificate) does not carry the clientAuth
	 * extended key usage, so it is suppressed in that case.
	 */
	if ((Cfg->IsSelfSigned == XASU_TRUE) && (Cfg->IsCaCert != XASU_TRUE)) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_GenExtKeyUsageField();
		if (Status != XASUFW_SUCCESS) {
			Status = (s32)XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL;
			goto END;
		}
	}

	if (Cfg->UserCfg->IsSubAltNameAvailable == XASU_TRUE) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = X509_GenSubAltNameField(Cfg->UserCfg->SubAltName,
			Cfg->UserCfg->SubAltNameLen);
		if (Status != XASUFW_SUCCESS) {
			Status = (s32)XASUFW_X509_GEN_SUB_ALT_NAME_FIELD_FAIL;
			goto END;
		}
	}

	/** Update the length of each extensions field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(OptionalTagLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						OptionalTagValIdx),
					  OptionalTagValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*OptionalTagLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*OptionalTagLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Serial field of the TBS certificate.
 *
 * @param	DataHash	Hash which is to be used as value in Serial field.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Serial field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- XASUFW_X509_INVALID_FIELD_LEN, if the reserved slot size does not match the expected
 *	  encoding, or if the encoded serial does not fill X509_SERIAL_FIELD_LEN. It is packed
 *	  on top of the encoder status, so an integer-creation failure is reported together
 *	  with it.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	CertificateSerialNumber ::= INTEGER
 *		The length of the serial must not be more than X509_SERIAL_VAL_LEN bytes.
 *		The value of the serial is determined by calculating the
 *		hash of the fields in the TBS certificate except the Version
 *		and Serial Number fields. The first X509_SERIAL_VAL_LEN bytes of the
 *		calculated hash are used as the Serial Number.
 *		To keep the value a fixed X509_SERIAL_VAL_LEN bytes so the DER-encoded
 *		INTEGER field is exactly X509_SERIAL_FIELD_LEN bytes (one tag byte, one
 *		length byte and X509_SERIAL_VAL_LEN value bytes) and matches the reserved
 *		slot, the MSB of the first byte is cleared to prevent a
 *		0x00 prepend, and if that first byte becomes zero it is replaced
 *		with a non-zero, MSB-clear byte taken from the remaining hash so
 *		it is not stripped during encoding.
 *
 *************************************************************************************************/
static s32 X509_GenSerialField(const u8 *DataHash)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 Serial[X509_SERIAL_VAL_LEN] = {0U};
	u32 Offset;
	u32 Index;

	/** Temporary move offset to serial number to store serial number. */
	Offset = CertInstance.Offset;
	CertInstance.Offset = CertInstance.SerialOffset;

	/**
	 * The serial is derived from the certificate hash, and X.509 caps it at
	 * X509_SERIAL_VAL_LEN bytes, so the leading bytes of the digest are taken as its
	 * value. The first byte is conditioned below so the value always DER-encodes to a
	 * fixed width that fills the reserved slot.
	 */
	Status = Xil_SMemCpy(Serial, X509_SERIAL_VAL_LEN, DataHash,
			     X509_SERIAL_VAL_LEN, X509_SERIAL_VAL_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	/** Clear the MSB of the first byte to avoid a 0x00 prepend during DER encoding. */
	Serial[X509_SERIAL_FIRST_BYTE_INDEX] = Serial[X509_SERIAL_FIRST_BYTE_INDEX] &
					       (u8)(~X509_ASN1_BIT7_MASK);

	/**
	 * A zero first byte would be stripped during DER encoding, shortening the serial.
	 * Replace it with the first non-zero, MSB-clear byte from the rest of the digest so
	 * the value stays hash-derived and encodes to the full X509_SERIAL_FIELD_LEN.
	 */
	if (Serial[X509_SERIAL_FIRST_BYTE_INDEX] == 0U) {
		for (Index = X509_SERIAL_VAL_LEN; Index < X509_HASH_MAX_SIZE_IN_BYTES;
		     Index++) {
			if ((DataHash[Index] != 0U) &&
			    ((DataHash[Index] & X509_ASN1_BIT7_MASK) == 0U)) {
				Serial[X509_SERIAL_FIRST_BYTE_INDEX] = DataHash[Index];
				break;
			}
		}
		/**
		 * If the loop above found no usable byte (every remaining digest byte
		 * was either zero or had its MSB set), Serial[0] is still zero. This is
		 * extremely unlikely for the configured digest, but to be safe a fixed value
		 * (X509_SERIAL_FIRST_BYTE_FALLBACK) that is non-zero and has its MSB clear is
		 * used, so the serial value stays X509_SERIAL_VAL_LEN bytes and always encodes
		 * to exactly X509_SERIAL_FIELD_LEN bytes.
		 */
		if (Serial[X509_SERIAL_FIRST_BYTE_INDEX] == 0U) {
			Serial[X509_SERIAL_FIRST_BYTE_INDEX] = X509_SERIAL_FIRST_BYTE_FALLBACK;
		}
	}

	/**
	 * Verify the reserved slot equals the header plus value size the encoder will
	 * produce, before anything is written, so a mismatch cannot overrun into the
	 * following signatureAlgorithm field.
	 */
	if (X509_SERIAL_FIELD_LEN != (X509_TAG_BYTE_FIELD_SIZE + X509_SERIAL_VAL_LEN)) {
		Status = XASUFW_X509_INVALID_FIELD_LEN;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger(Serial, X509_SERIAL_VAL_LEN);

	/**
	 * Confirm the serial exactly filled its reserved slot. On mismatch,
	 * XASUFW_X509_INVALID_FIELD_LEN is packed on top of the current status so a failure
	 * is reported without discarding any error already set by the encoder.
	 */
	if ((CertInstance.Offset - CertInstance.SerialOffset) != X509_SERIAL_FIELD_LEN) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_INVALID_FIELD_LEN);
	}

END:
	/** Restore offset. */
	CertInstance.Offset = Offset;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the TBS(To Be Signed) certificate.
 *
 * @param	Cfg		Structure which includes configuration for the TBS certificate.
 * @param	TBSCertLen	Length of the TBS certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated TBS certificate.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL, error in generating Public Key Info
 *	  field.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, error in updating encoded length.
 *	- XASUFW_X509_GEN_ISSUER_FIELD_FAIL, if Issuer field generation is failed.
 *	- XASUFW_X509_GEN_VERSION_FIELD_FAIL, if Version field generation is failed.
 *	- XASUFW_X509_GEN_VALIDITY_FIELD_FAIL, if Validity field generation is failed.
 *	- XASUFW_X509_GEN_SUBJECT_FIELD_FAIL, if Subject field generation is failed.
 *	- XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL, if Public Key Info field generation is failed.
 *	- XASUFW_X509_GEN_SIGN_ALGO_FIELD_FAIL, if Signature Algorithm field generation is failed.
 *	- XASUFW_X509_GEN_SERIAL_FIELD_FAIL, if Signature field generation is failed.
 *	- XASUFW_X509_GENERATE_DIGEST_FAIL, if digest calculation is failed.
 *	- XASUFW_X509_GEN_EXTENSION_FIELD_FAIL, if Extensions field generation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	TBSCertificate ::= SEQUENCE {
 *			version			[0] EXPLICIT Version (v3),
 *			serialNumber		CertificateSerialNumber,
 *			signature		AlgorithmIdentifier,
 *			issuer			Name,
 *			validity		Validity,
 *			subject			Name,
 *			subjectPublicKeyInfo	SubjectPublicKeyInfo,
 *			extensions		[3] EXPLICIT Extensions
 *		}
 *
 *************************************************************************************************/
static s32 X509_GenTBSCertificate(const X509_Config *Cfg, u32 *TBSCertLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const X509_InitData *InitData = X509_GetInitData();
	u32 HashLen = 0U;
	const u8 Hash[X509_HASH_MAX_SIZE_IN_BYTES] = {0U};
	u32 TbsStart = CertInstance.Offset;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *HashStartIdx = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add ASN.1 tag and version field length. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndAddTagByteFields(X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG,
						  X509_VERSION_FIELD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/**
	 * Generate Version field.
	 * As per RFC 5280 (Internet X.509 Public Key Infrastructure Certificate and
	 * Certificate Revocation List (CRL) Profile), when extensions are used, version
	 * MUST be v3 (value is 2).
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenVersionField((u8)X509_VERSION_VALUE_V3);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VERSION_FIELD_FAIL);
		goto END;
	}

	/** Validate buffer space for serial field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateBufferSpace(X509_SERIAL_FIELD_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Store the serial offset to update it later and skip the serial field for now. */
	CertInstance.SerialOffset = CertInstance.Offset;
	CertInstance.Offset += X509_SERIAL_FIELD_LEN;
	HashStartIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate Signature Algorithm field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSignAlgoField();
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SIGN_ALGO_FIELD_FAIL);
		goto END;
	}

	/** Generate Issuer field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenIssuerField(Cfg->UserCfg->Issuer, Cfg->UserCfg->IssuerLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_GEN_ISSUER_FIELD_FAIL;
		goto END;
	}

	/** Generate Validity field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenValidityField(Cfg->UserCfg->Validity, Cfg->UserCfg->ValidityLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_GEN_VALIDITY_FIELD_FAIL;
		goto END;
	}

	/** Generate Subject field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSubjectField(Cfg->UserCfg->Subject, Cfg->UserCfg->SubjectLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_GEN_SUBJECT_FIELD_FAIL;
		goto END;
	}

	/** Generate Public Key Info field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenPublicKeyInfoField(&Cfg->PubKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL);
		goto END;
	}

	/** Generate X.509 v3 extensions field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenX509v3ExtensionsField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_EXTENSION_FIELD_FAIL);
		goto END;
	}

	/**
	 * Calculate hash for all fields in the TBS certificate except Version and Serial.
	 * The hash is calculated starting from the field after the Serial field.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateDigest(HashStartIdx,
					  (&(CertInstance.Buf[CertInstance.Offset]) -
					   HashStartIdx),
					  Hash, (u32)X509_HASH_MAX_SIZE_IN_BYTES, &HashLen,
					  Cfg->PlatformData);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GENERATE_DIGEST_FAIL);
		goto END;
	}

	/** Generate Serial field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSerialField(Hash);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SERIAL_FIELD_FAIL);
		goto END;
	}

	/** Update the encoded length in the TBS certificate SEQUENCE. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	*TBSCertLen = CertInstance.Offset - TbsStart;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Signature field in the X.509 certificate.
 *
 * @param	Signature	Value of the Signature field in X.509 certificate.
 * @param	SignLen		Length of the Signature field.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Sign field.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_INTEGER_FIELD_FAIL, if integer field creation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	Th signature value is encoded as a BIT STRING and included in the
 *		signature field. When signing, the ECDSA algorithm generates
 *		two values - r and s.
 *		To easily transfer these two values as one signature,
 *		they MUST be DER encoded using the following ASN.1 structure:
 *		Ecdsa-Sig-Value ::= SEQUENCE {
 *			r	INTEGER,
 *			s	INTEGER
 *		}
 *
 *************************************************************************************************/
static s32 X509_GenSignField(const u8 *Signature, u32 SignLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *BitStrLenIdx = NULL;
	u8 *BitStrValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	const u8 *End = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_BIT_STRING, &BitStrLenIdx,
						&BitStrValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and write zero unused bits. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateByteField(X509_ASN1_ZERO_UNUSED_BITS);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger(Signature, (SignLen / X509_ECDSA_SIG_R_S_SIZE_DIVISOR));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_INTEGER_FIELD_FAIL);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger((Signature + (SignLen / X509_ECDSA_SIG_R_S_SIZE_DIVISOR)),
					(SignLen / X509_ECDSA_SIG_R_S_SIZE_DIVISOR));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_INTEGER_FIELD_FAIL);
		goto END;
	}

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*SequenceLenIdx = (u8)(End - SequenceValIdx);
	*BitStrLenIdx = (u8)(End - BitStrValIdx);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the X.509 v3 extensions field present in
 *		Certificate request Info.
 *
 * @param	Cfg	Structure which includes configuration for the Certificate Request
 *		Info.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully generated Extensions field for Certification Request Info.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL, if key usage field generation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_GEN_BASIC_CONSTRAINTS_FIELD_FAIL, if basic constraints field generation is
 *	  failed.
 *	- XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL, if extended key usage field generation is
 *	  failed.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if update encoded length is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GenCsrExtensions(const X509_Config *Cfg)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const u8 Oid_ExtnRequest[] = {0x06U, 0x09U, 0x2AU, 0x86U, 0x48U, 0x86U, 0xF7U,
				      0x0DU, 0x01U, 0x09U, 0x0EU};
	u8 *ExtnReqSeqLenIdx = NULL;
	u8 *ExtnReqSeqValIdx = NULL;
	u8 *OptionalTagLenIdx = NULL;
	u8 *OptionalTagValIdx = NULL;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u8 *SetLenIdx = NULL;
	u8 *SetValIdx = NULL;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG,
						&OptionalTagLenIdx, &OptionalTagValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &ExtnReqSeqLenIdx,
						&ExtnReqSeqValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Create field for extension request OID in case of CSR. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_ExtnRequest, sizeof(Oid_ExtnRequest));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SET, &SetLenIdx, &SetValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Validate buffer space and update tag, length and value field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Generate key usage field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenKeyUsageField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL;
		goto END;
	}

	/** Generate basic constraints field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenBasicConstraintsField();
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_BASIC_CONSTRAINTS_FIELD_FAIL;
		goto END;
	}

	/** Generate extended key usage field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenExtKeyUsageField();
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL;
		goto END;
	}

	/** Update the length of each CSR extensions field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SetLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SetValIdx),
					  SetValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SetLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SetLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(ExtnReqSeqLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						ExtnReqSeqValIdx),
					  ExtnReqSeqValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*ExtnReqSeqLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*ExtnReqSeqLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(OptionalTagLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						OptionalTagValIdx),
					  OptionalTagValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*OptionalTagLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*OptionalTagLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Certification Request Info in CSR format.
 *
 * @param	Cfg		Structure which includes configuration for the Certification
 *				Request	Info.
 * @param	CsrLen		Length of the CSR info.
 *
 * @return
 *	- XASUFW_SUCCESS, if Certificate request info created successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL, error in generating Public Key Info
 *	  field.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, error in updating encoded length.
 *	- XASUFW_X509_GEN_VERSION_FIELD_FAIL, if Version field generation is failed.
 *	- XASUFW_X509_GEN_SUBJECT_FIELD_FAIL, if Subject field generation is failed.
 *	- XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL, if Public Key Information field generation
 *	  is failed.
 *	- XASUFW_X509_GEN_EXTENSION_FIELD_FAIL, if Extension field generation is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 *************************************************************************************************/
static s32 X509_GenCertReqInfo(const X509_Config *Cfg, u32 *CsrLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;
	u32 CsrStart = CertInstance.Offset;

	/** Validate buffer space and update tag, length and value field. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/**
	 * Generate Version field.
	 * As per RFC 2986 (PKCS #10: Certification Request Syntax Specification Version 1.7),
	 * the version shall be v1 (value is 0) for Certificate Signing Requests.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenVersionField((u8)X509_VERSION_VALUE_V1);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VERSION_FIELD_FAIL);
		goto END;
	}

	/** Generate Subject field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenSubjectField(Cfg->UserCfg->Subject, Cfg->UserCfg->SubjectLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_SUBJECT_FIELD_FAIL);
		goto END;
	}

	/** Generate Public Key Info field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenPublicKeyInfoField(&Cfg->PubKeyInfo);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_PUB_KEY_INFO_FIELD_FAIL);
		goto END;
	}

	/** Generate CSR extensions. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenCsrExtensions(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_EXTENSION_FIELD_FAIL);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	*CsrLen = CertInstance.Offset - CsrStart;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function DER encodes the distinguished name attributes into an X.501
 *		RDNSequence.
 *
 * @param	Name		Pointer to the structure holding the distinguished name attributes.
 * @param	DerBuf		Pointer to the buffer for storing the DER encoded name.
 * @param	DerBufSize	Size of the buffer used for storing the DER encoded name.
 * @param	DerLen		Pointer to store the length of the DER encoded name.
 *
 * @return
 *	- XASUFW_SUCCESS, if the name is DER encoded successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_NAME_ATTRIBUTE_FAIL, if creation of a name attribute is failed.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if updating encoded length is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 * @note	Name ::= RDNSequence
 *		RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
 *
 *		The organization name attribute (OID 2.5.4.10) is encoded before the common name
 *		attribute (OID 2.5.4.3), and each attribute value is encoded as a PrintableString.
 *
 *************************************************************************************************/
static s32 X509_Asn1EncodeName(const XAsu_X509Name *Name, u8 *DerBuf, u32 DerBufSize,
			       u32 *DerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	static const u8 Oid_OrganizationName[] = {0x06U, 0x03U, 0x55U, 0x04U, 0x0AU};
	static const u8 Oid_CommonName[] = {0x06U, 0x03U, 0x55U, 0x04U, 0x03U};
	X509_CertGenInfo CallerInstance = CertInstance;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;

	/** Initialize certificate generation instance at the given buffer. */
	CertInstance.Buf = DerBuf;
	CertInstance.BufSize = DerBufSize;
	CertInstance.Offset = 0U;

	/** Validate buffer space and update tag, length and value field of the RDNSequence. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add the organization name attribute. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateNameAttribute(Oid_OrganizationName,
					      XASUFW_ARRAY_SIZE(Oid_OrganizationName),
					      Name->OrgName, Name->OrgNameLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_NAME_ATTRIBUTE_FAIL);
		goto END;
	}

	/** Add the common name attribute. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateNameAttribute(Oid_CommonName, XASUFW_ARRAY_SIZE(Oid_CommonName),
					      Name->CommonName, Name->CommonNameLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_NAME_ATTRIBUTE_FAIL);
		goto END;
	}

	/** Update the length of the RDNSequence. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	*DerLen = CertInstance.Offset;

END:
	/** Restore certificate generation instance. */
	CertInstance = CallerInstance;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function DER encodes the validity period into an ASN.1 SEQUENCE of two
 *		times.
 *
 * @param	Validity	Pointer to the structure holding the validity period.
 * @param	DerBuf		Pointer to the buffer for storing the DER encoded validity.
 * @param	DerBufSize	Size of the buffer used for storing the DER encoded validity.
 * @param	DerLen		Pointer to store the length of the DER encoded validity.
 *
 * @return
 *	- XASUFW_SUCCESS, if the validity is DER encoded successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL, if buffer validation is failed.
 *	- XASUFW_X509_CREATE_TIME_FIELD_FAIL, if creation of an ASN.1 time is failed.
 *	- XASUFW_X509_UPDATE_ENCODED_LEN_FAIL, if updating encoded length is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 * @note	Validity ::= SEQUENCE {
 *			notBefore	Time,
 *			notAfter	Time
 *		}
 *
 *		Time ::= CHOICE {
 *			utcTime		UTCTime,
 *			generalTime	GeneralizedTime
 *		}
 *
 *************************************************************************************************/
static s32 X509_Asn1EncodeValidity(const XAsu_X509Validity *Validity, u8 *DerBuf,
				   u32 DerBufSize, u32 *DerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_CertGenInfo CallerInstance = CertInstance;
	u8 *SequenceLenIdx = NULL;
	u8 *SequenceValIdx = NULL;

	/** Initialize certificate generation instance at the given buffer. */
	CertInstance.Buf = DerBuf;
	CertInstance.BufSize = DerBufSize;
	CertInstance.Offset = 0U;

	/** Validate buffer space and update tag, length and value field of the validity. */
	Status = X509_ValidateAndUpdateTlvField(X509_ASN1_TAG_SEQUENCE, &SequenceLenIdx,
						&SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_VALIDATE_BUFFER_SPACE_FAIL;
		goto END;
	}

	/** Add the start time of the validity period. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateCertTime(&Validity->NotBefore);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_TIME_FIELD_FAIL);
		goto END;
	}

	/** Add the end time of the validity period. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateCertTime(&Validity->NotAfter);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_TIME_FIELD_FAIL);
		goto END;
	}

	/** Update the length of the validity. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_UpdateEncodedLength(SequenceLenIdx,
					  (u32)(&(CertInstance.Buf[CertInstance.Offset]) -
						SequenceValIdx),
					  SequenceValIdx);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_UPDATE_ENCODED_LEN_FAIL);
		goto END;
	}
	if ((*SequenceLenIdx & (u8)(~X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES)) != 0U) {
		CertInstance.Offset += (u32)((*SequenceLenIdx) & X509_LOWER_NIBBLE_MASK);
	}

	*DerLen = CertInstance.Offset;

END:
	/** Restore certificate generation instance. */
	CertInstance = CallerInstance;

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks whether the given character belongs to the ASN.1
 *		PrintableString character set.
 *
 * @param	Char	Character to be checked.
 *
 * @return
 *	- XASU_TRUE, if the character belongs to the PrintableString character set.
 *	- XASU_FALSE, otherwise.
 *
 *************************************************************************************************/
static u32 X509_Asn1IsPrintableChar(u8 Char)
{
	u32 IsPrintable = (u32)XASU_FALSE;

	if (((Char >= (u8)'A') && (Char <= (u8)'Z')) ||
	    ((Char >= (u8)'a') && (Char <= (u8)'z')) ||
	    ((Char >= (u8)'0') && (Char <= (u8)'9')) ||
	    (Char == (u8)' ') || (Char == (u8)'\'') || (Char == (u8)'(') ||
	    (Char == (u8)')') || (Char == (u8)'+') || (Char == (u8)',') ||
	    (Char == (u8)'-') || (Char == (u8)'.') || (Char == (u8)'/') ||
	    (Char == (u8)':') || (Char == (u8)'=') || (Char == (u8)'?')) {
		IsPrintable = (u32)XASU_TRUE;
	}

	return IsPrintable;
}

/*************************************************************************************************/
/**
 * @brief	This function validates that the given string can be encoded as an ASN.1
 *		PrintableString.
 *
 * @param	Str		Pointer to the string.
 * @param	StrLen		Length of the string.
 * @param	MaxStrLen	Maximum permitted length of the string.
 *
 * @return
 *	- XASUFW_SUCCESS, if the string is a valid PrintableString.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_PARAM, if parameter is invalid.
 *	- XASUFW_X509_INVALID_DATA, if the string holds a character outside the PrintableString
 *	  character set.
 *
 *************************************************************************************************/
static s32 X509_ValidatePrintableString(const u8 *Str, u32 StrLen, u32 MaxStrLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Idx;

	/** Validate the string and its length. */
	if ((Str == NULL) || (StrLen == 0U) || (StrLen > MaxStrLen)) {
		Status = XASUFW_X509_INVALID_PARAM;
		goto END;
	}

	/** Validate each character of the string against the PrintableString character set. */
	for (Idx = 0U; Idx < StrLen; Idx++) {
		if (X509_Asn1IsPrintableChar(Str[Idx]) != (u32)XASU_TRUE) {
			Status = XASUFW_X509_INVALID_DATA;
			goto END;
		}
	}

	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the attributes of a distinguished name.
 *
 * @param	Name	Pointer to the structure holding the distinguished name attributes.
 *
 * @return
 *	- XASUFW_SUCCESS, if the distinguished name is valid.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_INVALID_DATA, if the distinguished name is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_ValidateName(const XAsu_X509Name *Name)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/** Validate the organization name. */
	Status = X509_ValidatePrintableString(Name->OrgName, Name->OrgNameLen,
					      XASU_X509_ORG_NAME_MAX_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_INVALID_DATA;
		goto END;
	}

	/** Validate the common name. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_ValidatePrintableString(Name->CommonName, Name->CommonNameLen,
					      XASU_X509_COMMON_NAME_MAX_LEN);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_INVALID_DATA;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates the validity period.
 *
 * @param	Validity	Pointer to the structure holding the validity period.
 *
 * @return
 *	- XASUFW_SUCCESS, if the validity period is valid.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_X509_TIME_AND_DATE_INVALID, if the date and time is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_ValidateValidity(const XAsu_X509Validity *Validity)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_DateTime DateTime = {0U};

	/** Validate the start time of the validity period. */
	X509_GetDateTime(&Validity->NotBefore, &DateTime);
	Status = X509_ValidateTimeAndDate(&DateTime);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_TIME_AND_DATE_INVALID;
		goto END;
	}

	/** Validate the end time of the validity period. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	X509_GetDateTime(&Validity->NotAfter, &DateTime);
	Status = X509_ValidateTimeAndDate(&DateTime);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_X509_TIME_AND_DATE_INVALID;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fills the date and time used within the module from the date and time
 *		supplied by the user.
 *
 * @param	Time		Pointer to the user supplied date and time.
 * @param	DateTime	Pointer to the date and time to be filled.
 *
 *************************************************************************************************/
static inline void X509_GetDateTime(const XAsu_X509Time *Time, X509_DateTime *DateTime)
{
	DateTime->Year = Time->Year;
	DateTime->Month = Time->Month;
	DateTime->Day = Time->Day;
	DateTime->Hour = Time->Hour;
	DateTime->Min = Time->Minute;
	DateTime->Sec = Time->Second;
}

/*************************************************************************************************/
/**
 * @brief	This function creates a DER encoded ASN.1 time from the given date and time.
 *
 * @param	Time	Pointer to the date and time.
 *
 * @return
 *	- XASUFW_SUCCESS, if the ASN.1 time is created successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_INT_TO_ASCII_FAIL, if the integer to ASCII conversion fails.
 *	- XASUFW_X509_INVALID_DATA, if the date and time is invalid.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 * @note	RFC 5280 requires a date through the year 2049 to be encoded as an UTCTime holding
 *		a two digit year and a date from the year 2050 onwards to be encoded as a
 *		GeneralizedTime holding a four digit year. Both forms are terminated with the zero
 *		time offset, as the time is always UTC.
 *
 *		The date and time is validated by X509_BuildUserCfg() before it reaches this
 *		function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateCertTime(const XAsu_X509Time *Time)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	X509_DateTime DateTime = {0U};
	u8 TimeStr[X509_GENERALIZED_TIME_LEN] = {0U};
	const u8 *const SegmentVal[] = {&DateTime.Month, &DateTime.Day, &DateTime.Hour,
					&DateTime.Min, &DateTime.Sec};
	u32 Offset;
	u32 Idx;
	u32 YearLen;
	u32 YearVal;
	u32 TimeLen;
	u8 Asn1Tag;

	X509_GetDateTime(Time, &DateTime);

	/** Select the encoded form and the year it holds from the year of the date. */
	if (DateTime.Year < X509_ASN1_MIN_GENERALIZED_YEAR) {
		if (DateTime.Year < X509_ASN1_MIN_UTC_YEAR) {
			Status = XASUFW_X509_INVALID_DATA;
			goto END;
		}
		YearLen = X509_ASN1_YEAR_LEN_UTC_BYTES;
		TimeLen = X509_UTC_TIME_LEN;
		Asn1Tag = X509_ASN1_TAG_UTC_TIME;
		YearVal = (u32)DateTime.Year % X509_ASN1_YEARS_PER_CENTURY;
	} else {
		YearLen = X509_ASN1_YEAR_LEN_GEN_BYTES;
		TimeLen = X509_GENERALIZED_TIME_LEN;
		Asn1Tag = X509_ASN1_TAG_GENERALIZED_TIME;
		YearVal = (u32)DateTime.Year;
	}

	/** Format the date and time as the characters of the selected form. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XAsufw_IntToAscii(TimeStr, YearLen, YearVal);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_INT_TO_ASCII_FAIL;
		goto END;
	}

	Offset = YearLen;
	for (Idx = 0U; Idx < XASUFW_ARRAY_SIZE(SegmentVal); Idx++) {
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_IntToAscii(&TimeStr[Offset], X509_ASN1_TIME_SEGMENT_LEN,
					   (u32)*SegmentVal[Idx]);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_INT_TO_ASCII_FAIL;
			goto END;
		}
		Offset += X509_ASN1_TIME_SEGMENT_LEN;
	}
	TimeStr[Offset] = (u8)X509_ASN1_ZERO_TIME_OFFSET_VAL;

	/** Add the tag, length and value field of the time. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateTlvField(Asn1Tag, TimeStr, TimeLen);

END:
	return Status;
}
/** @} */
