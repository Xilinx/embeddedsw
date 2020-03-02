/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_efuse.h
* @addtogroup xnvm_efuse_apis XilNvm eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains NVM library eFUSE APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
* 2.0 	kal  02/27/2020	Added Wrapper APIs for eFuse.
*
* </pre>
*
* @note
*
* @endcond
*******************************************************************************/
#ifndef XNVM_EFUSE_H
#define XNVM_EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/

#define XNvm_Printf(type,...)   if ((type) == (1U)) {xil_printf (__VA_ARGS__);}

/* Error codes for EFuse */
#define XNVM_EFUSE_ERR_INVALID_PARAM				(0x2U)
#define XNVM_EFUSE_ERR_RD					(0x3U)
#define XNVM_EFUSE_ERR_RD_TIMEOUT				(0x4U)
#define	XNVM_EFUSE_ERR_CACHE_PARITY				(0x5U)
#define XNVM_EFUSE_ERR_LOCK					(0x6U)
#define XNVM_EFUSE_ERR_UNLOCK					(0x7U)
#define XNVM_EFUSE_ERR_PGM_VERIFY				(0x8U)
#define XNVM_EFUSE_ERR_PGM					(0x9U)
#define XNVM_EFUSE_ERR_PGM_TIMEOUT				(0xAU)
#define XNVM_EFUSE_ERR_PGM_TBIT_PATTERN				(0xBU)
#define XNVM_EFUSE_ERR_CACHE_LOAD 				(0xCU)
#define XNVM_EFUSE_ERR_CRC_VERIFICATION				(0xDU)


#define XNVM_EFUSE_ERR_AES_ALREADY_PRGMD			(0x10U)
#define XNVM_EFUSE_ERR_USER_KEY0_ALREADY_PRGMD			(0x20U)
#define XNVM_EFUSE_ERR_USER_KEY1_ALREADY_PRGMD			(0x30U)
#define XNVM_EFUSE_ERR_PPK0_HASH_ALREADY_PRGMD			(0x40U)
#define XNVM_EFUSE_ERR_PPK1_HASH_ALREADY_PRGMD			(0x50U)
#define XNVM_EFUSE_ERR_PPK2_HASH_ALREADY_PRGMD			(0x60U)
#define XNVM_EFUSE_ERR_DEC_EFUSE_ONLY_ALREADY_PRGMD		(0x70U)
#define	XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD			(0x80U)
#define	XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD			(0x90U)
#define XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD			(0xA0U)
#define XNVM_EFUSE_ERR_BIT_CANT_REVERT				(0xF0U)

#define XNVM_EFUSE_ERR_WRITE_AES_KEY				(0x8000U)
#define XNVM_EFUSE_ERR_WRITE_USER0_KEY				(0x8100U)
#define	XNVM_EFUSE_ERR_WRITE_USER1_KEY				(0x8200U)
#define	XNVM_EFUSE_ERR_WRITE_PPK0_HASH				(0x8300U)
#define XNVM_EFUSE_ERR_WRITE_PPK1_HASH				(0x8400U)
#define XNVM_EFUSE_ERR_WRITE_PPK2_HASH				(0x8500U)
#define XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY			(0x8600U)
#define XNVM_EFUSE_ERR_WRITE_META_HEADER_IV			(0x8700U)
#define XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV			(0x8800U)
#define XNVM_EFUSE_ERR_WRITE_PLML_IV				(0x8900U)
#define XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV			(0x8A00U)
#define XNVM_EFUSE_ERR_WRTIE_AES_DIS				(0x8B00U)
#define XNVM_EFUSE_ERR_WRTIE_JTAG_ERROUT_DIS			(0x8C00U)
#define XNVM_EFUSE_ERR_WRTIE_JTAG_DIS				(0x8D00U)
#define XNVM_EFUSE_ERR_WRTIE_HWTSTBITS_DIS			(0x8E00U)
#define XNVM_EFUSE_ERR_WRTIE_IP_DIS_WR_LK			(0x8F00U)
#define XNVM_EFUSE_ERR_WRTIE_PPK0_WR_LK				(0x9000U)
#define XNVM_EFUSE_ERR_WRTIE_PPK1_WR_LK				(0x9100U)
#define XNVM_EFUSE_ERR_WRTIE_PPK2_WR_LK				(0x9200U)
#define XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_1			(0x9300U)
#define	XNVM_EFUSE_ERR_WRTIE_AES_CRC_LK_BIT_2			(0x9400U)
#define XNVM_EFUSE_ERR_WRTIE_AES_WR_LK				(0x9500U)
#define	XNVM_EFUSE_ERR_WRTIE_USER_KEY0_CRC_LK			(0x9600U)
#define XNVM_EFUSE_ERR_WRTIE_USER_KEY0_WR_LK			(0x9700U)
#define XNVM_EFUSE_ERR_WRTIE_USER_KEY1_CRC_LK			(0x9800U)
#define XNVM_EFUSE_ERR_WRTIE_USER_KEY1_WR_LK			(0x9900U)
#define XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_1			(0x9A00U)
#define XNVM_EFUSE_ERR_WRTIE_SECDBG_DIS_BIT_2			(0x9B00U)
#define XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_1		(0x9C00U)
#define XNVM_EFUSE_ERR_WRTIE_SECLOCKDBG_DIS_BIT_2		(0x9D00U)
#define XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_1			(0x9E00U)
#define XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_2			(0x9F00U)
#define XNVM_EFUSE_ERR_WRTIE_PMC_SC_EN_BIT_3			(0xA000U)
#define XNVM_EFUSE_ERR_WRTIE_SVD_WR_LK				(0xA100U)
#define XNVM_EFUSE_ERR_WRTIE_DNA_WR_LK				(0xA200U)
#define XNVM_EFUSE_ERR_WRTIE_BOOTENV_WR_LK			(0xA300U)
#define	XNVM_EFUSE_ERR_WRTIE_CACHE_WR_LK			(0xA400U)
#define XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_1			(0xA500U)
#define XNVM_EFUSE_ERR_WRTIE_REG_INIT_DIS_BIT_2			(0xA600U)
#define XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1			(0xA700U)
#define XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_2			(0xA800U)
#define XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1			(0xA900U)
#define XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_2			(0xAA00U)
#define XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1			(0xAB00U)
#define XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_2			(0xAC00U)

