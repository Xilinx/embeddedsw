/*******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file common/server/xnvm_efuse_common.c
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
* 3.1   skg  10/25/2022 Added in body comments for APIs
*       kal  03/07/2023 Added volatile keyword for Status variables
* 3.2   kum  04/11/2023 Moved Env monitor API's from versal xnvm_efuse.c to make use for both versal and versalnet
*	vss  09/19/2023	Fixed MISRA-C Rule 8.7 violation
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

/**< Sysmon VCCPMC Amux ctrl value */
#define XNVM_EFUSE_SYSMON_VCCPMC_AMUX_CTRL	(0x0bU)
/**< Sysmon VCCPMC Abus sw1 value */
#define XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW1	(0x00U)
/**< Sysmon VCCPMC Abus sw0 value*/
#define XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW0	(0x02U)
/**< Sysmon VCCPMC mode value */
#define XNVM_EFUSE_SYSMON_VCCPMC_MODE		(0x00U)
/**< Sysmon number of measurement registers */
#define XNVM_EFUSE_SYSMON_NUM_MEASURE_REGS	(0x20U)
/**< Sysmon number of supplies per flag */
#define XNVM_EFUSE_SYSMON_NUM_SUPPLIES_PER_FLAG	(32U)
/**< Sysmon Psv timeout value */
#define XNVM_EFUSE_SYSMONPSV_TIMEOUT		(100000U)
/**< Fraction multiplier value */
#define XNVM_EFUSE_FRACTION_MUL_VALUE		(1000000U)
/**< eFuse word length */
#define XNVM_EFUSE_WORD_LEN			(4U)
/** @} */

/***************************** Type Definitions *******************************/


/****************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
static int XNvm_EfusePmcVoltageCheck(float Voltage);
static int XNvm_EfuseTemparatureCheck(float Temparature);

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
	volatile int Status = XST_FAILURE;
	u32 CacheStatus;

    /**
	 * @{ Write 1 to load bit of eFuse_CACHE_LOAD register.
     *	  Wait for CACHE_DONE bit to set in EFUSE_STATUS register . If timed out return timout error.
     *	  Return XST_SUCCESS
     */
	XNvm_EfuseWriteReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CACHE_LOAD_REG_OFFSET,
			XNVM_EFUSE_CACHE_LOAD_MASK);

	CacheStatus = Xil_WaitForEvent((UINTPTR)(XNVM_EFUSE_CTRL_BASEADDR +
				XNVM_EFUSE_STATUS_REG_OFFSET),
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_STATUS_CACHE_DONE,
				XNVM_EFUSE_CACHE_LOAD_TIMEOUT_VAL);
	if (CacheStatus != (u32)XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

    /**
	 *  @{ Read EFUSE_ISR_REG. If EFUSE_ISR_CHACE_ERROR set return cache load error.
	 *     Return XST_SUCCES.
	 */
	CacheStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_ISR_REG_OFFSET);
	if ((CacheStatus & XNVM_EFUSE_ISR_CACHE_ERROR) ==
			XNVM_EFUSE_ISR_CACHE_ERROR) {
		Status = (int)XNVM_EFUSE_ERR_CACHE_LOAD;
		goto END;
	}

	Status = XST_SUCCESS;
