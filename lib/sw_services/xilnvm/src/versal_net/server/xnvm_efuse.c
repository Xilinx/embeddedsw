/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_efuse.c
*
* This file contains the xilnvm server APIs implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 3.0   kal  07/12/2022 Initial release
* 3.1   kal  11/01/2022 Make Revocation id number 0 as valid to align with ROM
*                       behaviour
*       skg  11/08/2022 Added In Body comments for APIs
* 3.2   har  02/22/2023 Added API to program ROM Rsvd eFUSEs.
*       vss  03/14/2023 Fixed compilation warining
*       kum  04/11/2023 Added Env monitor before efuse programming
*       kpt  07/26/2023 Removed XNvm_EfuseReadCacheRange
*       kpt  07/26/2023 Fix security review comments
*       kpt  08/28/2023 Fix SW-BP-REDUNDANCY while assigning Status to CloseStatus
*       kpt  09/05/2023 Fix SW-BP-REDUNDANCY in XNvm_EfusePrgmIv and XNvm_EfuseWriteDmeRevoke
*       kpt  09/09/2023 Avoid returning XST_SUCCESS incase of glitch
*       yog  09/13/2023 Used XNvm_IsDmeModeEn() API for reading DME Mode
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_util.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_efuse_hw.h"
#include "xnvm_utils.h"
#include "xnvm_validate.h"
#include "xnvm_efuse_common.h"
#include "xsysmonpsv.h"
#include "xplmi_sysmon.h"
/**************************** Type Definitions *******************************/
typedef struct {
	u32 StartRow; /**< Start row number of eFuse */
	u32 ColStart; /**< Start column number of eFuse */
	u32 ColEnd; /**< End column number of eFuse */
	u32 NumOfRows; /**< Number of rows of eFuse  */
	u32 SkipVerify; /**< Flag to check if eFuse bit should be verified after programming */
	XNvm_EfuseType EfuseType; /**< Efuse page number */
}XNvm_EfusePrgmInfo;

/************************** Function Prototypes ******************************/
static int XNvm_EfusePrgmIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv);
static int XNvm_EfusePrgmPpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash);
static int XNvm_EfusePrgmAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey);
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
		u32 StartOffset, u32 EndOffset);
static int XNvm_EfusePgmAndVerifyData(XNvm_EfusePrgmInfo *EfusePrgmInfo,
		const u32* RowData);
static int XNvm_EfusePgmBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col);
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col,
		u32 SkipVerify);
static int XNvm_EfuseCloseController(void);
static int XNvm_EfusePrgmFipsInfo(u32 FipsMode, u32 FipsVersion);
static int XNvm_EfusePrgmDmeUserKey(XNvm_DmeKeyType KeyType, const XNvm_DmeKey *EfuseKey);
static int XNvm_EfuseIsPufHelperDataEmpty(void);
static int XNvm_EfuseWritePufSynData(const u32 *SynData);
static int XNvm_EfuseWritePufChash(u32 Chash);
static int XNvm_EfuseWritePufAux(u32 Aux);
static int XNvm_EfuseWriteRoSwapEn(u32 RoSwap);
static int XNvm_UdsCrcCalc(const u32 *Uds);
static int XNvm_EfuseCacheReloadAndProtectionChecks(void);
static int XNvm_EfusePrgmProtectionBits(void);
static int XNvm_EfuseProtectionChecks(void);
static int XNvm_EfuseChangeEndianness(u8 *Dest, u8 *Src, u32 Size);
static int XNvm_EfuseReadRow(XNvm_EfuseType Page, u32 Row, u32 *RegData);

/************************** Constant Definitions *****************************/
#define XNVM_EFUSE_ERROR_BYTE_SHIFT	(8U) /**< Byte shift used in error code */
#define XNVM_EFUSE_ERROR_NIBBLE_SHIFT	(4U) /**< Nibble shift used in error code */
#define XNVM_EFUSE_MAX_FIPS_VERSION	(7U) /**< Max Value of FIPS version */
#define XNVM_EFUSE_MAX_FIPS_MODE	(0xFFU) /**< Max value of FIPS mode */
#define XNVM_EFUSE_BITS_IN_A_BYTE	(8U) /**< Number of bits in a byte */
#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU)
						 /**< Secure Default Value for a register */
#define REVERSE_POLYNOMIAL			(0x82F63B78U)
						/**< Value used in CRC calculation to reverse a polynomial */
#define XNVM_EFUSE_SKIP_VERIFY			(1U)
						/**< Skip verification of eFuses after programming */
#define XNVM_EFUSE_PROGRAM_VERIFY		(0U) /**< Verify eFuses after programming */
#define XNVM_EFUSE_CRC_SALT			(0x000000FFU) /**< CRC salt value */
#define XNVM_EFUSE_REVOKE_ID_127	(127U) /**< Efuse revoke ID */
#define XNVM_EFUSE_PUF_SEC_CTRL_INVLD_MASK       0xE0000000U
		/**< Mask for PUF control bits in PUF_ECC_PUF_CTRL register in EFUSE_CACHE module*/
#define XNVM_EFUSE_PUF_CTRL_PUF_REGEN_DIS_MASK   0x80000000U
		/**< Mask for PUF_REGEN_DISABLE */
#define XNVM_EFUSE_PUF_CTRL_PUF_HD_INVLD_MASK    0x40000000U
		/**< Mask for PUF_HD_INVLD */
#define XNVM_EFUSE_PUF_CTRL_PUF_REGIS_DIS_MASK   0x20000000U
		/**< Mask for PUF_REGIS_DIS */
#define XNVM_EFUSE_DME_KEY_SIZE_IN_BYTES	 (48U)
			/**< DME key size in bytes */
/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	This function is used to take care of prerequisites to
 * 		program below eFuses
 * 		AES key/User key 0/User key 1.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user.
 * 				when set to true it will not check for voltage
 * 				and temparature limits.
 * @param	KeyType - Type of key to be programmed
 * 			(AesKey/UserKey0/UserKey1)
 * @param	EfuseKey - Pointer to the XNvm_AesKey structure, which holds
 * 			Aes key to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_BEFORE_PROGRAMMING - Error before programming
 *							eFuse.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY	 - Error while writing
 *							AES key.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0 - Error while writing
 *							User key 0.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_KEY1 - Error while writing
 *							User key 1.
 *
 ******************************************************************************/
