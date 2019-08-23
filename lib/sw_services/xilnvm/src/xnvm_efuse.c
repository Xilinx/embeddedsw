/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xnvm_efuse.c
*
* This file contains NVM library eFuse functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/16/2019 Initial release
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "sleep.h"
#include "xil_util.h"
#include "xnvm_efuse.h"
#include "xnvm_efuse_hw.h"

/*************************** Constant Definitions *****************************/

/***************************** Type Definitions *******************************/
/* Operation mode - Read, Program(Write) */
typedef enum {
	XNVM_EFUSE_MODE_RD,
	XNVM_EFUSE_MODE_PGM
} XNvm_EfuseOpMode;

/* eFUSE read type - Normal read, Margin read */
typedef enum {
	XNVM_EFUSE_NORMAL_RD,
	XNVM_EFUSE_MARGIN_RD
} XNvm_EfuseRdMode;

/****************** Macros (Inline Functions) Definitions *********************/

#define XNVM_ONE_MICRO_SECOND			(1U)
#define XNVM_ONE_MILLI_SECOND			(1000U)

/*************************** Function Prototypes ******************************/
static inline u32 XNvm_EfuseLockController(void);
static inline u32 XNvm_EfuseUnlockController(void);
static inline void XNvm_EfuseDisablePowerDown(void);
static inline void XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode);
static inline void XNvm_EfuseSetRefClk(void);
static inline void XNvm_EfuseEnableProgramming(void);
static inline void XNvm_EfuseDisableProgramming(void);
static inline void XNvm_EfuseInitTimers(void);
static u32 XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
					XNvm_EfuseRdMode RdMode);
static u32 XNvm_EfuseReadRow(u8 Page, u8 Row, u32* RowData);
static u32 XNvm_EfuseReadRowRange(XNvm_EfuseRdMode RdMode, u8 Page,
				u8 StartRow, u8 RowCount, u32* RowData);
static u32 XNvm_EfuseReadCache(u8 Row, u32* RowData);
static u32 XNvm_EfuseReadCacheRange(u8 StartRow, u8 RowCount, u32* RowData);
static u32 XNvm_EfusePgmBit(u8 Page, u8 Row, u8 Col);
static u32 XNvm_EfuseVerifyBit(u8 Page, u8 Row, u8 Col);
static u32 XNvm_EfusePgmAndVerifyBit(u8 Page, u8 Row, u8 Col);
static u32 XNvm_EfusePgmTBits(void);
static u32 XNvm_EfuseCacheLoad(void);
static u32 XNvm_EfuseCheckForTBits(void);
u32 XNvm_EfusePgmRows(u8 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData);
u32 XNvm_EfuseReadRows(XNvm_EfuseRdOpt ReadOption, u8 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, u32* RowData);

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit rows from specified location Cache/eFUSE.
 *
 * @param	ReadOption	Read option Cache/eFUSE
 *		StartRow	Starting Row number (0-based addressing)
 *		RowCount	Number of Rows to be read
 *		RowData		Pointer to memory location where read 32-bit
 *				row data(s) is to be stored
 *
 * @return
 *		XST_SUCCESS - 	Specified data read
 *		XNVM_EFUSE_ERROR_CACHE_PARITY -	Parity error exist in cache
 *		XNVM_EFUSE_ERROR_RD_TIMEOUT -	Timeout occured while reading
 *						the eFUSE
 *		XNVM_EFUSE_ERROR_RD -	eFUSE Read failed
 *		XST_FAILURE - 	Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
