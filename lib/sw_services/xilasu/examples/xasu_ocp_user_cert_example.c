/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_ocp_user_cert_example.c
 * @addtogroup Overview
 * @{
 *
 * This example shows how an application stores its own ECC keys in a key vault and then has the
 * ASU issue an X.509 certificate for them. The private keys are handed to the vault once and the
 * certificate is produced from key IDs alone, so no private key is exposed at the time of signing.
 *
 * The example performs the following steps:
 *  - Creates a key vault owned by the calling subsystem.
 *  - Stores the subject ECC key pair, which is the identity that the certificate describes. The
 *    public key is the one that gets certified, and the use cases it is stored with have to cover
 *    the key usage requested in the certificate configuration.
 *  - Stores the issuer ECC key pair, which is the authority that signs the certificate. Its
 *    private key produces the signature and its public key yields the authority key identifier.
 *  - Fills in the certificate configuration: subject and issuer names, validity period, key usage
 *    and the vault key IDs of the keys involved.
 *  - Requests the certificate, which the ASU builds, signs and returns in DER encoding.
 *
 * All keys are NIST P-384 and every value is built into this file, so the example is self
 * contained and repeatable. Each key is printed as it is stored and the generated certificate is
 * printed as a hex string, which is everything a host needs to check the result: converting the
 * certificate to DER and verifying its signature against the issuer public key confirms that the
 * ASU signed the expected contents. Note that the certificate configuration names the issuer by
 * organization and common name only, so a full "openssl verify" chain check additionally needs a
 * trust anchor built from that same name and the issuer public key.
 *
 * Procedure to link and compile the example for the default ddr less designs
 * ------------------------------------------------------------------------------------------------
 * Data shared between the client running on A78/R52/PL and the server running on ASU must be placed
 * in an area accessible to both. Place the shared ".data" elements in a shared memory section, as
 * described in the other xilasu examples. To keep things simple, the cache is disabled here using
 * the XASU_CACHE_DISABLE macro.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rv   08/14/26 Initial release
 *       kp   08/22/26 Used composite key ID in key metadata.
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/
#include "xasu_client.h"
#include "xasu_keymanager.h"
#include "xasu_keymanager_common.h"
#include "xasu_ocp.h"
#include "xasu_eccinfo.h"
#include "xil_cache.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#define XASU_CACHE_DISABLE

#define XOCP_USER_CERT_X509_BUF_SIZE		(2048U)	/**< Certificate buffer size */
#define XOCP_USER_CERT_CURVE_LEN		XASU_ECC_P384_PVT_KEY_SIZE_IN_BYTES
						/**< NIST P-384 private key length */
#define XOCP_USER_CERT_PUB_KEY_LEN		XASU_ECC_P384_PUB_KEY_SIZE_IN_BYTES
						/**< NIST P-384 public key length */
#define XOCP_USER_CERT_VAULT_CAPACITY		(4U)	/**< Keys reserved in each subvault */
#define XOCP_USER_CERT_ACCESS_RIGHTS		(0xFFFFU) /**< Vault access rights */

#define XOCP_USER_CERT_SUBJECT_ORG_NAME		"AMD"	/**< Subject organization name */
#define XOCP_USER_CERT_SUBJECT_COMMON_NAME	"User Device"	/**< Subject common name */
#define XOCP_USER_CERT_ISSUER_ORG_NAME		"AMD INDIA"	/**< Issuer organization name */
#define XOCP_USER_CERT_ISSUER_COMMON_NAME	"AMD IN"	/**< Issuer common name */

#define XOCP_USER_CERT_NOT_BEFORE_YEAR		(2026U)	/**< Validity start year */
#define XOCP_USER_CERT_NOT_BEFORE_MONTH		(8U)	/**< Validity start month */
#define XOCP_USER_CERT_NOT_BEFORE_DAY		(14U)	/**< Validity start day */
#define XOCP_USER_CERT_NOT_AFTER_YEAR		(2036U)	/**< Validity end year */
#define XOCP_USER_CERT_NOT_AFTER_MONTH		(8U)	/**< Validity end month */
#define XOCP_USER_CERT_NOT_AFTER_DAY		(14U)	/**< Validity end day */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsu_OcpUserCertCallBackRef(void *CallBackRef, u32 Status);
static s32 XAsu_OcpUserCertWaitForResponse(void);
static s32 XAsu_OcpUserCertCreateVault(u32 *VaultId);
static s32 XAsu_OcpUserCertStoreEccPvtKey(u32 VaultId, const u8 *Key, u8 KeyUseCase, u32 *KeyId);
static s32 XAsu_OcpUserCertStoreEccPubKey(u32 VaultId, const u8 *Key, u8 KeyUseCase, u32 *KeyId);
static s32 XAsu_OcpUserCertFillUserCertCfg(u32 SubjectPubKeyId, u32 IssuerPubKeyId,
					   u32 IssuerPvtKeyId);
static s32 XAsu_OcpUserCertCopyField(u8 *Dest, u32 DestSize, const char *Src, u32 *DestLen);
static s32 XAsu_OcpUserCertGenerate(void);
static void XAsu_OcpUserCertPrintData(const u8 *Data, u32 DataLen, const char *BufName);

/************************************ Variable Definitions ***************************************/
/** Issuer ECC private key, which the ASU uses to sign the generated certificate. */
static const u8 IssuerEccPrivKey[XOCP_USER_CERT_CURVE_LEN]
	__attribute__((section(".data.IssuerEccPrivKey"))) = {
	0xD2U, 0x90U, 0x29U, 0xD0U, 0xD4U, 0x4EU, 0x3DU, 0xFEU,
	0xB0U, 0xADU, 0x56U, 0xFEU, 0x79U, 0xA9U, 0xA8U, 0xB3U,
	0x62U, 0x55U, 0x74U, 0x0DU, 0x34U, 0x40U, 0xA5U, 0x21U,
	0xF8U, 0x4DU, 0x6AU, 0xDDU, 0x25U, 0x5EU, 0xBAU, 0x26U,
	0xE4U, 0x0DU, 0xC9U, 0x05U, 0xAEU, 0xB1U, 0x8DU, 0xB7U,
	0x3BU, 0xC7U, 0x59U, 0xABU, 0x42U, 0xCDU, 0x00U, 0xA5U
};

/** Issuer ECC public key, from which the authority key identifier extension is derived. */
static const u8 IssuerEccPubKey[XOCP_USER_CERT_PUB_KEY_LEN]
	__attribute__((section(".data.IssuerEccPubKey"))) = {
	0x86U, 0xAFU, 0x67U, 0xF7U, 0xA1U, 0x29U, 0xF7U, 0xB0U,
	0x82U, 0xB0U, 0x41U, 0xEBU, 0x80U, 0xD0U, 0xF1U, 0x5FU,
	0x53U, 0xD3U, 0xCBU, 0xB4U, 0x51U, 0x26U, 0x68U, 0x47U,
	0xCCU, 0x90U, 0xB6U, 0x1FU, 0x26U, 0x37U, 0xEFU, 0x89U,
	0x86U, 0xEAU, 0x91U, 0xC3U, 0xDBU, 0xF2U, 0x80U, 0x55U,
	0xA3U, 0x9BU, 0x7DU, 0x7CU, 0x92U, 0x0FU, 0x56U, 0xD4U,
	0x60U, 0xEDU, 0xF5U, 0x3EU, 0x0AU, 0x3EU, 0xECU, 0x7DU,
	0xC1U, 0x1FU, 0xE7U, 0xF1U, 0x6BU, 0x8CU, 0x3CU, 0x5BU,
	0x0AU, 0xABU, 0x97U, 0xBDU, 0xC2U, 0xD4U, 0x18U, 0x96U,
	0xC8U, 0xB1U, 0x6AU, 0x17U, 0x21U, 0xB8U, 0x69U, 0x58U,
	0x2EU, 0x30U, 0x9BU, 0xCAU, 0x06U, 0xEAU, 0xE7U, 0xDBU,
	0x14U, 0x6BU, 0x1BU, 0xAAU, 0x0BU, 0xD0U, 0x48U, 0xF7U
};

/**
 * Subject ECC private key. Certificate generation needs only the subject public key, so this key
 * is stored to keep the full key pair of the certified identity under vault control.
 */
static const u8 SubjectEccPrivKey[XOCP_USER_CERT_CURVE_LEN]
	__attribute__((section(".data.SubjectEccPrivKey"))) = {
	0x62U, 0xBDU, 0x65U, 0x34U, 0x74U, 0x72U, 0x27U, 0x4CU,
	0xBEU, 0x9EU, 0x45U, 0xFDU, 0x4DU, 0x68U, 0xACU, 0x78U,
	0xF2U, 0x0AU, 0x46U, 0x4CU, 0x78U, 0xDDU, 0x49U, 0xA2U,
	0xB2U, 0x6DU, 0x4EU, 0x8AU, 0xDBU, 0x52U, 0xA1U, 0x88U,
	0xF4U, 0xCFU, 0xA7U, 0x20U, 0xD4U, 0x4FU, 0x71U, 0x02U,
	0xD7U, 0xC5U, 0x23U, 0x09U, 0x9FU, 0x90U, 0xE6U, 0x8EU
};

/** Subject ECC public key, which the generated certificate binds to the subject name. */
static const u8 SubjectEccPubKey[XOCP_USER_CERT_PUB_KEY_LEN]
	__attribute__((section(".data.SubjectEccPubKey"))) = {
	0x1FU, 0x0DU, 0xEAU, 0xEEU, 0x86U, 0x4BU, 0x62U, 0x18U,
	0x95U, 0x88U, 0x98U, 0x44U, 0x81U, 0xABU, 0x15U, 0x12U,
	0xEFU, 0x49U, 0xB3U, 0x47U, 0x15U, 0x59U, 0xF4U, 0x33U,
	0xE8U, 0x44U, 0x23U, 0x1BU, 0x27U, 0x1CU, 0x26U, 0x0AU,
	0x12U, 0xECU, 0x68U, 0x13U, 0x92U, 0x64U, 0xE6U, 0xAAU,
	0x78U, 0xD2U, 0xB0U, 0xB0U, 0x4AU, 0x11U, 0x28U, 0x8EU,
	0xDBU, 0x44U, 0x47U, 0x29U, 0xBEU, 0xDBU, 0x5DU, 0xB1U,
	0x0FU, 0xACU, 0x97U, 0xC5U, 0xD0U, 0x2BU, 0x0EU, 0xF4U,
	0xB2U, 0x82U, 0x57U, 0xDAU, 0x78U, 0x39U, 0x38U, 0x7FU,
	0x49U, 0xBCU, 0x32U, 0x87U, 0x3CU, 0xC6U, 0xC7U, 0x0DU,
	0xABU, 0xC6U, 0xF4U, 0x4FU, 0xAEU, 0x24U, 0x3AU, 0x6CU,
	0xC6U, 0x93U, 0xD3U, 0x29U, 0x76U, 0xDFU, 0xBDU, 0xD2U
};

/**< Buffer which receives the DER encoded certificate. */
static u8 UserX509Cert[XOCP_USER_CERT_X509_BUF_SIZE] __attribute__((section(".data.UserX509Cert")));
/**< Actual size of the generated certificate. */
static u32 UserX509CertActualSize __attribute__((section(".data.UserX509CertActualSize")));
/**< Certificate configuration passed to the ASU. */
static XAsu_X509UserCertCfg UserCertCfg __attribute__((aligned(64U)))
	__attribute__((section(".data.UserCertCfg")));
/**< Key vault created by this example. */
static u32 UserVaultId __attribute__((section(".data.UserVaultId")));
/**< Key ID of the subject public key, which the certificate certifies. */
static u32 SubjectPubKeyId __attribute__((section(".data.SubjectPubKeyId")));
/**< Key ID of the subject private key. */
static u32 SubjectPvtKeyId __attribute__((section(".data.SubjectPvtKeyId")));
/**< Key ID of the issuer public key, which yields the authority key identifier. */
static u32 IssuerPubKeyId __attribute__((section(".data.IssuerPubKeyId")));
/**< Key ID of the issuer private key, which signs the certificate. */
static u32 IssuerPvtKeyId __attribute__((section(".data.IssuerPvtKeyId")));

static volatile u8 Notify = 0U;			/**< To notify the call back from client library */
static volatile u32 ErrorStatus = XST_FAILURE;	/**< Variable holds the status of the operation
						from client library and gets updated during
						callback. */

/************************************ Function Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	Main function to run the OCP user X.509 certificate example. It creates a key
 *		vault, stores the subject and issuer ECC key pairs in it, fills in the certificate
 *		configuration and requests the certificate.
 *
 * @return
 *	- XST_SUCCESS, if the certificate is generated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XST_FAILURE;
	XMailbox MailboxInstance;

#ifdef XASU_CACHE_DISABLE
	Xil_DCacheDisable();
#endif

	Status = (s32)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Mailbox initialize failed: %08x\r\n", Status);
		goto END;
	}

	Status = XAsu_ClientInit(&MailboxInstance);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Client initialize failed: %08x\r\n", Status);
		goto END;
	}

	/** Create the key vault which holds the subject and issuer key pairs. */
	Status = XAsu_OcpUserCertCreateVault(&UserVaultId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XAsu_OcpUserCertPrintData(SubjectEccPrivKey, XOCP_USER_CERT_CURVE_LEN,
				  "SUBJECT ECC PRIVATE KEY");

	/** Store the private key of the subject. */
	Status = XAsu_OcpUserCertStoreEccPvtKey(UserVaultId, SubjectEccPrivKey,
		(u8)XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE, &SubjectPvtKeyId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Subject ECC private key storage failed with Status = %08x\r\n",
			      Status);
		goto END;
	}
	XilAsu_Printf("Successfully stored subject ECC private key with KeyId = %08x\r\n",
		      SubjectPvtKeyId);

	XAsu_OcpUserCertPrintData(SubjectEccPubKey, XOCP_USER_CERT_PUB_KEY_LEN,
				  "SUBJECT ECC PUBLIC KEY");

	/** Store the public key of the subject, which the certificate certifies. */
	Status = XAsu_OcpUserCertStoreEccPubKey(UserVaultId, SubjectEccPubKey,
		(u8)(XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE |
		     XASU_KEYMANAGER_ECC_PUB_KEY_AUTH_ENC_USE_CASE), &SubjectPubKeyId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Subject ECC public key storage failed with Status = %08x\r\n",
			      Status);
		goto END;
	}
	XilAsu_Printf("Successfully stored subject ECC public key with KeyId = %08x\r\n",
		      SubjectPubKeyId);

	XAsu_OcpUserCertPrintData(IssuerEccPrivKey, XOCP_USER_CERT_CURVE_LEN,
				  "ISSUER ECC PRIVATE KEY");

	/** Store the private key of the issuer, which signs the certificate. */
	Status = XAsu_OcpUserCertStoreEccPvtKey(UserVaultId, IssuerEccPrivKey,
		(u8)XASU_KEYMANAGER_ECC_PVT_SIGN_GEN_USE_CASE, &IssuerPvtKeyId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Issuer ECC private key storage failed with Status = %08x\r\n",
			      Status);
		goto END;
	}
	XilAsu_Printf("Successfully stored issuer ECC private key with KeyId = %08x\r\n",
		      IssuerPvtKeyId);

	XAsu_OcpUserCertPrintData(IssuerEccPubKey, XOCP_USER_CERT_PUB_KEY_LEN,
				  "ISSUER ECC PUBLIC KEY");

	/** Store the public key of the issuer, which yields the authority key identifier. */
	Status = XAsu_OcpUserCertStoreEccPubKey(UserVaultId, IssuerEccPubKey,
		(u8)XASU_KEYMANAGER_ECC_PUB_SIGN_VER_USE_CASE, &IssuerPubKeyId);
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("Issuer ECC public key storage failed with Status = %08x\r\n",
			      Status);
		goto END;
	}
	XilAsu_Printf("Successfully stored issuer ECC public key with KeyId = %08x\r\n",
		      IssuerPubKeyId);

	/** Fill user certificate configuration. */
	Status = XAsu_OcpUserCertFillUserCertCfg(SubjectPubKeyId, IssuerPubKeyId, IssuerPvtKeyId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Generate the user X.509 certificate. */
	Status = XAsu_OcpUserCertGenerate();

END:
	if (Status == XST_SUCCESS) {
		XilAsu_Printf("Successfully ran OCP user certificate example\r\n");
	} else {
		XilAsu_Printf("OCP user certificate example failed\r\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates a key vault owned by the calling subsystem, with capacity for
 *		the ECC private, ECC public and X.509 keys which the example stores in it.
 *
 * @param	VaultId		Pointer to store the ID of the created key vault.
 *
 * @return
 *	- XST_SUCCESS, if the key vault is created successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertCreateVault(u32 *VaultId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_KeyManagerSubVaultParams VaultParams = {0U};

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_OcpUserCertCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	VaultParams.VaultIdAddr = (u64)(UINTPTR)VaultId;
	VaultParams.ECCPvtKeyVaultCapacity = XOCP_USER_CERT_VAULT_CAPACITY;
	VaultParams.ECCPubKeyVaultCapacity = XOCP_USER_CERT_VAULT_CAPACITY;
	VaultParams.AccessRights = XOCP_USER_CERT_ACCESS_RIGHTS;
	VaultParams.Restrictions = XASU_KEYMANAGER_NON_EXPORTABLE_VAULT;

	ErrorStatus = XST_FAILURE;
	Status = XAsu_KmCreateKeyVault(&ClientParams, &VaultParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertWaitForResponse();

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("User key vault creation failed with Status = %08x\r\n", Status);
	} else {
		XilAsu_Printf("Successfully created user key vault with VaultId = %u\r\n",
			      *VaultId);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function stores a NIST P-384 ECC private key in the given key vault.
 *
 * @param	VaultId		ID of the key vault which holds the key.
 * @param	Key		Pointer to the private key.
 * @param	KeyUseCase	Use cases the key is permitted to be used for.
 * @param	KeyId		Pointer to store the key ID assigned by the key vault.
 *
 * @return
 *	- XST_SUCCESS, if the key is stored successfully.
 *	- The error code reported by the server, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertStoreEccPvtKey(u32 VaultId, const u8 *Key, u8 KeyUseCase, u32 *KeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_KeyManagerParams KeyParams = {0U};

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_OcpUserCertCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	KeyParams.KeyMetadata.CompositeKeyId =
			XASU_KM_COMPOSE_KEY_ID((u8)VaultId, (u8)XASU_ECC_PVT_SUBVAULT_ID, 0U);
	KeyParams.KeyMetadata.KeyAttributes = (u8)XASU_ECC_NIST_P384;
	KeyParams.KeyMetadata.KeyUseCase = KeyUseCase;
	KeyParams.KeyMetadata.Length = XOCP_USER_CERT_CURVE_LEN;
	KeyParams.KeyMetadata.UsageCount = XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE;
	KeyParams.KeyObjectAddr = (u64)(UINTPTR)Key;
	KeyParams.KeyIdAddr = (u64)(UINTPTR)KeyId;

	ErrorStatus = XST_FAILURE;
	Status = XAsu_KmStoreKey(&ClientParams, &KeyParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertWaitForResponse();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function stores a NIST P-384 ECC public key in the given key vault. The use
 *		cases the key is stored with have to cover the key usage requested in the
 *		certificate configuration.
 *
 * @param	VaultId		ID of the key vault which holds the key.
 * @param	Key		Pointer to the public key.
 * @param	KeyUseCase	Use cases the key is permitted to be used for.
 * @param	KeyId		Pointer to store the key ID assigned by the key vault.
 *
 * @return
 *	- XST_SUCCESS, if the key is stored successfully.
 *	- The error code reported by the server, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertStoreEccPubKey(u32 VaultId, const u8 *Key, u8 KeyUseCase, u32 *KeyId)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_KeyManagerParams KeyParams = {0U};

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_OcpUserCertCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;

	KeyParams.KeyMetadata.CompositeKeyId =
			XASU_KM_COMPOSE_KEY_ID((u8)VaultId, (u8)XASU_ECC_PUB_SUBVAULT_ID, 0U);
	KeyParams.KeyMetadata.KeyAttributes = (u8)XASU_ECC_NIST_P384;
	KeyParams.KeyMetadata.KeyUseCase = KeyUseCase;
	KeyParams.KeyMetadata.Length = XOCP_USER_CERT_CURVE_LEN;
	KeyParams.KeyMetadata.UsageCount = XASU_KM_USAGE_COUNT_NON_DEPLETING_VALUE;
	KeyParams.KeyObjectAddr = (u64)(UINTPTR)Key;
	KeyParams.KeyIdAddr = (u64)(UINTPTR)KeyId;

	ErrorStatus = XST_FAILURE;
	Status = XAsu_KmStoreKey(&ClientParams, &KeyParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertWaitForResponse();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function fills in the user certificate configuration with the subject and
 *		issuer names, the validity period, the key usage and the vault key IDs of the keys
 *		involved.
 *
 * @param	SubjectPubKeyIdIn	Key ID of the public key to be certified.
 * @param	IssuerPubKeyIdIn	Key ID of the issuer public key.
 * @param	IssuerPvtKeyIdIn	Key ID of the private key which signs the certificate.
 *
 * @return
 *	- XST_SUCCESS, if the configuration is filled successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertFillUserCertCfg(u32 SubjectPubKeyIdIn, u32 IssuerPubKeyIdIn,
					   u32 IssuerPvtKeyIdIn)
{
	s32 Status = XST_FAILURE;
	XAsu_X509CertFields *CertCfg = &UserCertCfg.CertCfg;

	Status = Xil_SMemSet(&UserCertCfg, sizeof(UserCertCfg), 0U, sizeof(UserCertCfg));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertCopyField(CertCfg->Subject.OrgName, XASU_X509_ORG_NAME_MAX_LEN,
					   XOCP_USER_CERT_SUBJECT_ORG_NAME,
					   &CertCfg->Subject.OrgNameLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertCopyField(CertCfg->Subject.CommonName,
					   XASU_X509_COMMON_NAME_MAX_LEN,
					   XOCP_USER_CERT_SUBJECT_COMMON_NAME,
					   &CertCfg->Subject.CommonNameLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertCopyField(CertCfg->Issuer.OrgName, XASU_X509_ORG_NAME_MAX_LEN,
					   XOCP_USER_CERT_ISSUER_ORG_NAME,
					   &CertCfg->Issuer.OrgNameLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertCopyField(CertCfg->Issuer.CommonName,
					   XASU_X509_COMMON_NAME_MAX_LEN,
					   XOCP_USER_CERT_ISSUER_COMMON_NAME,
					   &CertCfg->Issuer.CommonNameLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	CertCfg->Validity.NotBefore.Year = XOCP_USER_CERT_NOT_BEFORE_YEAR;
	CertCfg->Validity.NotBefore.Month = XOCP_USER_CERT_NOT_BEFORE_MONTH;
	CertCfg->Validity.NotBefore.Day = XOCP_USER_CERT_NOT_BEFORE_DAY;
	CertCfg->Validity.NotAfter.Year = XOCP_USER_CERT_NOT_AFTER_YEAR;
	CertCfg->Validity.NotAfter.Month = XOCP_USER_CERT_NOT_AFTER_MONTH;
	CertCfg->Validity.NotAfter.Day = XOCP_USER_CERT_NOT_AFTER_DAY;

	CertCfg->KeyUsage = XASU_X509_KEY_USAGE_DIGITAL_SIGNATURE |
			    XASU_X509_KEY_USAGE_DATA_ENCIPHERMENT;

	UserCertCfg.SubjectPubKeyId = SubjectPubKeyIdIn;
	UserCertCfg.IssuerPubKeyId = IssuerPubKeyIdIn;
	UserCertCfg.IssuerPvtKeyId = IssuerPvtKeyIdIn;

	Status = XST_SUCCESS;

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("User certificate configuration failed with Status = %08x\r\n",
			      Status);
	} else {
		XilAsu_Printf("Successfully configured user certificate with subject CN = %s and "
			      "issuer CN = %s\r\n", XOCP_USER_CERT_SUBJECT_COMMON_NAME,
			      XOCP_USER_CERT_ISSUER_COMMON_NAME);
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies a name string into a certificate name field and stores its
 *		length, rejecting a string which is empty or does not fit the field.
 *
 * @param	Dest		Pointer to the destination name field.
 * @param	DestSize	Size of the destination name field.
 * @param	Src		Pointer to the source string.
 * @param	DestLen		Pointer to store the length of the copied string.
 *
 * @return
 *	- XST_SUCCESS, if the string is copied successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertCopyField(u8 *Dest, u32 DestSize, const char *Src, u32 *DestLen)
{
	s32 Status = XST_FAILURE;
	u32 SrcLen = Xil_Strnlen(Src, DestSize + 1U);

	if ((SrcLen == 0U) || (SrcLen > DestSize)) {
		goto END;
	}

	Status = Xil_SMemCpy(Dest, DestSize, Src, SrcLen, SrcLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*DestLen = SrcLen;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function requests the user X.509 certificate from the ASU and prints the DER
 *		encoded certificate which is returned.
 *
 * @return
 *	- XST_SUCCESS, if the certificate is generated successfully.
 *	- XST_FAILURE, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertGenerate(void)
{
	s32 Status = XST_FAILURE;
	XAsu_ClientParams ClientParams = {0U};
	XAsu_OcpUserCertParams OcpUserCertParams = {0U};

	ClientParams.Priority = XASU_PRIORITY_HIGH;
	ClientParams.SecureFlag = XASU_CMD_SECURE;
	ClientParams.CallBackFuncPtr =
		(XAsuClient_ResponseHandler)((void *)XAsu_OcpUserCertCallBackRef);
	ClientParams.CallBackRefPtr = (void *)&ClientParams;
	ClientParams.AdditionalStatus = XST_FAILURE;

	OcpUserCertParams.UserCertCfgAddr = (u64)(UINTPTR)&UserCertCfg;
	OcpUserCertParams.CertBufAddr = (u64)(UINTPTR)UserX509Cert;
	OcpUserCertParams.CertBufLen = XOCP_USER_CERT_X509_BUF_SIZE;
	OcpUserCertParams.CertActualSizeAddr = (u64)(UINTPTR)&UserX509CertActualSize;

	ErrorStatus = XST_FAILURE;
	Status = XAsu_OcpGetUserX509Certificate(&ClientParams, &OcpUserCertParams);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XAsu_OcpUserCertWaitForResponse();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (ClientParams.AdditionalStatus != XASU_OCP_CERT_GENERATION_SUCCESS) {
		goto END;
	}

	XAsu_OcpUserCertPrintData(UserX509Cert, UserX509CertActualSize, "USER X.509 CERTIFICATE");

END:
	if (Status != XST_SUCCESS) {
		XilAsu_Printf("User X.509 certificate generation failed with Status = %08x\r\n",
			      Status);
	} else if (ClientParams.AdditionalStatus != XASU_OCP_CERT_GENERATION_SUCCESS) {
		XilAsu_Printf("User X.509 certificate generation failed with additional error of "
			      "%08x\r\n", ClientParams.AdditionalStatus);
		Status = XST_FAILURE;
	} else {
		XilAsu_Printf("Successfully generated user X.509 certificate\r\n");
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function waits for the client library to notify completion of the request and
 *		returns the status the server reported.
 *
 * @return
 *	- XST_SUCCESS, if the request completed successfully.
 *	- The error code reported by the server, in case of failure.
 *
 *************************************************************************************************/
static s32 XAsu_OcpUserCertWaitForResponse(void)
{
	while (Notify == 0U) {
		;
	}
	Notify = 0U;

	return (s32)ErrorStatus;
}

/*************************************************************************************************/
/**
 * @brief	This function prints the given buffer as a hex string, framed by a start and an end
 *		marker so that a host can extract it from the console log.
 *
 * @param	Data		Pointer to the data to be printed.
 * @param	DataLen		Length of the data in bytes.
 * @param	BufName		Name used in the start and end markers.
 *
 *************************************************************************************************/
static void XAsu_OcpUserCertPrintData(const u8 *Data, u32 DataLen, const char *BufName)
{
	u32 Index;

	XilAsu_Printf("%s START\r\n", BufName);
	for (Index = 0U; Index < DataLen; Index++) {
		XilAsu_Printf("%02x", Data[Index]);
	}
	XilAsu_Printf("\r\n%s END\r\n", BufName);
}

/*************************************************************************************************/
/**
 * @brief	Callback function which is registered with library to get request completion
 *		request.
 *
 * @param	CallBackRef	Pointer to the callback reference.
 * @param	Status		Status of the request is passed as an argument during callback.
 *
 *************************************************************************************************/
static void XAsu_OcpUserCertCallBackRef(void *CallBackRef, u32 Status)
{
	(void)CallBackRef;

	ErrorStatus = Status;
	Notify = 1U;
}

/** @} */
