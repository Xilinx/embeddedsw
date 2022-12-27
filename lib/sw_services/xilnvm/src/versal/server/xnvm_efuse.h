/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file versal/server/xnvm_efuse.h
* @addtogroup xnvm_versal_efuse_apis XilNvm Versal eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains function declarations of eFuse APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
* 2.0 	kal  02/27/2020	Added eFuse wrapper APIs to program AES keys, PPK hash,
*                       Revocation ID, SecCtrl eFuses, Puf HD and APIs to read
*                       eFuse Cache values.
*       kal  03/03/2020 Added protection eFuse row programming.
* 2.1   rpo  06/06/2020 Support added to write glitch configuration data.
*       rpo  06/08/2020 Support added to program eFuse halt boot bits to stop
*                       at ROM stage.
* 	am   08/19/2020 Resolved MISRA C violations.
* 	kal  09/03/2020 Fixed Security CoE review comments
*	am   10/13/2020 Resolved MISRA C violations
*	ana  10/15/2020 Updated doxygen comments.
* 2.3	kal  01/07/2021	Added support to SecurityMisc1, BootEnvCtrl,MiscCtrl
*			and remaining eFuses in SecCtrl eFuse rows programming
*			and reading
*	kal  01/28/2021 Added new error code for glitch detection
*	kal  02/20/2021 Added new error codes for detecting voltage and
*			temparature out of range cases
*	har  04/21/2021 Fixed warnings for R5 processor
*       kpt  05/12/2021 Added sysmon instance to the function prototype of
*                   individual write API's
*	kpt  05/20/2021 Added support for programming PUF efuses as
*					 general purpose data
* 2.4   kal  07/25/2021 Moved common structures between client and server
*                       to xnvm_defs.h
* 2.5   har  01/03/2022 Renamed NumOfPufFuses as NumOfPufFusesRows
* 3.0	kal  07/12/2022	Moved common code to xnvm_efuse_common.h
* 3.1   skg  10/25/2022 Added comments for macros
*       skg  12/07/2022 Added Additional PPKs related enums and macros
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
#include "xsysmonpsv.h"
#include "xnvm_defs.h"
#include "xnvm_efuse_common.h"

/*************************** Constant Definitions *****************************/

/**< PUF syndrome length definations for Versal eFuse*/
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)
#define XNVM_PUF_ROW_UPPER_NIBBLE_MASK              (0xF0000000U)

/**< User efuses start, end and number of efuses definations*/
#define XNVM_USER_FUSE_START_NUM			(1U)
#define XNVM_USER_FUSE_END_NUM				(63U)
#define XNVM_NUM_OF_USER_FUSES				(63U)

/**< Maximum revoke id defination for versal*/
#define XNVM_MAX_REVOKE_ID_FUSES			(XNVM_NUM_OF_REVOKE_ID_FUSES	\
											* XNVM_EFUSE_MAX_BITS_IN_ROW)
/**
 *  @{ Temaperature limits defination for versal Efuses
 */
#define XNVM_EFUSE_FULL_RANGE_TEMP_MIN	(-55.0f)
#define XNVM_EFUSE_FULL_RANGE_TEMP_MAX	(125.0f)

#define XNVM_EFUSE_TEMP_LP_MIN		(0.0f)
#define XNVM_EFUSE_TEMP_LP_MAX		(100.0f)
#define XNVM_EFUSE_TEMP_MP_MIN		(-40.0f)
#define XNVM_EFUSE_TEMP_MP_MAX		(110.0f)
#define XNVM_EFUSE_TEMP_HP_MIN		(-55.0f)
#define XNVM_EFUSE_TEMP_HP_MAX		(125.0f)

/**< eFuse Range check  definations*/
#define XNVM_EFUSE_FULL_RANGE_CHECK		(0U)
#define XNVM_EFUSE_LP_RANGE_CHECK		(1U)
#define XNVM_EFUSE_MP_RANGE_CHECK		(2U)
#define XNVM_EFUSE_HP_RANGE_CHECK		(3U)

/**< eFuse volatage limits definations*/
#define XNVM_EFUSE_VCC_PMC_LP_MIN		(0.676f)
#define XNVM_EFUSE_VCC_PMC_LP_MAX		(0.724f)
#define XNVM_EFUSE_VCC_PMC_MP_MIN		(0.775f)
#define XNVM_EFUSE_VCC_PMC_MP_MAX		(0.825f)
#define XNVM_EFUSE_VCC_PMC_HP_MIN		(0.854f)
#define XNVM_EFUSE_VCC_PMC_HP_MAX		(0.906f)

