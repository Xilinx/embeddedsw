/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file x509_cert.h
 *
 * This file contains declarations for X.509 certificate related files.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  05/08/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup x509_apis X.509 APIs
* @{
*/
#ifndef X509_CERT_H_
#define X509_CERT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
#define X509_ISSUER_MAX_SIZE		(600U)	/**< Max length of the DER encoded Issuer field
						received from CDO */
#define X509_SUBJECT_MAX_SIZE		(600U)	/**< Max length of the DER encoded Subject field
						received from CDO */
#define X509_VALIDITY_MAX_SIZE		(40U)	/**< Max length of the DER encoded Validity field
						received from CDO */
#define X509_SUB_ALT_NAME_MAX_SIZE	(90U)	/**< Max length of the DER encoded Subject
						Alternative Name field received from CDO */

/************************************** Type Definitions *****************************************/
/**
 * This typedef contains information about public key type.
 */
typedef enum {
	X509_PUB_KEY_UNSUPPORTED = 0U,	/**< Unsupported or unknown key */
	X509_PUB_KEY_ECC,		/**< ECC public key */
	X509_PUB_KEY_RSA,		/**< RSA public key */
	X509_PUB_KEY_MAX,		/**< Maximum public key supported */
} X509_PublicKeyType;

/**
 * This structure contains information about the type of public key and its OID.
 */
typedef struct {
	X509_PublicKeyType PubKeyType;	/**< Public key type */
	const u8 *Oid;			/**< Pointer to variable containing OID */
	u8 OidLen;			/**< Public key OID length */
} X509_OidPublicKeyDescriptor;

/**
 * This typedef contains information about extension types.
 */
typedef enum {
	X509_EXTN_UNSUPPORTED = 0U,	/**< Unsupported or unknown extension */
	X509_EXTN_AUTH_KEY_IDENTIFIER,	/**< Authority Key Identifier extension */
	X509_EXTN_SUB_KEY_IDENTIFIER,	/**< Subject Key Identifier extension */
	X509_EXTN_KEY_USAGE,		/**< Key Usage extension */
	X509_EXTN_SUB_ALT_NAME,		/**< Subject Alternative Name extension */
	X509_EXTN_BASIC_CONSTRAINTS,	/**< Basic Constraints extension */
	X509_EXTN_EXTD_KEY_USAGE,	/**< Extended Key Usage extension */
} X509_ExtensionType;

/**
 * This structure contains information about extension type and its OID.
 */
typedef struct {
	X509_ExtensionType ExtnType;	/**< Extension type */
	const u8 *ExtnId;		/**< Pointer to variable containing OID */
	u8 ExtnLen;			/**< Extension OID length */
} X509_ExtensionIdDescriptor;

/**
 * This structure holds the information about date and time.
 */
typedef struct {
	u8 Day;		/**< Day of the month (1-31). */
	u8 Hour;	/**< Hour of the day (0-23). */
	u8 Min;		/**< Minute of the hour (0-59). */
	u8 Sec;		/**< Second of the minute (0-59). */
	u8 Month;	/**< Month of the year (1-12). */
	u16 Year;	/**< Year */
} X509_DateTime;

/**
 * This typedef holds information about months of the year.
 */
enum {
	JANUARY = 1U,	/**< January */
	FEBRUARY,	/**< February */
	MARCH,		/**< March */
	APRIL,		/**< April */
	MAY,		/**< May */
	JUNE,		/**< June */
	JULY,		/**< July */
	AUGUST,		/**< August */
	SEPTEMBER,	/**< September */
	OCTOBER,	/**< October */
	NOVEMBER,	/**< November */
	DECEMBER,	/**< December */
};

/**
 * This structure holds RSA public key parameters information.
 */
typedef struct {
	u8 *Modulus;			/**< Pointer to the Modulus part of RSA public key */
	u32 ModulusLen;			/**< Modulus length */
	u8 *Exponent;			/**< Pointer to the Exponent part of RSA public key */
	u32 ExponentLen;		/**< Exponent length */
} X509_RsaPublicKey;

/**
 * This structure holds ECC public key parameters information.
 */
typedef struct {
	u8 *PublicKey;			/**< Pointer to the uncompressed ECC public key */
	u32 PublicKeyLen;		/**< ECC public key length */
} X509_EccPublicKey;

/**
 * This union holds public key value for RSA or ECC algorithms.
 */
typedef union {
	X509_EccPublicKey EccPublicKey;		/**< ECC public key */
	X509_RsaPublicKey RsaPublicKey;		/**< RSA public key */
} X509_PublicKey;

/**
 * This structure holds X.509 certificate fields to be parsed.
 */
typedef struct {
	X509_PublicKey PublicKey;		/**< Public key */
	X509_PublicKeyType PublicKeyType;	/**< Public key type (e.g. ECC, RSA, etc) */
	u32 KeyUsage;				/**< Key usage extension value */
} X509_CertInfo;

/**
 * This typedef contains information about signature type.
 */
typedef enum {
	X509_SIGN_TYPE_ECC_SHA3_256 = 0U,	/**< Signature type SHA3-256 */
	X509_SIGN_TYPE_ECC_SHA3_384,		/**< Signature type SHA3-384 */
	X509_SIGN_TYPE_MAX,			/**< Maximum no of supported signature type */
} X509_SignAlgoType;

/**
 * This structure contains information about signature type and its OID.
 */
typedef struct {
	X509_SignAlgoType SignType;	/**< Signature algorithm type */
	const u8 *SignOid;		/**< Pointer to variable containing OID */
	u8 SignLen;			/**< Signature OID length */
} X509_SignatureOidDescriptor;

/**
 * This typedef contains information about ECC curve type.
 */
typedef enum {
	X509_ECC_CURVE_TYPE_256 = 0U,	/**< ECC curve type P-256 */
	X509_ECC_CURVE_TYPE_384,	/**< ECC curve type P-384 */
	X509_ECC_CURVE_TYPE_MAX,	/**< Maximum number of supported curve type */
} X509_EccCurveType;

/**
 * This structure contains information about parameter type and its OID.
 */
typedef struct {
	X509_EccCurveType EccCurveType;	/**< ECC curve type */
	const u8 *ParamOid;		/**< Pointer to variable containing OID */
	u8 ParamOidLen;			/**< Parameter OID length */
} X509_AlgoEccParam;

typedef s32 (*X509_GenerateDigest_t)(const u8 *Buf, u32 DataLen, const u8 *Hash, u32 HashBufLen,
				     u32 *HashLen, const void *PlatformData);
typedef	s32 (*X509_GenerateSignature_t)(const u8 *Hash, u32 HashLen, const u8 *Sign, u32 SignLen,
					u32 *SignActualLen, u8 *PvtKey, const void *PlatformData);
/**
 * This structure holds the information about user configs.
 */
typedef struct {
	X509_SignAlgoType SignType;			/**< Signature algorithm type */
	X509_GenerateDigest_t GenerateDigest;		/**< Function pointer to generate digest */
	X509_GenerateSignature_t GenerateSignature;	/**< Function pointer to generate
							signature */
} X509_InitData;

/**
 * This structure contains information about fields of X.509 certificate which is provided by user.
 */
typedef struct {
	u8 Issuer[X509_ISSUER_MAX_SIZE];		/**< DER encoded value of Issuer */
	u32 IssuerLen;					/**< Length of DER encoded Issuer field */
	u8 Subject[X509_SUBJECT_MAX_SIZE];		/**< DER encoded value of Subject */
	u32 SubjectLen;					/**< Length of DER encoded Subject field */
	u8 Validity[X509_VALIDITY_MAX_SIZE];		/**< DER encoded value of Validity */
	u32 ValidityLen;				/**< Length of DER encoded Validity field */
	u32 IsSubAltNameAvailable;			/**< Flag to indicate if Subject Alt Name is
							given by user */
	u8 SubAltName[X509_SUB_ALT_NAME_MAX_SIZE];	/**< DER encoded value of Subject Alt
							Name */
	u32 SubAltNameLen;				/**< Length of DER encoded Subject Alt
							Name */
} X509_UserCfg;

/**
 * This structure contains information about subject public key.
 */
typedef struct {
	X509_PublicKeyType PubKeyType;	/**< Public Key type */
	X509_EccCurveType EccCurveType;	/**< ECC curve type */
	u32 SubjectPubKeyLen;		/**< Issuer Public Key Length */
	u8 *SubjectPublicKey;		/**< Subject Public Key */
} X509_SubjectPublicKeyInfo;

/**
 * This structure contains the configuration for X.509 certificate for given subsystem ID.
 */
typedef struct {
	X509_UserCfg *UserCfg;			/**< Configuration from User */
	X509_SubjectPublicKeyInfo PubKeyInfo;	/**< Subject public key information */
	u32 IsCsr;				/**< Flag to check if Certificate Signing Request */
	u32 IsSelfSigned;			/**< Flag to check if self-signed certificate */
	u32 IssuerPubKeyLen;			/**< Issuer public Key length */
	u8 *IssuerPublicKey;			/**< Issuer public Key */
	u8 *IssuerPrvtKey;			/**< Issuer private Key */
	void *PlatformData;			/**< Platform specific parameters required for
						platform specific digest calculation and signature
						generation APIs */
} X509_Config;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define X509_SINGLE_BYTE				(1U)	/**< Value of single byte */
#define X509_VERSION_VALUE_V3				(0x02U)	/**< X.509 certificate version
								v3 */

#define X509_ASN1_UNUSED_BITS_SIZE_IN_BYTE		(1U)	/**< No of bytes used to store
								count of unused bits */
#define X509_ASN1_TAG_LEN_VAL_MIN_SIZE			(3U)	/**< Minimum size of ASN.1
								TLV(Tag, Len, Value) pair */
#define X509_ASN1_LEN_VAL_MIN_SIZE			(2U)	/**< Minimum size of ASN.1
								LV(Len, Value) pair */
#define X509_ASN1_TAG_LEN				(1U)	/**< ASN.1 tag length */
#define X509_ASN1_BIT7_MASK				(0x80U)	/**< Mask to get bit 7 */
#define X509_ASN1_NO_OF_BITS_IN_BYTE			(8U)	/**< Number of bits in a byte */
#define X509_ASN1_YEAR_LEN_GEN_BYTES			(4U)	/**< Year length for
								generalized(YYYY) format */
#define X509_ASN1_YEAR_LEN_UTC_BYTES			(2U)	/**< Year length for UTC(YY)
								format */
#define X509_ASN1_TIME_WITH_OUT_ZERO_TIME_OFFSET(n)	(10U + (n))	/**< Length value without
									trailing 'Z' (YYMMDDHHMMSSZ)
									/(YYYYMMDDHHMMSS) */
#define X509_ASN1_TIME_WITH_ZERO_TIME_OFFSET(n)		(11U + (n))	/**< Length value with
									trailing 'Z' (YYMMDDHHMMSSZ)
									/(YYYYMMDDHHMMSSZ) */
#define X509_ASN1_MAX_LEN_BYTES				(4U)	/**< Maximum supported length bytes
								in ASN.1 */
#define X509_ASN1_MIN_LEN_BYTES				(1U)	/**< Minimum supported length bytes
								in ASN.1 */
#define X509_ASN1_YEAR_MIN_OFFSET			(1900U)	/**< Minimum year offset for valid
								certificate */
#define X509_ASN1_YEAR_MAX_OFFSET			(2000U)	/**< Maximum year offset for valid
								certificate */
#define X509_ASN1_THRESHOLD_YEARS			(50U)	/**< Threshold year to decide valid
								year */
#define X509_ASN1_TIME_SEGMENT_LEN			(2U)	/**< Number of bytes for each ASN.1
								time field (e.g month, day, hour) */
#define X509_ASN1_ZERO_TIME_OFFSET_LEN			(1U)	/**< Length of zero time offset */
#define X509_ASN1_ZERO_TIME_OFFSET_VAL			(0x5A)	/**< Value of zero time offset */
#define X509_ASN1_MAX_UNUSED_BITS			(7U)	/**< Maximum unused bits */
#define X509_ASN1_BYTE0_MASK				(0xFFU)	/**< Mask to get byte 0 */
#define X509_ASN1_BYTE1_MASK				(0xFF00U)	/**< Mask to get byte 1 */
#define X509_ASN1_SHORT_FORM_MAX_LENGTH_IN_BYTES	(127U)	/**< Max length for which short form
								encoding of length is used */
#define X509_ASN1_LONG_FORM_2_BYTES_MAX_LENGTH_IN_BYTES	(255U)	/**< Max length for which long form
								encoding of 2 byte length is used */
#define X509_ASN1_LONG_FORM_LENGTH_1BYTE		(0x81U)	/**< Indicates that length is
								1 byte long*/
#define X509_ASN1_LONG_FORM_LENGTH_2BYTES		(0x82U) /**< Indicates that length is
								2 bytes long*/
#define X509_ASN1_LEN_OF_VALUE_OF_BOOLEAN		(0x1U)	/**< Len of boolean */
#define X509_ASN1_BOOLEAN_TRUE				(0xFFU)	/**< ASN.1 value of boolean TRUE */
#define X509_ASN1_BOOLEAN_FALSE				(0x00U)	/**< ASN.1 value of boolean FALSE */

/**< ASN.1 tags */
#define X509_ASN1_TAG_BOOLEAN				(0x01U)	/**< ASN.1 tag for boolean */
#define X509_ASN1_TAG_INTEGER				(0x02U)	/**< ASN.1 tag for integer */
#define X509_ASN1_TAG_BIT_STRING			(0x03U)	/**< ASN.1 tag for bit string */
#define X509_ASN1_TAG_OCTET_STRING			(0x04U)	/**< ASN.1 tag for octet string */
#define X509_ASN1_TAG_NULL				(0x05U)	/**< ASN.1 tag for null */
#define X509_ASN1_TAG_OID				(0x06U)	/**< ASN.1 tag for OIDs */
#define X509_ASN1_TAG_UTC_TIME				(0x17U)	/**< ASN.1 tag for UTC time
								format */
#define X509_ASN1_TAG_GENERALIZED_TIME			(0x18U)	/**< ASN.1 tag for GENERALIZED
								time format */
#define X509_ASN1_TAG_SEQUENCE				(0x30U)	/**< ASN.1 tag for sequence */
#define X509_ASN1_TAG_SET				(0x31U) /**< ASN.1 tag for set */
#define X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG	(0xA0U)	/**< ASN.1 optional parameter 0
								constructed tag */
#define X509_ASN1_TAG_OPTIONAL_PARAM_3_CONSTRUCTED_TAG	(0xA3U)	/**< ASN.1 optional parameter 3
								constructed tag */
#define X509_ASN1_TAG_CONTEXT_SPECIFIC			(0x80U) /**< ASN.1 tag for context */

/************************************ Function Prototypes ****************************************/
s32 X509_ParseCertificate(u64 X509CertAddr, u32 Size, X509_CertInfo *CertInfo);
s32 X509_GenerateX509Cert(u64 X509CertAddr, u32 MaxCertSize, u32 *X509CertSize,
			  const X509_Config *Cfg);
s32 X509_Init(const X509_InitData *CfgData);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* X509_CERT_H_ */
/** @} */
