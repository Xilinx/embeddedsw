/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse.h
* @addtogroup xnvm_efuse_apis XilNvm eFuse APIs
* @{
*
* @cond xnvm_internal
* This file contains function declarations of eFUSE APIs
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
*       rpo  06/08/2020 Support added to program eFUSE halt boot bits to stop
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
* 2.6	kal  12/07/2022	Moved common code to xnvm_efuse_common.h
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
/**@cond xnvm_internal
 * @{
 */

/* PUF syndrome length definitions for Versal eFuse */
#define XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS	(127U)
#define XNVM_PUF_ROW_UPPER_NIBBLE_MASK              (0xF0000000U)

#define XNVM_USER_FUSE_START_NUM			(1U)
#define XNVM_USER_FUSE_END_NUM				(63U)
#define XNVM_NUM_OF_USER_FUSES				(63U)

#define XNVM_MAX_REVOKE_ID_FUSES			(XNVM_NUM_OF_REVOKE_ID_FUSES	\
											* XNVM_EFUSE_MAX_BITS_IN_ROW)
#define XNVM_EFUSE_FULL_RANGE_TEMP_MIN	(-55.0f)
#define XNVM_EFUSE_FULL_RANGE_TEMP_MAX	(125.0f)

#define XNVM_EFUSE_TEMP_LP_MIN		(0.0f)
#define XNVM_EFUSE_TEMP_LP_MAX		(100.0f)
#define XNVM_EFUSE_TEMP_MP_MIN		(-40.0f)
#define XNVM_EFUSE_TEMP_MP_MAX		(110.0f)
#define XNVM_EFUSE_TEMP_HP_MIN		(-55.0f)
#define XNVM_EFUSE_TEMP_HP_MAX		(125.0f)

#define XNVM_EFUSE_FULL_RANGE_CHECK		(0U)
#define XNVM_EFUSE_LP_RANGE_CHECK		(1U)
#define XNVM_EFUSE_MP_RANGE_CHECK		(2U)
#define XNVM_EFUSE_HP_RANGE_CHECK		(3U)

#define XNVM_EFUSE_VCC_PMC_LP_MIN		(0.676f)
#define XNVM_EFUSE_VCC_PMC_LP_MAX		(0.724f)
#define XNVM_EFUSE_VCC_PMC_MP_MIN		(0.775f)
#define XNVM_EFUSE_VCC_PMC_MP_MAX		(0.825f)
#define XNVM_EFUSE_VCC_PMC_HP_MIN		(0.854f)
#define XNVM_EFUSE_VCC_PMC_HP_MAX		(0.906f)

#define XNVM_EFUSE_SYSMON_LOCK_CODE	(0xF9E8D7C6U)

/**
* @}
* @endcond
*/

/***************************** Type Definitions *******************************/

/**
* @}
*/

/**@cond xnvm_internal
 * @{
 */
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

typedef enum {
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_0 = 2,
	XNVM_EFUSE_MISC_PPK0_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK1_INVALID_BIT_1,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_0,
	XNVM_EFUSE_MISC_PPK2_INVALID_BIT_1,
	XNVM_EFUSE_MISC_SAFETY_MISSION_EN,
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

typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	u8 PrgmPufHelperData;
	u8 EnvMonitorDis;
	XSysMonPsv *SysMonInstPtr;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}XNvm_EfusePufHd;

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

}XNvm_EfuseData;

/**
* @}
* @endcond
*/

/**
 * @addtogroup xilnvm_versal_error_codes XilNvm Error Codes
 * @{
 */

/**
 The following table lists the Versal eFuse library error codes.
 */
