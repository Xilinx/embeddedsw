/*******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_bbram.c
*
* This file contains NVM library BBRAM functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   mmd  04/01/2019 Initial release
* 2.0   kal  11/14/2019 Added error check when BBRAM keylen is not 128 or 256.
* 2.1	am   08/19/2020 Resolved MISRA C violations.
*	kal  09/03/2020 Fixed Security CoE review comments
*	am   10/13/2020 Resolved MISRA C violations
* 2.3   am   11/23/2020 Resolved MISRA C violation Rule 10.6
* 	kal  12/23/2020 Disable BBRAM programming in error case also
*	kal  01/27/2021	Zeroize BBRAM in case of CRC mismatch
*			Zeroize BBRAM User Data in case of write failure
* 2.4   kal  07/13/2021 Fixed doxygen warnings
*       kal  08/03/2021 Removed clearing BBRAM UsrData in case for write failure
* 3.0   kal  08/01/2022 Added redundancy to XNvm_BbramEnablePgmMode function
*       dc   08/29/2022 Changed u8 to u32 type
* 3.1   skg  10/23/2022 Added In body comments for APIs
* 3.4   pre  09/11/2024 Removed zeroization before writing key
*       ng   09/20/2024 Fixed doxygen grouping
*
* </pre>
*
*******************************************************************************/

/**
 * @addtogroup xnvm_bbram_server_apis XilNvm BBRAM Server APIs
 * @{
 */

/***************************** Include Files **********************************/
#include "xil_io.h"
#include "xstatus.h"
#include "xnvm_bbram.h"
#include "xnvm_bbram_hw.h"
#include "xnvm_utils.h"
#include "xnvm_defs.h"

/*************************** Constant Definitions *****************************/

#define REVERSE_POLYNOMIAL			(0x82F63B78U)
						/**< Polynomial used for CRC calculation */

#ifdef VERSAL_AIEPG2
#define XNVM_CL_ENABLE_SHIFT			(30U)
			/**< Shift for enabling Configuration Limiter feature */
#define XNVM_CL_MODE_SHIFT			(28U)
			/**< Shift for Counter mode in Configuration Limiter */
#define XNVM_CL_MAX_COUNT_VAL			(0x0FFFFFFFU)
/**< Max value of counter for total/failed configurations in configuration limiter*/
#define XNVM_RTCFG_SECURESTATE_AHWROT_ADDR	(0xF201414CU)
/**< Address of register in Run time configuration area where the state of A-HWRoT is stored */
#define XNVM_RTCFG_SECURESTATE_SHWROT_ADDR	(0xF2014150U)
/**< Address of register in Run time configuration area where the state of S-HWRoT is stored */
#define XNVM_RTCFG_SECURESTATE_AHWROT		(0xA5A5A5A5U)
/**< Value to indicate that Secure State of boot is A-HWRoT */
#define XNVM_RTCFG_SECURESTATE_SHWROT		(0x96969696U)
/**< Value to indicate that Secure State of boot is S-HWRoT */

#endif

/***************************** Type Definitions *******************************/

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 * @brief	This function reads the given register.
 *
 * @param	Offset is the register offset of the register.
 *
 * @return	The 32-bit value of the register.
 *
 ******************************************************************************/
static INLINE u32 XNvm_BbramReadReg(u32 Offset)
{
	/**
	 *  Performs Reading the BBRAM register at the given offset
	 */
	return Xil_In32((UINTPTR)(XNVM_BBRAM_BASE_ADDR + Offset));
}

/******************************************************************************/
/**
 * @brief	This function writes the value into the given register.
 *
 * @param	Offset is the register offset of the register.
 * @param	Data is the 32-bit value to write to the register.
 *
 ******************************************************************************/
static INLINE void XNvm_BbramWriteReg(u32 Offset, u32 Data)
{
	/**
	 *  Writes the data in to BBRAM register at given offset
	 */
	Xil_Out32((UINTPTR)(XNVM_BBRAM_BASE_ADDR + Offset), Data);
}

