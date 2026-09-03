/**************************************************************************************************
* Copyright (c) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xocp.h
 *
 * This file contains declarations for xocp.c file.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/16/25 Initial release
 *       yog  08/21/25 Added OCP-UDE support
 *       rmv  02/18/26 Increase value of maximum OCP subsystems
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xocp_server_apis OCP server APIs
* @{
*/
#ifndef XOCP_H_
#define XOCP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "x509_cert.h"
#include "xasu_eccinfo.h"
#include "xasu_ocpinfo.h"
#include "xasufw_dma.h"
#include "xil_types.h"

#ifdef XASU_OCP_ENABLE
/************************************ Constant Definitions ***************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XOCP_MAX_USER_OCP_SUBSYS	(6U)	/**< Maximum number of user OCP subsystems */
#define XOCP_USER_SUBSYS_START_INDEX	(1U)	/**< Start index for OCP subsystem */

#define XOCP_PERSONAL_STRING_LEN		(48U)	/**< Personalised string length */

#define XASUFW_SUBSYTEM_ID		(0x1C000002U)	/**< ASUFW subsystem ID */

/************************************** Type Definitions *****************************************/
/**
 * This structure is used to store generated device keys.
 */
typedef struct {
	u32 PvtKeyId;			/**< Key ID of the private key in the ASU key vault */
	u32 PubKeyId;			/**< Key ID of the public key in the ASU key vault */
	u8 IsKeyReady;			/**< Indicates if the device key is ready or not */
} XOcp_DeviceKeys;

/**
 * This structure contains information about seed, personalized string and ECC private key.
 */
typedef struct {
	u32 CdiAddr;		/**< Address to the CDI buffer. */
	u32 CdiLength;		/**< CDI length. */
	u32 PerStringAddr;	/**< Personalized string. */
	u32 PvtKeyAddr;		/**< Key output address. */
} XOcp_PrivateKeyGen;

/**
 * This structure contains information about Subsystems data.
 */
typedef struct {
	u32 SubSystemID;				/**< Subsystem Id */
	u8 PersonalString[XOCP_PERSONAL_STRING_LEN];	/**< Subsystem personalized string */
	u32 IsPersonalStringAvailable;			/**< Flag to indicate if personalized string
							is given by user */
	X509_UserCfg UserCfg;				/**< User configurations */
} XOcp_SubsystemData;

/**
 * This structure contains information related to CDO data.
 */
typedef struct {
	u32 NumSubsys;						/**< Total number of subsystems */
	XOcp_SubsystemData AsuSubsysData;			/**< ASU Subsystem data */
	XOcp_SubsystemData SubsysData[XOCP_MAX_USER_OCP_SUBSYS];	/**< Subsystem data */
} XOcp_CdoData;

/**
 * This structure contains X.509 certificate related data.
 */
typedef struct {
	u32 DevKeyType;			/**< Device key type */
	u64 CertAddr;			/**< X.509 certificate address */
	u32 CertMaxSize;		/**< Maximum size of X.509 certificate */
} XOcp_CertData;

/**
 * This structure contains the working data of an X.509 certificate generation request. It is
 * reserved in the crypto algorithm data block instead of the caller's stack. The caller obtains it
 * through XOcp_GetUserCertData(), clears it, populates the fields it supplies, and zeroizes it
 * when the request completes.
 */
typedef struct {
	u8 CertBuf[X509_CERTIFICATE_MAX_SIZE_IN_BYTES];	/**< Generated user certificate */
	X509_UserCfg DerCfg;		/**< DER encoded user certificate configuration */
	XAsu_X509UserCertCfg UserCfg;	/**< User supplied certificate configuration */
	X509_Config CertCfg;		/**< Certificate generation configuration */
	XAsu_EccKeyObject SubjectPubKeyObj;	/**< Subject public key of the certificate */
	XAsu_EccKeyObject IssuerPubKeyObj;	/**< Issuer public key verifying the signature */
	XAsu_EccKeyObject IssuerPvtKeyObj;	/**< Issuer private key signing the certificate */
} XOcp_UserCertData;

/**
 * This structure contains the caller supplied key material and configuration used to generate an
 * X.509 certificate. The issuer key pair and the subject public key are supplied directly by the
 * caller, which allows the same generator to be used for user certificates (keys fetched from the
 * key vault) and for the UDE self-signed CA certificate (freshly generated keys).
 */
typedef struct {
	const XAsu_X509CertFields *CertFields;	/**< Subject and issuer distinguished names,
						validity period and key usage */
	const u8 *SubjectPubKey;		/**< Subject public key (uncompressed X || Y) */
	const u8 *IssuerPubKey;			/**< Issuer public key (uncompressed X || Y). Equal
						to SubjectPubKey for a self-signed certificate */
	const u8 *IssuerPvtKey;			/**< Issuer private key that signs the certificate */
	u64 CertBufAddr;			/**< Destination address for the DER encoded
						certificate */
	u32 SubjectPubKeyLen;			/**< Subject public key length in bytes */
	u32 IssuerPubKeyLen;			/**< Issuer public key length in bytes */
	u32 CertBufMaxSize;			/**< Size in bytes of the destination buffer */
	u32 IsSelfSigned;			/**< XASU_TRUE if the certificate is self-signed
						(issuer name and key equal the subject) */
	u32 IsCaCert;				/**< XASU_TRUE to emit the CA BasicConstraints
						extension (CA=TRUE, pathLenConstraint=0) */
} XOcp_X509CertGenParams;

/************************************ Function Prototypes ****************************************/
s32 XOcp_GenerateDeviceKeys(XAsufw_Dma *DmaPtr, u32 EventMask);
s32 XOcp_GetX509Cert(u32 SubsystemId, const XAsu_OcpCertParams *OcpCertParam, void *PlatData,
		     u8 IsCsr);
s32 XOcp_GetUserX509Cert(const XOcp_X509CertGenParams *CertGenParams, u32 *CertSize,
			 void *PlatData);
XOcp_UserCertData *XOcp_GetUserCertData(void);
s32 XOcp_AttestWithDevAk(XAsufw_Dma *DmaPtr, const XAsu_OcpDevAkAttest *OcpAttestParam,
			 u32 SubsystemId);
XOcp_DeviceKeys* XOcp_GetDevIk(void);
s32 XOcp_GetDevIkPublicKey(const u8 **PubKeyPtr);
s32 XOcp_GetOcpEventMaskFromPlm(u32 *EventMask);
s32 XOcp_GetHuk(u8 *HukBuf);

/************************************ Variable Definitions ***************************************/

#endif /* XASU_OCP_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* XOCP_H_ */
/** @} */
