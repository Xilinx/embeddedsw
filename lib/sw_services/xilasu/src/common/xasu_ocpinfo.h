/**************************************************************************************************
* Copyright (c) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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
#define XASU_OCP_UDE_CHALLENGE_REQ_CMD_ID	(4U)	/**< Command ID for UDE challenge request */
#define XASU_OCP_RESERVED_CMD_ID		(5U)	/**< Reserved command ID (previously used for
							UDE private keys encryption) */
#define XASU_OCP_GET_HUK_CMD_ID			(6U)	/**< Command ID for Hardware Unique Key generation */
#define XASU_OCP_GET_USER_X509_CERT_CMD_ID	(7U)	/**< Command ID for user x509 certificate
							generation using key vault keys */
#define XASU_OCP_MAX_CMDS			(8U)	/**< Maximum number of commands
											  * supported by OCP module */

#define XASU_OCP_UDE_DEVICE_ID_SIZE_IN_WORDS	(12U)	/**< Device ID size in words */
#define XASU_OCP_UDE_NONCE_SIZE_IN_WORDS	(8U)	/**< Nonce size in words */
#define XASU_OCP_UDE_MEASUREMENT_SIZE_IN_WORDS	(12U)	/**< Measurement size in words */
#define XASU_OCP_UDE_DEVICE_ID_SIZE_IN_BYTES	((XASU_OCP_UDE_DEVICE_ID_SIZE_IN_WORDS) << 2U)
							/**< Device ID size in bytes */
#define XASU_OCP_UDE_NONCE_SIZE_IN_BYTES	((XASU_OCP_UDE_NONCE_SIZE_IN_WORDS) << 2U)
							/**< Nonce size in bytes */
#define XASU_OCP_UDE_MEASUREMENT_SIZE_IN_BYTES	((XASU_OCP_UDE_MEASUREMENT_SIZE_IN_WORDS) << 2U)
							/**< Measurement size in bytes */
#define XASU_OCP_ECC_P384_SIZE_IN_WORDS		(12U)	/**< P-384 size in words */
#define XASU_OCP_UDE_KEY_SIZE_IN_BYTES		(48U)	/**< UDE key size in bytes */
#define XASU_OCP_UDE_PUB_KEY_SIZE_IN_BYTES	(96U)	/**< UDE public key size in bytes
							(ECC P-384 X || Y) */
#define XASU_OCP_UDE_IV_SIZE_IN_BYTES		(12U)	/**< UDE Iv size in bytes */
#define XASU_OCP_UDE_USER_KEY_0_ID		(0U)	/**< UDE User Key 0 ID */
#define XASU_OCP_UDE_USER_KEY_1_ID		(1U)	/**< UDE User Key 1 ID */
#define XASU_OCP_UDE_USER_KEY_2_ID		(2U)	/**< UDE User Key 2 ID */

#define XASU_OCP_HUK_SIZE_IN_BYTES		(32U)	/**< Hardware Unique Key size in bytes (256-bit AES key) */
#define XASU_X509_MAX_SIZE_IN_BYTES		(2000U)	/**< X.509 certificate max size in bytes */

/**
 * Maximum lengths of the text fields of the user X.509 certificate configuration. The
 * organization name and common name limits are as per the upper bounds defined by X.520.
 */
#define XASU_X509_ORG_NAME_MAX_LEN		(64U)	/**< Maximum length of the organization
							name */
#define XASU_X509_COMMON_NAME_MAX_LEN		(64U)	/**< Maximum length of the common name */

/**
 * Bounds of the validity period fields of the user X.509 certificate configuration. The year
 * lower bound is the earliest year that RFC 5280 permits a certificate validity date to hold.
 */
#define XASU_X509_MIN_YEAR			(1950U)	/**< Earliest permitted year */
#define XASU_X509_MAX_YEAR			(9999U)	/**< Latest permitted year */

/**
 * Key usage bit positions of the X.509 Key Usage extension as defined by RFC 5280. The user
 * supplies the key usage as a bit wise OR of these masks.
 */
#define XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE	(0x0001U)	/**< Digital Signature */
#define XASU_X509_KEY_USAGE_NON_REPUDIATION	(0x0002U)	/**< Non Repudiation */
#define XASU_X509_KEY_USAGE_KEY_ENCIPHERMENT	(0x0004U)	/**< Key Encipherment */
#define XASU_X509_KEY_USAGE_DATA_ENCIPHERMENT	(0x0008U)	/**< Data Encipherment */
#define XASU_X509_KEY_USAGE_KEY_AGREEMENT	(0x0010U)	/**< Key Agreement */
#define XASU_X509_KEY_USAGE_KEY_CERT_SIGN	(0x0020U)	/**< Key Certificate Sign */
#define XASU_X509_KEY_USAGE_CRL_SIGN		(0x0040U)	/**< CRL Sign */
#define XASU_X509_KEY_USAGE_ENCIPHER_ONLY	(0x0080U)	/**< Encipher Only */
#define XASU_X509_KEY_USAGE_DECIPHER_ONLY	(0x0100U)	/**< Decipher Only */
#define XASU_X509_KEY_USAGE_MAX_MASK		(XASU_X509_KEY_USAGE_DECIPHER_ONLY)
								/**< Mask of the highest numbered
								key usage bit */