u32 XNvm_EfuseReadRows(XNvm_EfuseRdOpt ReadOption, u8 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus = XST_FAILURE;

	if ((ReadOption != XNVM_EFUSE_RD_FROM_CACHE) &&
		(ReadOption != XNVM_EFUSE_RD_FROM_EFUSE)){

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;

	}
	if ((EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfuseType != XNVM_EFUSE_PAGE_2)) {

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;

	}
	if (RowData == NULL) {

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;

	}
	if (RowCount == 0) {

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;
	}

	if (XNVM_EFUSE_RD_FROM_CACHE == ReadOption) {
		Status = XNvm_EfuseReadCacheRange(StartRow, RowCount, RowData);
	}
	else {
		Status = XNvm_EfuseReadRowRange(XNVM_EFUSE_NORMAL_RD,
				EfuseType, StartRow, RowCount, RowData);
	}
END :
	if (ReadOption != XNVM_EFUSE_RD_FROM_CACHE) {
		LockStatus = XNvm_EfuseLockController();
		if (XST_SUCCESS == Status) {
			Status = LockStatus;
		}
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets and then verifies the specified bits in the eFUSE.
 *
 * @param	StartRow	Starting Row number (0-based addressing)
 *		RowCount	Number of Rows to be read
 *		RowData		Pointer to memory location where bitmap
 *				to be written is stored.
 *				Only bit set are used for programming eFUSE.
 *
 * @return
 *		XST_SUCCESS - 	Specified bit set in eFUSE
 *		XNVM_EFUSE_ERROR_PGM_TIMEOUT - 	eFUSE programming timed out
 *		XNVM_EFUSE_ERROR_PGM - 	eFUSE programming failed
 *		XNVM_EFUSE_ERROR_PGM_VERIFY - 	Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - 	Unexpected error
 *
 * @note    None.
 *
 ******************************************************************************/
u32 XNvm_EfusePgmRows(u8 StartRow, u8 RowCount,
			XNvm_EfuseType EfuseType, const u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus = XST_FAILURE;
	u32 Data;
	u8 Row = StartRow;
	u8 Idx;

	if ((EfuseType != XNVM_EFUSE_PAGE_0) &&
		(EfuseType != XNVM_EFUSE_PAGE_1) &&
		(EfuseType != XNVM_EFUSE_PAGE_2)){

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;
	}
	if (RowData == NULL) {

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;
	}
	if (RowCount == 0) {

		Status = XNVM_EFUSE_ERROR_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_PGM,
					XNVM_EFUSE_MARGIN_RD);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseCheckForTBits();

	if (Status != XST_SUCCESS) {
		goto END;
	}

	do {
		Data = *RowData;
		Idx = 0;
		while(Data) {
			if(Data & 0x01) {
				Status = XNvm_EfusePgmAndVerifyBit(
					EfuseType, Row, Idx);
				if (Status != XST_SUCCESS) {
					break;
				}
			}
			Idx++;
			Data = Data >> 1;
		}

		RowCount--;
		Row++;
		RowData++;
	}
	while (RowCount > 0U);

	Status = XNvm_EfuseCacheLoad();
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	LockStatus = XNvm_EfuseLockController();
	if (XST_SUCCESS == Status) {
		Status = LockStatus;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function Locks the eFUSE Controller to prevent accidental writes to
 * eFUSE controller registers.
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - 	eFUSE controller locked
 *		XNVM_EFUSE_ERROR_LOCK - Failed to lock eFUSE controller
 *					register access
 *		XST_FAILURE - 	Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseLockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
			XNVM_EFUSE_WR_LOCK_REG_OFFSET,
			~XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x01 == LockStatus)	{
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_EFUSE_ERROR_LOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function Unlocks the eFUSE Controller for writing to its registers.
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - 	eFUSE controller locked
 *		XNVM_EFUSE_ERROR_UNLOCK - Failed to unlock eFUSE controller
 *						register access
 *		XST_FAILURE - 	Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static inline u32 XNvm_EfuseUnlockController(void)
{
	u32 Status = XST_FAILURE;
	u32 LockStatus;

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_WR_LOCK_REG_OFFSET,
				XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	LockStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	if(0x00 == LockStatus)	{
		Status = XST_SUCCESS;
	}
	else {
		Status = XNVM_EFUSE_ERROR_UNLOCK;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function disables power down of eFUSE macros.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisablePowerDown(void)
{
	u32 PowerDownStatus;

	PowerDownStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
						XNVM_EFUSE_PD_REG_OFFSET);
	if(XNVM_EFUSE_PD_ENABLE == PowerDownStatus) {
		/* When changing the Power Down state, wait a separation period
		 *  of 1us, before and after accessing the eFuse-Macro.
		 */
		usleep(XNVM_ONE_MICRO_SECOND);
		XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_PD_REG_OFFSET,
					~XNVM_EFUSE_PD_ENABLE);
		usleep(XNVM_ONE_MICRO_SECOND);
	}
}

/******************************************************************************/
/**
 * @brief
 * This function sets read mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode)
{
	if(XNVM_EFUSE_NORMAL_RD == RdMode) {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					XNVM_EFUSE_CFG_NORMAL_RD);
	}
	else {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_CFG_REG_OFFSET,
					(XNVM_EFUSE_CFG_ENABLE_PGM |
					XNVM_EFUSE_CFG_MARGIN_RD));
	}
}

/******************************************************************************/
/**
 * @brief
 * This function sets reference clock of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseSetRefClk(void)
{
	XNvm_Efuse_WriteReg(XNVM_CRP_BASE_ADDR,
				XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET,
				XNVM_CRP_EFUSE_REF_CLK_SELSRC);
}

/******************************************************************************/
/**
 * @brief
 * This function enabled programming mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseEnableProgramming(void)
{
	u32 Cfg = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief
 * This function disables programming mode of eFUSE controller.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseDisableProgramming(void)
{
	u32 Cfg = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg & ~XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
}

/******************************************************************************/
/**
 * @brief
 * This function initializes eFUSE controller timers.
 *
 * @param	None.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XNvm_EfuseInitTimers(void)
{
	u32 Tpgm;
	u32 Trd;
	u32 Trdm;
	u32 Tsu_h_ps;
	u32 Tsu_h_ps_cs;
	u32 Tsu_h_cs;

	/* CLK_FREQ = 1/CLK_PERIOD */
	/* TPGM = ceiling(5us/REF_CLK_PERIOD) */
	Tpgm = Xil_Ceil(5.0e-6 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TPGM_REG_OFFSET, Tpgm);

	/* TRD = ceiling(217ns/REF_CLK_PERIOD) */
	Trd = Xil_Ceil(217.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TRD_REG_OFFSET, Trd);

	/* TRDM = ceiling(500ns/REF_CLK_PERIOD)*/
	Trdm = Xil_Ceil(500.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TRDM_REG_OFFSET, Trdm);

	/* TSU_H_PS = ceiling(208ns/REF_CLK_PERIOD) */
	Tsu_h_ps = Xil_Ceil(208.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TSU_H_PS_REG_OFFSET,
				Tsu_h_ps);

	/* TSU_H_PS_CS = ceiling(143ns/REF_CLK_PERIOD) */
	Tsu_h_ps_cs = Xil_Ceil(143.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET,
				Tsu_h_ps_cs);

	/* TSU_H_CS = ceiling(184ns/REF_CLK_PERIOD) */
	Tsu_h_cs = Xil_Ceil(184.0e-9 * XNVM_PS_REF_CLK_FREQ);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_TSU_H_CS_REG_OFFSET,
				Tsu_h_cs);
}

/******************************************************************************/
/**
 * @brief
 * This function setups eFUSE controller for given operation and read mode.
 *
 * @param	Op - 	Opeartion to be performed read/program(write).
 *		RdMode - Read mode for eFUSE read operation
 *
 * @return
 *		XST_SUCCESS - eFUSE controller setup for given op
 *		XNVM_EFUSE_ERROR_UNLOCK - Failed to unlock eFUSE controller
 *					register access
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseSetupController(XNvm_EfuseOpMode Op,
			XNvm_EfuseRdMode RdMode)
{
	u32 Status = XST_FAILURE;

	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XNvm_EfuseDisablePowerDown();
	XNvm_EfuseSetRefClk();

	if (XNVM_EFUSE_MODE_PGM == Op) {
		XNvm_EfuseEnableProgramming();
	}

	XNvm_EfuseSetReadMode(RdMode);
	XNvm_EfuseInitTimers();

	/* Enable programming of Xilinx reserved EFUSE */
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00);


	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit data from eFUSE specified Row and Page.
 *
 * @param	Page - Page number
 *		Row - Row number (0-based addressing)
 *		RowData	- Pointer to memory location where 32-bit read data
 *			is to be stored
 *
 * @return
 *		XST_SUCCESS - 32-bit data is read from specified location
 *		XNVM_EFUSE_ERROR_RD_TIMEOUT - Timeout occured while reading the
 *						eFUSE
 *		XNVM_EFUSE_ERROR_RD - eFUSE Read failed
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadRow(u8 Page, u8 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 EventMask = 0U;
	u32 EfuseReadAddr;

	EfuseReadAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
			(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_RD_ADDR_REG_OFFSET,
				EfuseReadAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_BASE_ADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				(XNVM_EFUSE_ISR_RD_DONE |
				XNVM_EFUSE_ISR_RD_ERROR),
				(XNVM_EFUSE_ISR_RD_DONE |
				XNVM_EFUSE_ISR_RD_ERROR),
				XNVM_EFUSE_RD_TIMEOUT_VAL,
				&EventMask);
	if(XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERROR_RD_TIMEOUT;
	}
	else if ((EventMask & XNVM_EFUSE_ISR_RD_ERROR) != 0x0U) {
		Status = XNVM_EFUSE_ERROR_RD;
	}
	else {
		*RowData = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		Status = XST_SUCCESS;
	}
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit rows from eFUSE specified location.
 *
 * @param	RdMode	- Read mode for eFUSE read operation
 *		Page	- Page number
 *		StartRow - Starting Row number (0-based addressing)
 *		RowCount - Number of Rows to be read
 *		RowData	- Pointer to memory location where read 32-bit
 *			 row data(s) are to be stored
 *
 * @return
 *		XST_SUCCESS	- Specified data read
 *		XNVM_EFUSE_ERROR_RD_TIMEOUT - Timeout occured while reading the
 *						eFUSE
 *		XNVM_EFUSE_ERROR_RD - eFUSE Read failed
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadRowRange(XNvm_EfuseRdMode RdMode, u8 Page,
			u8 StartRow, u8 RowCount, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 Row = StartRow;

	Status = XNvm_EfuseSetupController(XNVM_EFUSE_MODE_RD, RdMode);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	do {
		Status = XNvm_EfuseReadRow(Page, Row, RowData);
		RowCount--;
		Row++;
		RowData++;
	}
	while((RowCount > 0U) && (XST_SUCCESS == Status));

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit data from cache specified by Row
 *
 * @param	Row - Starting Row number (0-based addressing)
 *		RowData	- Pointer to memory location where read 32-bit row data
 *						is to be stored
 *
 * @return
 *		XST_SUCCESS - Specified data read
 *		XNVM_EFUSE_ERROR_CACHE_PARITY - Parity error exist in cache
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadCache(u8 Row, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 CacheData;
	u32 IsrStatus;

	CacheData = Xil_In32(XNVM_EFUSE_CACHE_BASEADDR + Row * sizeof(u32));
	IsrStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if (IsrStatus & XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = XNVM_EFUSE_ERROR_CACHE_PARITY;
		goto END;
	}
	*RowData = CacheData;
	Status = XST_SUCCESS;

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function reads 32-bit rows from eFUSE specified location.
 *
 * @param	StartRow - Starting Row number (0-based addressing)
 *		RowCount - Number of Rows to be read
 *		RowData  - Pointer to memory location where read 32-bit row data(s)
 *				is to be stored
 *
 * @return
 *		XST_SUCCESS	- Specified data read
 *		XNVM_EFUSE_ERROR_CACHE_PARITY - Parity error exist in cache
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseReadCacheRange(u8 StartRow, u8 RowCount, u32* RowData)
{
	u32 Status = XST_FAILURE;
	u32 Row = StartRow;

	do {
		Status = XNvm_EfuseReadCache(Row, RowData);
		RowCount--;
		Row++;
		RowData++;
	}
	while ((RowCount > 0U) && (XST_SUCCESS == Status));

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS	- Specified bit set in eFUSE
 *		XNVM_EFUSE_ERROR_PGM_TIMEOUT - eFUSE programming timed out
 *		XNVM_EFUSE_ERROR_PGM - eFUSE programming failed
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmBit(u8 Page, u8 Row, u8 Col)
{
	u32 Status = XST_FAILURE;
	u32 PgmAddr;
	u32 EventMask;

	PgmAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
				(Row << XNVM_EFUSE_ADDR_ROW_SHIFT) |
				(Col << XNVM_EFUSE_ADDR_COLUMN_SHIFT);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_PGM_ADDR_REG_OFFSET, PgmAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_BASE_ADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				(XNVM_EFUSE_ISR_PGM_DONE |
				XNVM_EFUSE_ISR_PGM_ERROR),
				(XNVM_EFUSE_ISR_PGM_DONE |
				XNVM_EFUSE_ISR_PGM_ERROR),
				XNVM_EFUSE_PGM_TIMEOUT_VAL,
				&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERROR_PGM_TIMEOUT;
	} else if (EventMask & XNVM_EFUSE_ISR_PGM_ERROR) {
		Status = XNVM_EFUSE_ERROR_PGM;
	} else {
		Status = XST_SUCCESS;
	}

	return Status;

}

/******************************************************************************/
/**
 * @brief
 * This function verify the specified bit set in the eFUSE.
 *
 * @param	Page- Page number
 *		Row - Row number (0-based addressing)
 *		Col - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS - Specified bit set in eFUSE
 *		XNVM_EFUSE_ERROR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseVerifyBit(u8 Page, u8 Row, u8 Col)
{
	u32 RdAddr;
	u32 RegData;
	u32 EventMask = 0x00;
	u32 Status = XST_FAILURE;

	RdAddr = (Page << XNVM_EFUSE_ADDR_PAGE_SHIFT) |
				(Row << XNVM_EFUSE_ADDR_ROW_SHIFT);
	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_RD_ADDR_REG_OFFSET, RdAddr);
	Status = Xil_WaitForEvents((XNVM_EFUSE_BASE_ADDR +
				XNVM_EFUSE_ISR_REG_OFFSET),
				XNVM_EFUSE_ISR_RD_DONE,
				XNVM_EFUSE_ISR_RD_DONE,
				XNVM_EFUSE_PGM_TIMEOUT_VAL,
				&EventMask);
	if (XST_TIMEOUT == Status) {
		Status = XNVM_EFUSE_ERROR_PGM_TIMEOUT;
	} else if (EventMask & XNVM_EFUSE_ISR_RD_DONE) {
		RegData = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_RD_DATA_REG_OFFSET);
		if (RegData & (0x01U << Col)) {
			Status = XST_SUCCESS;
		}
	} else {
		Status = XNVM_EFUSE_ERROR_PGM_VERIFY;
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function sets and then verifies the specified bit in the eFUSE.
 *
 * @param	Page - Page number
 *		Row  - Row number (0-based addressing)
 *		Col  - Col number (0-based addressing)
 *
 * @return
 *		XST_SUCCESS - Specified bit set in eFUSE
 *		XNVM_EFUSE_ERROR_PGM_TIMEOUT - eFUSE programming timed out
 *		XNVM_EFUSE_ERROR_PGM - eFUSE programming failed
 *		XNVM_EFUSE_ERROR_PGM_VERIFY - Verification failed,
 *						specified bit is not set.
 *		XST_FAILURE - Unexpected error
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmAndVerifyBit(u8 Page, u8 Row, u8 Col)
{
	u32 Status = XST_FAILURE;
	Status = XNvm_EfusePgmBit(Page, Row, Col);
	if(XST_SUCCESS == Status) {
		Status = XNvm_EfuseVerifyBit(Page, Row, Col);
	}

	return Status;
}

/******************************************************************************/
/**
 * @brief
 * This function program Tbits
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - On Success
 *		XST_FAILURE - Failure in programming
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfusePgmTBits(void)
{
	u32 Status = XST_FAILURE;
	u32 TbitsPrgrmReg;
	u32 RowDataVal = 0U;
	u32 Column;

	/* Enable TBITS programming bit */
	TbitsPrgrmReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_TEST_CTRL_REG_OFFSET);

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			(TbitsPrgrmReg & (~XNVM_EFUSE_TBITS_PRGRMG_EN_MASK)));

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		 goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
			XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERROR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_1,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
				XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERROR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XNvm_EfuseReadRow(XNVM_EFUSE_PAGE_2,
				XNVM_EFUSE_TBITS_ROW,
				&RowDataVal);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}
	if (((RowDataVal >> XNVM_EFUSE_TBITS_SHIFT) &
				XNVM_EFUSE_TBITS_MASK) != 0x00U) {
		Status = (u32)XNVM_EFUSE_ERROR_PGM_TBIT_PATTERN;
		goto END;
	}

	/* Programming Tbits with pattern 1010 */
	for (Column = XNVM_EFUSE_TBITS_0_COLUMN;
		Column <= XNVM_EFUSE_TBITS_3_COLUMN; Column++) {
		if ((Column == XNVM_EFUSE_TBITS_0_COLUMN) ||
			(Column == XNVM_EFUSE_TBITS_2_COLUMN)) {
			continue;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_0,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_1,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfusePgmAndVerifyBit(XNVM_EFUSE_PAGE_2,
				XNVM_EFUSE_TBITS_ROW, Column);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
	}

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET,
			TbitsPrgrmReg);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function reloads the cache of eFUSE so that can be directly read from
 * cache.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS on successful cache reload
 *		ErrorCode on failure
 *
 * @note	Not recommended to call this API frequently,
 *		if this API is called all the cache memory is reloaded
 *		by reading eFUSE array, reading eFUSE bit multiple times may
 *		diminish the life time.
 *
 *******************************************************************************/
static u32 XNvm_EfuseCacheLoad(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 RegStatus;
	volatile u32 CacheStatus;

	RegStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET);
	/* Check the unlock status */
	if (RegStatus != 0U) {
		XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_WR_LOCK_REG_OFFSET,
					XNVM_EFUSE_WR_UNLOCK_PASSCODE);
	}

	XNvm_Efuse_WriteReg(XNVM_EFUSE_BASE_ADDR,
			XNVM_EFUSE_CACHE_LOAD_REG_OFFSET,
			XNVM_EFUSE_CACHE_LOAD_MASK);

	CacheStatus = Xil_WaitForEvent((XNVM_EFUSE_BASE_ADDR +
				XNVM_EFUSE_STATUS_REG_OFFSET),
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL);
	if (CacheStatus != XST_SUCCESS) {
		Status = (u32)XNVM_EFUSE_ERROR_CACHE_LOAD;
		goto END;
	}

	CacheStatus = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((CacheStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
			XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (u32)XNVM_EFUSE_ERROR_CACHE_LOAD;
		goto END;
	}
	Status = (u32)XST_SUCCESS;
END:
	return Status;


}

/******************************************************************************/
/**
 * @brief
 * This function checks wheather Tbits are programmed or not
 *
 * @param	None.
 *
 * @return
 *		XST_SUCCESS - On Success
 *		XST_FAILURE - On Failure
 *
 * @note	None.
 *
 ******************************************************************************/
static u32 XNvm_EfuseCheckForTBits(void)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ReadReg;
	u32 TbitMask = (XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );

	ReadReg = XNvm_Efuse_ReadReg(XNVM_EFUSE_BASE_ADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);
	if ((ReadReg & TbitMask) != TbitMask)
	{
		Status = XNvm_EfusePgmTBits();
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		Status = XNvm_EfuseCacheLoad();
		if (Status != (u32)XST_SUCCESS) {
				goto END;
		}
	} else {
		Status = (u32)XST_SUCCESS;
	}
END :
	return Status;
}
