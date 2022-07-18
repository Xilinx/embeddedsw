/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_defs.h
* @addtogroup xnvm_api_ids XilNvm API IDs
* @{
*
* @cond xnvm_internal
* This file contains the xilnvm API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
*       kal  07/25/21 Added eFUSE IPI API_IDs and common structures between
*                     client and server
*       kpt  08/27/21 Added client-server support for puf helper data efuse
*                     programming
* 1.1   kpt  11/29/21 Added macro XNvm_DCacheFlushRange
*       har  01/03/22 Renamed NumOfPufFuses as NumOfPufFusesRows
*       am   02/28/22 Fixed MISRA C violation rule 4.5
*       kpt  03/03/22 Fixed alignment issue in XNvm_EfusePufFuseAddr
*                     by rearranging the structure elements
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

/* Key and Iv length definitions for Versal eFuse */
#define XNVM_EFUSE_DNA_IN_WORDS				(4U)
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)
#define XNVM_NUM_OF_REVOKE_ID_FUSES			(8U)
#define XNVM_NUM_OF_OFFCHIP_ID_FUSES			(8U)
#define XNVM_EFUSE_IV_LEN_IN_BITS			(96U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS			(256U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS			(256U)
#define XNVM_EFUSE_IV_LEN_IN_BYES			(12U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES                 (32U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES		(32U)
#define XNVM_IV_STRING_LEN              		(24U)

/***************** Macros (Inline Functions) Definitions *********************/
#define XNvm_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}

#ifndef XNVM_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
	#else
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
	#endif
#else
	#define XNvm_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

/* Macro to typecast XILSECURE API ID */
#define XNVM_API(ApiId)	((u32)ApiId)

#define XNVM_API_ID_MASK	(0xFFU)

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_IN_WORDS];
} XNvm_Dna;

typedef enum {
	XNVM_EFUSE_REVOCATION_ID_0 = 0,
	XNVM_EFUSE_REVOCATION_ID_1,
	XNVM_EFUSE_REVOCATION_ID_2,
	XNVM_EFUSE_REVOCATION_ID_3,
	XNVM_EFUSE_REVOCATION_ID_4,
	XNVM_EFUSE_REVOCATION_ID_5,
	XNVM_EFUSE_REVOCATION_ID_6,
	XNVM_EFUSE_REVOCATION_ID_7
} XNvm_RevocationId;

typedef enum {
	XNVM_EFUSE_INVLD = -1,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_0 = 0,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_1,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_2,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_3,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_4,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_5,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_6,
	XNVM_EFUSE_OFFCHIP_REVOKE_ID_7
} XNvm_OffchipId;

typedef struct {
	u8 AesDis;
	u8 JtagErrOutDis;
	u8 JtagDis;
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
	u8 BootEnvWrLk;
	u8 RegInitDis;
} XNvm_EfuseSecCtrlBits;

typedef struct {
	u8 PufRegenDis;
	u8 PufHdInvalid;
	u8 PufTest2Dis;
	u8 PufDis;
	u8 PufSynLk;
} XNvm_EfusePufSecCtrlBits;

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
} XNvm_EfuseMiscCtrlBits;

typedef struct {
	u8 LpdMbistEn;
	u8 PmcMbistEn;
	u8 LpdNocScEn;
	u8 SysmonVoltMonEn;
	u8 SysmonTempMonEn;
} XNvm_EfuseSecMisc1Bits;

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

typedef struct {
	u8 PrgmGlitch;
	u8 GlitchDetWrLk;
	u32 GlitchDetTrim;
	u8 GdRomMonitorEn;
	u8 GdHaltBootEn;
} XNvm_EfuseGlitchCfgBits;

typedef struct {
	u8 PrgmAesKey;
	u8 PrgmUserKey0;
	u8 PrgmUserKey1;
	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_WORDS];
} XNvm_EfuseAesKeys;

typedef struct {
	u8 PrgmPpk0Hash;
	u8 PrgmPpk1Hash;
	u8 PrgmPpk2Hash;
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
} XNvm_EfusePpkHash;

typedef struct {
	u8 PrgmDecOnly;
} XNvm_EfuseDecOnly;

typedef struct {
	u8 PrgmRevokeId;
	u32 RevokeId[XNVM_NUM_OF_REVOKE_ID_FUSES];
} XNvm_EfuseRevokeIds;

typedef struct {
	u8 PrgmOffchipId;
	u32 OffChipId[XNVM_NUM_OF_OFFCHIP_ID_FUSES];
} XNvm_EfuseOffChipIds;

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

typedef struct {
	u32 StartUserFuseNum;
	u32 NumOfUserFuses;
	u64 UserFuseDataAddr;
} XNvm_EfuseUserDataAddr;

#ifdef XNVM_ACCESS_PUF_USER_DATA
typedef struct {
	u64 PufFuseDataAddr;
	u32 StartPufFuseRow;
	u32 NumOfPufFusesRows;
	u8 EnvMonitorDis;
	u8 PrgmPufFuse;
} XNvm_EfusePufFuseAddr;
#else
typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	u8 PrgmPufHelperData;
	u8 EnvMonitorDis;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}XNvm_EfusePufHdAddr;
#endif

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
} XNvm_EfuseDataAddr;

/* XilNVM API ids */
typedef enum {
	XNVM_API_FEATURES = 0,
	XNVM_API_ID_BBRAM_WRITE_AES_KEY,
	XNVM_API_ID_BBRAM_ZEROIZE,
	XNVM_API_ID_BBRAM_WRITE_USER_DATA,
	XNVM_API_ID_BBRAM_READ_USER_DATA,
	XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA,
	XNVM_API_ID_EFUSE_WRITE,
	XNVM_API_ID_EFUSE_WRITE_PUF,
	XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE,
	XNVM_API_ID_EFUSE_READ_IV,
	XNVM_API_ID_EFUSE_READ_REVOCATION_ID,
	XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID,
	XNVM_API_ID_EFUSE_READ_USER_FUSES,
	XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS,
	XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_PPK_HASH,
	XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY,
	XNVM_API_ID_EFUSE_READ_DNA,
	XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE,
	XNVM_API_ID_EFUSE_READ_PUF,
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