/*************************** Function Prototypes ******************************/
static inline int XNvm_BbramEnablePgmMode(void);
static inline int XNvm_BbramDisablePgmMode(void);
static int XNvm_BbramValidateAesKeyCrc(const u32* Key);
static int XNvm_BbramWriteBbram8(u32 Data);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	This function does programming of key provided into BBRAM and
 * 		validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param	Key - Pointer to hex buffer which is pointing to key
 * @param 	KeyLen - XNVM_256_BITS_AES_KEY_LEN_IN_BYTES for 256-bit AES key
 *
 * @return
 * 		- XST_SUCCESS  Key is written to BBRAM.
 *		- XST_INVALID_PARAM  Invalid parameter passed.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT  Timeout during enabling
 *							programming mode.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_DISABLE_TIMEOUT  Timeout during disabling
 *							programming mode.
 *		- XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT  CRC validation check timed out.
 *		- XNVM_BBRAM_ERROR_AES_CRC_MISMATCH  CRC mismatch.
 *
 ******************************************************************************/
int XNvm_BbramWriteAesKey(const u8* Key, u16 KeyLen)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	int DisableStatus = XST_FAILURE;
	int ZeroizeStatus = XST_FAILURE;
	const u32 *AesKey = NULL;
	u32 BbramKeyAddr;
	u32 Idx;

        /**
	 *  Perform input parameter validation.
	 *  Return appropriate error code if input parameters are invalid.
	 */
	if ((KeyLen != XNVM_256_BITS_AES_KEY_LEN_IN_BYTES) ||
		(Key == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

        /**
	 * Assign the key address to local pointer.
	 */
	AesKey = (const u32 *) Key;

	/**
	 * As per hardware design, zeroization is must between two BBRAM
	 * AES CRC Check requests.
	 */
	ZeroizeStatus = XNvm_BbramZeroize();
	if (ZeroizeStatus != XST_SUCCESS) {
		goto END;
	}

        /**
	 *  Bring BBRAM to programming mode by writing Magic Word 0x757BDF0D to PGM_MODE register.
	 */
	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XNvm_BbramEnablePgmMode);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = (Status | StatusTmp);
		goto END;
	}

        /**
	 * Write 256-bit AES Key to BBRAM registers BBRAM_0 to BBRAM_7.
	 */
	BbramKeyAddr = XNVM_BBRAM_0_REG;
	for (Idx = 0U; Idx < XNVM_BBRAM_AES_KEY_SIZE_IN_WORDS; Idx++) {

		XNvm_BbramWriteReg(BbramKeyAddr, AesKey[Idx]);
		BbramKeyAddr += sizeof(u32);
	}

        /**
	 *  @{ Calculate CRC on input AES Key and write it to BBRAM_AES_CRC which triggers BBRAM CRC integrity check.
	 *  Wait for AES_CRC_DONE bit to set in BBRAM_STATUS register with timeout of 1 second.
	 *  If timed out return timeout error.
	 *  If AES_CRC_PASS bit is set in BBRAM_STATUS register, return XST_SUCCESS else return CRC mismatch error.
	 */
	Status = XNvm_BbramValidateAesKeyCrc(AesKey);
	if (Status != XST_SUCCESS) {
		ZeroizeStatus = XNvm_BbramZeroize();
		if (ZeroizeStatus != XST_SUCCESS) {
			Status = (Status | ZeroizeStatus);
			goto END;
		}
	}