#define XNVM_EFUSE_SYSMON_LOCK_CODE	(0xF9E8D7C6U)

#define XNVM_EFUSE_PPK_READ_START XNVM_EFUSE_PPK0
#ifdef XNVM_EN_ADD_PPKS
#define XNVM_EFUSE_PPK_READ_END XNVM_EFUSE_PPK4
#else
#define XNVM_EFUSE_PPK_READ_END XNVM_EFUSE_PPK2
#endif
/**
* @}
*/

/***************************** Type Definitions *******************************/


/**
 * @{ eFuse control bits
 */
 /**< This structer defines Security control bits*/
typedef enum {
	XNVM_EFUSE_SEC_AES_DIS = 0,
	XNVM_EFUSE_SEC_JTAG_ERROUT_DIS,
	XNVM_EFUSE_SEC_JTAG_DIS,
	XNVM_EFUSE_SEC_HWTSTBITS_DIS,
	XNVM_EFUSE_SEC_IP_DIS_WRLK = 5,
	XNVM_EFUSE_SEC_PPK0_WRLK,
	XNVM_EFUSE_SEC_PPK1_WRLK,
	XNVM_EFUSE_SEC_PPK2_WRLK,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_0,
	XNVM_EFUSE_SEC_AES_CRC_LK_BIT_1,
	XNVM_EFUSE_SEC_AES_WRLK,
	XNVM_EFUSE_SEC_USER_KEY0_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY0_WRLK,
	XNVM_EFUSE_SEC_USER_KEY1_CRC_LK,
	XNVM_EFUSE_SEC_USER_KEY1_WRLK,
	XNVM_EFUSE_SEC_PUF_SYN_LK,
	XNVM_EFUSE_SEC_PUF_TEST2_DIS,
	XNVM_EFUSE_SEC_PUF_DIS,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_0,
	XNVM_EFUSE_SEC_SECDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_0,
	XNVM_EFUSE_SEC_SECLOCKDBG_DIS_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_0,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_1,
	XNVM_EFUSE_SEC_PMC_SC_EN_BIT_2,
	XNVM_EFUSE_SEC_SVD_WRLK,
	XNVM_EFUSE_SEC_DNA_WRLK,
	XNVM_EFUSE_SEC_BOOTENV_WRLK,
	XNVM_EFUSE_SEC_CACHE_WRLK,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_0,
	XNVM_EFUSE_SEC_REG_INIT_DIS_BIT_1
}XNvm_SecCtrlBitColumns;

/**< This enum defines Miscellaneous control bits*/
typedef enum {
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_0 = 2,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1,
	XNVM_EFUSE_MISC_SAFETY_MISSION_EN,
#ifdef XNVM_EN_ADD_PPKS
	XNVM_EFUSE_MISC_PPK3_INVALID_BIT_0 = 9,
	XNVM_EFUSE_MISC_PPK3_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK4_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK4_INVALID_BIT_1,
#endif /* END OF XNVM_EN_ADD_PPKS */
	XNVM_EFUSE_MISC_LBIST_EN = 14,
	XNVM_EFUSE_MISC_CRYPTO_KAT_EN,
	XNVM_EFUSE_MISC_HALT_BOOT_ENV_BIT_0 = 19,
	XNVM_EFUSE_MISC_HALT_BOOT_ENV_BIT_1,
	XNVM_EFUSE_MISC_HALT_BOOT_ERROR_BIT_0,
	XNVM_EFUSE_MISC_HALT_BOOT_ERROR_BIT_1,
	XNVM_EFUSE_MISC_GD_ROM_MONITOR_EN = 29,
	XNVm_EFUSE_MISC_GD_HALT_BOOT_EN_BIT_0,
	XNVm_EFUSE_MISC_GD_HALT_BOOT_EN_BIT_1
}XNvm_MiscCtrlBitColumns;

/**< user efuses details*/
typedef struct {
	u32 StartUserFuseNum;
	u32 NumOfUserFuses;
	u32 *UserFuseData;
}XNvm_EfuseUserData;

