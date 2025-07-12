/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file x509_cert.h
 *
 * This file contains declarations for x509 certificate related files.
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
	u8 OidLen;			/**< Public key OID length */
	const u8 *Oid;			/**< Pointer to variable containing OID */
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
	u8 ExtnLen;			/**< Extension OID length */
	const u8 *ExtnId;		/**< Pointer to variable containing OID */
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

/*************************** Macros (Inline Functions) Definitions *******************************/
#define X509_SINGLE_BYTE				(1U)	/**< Value of single byte */
#define X509_VERSION_VALUE_V3				(0x02U)	/**< X.509 certificate version
								v3 */

#define X509_ASN1_UNUSED_BITS_SIZE_IN_BYTE		(1U)	/**< No of unused bits size in
								byte */
#define X509_ASN1_TAG_LEN_VAL_MIN_SIZE			(3U)	/**< Minimum size of ASN.1
								TLV(Tag, Len, Value) pair */
#define X509_ASN1_LEN_VAL_MIN_SIZE			(2U)	/**< Minimum size of ASN.1
								LV(Len, Value) pair */
#define X509_ASN1_TAG_LEN				(1U)	/**< ASN.1 tag length */
#define X509_ASN1_BIT7_MASK				(0x80U)	/**< Mask to get bit 7 */
#define X509_ASN1_NO_OF_BITS_IN_BYTE			(8U)	/**< Number of bits in a byte */
#define X509_ASN1_YEAR_LEN_GEN_BYTES			(4U)	/**< Year length for
								Generalized(YYYY) format */
#define X509_ASN1_YEAR_LEN_UTC_BYTES			(2U)	/**< Year length for UTC(YY)
								format */
#define X509_ASN1_TIME_WITH_OUT_ZERO_TIME_OFFSET(n)	(10U + (n))	/**< Length value without
									trailing 'Z' (YYDDMMHHMMSSZ)
									/(YYYYDDMMHHMMSS) */
#define X509_ASN1_TIME_WITH_ZERO_TIME_OFFSET(n)		(11U + (n))	/**< Length value with
									trailing 'Z' (YYDDMMHHMMSSZ)
									/(YYYYDDMMHHMMSSZ) */
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
								time field (e.g month, day, hour)*/
#define X509_ASN1_ZERO_TIME_OFFSET_LEN			(1U)	/**< Length of zero time offset */
#define X509_ASN1_ZERO_TIME_OFFSET_VAL			(0x5A)	/**< Value of zero time offset */
#define X509_ASN1_MAX_UNUSED_BITS			(7U)	/**< Maximum unused bits */

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
#define X509_ASN1_TAG_OPTIONAL_PARAM_0_CONSTRUCTED_TAG	(0xA0U)	/**< ASN.1 optional parameter 0
								constructed tag */
#define X509_ASN1_TAG_OPTIONAL_PARAM_3_CONSTRUCTED_TAG	(0xA3U)	/**< ASN.1 optional parameter 3
								constructed tag */
/************************************ Function Prototypes ****************************************/
s32 X509_ParseCertificate(u64 X509CertAddr, u32 Size, X509_CertInfo *CertInfo);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* X509_CERT_H_ */
/** @} */
