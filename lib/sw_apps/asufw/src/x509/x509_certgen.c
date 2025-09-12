/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
*
**************************************************************************************************/
/**
 * @addtogroup x509_apis X.509 APIs
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
#define X509_VERSION_VALUE_V0					(0x00U)	/**< Value of version 0 */
#define X509_SERIAL_VAL_LEN					(0x14U)	/**< Length of Serial
									field */
#define X509_SERIAL_FIELD_LEN					(22U)	/**< Length of Serial
									Field */
#define X509_AUTH_KEY_ID_VAL_LEN				(20U)	/**< Length of value of
									Authority Key ID */
#define X509_SUB_KEY_ID_VAL_LEN					(20U)	/**< Length of value of
									Subject Key ID */
#define X509_KEYUSAGE_VAL_LEN					(2U)	/**< Length of value of key
									usage */
#define X509_LOWER_NIBBLE_MASK					(0xFU)	/**< Mask to get lower
									nibble */
#define X509_NULL_VALUE						(0x00U)	/**< Value of NULL */
#define X509_HASH_MAX_SIZE_IN_BYTES				(48)	/**< Maximum length of
									hash */
#define X509_SIGNATURE_MAX_SIZE_IN_BYTES			(96U)	/**< Maximum length of
									signature */
#define X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN		(97U)	/**< ECC uncompressed public
									key length */

/************************************ Type Definitions *******************************************/
/**
 * This structure holds certificate generation information.
 */
typedef struct {
	u8 *Buf;			/**< Pointer to store X.509 certificate */
	u32 Offset;			/**< Current offset of buffer */
	u32 SerialOffset;		/**< Offset of serial field */
} X509_CertGenInfo;

/**
 * This typedef contains information about Key Usage options.
 */