int XNvm_EfuseWriteAesKey(u32 EnvDisFlag, XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();
    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((KeyType != XNVM_EFUSE_AES_KEY) &&
		(KeyType != XNVM_EFUSE_USER_KEY_0) &&
		(KeyType != XNVM_EFUSE_USER_KEY_1)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (EfuseKey == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse controller. On error return appropriate failure code
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 *  Validate the Aes write request before writing AES into eFuse. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING if not success
	 */
	Status = XNvm_EfuseValidateAesKeyWriteReq(KeyType);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 *  Program Aes key into eFuse
	 */
	Status = XNvm_EfusePrgmAesKey(KeyType, EfuseKey);

END:
    /**
	 *  Lock eFuse Controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to to take care of prerequisitis to
 * 		program below eFuses
 * 		PPK0/PPK1/PPK2 hash.
 *
 *@param	EnvDisFlag - Environmental monitoring flag set by the user.
 * 				when set to true it will not check for voltage
 * 				and temparature limits.
 * @param	PpkType - Type of ppk hash to be programmed(PPK0/PPK1/PPK2)
 * @param	EfuseHash - Pointer to the XNvm_PpkHash structure which holds
 * 			PPK hash to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash.
 *
 ******************************************************************************/
int XNvm_EfuseWritePpkHash(u32 EnvDisFlag, XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();
    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */

	if ((PpkType != XNVM_EFUSE_PPK0) &&
		(PpkType != XNVM_EFUSE_PPK1) &&
		(PpkType != XNVM_EFUSE_PPK2)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (EfuseHash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse controller. On error return appropriate failure code
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

    /**
	 *  Validate the PPK hash write request before writing PPK into eFuse. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING if not success
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseValidatePpkHashWriteReq(PpkType);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 *  Program PPK hash into eFuse
	 */
	Status = XNvm_EfusePrgmPpkHash(PpkType, EfuseHash);

END:
	/**
	 *  Lock eFuse Controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to to take care of prerequisitis to
 * 		program below IV eFuses
 *		XNVM_EFUSE_ERR_WRITE_META_HEADER_IV /
 *		XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV /
 *		XNVM_EFUSE_ERR_WRITE_PLM_IV /
 *		XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user.
 * 				when set to true it will not check for voltage
 * 				and temparature limits.
 * @param	IvType - Type of IV eFuses to be programmmed
 * @param	EfuseIv - Pointer to the XNvm_EfuseIvs structure which holds IV
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM           - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV    - Error while writing
 *							   Meta Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV 	 - Error while writing
 *							   BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV 		 - Error while writing
 *							   PLM IV.
 * 		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing
 * 							Data Partition IV.
 *
 ******************************************************************************/
int XNvm_EfuseWriteIv(u32 EnvDisFlag, XNvm_IvType IvType, XNvm_Iv *EfuseIv)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();
    /**
	 *  validate Input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((IvType != XNVM_EFUSE_META_HEADER_IV_RANGE) &&
		(IvType != XNVM_EFUSE_BLACK_IV) &&
		(IvType != XNVM_EFUSE_PLM_IV_RANGE) &&
		(IvType != XNVM_EFUSE_DATA_PARTITION_IV_RANGE)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EfuseIv == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse controller. On error return appropriate failure code
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

     /**
	 *  Validate the IVs write request before writing IVs into eFuse. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING if not success
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseValidateIvWriteReq(IvType, EfuseIv);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  Program IVs into eFuse
	 */
	Status = XNvm_EfusePrgmIv(IvType, EfuseIv);

END:
    /**
	 *  Lock eFuse Controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program Glitch Configuration given by
 * 		the user.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user.
 * 				when set to true it will not check for voltage
 * 				and temparature limits.
 * @param	GlitchConfig - Pointer to the Glitch configuration
 *
 * @return	- XST_SUCCESS - On Successful Write.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_CFG - Error in programming of
 *					glitch configuration.
 *		- XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK - Error in programming
 *					glitch write lock.
 ******************************************************************************/
int XNvm_EfuseWriteGlitchConfigBits(u32 EnvDisFlag, u32 GlitchConfig)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 GlitchDetVal = 0U;
	u32 GlitchDetWrLk = 0U;
	u32 PrgmGlitchConfig = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse controller. On error return appropriate failure code
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;

	/**
	 *  Compute Programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&GlitchConfig,
					&PrgmGlitchConfig,
					XNVM_EFUSE_CACHE_ANLG_TRIM_3_OFFSET,
					XNVM_EFUSE_CACHE_ANLG_TRIM_3_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	GlitchDetVal = PrgmGlitchConfig & XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK;
	GlitchDetWrLk = PrgmGlitchConfig & (~XNVM_EFUSE_GLITCH_CONFIG_DATA_MASK);

	EfusePrgmInfo.StartRow = XNVM_EFUSE_ANLG_TRIM_3_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_GLITCH_DET_CONFIG_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_GLITCH_DET_CONFIG_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_GLITCH_DET_CONFIG_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *  Program and verify the glitch config bits
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &GlitchDetVal);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_GLITCH_CFG);
		goto END;
	}

	if (GlitchDetWrLk != 0U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_GLITCH_DET_WR_LK_ROW,
				XNVM_EFUSE_GLITCH_DET_WR_LK_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_GLITCH_WRLK);
		}
	}

END:
    /**
	 *  Lock eFuse Controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below DEC_ONLY fuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return	- XST_SUCCESS - Specified data read.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM        - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY - Error in DEC_ONLY
 *							programming.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY         - Error in Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDecOnly(u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 Data = XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  validate Decrypt only request before programming. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING if not success
	 */
	Status = XNvm_EfuseValidateDecOnlyRequest();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_SECURITY_MISC_0_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DEC_ONLY_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DEC_ONLY_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DEC_ONLY_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify decrypt only data. Return XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &Data);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_DEC_EFUSE_ONLY);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function writes Revocation eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	RevokeIdNum - RevokeId number to program Revocation Id eFuses.
 *
 * @return	- XST_SUCCESS - On successful write to eFuse.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_LOCK - Error while Locking the controller.
 *
 * @note
 * Example: 	If the revoke id to program is 64, it will program the 0th bit
 * 		of the REVOCATION_ID_2 eFuse row.
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationID(u32 EnvDisFlag, u32 RevokeIdNum)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	u32 RevokeIdRow;
	u32 RevokeIdCol;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	/**
	 *  Validate Input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (RevokeIdNum > (XNVM_MAX_REVOKE_ID_FUSES - 1U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

	/**
	 *   Unlock eFuse controller. Return appropriate error code upon failure
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (RevokeIdNum <= XNVM_EFUSE_REVOKE_ID_127) {
		RevokeIdRow = XNVM_EFUSE_REVOKE_ID_0_TO_127_START_ROW;
		RevokeIdCol = XNVM_EFUSE_REVOKE_ID_0_TO_127_START_COL_NUM;
	} else {
		RevokeIdRow = XNVM_EFUSE_REVOKE_ID_128_TO_255_START_ROW;
		RevokeIdCol = XNVM_EFUSE_REVOKE_ID_128_TO_255_START_COL_NUM;
	}
	RevokeIdRow = RevokeIdRow + (RevokeIdNum / XNVM_EFUSE_BITS_IN_A_BYTE);
	RevokeIdCol = RevokeIdCol + (RevokeIdNum % XNVM_EFUSE_BITS_IN_A_BYTE);

	Status = XST_FAILURE;

	/**
	 *  Program and Revocation Ids data. Return XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			RevokeIdRow, RevokeIdCol, XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_REVOCATION_IDS);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs OffChip Revoke eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	OffchipIdNum - OffchipIdNum number to program OffchipId eFuses.
 *
 * @return	- XST_SUCCESS - On successful write of OffChipId.
 *		- XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS - Error in writing
 *							OffChipId eFuses.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
int XNvm_EfuseWriteOffChipRevokeID(u32 EnvDisFlag, u32 OffchipIdNum)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	u32 OffchipIdRow;
	u32 OffchipIdCol;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((OffchipIdNum == 0U) ||
		(OffchipIdNum > XNVM_MAX_REVOKE_ID_FUSES)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	OffchipIdRow = XNVM_EFUSE_OFFCHIP_REVOKE_ID_START_ROW +
			((OffchipIdNum - 1U)/ XNVM_EFUSE_MAX_BITS_IN_ROW);
	OffchipIdCol = ((OffchipIdNum - 1U) % XNVM_EFUSE_MAX_BITS_IN_ROW);

	Status = XST_FAILURE;
	/**
	 *   Program and verify Revocation Ids data. Return XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			OffchipIdRow, OffchipIdCol, XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_OFFCHIP_REVOKE_IDS);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs MiscCtrl eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	MiscCtrlBits - MiscCtrl data to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS - Error while writing
 *						MiscCtrlBits
 ******************************************************************************/
int XNvm_EfuseWriteMiscCtrlBits(u32 EnvDisFlag, u32 MiscCtrlBits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 RdMiscCtrlBits = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  compute programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&MiscCtrlBits,
				&RdMiscCtrlBits,
				XNVM_EFUSE_CACHE_MISC_CTRL_CACHE_OFFSET,
				XNVM_EFUSE_CACHE_MISC_CTRL_CACHE_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_MISC_CTRL_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_MISC_CTRL_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_MISC_CTRL_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_MISC_CTRL_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify miscellaneous control bits. Return XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &RdMiscCtrlBits);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_MISC_CTRL_BITS);
	}

END:
    /**
	 *  Lock eFuse Controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs SecCtrl eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	SecCtrlBits - SecCtrl data to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS - Error while writing
 *					SecCtrlBits
 ******************************************************************************/
int XNvm_EfuseWriteSecCtrlBits(u32 EnvDisFlag, u32 SecCtrlBits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 RdSecCtrlBits = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  compute programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&SecCtrlBits, &RdSecCtrlBits,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_SEC_CTRL_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_SEC_CTRL_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_SEC_CTRL_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_SEC_CTRL_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify security controls bits. Return XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &RdSecCtrlBits);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_SEC_CTRL_BITS);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs Misc1Ctrl eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	Misc1Bits - Misc1Bits data to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		-XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS - Error while writing
 *						Misc1CtrlBits
 ******************************************************************************/
int XNvm_EfuseWriteMisc1Bits(u32 EnvDisFlag, u32 Misc1Bits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 RdMisc1Bits = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  compute programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&Misc1Bits, &RdMisc1Bits,
				XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET,
				XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_SECURITY_MISC1_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_SECURITY_MISC1_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_SECURITY_MISC1_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_SECURITY_MISC1_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify miscellaneous 1 controls bits. Return XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &RdMisc1Bits);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_MISC1_CTRL_BITS);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs BootEnvCtrl eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	BootEnvCtrlBits - BootEnvCtrl data to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL - Error while writing
 *						BootEnvCtrl Bits
 ******************************************************************************/
