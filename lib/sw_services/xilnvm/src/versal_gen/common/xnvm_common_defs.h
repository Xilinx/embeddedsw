/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_common_defs.h
*
* This file contains the xilnvm Versal_Net API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  01/05/22 Initial release
* 3.2   ng   06/30/23 Added support for system device tree flow
*       yog  09/13/23 Fixed review comments
* 3.3	vss  02/23/24 Added IPI support for eFuse read and write
*       ng   11/22/2023 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/

#ifndef XNVM_COMMON_DEFS_H
#define XNVM_COMMON_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

#ifdef SDT
#include "xilnvm_bsp_config.h"
#endif

/************************** Constant Definitions ****************************/
/**
 * @cond xnvm_internal
 * @{
 */
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)	/**< PUF syndrome data formatted */
#define XNVM_NUM_OF_REVOKE_ID_FUSES			(8U)	/**< Number of revocation ID eFuses */
#define XNVM_NUM_OF_OFFCHIP_ID_FUSES			(8U)	/**< Number of offchip revocation ID eFuses */
#define XNVM_EFUSE_AES_KEY_LEN_IN_WORDS			(8U)	/**< AES key length in words */
#define XNVM_EFUSE_IV_LEN_IN_WORDS                      (3U)	/**< eFuse IV length in words */
#ifndef VERSAL_2VE_2VM
#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS		(8U)	/**< PPK Hash length in words for Versal and VersalNet */
#else
#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS                (12U)	/**< PPK Hash length in words for Versal_2Ve_2Vm */
#endif
#define XNVM_EFUSE_DNA_LEN_IN_WORDS			(4U)	/**< DNA length in words */
#define XNVM_EFUSE_IV_LEN_IN_BITS			(96U)	/**< IV length in words */
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS			(256U)	/**< AES key length in bits */
#ifndef VERSAL_2VE_2VM
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS			(256U)	/**< PPK hash length in bits for Versal and VersalNet */
#else
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS                 (384U)	/**< PPK hash length in bits for Versal_2Ve_2Vm */
#endif
#define XNVM_EFUSE_IV_LEN_IN_BYTES			(12U)	/**< IV length in bits */
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES			(32U)	/**< Aes key length in bytes */
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES		(32U)	/**< PPK hash length in bytes */
#define XNVM_IV_STRING_LEN				(24U)	/**< IV length in string */
#define XNVM_WORD_LEN					(4U)	/**< WORD length */
#define XNVM_EFUSE_CRC_AES_ZEROS			(0x6858A3D5U)	/**< CRC value for all zero key */
#define XNVM_EFUSE_MAX_BITS_IN_ROW			(32U)	/**< Maximum bits in a row */
#define XNVM_MAX_REVOKE_ID_FUSES		(XNVM_NUM_OF_REVOKE_ID_FUSES	\
						* XNVM_EFUSE_MAX_BITS_IN_ROW)	/**< Maximum revocation ID eFuses */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/** Structure for Hash data to be programmed to eFuses */
typedef struct {
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
} XNvm_PpkHash;

/** Structure for IV data */
typedef struct {
	u32 Iv[XNVM_EFUSE_IV_LEN_IN_WORDS];
} XNvm_Iv;

/** Structure for Aes key data */
typedef struct {
	u32 Key[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
} XNvm_AesKey;

/** Structure for DNA data */
typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_LEN_IN_WORDS];
} XNvm_Dna;

/** Enum for IV types */
typedef enum {
	XNVM_EFUSE_META_HEADER_IV_RANGE = 0,	/**< 0U */
	XNVM_EFUSE_BLACK_IV,			/**< 1U */
	XNVM_EFUSE_PLM_IV_RANGE,		/**< 2U */
	XNVM_EFUSE_DATA_PARTITION_IV_RANGE	/**< 3U */
} XNvm_IvType;

/** Enum for PPK types */
typedef enum {
	XNVM_EFUSE_PPK0 = 0,	/**< 0U */
	XNVM_EFUSE_PPK1,	/**< 1U */
	XNVM_EFUSE_PPK2,	/**< 2U */
#ifdef XNVM_EN_ADD_PPKS
	XNVM_EFUSE_PPK3,	/**< 3U */
	XNVM_EFUSE_PPK4		/**< 4U */
#endif
} XNvm_PpkType;

/** Enum for Aes key types */
typedef enum {
	XNVM_EFUSE_AES_KEY = 0,	/**< 0U */
	XNVM_EFUSE_USER_KEY_0,	/**< 1U */
	XNVM_EFUSE_USER_KEY_1,	/**< 2U */
} XNvm_AesKeyType;

/** Enum for Revocation IDs */
typedef enum {
	XNVM_EFUSE_REVOCATION_ID_0 = 0,	/**< 0U */
	XNVM_EFUSE_REVOCATION_ID_1,	/**< 1U */
	XNVM_EFUSE_REVOCATION_ID_2,	/**< 2U */
	XNVM_EFUSE_REVOCATION_ID_3,	/**< 3U */
	XNVM_EFUSE_REVOCATION_ID_4,	/**< 4U */
	XNVM_EFUSE_REVOCATION_ID_5,	/**< 5U */
	XNVM_EFUSE_REVOCATION_ID_6,	/**< 6U */
	XNVM_EFUSE_REVOCATION_ID_7	/**< 7U */
} XNvm_RevocationId;

