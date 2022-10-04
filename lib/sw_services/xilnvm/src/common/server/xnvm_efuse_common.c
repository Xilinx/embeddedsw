/*******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xnvm_efuse.c
*
* This file contains eFuse functions of xilnvm library
* and provides the access to program eFUSE
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 3.0   kal  07/16/2022 Initial release
*       dc   08/29/2022 Changed u8 to u32 type
*       kal  09/29/2022 Removed unlock and lock of eFuse controller
*                       from the XNvm_EfuseCacheReload function
*
* </pre>
*
* @note
*
*******************************************************************************/

/***************************** Include Files **********************************/
#include "sleep.h"
#include "xil_util.h"
#include "xil_io.h"
#include "xnvm_efuse_common.h"
#include "xnvm_efuse_common_hw.h"
#include "xnvm_utils.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/
/**
 * @name eFuse macro definitions
 */
/**< One Micro Second Timeout */
#define XNVM_ONE_MICRO_SECOND			(1U)
/**< TPGM Divisor value */
#define XNVM_EFUSE_TPGM_DIV			(200000UL)
/**< TRD Divisor value */
#define XNVM_EFUSE_TRD_DIV			(4608294UL)
/**< TRMD Divisor value */
#define XNVM_EFUSE_TRMD_DIV			(2000000UL)
/**< TSU_H_PS Divisor value */
#define XNVM_EFUSE_TSU_H_PS_DIV			(4807692UL)
/**< TSU_H_PS_CS Divisor value */
#define XNVM_EFUSE_TSU_H_PS_CS_DIV		(6993007UL)
/**< TSU_H_CS Divisor value */
#define XNVM_EFUSE_TSU_H_CS_DIV			(5434783UL)
/**< Default secure value for 32 bit */
#define XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET	(0xFFFFFFFFU)
/**< Default secure value for 8 bit */
#define XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET		(0xFFU)
/** @} */

/***************************** Type Definitions *******************************/


/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/

/*************************** Variable Definitions *****************************/

/*************************** Function Definitions *****************************/

/******************************************************************************/
/**
 * @brief	This function reloads the cache of eFUSE so that can be directly
 * 			read from cache.
 *
 * @return	- XST_SUCCESS - on successful cache reload.
 *		- XNVM_EFUSE_ERR_CACHE_LOAD - Error while loading the cache.
 *
 * @note	Not recommended to call this API frequently,if this API is called
 *		all the cache memory is reloaded by reading eFUSE array,
 *		reading eFUSE bit multiple times may diminish the life time.
 *
 ******************************************************************************/
int XNvm_EfuseCacheReload(void)
{
	int Status = XST_FAILURE;
	u32 CacheStatus;

	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CACHE_LOAD_REG_OFFSET,
			XNVM_EFUSE_CACHE_LOAD_MASK);

	CacheStatus = Xil_WaitForEvent((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_STATUS_REG_OFFSET),
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL);
	if (CacheStatus != (u32)XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	CacheStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((CacheStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
			XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	Status = XST_SUCCESS;
END:
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_ISR_REG_OFFSET,
			XNVM_EFUSE_ISR_CACHE_ERROR);
	return Status;

}

/******************************************************************************/
/**
 * @brief	This function disables power down of eFUSE macros.
 *
 ******************************************************************************/
void XNvm_EfuseDisablePowerDown(void)
{
	volatile u32 PowerDownStatus = ~XNVM_EFUSE_PD_ENABLE;

	PowerDownStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
						XNVM_EFUSE_PD_REG_OFFSET);
	if(XNVM_EFUSE_PD_ENABLE == PowerDownStatus) {
		/*
		 * When changing the Power Down state, wait a separation period
		 * of 1us, before and after accessing the eFuse-Macro.
		 */
		usleep(XNVM_ONE_MICRO_SECOND);
		XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_PD_REG_OFFSET,
					~XNVM_EFUSE_PD_ENABLE);
		usleep(XNVM_ONE_MICRO_SECOND);
	}
}

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
int XNvm_EfuseSetReadMode(XNvm_EfuseRdMode RdMode)
{
	int Status = XST_FAILURE;
	u32 RegVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 NewRegVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 RdModeVal = XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET;
	u32 Mask = XNVM_EFUSE_SEC_DEF_VAL_BYTE_SET;

	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CFG_REG_OFFSET);
	if(XNVM_EFUSE_NORMAL_RD == RdMode) {
		Mask = XNVM_EFUSE_CFG_NORMAL_RD;
	}
	else {
		Mask = XNVM_EFUSE_CFG_MARGIN_RD;
	}

	Xil_UtilRMW32((XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_CFG_REG_OFFSET),
				XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK,
				Mask);

	NewRegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CFG_REG_OFFSET);
	if (RegVal != (NewRegVal & (~XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK))) {
		goto END;
	}

	RdModeVal = NewRegVal & XNVM_EFUSE_CTRL_CFG_MARGIN_RD_MASK;
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
void XNvm_EfuseSetRefClk(void)
{
	XNvm_EfuseWriteReg(XNVM_CRP_BASE_ADDR,
				XNVM_CRP_EFUSE_REF_CLK_REG_OFFSET,
				XNVM_CRP_EFUSE_REF_CLK_SELSRC);
}