typedef enum {
	X509_DIGITALSIGNATURE,	/**< Digital Signature */
	X509_NONREPUDIATION,	/**< Non Repudiation */
	X509_KEYENCIPHERMENT,	/**< Key Encipherment */
	X509_DATAENCIPHERMENT,	/**< Data Encipherment */
	X509_KEYAGREEMENT,	/**< Key Agreement */
	X509_KEYCERTSIGN,	/**< Key Certificate Sign */
	X509_CRLSIGN,		/**< CRL Sign */
	X509_ENCIPHERONLY,	/**< Encipher Only */
	X509_DECIPHERONLY	/**< Decipher Only */
} X509_KeyUsageOption;

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
static s32 X509_UpdateEncodedLength(u8 *LenIdx, u32 Len, u8 *ValIdx);
static u32 X509_Asn1GetFirstNonZeroByteOffset(const u8 *Data, u32 Cnt);
static s32 X509_Asn1CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen);
static s32 X509_Asn1CreateInteger(const u8 *IntegerVal, u32 IntegerLen);
static s32 X509_Asn1CreateRawDataFromByteArray(const u8 *RawData, const u32 LenOfRawDataVal);
static u8 X509_Asn1GetTrailingZeroesCount(u8 Data);
static s32 X509_Asn1CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull);
static s32 X509_Asn1CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen);
static void X509_Asn1CreateBoolean(const u32 BooleanVal);
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
static void X509_UpdateKeyUsageVal(u8 *KeyUsageVal, X509_KeyUsageOption KeyUsageOption);
static s32 X509_GenKeyUsageField(const X509_Config *Cfg);
static s32 X509_GenExtKeyUsageField(void);
static inline s32 X509_GenSubAltNameField(const u8 *SubAltName, const u32 SubAltNameValLen);
static inline void X509_AddTagField(u8 Asn1Tag);
static s32 X509_GenX509v3ExtensionsField(const X509_Config *Cfg);
static s32 X509_GenSerialField(const u8 *DataHash);
static s32 X509_GenTBSCertificate(const X509_Config *Cfg, u32 *TBSCertLen);
static s32 X509_GenSignField(const u8 *Signature, u32 SignLen);
static s32 X509_GenCsrExtensions(const X509_Config *Cfg);
static s32 X509_GenCertReqInfo(const X509_Config *Cfg, u32 *CsrLen);

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
	const u8 *TBSCertStart;
	const u8 Hash[X509_HASH_MAX_SIZE_IN_BYTES] = {0U};
	const u8 Sign[X509_SIGNATURE_MAX_SIZE_IN_BYTES] = {0U};
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;

	/** Validate certificate address, size and config. */
	if ((X509CertAddr == 0U) || (X509CertSize == NULL) || (Cfg == NULL) ||
	    (MaxCertSize < X509_CERTIFICATE_MAX_SIZE_IN_BYTES)) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate X.509 configs. */
	if ((Cfg->UserCfg == NULL) || ((Cfg->IsCsr != XASU_TRUE) && (Cfg->IsCsr != XASU_FALSE)) ||
	    ((Cfg->IsSelfSigned != XASU_TRUE) && (Cfg->IsSelfSigned != XASU_FALSE))) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate X.509 user configs. */
	if ((Cfg->UserCfg->IssuerLen == 0U) || (Cfg->UserCfg->SubjectLen == 0U) ||
	    (Cfg->UserCfg->ValidityLen == 0U)) {
		Status = XASUFW_INVALID_PARAM;
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
	     XAsu_DoubleCurveLength(XASU_ECC_P384_SIZE_IN_BYTES))) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Initialize X.509 certificate generation instance. */
	CertInstance.Buf = (u8 *)(UINTPTR)X509CertAddr;
	CertInstance.Offset = 0U;

	/** Add sequence tag and move buffer to next address. */
	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	TBSCertStart = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate CSR or TBS depending on the request. */
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
	InitData->VerifySignature = CfgData->VerifySignature;

	Status = XASUFW_SUCCESS;

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

	if (Len <= X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*LenIdx = (u8)Len;
		Status = XASUFW_SUCCESS;
	} else if (Len <= X509_ASN1_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES) {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_1BYTE;
		Status = Xil_SMemMove(ValIdx + XASUFW_BUFFER_INDEX_ONE, Len, ValIdx, Len, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_MOVE_FAIL;
			goto END;
		}
		*(LenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)Len;
	} else {
		*LenIdx = X509_ASN1_LONG_FORM_LENGTH_2BYTES;
		Status = Xil_SMemMove(ValIdx + XASUFW_BUFFER_INDEX_TWO, Len, ValIdx, Len, Len);
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
 * @brief	This function provides the pointer to the X.509 initial data.
 *
 * @return
 *	- Pointer to structure which stores initialized data.
 *
 *************************************************************************************************/
X509_InitData *X509_GetInitData(void)
{
	static X509_InitData InitData;

	return &InitData;
}

/*************************************************************************************************/
/**
 * @brief	This function takes a byte array as input and returns offset of a
 *		starting non-zero byte.
 *
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
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateIntegerFieldFromByteArray(const u8 *IntegerVal, u32 IntegerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 Offset = 0U;

	Offset = X509_Asn1GetFirstNonZeroByteOffset(IntegerVal, IntegerLen - 1U);
	/**
	 * If the most significant bit in the first byte of IntegerVal is set,
	 * then the value must be prepended with 0x00.
	 */
	if ((IntegerVal[Offset] & X509_ASN1_BIT7_MASK) == X509_ASN1_BIT7_MASK) {
		CertInstance.Buf[CertInstance.Offset++] = 0x0U;
	}

	/** Copy integer values. */
	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), IntegerLen - Offset,
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
 *	- XASUFW_X509_CREATE_INTEGER_FIELD_FROM_ARRAY_FAIL, if create integer field is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateInteger(const u8 *IntegerVal, u32 IntegerLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *IntegerLenIdx;
	const u8 *IntegerValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_INTEGER);

	IntegerLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	IntegerValIdx = &(CertInstance.Buf[CertInstance.Offset]);

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

	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), LenOfRawDataVal, RawData,
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
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateBitString(const u8 *BitStringVal, u32 BitStringLen, u32 IsLastByteFull)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 NumofTrailingZeroes = 0U;
	u8 *BitStringLenIdx;
	const u8 *BitStringValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_BIT_STRING);
	BitStringLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	BitStringValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/**
	 * The first byte of the value of the BITSTRING is used to show the number
	 * of unused bits in the last byte of the BITSTRING.
	 */
	if (IsLastByteFull == XASU_FALSE) {
		NumofTrailingZeroes = X509_Asn1GetTrailingZeroesCount(*(BitStringVal +
								      BitStringLen - 1U));
	}
	CertInstance.Buf[CertInstance.Offset++] = NumofTrailingZeroes;

	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), BitStringLen,
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
 * @brief	This function creates DER encoded ASN.1 OctetString.
 *
 * @param	OctetStringVal	Value of the ASN.1 OctetString.
 * @param	OctetStringLen	Length of the value of the ASN.1 OctetString.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created DER encoded ASN.1 OctetString.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy is failed.
 *	- Error code received from called functions in case of other failure from the called
 *	function.
 *
 *************************************************************************************************/