END:
        /**
	 *  Disable BBRAM programming mode by writing 0x00 to PGM_MODE register.
	 */
	DisableStatus = XNvm_BbramDisablePgmMode();
	if ((DisableStatus != XST_SUCCESS) && (Status == XST_SUCCESS)) {
		Status = DisableStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function locks the user data written in BBRAM_8 that is
 * 		making user data written in BBRAM as read only.
 *
 * @return
 *		- XST_SUCCESS  Success in locking the BBRAM user data.
 *		- XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE  Failure to lock BBRAM user data.
 *
 ******************************************************************************/
int XNvm_BbramLockUsrDataWrite(void)
{
	int Status = XST_FAILURE;
	u32 LockStatus = 0U;

        /**
	 *  Write to BBRAM Lock register and readback to confirm if lock is successful.
	 *  Return the status of the lock, Success if the lock is done else error.
	 */
	XNvm_BbramWriteReg(XNVM_BBRAM_MSW_LOCK_REG, XNVM_BBRAM_MSW_LOCK);
	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);
	if((LockStatus & XNVM_BBRAM_MSW_LOCK) == XNVM_BBRAM_MSW_LOCK) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_BBRAM_ERROR_LOCK_USR_DATA_WRITE;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Programs BBRAM_8 register
 *
 * @param	Data - 32-bit data to be written to BBRAM_8 register
 *
 * @return
 * 		- XST_SUCCESS  Successfully programmed data in BBRAM_8 register
 *		- XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED  Failure since write is locked for the register
 *
 ******************************************************************************/
static int XNvm_BbramWriteBbram8(u32 Data)
{
	int Status = XST_FAILURE;
	u32 LockStatus;
	u32 ReadReg;

        /**
	 * Check for BBRAM Lock register for Lock.
         * If it Locked then return XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED
	 */
	LockStatus = XNvm_BbramReadReg(XNVM_BBRAM_MSW_LOCK_REG);
	if((LockStatus & XNVM_BBRAM_MSW_LOCK) == XNVM_BBRAM_MSW_LOCK) {
		Status = (int)XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED;
	}
	else {
		/**
		 * Else write user data to BBRAM and Return XST_SUCCESS
		 */
		XNvm_BbramWriteReg(XNVM_BBRAM_8_REG, Data);
		ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_8_REG);
		if (ReadReg == Data) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Writes user provided 32-bit data to BBRAM.
 *
 * @param	GeneralPurposeData - 32-bit user data to be written to BBRAM
 *
 * @return
 * 		- XST_SUCCESS  User data written to BBRAM
 *  		- XNVM_BBRAM_INVALID_PARAM  Invalid input parameter
 *		- XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED  Failure since write is locked for the register
 *
 * @note	Provisoning of general purpose data is allowed only
 * 		if Symmetric/Asymeetric HWRoT boot is not enabled for Versal Gen 2 devices
 *
 ******************************************************************************/
int XNvm_BbramWriteUsrData(u32 GeneralPurposeData)
{
	int Status = XST_FAILURE;
#ifdef VERSAL_AIEPG2
	u32 AHwRotState = Xil_In32(XNVM_RTCFG_SECURESTATE_AHWROT_ADDR);
	u32 SHwRotState = Xil_In32(XNVM_RTCFG_SECURESTATE_SHWROT_ADDR);

	if ((AHwRotState != XNVM_RTCFG_SECURESTATE_AHWROT) &&
		(SHwRotState != XNVM_RTCFG_SECURESTATE_SHWROT)) {
		Status = XNvm_BbramWriteBbram8(GeneralPurposeData);
	}
#else
	Status = XNvm_BbramWriteBbram8(GeneralPurposeData);
#endif

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads 32-bit user data from BBRAM_8 register.
 *
 * @return  	32-bit user data stored in BBRAM_8 register
 *
 ******************************************************************************/
u32 XNvm_BbramReadUsrData(void)
{
	/**
	 * Read the 32 bit user data from the BBRAM_8 register and return it
	 */
	return XNvm_BbramReadReg(XNVM_BBRAM_8_REG);
}

/******************************************************************************/
/**
 * @brief	This function zeroizes the BBRAM.
 *
 * @return
 *		- XST_SUCCESS  Success in Zeroization of BBRAM.
 *		- XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT  If Timed out during BBRAM zeroization
 *
 ******************************************************************************/
int XNvm_BbramZeroize(void)
{
	int Status = XST_FAILURE;

	/**
	 * Write 1 to BBRAM_CTRL register.
	 * Wait for BBRAM_ZEROIZED bit to set in BBRAM_STATUS register with timeout of 1 second.
	 * If timed out return timout error. Return XST_SUCCESS.
	 */
	XNvm_BbramWriteReg(XNVM_BBRAM_CTRL_REG, XNVM_BBRAM_CTRL_START_ZEROIZE);
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG),
		XNVM_BBRAM_STATUS_ZEROIZED,
		XNVM_BBRAM_STATUS_ZEROIZED,
		XNVM_BBRAM_ZEROIZE_TIMEOUT_VAL);

	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_BBRAM_ERROR_ZEROIZE_TIMEOUT;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enables the BBRAM programming mode by writing
 * 		to the BBRAM_PGM_MODE register with the required value.
 *
 * @return
 *		- XST_SUCCESS  Enabled programming mode.
 *		- XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT  On failure.
 *
 ******************************************************************************/
static inline int XNvm_BbramEnablePgmMode(void)
{
	int Status = XST_FAILURE;

	/**
	 * Write PassCode 0x757BDF0DU to BBRAM_PGM_MODE register to enable programming mode.
         * Wait for BBRAM_PGM_MODE_DONE bit to set in BBRAM_STATUS register with timeout of 1 second.
	 * If timed out return timout error else Return XST_SUCCESS.
         */
	XNvm_BbramWriteReg(XNVM_BBRAM_PGM_MODE_REG,
	                   XNVM_EFUSE_PGM_MODE_PASSCODE);
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG),
	        XNVM_BBRAM_STATUS_PGM_MODE_DONE,
	        XNVM_BBRAM_STATUS_PGM_MODE_DONE,
	        XNVM_BBRAM_PGM_MODE_TIMEOUT_VAL);

	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_BBRAM_ERROR_PGM_MODE_ENABLE_TIMEOUT;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function disables the programming mode of the BBRAM by
 * 		writing to 0 BBRAM_PGM_MODE register.
 *
 * @return
 *		- XST_SUCCESS  Disabled programming mode.
 *		- XST_FAILURE  On failure in disabling programming mode.
 *
 ******************************************************************************/
static inline int XNvm_BbramDisablePgmMode(void)
{
	int Status = XST_FAILURE;

	/**
	 * Write 0x0 to BBRAM_PGM_MODE register.
         * Read back and verify that 0x0 is written to BBRAM_PGM_MODE register.
	 * If PGM_MODE is disabled return XST_SUCCESS else XST_FAILURE.
         */
	u32 ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_PGM_MODE_REG);

	XNvm_BbramWriteReg(XNVM_BBRAM_PGM_MODE_REG, 0x00U);

	ReadReg = XNvm_BbramReadReg(XNVM_BBRAM_PGM_MODE_REG);
	if (ReadReg == 0x00U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	Validates CRC of the key stored in BBRAM with CRC of input key.
 *
 * @param   	Key - Pointer to key which is to be matched with key
 *				  stored in BBRAM.
 *
 * @return
 *		- XST_SUCCESS  CRC matched
 *		- XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT  CRC validation check timed out.
 *		- XNVM_BBRAM_ERROR_AES_CRC_MISMATCH  CRC mismatch
 *
 ******************************************************************************/
static int XNvm_BbramValidateAesKeyCrc(const u32* Key)
{
	int Status = XST_FAILURE;
	u32 Crc;
	u32 BbramStatus;

	/**
	 * Calculate the 32 bit CRC of the input key.
         * Write that CRC to BBRAM_AES_CRC_REG register.
	 * Wait for BBRAM_AES_CRC_DONE to set in BBRAM_STATUS register with timeout of 1 second.
	 * If timed out return timeout error else Check for CRC_PASS.
	 * Check for BBRAM_AES_CRC_PASS bit in BBRAM_STATUS register.
	 * If set return XST_SUCCESS else return XNVM_BBRAM_ERROR_AES_CRC_MISMATCH error.
         */
	Crc = XNvm_AesCrcCalc(Key);

	XNvm_BbramWriteReg(XNVM_BBRAM_AES_CRC_REG, Crc);
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_BBRAM_BASE_ADDR + XNVM_BBRAM_STATUS_REG),
	        XNVM_BBRAM_STATUS_AES_CRC_DONE,
	        XNVM_BBRAM_STATUS_AES_CRC_DONE,
	        XNVM_BBRAM_AES_CRC_DONE_TIMEOUT_VAL);

	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_BBRAM_ERROR_AES_CRC_DONE_TIMEOUT;
		goto END;
	}

	Status = XST_FAILURE;

	BbramStatus = XNvm_BbramReadReg(XNVM_BBRAM_STATUS_REG);

	if((BbramStatus & XNVM_BBRAM_STATUS_AES_CRC_PASS)
			== XNVM_BBRAM_STATUS_AES_CRC_PASS) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_BBRAM_ERROR_AES_CRC_MISMATCH;
	}

