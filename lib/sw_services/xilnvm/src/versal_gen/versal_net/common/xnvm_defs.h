/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/common/xnvm_defs.h
* @addtogroup xnvm_versal_net_api_ids XilNvm Versal Net API IDs
* @{
*
* @cond xnvm_internal
* This file contains the xilnvm Versal_Net API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0   kal  07/12/22 Initial release
* 3.2   har  02/21/23 Added support for writing Misc Ctrl bits and ROM Rsvd bits
*    	vek  05/31/23 Added support for Programming PUF secure control bits
* 3.6   vss  08/22/25 Added macros required to support DME in versal_2ve_2vm.
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_DEFS_H
#define XNVM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"
#include "xnvm_common_defs.h"

/************************** Constant Definitions ****************************/
/**@cond xnvm_internal
 * @{
 */
/* Enable client printfs by setting XNVM_DEBUG to 1 */
#define XNVM_DEBUG	(0U)

#if (XNVM_DEBUG)
#define XNVM_DEBUG_GENERAL (1U)
#else
#define XNVM_DEBUG_GENERAL (0U)
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XNvm_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}

/* Macro to typecast XILNVM API ID */
#define XNVM_API(ApiId)	((u32)ApiId)

#define XNVM_API_ID_MASK		(0xFFU)		/** Macro to extract the API_ID from CmdId */
#define XNVM_EFUSE_CRC_AES_ZEROS 	(0x6858A3D5U) 	/** All zero AES key CRC value */
#define XNVM_NUM_OF_CACHE_ADDR_PER_PAGE	(0x400U) 	/** Number of cache addresses per eFuse page */
#define XNVM_EFUSE_ERROR_BYTE_SHIFT	(8U) 		/** Number of bit positions to be shifted to achieve 1 byte shift */
#define XNVM_EFUSE_ERROR_NIBBLE_SHIFT	(4U) 		/** Number of bit positions to be shifted to achieve 1 byte shift */

/**
 * BBRAM configuration limiter
*/
#define XNVM_BBRAM_CONFIG_LIMITER_DISABLED	(0x0U) /** Configuration Limiter feature is disabled */
#define XNVM_BBRAM_CONFIG_LIMITER_ENABLED	(0x3U) /** Configuration Limiter feature is enabled */

#define XNVM_BBRAM_CONFIG_LIMITER_FAIL_CONFIGS_COUNT	(0x0U) /** Mode of counter is for failed configurations */
#define XNVM_BBRAM_CONFIG_LIMITER_TOTAL_CONFIGS_COUNT	(0x3U) /** Mode of counter is for total configurations */

#ifndef VERSAL_2VE_2VM
#define XNVM_DME_REVOKE_MAX_VALUE	XNVM_EFUSE_DME_REVOKE_3 /** Maximum value for DME revoke */
#else
#define XNVM_DME_REVOKE_MAX_VALUE	XNVM_EFUSE_DME_REVOKE_2 /** Maximum value for DME revoke */
#endif

#ifndef VERSAL_2VE_2VM
#define XNVM_DME_USER_KEY_MAX_VALUE	XNVM_EFUSE_DME_USER_KEY_3 /** Maximum value for DME user key */
#else
#define XNVM_DME_USER_KEY_MAX_VALUE	XNVM_EFUSE_DME_USER_KEY_2 /** Maximum value for DME user key */
#endif

/************************** Variable Definitions *****************************/
#define XNVM_UDS_SIZE_IN_WORDS          (12U) /** UDS size in words */
#define XNVM_DME_USER_KEY_SIZE_IN_WORDS	(12U) /** DME key size in words */

/**************************** Type Definitions *******************************/
/** Structure for RomRsvd eFuses */
typedef struct {
	u8 PlmUpdate;
	u8 AuthKeysToHash;
	u8 IroSwap;
	u8 RomSwdtUsage;
} XNvm_EfuseRomRsvdBits;

/** Structure for Fips version and Fips mode eFuses */
typedef struct {
	u8 FipsMode;
	u8 FipsVersion;
} XNvm_EfuseFipsInfoBits;