typedef enum {
	XNVM_EFUSE_ERR_WRITE_AES_DIS = 0x8B00,/**<0x8B00 - Error in
						* AES_DIS efuse
						* programming */

	XNVM_EFUSE_ERR_WRITE_JTAG_ERROUT_DIS = 0x8C00,/**<0x8C00 - Error in
						* JTAG_ERROUT_DIS efuse
						* programming */

	XNVM_EFUSE_ERR_WRITE_JTAG_DIS = 0x8D00,/**<0x8D00 - Error in
						* JTAG_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_HWTSTBITS_DIS = 0x8E00,/**<0x8E00 - Error in
						* HWTSTBITS_DIS efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_IP_DIS_WR_LK = 0x8F00,/**<0x8F00 - Error in
						* IP_DIS_WR_LK efuse
						* programming */



	XNVM_EFUSE_ERR_WRITE_PPK0_WR_LK = 0x9000,/**<0x9000 - Error in
						* PPK0_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_PPK1_WR_LK = 0x9100,/**<0x9100 - Error in
						* PPK1_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_PPK2_WR_LK	= 0x9200,/**<0x9200 - Error in
						* PPK2_WR_LK efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_0 = 0x9300,/**<0x9300 - Error in
						* AES_CRC_LK_BIT0 efuse
						* programming */


	XNVM_EFUSE_ERR_WRITE_AES_CRC_LK_BIT_1 = 0x9400,/**<0x9400 - Error in
							 * AES_CRC_LK_BIT1 efuse
							 * programming */


	XNVM_EFUSE_ERR_WRITE_AES_WR_LK = 0x9500,/**<0x9500 - Error in
						 * AES_WR_LK efuse
						 * programming */

	XNVM_EFUSE_ERR_WRITE_USER_KEY0_CRC_LK = 0x9600,/**<0x9600 - Error in
							* USER_KEY0_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY0_WR_LK = 0x9700,/**<0x9700 - Error in
							* USER_KEY0_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY1_CRC_LK = 0x9800,/**<0x9800 - Error in
							* USER_KEY1_CRC_LK
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_USER_KEY1_WR_LK = 0x9900,/**<0x9900 - Error in
							* USER_KEY1_WR_LK efuse
							* programming */


	XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_0 = 0x9A00,/**<0x9600 - Error in
							* SECDBG_DIS_BIT_0
							* efuse programming */


	XNVM_EFUSE_ERR_WRITE_SECDBG_DIS_BIT_1 = 0x9B00,/**<0x9B00 - Error in
							* SECDBG_DIS_BIT_1
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_0 = 0x9C00,/**<0x9C00 - Error in
							* SECLOCKDBG_DIS
							* bit0
							* programming */
	XNVM_EFUSE_ERR_WRITE_SECLOCKDBG_DIS_BIT_1 = 0x9D00,/**<0x9D00 - Error in
							* SECLOCKDBG_DIS
							* bit1 efuse
							* programming */
	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_0 = 0x9E00,/**<0x9E00 - Error in
							* PMC_SC_EN_BIT_0
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_1 = 0x9F00,/**<0x9F00 - Error in
							* PMC_SC_EN_BIT_1
							* efuse programming */

	XNVM_EFUSE_ERR_WRITE_PMC_SC_EN_BIT_2 = 0xA000,/**<0xA000 - Error in
							* PMC_SC_EN_BIT_2
							* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SVD_WR_LK = 0xA100,/**<0xA100 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_DNA_WR_LK = 0xA200,/**<0xA200 - Error in
						* DNA_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_BOOTENV_WR_LK = 0xA300,/**<0xA300 - Error in
						* BOOTENV_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_CACHE_WR_LK = 0xA400,/**<0xA400 - Error in
						* SVD_WR_LK
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_0 = 0xA500,/**<0xA500 - Error in
						* REG_INIT_DIS_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_REG_INIT_DIS_BIT_1 = 0xA600,/**<0xA600 - Error in
						* REG_INIT_DIS_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_0 = 0xA700,/**<0xA700 - Error in
						* PPK0_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK0_INVALID_BIT_1 = 0xA800,/**<0xA800 - Error in
						* PPK0_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_0 = 0xA900,/**<0xA900 - Error in
						* PPK1_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK1_INVALID_BIT_1 = 0xAA00,/**<0xAA00 - Error in
						* PPK1_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_0 = 0xAB00,/**<0xAB00 - Error in
						* PPK2_INVALID_BIT_0
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_PPK2_INVALID_BIT_1 = 0xAC00,/**<0xAC00 - Error in
						* PPK2_INVALID_BIT_1
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_SAFETY_MISSION_EN = 0xAD00,/**<0xAD00 - Error in
						* SAFETY_MISSION_EN
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_LBIST_EN = 0xAE00,/**<0xAE00 - Error in
						* LBIST_EN
						* efuse programming */
	XNVM_EFUSE_ERR_WRITE_CRYPTO_KAT_EN = 0xAF00,/**<0xAF00 - Error in
							 * CRYPTO_KAT_EN
							 * efuse programming */
	XNVM_ERR_WRITE_PUF_USER_DATA = 0x10000, /**< 0x10000
						* When user chooses PUF efuses as user efuses
						* data provided for last row
						* i.e row 255 is not valid */
	XNVM_EFUSE_ERR_RD_SEC_CTRL_BITS = 0xD000,/**<0xD000 - Error in
						* reading Sec Ctrl efuses */
	XNVM_EFUSE_ERR_RD_MISC_CTRL_BITS = 0xD100,/**<0xD100 - Error in
						* reading in Misc Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SEC_CTRL = 0xD200,/**<0xD200 - Error in
						* reading in Puf Ctrl efuses */
	XNVM_EFUSE_ERR_RD_PUF_SYN_DATA = 0xD300,/**<0xD300 - Error in
						* reading Puf syn efuses */
	XNVM_EFUSE_ERR_RD_PUF_CHASH = 0xD400,/**<0xD400 - Error in
						* reading Puf Chash efuses */
	XNVM_EFUSE_ERR_RD_PUF_AUX = 0xD500,/**<0xD500 - Error in
						* reading Puf Aux efuses */
	XNVM_EFUSE_ERR_RD_DNA = 0xD600,/**<0xD600 - Error in
						* reading DNA efuses */
	XNVM_EFUSE_ERR_RD_PPK_HASH = 0xD700,/**<0xD700 - Error in
						* reading PPK hash efuses */
	XNVM_EFUSE_ERR_RD_META_HEADER_IV_RANGE = 0xD800,/**<0xD800 - Error in
						* reading Meta IV efuses */
	XNVM_EFUSE_ERR_RD_BLACK_IV = 0xD900,/**<0xD900 - Error in
						* reading Blk IV efuses */
	XNVM_EFUSE_ERR_RD_PLM_IV_RANGE = 0xDA00,/**<0xDA00 - Error in
						* reading PLM Iv efuses */
	XNVM_EFUSE_ERR_RD_DATA_PARTITION_IV_RANGE = 0xDB00,/**<0xDB00 - Error
						* in
						* reading Data Partition IV
						* efuses */
	XNVM_EFUSE_ERR_RD_DEC_ONLY = 0xDC00,/**<0xDC00 - Error in
						* reading in Dec_only efuses */
	XNVM_EFUSE_ERR_RD_USER_FUSES = 0xDD00,/**<0xDD00 - Error in
						* reading User efuses */
	XNVM_EFUSE_ERR_RD_PUF_FUSES = 0xDE00,/**<0xDE00 - Error in
						* reading PUF efuses */

	XNVM_EFUSE_ERR_WRITE_ROW_37_PROT = 0xE000,/**<0xE000 - Error in
						* ROW_37_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_40_PROT = 0xE100,/**<0xE100 - Error in
						* ROW_40_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_42_PROT = 0xE200,/**<0xE200 - Error in
						* ROW_42_PROT programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_0_PROT = 0xE300,/**<0xE300 - Error in
						* ROW_43_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_43_1_PROT = 0xE400,/**<0xE400 - Error in
						* ROW_43_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_0_PROT = 0xE500,/**<0xE500 - Error in
							* ROW_57_0_PROT
							* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_57_1_PROT = 0xE600,/**<0xE600 - Error in
						* ROW_57_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW_58_PROT = 0xE700,/**<0xE700 - Error in
						* ROW_58_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_0_PROT = 0xE800,/**<0xE800 - Error in
						* ROW64_87_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW64_87_1_PROT = 0xE900,/**<0xE900 - Error in
						* ROW64_87_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_0_PROT = 0xEA00,/**<0xEA00 - Error in
						* ROW96_99_0_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_ROW96_99_1_PROT = 0xEB00,/**<0xEB00 - Error in
						* ROW96_99_1_PROT
						* programming */
	XNVM_EFUSE_ERR_WRITE_GD_ROM_BITS = 0xEE00,/**<0xEE00 - Error in
						* programming of Glitch ROM monitor
						* or glitch halt boot */
	XNVM_EFUSE_ERR_WRITE_HALT_BOOT_BITS = 0xEF00,/**<0xEF00 - Error in
						* programming of ROM flow control halt boot
						* bits for generic and environment errors*/
	XNVM_EFUSE_ERROR_READ_TMEPERATURE_OUT_OF_RANGE = 0xF100,/**<0xF100 - Error
						* before programming eFuse,
						* Temparature is out of range */
	XNVM_EFUSE_ERROR_READ_VOLTAGE_OUT_OF_RANGE = 0xF200,/**<0xF200 - Error
						* before programming eFuse,
						* Voltage is out of range */
	XNVM_EFUSE_ERROR_NO_SUPPLIES_ENABLED = 0xF300,/**<0xF200 - Error
						* before programming eFuse,
						* no supplies are enabled */
	XNVM_EFUSE_ERROR_SYSMON_NO_NEW_DATA = 0xF400,/**<0xF400 - Error
						* before programming eFuse,
						* new data is not available */
} XNvm_EfuseVersalErrorCodes;

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

#ifdef __cplusplus
}
#endif

#endif	/* XNVM_EFUSE_H */

/* @} */
