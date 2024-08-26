/*******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "xnvm_utils.h"
#include "xstatus.h"
#include "xil_util.h"

/*************************** Constant Definitions *****************************/

/**< Polynomial used for CRC calculation */
#define REVERSE_POLYNOMIAL			(0x82F63B78U)
#define XNVM_EFUSE_CLK_CTRL_ADDR	(0x040A003CU)
#define XNVM_EFUSE_IO_CTRL_ADDR		(0x040A00E8U)
#define XNVM_EFUSE_EMC_CLK_EN_VAL	(1 << 1U)
#define XNVM_EFUSE_CLK_SRC_SEL_VAL	(1 << 0U)
#define XNVM_EFUSE_TPRGM_VALUE \
	(((5.0f) * (XNVM_EFUSE_PS_REF_CLK_FREQ)) / (1000000.0f))
#define XNVM_EFUSE_TRD_VALUE	\
 (((15.0f) * (XNVM_EFUSE_PS_REF_CLK_FREQ)) / (100000000.0f))
#define XNVM_EFUSE_TSUHPS_VALUE \
 (((67.0f) * (XNVM_EFUSE_PS_REF_CLK_FREQ)) / (1000000000.0f))
#define XNVM_EFUSE_TSUHPSCS_VALUE \
 (((46.0f) * (XNVM_EFUSE_PS_REF_CLK_FREQ)) / (1000000000.0f))
#define XNVM_EFUSE_TSUHCS_VALUE \
 (((30.0f) * (XNVM_EFUSE_PS_REF_CLK_FREQ)) / (1000000000.0f))

/* Timer related macros */
#define Tpgm() \
	Xil_Ceil(XNVM_EFUSE_TPRGM_VALUE)
#define Trd() \
	Xil_Ceil(XNVM_EFUSE_TRD_VALUE)
#define Tsu_h_ps() \
	Xil_Ceil(XNVM_EFUSE_TSUHPS_VALUE)
#define Tsu_h_ps_cs() \
	Xil_Ceil(XNVM_EFUSE_TSUHPSCS_VALUE)
#define Tsu_h_cs() \
	Xil_Ceil(XNVM_EFUSE_TSUHCS_VALUE)

/***************************** Function Definitions *******************************/

static int XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
static void XNvm_EfuseSetRefClk(void);
static int XNvm_EfuseCheckForTBits(void);
static void XNvm_EfuseEnableProgramming(void);
static void XNvm_EfuseInitTimers(void);

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 * @brief	This function sets read mode of eFUSE controller.
 *
 * @param	RdMode - Mode to be used for eFUSE read.
 *
 * @return	XST_SUCCESS - if Setting read mode is successful.
 *		XST_FAILURE - if there is a failure
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
	 *  Read EFUSE_CFG_REG register
	 */
	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CFG_OFFSET);
	if(XNVM_EFUSE_NORMAL_RD == RdMode) {
		Mask = XNVM_EFUSE_CFG_NORMAL_RD;
	}
	else {
		Mask = XNVM_EFUSE_CFG_MARGIN_RD;
	}

    /**
	 *  Read modify and write to EFUSE_CFG_REG
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
 * @brief	This function sets reference clock of eFUSE controller.
 *
 ******************************************************************************/
static void XNvm_EfuseSetRefClk(void)
{
	/**
	 *  Set Reference clock for efuse
	 */
	Xil_UtilRMW32(XNVM_EFUSE_IO_CTRL_ADDR, XNVM_EFUSE_EMC_CLK_EN_VAL, XNVM_EFUSE_EMC_CLK_EN_VAL);
	Xil_UtilRMW32(XNVM_EFUSE_CLK_CTRL_ADDR, XNVM_EFUSE_CLK_SRC_SEL_VAL, XNVM_EFUSE_CLK_SRC_SEL_VAL);
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
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_OFFSET);

    /**
	 *  Enable eFuse program mode by writing EFUSE_CFG_REG register
	 */
	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief	This function disables programming mode of eFUSE controller.
 *
 * @return      XST_SUCCESS - if eFUSE programming is disabled successfully.
 *              XST_FAILURE - if there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseDisableProgramming(void)
{
	volatile int Status = XST_FAILURE;
	/**
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_OFFSET);

	/**
	 *  disable eFuse program mode by writing EFUSE_CFG_REG register
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
 * @return      XST_SUCCESS - if resetting read mode is successful.
 *              XST_FAILURE - if there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseResetReadMode(void)
{
	volatile int Status = XST_FAILURE;

	/**
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_OFFSET);

	/**
	 *  Reset Read mode from margin read mode by writing the EFUSE_CFG_REG
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
static void XNvm_EfuseInitTimers(void)
{
	/* CLK_FREQ = 1/CLK_PERIOD */
	/* TPGM = ceiling(5us/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TPGM_OFFSET, Tpgm());

	/* TRD = ceiling(150ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRD_OFFSET, Trd());

	/* TSU_H_PS = ceiling(67ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_OFFSET,
				Tsu_h_ps());

	/* TSU_H_PS_CS = ceiling(46ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_CS_OFFSET,
				Tsu_h_ps_cs());

	/* TSU_H_CS = ceiling(30ns/REF_CLK_PERIOD) */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_CS_OFFSET,
				Tsu_h_cs());
}