/** Structure for Read eFuse cache IPI command */
typedef struct {
	u16 StartOffset;
	u16 RegCount;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_RdCachePload;

/** Structure for XNvm_RdCachePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_RdCachePload Pload;
} XNvm_RdCacheCdo;

/** Structure for AES key write params for IPI */
typedef struct {
	u16 EnvDisFlag;
	u16 KeyType;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_AesKeyWritePload;

/** Structure for XNvm_AesKeyWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_AesKeyWritePload Pload;
} XNvm_AesKeyWriteCdo;

/** Structure for IPI payload params for PPK write */
typedef struct {
	u16 EnvDisFlag;
	u16 PpkType;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_PpkWritePload;

/** Structure for XNvm_PpkWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_PpkWritePload Pload;
} XNvm_PpkWriteCdo;

/** Structure for IPI payload params for IV write */
typedef struct {
	u16 EnvDisFlag;
	u16 IvType;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_IvWritePload;

/** Structure for XNvm_IvWriteCdo with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_IvWritePload Pload;
} XNvm_IvWriteCdo;

/** Structure for IPI payload params for UDS write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_UdsWritePload;

/** Structure for XNvm_UdsWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_UdsWritePload Pload;
} XNvm_UdsWriteCdo;

/** Structure for IPI payload params for DME key write */
typedef struct {
	u16 EnvDisFlag;
	u16 DmeKeyType;
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_DmeKeyWritePload;

/** Structure for XNvm_DmeKeyWriteCdo with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_DmeKeyWritePload Pload;
} XNvm_DmeKeyWriteCdo;

/** Structure for CDO payload params for AES key write */
typedef struct {
	u16 EnvDisFlag;
	u16 KeyType;
	XNvm_AesKey EfuseKey;
} XNvm_AesKeyWriteDirectPload;

/** Structure for CDO payload params for PPK hash write */
typedef struct {
	u16 EnvDisFlag;
	u16 PpkType;
	XNvm_PpkHash EfuseHash;
} XNvm_PpkWriteDirectPload;

/** Structure for CDO payload params for IV write */
typedef struct {
	u16 EnvDisFlag;
	u16 IvType;
	XNvm_Iv EfuseIv;
} XNvm_IvWriteDirectPload;

/** Structure for IPI/CDO payload params for Glitch config write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 GlitchConfigVal;
} XNvm_GlitchConfig;

/** Structure for IPI/CDO payload params for DECRYPT_ONLY eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
} XNvm_DecOnly;

/** Structure for IPI/CDO payload params for REVOCATION_ID eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 RevokeIdNum;
} XNvm_RevodeId;

/** Structure for IPI/CDO payload params for OFFCHIP_REVOCATION_ID eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 OffChipIdNum;
} XNvm_OffChipId;

/** Structure for IPI/CDO payload params for MISC_CTRL eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 MiscCtrlBitsVal;
} XNvm_MiscCtrlBits;

/** Structure for IPI/CDO payload params for SECURITY_CONTROL eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 SecCtrlBitsVal;
} XNvm_SecCtrlBits;

/** Structure for IPI/CDO payload params for MISC1_CTRL eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 Misc1CtrlBitsVal;
} XNvm_Misc1CtrlBits;

/** Structure for IPI/CDO payload params for BOOT_ENV_CTRL eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 BootEnvCtrlVal;
} XNvm_BootEnvCtrlBits;

/** Structure for IPI/CDO payload params for FIPS_MODE and FIPS_VERSION eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u16 FipsMode;
	u16 FipsVersion;
} XNvm_FipsInfo;

/** Structure for UDS data to be programmed into UDS eFuses */
typedef struct {
	u32 Uds[XNVM_UDS_SIZE_IN_WORDS];
} XNvm_Uds;

/** Structure for CDO payload params for UDS write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	XNvm_Uds EfuseUds;
} XNvm_UdsDirectPload;

/** Structure for DME_KEY data to be programmed into DME key eFuses */
typedef struct {
	u32 Key[XNVM_DME_USER_KEY_SIZE_IN_WORDS];
} XNvm_DmeKey;

/** Structure for CDO payload params for DME key write */
typedef struct {
	u16 EnvDisFlag;
	u16 KeyType;
	XNvm_DmeKey EfuseDmeKey;
} XNvm_DmeKeyDirectPload;

/** Structure for CDO payload params for DME_REVOKE write */
typedef struct {
	u16 EnvDisFlag;
	u16 DmeRevokeNum;
} XNvm_DmeRevokeDirectPload;

/** Structure for IPI/CDO payload params DME Revoke */
typedef struct {
	u32 CdoHdr;
	XNvm_DmeRevokeDirectPload Pload;
} XNvm_DmeRevokeId;

/** Structure for IPI/CDO payload params for DISABLE_PLM_UPDATE eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
} XNvm_DisPlmUpdate;

/** Structure for IPI payload params for CRC eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 EfuseCrc;
} XNvm_Crc;

/** Structure for IPI payload params for DME_MODE eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 EfuseDmeMode;
} XNvm_DmeMode;

/** Structure for CDO payload params for PUF_HD, CHASH, AUX eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 Chash;
	u32 Aux;
	u32 RoSwap;
	u32 SynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
} XNvm_PufHDInfoDirectPload;

/** Structure for CDO payload params for PUF_CTRL eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 Reserved;
	u32 PufCtrlBits;
} XNvm_PufCtrlDirectPload ;

/** Structure for IPI/CDO payload params for BOOT_MODE_DIS eFuses write */
typedef struct {
	u16 EnvDisFlag;
	u16 BootModeDisVal;
} XNvm_BootModeDis;

/** Enum for DME_USER_KEY */
typedef enum {
	XNVM_EFUSE_DME_USER_KEY_0 = 0,	/** 0U */
	XNVM_EFUSE_DME_USER_KEY_1,	/** 1U */
	XNVM_EFUSE_DME_USER_KEY_2,	/** 2U */
	XNVM_EFUSE_DME_USER_KEY_3	/** 3U */
} XNvm_DmeKeyType;

/** Enum for DME_REVOKE */
typedef enum {
	XNVM_EFUSE_DME_REVOKE_0 = 0,	/** 0U */
	XNVM_EFUSE_DME_REVOKE_1,	/** 1U */
	XNVM_EFUSE_DME_REVOKE_2,	/** 2U */
	XNVM_EFUSE_DME_REVOKE_3		/** 3U */
} XNvm_DmeRevoke;

/** Structure for IPI payload params for DME_MODE eFuses write */
typedef struct {
	u32 EnvMonitorDis;
	u32 DmeMode;
} XNvm_DmeModeWritePload;

/** Structure for XNvm_DmeModeWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_DmeModeWritePload Pload;
} XNvm_DmeModeWriteCdo;

/** Structure for IPI payload params for SECURITY_CONTROL eFuses write */
typedef struct {
	u32 EnvMonitorDis;
	u32 SecCtrlBits;
} XNvm_SecCtrlBitsWritePload;

/** Structure for XNvm_SecCtrlBitsWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_SecCtrlBitsWritePload Pload;
} XNvm_SecCtrlBitsWriteCdo;

/** Structure for IPI payload params for PUF_CTRL eFuses write */
typedef struct {
	u32 EnvMonitorDis;
	u32 PufCtrlBits;
} XNvm_PufCtrlBitsWritePload;

/** Structure for XNvm_PufCtrlBitsWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_PufCtrlBitsWritePload Pload;
} XNvm_PufCtrlBitsWriteCdo;

/** Structure for IPI payload params for MISC_CTRL eFuses write */
typedef struct {
	u32 EnvMonitorDis;
	u32 MiscCtrlBits;
} XNvm_MiscCtrlBitsWritePload;

/** Structure for XNvm_MiscCtrlBitsWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_MiscCtrlBitsWritePload Pload;
} XNvm_MiscCtrlBitsWriteCdo;

/** Structure for IPI payload params for ROM_RSVD eFuses write */
typedef struct {
	u32 EnvMonitorDis;
	u32 RomRsvdBits;
} XNvm_RomRsvdBitsWritePload;

/** Structure for XNvm_RomRsvdBitsWritePload with Cdo header */
typedef struct {
	u32 CdoHdr;
	XNvm_RomRsvdBitsWritePload Pload;
} XNvm_RomRsvdBitsWriteCdo;

/** Structure for IPI payload params for PUF_HD eFuses write */
typedef struct {
	u32 AddrLow;
	u32 AddrHigh;
} XNvm_PufWritePload;

/** Structure for XNvm_PufWritePload with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_PufWritePload Pload;
} XNvm_PufWriteCdo;

/** Structure for IPI payload params for SEC_MISC1 eFuses write */
typedef struct {
	u32 CdoHdr;
	XNvm_Misc1CtrlBits Pload;
} XNvm_SecMisc1BitsCdo;

/** Structure for XNvm_DisPlmUpdate with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_DisPlmUpdate Pload;
} XNvm_DisPlmUpdateCdo;

/** Structure for XNvm_DecOnly with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_DecOnly Pload;
} XNvm_DecOnlyCdo;

/** Structure for XNvm_RevodeId with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_RevodeId Pload;
} XNvm_RevokeIdCdo;

/** Structure for XNvm_OffChipId with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_OffChipId Pload;
} XNvm_OffChipIdCdo;

/** Structure for XNvm_BootModeDis with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_BootModeDis Pload;
} XNvm_BootModeDisCdo;

/** Structure for XNvm_FipsInfo with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_FipsInfo Pload;
} XNvm_FipsInfoCdo;

/** Structure for XNvm_GlitchConfig with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_GlitchConfig Pload;
} XNvm_GlitchConfigCdo;

/** Structure for XNvm_BootEnvCtrlBits with Cdo Header */
typedef struct {
	u32 CdoHdr;
	XNvm_BootEnvCtrlBits Pload;
} XNvm_BootEnvCtrlBitsCdo;

/** Structure for PUF HD, PUF_CTRL, CHASH, AUX and RoSwap */
typedef struct {
	u32 PufSecCtrlBits;
	u32 PrgmPufHelperData;
	u32 EnvMonitorDis;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
	u32 RoSwap;
} XNvm_EfusePufHdAddr;

/** Structure for addresses of AES, PPK Hash and IV data structures */
typedef struct {
	u64 EnvMonDisFlag;
	u64 AesKeyAddr;
	u64 PpkHashAddr;
	u64 IvAddr;
} XNvm_EfuseWriteDataAddr;

/** XilNVM API ids */
typedef enum {
	XNVM_API_FEATURES = 0,
	XNVM_API_ID_BBRAM_WRITE_AES_KEY,
	XNVM_API_ID_BBRAM_ZEROIZE,
	XNVM_API_ID_BBRAM_WRITE_USER_DATA,
	XNVM_API_ID_BBRAM_READ_USER_DATA,
	XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA,
	XNVM_API_ID_BBRAM_WRITE_AES_KEY_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_AES_KEY,
	XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_PPK_HASH,
	XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_IV,
	XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG,
	XNVM_API_ID_EFUSE_WRITE_DEC_ONLY,
	XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID,
	XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID,
	XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS,
	XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS,
	XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS,
	XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS,
	XNVM_API_ID_EFUSE_WRITE_FIPS_INFO,
	XNVM_API_ID_EFUSE_WRITE_UDS_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_DME_REVOKE,
	XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE,
	XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE,
	XNVM_API_ID_EFUSE_WRITE_CRC,
	XNVM_API_ID_EFUSE_WRITE_DME_MODE,
	XNVM_API_ID_EFUSE_WRITE_PUF_HD_FROM_PLOAD,
	XNVM_API_ID_EFUSE_WRITE_PUF,
	XNVM_API_ID_EFUSE_WRITE_ROM_RSVD,
	XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_CACHE,
	XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS,
	XNVM_API_ID_EFUSE_WRITE_UDS,
	XNVM_API_ID_EFUSE_WRITE_DME_KEY,
#ifdef VERSAL_2VE_2VM
	XNVM_API_ID_BBRAM_WRITE_CFG_LMT_PARAMS,
#endif
	XNVM_API_MAX,
} XNvm_ApiId;

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_DEFS_H */
/** @} */