END:
	return Status;
}

#ifdef VERSAL_AIEPG2
/******************************************************************************/
/**
 * @brief	This function provisions BBRAM_8 register with the parameters of
 * 		configuration limiter.
 * 		-------------------------------------------------------
 *		| CL enable(31:30) | CL mode (29:28) | Counter (27:0) |
 * 		-------------------------------------------------------
 *
 * 		CL enable indicates that the Configuration Limiter feature is
 * 		enabled/disabled.
 * 		CL mode indicates if the counter maintains the count of
 * 		failed/total configurations.
 * 		Counter indicates the counter of failed/total configurations
 * 		depending on the CL mode.
 *
 * @param	ClEnFlag - Flag to indicate if the configuration limiter feature is enabled/disabled
 * @param	ClMode - Flag to indicate if the counter maintains the count of failed/total
 * 		configurations.
 * @param	MaxNumOfConfigs - Value of maximum number of configurations(failed/total) which are
 * 		allowed
 *
 * @return
 * 		- XST_SUCCESS  Configuration parameters are written to BBRAM
 * 		- XNVM_BBRAM_INVALID_PARAM  Invalid input parameter
 *		- XNVM_BBRAM_ERROR_USR_DATA_WRITE_LOCKED  Failure since write is locked for the register
 *
 * @note	Provisoning of Configuration limiter parameters is allowed only
 * 		in case of Symmetric/Asymeetric HWRoT boot.
 *
 ******************************************************************************/