/******************************************************************************/
/**
 * @brief	This function enabled programming mode of eFUSE controller.
 *
 *
 ******************************************************************************/
void XNvm_EfuseEnableProgramming(void)
{
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg | XNVM_EFUSE_CFG_ENABLE_PGM;
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);
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
	int Status = XST_FAILURE;
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg & ~XNVM_EFUSE_CFG_ENABLE_PGM;
	Status = Xil_SecureOut32(XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);

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
	int Status = XST_FAILURE;
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	Cfg = Cfg & ~XNVM_EFUSE_CFG_MARGIN_RD;
	Status = Xil_SecureOut32(XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_CFG_REG_OFFSET, Cfg);

	return Status;
}

/******************************************************************************/
/**
 * @brief	This function initializes eFUSE controller timers.
 *
 ******************************************************************************/
void XNvm_EfuseInitTimers(void)
{
	u32 Tpgm;
	u32 Trd;
	u32 Trdm;
	u32 Tsu_h_ps;
	u32 Tsu_h_ps_cs;
	u32 Tsu_h_cs;

	/* CLK_FREQ = 1/CLK_PERIOD */
	/* TPGM = ceiling(5us/REF_CLK_PERIOD) */
	Tpgm = (u32)((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TPGM_DIV - 1U) /
							XNVM_EFUSE_TPGM_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TPGM_REG_OFFSET, Tpgm);

	/* TRD = ceiling(217ns/REF_CLK_PERIOD) */
	Trd = (u32)((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TRD_DIV - 1U) /
							XNVM_EFUSE_TRD_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRD_REG_OFFSET, Trd);

	/* TRDM = ceiling(500ns/REF_CLK_PERIOD)*/
	Trdm = (u32)((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TRMD_DIV - 1U) /
							XNVM_EFUSE_TRMD_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TRDM_REG_OFFSET, Trdm);

	/* TSU_H_PS = ceiling(208ns/REF_CLK_PERIOD) */
	Tsu_h_ps = ((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TSU_H_PS_DIV - 1U) /
							XNVM_EFUSE_TSU_H_PS_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_REG_OFFSET,
				Tsu_h_ps);

	/* TSU_H_PS_CS = ceiling(143ns/REF_CLK_PERIOD) */
	Tsu_h_ps_cs = ((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TSU_H_PS_CS_DIV - 1U) /
							XNVM_EFUSE_TSU_H_PS_CS_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_PS_CS_REG_OFFSET,
				Tsu_h_ps_cs);

	/* TSU_H_CS = ceiling(184ns/REF_CLK_PERIOD) */
	Tsu_h_cs = (u32)((XNVM_PS_REF_CLK_FREQ + XNVM_EFUSE_TSU_H_CS_DIV - 1U) /
							XNVM_EFUSE_TSU_H_CS_DIV);
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_TSU_H_CS_REG_OFFSET,
				Tsu_h_cs);
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
	int Status = XST_FAILURE;

	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XNvm_EfuseDisablePowerDown();
	XNvm_EfuseSetRefClk();

	if (XNVM_EFUSE_MODE_PGM == Op) {
		XNvm_EfuseEnableProgramming();
	}

	Status = XNvm_EfuseSetReadMode(RdMode);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XNvm_EfuseInitTimers();

	/* Enable programming of Xilinx reserved EFUSE */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_TEST_CTRL_REG_OFFSET, 0x00U);

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
int XNvm_EfuseCheckForTBits(void)
{
	int Status = XST_FAILURE;
	volatile u32 ReadReg = ~(XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );
	u32 TbitMask = (XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
				XNVM_EFUSE_STATUS_REG_OFFSET);
	if ((ReadReg & TbitMask) != TbitMask)
	{
		Status = (int)XNVM_EFUSE_ERR_PGM_TBIT_PATTERN;
		goto END;
	}

	Status = XST_SUCCESS;
END :
	return Status;
}
