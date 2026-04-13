/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * @addtogroup xnvm_def_api_ids XilNvm Definitions
 * @{
 */

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
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS]; /**< PPK hash data */
} XNvm_PpkHash;

/** Structure for IV data */
typedef struct {
	u32 Iv[XNVM_EFUSE_IV_LEN_IN_WORDS];	/**< IV data */
} XNvm_Iv;

/** Structure for Aes key data */
typedef struct {
	u32 Key[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS]; /**< AES key data */
} XNvm_AesKey;

/** Structure for DNA data */
typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_LEN_IN_WORDS];	/**< DNA data */
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
	u8 AesDis;	/**< AES disable */
	u8 JtagErrOutDis; /**< JTAG error output disable */
	u8 JtagDis;	/**< JTAG disable */
	u8 HwTstBitsDis; /**< Hardware test bits disable */
	u8 Ppk0WrLk;	/**< PPK0 write lock */
	u8 Ppk1WrLk;	/**< PPK1 write lock */
	u8 Ppk2WrLk;	/**< PPK2 write lock */
	u8 AesCrcLk;	/**< AES CRC lock */
	u8 AesWrLk;	/**< AES write lock */
	u8 UserKey0CrcLk; /**< User key 0 CRC lock */
	u8 UserKey0WrLk; /**< User key 0 write lock */
	u8 UserKey1CrcLk; /**< User key 1 CRC lock */
	u8 UserKey1WrLk; /**< User key 1 write lock */
	u8 SecDbgDis;	/**< Secure debug disable */
	u8 SecLockDbgDis; /**< Secure lock debug disable */
	u8 PmcScEn;	/**< PMC secure control enable */
	u8 BootEnvWrLk;	/**< Boot environment write lock */
	u8 RegInitDis;	/**< Register initialization disable */
#if defined (VERSAL_NET)
	u8 UdsWrLk;	/**< UDS write lock */
#endif
} XNvm_EfuseSecCtrlBits;

/** Structure for Puf Secure Ctrl Bits */
typedef struct {
	u8 PufRegenDis;	/**< PUF regeneration disable */
	u8 PufHdInvalid; /**< PUF HD invalid */
	u8 PufTest2Dis;	/**< PUF test 2 disable */
	u8 PufDis;	/**< PUF disable */
	u8 PufSynLk;	/**< PUF synchronization lock */
#if defined (VERSAL_NET)
	u8 PufRegisDis;	/**< PUF registration disable */
#endif
} XNvm_EfusePufSecCtrlBits;

/** Structure for Misc Ctrl Bits */
typedef struct {
	u8 GlitchDetHaltBootEn;	/**< Glitch detection halt boot enable */
	u8 GlitchDetRomMonitorEn; /**< Glitch detection ROM monitor enable */
	u8 HaltBootError;	/**< Halt boot on error */
	u8 HaltBootEnv;		/**< Halt boot on environment */
	u8 CryptoKatEn;		/**< Crypto KAT enable */
	u8 LbistEn;		/**< LBIST enable */
	u8 SafetyMissionEn;	/**< Safety mission enable */
	u8 Ppk0Invalid;		/**< PPK0 invalid */
	u8 Ppk1Invalid;		/**< PPK1 invalid */
	u8 Ppk2Invalid;		/**< PPK2 invalid */
#ifdef XNVM_EN_ADD_PPKS
	u8 Ppk3Invalid;		/**< PPK3 invalid */
	u8 Ppk4Invalid;		/**< PPK4 invalid */
	u8 AdditionalPpkEn;	/**< Additional PPK enable */
#endif
} XNvm_EfuseMiscCtrlBits;

/** Structure for SecMisc1 Ctrl Bits */
typedef struct {
	u8 LpdMbistEn;	/**< LPD MBIST enable */
	u8 PmcMbistEn;	/**< PMC MBIST enable */
	u8 LpdNocScEn;	/**< LPD NOC secure control enable */
	u8 SysmonVoltMonEn; /**< System monitor voltage monitor enable */
	u8 SysmonTempMonEn; /**< System monitor temperature monitor enable */
} XNvm_EfuseSecMisc1Bits;

/** Structure for BootEnvCtrl Bits */
typedef struct {
	u8 PrgmSysmonTempHot;	/**< Program system monitor temperature hot threshold */
	u8 PrgmSysmonVoltPmc;	/**< Program system monitor voltage PMC threshold */
	u8 PrgmSysmonVoltPslp;	/**< Program system monitor voltage PSLP threshold */
	u8 PrgmSysmonTempCold;	/**< Program system monitor temperature cold threshold */
	u8 SysmonTempEn;	/**< System monitor temperature enable */
	u8 SysmonVoltEn;	/**< System monitor voltage enable */
	u8 SysmonVoltSoc;	/**< System monitor voltage SOC */
	u8 SysmonTempHot;	/**< System monitor temperature hot */
	u8 SysmonVoltPmc;	/**< System monitor voltage PMC */
	u8 SysmonVoltPslp;	/**< System monitor voltage PSLP */
	u8 SysmonTempCold;	/**< System monitor temperature cold */
} XNvm_EfuseBootEnvCtrlBits;

