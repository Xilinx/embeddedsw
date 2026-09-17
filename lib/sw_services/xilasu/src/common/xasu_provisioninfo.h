/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_provisioninfo.h
 *
 * This file contains the Provisioning module definitions which are common across the client and
 * server.
 *
 * The Provisioning module implements the ASU Provisioning Interface (SAS section 12.16). It
 * supports programming the USER eFUSE keys. Both USER_EFUSE_KEY_0 and USER_EFUSE_KEY_1 are always
 * programmed in black form (PUF KEK encrypted); a mixed red/black selection across the two user
 * keys is not a supported hardware use case.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   kp   06/27/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_common_defs Common Defs
 * @{
*/
#ifndef XASU_PROVISIONINFO_H_
#define XASU_PROVISIONINFO_H_

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
/* Provisioning module command IDs */
#define XASU_PROVISION_USER_EFUSE_CMD_ID	(0U) /**< Command ID for USER eFUSE key
								provisioning */
#define XASU_PROVISION_UDE_CMD_ID		(1U) /**< Command ID for Unique Device Endorsement
								(UDE) key provisioning */
#define XASU_PROVISION_GET_DPC_CMD_ID		(2U) /**< Command ID for generating and returning the
								Device Provisioning Certificate (DPC) */
#define XASU_PROVISION_DELETE_DPC_CMD_ID	(3U) /**< Command ID for deleting the provisioning
								key pairs backing the DPC */
#define XASU_PROVISION_MAX_CMDS			(4U) /**< Maximum number of commands supported by
								the Provisioning module */

#define XASU_PROVISION_USER_EFUSE_KEY_SIZE_IN_BYTES	(32U) /**< USER eFUSE key size in bytes
								(256-bit) */

/* UDE key sizes (ECC NIST P-384). */
#define XASU_PROVISION_UDE_PVT_KEY_SIZE_IN_BYTES	(48U) /**< UDE private key size in bytes
								(ECC P-384). */
#define XASU_PROVISION_UDE_CERT_MAX_SIZE_IN_BYTES	(2000U) /**< Maximum size in bytes of the
								UDE self-signed X.509 certificate. */
#define XASU_PROVISION_UDE_CERT_ACTUAL_SIZE_IN_BYTES	(4U) /**< Size in bytes of the UDE
								certificate actual-size value returned
								through the response buffer. */

/*
 * UDE key type. Only ECC NIST P-384 is supported currently; ML-DSA support is deferred to a later
 * release.
 */
#define XASU_PROVISION_UDE_KEY_TYPE_ECC_P384	(0U) /**< UDE key type ECC NIST P-384. */

#define XASU_PROVISION_DPC_ACTUAL_SIZE_IN_BYTES	(4U) /**< Size in bytes of the DPC actual-size
								value returned through the response
								buffer. */

/* USER eFUSE key identifier (which eFUSE user key to program). */
#define XASU_PROVISION_EFUSE_USER_KEY_0		(1U) /**< Program USER_EFUSE_KEY_0 */
#define XASU_PROVISION_EFUSE_USER_KEY_1		(2U) /**< Program USER_EFUSE_KEY_1 */

/* Key source for the key to be programmed into the USER eFUSE. */
#define XASU_PROVISION_KEY_SOURCE_ASU_GENERATED	(0U) /**< Key generated inside the ASU via TRNG.
							Only source allowed for USER_EFUSE_KEY_0. */
#define XASU_PROVISION_KEY_SOURCE_VAULT		(1U) /**< Key fetched from the AKV by KeyId. */
#define XASU_PROVISION_KEY_SOURCE_USER_BUFFER	(2U) /**< Key supplied by the client via KeyAddr. */

/************************************** Type Definitions *****************************************/
/**
 * This structure contains the parameters for the USER eFUSE key provisioning command.
 * The key is always programmed in black form (PUF KEK encrypted). The key source is selected
 * explicitly via KeySource:
 *  - XASU_PROVISION_KEY_SOURCE_ASU_GENERATED: the ASU generates the key with the TRNG; KeyId and
 *    KeyAddr must both be zero. This is the only source permitted for USER_EFUSE_KEY_0.
 *  - XASU_PROVISION_KEY_SOURCE_VAULT: KeyId selects an AKV key (KeyAddr must be zero).
 *  - XASU_PROVISION_KEY_SOURCE_USER_BUFFER: KeyAddr supplies a raw key buffer (KeyId must be zero).
 */
typedef struct {
	u64 KeyAddr;	/**< Address of the user supplied plaintext key buffer. The buffer must be
				allocated for 32 bytes (256-bit key). Used only when KeySource is
				XASU_PROVISION_KEY_SOURCE_USER_BUFFER. */
	u32 KeyId;	/**< Composite key identifier of the AKV key to be programmed (encodes the
				user-assigned vault ID in bits [31:24], key type in bits [23:16]
				and key index in bits [15:0]). Used only when KeySource is
				XASU_PROVISION_KEY_SOURCE_VAULT. */
	u16 KeyLen;	/**< Length of the key in bytes. Must be
				XASU_PROVISION_USER_EFUSE_KEY_SIZE_IN_BYTES (256-bit). */
	u8 EfuseKeyId;	/**< Target USER eFUSE key to program (XASU_PROVISION_EFUSE_USER_KEY_0 or
				XASU_PROVISION_EFUSE_USER_KEY_1). */
	u8 KeySource;	/**< Source of the key to be programmed (XASU_PROVISION_KEY_SOURCE_*). */
} XAsu_ProvisionUserEfuseParams;

/**
 * This structure contains the parameters for the Unique Device Endorsement (UDE) key provisioning
 * command. The UDE key pair (ECC NIST P-384) is generated inside the ASU (the private key never
 * leaves the ASU), the private key is encrypted with the PUF KEK and programmed into the requested
 * eFUSE slot, and a self-signed X.509 certificate over the generated public key is returned to the
 * caller at CertBufAddr with its actual size at CertActualSizeAddr.
 */
typedef struct {
	u64 CertBufAddr;	/**< Address of the caller buffer that receives the generated UDE
					self-signed X.509 certificate (DER encoded). */
	u64 CertActualSizeAddr;	/**< Address of the caller u32 that receives the actual size in
					bytes of the generated certificate. The server returns the
					size through the response buffer and the client copies it to
					this address during the callback. */
	u32 CertBufSize;	/**< Size in bytes of the caller certificate buffer at CertBufAddr.
					Must be able to hold up to
					XASU_PROVISION_UDE_CERT_MAX_SIZE_IN_BYTES bytes. */
	u8 UdeSlot;	/**< Target UDE eFUSE slot to program (XASU_OCP_UDE_USER_KEY_0/1/2_ID). */
	u8 UdeKeyType;	/**< UDE key type (XASU_PROVISION_UDE_KEY_TYPE_ECC_P384). */
	u16 Reserved;	/**< Reserved for alignment. */
} XAsu_ProvisionUdeParams;

/**
 * This structure contains the parameters for the Get Device Provisioning Certificate command
 * (SAS section 12.16.2). The ASU checks the internal exportable key vault for the calling
 * subsystem's provisioning authentication key pair (ECC NIST P-384) and provisioning wrapping key
 * pair (ECIES over NIST P-384); any pair that is missing is generated with the TRNG and stored in
 * the exportable vault. A UDE-signed X.509 certificate is generated over each public key, the DPC
 * container (Device DNA, both certificates and a UDE signature over them) is assembled and returned
 * to the caller at DpcBufAddr, and its actual size is written to DpcActualSizeAddr.
 */
typedef struct {
	u64 DpcBufAddr;		/**< Address of the caller buffer that receives the assembled DPC
					(DER encoded X.509 certificates inside the container). */
	u64 DpcActualSizeAddr;	/**< Address of the caller u32 that receives the actual size in
					bytes of the assembled DPC. */
	u32 DpcBufSize;		/**< Size in bytes of the caller DPC buffer at DpcBufAddr. */
	u32 Reserved;		/**< Reserved for alignment. */
} XAsu_DevProvisionCertParams;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#ifdef __cplusplus
}
#endif

#endif  /* XASU_PROVISIONINFO_H_ */
/** @} */