int XNvm_EfuseWriteBootEnvCtrlBits(u32 EnvDisFlag, u32 BootEnvCtrlBits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 RdBootEnvCtrlBits = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  compute programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&BootEnvCtrlBits,
				&RdBootEnvCtrlBits,
				XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET,
				XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_BOOT_ENV_CTRL_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_BOOT_ENV_CTRL_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_BOOT_ENV_CTRL_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_BOOT_ENV_CTRL_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify miscellaneous 1 controls bits. Return XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &RdBootEnvCtrlBits);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_BOOT_ENV_CTRL);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to to take care of prerequisitis to
 * 		program FIPS mode and FIPS version eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	FipsMode - Fips mode to be written to eFuses.
 * @param	FipsVersion - Fips version to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- ErrorCode - On Failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteFipsInfo(u32 EnvDisFlag, u32 FipsMode, u32 FipsVersion)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (FipsVersion > XNVM_EFUSE_MAX_FIPS_VERSION) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (FipsMode > XNVM_EFUSE_MAX_FIPS_MODE) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  Validate Fips information. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING upon failure
	 */
	Status = XNvm_EfuseValidateFipsInfo(FipsMode, FipsVersion);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  Program Fips information inti eFuse
	 */
	Status = XNvm_EfusePrgmFipsInfo(FipsMode, FipsVersion);

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}



/******************************************************************************/
/**
 * @brief	This function programs UDS eFuses.
 *
 * @param	EfuseUds - Pointer to the XNvm_Uds structure.
 *
 * @return	- XST_SUCCESS - On successful write.
 * 		- XNVM_EFUSE_ERR_WRITE_UDS - On failure in UDS write.
 *
 ******************************************************************************/
int XNvm_EfuseWriteUds(u32 EnvDisFlag, XNvm_Uds *EfuseUds)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 SecCtrlBits = 0U;
	u32 Crc;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (EfuseUds == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

    /**
	 *  Read directly from cache offset of the SecCtrl to fill the SecCtrlBits structure
	 */
	SecCtrlBits = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);

	if ((SecCtrlBits & XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_MASK) != 0U) {
		Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_UDS |
				XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	/**
	 *   Program and verify Uds data
	 */
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	EfusePrgmInfo.SkipVerify = XNVM_EFUSE_SKIP_VERIFY;

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DICE_UDS_0_TO_63_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DICE_UDS_0_TO_63_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DICE_UDS_0_TO_63_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DICE_UDS_0_TO_63_NUM_OF_ROWS;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, EfuseUds->Uds);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DICE_UDS_64_TO_191_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DICE_UDS_64_TO_191_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DICE_UDS_64_TO_191_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DICE_UDS_64_TO_191_NUM_OF_ROWS;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseUds->Uds[2U]);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DICE_UDS_192_TO_255_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DICE_UDS_192_TO_255_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DICE_UDS_192_TO_255_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DICE_UDS_192_TO_255_NUM_OF_ROWS;

	Status = XST_FAILURE;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseUds->Uds[6U]);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DICE_UDS_256_TO_383_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DICE_UDS_256_TO_383_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DICE_UDS_256_TO_383_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DICE_UDS_256_TO_383_NUM_OF_ROWS;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseUds->Uds[8U]);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
		goto END;
	}

    /**
	 *  Reload cache and check protection bits
	 */
	Status = XNvm_EfuseCacheReloadAndProtectionChecks();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
		goto END;
	}

    /**
	 *  Calculate CRC for Uds data
	 */
	Crc = XNvm_UdsCrcCalc(EfuseUds->Uds);

    /**
	 *  check aes key Crc. Return XNVM_EFUSE_ERR_WRITE_UDS upon failure
	 */
	Status = XNvm_EfuseCheckAesKeyCrc(XNVM_EFUSE_CTRL_UDS_DICE_CRC_OFFSET,
			XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_DONE_MASK,
                        XNVM_EFUSE_CTRL_STATUS_UDS_DICE_CRC_PASS_MASK, Crc);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_UDS);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs DME userkey eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	KeyType - DME Key type DME_USER_KEY_0/1/2/3
 * @param	EfuseKey - Pointer to DME userkey structure.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- ErrorCode - On failure.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDmeUserKey(u32 EnvDisFlag, XNvm_DmeKeyType KeyType, XNvm_DmeKey *EfuseKey)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((KeyType != XNVM_EFUSE_DME_USER_KEY_0) &&
		(KeyType != XNVM_EFUSE_DME_USER_KEY_1) &&
		(KeyType != XNVM_EFUSE_DME_USER_KEY_2 &&
		(KeyType != XNVM_EFUSE_DME_USER_KEY_3))) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if (EfuseKey == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

	/**
	 *  Read directly from cache offset of the dme fips to fill the DmeModeCacheVal structure
	 */
	Status = XNvm_IsDmeModeEn();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *   Program dme user key into eFuse
	 */
	Status = XNvm_EfusePrgmDmeUserKey(KeyType, EfuseKey);

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs DME Revoke eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	RevokeNum - DmeRevoke eFuse number.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 - Error while writing
 *					DmeRevoke 0.
 *		- XNVM_EFUSE_ERR_WRITE_DME_REVOKE_1 - Error while writing
 *					DmeRevoke 1.
 *		- XNVM_EFUSE_ERR_WRITE_DME_REVOKE_2 - Error while writing
 *					DmeRevoke 2.
 *		- XNVM_EFUSE_ERR_WRITE_DME_REVOKE_3 - Error while writing
 *					DmeRevoke 3.
 ******************************************************************************/
int XNvm_EfuseWriteDmeRevoke(u32 EnvDisFlag, XNvm_DmeRevoke RevokeNum)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	u32 Row = 0U;
	u32 Col_0_Num = 0U;
	u32 Col_1_Num = 0U;
	volatile XNvm_DmeRevoke RevokeNumTmp;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((RevokeNum != XNVM_EFUSE_DME_REVOKE_0) &&
		(RevokeNum != XNVM_EFUSE_DME_REVOKE_1) &&
		(RevokeNum != XNVM_EFUSE_DME_REVOKE_2) &&
		(RevokeNum != XNVM_EFUSE_DME_REVOKE_3)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	if (RevokeNum == XNVM_EFUSE_DME_REVOKE_0) {
		RevokeNumTmp = XNVM_EFUSE_DME_REVOKE_0;
		Row = XNVM_EFUSE_DME_REVOKE_0_AND_1_ROW;
		Col_0_Num = XNVM_EFUSE_DME_REVOKE_0_0_COL_NUM;
		Col_1_Num = XNVM_EFUSE_DME_REVOKE_0_1_COL_NUM;
	}
	else if (RevokeNum == XNVM_EFUSE_DME_REVOKE_1) {
		RevokeNumTmp = XNVM_EFUSE_DME_REVOKE_1;
		Row = XNVM_EFUSE_DME_REVOKE_0_AND_1_ROW;
		Col_0_Num = XNVM_EFUSE_DME_REVOKE_1_0_COL_NUM;
		Col_1_Num = XNVM_EFUSE_DME_REVOKE_1_1_COL_NUM;
	}
	else if (RevokeNum == XNVM_EFUSE_DME_REVOKE_2) {
		RevokeNumTmp = XNVM_EFUSE_DME_REVOKE_2;
		Row = XNVM_EFUSE_DME_REVOKE_2_AND_3_ROW;
		Col_0_Num = XNVM_EFUSE_DME_REVOKE_2_0_COL_NUM;
		Col_1_Num = XNVM_EFUSE_DME_REVOKE_2_1_COL_NUM;
	}
	else if (RevokeNum == XNVM_EFUSE_DME_REVOKE_3) {
		RevokeNumTmp = XNVM_EFUSE_DME_REVOKE_3;
		Row = XNVM_EFUSE_DME_REVOKE_2_AND_3_ROW;
		Col_0_Num = XNVM_EFUSE_DME_REVOKE_3_0_COL_NUM;
		Col_1_Num = XNVM_EFUSE_DME_REVOKE_3_1_COL_NUM;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (RevokeNumTmp != RevokeNum) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *   Program and verify dme revoke data
	 */
	Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					Row, Col_0_Num, XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 +
			(RevokeNum << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
			Row, Col_1_Num, XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_DME_REVOKE_0 +
			(RevokeNum << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs PLM_UPDATE eFuse.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_UPDATE - Error while writing
 *					PlmUpdate eFuse.
 *
 ******************************************************************************/
int XNvm_EfuseWriteDisableInplacePlmUpdate(u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *   Program and verify Plm update data
	 */
	Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
					XNVM_EFUSE_DISABLE_PLM_UPDATE_ROW,
					XNVM_EFUSE_DISABLE_PLM_UPDATE_COL_NUM,
					XNVM_EFUSE_PROGRAM_VERIFY);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_PLM_UPDATE);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs BootModeDisable eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	BootModeMask - BootModeMask to be programmed to BootModeDisable
 * 				eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_BOOT_MODE_DISABLE - Error while writing
 *					BootModeDisable eFuses.
 *
 ******************************************************************************/
int XNvm_EfuseWriteBootModeDisable(u32 EnvDisFlag, u32 BootModeMask)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_BOOT_MODE_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_BOOT_MODE_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_BOOT_MODE_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_BOOT_MODE_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify boot mode disable data
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &BootModeMask);

	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_BOOT_MODE_DISABLE);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs DmeMode eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	DmeMode - DmeMode eFuses to be written.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_DME_MODE - Error while writing DmeMode
 *					eFuse.
 ******************************************************************************/
int XNvm_EfuseWriteDmeMode(u32 EnvDisFlag, u32 DmeMode)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_MODE_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_DME_MODE_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_DME_MODE_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_MODE_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify dme mode data
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &DmeMode);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_DME_MODE);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs EFUSE_CRC_SALT eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	Crc - Crc value to be written.
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_CRC_SALT - Error while writing Crc
 *						salt eFuses.
 ******************************************************************************/
