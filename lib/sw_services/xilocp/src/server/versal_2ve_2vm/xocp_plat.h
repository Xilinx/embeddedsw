/**************************************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_plat.h
* @addtogroup xil_ocpapis APIs to communicate with ASUFW for OCP functionalities
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.0   rmv  07/17/25 Initial release
* 1.6   tvp  05/16/25 Add XOcp_GetRegSpace function
*       tvp  09/13/25 Moved XOcp_ReadSecureConfig to platform specific file
*
* </pre>
*
**************************************************************************************************/
#ifndef XOCP_PLAT_H
#define XOCP_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xil_types.h"

#ifdef PLM_OCP
#include "xocp_hw.h"

/********************************** Constant Definitions *****************************************/

/************************************ Type Definitions *******************************************/
/**
 * Data structure to hold HW register addresses required for OCP key management
 */
typedef struct {
	u32 DmeSignRAddr;		/**< Address of DME Challenge R */
	u32 DmeSignSAddr;		/**< Address of DME Challenge S */
	u32 DiceCdiSeedAddr;		/**< Address of DICE CDI SEED_0 */
	u32 DiceCdiSeedValidAddr;	/**< Address of DICE CDI SEED VALIDITY */
	u32 DiceCdiSeedParityAddr;	/**< Address of DICE CDI SEED PARITY */
	u32 DevIkPvtAddr;		/**< Address of DEV_IK_PVT_KEY_0 */
	u32 DevIkPubXAddr;		/**< Address of DEV_IK_PUBLIC_KEY_X_0 */
	u32 DevIkPubYAddr;		/**< Address of DEV_IK_PUBLIC_KEY_Y_0 */
} XOcp_RegSpace;

/**
 * Data structure to hold secure configs.
 */
typedef struct {
	u32 BootmodeDis;	/* BOOT_MODE_DIS_15_0 */
	u32 MiscCtrl;		/* MISC_CTRL */
	u32 AnlgTrim3; 		/* ANLG_TRIM_3 */
	u32 BootEnvCtrl;	/* BOOT_ENV_CTRL considering reserved bits */
	u32 IpDisable1;		/* IP_DISABLE_1 */
	u32 Caher1;		/* Caher_1 */
	u32 DecOnly;		/* DEC_ONLY */
	u32 SecCtrl;		/* Secure control */
	u32 SecMisc1;		/* SEC_MISC 1 */
	u32 DmeFips;		/* DME FIPS */
	u32 IPDisable0;		/* IP_DISABLE_0 */
	u32 RomRsvd;		/* ROM RSVD */
	u32 RoSwapEn;		/* RO_SWAP_EN */
} XOcp_SecureConfig;

#ifdef PLM_OCP_ASUFW_KEY_MGMT

/**************************** Macros (Inline Functions) Definitions ******************************/
#define XOCP_ASUFW_PLM_MODULE_ID		(10U)	/**< PLM module ID in ASUFW */
#define XOCP_ASUFW_DEV_KEY_CMD_ID		(0U)	/**< Command ID for device key generation */
#define XOCP_ASUFW_PLM_REQ_ID			(0x3FU)	/**< Fixed request ID for PLM request */

#define XOCP_ASUFW_COMMAND_ID_MASK		(0x0000003FU)	/**< Command ID mask */
#define XOCP_ASUFW_UNIQUE_REQ_ID_MASK		(0x00000FC0U)	/**< Unique request ID mask */
#define XOCP_ASUFW_UNIQUE_REQ_ID_SHIFT		(6U)		/**< Unique request ID shift
								  value */
#define XOCP_ASUFW_MODULE_ID_MASK		(0x0003F000U)	/**< Module ID mask */
#define XOCP_ASUFW_MODULE_ID_SHIFT		(12U)		/**< Module ID shit value */
#define XOCP_ASUFW_CMD_LENGTH_MASK		(0x00FC0000U)	/**< Command length mask */
#define XOCP_ASUFW_CMD_LENGTH_SHIFT		(18U)		/**< Command length shift value */

#define XOCP_ASU_EVENT_CMD_LEN			(1U)	/**< Command length for PLM to ASU event */
#define XOCP_PLM_ASUFW_CMD_CMD_VAL	(XOCP_ASUFW_DEV_KEY_CMD_ID & XOCP_ASUFW_COMMAND_ID_MASK)
	/**< Command value in device key generation command */
#define XOCP_PLM_ASUFW_CMD_REQ_ID_VAL		(((u32)XOCP_ASUFW_PLM_REQ_ID << \
						  XOCP_ASUFW_UNIQUE_REQ_ID_SHIFT) \
						 & XOCP_ASUFW_UNIQUE_REQ_ID_MASK)
	/**< Request ID value in device key generation command */
#define XOCP_PLM_ASUFW_CMD_MODULE_ID_VAL	(((u32)XOCP_ASUFW_PLM_MODULE_ID << \
						  XOCP_ASUFW_MODULE_ID_SHIFT) \
						 & XOCP_ASUFW_MODULE_ID_MASK)
	/**< Module ID value in device key generation command */
#define XOCP_PLM_ASUFW_CMD_LEN_VAL		(((u32)XOCP_ASU_EVENT_CMD_LEN << \
						  XOCP_ASUFW_CMD_LENGTH_SHIFT) \
						 & XOCP_ASUFW_CMD_LENGTH_MASK)
	/**< Command length value in device key generation command */
#define XOCP_PLM_ASUFW_CMD_HDR			((u32)(XOCP_PLM_ASUFW_CMD_LEN_VAL | \
						      XOCP_PLM_ASUFW_CMD_MODULE_ID_VAL | \
						      XOCP_PLM_ASUFW_CMD_REQ_ID_VAL | \
						      XOCP_PLM_ASUFW_CMD_CMD_VAL))
	/**< Command header for device key generation command */

/************************************ Function Prototypes ****************************************/
int XOcp_StoreOcpSubsysIDs(u32 SubsystemIdListLen, const u32 *SubsystemIdList);
int XOcp_StoreSubsysDigest(u32 SubsystemId, u64 Hash);
int XOcp_GetSubsysDigest(u32 SubsystemId, u32 SubsysHashAddrPtr);
int XOcp_GetAsuCdiSeed(u32 CdiAddr);
int XOcp_NotifyAsu(void);
#endif /* PLM_OCP_ASUFW_KEY_MGMT */

XOcp_RegSpace* XOcp_GetRegSpace(void);
void XOcp_ReadSecureConfig(XOcp_SecureConfig* EfuseConfig);

/********************************** Variable Definitions *****************************************/

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif
#endif /* XOCP_PLAT_H */
