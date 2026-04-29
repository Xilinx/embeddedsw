/*******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
*
* @file xnvm_utils.c
*
* This file contains NVM library utility functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kpt  08/14/2024 Initial release
* 1.1   mb   04/03/2025 Remove EMC clock control enable
* 3.6   mb   06/25/2025 Update doxygen comments
*       mb   07/22/2025 Update doxygen comments for XNvm_AesCrcCalc
*       aa   07/24/2025 Typecast to essential datatypes to avoid implicit conversions
*       mb   20/08/2025 Update Timer calculate API's by passing Frequency value to macros
* 3.7   mb   03/18/2026 Add support for temperature and voltage before efuse programming
*       mb    03/25/2026 Use VCCAUX channel to read voltage for efuse programming checks
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xnvm_utils.h"
#include "xstatus.h"
#include "xil_sutil.h"

/*************************** Constant Definitions *****************************/

/**@cond xnvm_internal
 * @{
 */
/**< Polynomial used for CRC calculation */
#define REVERSE_POLYNOMIAL			(0x82F63B78U)

/**<  Timer related macros */
#define Tpgm(ClkFreq) \
	XIL_SCEILDIV(u64, 5U * (u64)(ClkFreq), 1000000U)
#define Trd(ClkFreq) \
	XIL_SCEILDIV(u64, 15U * (u64)(ClkFreq), 100000000U)
#define Tsu_h_ps(ClkFreq) \
	XIL_SCEILDIV(u64, 67U * (u64)(ClkFreq), 1000000000U)
#define Tsu_h_ps_cs(ClkFreq) \
	XIL_SCEILDIV(u64, 46U * (u64)(ClkFreq), 1000000000U)
#define Tsu_h_cs(ClkFreq) \
	XIL_SCEILDIV(u64, 30U * (u64)(ClkFreq), 1000000000U)

/**< Temperature limits for Spartan UltraScale Plus eFuses */
#define XNVM_EFUSE_TEMP_MIN			(-40.0f)
#define XNVM_EFUSE_TEMP_MAX			(125.0f)

/**< Voltage limits for VCCAUX for Spartan UltraScale Plus */
#define XNVM_EFUSE_VCCAUX_MIN		(1.746f)
#define XNVM_EFUSE_VCCAUX_MAX		(1.854f)

/**
 * @}
 * @endcond
 */

/***************************** Function Definitions *******************************/