static s32 X509_Asn1CreateOctetString(const u8 *OctetStringVal, u32 OctetStringLen)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u8 *OctetStringLenIdx;

	X509_AddTagField(X509_ASN1_TAG_OCTET_STRING);
	OctetStringLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);

	if (OctetStringLen <= X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES) {
		*OctetStringLenIdx = (u8)OctetStringLen;
	} else if (OctetStringLen <= X509_ASN1_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES) {
		*OctetStringLenIdx = X509_ASN1_LONG_FORM_LENGTH_1BYTE;
		*(OctetStringLenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)OctetStringLen;
		CertInstance.Offset++;
	} else {
		*OctetStringLenIdx = X509_ASN1_LONG_FORM_LENGTH_2BYTES;
		*(OctetStringLenIdx + XASUFW_BUFFER_INDEX_ONE) = (u8)((OctetStringLen &
								      X509_ASN1_BYTE1_MASK)
								>> X509_ASN1_NO_OF_BITS_IN_BYTE);
		*(OctetStringLenIdx + XASUFW_BUFFER_INDEX_TWO) = (u8)(OctetStringLen &
								      X509_ASN1_BYTE0_MASK);
		CertInstance.Offset += XASUFW_VALUE_TWO;
	}

	Status = Xil_SMemCpy(&(CertInstance.Buf[CertInstance.Offset]), OctetStringLen,
			     OctetStringVal, OctetStringLen, OctetStringLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}
	CertInstance.Offset += OctetStringLen;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates DER encoded ASN.1 Boolean.
 *
 * @param	BooleanVal	Can be TRUE or FALSE.
 *
 * @note	ASN.1 tag for Boolean is 0x01.
 *
 *************************************************************************************************/