END:
    /**
	 *  Reset EFUSE_ISR_CACHE_ERROR bit to 1
	 */
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

    /**
	 *  Read EFUSE_PD_REG. If enable disable by writing EFUSE_PD_REG to 0
	 */
	PowerDownStatus = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
						XNVM_EFUSE_PD_REG_OFFSET);
	if(XNVM_EFUSE_PD_ENABLE == PowerDownStatus) {
		/**
		 * @{ When changing the Power Down state, wait a separation period
		 *    of 1us, before and after accessing the eFuse-Macro.
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

    /**
	 *  Read EFUSE_CFG_REG register
	 */
	RegVal = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
			XNVM_EFUSE_CFG_REG_OFFSET);
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
	/**
	 *  Set Reference clock for efuse by writing to EFUSE_REF_CLK_REG
	 */
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
	/**
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

    /**
	 *  Enable eFuse program mode by writing EFUSE_CFG_REG register
	 */
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
	volatile int Status = XST_FAILURE;
	/**
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	/**
	 *  disable eFuse program mode by writing EFUSE_CFG_REG register
	 */
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
	volatile int Status = XST_FAILURE;

	/**
	 *  Read EFUSE_CFG_REG
	 */
	u32 Cfg = XNvm_EfuseReadReg(XNVM_EFUSE_CTRL_BASEADDR,
					XNVM_EFUSE_CFG_REG_OFFSET);

	/**
	 *  Reset Read mode from margin read mode by writing the EFUSE_CFG_REG
	 */
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
	volatile int Status = XST_FAILURE;

    /**
	 *  Unlock eFuse controller to write into eFuse registers
	 */
	Status = XNvm_EfuseUnlockController();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 *  Disable power down mode and set refernce clock to eFuse
	 */
	XNvm_EfuseDisablePowerDown();
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
	 *   Intialize eFuse Timers
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
int XNvm_EfuseCheckForTBits(void)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg = ~(XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );
	u32 TbitMask = (XNVM_EFUSE_STATUS_TBIT_0 |
			XNVM_EFUSE_STATUS_TBIT_1 |
			XNVM_EFUSE_STATUS_TBIT_2 );

    /**
	 *  Read EFUSE_STATUS_REG. Return error code if Read register value not equals to Tbit mask
	 */
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

/******************************************************************************/
/**
 * @brief	This function checks Device Temparature for
 * 		LP,MP and HP devices based on the Efuse value.
 *
 * @return	- XST_SUCCESS - On Temparature within thresholds.
 *		- XST_FAILURE - On Temparature not within thresholds.
 *
 ******************************************************************************/
