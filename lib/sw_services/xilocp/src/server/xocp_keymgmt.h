/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_keymgmt.h
* @addtogroup xil_ocpapis DeviceKeysMgmt APIs
* @{
*
* @cond xocp_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ----- ---- ----------   -------------------------------------------------------
* 1.0   vns  07/08/2022   Initial release
*       vns  01/10/2023   Adds logic to generate the DEVA on subsystem based.
* 1.2   har  02/24/2023   Added macro XOCP_INVALID_USR_CFG_INDEX
*       vns  07/06/2023   Added DEVAK regenerate support and Data clear before shutdown
*       am   07/20/2023   Added macro XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_CLEAR_MASK
* 1.3   am   01/31/2024   Fixed internal security review comments
* 1.4   har  06/10/2024   Moved XOCP_DEVAK_SUBSYS_HASH_DS_ID macro to xocp.h
* 1.4   har  26/04/2024   Add support to store personalization string for additional DevAk
*                         Renamed XOCP_GenSubSysDevAk to XOcp_GenSubSysDevAk
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XOCPKEYMGMT_SERVER_H
#define XOCPKEYMGMT_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP_KEY_MNGMT
#include "xocp.h"
#include "xocp_common.h"
#include "xsecure_sha.h"

/************************** Constant Definitions *****************************/
#define XOCP_EFUSE_DEVICE_DNA_CACHE			(0xF1250020U) /**< DNA cache */
#define XOCP_EFUSE_DEVICE_DNA_SIZE_WORDS		(4U) /**< DNA size in words */
#define XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES		(16U) /**< DNA size in bytes */
#define XOCP_CDI_SIZE_IN_BYTES				(48U) /**< CDI size in bytes */
#define XOCP_CDI_SIZE_IN_WORDS				(12U) /**< CDI size in words */
#define XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES		(48U) /**< Trng seed size in bytes */
#define XOCP_TIMEOUT_MAX				(0x1FFFFU) /**< Maximum timeout */

#define XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK	(0x00000001U) /**< Zeorize mask */
#define XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_CLEAR_MASK	(0x00000000U) /**< Zeorize clear mask */
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK	(0x00000002U)
							/**< Zeorize status pass mask */
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_DONE_MASK	(0x00000001U)
							/**< Zeorize status done mask */

#define XOCP_MAX_DEVAK_SUPPORT				(4U) /**< Maximum DEVAK support */
#define XOCP_MAX_KEYS_SUPPPORTED_PER_SUBSYSTEM		(2U)
				/**< Maximum number of DevAk key pairs supported for each subsystem */
#define XOCP_DEFAULT_DEVAK_KEY_INDEX			(0U)
				/**< Index of default DevAk for each subsystem */
#define XOCP_KEYWRAP_DEVAK_KEY_INDEX			(1U)
				/**< Index of Key wrap DevAk for each subsystem */
#define XOCP_INVALID_DEVAK_INDEX			(0xFFFFFFFFU)
							/**< Invalid DEVAK index value*/
#define XOCP_INVALID_USR_CFG_INDEX			(0xFFFFFFFFU)
							/**< Invalid user configuration index */
#define XOCP_APP_VERSION_MAX_LENGTH			(64U)
							/**< Max length of app version in bytes */

/**************************** Type Definitions *******************************/
/**
 * OCP key management driver instance to store the states
 * A pointer to an instance data structure is passed around by functions
 * to refer to a specific driver instance.
 */
typedef struct {
	u32 KeyMgmtReady;	/**< Key management ready */
	u32 DevAkInputIndex;	/**< Points to the next empty dev AK */
} XOcp_KeyMgmt;

/**
 * DEVAK data storage for in place PLM update
 */
typedef struct {
	u32 SubSystemId;	/**< Corresponding Sub system ID */
	u8 SubSysHash[XSECURE_HASH_SIZE_IN_BYTES]; /**< Hash of the subsystem */
	u8 AppVersion[XOCP_APP_VERSION_MAX_LENGTH];	/**< App version */
	u32 AppVersionLen;		/**< Length of app version */
	u32 ValidData;		/**< Valid Data */
} XOcp_SubSysHash;

/**
 * DEV AK data structure
 */
typedef struct {
	u32 SubSystemId;	/**< Corresponding Sub system ID */
	u32 KeyIndex;		/**< Index of DevAk for the subsystem */
	u32 AppVersionLen;		/**< Length of app version */
	u8 AppVersion[XOCP_APP_VERSION_MAX_LENGTH];		/**< App version */
	u8 PerString[XTRNGPSX_PERS_STRING_LEN_IN_BYTES];/**< Personalization string */
	u8 SubSysHash[XSECURE_HASH_SIZE_IN_BYTES]; /**< Hash of the subsystem */
	u8 EccPrvtKey[XOCP_ECC_P384_SIZE_BYTES]; /**< ECC DevAK private key */
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];	/**< ECC DevAK public key X */
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];	/**< ECC DevAK public key Y */
	u32 IsDevAkKeyReady; /**< Indicates Dev AK availability */
} XOcp_DevAkData;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_GenerateDevIKKeyPair(void);
int XOcp_DevAkInputStore(u32 SubSystemId, u8 *PerString, u32 KeyIndex);
int XOcp_GetSubSysDevAkIndex(u32 SubSystemId, u32* DevAkIndex);
int XOcp_GenerateDevAk(u32 SubSystemId);
int XOcp_GetX509Certificate(XOcp_X509Cert *XOcp_GetX509CertPtr, u32 SubSystemId);
int XOcp_AttestWithDevAk(XOcp_Attest *AttestWithDevAkPtr, u32 SubSystemId);
int XOcp_AttestWithKeyWrapDevAk(XOcp_Attest *AttestationInfoPtr, u32 SubSystemId,
	u64 AttnPloadAddr, u32 AttnPloadSize);
u32 XOcp_IsDevIkReady(void);
int XOcp_RegenSubSysDevAk(void);
int XOcp_ShutdownHandler(XPlmi_ModuleOp Op);
int XOcp_GenSubSysDevAk(u32 SubsystemID, u64 InHash);
int XOcp_GenSharedSecretwithDevAk(u32 SubSystemId, u64 PubKeyAddr, u64 SharedSecretAddr);
#ifndef VERSAL_AIEPG2
int XOcp_SetAppVersion(u32 SubsystemID, u64 AppVersion, u32 AppVersionLen);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PLM_OCP_KEY_MNGMT */
#endif /* XOCPKEYMGMT_SERVER_H */