static int XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
static int XNvm_EfuseCheckForTBits(void);
static void XNvm_EfuseEnableProgramming(void);
static void XNvm_EfuseInitTimers(u32 ClkFreq);
#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
static int XNvm_EfuseTemperatureCheck(float Temperature);
static int XNvm_EfuseVccauxVoltageCheck(float Voltage);
#endif

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 * @brief	This function sets read mode of eFUSE controller.
 *
 * @param	RdMode - Mode to be used for eFUSE read.
 *
 * @return
 * 		- XST_SUCCESS - if Setting read mode is successful.
 * 		- XST_FAILURE - if there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode)
{
	int Status = XST_FAILURE;
	u32 RegVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 NewRegVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 RdModeVal = XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET;
	u32 Mask = XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET;

	/**
	 *  - Read EFUSE_CFG_REG register
	 */
	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				   XNVM_EFUSE_CFG_OFFSET);
	if (XNVM_EFUSE_NORMAL_RD == RdMode) {
		Mask = XNVM_EFUSE_CFG_NORMAL_RD;
	} else {
		Mask = XNVM_EFUSE_CFG_MARGIN_RD;
	}

	/**
	 *  - Read modify and write to EFUSE_CFG_REG
	 */
	Xil_UtilRMW32((XNVM_EFUSE_CTRL_BASEADDR +
			XNVM_EFUSE_CFG_OFFSET),
			XNVM_EFUSE_CFG_MARGIN_RD_MASK,
			Mask);

	NewRegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				      XNVM_EFUSE_CFG_OFFSET);
	if (RegVal != (NewRegVal & (~XNVM_EFUSE_CFG_MARGIN_RD_MASK))) {
		goto END;
	}

	RdModeVal = NewRegVal & XNVM_EFUSE_CFG_MARGIN_RD_MASK;
	if (RdModeVal != Mask) {
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function enabled programming mode of eFUSE controller.
 *
 *
 ******************************************************************************/
static void XNvm_EfuseEnableProgramming(void)
{
	/**
	 *  - Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_CFG_OFFSET);

	/**
	 *  - Enable eFuse program mode by writing EFUSE_CFG_REG register
	 */
	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_CFG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief	This function disables programming mode of eFUSE controller.
 *
 * @return
 * 		- XST_SUCCESS - if eFUSE programming is disabled successfully.
 * 		- XST_FAILURE - if there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseDisableProgramming(void)
{
	volatile int Status = XST_FAILURE;
	/**
	 *  - Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_CFG_OFFSET);

	/**
	 *  - Disable eFuse program mode by writing EFUSE_CFG_REG register
	 *  and return XST_FAILURE on failure
	 */
	Cfg = Cfg & ~XNVM_EFUSE_CFG_ENABLE_PGM;
	Status = Xil_SecureOut32(XNVM_EFUSE_CTRL_BASEADDR +
				 XNVM_EFUSE_CFG_OFFSET, Cfg);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function disables Margin Read mode of eFUSE controller.
 *
 * @return
 * 		- XST_SUCCESS - if resetting read mode is successful.
 * 		- XST_FAILURE - if there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseResetReadMode(void)
{
	volatile int Status = XST_FAILURE;

	/**
	 *  - Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_CFG_OFFSET);

	/**
	 *  - Reset Read mode from margin read mode by writing the EFUSE_CFG_REG
	 */
	Cfg = Cfg & ~XNVM_EFUSE_CFG_MARGIN_RD;
	Status = Xil_SecureOut32(XNVM_EFUSE_CTRL_BASEADDR +
				 XNVM_EFUSE_CFG_OFFSET, Cfg);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function initializes eFUSE controller timers.
 *
 ******************************************************************************/
static void XNvm_EfuseInitTimers(u32 ClkFreq)
{
	/* CLK_FREQ = 1/CLK_PERIOD */
	/* TPGM = ceiling(5us/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_TPGM_OFFSET,  (u32)Tpgm(ClkFreq));

	/* TRD = ceiling(150ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_TRD_OFFSET, (u32)Trd(ClkFreq));

	/* TSU_H_PS = ceiling(67ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_TSU_H_PS_OFFSET, (u32)Tsu_h_ps(ClkFreq));

	/* TSU_H_PS_CS = ceiling(46ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_TSU_H_PS_CS_OFFSET, (u32)Tsu_h_ps_cs(ClkFreq));

	/* TSU_H_CS = ceiling(30ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
		XNVM_EFUSE_TSU_H_CS_OFFSET, (u32)Tsu_h_cs(ClkFreq));
}

/******************************************************************************/
/**
 * @brief	This function sets up eFUSE controller for given operation and
 *			read mode.
 *
 * @param	Op     - Operation to be performed read/program(write).
 * @param	RdMode - Read mode for eFUSE read operation.
 * @param	EfuseClkFreq - Input Clock frequency for timers initialization.
 *
 * @return
 * 		- XST_SUCCESS - eFUSE controller setup for given op.
 * 		- XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 * 					  register access.
 * 		- XNVM_EFUSE_ERR_PGM_TBIT_PATTERN - Error in T-Bit pattern.
 * 		- XST_FAILURE - Error on set read mode failure.
 ******************************************************************************/
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
			      XNvm_EfuseRdMode RdMode, u32 EfuseClkFreq)
{
	volatile int Status = XST_FAILURE;

	/**
	 *  - Unlock eFuse controller by writing into WR_LOCK_REG
	 */
	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (XNVM_EFUSE_MODE_PGM == Op) {
		XNvm_EfuseEnableProgramming();
	}

	/**
	 *  - Set Read mode and return XST_FAILURE on failure
	 */
	Status = XNvm_EfuseSetReadMode(RdMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 *  - Initialize eFuse controller timers with validated frequency
	 *  by writing into registers(tpgm, trd, tsu_h_ps, tsu_h_ps_cs, tsu_h_cs)
	 */
	XNvm_EfuseInitTimers(EfuseClkFreq);

	/**
	 *  - Enable programming of Xilinx reserved eFuse
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00U);

	/**
	 *   - Check for T bits enabled and return XNVM_EFUSE_ERR_PGM_TBIT_PATTERN
	 *   on failure
	 */
	Status = XNvm_EfuseCheckForTBits();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function checks whether Tbits are programmed or not.
 *
 * @return
 * 		- XST_SUCCESS - On Success.
 * 		- XNVM_EFUSE_ERR_PGM_TBIT_PATTERN - Error in T-Bit pattern.
 *
 ******************************************************************************/
static int XNvm_EfuseCheckForTBits(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg = ~(XNVM_EFUSE_STATUS_TBIT_0);
	u32 TbitMask = XNVM_EFUSE_STATUS_TBIT_0;

	/**
	 *  - Read EFUSE_STATUS_REG. Return error code if Read register value not equals to Tbit mask
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_STS_OFFSET);
	if ((ReadReg & TbitMask) != TbitMask) {
		Status = (int)XNVM_EFUSE_ERR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XST_SUCCESS;
END :
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads the given register.
 *
 * @param	BaseAddress - eFuse controller base address.
 * @param	RegOffset   - Register offset from the base address.
 *
 * @return	The 32-bit value of the register.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadReg(u32 BaseAddress, u32 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
}

/******************************************************************************/
/**
 * @brief	This function writes the value into the given register.
 *
 * @param	BaseAddress - eFuse controller base address.
 * @param	RegOffset   - Register offset from the base address.
 * @param	Data        - 32-bit value to be written to the register.
 *
 ******************************************************************************/
void XNvm_EfuseWriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), Data);
}

/******************************************************************************/
/**
 * @brief	This function locks the eFUSE Controller to prevent accidental
 * 		writes to eFUSE controller registers.
 *
 * @return
 * 		- XST_SUCCESS - eFUSE controller locked.
 * 		- XNVM_EFUSE_ERR_LOCK - Failed to lock eFUSE controller
 * 					  register access.
 *
 ******************************************************************************/
int XNvm_EfuseLockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_WRITE_LOCKED;

	/**
	 *  - Write lock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_WR_LOCK_OFFSET,
			   ~XNVM_EFUSE_WR_UNLOCK_VALUE);
	/**
	*  - Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_LOCK if not success
	*/
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				       XNVM_EFUSE_WR_LOCK_OFFSET);
	if (XNVM_EFUSE_WRITE_LOCKED == LockStatus) {
		Status = XST_SUCCESS;
	} else {
		Status = (int)XNVM_EFUSE_ERR_LOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function unlocks the eFUSE Controller for writing
 *		to its registers.
 *
 * @return
 * 		- XST_SUCCESS - eFUSE controller unlocked.
 * 		- XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 * 					  register access.
 *
 ******************************************************************************/
int XNvm_EfuseUnlockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_WR_UNLOCK_VALUE;

	/**
	 *  - Write unlock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			   XNVM_EFUSE_WR_LOCK_OFFSET,
			   XNVM_EFUSE_WR_UNLOCK_VALUE);
	/**
	*  - Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_UNLOCK if not success
	*/
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				       XNVM_EFUSE_WR_LOCK_OFFSET);
	if (XNVM_EFUSE_WRITE_UNLOCKED == LockStatus) {
		Status = XST_SUCCESS;
	} else {
		Status = (int)XNVM_EFUSE_ERR_UNLOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function calculates CRC of AES key.
 *
 * @param	Key - Pointer to the little-endian formatted key for which CRC has to be calculated.
 *
 * @return	CRC of AES key.
 *
 ******************************************************************************/
u32 XNvm_AesCrcCalc(const u32 *Key)
{
	u32 Crc = 0U;
	u32 Value;
	u32 Idx;
	u32 BitNo;
	volatile u32 Temp1Crc;
	volatile u32 Temp2Crc;

	for (Idx = 0U; Idx < XNVM_AES_KEY_SIZE_IN_WORDS; Idx++) {
		/**
		 *  - Process each bit of 32-bit Value
		 */
		Value = Key[XNVM_AES_KEY_SIZE_IN_WORDS - Idx - 1U];
		for (BitNo = 0U; BitNo < 32U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			} else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}

		/**
		 *  - Get 5-bit from Address
		 */
		Value = XNVM_AES_KEY_SIZE_IN_WORDS - (u32)Idx;
		for (BitNo = 0U; BitNo < 5U; BitNo++) {
			Temp1Crc = Crc >> 1U;
			Temp2Crc = Temp1Crc ^ REVERSE_POLYNOMIAL;
			if (((Value ^ Crc) & 0x1U) != 0U) {
				Crc = Temp2Crc;
			} else {
				Crc = Temp1Crc;
			}
			Value = Value >> 1U;
		}
	}

	/**
	 *  - Return CRC value upon success
	 */
	return Crc;
}

/*****************************************************************************/
/**
 * @brief	This function zeroizes the memory and verifies that the memory
 *		has been correctly zeroized.
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data in bytes.
 *
 * @return
 * 		- XST_SUCCESS: If Zeroization is successful.
 * 		- XST_FAILURE: If Zeroization is not successful.
 ********************************************************************************/
int XNvm_ZeroizeAndVerify(u8 *DataPtr, const u32 Length)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/**
	 *  - Clear the buffer provided.
	 */
	Status = Xil_SMemSet(DataPtr, Length, 0, Length);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Read it back to verify.
	 * Return success upon successful zeroization else return XST_FAILURE.
	 */
	Status = XST_FAILURE;
	for (Index = 0U; Index < Length; Index++) {
		if (DataPtr[Index] != 0x00U) {
			goto END;
		}
	}
	if (Index == Length) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the CRC check of AES key/User0 key/User1 key
 *
 * @param	CrcRegOffSet - Register offset of respective CRC register
 * @param	CrcDoneMask - Respective CRC done mask in status register
 * @param	CrcPassMask - Respective CRC pass mask in status register
 * @param	Crc - A 32 bit CRC value of an expected AES key.
 *
 * @return
 * 		- XST_SUCCESS - On successful CRC check.
 * 		- XNVM_EFUSE_ERR_CRC_VERIFICATION - If AES boot key integrity
 * 			check is failed.
 * 		- XST_FAILURE - If AES boot key integrity check
 * 			has not finished.
 *
 * @note	Expected CRC calculation steps:
 *		1. Convert the key provided at XNVM_EFUSE_AES_KEY into hexadecimal little-endian format.
 *		2. Calculate the CRC on this little-endian formatted AES key using XNvm_AesCrcCalc() function.
 *		3. Provide the calculated CRC as the value of expected CRC.
 *
 ******************************************************************************/
int XNvm_EfuseCheckAesKeyCrc(u32 CrcRegOffSet, u32 CrcDoneMask, u32 CrcPassMask, u32 Crc)
{
	int Status = XST_FAILURE;
	int LockStatus = XST_FAILURE;
	u32 ReadReg;
	u32 IsUnlocked = (u32)FALSE;

	/**
	 * - Read the WR_LOCK_REG. Unlock the controller if read as locked
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_WR_LOCK_OFFSET);
	if (XNVM_EFUSE_WRITE_LOCKED == ReadReg) {
		Status = XNvm_EfuseUnlockController();
		if (Status != XST_SUCCESS) {
			goto END;
		}
		IsUnlocked = (u32)TRUE;
	}

	/**
	 * - Write the crc to crcregoffset of eFuse_ctrl register
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR, CrcRegOffSet, Crc);

	/**
	 *  - Wait for crcdone
	 */
	Status = (int)Xil_WaitForEvent((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR + XNVM_EFUSE_STS_OFFSET),
				       CrcDoneMask, CrcDoneMask, XNVM_POLL_TIMEOUT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Read efuse status register. If Crc is not done return XST_FAILURE
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				    XNVM_EFUSE_STS_OFFSET);

	if ((ReadReg & CrcDoneMask) != CrcDoneMask) {
		Status = XST_FAILURE;
	}
	/**
	 * - Return XNVM_EFUSE_ERR_CRC_VERIFICATION if Crc is not Pass. Return XST_SUCCESS upon crc pass and done
	 */
	else if ((ReadReg & CrcPassMask) != CrcPassMask) {
		Status = XNVM_EFUSE_ERR_CRC_VERIFICATION;
	} else {
		Status = XST_SUCCESS;
	}
END:
	/**
	 * - Lock efuse controller
	 */
	if (IsUnlocked == (u32)TRUE) {
		LockStatus = XNvm_EfuseLockController();
		if (XST_SUCCESS == Status) {
			Status = LockStatus;
		}
	}

	return Status;
}

#ifdef XNVM_ENABLE_ENV_MONITOR_CHECKS
/******************************************************************************/
/**
 * @brief	This function checks device temperature.
 *
 * @param	Temperature - Current device temperature in Celsius.
 *
 * @return
 *		- XST_SUCCESS  On temperature within thresholds.
 *		- XST_FAILURE  On temperature not within thresholds.
 *
 ******************************************************************************/
static int XNvm_EfuseTemperatureCheck(float Temperature)
{
	int Status = XST_FAILURE;

	/**
	 * - Check if temperature is within the limits of -40 to 125 degree Celsius
	 * If not in limits return XST_FAILURE else return XST_SUCCESS.
	 */
	if ((Temperature < XNVM_EFUSE_TEMP_MIN) || (Temperature > XNVM_EFUSE_TEMP_MAX)) {
		Status = XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function checks VCCAUX voltage.
 *
 * @param	Voltage - Current VCCAUX voltage in Volts.
 *
 * @return
 *		- XST_SUCCESS  On VCCAUX within thresholds.
 *		- XST_FAILURE  On VCCAUX not within thresholds.
 *
 ******************************************************************************/
static int XNvm_EfuseVccauxVoltageCheck(float Voltage)
{
	int Status = XST_FAILURE;

	/**
	 * - Check if VCCAUX voltage is within the limits.
	 * If not in limits return XST_FAILURE else XST_SUCCESS.
	 */
	if ((Voltage < XNVM_EFUSE_VCCAUX_MIN) || (Voltage > XNVM_EFUSE_VCCAUX_MAX)) {
		Status = XST_FAILURE;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function performs the temperature and voltage checks to
 * 		ensure that they are in limits before eFuse programming.
 *
 * @param	SysMonInstPtr - Pointer to XSysMon instance.
 *
 * @return
 * 		- XST_SUCCESS - On successful voltage and temperature checks.
 * 		- XNVM_EFUSE_ERR_INVALID_PARAM - Input validation failure.
 * 		- XNVM_EFUSE_ERR_READ_VOLTAGE_OUT_OF_RANGE - Voltage is out of range
 * 		- XNVM_EFUSE_ERR_READ_TEMPERATURE_OUT_OF_RANGE - Temperature is out of range
 *
 ******************************************************************************/
int XNvm_EfuseTempAndVoltChecks(XSysMon *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	u16 RawTemp;
	u16 RawVoltage;
	float Voltage;
	float Temperature;
	u8 OriginalSeqMode;

	if (SysMonInstPtr == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (SysMonInstPtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Save the original sequencer mode to restore it later
	 */
	OriginalSeqMode = XSysMon_GetSequencerMode(SysMonInstPtr);

	/**
	 * - Disable the Channel Sequencer before configuring the Sequence registers.
	 * Set to Safe Mode to allow configuration changes.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE);

	/**
	 * - Enable TEMP and VCCAUX channels in the sequencer.
	 * This ensures these channels will be converted when the sequencer runs.
	 */
	Status = XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP | XSM_SEQ_CH_VCCAUX);
	if (Status != XST_SUCCESS) {
		/* Restore original mode before returning */
		XSysMon_SetSequencerMode(SysMonInstPtr, OriginalSeqMode);
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/**
	 * - Enable the Channel Sequencer in one-pass mode to trigger fresh conversions.
	 * This will perform a single conversion cycle through the enabled channels.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_ONEPASS);

	/**
	 * - Clear the old status and wait till the End of Sequence occurs.
	 * This ensures we read fresh conversion data.
	 */
	XSysMon_GetStatus(SysMonInstPtr);
	while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) !=
			XSM_SR_EOS_MASK);

	/**
	 * - Read the current value of the on-chip temperature.
	 * Convert the raw ADC value to temperature in Celsius.
	 */
	RawTemp = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
	Temperature = XSysMon_RawToTemperature(RawTemp);

	/**
	 * - Check for temperature operating limits.
	 * Return error if temperature is not within operating limits.
	 */
	Status = XNvm_EfuseTemperatureCheck(Temperature);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_READ_TEMPERATURE_OUT_OF_RANGE;
		/* Restore original mode before returning */
		XSysMon_SetSequencerMode(SysMonInstPtr, OriginalSeqMode);
		goto END;
	}

	/**
	 * - Read the raw VCCAUX voltage value from SysMon
	 * Convert the raw ADC value to voltage in Volts.
	 */
	RawVoltage = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);
	Voltage = XSysMon_RawToVoltage(RawVoltage);

	/**
	 * - Check for voltage operating limits.
	 * Return error if voltage is not within operating limits.
	 */
	Status = XNvm_EfuseVccauxVoltageCheck(Voltage);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_READ_VOLTAGE_OUT_OF_RANGE;
	}

	/**
	 * - Restore the original sequencer mode
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, OriginalSeqMode);

END:
	return Status;
}
#endif /* XNVM_ENABLE_ENV_MONITOR_CHECKS */