int XNvm_EfuseWriteCrc(u32 EnvDisFlag, u32 Crc)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 CrcSalt = XNVM_EFUSE_CRC_SALT;
	u32 ReadReg = 0U;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Read directly from cache offset of the CRC. Return XNVM_EFUSE_ERR_WRITE_CRC | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING if crc non zero
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, XNVM_EFUSE_CACHE_CRC_OFFSET);
	if (ReadReg != 0x0U) {
		Status = (XNVM_EFUSE_ERR_WRITE_CRC | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM, XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_CRC_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_CRC_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_CRC_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_CRC_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify Uds Crc data. Return XNVM_EFUSE_ERR_WRITE_CRC upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &Crc);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_CRC);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_CRC_SALT_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_CRC_SALT_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_CRC_SALT_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_CRC_SALT_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify Uds Crc salt data. Return XNVM_EFUSE_ERR_WRITE_CRC_SALT upon failure
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &CrcSalt);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_CRC_SALT);
	}

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs PUF ctrl and PUF helper data, Chash and
 * 		Aux.
 *
 * @param	PufHelperData - Pointer to XNvm_EfusePufHdAddr structure
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA - Error while writing
 *			  					Puf helper data.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA    - Error while writing
 *			 					Puf Syndata.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_CHASH       - Error while writing
 *								Puf Chash.
 * 		- XNVM_EFUSE_ERR_WRITE_PUF_AUX         - Error while writing
 *								Puf Aux.
 *
 * @note	To generate PufSyndromeData please use XPuf_Registration API.
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(const XNvm_EfusePufHdAddr *PufHelperData)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	u32 PufSecurityCtrlReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

    /**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if (PufHelperData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

    /**
	 *  Read directly from cache offset of the PufCtrl data
	 */
	PufSecurityCtrlReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_OFFSET);

	if ((PufSecurityCtrlReg &
		(XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK |
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK)) != 0x0U) {

		Status = (XNVM_EFUSE_ERR_FUSE_PROTECTED |
				XNVM_EFUSE_ERR_WRITE_PUF_HELPER_DATA);
		goto END;
	}

	if (PufHelperData->EnvMonitorDis != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}
	if (PufHelperData->PrgmPufHelperData == TRUE) {
		Status = XST_FAILURE;
		/**
		 *  Check Puf helper data is empty. Return XNVM_EFUSE_ERR_BEFORE_PROGRAMMING is empty
		 */
		Status = XNvm_EfuseIsPufHelperDataEmpty();
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}

		Status = XST_FAILURE;
		/**
		 *  Write puf sync data into eFuse. Return XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA upon failure
		 */
		Status = XNvm_EfuseWritePufSynData(PufHelperData->EfuseSynData);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_SYN_DATA);
			goto END;
		}

		Status = XST_FAILURE;
		/**
		 *  Write puf chash data into eFuse. Return XNVM_EFUSE_ERR_WRITE_PUF_CHASH upon failure
		 */
		Status = XNvm_EfuseWritePufChash(PufHelperData->Chash);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_CHASH);
			goto END;
		}

		Status = XST_FAILURE;
		/**
		 *  Write puf aux into eFuse. Return XNVM_EFUSE_ERR_WRITE_PUF_AUX upon failure
		 */
		Status = XNvm_EfuseWritePufAux(PufHelperData->Aux);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_AUX);
			goto END;
		}

		Status = XST_FAILURE;
		/**
		 *  Write puf RoSwap data into eFuse. Return XNVM_EFUSE_ERR_WRITE_RO_SWAP upon failure
		 */
		Status = XNvm_EfuseWriteRoSwapEn(PufHelperData->RoSwap);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_RO_SWAP);
		}
	}


END :
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function calculates CRC of UDS.
 *
 * @param	Uds - Pointer to buffer which contains UDS
 *
 * @return	CRC of UDS.
 *
 ******************************************************************************/
static int XNvm_UdsCrcCalc(const u32 *Uds)
{
	u32 Crc = 0U;
	u32 Value;
	u8 Idx;
	u8 BitNo;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = 0U; Idx < XNVM_UDS_SIZE_IN_WORDS; Idx++) {
		/* Process each bits of 32-bit Value */
		Value = Uds[XNVM_UDS_SIZE_IN_WORDS - Idx - 1U];
		for (BitNo = 0U; BitNo < 32U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			}
			else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}

		/* Get 5-bit from Address */
		Value = XNVM_UDS_SIZE_IN_WORDS - (u32)Idx;
		for (BitNo = 0U; BitNo < 5U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			}
			else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}
	}

	return Crc;
}

/******************************************************************************/
/**
 * @brief	This function is used to program FIPS mode and FIPS version
 * 		eFuses
 *
 * @param	FipsMode - Fips mode to be written to eFuses.
 * @param	FipsVersion - Fips version to be written to eFuses.
 *
 * @return	- XST_SUCCESS - On successful write.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmFipsInfo(u32 FipsMode, u32 FipsVersion)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 PrgmFipsVer = FipsVersion;

	EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_FIPS_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_FIPS_MODE_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_FIPS_MODE_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_FIPS_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &FipsMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PrgmFipsVer & 0x01U) == 0x01U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_IP_DISABLE_ROW,
				XNVM_EFUSE_FIPS_VERSION_COL_0_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	PrgmFipsVer = PrgmFipsVer >> 1U;
	if ((PrgmFipsVer & 0x01U) == 0x01U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_IP_DISABLE_ROW,
				XNVM_EFUSE_FIPS_VERSION_COL_1_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	PrgmFipsVer = PrgmFipsVer >> 1U;
	if ((PrgmFipsVer & 0x01U) == 0x01U) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_IP_DISABLE_ROW,
				XNVM_EFUSE_FIPS_VERSION_COL_2_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs Puf control bits specified by user.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	PufCtrlBits - PufCtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - On success.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM       -  Invalid parameter.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_REGEN_DIS - 	Error while writing
 *						   	PUF_REGEN_DIS.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_HD_INVLD  - 	Error while writing
 *							PUF_HD_INVLD.
 *		- XNVM_EFUSE_ERR_WRITE_PUF_SYN_LK - 	Error while writing
 * 							PUF_SYN_LK.
 *
 ******************************************************************************/
 int XNvm_EfuseWritePufSecCtrl(u32 EnvDisFlag, u32 PufCtrlBits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	/**
	 *  Validate input parameters. Return XNVM_EFUSE_ERR_INVALID_PARAM if input parameters are invalid
	 */
	if ((PufCtrlBits == 0U) || ((PufCtrlBits & (~XNVM_EFUSE_PUF_SEC_CTRL_INVLD_MASK)) != 0U)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}


	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

	if ((PufCtrlBits & XNVM_EFUSE_PUF_CTRL_PUF_REGIS_DIS_MASK) == XNVM_EFUSE_PUF_CTRL_PUF_REGIS_DIS_MASK) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_PUF_AUX_ROW,
				XNVM_EFUSE_PUF_REGIS_DIS_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS);
			goto END;
		}
	}

	if ((PufCtrlBits & XNVM_EFUSE_PUF_CTRL_PUF_HD_INVLD_MASK) == XNVM_EFUSE_PUF_CTRL_PUF_HD_INVLD_MASK) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_PUF_AUX_ROW,
				XNVM_EFUSE_PUF_HD_INVLD_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS);
			goto END;
		}
	}

	if ((PufCtrlBits & XNVM_EFUSE_PUF_CTRL_PUF_REGEN_DIS_MASK) == XNVM_EFUSE_PUF_CTRL_PUF_REGEN_DIS_MASK) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_PUF_AUX_ROW,
				XNVM_EFUSE_PUF_REGEN_DIS_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_PUF_SEC_CTRL_BITS);
		}
	}