#define XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA			(0xB000U)
#define XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA			(0xB100U)
#define XNVM_EFUSE_ERR_WRITE_PUF_CHASH				(0xB200U)
#define XNVM_EFUSE_ERR_WRITE_PUF_AUX				(0xB300U)

#define XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS			(0xC000U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_0			(0xC100U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_1			(0xC200U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_2			(0xC300U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_3			(0xC400U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_4			(0xC500U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_5			(0xC600U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_6			(0xC700U)
#define XNVM_EFUSE_ERR_WRITE_REVOCATION_ID_7			(0xC800U)

#define XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS				(0xD000U)
#define XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS			(0xD100U)
#define XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL				(0xD200U)
#define XNVM_EFUSE_ERR_RD_PUF_SYN_DATA				(0xD300U)
#define XNVM_EFUSE_ERR_RD_PUF_CHASH				(0xD400U)
#define XNVM_EFUSE_ERR_RD_PUF_AUX				(0xD500U)
#define XNVM_EFUSE_ERR_RD_DNA					(0xD600U)
#define XNVM_EFUSE_ERR_RD_PPK_HASH				(0xD700U)
#define XNVM_EFUSE_ERR_RD_META_HEADER_IV			(0xD800U)
#define XNVM_EFUSE_ERR_RD_BLACK_OBFUS_IV			(0xD900U)
#define XNVM_EFUSE_ERR_RD_PLML_IV				(0xDA00U)
#define XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV			(0xDB00U)
#define XNVM_EFUSE_ERR_RD_DEC_ONLY				(0xDC00U)

#define XNVM_EFUSE_ERR_FUSE_PROTECTED				(0x40000U)
#define XNVM_EFUSE_ERR_BEFORE_PROGRAMMING			(0x80000U)

/* Key and Iv length definitions for Versal eFuse */
#define XNVM_EFUSE_AES_KEY_LEN_IN_BYTES				(32U)
#define XNVM_EFUSE_IV_LEN_IN_BYTES				(12U)
#define XNVM_EFUSE_IV_LEN_IN_BITS				(96U)

/* PPK definition for Versal eFuse */
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES			(32U)

/* PUF syndrome length definitions for Versal eFuse */
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS		(127U)
#define XNVM_EFUSE_PUF_AUX_LEN_IN_BITS				(24U)
#define XNVM_EFUSE_AES_KEY_LEN_IN_BITS				(256U)
#define XNVM_EFUSE_PPK_HASH_LEN_IN_BITS				(256U)

/* Versal eFuse maximum bits in a row */
#define XNVM_EFUSE_MAX_BITS_IN_ROW				(32U)

#define XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS			(8U)
#define XNVM_EFUSE_DNA_IN_WORDS					(4U)

#define XNVM_EFUSE_CRC_AES_ZEROS				(0x6858A3D5U)
/***************************** Type Definitions *******************************/

typedef enum {
	XNVM_EFUSE_RD_FROM_CACHE,
	XNVM_EFUSE_RD_FROM_EFUSE
}XNvm_EfuseRdOpt;

typedef enum {
	XNVM_EFUSE_PAGE_0,
	XNVM_EFUSE_PAGE_1,
	XNVM_EFUSE_PAGE_2
}XNvm_EfuseType;