#define XASU_X509_ECC_KEY_USAGE_MASK		(XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE | \
						 XASU_X509_KEY_USAGE_NON_REPUDIATION | \
						 XASU_X509_KEY_USAGE_DATA_ENCIPHERMENT | \
						 XASU_X509_KEY_USAGE_KEY_AGREEMENT | \
						 XASU_X509_KEY_USAGE_KEY_CERT_SIGN | \
						 XASU_X509_KEY_USAGE_CRL_SIGN | \
						 XASU_X509_KEY_USAGE_ENCIPHER_ONLY | \
						 XASU_X509_KEY_USAGE_DECIPHER_ONLY)
						 /**< Mask of the key usage bits an ECC key certificate
						 request may carry. keyEncipherment is excluded as it
						 applies to RSA key transport, not ECC. */

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
	u64 CertActualSizeAddr;	/**< Address at which the actual size of the generated certificate
					is written */
	u32 CertBufLen;		/**< Certificate buffer length */
	u32 DevKeySel;		/**< Device key type */
} XAsu_OcpCertParams;

/**
 * This structure contains organization name and common name of a user X.509 certificate.
 */
typedef struct {
	u32 OrgNameLen;					/**< Length of the organization name */
	u32 CommonNameLen;				/**< Length of the common name */
	u8 OrgName[XASU_X509_ORG_NAME_MAX_LEN];		/**< Organization name */
	u8 CommonName[XASU_X509_COMMON_NAME_MAX_LEN];	/**< Common name */
} XAsu_X509Name;

/**
 * This structure contains a point in time of the validity period of a user X.509 certificate.
 */
typedef struct {
	u16 Year;	/**< Year, in the range XASU_X509_MIN_YEAR to XASU_X509_MAX_YEAR */
	u8 Month;	/**< Month of the year, in the range 1 to 12 */
	u8 Day;		/**< Day of the month, in the range 1 to the number of days in the month */
	u8 Hour;	/**< Hour of the day, in the range 0 to 23 */
	u8 Minute;	/**< Minute of the hour, in the range 0 to 59 */
	u8 Second;	/**< Second of the minute, in the range 0 to 59 */
	u8 Reserved; /**< Explicit padding to ensure consistent struct size across architectures */
} XAsu_X509Time;

/**
 * This structure contains the validity period of a user X.509 certificate.
 */
typedef struct {
	XAsu_X509Time NotBefore;	/**< Start time of the validity period */
	XAsu_X509Time NotAfter;		/**< End time of the validity period */
} XAsu_X509Validity;

/**
 * This structure contains the user supplied certificate fields.
 */
typedef struct {
	XAsu_X509Name Subject;			/**< Subject distinguished name */
	XAsu_X509Name Issuer;			/**< Issuer distinguished name, which the user is
						required to supply */
	XAsu_X509Validity Validity;		/**< Validity period */
	u32 KeyUsage;				/**< Bit wise OR of the XASU_X509_KEY_USAGE_*
						masks to be encoded in the Key Usage
						extension */
} XAsu_X509CertFields;

/**
 * This structure contains the user supplied configuration of a user X.509 certificate.
 */
typedef struct {
	XAsu_X509CertFields CertCfg;	/**< Certificate fields */
	u32 SubjectPubKeyId;		/**< Key vault key ID of the public key which is certified
					as the subject public key */
	u32 IssuerPubKeyId;		/**< Key vault key ID of the issuer public key, from which
					the authority key identifier extension is derived */
	u32 IssuerPvtKeyId;		/**< Key vault key ID of the private key which signs the
					certificate */
} XAsu_X509UserCertCfg;

/**
 * This structure contains the parameters of a user X.509 certificate request.
 */
typedef struct {
	u64 UserCertCfgAddr;	/**< Address of the XAsu_X509UserCertCfg structure */
	u64 CertBufAddr;	/**< Certificate buffer address */
	u64 CertActualSizeAddr;	/**< Address at which the actual size of the generated certificate
					is written */
	u32 CertBufLen;		/**< Certificate buffer length */
	u8 Reserved[4];		/**< Explicit padding to keep the structure size consistent across
					architectures */
} XAsu_OcpUserCertParams;

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
 * This structure contains information related to UDE challenge parameters buffers.
 */
typedef struct {
	u32 DeviceId[XASU_OCP_UDE_DEVICE_ID_SIZE_IN_WORDS]; /**< Device ID buffer */
	u32 Nonce[XASU_OCP_UDE_NONCE_SIZE_IN_WORDS]; /**< Nonce buffer */
	u32 Measurement[XASU_OCP_UDE_MEASUREMENT_SIZE_IN_WORDS]; /**< Measurement buffer */
} XAsu_OcpUde;

/** UDE response */
typedef struct {
	XAsu_OcpUde Ude;					/**< UDE */
	u32 UdeSignatureR[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Signature comp R */
	u32 UdeSignatureS[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Signature comp S */
	u32 UdePublicKeyX[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Public key component X */
	u32 UdePublicKeyY[XASU_OCP_ECC_P384_SIZE_IN_WORDS];	/**< Public key component Y */
} XAsu_OcpUdeResponse;

/** This structure contains information related to UDE challenge parameters. */
typedef struct {
	u64 NonceAddr;	/**< Address of the Nonce buffer */
	u64 OcpUdeResponseAddr;	/**< Address of the UDE response structure */
} XAsu_OcpUdeParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_OCPINFO_H_ */
/** @} */