END:
	/**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF syndrome data.
 *
 * @param	SynData - Pointer to the Puf Syndrome data to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufSynData(const u32 *SynData)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};

	if (SynData == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_PAGE_0_PUF_SYN_DATA_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_PUF_SYN_DATA_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_PUF_SYN_DATA_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PAGE_0_PUF_SYN_DATA_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, SynData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_PAGE_1_PUF_SYN_DATA_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_PUF_SYN_DATA_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_PUF_SYN_DATA_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PAGE_1_PUF_SYN_DATA_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_1;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &SynData[64U]);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF Chash.
 *
 * @param	Chash - A 32-bit Chash to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufChash(u32 Chash)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};

	EfusePrgmInfo.StartRow = XNVM_EFUSE_PUF_CHASH_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_PUF_CHASH_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_PUF_CHASH_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &Chash);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with the PUF Aux.
 *
 * @param	Aux - A 32-bit Aux to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufAux(u32 Aux)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 AuxData;

	AuxData = (Aux & XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK);

	EfusePrgmInfo.StartRow = XNVM_EFUSE_PUF_AUX_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_PUF_AUX_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_PUF_AUX_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PUF_AUX_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &AuxData);

	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs the eFUSEs with th RO_SWAP_EN.
 *
 * @param	RoSwap - A 32-bit RoSwap to program the eFUSE.
 *
 * @return	- XST_SUCCESS - if programs successfully.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfuseWriteRoSwapEn(u32 RoSwap)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};

	EfusePrgmInfo.StartRow = XNVM_EFUSE_PUF_RO_SWAP_EN_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_PUF_RO_SWAP_EN_START_COL_NUM;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_PUF_RO_SWAP_EN_END_COL_NUM;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PUF_RO_SWAP_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &RoSwap);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function checks whether PUF is already programmed or not.
 *
 * @return	- XST_SUCCESS - if all rows are zero.
 * 		- XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD	 - Puf Syn data already
 * 							programmed.
 * 		- XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD - Puf chash already
 * 							programmed.
 * 		- XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD	 - Puf Aux is already
 * 							programmed.
 *
 *******************************************************************************/
static int XNvm_EfuseIsPufHelperDataEmpty(void)
{
	int Status = XST_FAILURE;
	u32 RowDataVal;

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET,
					XNVM_EFUSE_PUF_CHASH_NUM_OF_ROWS);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_CHASH_ALREADY_PRGMD;
		goto END;
	}

	RowDataVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_OFFSET);
	if ((RowDataVal &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK) != 0x00U) {
		Status = (int)XNVM_EFUSE_ERR_PUF_AUX_ALREADY_PRGMD;
		goto END;
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_CACHE_PUF_SYN_DATA_OFFSET,
			XNVM_EFUSE_PUF_SYN_DATA_NUM_OF_ROWS);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_PUF_SYN_ALREADY_PRGMD;
		goto END;
	}