/******************************************************************************/
/**
 * @brief	This function setups eFUSE controller for given operation and
 *			read mode.
 *
 * @param	Op     - Operation to be performed read/program(write).
 * @param	RdMode - Read mode for eFUSE read operation.
 *
 * @return	- XST_SUCCESS - eFUSE controller setup for given op.
 *		- XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *						register access.
 *
 ******************************************************************************/
int XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
			XNvm_EfuseRdMode RdMode)
{
	volatile int Status = XST_FAILURE;

    /**
	 *  Unlock eFuse controller to write into eFuse registers
	 */
	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XNvm_EfuseSetRefClk();

	if (XNVM_EFUSE_MODE_PGM == Op) {
		XNvm_EfuseEnableProgramming();
	}

    /**
	 *  Set Read mode
	 */
	Status = XNvm_EfuseSetReadMode(RdMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

    /**
	 *   Initialize eFuse Timers
	 */
	XNvm_EfuseInitTimers();

	/**
     *	Enable programming of Xilinx reserved eFuse
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00U);

    /**
	 *   Check for T bits enabled
	 */
	Status = XNvm_EfuseCheckForTBits();

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function checks whether Tbits are programmed or not.
 *
 * @return	- XST_SUCCESS - On Success.
 *		- XNVM_EFUSE_ERR_PGM_TBIT_PATTERN - Error in T-Bit pattern.
 *
 ******************************************************************************/
static int XNvm_EfuseCheckForTBits(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg = ~(XNVM_EFUSE_STATUS_TBIT_0);
	u32 TbitMask = XNVM_EFUSE_STATUS_TBIT_0;

    /**
	 *  Read EFUSE_STATUS_REG. Return error code if Read register value not equals to Tbit mask
	 */
	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STS_OFFSET);
	if ((ReadReg & TbitMask) != TbitMask)
	{
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
 * @param	BaseAddress is the eFuse controller base address.
 * @param	RegOffset is the register offset from the base address.
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
 * @param	BaseAddress is the eFuse controller base address.
 * @param	RegOffset is the register offset from the base address.
 * @param	Data is the 32-bit value to be written to the register.
 *
 * @return	None
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
 * @return	- XST_SUCCESS - eFUSE controller locked.
 *		- XNVM_EFUSE_ERR_LOCK - Failed to lock eFUSE controller
 *					                register access.
 *
 ******************************************************************************/
int XNvm_EfuseLockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_WRITE_LOCKED;

    /**
	 *  Write lock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_WR_LOCK_OFFSET,
			~XNVM_EFUSE_WR_UNLOCK_VALUE);
	/**
     *  Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_LOCK if not success
     */
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_OFFSET);
	if(XNVM_EFUSE_WRITE_LOCKED == LockStatus) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_LOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function unlocks the eFUSE Controller for writing
 *		to its registers.
 *
 * @return	XST_SUCCESS - eFUSE controller locked.
 *		XNVM_EFUSE_ERR_UNLOCK - Failed to unlock eFUSE controller
 *							register access.
 *
 ******************************************************************************/
int XNvm_EfuseUnlockController(void)
{
	int Status = XST_FAILURE;
	volatile u32 LockStatus = ~XNVM_EFUSE_WR_UNLOCK_VALUE;

     /**
	 *  Write unlock Passcode in efuse control at offset of WR_LOCK_REG
	 */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_WR_LOCK_OFFSET,
				XNVM_EFUSE_WR_UNLOCK_VALUE);
	/**
     *  Read the WR_LOCK_REG if above write was successful. Return XNVM_EFUSE_ERR_UNLOCK if not success
     */
	LockStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_WR_LOCK_OFFSET);
	if(XNVM_EFUSE_WRITE_UNLOCKED == LockStatus) {
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XNVM_EFUSE_ERR_UNLOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function calculates CRC of AES key.
 *
 * @param	Key - Pointer to the key for which CRC has to be calculated.
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
         *	Process each bits of 32-bit Value
		 */
		Value = Key[XNVM_AES_KEY_SIZE_IN_WORDS - Idx - 1U];
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

		/**
         *	Get 5-bit from Address
		 */
		Value = XNVM_AES_KEY_SIZE_IN_WORDS - (u32)Idx;
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

    /**
	 *  Return CRC value upon success
	 */
	return Crc;
}

/*****************************************************************************/
/**
 * @brief	This function is used to zeroize the memory
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data in bytes.
 *
 * @return
 *		- XST_SUCCESS: If Zeroization is successful.
 *		- XST_FAILURE: If Zeroization is not successful.
 ********************************************************************************/
int XNvm_ZeroizeAndVerify(u8 *DataPtr, const u32 Length)
{
	volatile int Status = XST_FAILURE;
	volatile u32 Index;

	/**
     *	Clear the decrypted data
	 */
	Status = Xil_SMemSet(DataPtr, Length, 0, Length);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
     *	Read it back to verify
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