/** Structure for Glitch Config Bits */
typedef struct {
	u8 PrgmGlitch;		/**< Program glitch configuration */
	u8 GlitchDetWrLk;	/**< Glitch detection write lock */
	u32 GlitchDetTrim;	/**< Glitch detection trim */
	u8 GdRomMonitorEn;	/**< Glitch detection ROM monitor enable */
	u8 GdHaltBootEn;	/**< Glitch detection halt boot enable */
} XNvm_EfuseGlitchCfgBits;

/** Structure for Aes keys data */
typedef struct {
	u8 PrgmAesKey;		/**< Program AES key */
	u8 PrgmUserKey0;	/**< Program user key 0 */
	u8 PrgmUserKey1;	/**< Program user key 1 */
	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];	/**< AES key data */
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];	/**< User key 0 data */
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];	/**< User key 1 data */
} XNvm_EfuseAesKeys;

/** Structure for Ppk hash data */
typedef struct {
	u8 PrgmPpk0Hash;	/**< Program PPK0 hash */
	u8 PrgmPpk1Hash;	/**< Program PPK1 hash */
	u8 PrgmPpk2Hash;	/**< Program PPK2 hash */
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];	/**< PPK0 hash data */
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];	/**< PPK1 hash data */
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];	/**< PPK2 hash data */
} XNvm_EfusePpkHash;

/** Structure for IV data */
typedef struct {
	u8 PrgmMetaHeaderIv;	/**< Program Meta Header IV */
	u8 PrgmBlkObfusIv;	/**< Program Block Obfuscation IV */
	u8 PrgmPlmIv;		/**< Program PLM IV */
	u8 PrgmDataPartitionIv;	/**< Program Data Partition IV */
	u32 MetaHeaderIv[XNVM_EFUSE_IV_LEN_IN_WORDS];	/**< Meta Header IV data */
	u32 BlkObfusIv[XNVM_EFUSE_IV_LEN_IN_WORDS];	/**< Block Obfuscation IV data */
	u32 PlmIv[XNVM_EFUSE_IV_LEN_IN_WORDS];		/**< PLM IV data */
	u32 DataPartitionIv[XNVM_EFUSE_IV_LEN_IN_WORDS]; /**< Data Partition IV data */
} XNvm_EfuseIvs;

/** Structure for DecryptOnly data */
typedef struct {
	u8 PrgmDecOnly;	/**< Program decrypt only */
} XNvm_EfuseDecOnly;

/** Structure for RevokeId data */
typedef struct {
	u32 PrgmRevokeId;	/**< Program revoke ID */
	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];	/**< Revoke ID data */
} XNvm_EfuseRevokeIds;

/** Structure for OffchipId data */
typedef struct {
	u32 PrgmOffchipId;	/**< Program offchip ID */
	u32 OffChipId[XNVM_NUM_OF_OFFCHIP_ID_FUSES];	/**< Offchip ID data */
} XNvm_EfuseOffChipIds;

/** Structure for User eFuses data */
typedef struct {
	u32 StartUserFuseNum;	/**< Starting user fuse number to program/read */
	u32 NumOfUserFuses;	/**< Number of user fuses to program/read */
	u64 UserFuseDataAddr;	/**< Address of user fuse data */
} XNvm_EfuseUserDataAddr;

/** Structure for holding addresses of eFuse data structures */
typedef struct {
	u64 EnvMonDisFlag;	/**< Environment monitor disable flag */
	u64 AesKeyAddr;		/**< AES key address */
	u64 PpkHashAddr;	/**< PPK hash address */
	u64 DecOnlyAddr;	/**< Decrypt only address */
	u64 SecCtrlAddr;	/**< Security control address */
	u64 MiscCtrlAddr;	/**< Miscellaneous control address */
	u64 RevokeIdAddr;	/**< Revoke ID address */
	u64 IvAddr;		/**< IV address */
	u64 UserFuseAddr;	/**< User fuse address */
	u64 GlitchCfgAddr;	/**< Glitch configuration address */
	u64 BootEnvCtrlAddr;	/**< Boot environment control address */
	u64 Misc1CtrlAddr;	/**< Miscellaneous 1 control address */
	u64 OffChipIdAddr;	/**< Offchip ID address */
#ifdef XNVM_EN_ADD_PPKS
	u64 AdditionalPpkHashAddr;	/**< Additional PPK hash address */
#endif
} XNvm_EfuseDataAddr;

/** Structure for DME Revoke ID data */
typedef struct {
	u8 DmeRevoke0;	/**< DME revoke ID 0 */
	u8 DmeRevoke1;	/**< DME revoke ID 1 */
	u8 DmeRevoke2;	/**< DME revoke ID 2 */
	u8 DmeRevoke3;	/**< DME revoke ID 3 */
} XNvm_EfuseDmeRevokeId;

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_COMMON_DEFS_H */
/** @} */