static int XNvm_EfuseTemparatureCheck(float Temparature)
{
	int Status = XST_FAILURE;
	u32 ReadReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 EfuseTempMax = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 EfuseTempMin = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	float TempMin;
	float TempMax;

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);

	EfuseTempMax = (ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT;
	EfuseTempMin = (ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT;

	switch (EfuseTempMax) {
		case XNVM_EFUSE_FULL_RANGE_CHECK:
			TempMax = XNVM_EFUSE_FULL_RANGE_TEMP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_LP_RANGE_CHECK:
			TempMax = XNVM_EFUSE_TEMP_LP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_MP_RANGE_CHECK:
			TempMax = XNVM_EFUSE_TEMP_MP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_HP_RANGE_CHECK:
			TempMax = XNVM_EFUSE_TEMP_HP_MAX;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	if (Status == XST_FAILURE) {
		goto END;
	}

	/* Junction temparature operating range for
	 * eFUSE programming as per TSMC data sheet
	 */

	if (EfuseTempMin == XNVM_EFUSE_LP_RANGE_CHECK) {
		TempMin = XNVM_EFUSE_TEMP_LP_MIN;
	}
	else {
		TempMin = XNVM_EFUSE_TEMP_MP_MIN;
	}

	if ((Temparature < TempMin) || (Temparature > TempMax)) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function checks VCC_PMC voltage checks for
 * 		LP,MP and HP devices based on the Efuse value.
 *
 * @return	- XST_SUCCESS - On VCC_PMC within thresholds.
 *		- XST_FAILURE - On VCC_PMC not within thresholds.
 *
 ******************************************************************************/
static int XNvm_EfusePmcVoltageCheck(float Voltage)
{
	int Status = XST_FAILURE;
	u32 ReadReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 EfuseVoltVal = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	float VoltMin;
	float VoltMax;

	ReadReg = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR,
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET);

	EfuseVoltVal = (ReadReg &
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK) >>
			XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT;

	switch (EfuseVoltVal) {
		case XNVM_EFUSE_FULL_RANGE_CHECK:
			VoltMin = XNVM_EFUSE_VCC_PMC_LP_MIN;
			VoltMax = XNVM_EFUSE_VCC_PMC_HP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_LP_RANGE_CHECK:
			VoltMin = XNVM_EFUSE_VCC_PMC_LP_MIN;
			VoltMax = XNVM_EFUSE_VCC_PMC_LP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_MP_RANGE_CHECK:
			VoltMin = XNVM_EFUSE_VCC_PMC_MP_MIN;
			VoltMax = XNVM_EFUSE_VCC_PMC_MP_MAX;
			Status = XST_SUCCESS;
			break;
		case XNVM_EFUSE_HP_RANGE_CHECK:
			VoltMin = XNVM_EFUSE_VCC_PMC_HP_MIN;
			VoltMax = XNVM_EFUSE_VCC_PMC_HP_MAX;
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	if (Status == XST_FAILURE) {
		goto END;
	}

	if ((Voltage < VoltMin) || (Voltage > VoltMax)) {
		Status = XST_FAILURE;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * @brief	This function reads root register ID where measurement is
 * 		stored in Root SysMon.
 *
 * @param	SysmonpsvSatBaseAddr - Pointer to SysMon base address
 *
 * @return	On Success reads and returns SupplyReg Id
 *		On Failure returns default SupplyReg value
 *
 ******************************************************************************/
u32 XNvm_GetSysmonSupplyRegId(UINTPTR SysmonpsvSatBaseAddr)
{
	UINTPTR BaseAddr = SysmonpsvSatBaseAddr;
	u32 ReadReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 SupplyReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 Index;
	u32 AbusSw0;
	u32 AbusSw1;
	u32 AmuxCtrl;
	u32 Mode;

	for (Index = 0U; Index < XNVM_EFUSE_SYSMON_NUM_MEASURE_REGS; Index++) {
		ReadReg = XSysMonPsv_ReadReg((UINTPTR)(BaseAddr +
				XNVM_EFUSE_SYSMONPSV_SAT_MEASURE0_OFFSET +
				(XNVM_EFUSE_WORD_LEN * Index)));
		AbusSw0 = (ReadReg &
				XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_MASK) >>
				XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW0_SHIFT;
		AbusSw1 = (ReadReg &
				XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_MASK) >>
				XNVM_EFUSE_SYSMON_SAT_CONFIG_ABUS_SW1_SHIFT;
		AmuxCtrl = (ReadReg &
				XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_MASK) >>
				XNVM_EFUSE_SYSMON_SAT_CONFIG_AMUX_CTRL_SHIFT;
		Mode = (ReadReg &
				XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_MASK) >>
				XNVM_EFUSE_SYSMON_SAT_CONFIG_MODE_SHIFT;

		if ((XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW1 == AbusSw1) &&
			(XNVM_EFUSE_SYSMON_VCCPMC_ABUS_SW0 == AbusSw0) &&
			(XNVM_EFUSE_SYSMON_VCCPMC_AMUX_CTRL == AmuxCtrl) &&
			(XNVM_EFUSE_SYSMON_VCCPMC_MODE == Mode)) {
			SupplyReg = (ReadReg &
					XNVM_EFUSE_SYSMON_SAT_ADDR_ID_MASK) >>
					XNVM_EFUSE_SYSMON_SAT_ADDR_ID_SHIFT;
			break;
		}
	}

	return SupplyReg;
}

/******************************************************************************/
/**
 * @brief	This function performs the Temparature and Voltage checks to
 * 		ensure that they are in limits before eFuse programming.
 *
 * @param	SysMonInstPtr - Pointer to SysMon instance.
 *
 * @return	- XST_SUCCESS - On successful Voltage and Temparature checks.
 *		- XNVM_EFUSE_ERROR_READ_VOLTAGE_OUT_OF_RANGE - Voltage is
 *								out of range
 *		- XNVM_EFUSE_ERROR_READ_TMEPERATURE_OUT_OF_RANGE - Temparature
 *								is out of range
 *
 ******************************************************************************/
int XNvm_EfuseTempAndVoltChecks(const XSysMonPsv *SysMonInstPtr)
{
	int Status = XST_FAILURE;
	u32 ReadReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 SupplyReg = XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET;
	u32 SysmonEventsMask = 0U;
	u32 RawTemp;
	u32 RawVoltage;
	int FractionalPart;
	int IntegralPart;
	float Voltage;
	float Temparature;
	u32 Offset;
	u32 Shift;
	char Signchar = ' ';

	if (SysMonInstPtr == NULL) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	if (SysMonInstPtr->IsReady != XIL_COMPONENT_IS_READY) {
		Status = (int)XNVM_EFUSE_ERR_INVALID_PARAM;
		goto END;
	}

	/* Unlock the sysmon register space */
	XSysMonPsv_WriteReg(
			SysMonInstPtr->Config.BaseAddress + XSYSMONPSV_PCSR_LOCK,
			XNVM_EFUSE_SYSMON_LOCK_CODE);

	SupplyReg = XNvm_GetSysmonSupplyRegId(
		(UINTPTR)XNVM_EFUSE_SYSMONPSV_SAT0_BASEADDR);
	if (SupplyReg == XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET) {
		SupplyReg = XNvm_GetSysmonSupplyRegId(
			(UINTPTR)XNVM_EFUSE_SYSMONPSV_SAT1_BASEADDR);
	}

	if (SupplyReg == XNVM_EFUSE_SEC_DEF_VAL_ALL_BIT_SET) {
		Status = (int)XNVM_EFUSE_ERROR_NO_SUPPLIES_ENABLED;
		goto END;
	}
	Offset = XNVM_EFUSE_WORD_LEN *
		(SupplyReg / XNVM_EFUSE_SYSMON_NUM_SUPPLIES_PER_FLAG);
	Shift = SupplyReg % XNVM_EFUSE_SYSMON_NUM_SUPPLIES_PER_FLAG;
	SysmonEventsMask = (u32)1U << Shift;

	Status = (int)Xil_WaitForEvents((SysMonInstPtr->Config.BaseAddress +
		Offset + XSYSMONPSV_NEW_DATA_FLAG0), SysmonEventsMask,
		SysmonEventsMask, XNVM_EFUSE_SYSMONPSV_TIMEOUT, &ReadReg);
	if(Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERROR_SYSMON_NO_NEW_DATA;
		goto END;
	}
	else {
		/* Clear the New data flag if its set */
		XSysMonPsv_WriteReg(SysMonInstPtr->Config.BaseAddress +
			Offset + XSYSMONPSV_NEW_DATA_FLAG0, ReadReg);
	}

	if (ReadReg != 0x00U) {
		RawVoltage = XSysMonPsv_ReadReg(
				SysMonInstPtr->Config.BaseAddress +
				XSYSMONPSV_SUPPLY +
				(SupplyReg * XNVM_EFUSE_WORD_LEN));

		Voltage = XSysMonPsv_RawToVoltage(RawVoltage);
		IntegralPart = (int)Voltage;
		FractionalPart = (Voltage - IntegralPart) *
					XNVM_EFUSE_FRACTION_MUL_VALUE;
		XNvm_Printf(XNVM_DEBUG_GENERAL,"Voltage = %d.%06dV \r\n",
				IntegralPart, FractionalPart);
		Status = XNvm_EfusePmcVoltageCheck(Voltage);
		if (Status != XST_SUCCESS) {
			Status = (int)XNVM_EFUSE_ERROR_READ_VOLTAGE_OUT_OF_RANGE;
			goto END;
		}
	}

	RawTemp = XSysMonPsv_ReadReg(SysMonInstPtr->Config.BaseAddress +
			XSYSMONPSV_TEMP_SAT);
	Temparature = XSysMonPsv_FixedToFloat(RawTemp);

	if (Temparature < 0.0f) {
		Signchar = '-';
	}

	IntegralPart = (int)Temparature;
	FractionalPart = (Temparature - IntegralPart) *
				XNVM_EFUSE_FRACTION_MUL_VALUE;

	/* Convert IntegralPart and FractionalPart to absolute values */
	IntegralPart = (IntegralPart < 0) ? -IntegralPart : IntegralPart;
	FractionalPart = (FractionalPart < 0) ? -FractionalPart : FractionalPart;

	XNvm_Printf(XNVM_DEBUG_GENERAL,
		"Device temperature on the chip = %c%d.%06dC \r\n",
		Signchar, IntegralPart,FractionalPart);

	Status = XNvm_EfuseTemparatureCheck(Temparature);
	if (Status != XST_SUCCESS) {
		Status = (int)XNVM_EFUSE_ERROR_READ_TMEPERATURE_OUT_OF_RANGE;
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