END :
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function programs DME userkey eFuses.
 *
 * @param	KeyType - DME UserKey0/1/2/3.
 * @param	EfuseKey - Pointer to the XNvm_DmeKey structure.
 *
 * @return	- XST_SUCCESS - On successful write.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmDmeUserKey(XNvm_DmeKeyType KeyType, const XNvm_DmeKey *EfuseKey)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 Key[XNVM_DME_USER_KEY_SIZE_IN_WORDS] = {0U};
	u32 RegData = 0x00U;
	u32 Row;

	if (KeyType == XNVM_EFUSE_DME_USER_KEY_0) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_USER_KEY_0_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_DME_USER_KEY_0_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_DME_USER_KEY_0_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_USER_KEY_NUM_OF_ROWS;
		EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	}
	else if (KeyType == XNVM_EFUSE_DME_USER_KEY_1) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_USER_KEY_1_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_DME_USER_KEY_1_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_DME_USER_KEY_1_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_USER_KEY_NUM_OF_ROWS;
		EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	}
	else if (KeyType == XNVM_EFUSE_DME_USER_KEY_2) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_USER_KEY_2_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_DME_USER_KEY_2_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_DME_USER_KEY_2_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_USER_KEY_NUM_OF_ROWS;
		EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	}
	else if (KeyType == XNVM_EFUSE_DME_USER_KEY_3) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_DME_USER_KEY_3_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_DME_USER_KEY_3_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_DME_USER_KEY_3_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_DME_USER_KEY_NUM_OF_ROWS;
		EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/* Check DME key eFuse if they are already programmed */
	for (Row = EfusePrgmInfo.StartRow;
		Row < (EfusePrgmInfo.StartRow + EfusePrgmInfo.NumOfRows); Row++) {
		Status = XNvm_EfuseReadRow(EfusePrgmInfo.EfuseType, Row, &RegData);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (RegData != 0x00U) {
			Status = (int)XNVM_ERR_DME_KEY_ALREADY_PROGRAMMED;
			goto END;
		}
	}

	Status = XNvm_EfuseChangeEndianness((u8 *)Key, (u8 *)EfuseKey->Key, sizeof(Key));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, Key);
	if (Status != XST_SUCCESS) {
		Status = (Status |
				(XNVM_EFUSE_ERR_WRITE_DME_KEY_0 +
				(KeyType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reloads the cache of eFUSE so that can be directly
 * 		read from cache and programs required protections eFuses.
 *
 * @return	- XST_SUCCESS - On Successful Cache Reload.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
int XNvm_EfuseCacheLoadNPrgmProtectionBits(void)
{
	volatile int Status = XST_FAILURE;
	volatile int CloseStatus = XST_FAILURE;

	/**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

    /**
	 *  Reload Cache and check protection bits
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseCacheReloadAndProtectionChecks();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

    /**
	 *  Write protection bits inti eFuse. Return error code upon failure
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfusePrgmProtectionBits();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

    /**
	 *  Reload Cache and check protection bits
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseCacheReloadAndProtectionChecks();

END:
    /**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reloads the cache of eFUSE and check protection
 * 		eFuses.
 *
 * @return	- XST_SUCCESS - On Successful Cache Reload.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
static int XNvm_EfuseCacheReloadAndProtectionChecks(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseCacheReload();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Status = XST_FAILURE;
	Status = XNvm_EfuseProtectionChecks();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs the Protection eFuses.
 *
 * @return	- XST_SUCCESS - if protection efuse programming is successful.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmProtectionBits(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RowData = 0U;
	volatile u32 RowDataTmp = 0U;

	RowData  = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);
	RowDataTmp =  XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);
	if ((RowData != 0x00U) || (RowDataTmp != 0x00U)) {
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_SEC_CTRL_PROT_0_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_CTRL_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_SEC_CTRL_PROT_1_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_CTRL_1_PROT);
			goto END;
		}
	}

	RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;
	RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

	if ((RowData != 0x00U) || (RowDataTmp != 0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_SEC_MISC0_PROT_0_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC0_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_SEC_MISC0_PROT_1_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC0_1_PROT);
			goto END;
		}
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_CACHE_PPK0_HASH_OFFSET,
				XNVM_EFUSE_PPK_HASH_NUM_OF_CACHE_ROWS * 3U);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_PPK_HASH_PROT_0_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_PPK_HASH_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_PPK_HASH_PROT_1_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_PPK_HASH_1_PROT);
			goto END;
		}
	}

	Status = XNvm_EfuseCheckZeros(XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET,
				XNVM_EFUSE_METAHEADER_IV_NUM_OF_ROWS);
	if (Status != XST_SUCCESS) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_META_HEADER_EXPORT_DFT_PROT_0_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_METAHEADER_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_META_HEADER_EXPORT_DFT_PROT_1_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_METAHEADER_1_PROT);
			goto END;
		}
	}

	RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;
	RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;

	if ((RowData != 0x00U) || (RowDataTmp != 0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_CRC_PROT_0_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_CRC_0_PROT);
			goto END;
		}
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_CRC_PROT_1_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_CRC_1_PROT);
			goto END;

		}
	}

	RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
	RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
	if ((RowData != 0x00U) || (RowDataTmp != 0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_PUF_CHASH_PROT_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_PUF_CHASH_PROT);
			goto END;
		}
	}

	RowData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET) &
			XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK;
	RowDataTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET) &
			XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK;
	if ((RowData != 0x00U) || (RowDataTmp != 0x00U)) {
		Status = XST_FAILURE;
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_XILINX_CTRL_ROW,
				XNVM_EFUSE_ROW_0_SEC_MISC1_PROT_COL_NUM,
				XNVM_EFUSE_PROGRAM_VERIFY);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_ROW_0_SEC_MISC1_PROT);
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the Protection checks when the eFuse
 * 		cache is reloaded
 *
 * @return	- XST_SUCCESS - on successful protection checks.
 *		- XNVM_EFUSE_ERR_IN_PROTECTION_CHECK - Error in protection check
 *		- XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN - Error in Anchor bits
 *							pattern
 *
 ******************************************************************************/
static int XNvm_EfuseProtectionChecks(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 RegVal = 0U;
	volatile u32 RegValTmp = 0U;
	volatile u32 ProtVal = 0U;
	volatile u32 ProtValTmp = 0U;
	volatile u32 RowVal = 0U;
	volatile u32 RowValTmp = 0U;
	u32 Index;

	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_TBITS0_SVD_OFFSET);
	RegValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_TBITS0_SVD_OFFSET);

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_2_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_0_MASK);

	if ((ProtVal != (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
			XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK)) ||
		(ProtValTmp != (XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_3_MASK |
				XNVM_EFUSE_CACHE_TBITS0_SVD_ANCHOR_1_MASK))) {

		Status = (int)XNVM_EFUSE_ERR_ANCHOR_BIT_PATTERN;
		goto END;
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_CTRL_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SECURITY_CTRL_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC0_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_SECURITY_MISC_0_OFFSET) &
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK;

		if ((RowVal != RowValTmp) || (RowVal !=
			XNVM_EFUSE_CACHE_DEC_EFUSE_ONLY_MASK)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & (XNVM_EFUSE_CACHE_TBITS0_SVD_PPK_HASH_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_PPK_HASH_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = 0x0U;
		RowValTmp = 0x0U;

		for (Index = 0U; Index < XNVM_EFUSE_TOTAL_PPK_HASH_ROWS;
			Index++) {
			RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PPK0_HASH_OFFSET +
					(Index * XNVM_WORD_LEN));
			RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
					XNVM_EFUSE_CACHE_PPK0_HASH_OFFSET +
					(Index * XNVM_WORD_LEN));
			if ((RowVal != 0x0U) || (RowValTmp != 0x0U)) {
				break;
			}
		}
		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal  & (XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK);
	ProtValTmp = RegValTmp & (XNVM_EFUSE_CACHE_TBITS0_SVD_META_HEADER_EXPORT_PROT_MASK);

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = 0x0U;
		RowValTmp = 0x0U;

		for (Index = 0U; Index < XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS; Index++) {
			RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET +
				(Index * XNVM_WORD_LEN));
			RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET +
				(Index * XNVM_WORD_LEN));

			if ((RowVal != 0x0U) || (RowValTmp != 0x0U)) {
				break;
			}
		}
		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_CRC_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_ME_ID_CODE_OFFSET) &
				XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK;

		if ((RowVal != RowValTmp) ||
			(RowVal != XNVM_EFUSE_CACHE_ME_ID_CODE_CRC_SALT_MASK)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;
	ProtValTmp = RegValTmp & XNVM_EFUSE_CACHE_TBITS0_SVD_PUF_CHASH_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET);

		if ((RowVal != RowValTmp) || (RowVal == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	ProtVal = RegVal & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK;
	ProtValTmp = RegValTmp  & XNVM_EFUSE_CACHE_TBITS0_SVD_SEC_MISC1_PROT_MASK;

	if((ProtVal != 0x0U) || (ProtValTmp != 0x0U)) {
		RowVal = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET);
		RowValTmp = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
				XNVM_EFUSE_CACHE_SEC_MISC_1_OFFSET);

		if ((RowVal != RowValTmp) ||
			((RowVal & XNVM_EFUSE_SECURITY_MISC_1_PROT_MASK) == 0x0U)) {
			Status = (int)XNVM_EFUSE_ERR_IN_PROTECTION_CHECK;
			goto END;
		}
	}

	if ((ProtVal == ProtValTmp) && (RowVal == RowValTmp)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs all close operation of eFuse controller,
 * 		resets the read mode, disables programming mode	and locks the
 * 		controller back.
 *
 * @return	- XST_SUCCESS - On Successful Close.
 *		- XST_FAILURE - On Failure.
 *
 ******************************************************************************/
static int XNvm_EfuseCloseController(void)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseResetReadMode();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseDisableProgramming();
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XNvm_EfuseLockController();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below IV eFuses
 *		XNVM_EFUSE_ERR_WRITE_META_HEADER_IV /
 *		XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV /
 *		XNVM_EFUSE_ERR_WRITE_PLM_IV /
 *		XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV
 *
 * @param	IvType - Type of IV eFuses to be programmmed
 * @param	EfuseIv - Pointer to the XNvm_Iv structure which holds IV
 * 			to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM           - On Invalid Parameter.
 * 		- XNVM_EFUSE_ERR_WRITE_META_HEADER_IV    - Error while writing
 *							   Meta Iv.
 *		- XNVM_EFUSE_ERR_WRITE_BLK_OBFUS_IV 	 - Error while writing
 *							   BlkObfus IV.
 *		- XNVM_EFUSE_ERR_WRITE_PLM_IV 		 - Error while writing
 *							   PLM IV.
 * 		- XNVM_EFUSE_ERR_WRITE_DATA_PARTITION_IV - Error while writing
 * 							Data Partition IV.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmIv(XNvm_IvType IvType, XNvm_Iv *EfuseIv)
{
	volatile int Status = XST_FAILURE;
	u32 PrgmIv[XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS] = {0U};
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 IvOffset = 0U;
	u32 Row = 0U;
	u32 StartColNum = 0U;
	u32 EndColNum = 0U;
	u32 NumOfRows = 0U;
	volatile XNvm_IvType IvTypeTmp;

	if (IvType == XNVM_EFUSE_META_HEADER_IV_RANGE) {
		IvTypeTmp = XNVM_EFUSE_META_HEADER_IV_RANGE;
		IvOffset = XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_OFFSET;
		Row = XNVM_EFUSE_META_HEADER_IV_START_ROW;
		StartColNum = XNVM_EFUSE_METAHEADER_IV_RANGE_START_COL_NUM;
		EndColNum = XNVM_EFUSE_METAHEADER_IV_RANGE_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_METAHEADER_IV_NUM_OF_ROWS;
	}
	else if (IvType == XNVM_EFUSE_BLACK_IV) {
		IvTypeTmp = XNVM_EFUSE_BLACK_IV;
		IvOffset = XNVM_EFUSE_CACHE_BLACK_IV_OFFSET;
		Row = XNVM_EFUSE_BLACK_IV_START_ROW;
		StartColNum = XNVM_EFUSE_BLACK_IV_START_COL_NUM;
		EndColNum = XNVM_EFUSE_BLACK_IV_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_BLACK_IV_NUM_OF_ROWS;
	}
	else if (IvType == XNVM_EFUSE_PLM_IV_RANGE) {
		IvTypeTmp = XNVM_EFUSE_PLM_IV_RANGE;
		IvOffset = XNVM_EFUSE_CACHE_PLM_IV_RANGE_OFFSET;
		Row = XNVM_EFUSE_PLM_IV_START_ROW;
		StartColNum = XNVM_EFUSE_PLM_IV_RANGE_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PLM_IV_RANGE_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_PLM_IV_NUM_OF_ROWS;
	}
	else if (IvType == XNVM_EFUSE_DATA_PARTITION_IV_RANGE) {
		IvTypeTmp = XNVM_EFUSE_DATA_PARTITION_IV_RANGE;
		IvOffset = XNVM_EFUSE_CACHE_DATA_PARTITION_IV_OFFSET;
		Row = XNVM_EFUSE_DATA_PARTITION_IV_START_ROW;
		StartColNum = XNVM_EFUSE_DATA_PARTITION_IV_START_COL_NUM;
		EndColNum = XNVM_EFUSE_DATA_PARTITION_IV_END_COL_NUM;
		NumOfRows = XNVM_EFUSE_DATA_PARTITION_IV_NUM_OF_ROWS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (IvTypeTmp != IvType) {
		Status = XST_GLITCH_ERROR;
		goto END;
	}

	Status = XNvm_EfuseComputeProgrammableBits(EfuseIv->Iv, PrgmIv,
			IvOffset, (IvOffset +
			(XNVM_EFUSE_IV_NUM_OF_CACHE_ROWS * XNVM_WORD_LEN)));
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = Row;
	EfusePrgmInfo.ColStart = StartColNum;
	EfusePrgmInfo.ColEnd = EndColNum;
	EfusePrgmInfo.NumOfRows = NumOfRows;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, PrgmIv);

	if (Status != XST_SUCCESS) {
		Status = (Status |
				(XNVM_EFUSE_ERR_WRITE_META_HEADER_IV_RANGE +
				(IvType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses
 * 		PPK0/PPK1/PPK2 hash.
 *
 * @param	PpkType - Type of ppk hash to be programmed(PPK0/PPK1/PPK2)
 * @param	EfuseHash - Pointer to the XNvm_PpkHash structure which holds
 * 			PPK hash to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_WRITE_PPK0_HASH - Error while writing PPK0 Hash.
 *		- XNVM_EFUSE_ERR_WRITE_PPK1_HASH - Error while writing PPK1 Hash.
 * 		- XNVM_EFUSE_ERR_WRITE_PPK2_HASH - Error while writing PPK2 Hash.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmPpkHash(XNvm_PpkType PpkType, XNvm_PpkHash *EfuseHash)
{
	int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 Row = 0U;
	u32 StartColNum = 0U;
	u32 EndColNum = 0U;

	if (EfuseHash == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if(PpkType == XNVM_EFUSE_PPK0) {
		Row = XNVM_EFUSE_PPK0_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK0_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK0_HASH_END_COL_NUM;
	}
	else if(PpkType == XNVM_EFUSE_PPK1) {
		Row = XNVM_EFUSE_PPK1_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK1_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK1_HASH_END_COL_NUM;
	}
	else if (PpkType == XNVM_EFUSE_PPK2) {
		Row = XNVM_EFUSE_PPK2_HASH_START_ROW;
		StartColNum = XNVM_EFUSE_PPK2_HASH_START_COL_NUM;
		EndColNum = XNVM_EFUSE_PPK2_HASH_END_COL_NUM;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfusePrgmInfo.StartRow = Row;
	EfusePrgmInfo.ColStart = StartColNum;
	EfusePrgmInfo.ColEnd = EndColNum;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_PPK_HASH_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, EfuseHash->Hash);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_PPK0_HASH +
			(PpkType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function is used to program below eFuses
 * 		AES key/User key 0/User key 1.
 *
 * @param	KeyType - Type of key to be programmed
 * 			(AesKey/UserKey0/UserKey1)
 * @param	EfuseKey - Pointer to the XNvm_AesKey structure, which holds
 * 			Aes key to be programmed to eFuse.
 *
 * @return	- XST_SUCCESS - On Successful Programming.
 *		- XNVM_EFUSE_ERR_WRITE_AES_KEY	 - Error while writing
 *							AES key.
 *		- XNVM_EFUSE_ERR_WRITE_USER_KEY0 - Error while writing
 *							User key 0.
 * 		- XNVM_EFUSE_ERR_WRITE_USER_KEY1 - Error while writing
 *							User key 1.
 *
 ******************************************************************************/
static int XNvm_EfusePrgmAesKey(XNvm_AesKeyType KeyType, XNvm_AesKey *EfuseKey)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	int CrcRegOffset = 0U;
	int CrcDoneMask = 0U;
        int CrcPassMask	= 0U;
	u32 Crc;

	if (EfuseKey == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;
	EfusePrgmInfo.SkipVerify = XNVM_EFUSE_SKIP_VERIFY;

	if (KeyType == XNVM_EFUSE_AES_KEY) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_AES_KEY_0_TO_127_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_AES_KEY_0_TO_127_COL_START_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_AES_KEY_0_TO_127_COL_END_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_AES_KEY_0_TO_127_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		EfusePrgmInfo.StartRow = XNVM_EFUSE_AES_KEY_128_TO_255_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_AES_KEY_128_TO_255_COL_START_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_AES_KEY_128_TO_255_COL_END_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_AES_KEY_128_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseKey->Key[4U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_CRC_PASS_MASK;
	}
	else if (KeyType == XNVM_EFUSE_USER_KEY_0) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY0_0_TO_63_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY0_0_TO_63_COL_START_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY0_0_TO_63_COL_END_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY0_0_TO_63_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY0_64_TO_191_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY0_64_TO_191_COL_START_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY0_64_TO_191_COL_END_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY0_64_TO_191_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseKey->Key[2U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY0_192_TO_255_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY0_192_TO_255_COL_START_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY0_192_TO_255_COL_END_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY0_192_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseKey->Key[6U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY0);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_USR_KEY0_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_0_CRC_PASS_MASK;
	}
	else if (KeyType == XNVM_EFUSE_USER_KEY_1) {
		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY1_0_TO_63_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY1_0_TO_63_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY1_0_TO_63_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY1_0_TO_63_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, EfuseKey->Key);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY1_64_TO_127_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY1_64_TO_127_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY1_64_TO_127_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY1_64_TO_127_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseKey->Key[2U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		EfusePrgmInfo.StartRow = XNVM_EFUSE_USER_KEY1_128_TO_255_START_ROW;
		EfusePrgmInfo.ColStart = XNVM_EFUSE_USER_KEY1_128_TO_255_START_COL_NUM;
		EfusePrgmInfo.ColEnd = XNVM_EFUSE_USER_KEY1_128_TO_255_END_COL_NUM;
		EfusePrgmInfo.NumOfRows = XNVM_EFUSE_USER_KEY1_128_TO_255_NUM_OF_ROWS;

		Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &EfuseKey->Key[4U]);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_WRITE_USER_KEY1);
			goto END;
		}

		CrcRegOffset = XNVM_EFUSE_AES_USR_KEY1_CRC_REG_OFFSET;
		CrcDoneMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_DONE_MASK;
		CrcPassMask = XNVM_EFUSE_CTRL_STATUS_AES_USER_KEY_1_CRC_PASS_MASK;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseCacheReloadAndProtectionChecks();
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_AES_KEY);
		goto END;
	}
	Crc = XNvm_AesCrcCalc(EfuseKey->Key);

	Status = XST_FAILURE;
	Status = XNvm_EfuseCheckAesKeyCrc(CrcRegOffset, CrcDoneMask,
                        CrcPassMask, Crc);
	if (Status != XST_SUCCESS) {
		Status = (Status |
			(XNVM_EFUSE_ERR_WRITE_AES_KEY + (KeyType << XNVM_EFUSE_ERROR_BYTE_SHIFT)));
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function programs ROM Rsvd Bits eFuses.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 *				when set to true it will not check for voltage
 *				and temperature limits.
 * @param	RomRsvdBits - RomRsvdBits to be programmed to RomRsvd eFUSE bits
 *
 * @return	- XST_SUCCESS - On successful write.
 *		- XNVM_EFUSE_ERR_WRITE_ROM_RSVD - Error while writing
 *					ROM Rsvd eFuse bits.
 *
 ******************************************************************************/
int XNvm_EfuseWriteRomRsvdBits(u32 EnvDisFlag, u32 RomRsvdBits)
{
	volatile int Status = XST_FAILURE;
	int CloseStatus = XST_FAILURE;
	XNvm_EfusePrgmInfo EfusePrgmInfo = {0U};
	u32 WrRomRsvdBits;
	XSysMonPsv *SysMonInstPtr = XPlmi_GetSysmonInst();

	if (EnvDisFlag != TRUE) {
		/**
		 *  Perform Environmental monitoring checks
		 */
		Status = XNvm_EfuseTempAndVoltChecks(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
			goto END;
		}
	}

    /**
	 *  Unlock eFuse Controller. Return appropriate error code if not success
	 */
	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	Status = XST_FAILURE;
	/**
	 *  compute programmable bits
	 */
	Status = XNvm_EfuseComputeProgrammableBits(&RomRsvdBits,
				&WrRomRsvdBits,
				XNVM_EFUSE_CACHE_ROM_RSVD_OFFSET,
				XNVM_EFUSE_CACHE_ROM_RSVD_OFFSET);
	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_BEFORE_PROGRAMMING);
		goto END;
	}

	EfusePrgmInfo.StartRow = XNVM_EFUSE_ROM_RSVD_START_ROW;
	EfusePrgmInfo.ColStart = XNVM_EFUSE_ROM_RSVD_START_COL;
	EfusePrgmInfo.ColEnd = XNVM_EFUSE_ROM_RSVD_END_COL;
	EfusePrgmInfo.NumOfRows = XNVM_EFUSE_ROM_RSVD_NUM_OF_ROWS;
	EfusePrgmInfo.EfuseType = XNVM_EFUSE_PAGE_0;

	Status = XST_FAILURE;
	/**
	 *   Program and verify RomRsvdBits
	 */
	Status = XNvm_EfusePgmAndVerifyData(&EfusePrgmInfo, &WrRomRsvdBits);

	if (Status != XST_SUCCESS) {
		Status = (Status | XNVM_EFUSE_ERR_WRITE_ROM_RSVD_BITS);
	}

END:
	/**
	 *  Lock eFuse controller
	 */
	CloseStatus = XNvm_EfuseCloseController();
	if (XST_SUCCESS == Status) {
		Status |= CloseStatus;
	}

	return Status;

}

/******************************************************************************/
/**
 * @brief	This function is used to compute the eFuse bits to be programmed
 * 			to the eFuse.
 *
 * @param	ReqData  - Pointer to the user provided eFuse data to be written.
 * @param	PrgmData - Pointer to the computed eFuse bits to be programmed,
 * 			which means that this API fills only unprogrammed
 * 			and valid bits.
 * @param	StartOffset - A 32 bit Start Cache offset of an expected
 * 			Programmable Bits.
 * @param	EndOffset   - A 32 bit End Cache offset of an expected
 * 			Programmable Bits.
 *
 * @return	- XST_SUCCESS - if the eFuse data computation is successful.
 *		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *		- XNVM_EFUSE_ERR_CACHE_PARITY  - Error in Cache reload.
 *
 ******************************************************************************/
static int XNvm_EfuseComputeProgrammableBits(const u32 *ReqData, u32 *PrgmData,
						u32 StartOffset, u32 EndOffset)
{
	volatile int Status = XST_FAILURE;
	int IsrStatus = XST_FAILURE;
	u32 ReadReg = 0U;
	volatile u32 Offset = 0U;
	u32 Idx = 0U;

	if ((ReqData == NULL) || (PrgmData == NULL)) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	IsrStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR)
			== XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_PARITY;
		goto END;
	}

	Offset = StartOffset;
	while (Offset <= EndOffset) {
		ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Offset);

		Idx = (Offset - StartOffset) / XNVM_WORD_LEN;
		PrgmData[Idx] = (~ReadReg) & ReqData[Idx];
		Offset = Offset + XNVM_WORD_LEN;
	}

	if (Offset == (EndOffset + XNVM_WORD_LEN)) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}



/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified bits
 *		in the eFUSE.
 *
 * @param	EfusePrgmInfo - Pointer to XNvm_EfusePrgmInfo structure
 * 				stores the info required to program the eFuses
 * @param	RowData   - Pointer to memory location where bitmap to be
 * 			written is stored. Only bit set are used for programming
 * 			eFUSE.
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *  		- XNVM_EFUSE_ERR_INVALID_PARAM - On Invalid Parameter.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyData(XNvm_EfusePrgmInfo *EfusePrgmInfo, const u32* RowData)
{
	volatile int Status = XNVM_EFUSE_ERR_NTHG_TO_BE_PROGRAMMED;
	const u32* DataPtr = RowData;
	volatile u32 Row = EfusePrgmInfo->StartRow;
	volatile u32 EndRow = EfusePrgmInfo->StartRow + EfusePrgmInfo->NumOfRows;
	u32 Idx = 0U;
	u32 Col = 0U;
	u32 Data;

	if ((EfusePrgmInfo->EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfusePrgmInfo->EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfusePrgmInfo->EfuseType != XNVM_EFUSE_PAGE_2)){

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}
	if ((DataPtr == NULL) || (EfusePrgmInfo->NumOfRows == 0U)) {

		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	Data = *DataPtr;
	while (Row < EndRow) {
		Col = EfusePrgmInfo->ColStart;
		while (Col <= EfusePrgmInfo->ColEnd) {
			if ((Data & 0x01U) != 0U) {
				Status = XST_FAILURE;
				Status = XNvm_EfusePgmAndVerifyBit(
						EfusePrgmInfo->EfuseType, Row,
						Col, EfusePrgmInfo->SkipVerify);
				if (Status != XST_SUCCESS) {
					XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
					goto END;
				}
			}
			Col++;
			Idx++;
			if (Idx == XNVM_EFUSE_MAX_BITS_IN_ROW) {
				DataPtr++;
				Data = *DataPtr;
				Idx = 0;
			} else {
				Data = Data >> 1U;
			}
		}
		Row++;
	}

	if (Row != EndRow) {
		Status = (int)XNVM_EFUSE_ERR_GLITCH_DETECTED;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets the specified bit in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS	- Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *		- XNVM_EFUSE_ERR_PGM - eFUSE programming failed.
 *
 ******************************************************************************/
static int XNvm_EfusePgmBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	volatile int Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask = 0U;

	PgmAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
		(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_ISR_REG_OFFSET),
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR),
			XNVM_EFUSE_PGM_TIMEOUT_VAL,
			&EventMask);

	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_PGM_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_PGM_ERROR) == XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_PGM;
	}
	else {
		Status = XST_SUCCESS;
	}

        XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, XNVM_EFUSE_ISR_REG_OFFSET,
			(XNVM_EFUSE_ISR_PGM_DONE | XNVM_EFUSE_ISR_PGM_ERROR));

        return Status;
}