/** Enum for Offchip revocation IDs */
typedef enum {
	XNVM_EFUSE_INVLD = -1,			/**< -1 */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_0 = 0,	/**< 0U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_1,		/**< 1U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_2,		/**< 2U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_3,		/**< 3U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_4,		/**< 4U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_5,		/**< 5U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_6,		/**< 6U */
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_7		/**< 7U */
} XNvm_OffchipId;

/** Structure for Secure Ctrl Bits */
typedef struct {
	u8 AesDis;
	u8 JtagErrOutDis;
	u8 JtagDis;
	u8 HwTstBitsDis;
	u8 Ppk0WrLk;
	u8 Ppk1WrLk;
	u8 Ppk2WrLk;
	u8 AesCrcLk;
	u8 AesWrLk;
	u8 UserKey0CrcLk;
	u8 UserKey0WrLk;
	u8 UserKey1CrcLk;
	u8 UserKey1WrLk;
	u8 SecDbgDis;
	u8 SecLockDbgDis;
	u8 PmcScEn;
	u8 BootEnvWrLk;
	u8 RegInitDis;
#if defined (VERSAL_NET)
	u8 UdsWrLk;
#endif
} XNvm_EfuseSecCtrlBits;

/** Structure for Puf Secure Ctrl Bits */
typedef struct {
	u8 PufRegenDis;
	u8 PufHdInvalid;
	u8 PufTest2Dis;
	u8 PufDis;
	u8 PufSynLk;
#if defined (VERSAL_NET)
	u8 PufRegisDis;
#endif
} XNvm_EfusePufSecCtrlBits;

/** Structure for Misc Ctrl Bits */
typedef struct {
	u8 GlitchDetHaltBootEn;
	u8 GlitchDetRomMonitorEn;
	u8 HaltBootError;
	u8 HaltBootEnv;
	u8 CryptoKatEn;
	u8 LbistEn;
	u8 SafetyMissionEn;
	u8 Ppk0Invalid;
	u8 Ppk1Invalid;
	u8 Ppk2Invalid;
#ifdef XNVM_EN_ADD_PPKS
	u8 Ppk3Invalid;
	u8 Ppk4Invalid;
	u8 AdditionalPpkEn;
#endif
} XNvm_EfuseMiscCtrlBits;

/** Structure for SecMisc1 Ctrl Bits */
typedef struct {
	u8 LpdMbistEn;
	u8 PmcMbistEn;
	u8 LpdNocScEn;
	u8 SysmonVoltMonEn;
	u8 SysmonTempMonEn;
} XNvm_EfuseSecMisc1Bits;

/** Structure for BootEnvCtrl Bits */
typedef struct {
	u8 PrgmSysmonTempHot;
	u8 PrgmSysmonVoltPmc;
	u8 PrgmSysmonVoltPslp;
	u8 PrgmSysmonTempCold;
	u8 SysmonTempEn;
	u8 SysmonVoltEn;
	u8 SysmonVoltSoc;
	u8 SysmonTempHot;
	u8 SysmonVoltPmc;
	u8 SysmonVoltPslp;
	u8 SysmonTempCold;
} XNvm_EfuseBootEnvCtrlBits;

/** Structure for Glitch Config Bits */
typedef struct {
	u8 PrgmGlitch;
	u8 GlitchDetWrLk;
	u32 GlitchDetTrim;
	u8 GdRomMonitorEn;
	u8 GdHaltBootEn;
} XNvm_EfuseGlitchCfgBits;

/** Structure for Aes keys data */
typedef struct {
	u8 PrgmAesKey;
	u8 PrgmUserKey0;
	u8 PrgmUserKey1;
	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
} XNvm_EfuseAesKeys;

/** Struture for Ppk hash data */
typedef struct {
	u8 PrgmPpk0Hash;
	u8 PrgmPpk1Hash;
	u8 PrgmPpk2Hash;
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
} XNvm_EfusePpkHash;

/** Structure for IV data */
typedef struct {
	u8 PrgmMetaHeaderIv;
	u8 PrgmBlkObfusIv;
	u8 PrgmPlmIv;
	u8 PrgmDataPartitionIv;
	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 PlmIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS];
} XNvm_EfuseIvs;

/** Structure for DecryptOnly data */
typedef struct {
	u8 PrgmDecOnly;
} XNvm_EfuseDecOnly;

/** Structure for RevokeId data */
typedef struct {
	u32 PrgmRevokeId;
	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
} XNvm_EfuseRevokeIds;

/** Structure for OffchipId data */
typedef struct {
	u32 PrgmOffchipId;
	u32 OffChipId[XNVM_NUM_OF_OFFCHIP_ID_FUSES];
} XNvm_EfuseOffChipIds;

/** Structure for User eFuses data */
typedef struct {
	u32 StartUserFuseNum;
	u32 NumOfUserFuses;
	u64 UserFuseDataAddr;
} XNvm_EfuseUserDataAddr;

/** Structure for holding addresses of eFuse data structures */
typedef struct {
	u64 EnvMonDisFlag;
	u64 AesKeyAddr;
	u64 PpkHashAddr;
	u64 DecOnlyAddr;
	u64 SecCtrlAddr;
	u64 MiscCtrlAddr;
	u64 RevokeIdAddr;
	u64 IvAddr;
	u64 UserFuseAddr;
	u64 GlitchCfgAddr;
	u64 BootEnvCtrlAddr;
	u64 Misc1CtrlAddr;
	u64 OffChipIdAddr;
#ifdef XNVM_EN_ADD_PPKS
	u64 AdditionalPpkHashAddr;
#endif
} XNvm_EfuseDataAddr;

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_COMMON_DEFS_H */