typedef enum {
	XNVM_EFUSE_META_HEADER_IV_TYPE,
	XNVM_EFUSE_BLACK_OBFUS_IV_TYPE,
	XNVM_EFUSE_PLML_IV_TYPE,
	XNVM_EFUSE_DATA_PARTITION_IV_TYPE
}XNvm_IvType;

typedef enum {
	XNVM_EFUSE_PPK0,
	XNVM_EFUSE_PPK1,
	XNVM_EFUSE_PPK2
}XNvm_PpkType;

typedef enum {
	XNVM_EFUSE_REVOCATION_ID_0,
	XNVM_EFUSE_REVOCATION_ID_1,
	XNVM_EFUSE_REVOCATION_ID_2,
	XNVM_EFUSE_REVOCATION_ID_3,
	XNVM_EFUSE_REVOCATION_ID_4,
	XNVM_EFUSE_REVOCATION_ID_5,
	XNVM_EFUSE_REVOCATION_ID_6,
	XNVM_EFUSE_REVOCATION_ID_7
}XNvm_RevocationId;
typedef enum {
	XNVM_EFUSE_SEC_AES_DIS,
	XNVM_EFUSE_SEC_JTAG_ERROUT_DIS,
	XNVM_EFUSE_SEC_JTAG_DIS,
	XNVM_EFUSE_SEC_HWTSTBITS_DIS,
	XNVM_EFUSE_SEC_RESERVED,
	XNVM_EFUSE_SEC_IP_DIS_WRLK,
	XNVM_EFUSE_SEC_PPK0_WRLK,
	XNVM_EFUSE_SEC_PPK1_WRLK,
	XNVM_EFUSE_SEC_PPK2_WRLK,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_2,
	XNVM_EFUSE_SEC_AES_WRLK,
	XNVM_EFUSE_SEC_USER_KEY0_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY0_WRLK,
	XNVM_EFUSE_SEC_USER_KEY1_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY1_WRLK,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_2,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_2,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_3,
	XNVM_EFUSE_SEC_SVD_WRLK,
	XNVM_EFUSE_SEC_DNA_WRLK,
	XNVM_EFUSE_SEC_BOOTENV_WRLK,
	XNVM_EFUSE_SEC_CACHE_WRLK,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_2
}XNvm_SecCtrlBitColumns;

typedef enum {
	XNVM_EFUSE_MISC_RESERVED_0,
	XNVM_EFUSE_MISC_RESERVED_1,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_2,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_2,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_2,
	XNVM_EFUSE_MISC_SAFETY_MISSION_EN,
	XNVM_EFUSE_MISC_RESERVED_9,
	XNVM_EFUSE_MISC_RESERVED_10,
	XNVM_EFUSE_MISC_RESERVED_11,
	XNVM_EFUSE_MISC_RESERVED_12,
	XNVM_EFUSE_MISC_RESERVED_13,
	XNVM_EFUSE_LBIST_EN,
	XNVM_EFUSE_CRYPTO_KAT_EN,
	XNVM_EFUSE_MISC_RESERVED_16,
	XNVM_EFUSE_MISC_RESERVED_17,
	XNVM_EFUSE_MISC_RESERVED_18,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_1,
	XNVM_EFUSE_HALT_BOOT_ENV_BIT_2,
	XNVM_HALT_BOOT_ERROR_BIT_1,
	XNVM_HALT_BOOT_ERROR_BIT_2,
	XNVM_EFUSE_MISC_RESERVED_23,
        XNVM_EFUSE_MISC_RESERVED_24,
        XNVM_EFUSE_MISC_RESERVED_25,
        XNVM_EFUSE_MISC_RESERVED_26,
        XNVM_EFUSE_MISC_RESERVED_27,
	XNVM_EFUSE_MISC_RESERVED_28,
	XNVM_EFUSE_GD_ROM_MONITOR_EN,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_1,
	XNVm_EFUSE_GD_HALT_BOOT_EN_BIT_2
}XNvm_MiscCtrlBitColumns;

typedef struct {
	u8 AesDis;
	u8 JtagErrOutDis;
	u8 JtagDis;
	u8 HwTstBitsDis;
	u8 IpDisWrLk;
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
	u8 SvdWrLk;
	u8 DnaWrLk;
	u8 BootEnvWrLk;
	u8 CacherWrLk;
	u8 RegInitDis;
}Xnvm_SecCtrlBits;

typedef struct {
	u8 PufRegenDis;
	u8 PufHdInvalid;
	u8 PufTest2Dis;
	u8 PufDis;
	u8 PufSynLk;
}Xnvm_PufSecCtrlBits;

typedef struct {
	u8 SafetyMissionEn;
	u8 LbistEn;
	u8 CrytoKatEn;
	u8 HaltBootEnv;
	u8 HaltBootError;
	u8 GdRomMonitorEn;
	u8 GdHaltBootEn;
	u8 Ppk0Invalid;
	u8 Ppk1Invalid;
	u8 Ppk2Invalid;
}Xnvm_MiscCtrlBits;