int XNvm_BbramWriteConfigLimiterParams(u32 ClEnFlag, u32 ClMode, u32 MaxNumOfConfigs)
{
	int Status = XST_FAILURE;
	u32 AHwRotState = Xil_In32(XNVM_RTCFG_SECURESTATE_AHWROT_ADDR);
	u32 SHwRotState = Xil_In32(XNVM_RTCFG_SECURESTATE_SHWROT_ADDR);
	u32 ValToBeProvisioned;

	if ((AHwRotState != XNVM_RTCFG_SECURESTATE_AHWROT) &&
		(SHwRotState != XNVM_RTCFG_SECURESTATE_SHWROT)) {
		Status = (int)XNVM_BBRAM_INVALID_PARAM;
		goto END;
	}

	if ((ClEnFlag != XNVM_BBRAM_CONFIG_LIMITER_DISABLED) &&
		(ClEnFlag != XNVM_BBRAM_CONFIG_LIMITER_ENABLED)) {
		Status = (int)XNVM_BBRAM_INVALID_PARAM;
		goto END;
	}

	if ((ClMode != XNVM_BBRAM_CONFIG_LIMITER_FAIL_CONFIGS_COUNT) &&
		(ClMode != XNVM_BBRAM_CONFIG_LIMITER_TOTAL_CONFIGS_COUNT)) {
		Status = (int)XNVM_BBRAM_INVALID_PARAM;
		goto END;
	}

	if (MaxNumOfConfigs > XNVM_CL_MAX_COUNT_VAL) {
		Status = (int)XNVM_BBRAM_INVALID_PARAM;
		goto END;
	}

	ValToBeProvisioned = (ClEnFlag << XNVM_CL_ENABLE_SHIFT) | (ClMode << XNVM_CL_MODE_SHIFT);
	ValToBeProvisioned = ValToBeProvisioned | MaxNumOfConfigs;

	Status = XNvm_BbramWriteBbram8(ValToBeProvisioned);

END:
	return Status;

}
#endif