/******************************************************************************/
/**
 * @brief	This function verify the specified bit set in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row - It is an 32-bit Row number (0-based addressing).
 * @param	Col - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						   is not set.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occurred.
 *		- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseReadRow(XNvm_EfuseType Page, u32 Row, u32 *RegData)
{
	volatile int Status = XST_FAILURE;
	u32 RdAddr;
	u32 EventMask = 0x00U;

	RdAddr = ((u32)Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
		(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);

	Status = (int)Xil_WaitForEvents((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
		XNVM_EFUSE_ISR_REG_OFFSET),
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_ISR_RD_DONE,
		XNVM_EFUSE_RD_TIMEOUT_VAL,
		&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = (int)XNVM_EFUSE_ERR_RD_TIMEOUT;
		goto END;
	}

	if ((EventMask & XNVM_EFUSE_ISR_RD_DONE) == XNVM_EFUSE_ISR_RD_DONE) {
		*RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function verify the specified bit set in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row - It is an 32-bit Row number (0-based addressing).
 * @param	Col - It is an 32-bit Col number (0-based addressing).
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						   is not set.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - If Programming timeout has occured.
 *		- XST_FAILURE                - Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfuseVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col)
{
	int Status = XST_FAILURE;
	volatile u32 RegData = 0x00U;

	Status = XNvm_EfuseReadRow(Page, Row, (u32 *)&RegData);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((RegData & (((u32)0x01U) << Col)) != 0U) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_PGM_VERIFY;
	}

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET,
			XNVM_EFUSE_ISR_RD_DONE);
END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function sets and then verifies the specified
 *			bit in the eFUSE.
 *
 * @param	Page - It is an enum variable of type XNvm_EfuseType.
 * @param	Row  - It is an 32-bit Row number (0-based addressing).
 * @param	Col  - It is an 32-bit Col number (0-based addressing).
 * @param	SkipVerify - Skips verification of bit if set to non zero
 *
 * @return	- XST_SUCCESS - Specified bit set in eFUSE.
 *		- XNVM_EFUSE_ERR_PGM_TIMEOUT - eFUSE programming timed out.
 *		- XNVM_EFUSE_ERR_PGM 	- eFUSE programming failed.
 *		- XNVM_EFUSE_ERR_PGM_VERIFY  - Verification failed, specified bit
 *						is not set.
 *		- XST_FAILURE 	- Unexpected error.
 *
 ******************************************************************************/