typedef struct {
	u8 SysmonTempMonEn;
	u8 SysmonVoltMonEn;
	u8 LpdNocScEn;
	u8 PmcMbistEn;
	u8 LpdMbistEn;
	u16 DpaMaskDis;
}Xnvm_SecurityMisc1Bits;

typedef struct {
	u8 AesKey;
	u8 UserKey0;
	u8 UserKey1;
	u8 Ppk0Hash;
	u8 Ppk1Hash;
	u8 Ppk2Hash;
	u8 DecOnly;
}Xnvm_WriteFlags;

typedef struct {
	Xnvm_SecCtrlBits PrgmSecCtrlFlags;
	Xnvm_WriteFlags	CheckWriteFlags;

	u32 AesKey[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 UserKey0[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 UserKey1[XNVM_EFUSE_AES_KEY_LEN_IN_BYTES / 4];
	u32 Ppk0Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 Ppk1Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 Ppk2Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_BYTES / 4];
	u32 DecEfuseOnly;
	Xnvm_SecCtrlBits ReadBackSecCtrlBits;
}Xnvm_EfuseWriteData;

typedef struct {
	u8 RevokeId0;
	u8 RevokeId1;
	u8 RevokeId2;
	u8 RevokeId3;
	u8 RevokeId4;
	u8 RevokeId5;
	u8 RevokeId6;
	u8 RevokeId7;
}Xnvm_RevokeIdFlags;

typedef struct {
	Xnvm_RevokeIdFlags CheckSpkEfuseToRevoke;
	u32 RevokeId0;
	u32 RevokeId1;
	u32 RevokeId2;
	u32 RevokeId3;
	u32 RevokeId4;
	u32 RevokeId5;
	u32 RevokeId6;
	u32 RevokeId7;
}Xnvm_RevokeIdEfuse;

typedef struct {
	u8 Ppk0;
	u8 Ppk1;
	u8 Ppk2;
}Xnvm_RevokePpkFlags;

typedef struct {
	Xnvm_PufSecCtrlBits ReadPufSecCtrlBits;
	u8 PrgmChash;
	u8 PrgmAux;
	u8 PrgmSynData;

	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}Xnvm_PufHelperData;

typedef struct {
	u8 PgrmMetaHeaderIv;
	u8 PgrmBlkObfusIv;
	u8 PgmPlmlIv;
	u8 PgmDataPartitionIv;


	u32 Iv[XNVM_EFUSE_IV_LEN_IN_BYTES / 4];
}Xnvm_Iv;

typedef struct {
	u32 Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
}Xnvm_PpkHash;

typedef struct {
	u32 Dna[XNVM_EFUSE_DNA_IN_WORDS];
}Xnvm_Dna;

/*************************** Function Prototypes ******************************/
u32 XNvm_EfuseWrite(Xnvm_EfuseWriteData *WriteNvm);
u32 XNvm_EfuseReadSecCtrlBits(Xnvm_SecCtrlBits *ReadbackSecCtrlBits);
u32 XNvm_EfuseWritePufHelperData(Xnvm_PufHelperData *PrgmPufHelperData);
u32 XNvm_EfuseReadPufSecCtrlBits(Xnvm_PufSecCtrlBits *ReadBackPufSecCtrlBits);
u32 XNvm_EfuseWriteIv(Xnvm_Iv *EfuseIv, XNvm_IvType IvType);
u32 XNvm_EfuseReadIv(Xnvm_Iv *EfuseIv, XNvm_IvType IvType);
u32 XNvm_EfuseReadPufHelperData(Xnvm_PufHelperData *PrgmPufHelperData);
u32 XNvm_EfuseReadDna(Xnvm_Dna *EfuseDna);
u32 XNvm_EfuseReadPpkHash(Xnvm_PpkHash *EfusePpk, XNvm_PpkType PpkType);
u32 Xnvm_EfuseRevokePpk(Xnvm_RevokePpkFlags *PpkRevoke);
u32 Xnvm_EfuseRevokeSpk(Xnvm_RevokeIdEfuse *WriteRevokeId);
u32 XNvm_EfuseReadMiscCtrlBits(Xnvm_MiscCtrlBits *ReadMiscCtrlBits);
u32 XNvm_EfuseReadDecOnly(u32* DecOnly);
u32 Xnvm_EfuseReadRevocationFuse(u32 *RevokeFusePtr, u8 RevokeFuse_Num);
u32 XNvm_EfuseCheckAesKeyCrc(u32 Crc);
u32 XNvm_EfuseCheckUserKey0Crc(u32 Crc);
u32 XNvm_EfuseCheckUserKey1Crc(u32 Crc);

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