static void X509_Asn1CreateBoolean(const u32 BooleanVal)
{
	X509_AddTagField(X509_ASN1_TAG_BOOLEAN);
	CertInstance.Buf[CertInstance.Offset++] = X509_ASN1_LEN_OF_VALUE_OF_BOOLEAN;

	if (BooleanVal == XASU_TRUE) {
		CertInstance.Buf[CertInstance.Offset++] = (u8)X509_ASN1_BOOLEAN_TRUE;
	} else {
		CertInstance.Buf[CertInstance.Offset++] = (u8)X509_ASN1_BOOLEAN_FALSE;
	}
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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = X509_Asn1CreateInteger(&Version, X509_VERSION_VAL_LEN);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates the Signature Algorithm field. This field
 *		is present in TBS certificate as well as the X.509 certificate.
 *
 * @return
 *	- XASUFW_SUCCESS, if successfully created Signature Algorithm field.
 *	- XASUFW_FAILURE, in case of failure.
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
 *************************************************************************************************/
static s32 X509_GenSignAlgoField(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const X509_InitData *InitData = X509_GetInitData();
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Check whether signature type is supported or not. */
	if (SignOidList[InitData->SignType].SignType != X509_SIGN_TYPE_ECC_SHA3_384) {
		Status = XASUFW_X509_UNSUPPORTED_SIGN_TYPE;
		goto END;
	}

	/** Copy signature type OID. */
	Status = X509_Asn1CreateRawDataFromByteArray(SignOidList[InitData->SignType].SignOid,
			(sizeof(SignOidList[InitData->SignType].SignOid[0]) *
			 SignOidList[InitData->SignType].SignLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	X509_AddTagField(X509_ASN1_TAG_NULL);
	CertInstance.Buf[CertInstance.Offset++] = X509_NULL_VALUE;

	End = &(CertInstance.Buf[CertInstance.Offset]);

	*SequenceLenIdx = (u8)(End - SequenceValIdx);

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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = X509_Asn1CreateRawDataFromByteArray(Issuer, IssuerValLen);

	return Status;
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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = X509_Asn1CreateRawDataFromByteArray(Validity, ValidityValLen);

	return Status;
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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = X509_Asn1CreateRawDataFromByteArray(Subject, SubjectValLen);

	return Status;
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
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	const u8 *End;
	u8 EccCurveType = (u8)PubKeyInfo->EccCurveType;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_OID);
	CertInstance.Buf[CertInstance.Offset++] = PubKeyOidList[PubKeyInfo->PubKeyType].OidLen;

	/** Copy public key type OID. */
	Status = X509_Asn1CreateRawDataFromByteArray(PubKeyOidList[PubKeyInfo->PubKeyType].Oid,
			(sizeof(PubKeyOidList[PubKeyInfo->PubKeyType].Oid[0]) *
				PubKeyOidList[PubKeyInfo->PubKeyType].OidLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	/** Copy ECC curve type OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(ParamOidList[EccCurveType].ParamOid,
				(sizeof(ParamOidList[EccCurveType].ParamOid[0]) *
				ParamOidList[EccCurveType].ParamOidLen));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
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
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate public key algorithm identifier field. */
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
	u8 *OctetStrLenIdx;
	const u8 *OctetStrValIdx;
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End;

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

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for subject key OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_SubKeyIdentifier,
						     sizeof(Oid_SubKeyIdentifier));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
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

	X509_AddTagField(X509_ASN1_TAG_OCTET_STRING);
	OctetStrLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OctetStrValIdx = &(CertInstance.Buf[CertInstance.Offset]);

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
	u8 *KeyIdSequenceLenIdx;
	u8 *KeyIdSequenceValIdx;
	u8 *OctetStrLenIdx;
	const u8 *OctetStrValIdx;
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	u8 UncompressedPublicKey[X509_ECC_P384_UNCOMPRESSED_PUBLIC_KEY_LEN] = {0U};
	const u8 *End;

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

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for authority key OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_AuthKeyIdentifier,
						     sizeof(Oid_AuthKeyIdentifier));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
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

	X509_AddTagField(X509_ASN1_TAG_OCTET_STRING);
	OctetStrLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OctetStrValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	KeyIdSequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	KeyIdSequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

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
 *		present in TBS Certificate as per the KeyUsageOption.
 *
 * @param	KeyUsageVal	Pointer in the Key Usage value buffer where the Key Usage option
 *				shall be updated.
 * @param	KeyUsageOption	Type of key usage which has to be updated.
 *
 *************************************************************************************************/
static void X509_UpdateKeyUsageVal(u8 *KeyUsageVal, X509_KeyUsageOption KeyUsageOption)
{
	u8 Idx = ((u8)KeyUsageOption / X509_ASN1_NO_OF_BITS_IN_BYTE);
	u8 ShiftVal = (X509_ASN1_NO_OF_BITS_IN_BYTE * (Idx + XASUFW_VALUE_ONE)) -
		      (u8)KeyUsageOption - XASUFW_VALUE_ONE;

	KeyUsageVal[Idx] |= 1U << ShiftVal;
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
	u8 *OctetStrLenIdx;
	const u8 *OctetStrValIdx;
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for key usage extension OID. */
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_KeyUsageExtn,
						     sizeof(Oid_KeyUsageExtn));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	X509_Asn1CreateBoolean(XASU_TRUE);

	X509_AddTagField(X509_ASN1_TAG_OCTET_STRING);
	OctetStrLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OctetStrValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	if (Cfg->IsSelfSigned == XASU_TRUE) {
		X509_UpdateKeyUsageVal(KeyUsageVal, X509_KEYCERTSIGN);
	} else {
		X509_UpdateKeyUsageVal(KeyUsageVal, X509_DIGITALSIGNATURE);
		X509_UpdateKeyUsageVal(KeyUsageVal, X509_KEYAGREEMENT);
	}

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
	u8 *EkuSequenceLenIdx;
	const u8 *EkuSequenceValIdx;
	u8 *OctetStrLenIdx;
	const u8 *OctetStrValIdx;
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for extended key usage extension OID. */
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_EkuExtn, sizeof(Oid_EkuExtn));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	X509_Asn1CreateBoolean(XASU_TRUE);

	X509_AddTagField(X509_ASN1_TAG_OCTET_STRING);
	OctetStrLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OctetStrValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	EkuSequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	EkuSequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for extended key usage client authority OID. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_EkuClientAuth, sizeof(Oid_EkuClientAuth));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
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
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	Status = X509_Asn1CreateRawDataFromByteArray(SubAltName, SubAltNameValLen);

	return Status;
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
	u8 *OptionalTagLenIdx;
	u8 *OptionalTagValIdx;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;

	X509_AddTagField(X509_ASN1_TAG_OPTIONAL_PARAM_3_CONSTRUCTED_TAG);
	OptionalTagLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OptionalTagValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate subject key identifier field. */
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

	/** Generate key usage field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenKeyUsageField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL;
		goto END;
	}

	/** Generate extended key usage field, if certificate is self signed. */
	if (Cfg->IsSelfSigned == XASU_TRUE) {
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
 *	- Error code received from called functions in case of other failure from the called
 *	  function.
 *
 * @note	CertificateSerialNumber ::= INTEGER
 *		The length of the serial must not be more than 20 bytes.
 *		The value of the serial is determined by calculating the
 *		SHA2 hash of the fields in the TBS certificate except the Version
 *		and Serial Number fields. 20 bytes from LSB of the calculated
 *		hash is updated as the Serial Number.
 *
 *************************************************************************************************/
static s32 X509_GenSerialField(const u8 *DataHash)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 LenToBeCopied = 0U;
	u8 Serial[X509_SERIAL_VAL_LEN] = {0U};
	u32 Offset;

	/** Temporary move offset to serial number to store serial number. */
	Offset = CertInstance.Offset;
	CertInstance.Offset = CertInstance.SerialOffset;

	/**
	 * The value of serial field must be 20 bytes. If the most significant
	 * bit in the first byte of Serial is set, then the value shall be
	 * prepended with 0x00 after DER encoding.
	 * So if MSB is set then 00 followed by 19 bytes of hash will be the serial
	 * value. If not set then 20 bytes of hash will be used as serial value.
	 */
	if ((*DataHash & X509_ASN1_BIT7_MASK) == X509_ASN1_BIT7_MASK) {
		LenToBeCopied = X509_SERIAL_VAL_LEN - 1U;
	} else {
		LenToBeCopied = X509_SERIAL_VAL_LEN;
	}

	Status = Xil_SMemCpy(Serial, X509_SERIAL_VAL_LEN, DataHash,
			     X509_SERIAL_VAL_LEN, LenToBeCopied);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_MEM_COPY_FAIL;
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger(Serial, LenToBeCopied);

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
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	const u8 *SerialStartIdx;
	const u8 *HashStartIdx;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG);
	CertInstance.Buf[CertInstance.Offset++] = X509_VERSION_FIELD_LEN;

	/** Generate Version field. */
	Status = X509_GenVersionField((u8)X509_VERSION_VALUE_V3);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VERSION_FIELD_FAIL);
		goto END;
	}

	/**
	 * Serial number is the lower 20 bytes of the digest calculated over the TBS certificate
	 * data excluding the Version and Serial Number field. This digest can be calculated only
	 * after these fields are populated. Store the index of the serial number where serial
	 * number is to be stored so this fields can be updated when digest is calculated over
	 * required fields.
	 */
	SerialStartIdx = &(CertInstance.Buf[CertInstance.Offset]);
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
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_ISSUER_FIELD_FAIL);
		goto END;
	}

	/** Generate Validity field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenValidityField(Cfg->UserCfg->Validity, Cfg->UserCfg->ValidityLen);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_VALIDITY_FIELD_FAIL);
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

	/** Generate X.509 v3 extensions field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenX509v3ExtensionsField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_GEN_EXTENSION_FIELD_FAIL);
		goto END;
	}

	/**
	 * Calculate Hash for all fields in the TBS certificate except Version and Serial
	 * Please note that currently SerialStartIdx points to the field after Serial.
	 * Hence this is the start pointer for calculating the hash.
	 */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = InitData->GenerateDigest(HashStartIdx,
					  (&(CertInstance.Buf[CertInstance.Offset]) -
					   SerialStartIdx),
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
	u8 *BitStrLenIdx;
	u8 *BitStrValIdx;
	u8 *SequenceLenIdx;
	const u8 *SequenceValIdx;
	const u8 *End;

	X509_AddTagField(X509_ASN1_TAG_BIT_STRING);
	BitStrLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	BitStrValIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	*BitStrValIdx = 0x00U;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	Status = X509_Asn1CreateInteger(Signature, (SignLen / 2U));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status, XASUFW_X509_CREATE_INTEGER_FIELD_FAIL);
		goto END;
	}

	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_Asn1CreateInteger((Signature + (SignLen / 2U)), (SignLen / 2U));
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
 *	- XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL, if key usage field generation is failed.
 *	- XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL, if raw data field creation is failed.
 *	- XASUFW_X509_GEN_EXT_KEY_USAGE_FIELD_FAIL, if extended key usage field generation is failed.
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
	u8 *ExtnReqSeqLenIdx;
	u8 *ExtnReqSeqValIdx;
	u8 *OptionalTagLenIdx;
	u8 *OptionalTagValIdx;
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u8 *SetLenIdx;
	u8 *SetValIdx;

	X509_AddTagField(X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG);
	OptionalTagLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	OptionalTagValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	ExtnReqSeqLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	ExtnReqSeqValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Create field for extension request OID in case of CSR. */
	Status = X509_Asn1CreateRawDataFromByteArray(Oid_ExtnRequest, sizeof(Oid_ExtnRequest));
	if (Status != XASUFW_SUCCESS) {
		Status = XAsufw_UpdateErrorStatus(Status,
				XASUFW_X509_CREATE_RAW_DATA_FIELD_FROM_ARRAY_FAIL);
		goto END;
	}

	X509_AddTagField(X509_ASN1_TAG_SET);
	SetLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SetValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate key usage field. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = X509_GenKeyUsageField(Cfg);
	if (Status != XASUFW_SUCCESS) {
		Status = (s32)XASUFW_X509_GEN_KEY_USAGE_FIELD_FAIL;
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
	u8 *SequenceLenIdx;
	u8 *SequenceValIdx;
	u32 CsrStart = CertInstance.Offset;

	X509_AddTagField(X509_ASN1_TAG_SEQUENCE);
	SequenceLenIdx = &(CertInstance.Buf[CertInstance.Offset++]);
	SequenceValIdx = &(CertInstance.Buf[CertInstance.Offset]);

	/** Generate Version field. */
	Status = X509_GenVersionField((u8)X509_VERSION_VALUE_V0);
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
/** @} */