static int XNvm_EfusePgmAndVerifyBit(XNvm_EfuseType Page, u32 Row, u32 Col, u32 SkipVerify)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 SkipVerifyTmp = SkipVerify;

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_EfusePgmBit, Page, Row, Col);
	if((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		if ((SkipVerify == XNVM_EFUSE_PROGRAM_VERIFY) || (SkipVerifyTmp == XNVM_EFUSE_PROGRAM_VERIFY)) {
			/* Return XST_GLITCH_ERROR in case of glitch */
			if (SkipVerify != SkipVerifyTmp) {
				Status = XST_GLITCH_ERROR;
				goto END;
			}
			Status = XST_FAILURE;
			Status = XNvm_EfuseVerifyBit(Page, Row, Col);
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief       This function changes the endianness of Data of given size
 *
 * @param	Dest    Pointer to the destination buffer
 * @param	Src	Pointer to the source buffer
 * @param	Size	Size of the Data
 *
 * @return
 *	-	XST_SUCCESS on successful conversion
 *	-	Error code on failure
 *
 ******************************************************************************/
static int XNvm_EfuseChangeEndianness(u8 *Dest, u8 *Src, u32 Size)
{
	int Status = XST_FAILURE;
	volatile u32 Index;

	if ((Size % XNVM_WORD_LEN) != 0U) {
		Status = XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	for (Index = 0U; Index < Size; Index++) {
		Dest[Size - 1U - Index] = Src[Index];
	}

	if (Index == Size) {
		Status = XST_SUCCESS;
	}
END:
	return Status;
}