#ifdef XNVM_ACCESS_PUF_USER_DATA
typedef struct {
	u8 EnvMonitorDis;
	u8 PrgmPufFuse;
	XSysMonPsv *SysMonInstPtr;
	u32 StartPufFuseRow;
	u32 NumOfPufFusesRows;
	u32 *PufFuseData;
}XNvm_EfusePufFuse;
#endif

/**< Defines Puf helper data*/
typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	u8 PrgmPufHelperData;
	u8 EnvMonitorDis;
	XSysMonPsv *SysMonInstPtr;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}XNvm_EfusePufHd;

/**< This structure defines sub structures of versal to be blown*/
typedef struct {
	u8 EnvMonitorDis;
	XSysMonPsv *SysMonInstPtr;
	XNvm_EfuseAesKeys *AesKeys;
	XNvm_EfusePpkHash *PpkHash;
	XNvm_EfuseDecOnly *DecOnly;
	XNvm_EfuseSecCtrlBits *SecCtrlBits;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits;
	XNvm_EfuseRevokeIds *RevokeIds;
	XNvm_EfuseIvs *Ivs;
	XNvm_EfuseUserData *UserFuses;
	XNvm_EfuseGlitchCfgBits *GlitchCfgBits;
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrl;
	XNvm_EfuseSecMisc1Bits *Misc1Bits;
	XNvm_EfuseOffChipIds *OffChipIds;
#ifdef XNVM_EN_ADD_PPKS
	XNvm_EfuseAdditionalPpkHash *AdditionalPpkHash;
#endif /* END OF XNVM_EN_ADD_PPKS*/
}XNvm_EfuseData;

/*************************** Function Prototypes ******************************/
int XNvm_EfuseWrite(const XNvm_EfuseData *WriteNvm);
int XNvm_EfuseWriteIVs(XNvm_EfuseIvs *EfuseIv, XSysMonPsv *SysMonInstPtr);
int XNvm_EfuseRevokePpk(XNvm_PpkType PpkRevoke, XSysMonPsv *SysMonInstPtr);
int XNvm_EfuseWriteRevocationId(u32 RevokeId, XSysMonPsv *SysMonInstPtr);
int XNvm_EfuseWriteUserFuses(XNvm_EfuseUserData *WriteUserFuses,
	XSysMonPsv *SysMonInstPtr);
int XNvm_EfuseReadIv(XNvm_Iv *EfuseIv, XNvm_IvType IvType);
int XNvm_EfuseReadRevocationId(u32 *RevokeFusePtr,
	XNvm_RevocationId RevokeFuseNum);
int XNvm_EfuseReadUserFuses(const XNvm_EfuseUserData *UserFusesData);
int XNvm_EfuseReadMiscCtrlBits(XNvm_EfuseMiscCtrlBits *MiscCtrlBits);
int XNvm_EfuseReadSecCtrlBits(XNvm_EfuseSecCtrlBits *SecCtrlBits);
int XNvm_EfuseReadPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType);
int XNvm_EfuseReadDecOnly(u32* DecOnly);
int XNvm_EfuseReadDna(XNvm_Dna *EfuseDna);
#ifndef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseWritePuf(const XNvm_EfusePufHd *PufHelperData);
#endif
int XNvm_EfuseReadPuf(XNvm_EfusePufHd *PufHelperData);
int XNvm_EfuseReadPufSecCtrlBits(XNvm_EfusePufSecCtrlBits *PufSecCtrlBits);
int XNvm_EfuseReadSecMisc1Bits(XNvm_EfuseSecMisc1Bits *SecMisc1Bits);
int XNvm_EfuseReadBootEnvCtrlBits(XNvm_EfuseBootEnvCtrlBits *BootEnvCtrlBits);
int XNvm_EfuseReadOffchipRevokeId(u32 *OffchipIdPtr,
	XNvm_OffchipId OffchipIdNum);
#ifdef XNVM_ACCESS_PUF_USER_DATA
int XNvm_EfuseWritePufAsUserFuses(XNvm_EfusePufFuse *PufFuse);
int XNvm_EfuseReadPufAsUserFuses(XNvm_EfusePufFuse *PufFuse);
#endif

#ifdef XNVM_EN_ADD_PPKS
int XNvm_EfuseReadAdditionalPpkHash(XNvm_PpkHash *EfusePpk, XNvm_PpkType PpkType);
#endif /* END OF XNVM_EN_ADD_PPKS*/
#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
