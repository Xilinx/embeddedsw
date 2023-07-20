/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_cfi.c
*
* This file contains the code related to PDI image loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bsv   06/17/2019 Initial release
* 1.01  bsv   04/09/2020 Code clean up of xilloader
* 1.02  kc    08/03/2020 Added status prints for CFU/CFI errors
*       kal   08/12/2020 Added PlHouseCleaning in case of any PL error.
*       td    08/19/2020 Fixed MISRA C violations Rule 10.3
*       bsv   10/13/2020 Code clean up
* 1.03  kal   12/22/2020 Perform Cfu/Cfi recovery before re-triggering
*                        PL House cleaning in case of error in PL loading.
* 1.04  skd   03/25/2021 Compilation warning fix
* 1.05  td    07/08/2021 Fix doxygen warnings
*       td    07/15/2021 Fix doxygen warnings
*       bsv   07/18/2021 Debug enhancements
*       bsv   08/31/2021 Code clean up
*       ma    09/01/2021 Fix issue in clearing the CFI and CFU errors
* 1.06  am    11/24/2021 Fixed doxygen warning
*       bsv   12/04/2021 Addressed security review comment
* 1.07  ma    01/17/2022 Enable SLVERR for CFU_APB registers
* 1.08  ma    07/27/2022 Added support for CFrame data clear check which is
*                        required during PL secure lockdown
* 1.09  ng    11/11/2022 Updated doxygen comments
*       ng    03/12/2023 Fixed Coverity warnings
*       sk    03/14/2023 Added Glitch detect for status check in
*                        XLoader_CframeDataClearCheck
*       dd    03/28/2023 Updated doxygen comments
*       ng    03/30/2023 Updated algorithm and return values in doxygen comments
* 1.10  ng    06/26/2023 Added support for system device tree flow
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xpm_pldomain.h"
#include "xplmi_hw.h"
#include "xloader.h"
#include "xcframe.h"
#include "xplmi_util.h"
#include "xplmi_config.h"

/************************** Constant Definitions *****************************/
/* CFRAM related register defines */
#define CFRAME_BCAST_REG_TESTMODE_OFFSET		(0x120U) /**< Test mode offset */
#define CFRAME_BCAST_REG_TESTMODE_CRAM_SELF_CHECK_MASK	(0x100U) /**< CRAM self check mask */

#define CFRAME_BCAST_REG_FAR_OFFSET			(0x10U) /**< CFrame address register offset */
#define CFRAME_BCAST_REG_FAR_BLOCKTYPE_SHIFT		(20U) /**< CFrame  address register block type shift */
#define CFRAME_MAX_BLOCK_TYPE_COUNT			(0x3U) /**< CFrame maximum block type count */

#define XPLMI_ZERO	(0x0U) /**< Zero */
#define CFRAME_CRC_POLL_TIMEOUT				(0xFFFFU) /**< CRC poll time out*/

#define XLOADER_CFRAME_DATACLEAR_CHECK_CMD_ID	(0xCU) /**< Data clear check command Id */
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef PLM_DEBUG_MODE
#define PMC_GLOBAL_PMC_ERR2_STATUS_CFI_SHIFT	(16U) /**< CFI Non-Correctable
                                                       * Error shift */

/************************** Function Prototypes ******************************/
static void XLoader_CfiErrHandler(const XCfupmc *InstancePtr);
static void XLoader_CfuErrHandler(const XCfupmc *InstancePtr);
#endif

/************************** Variable Definitions *****************************/
static XCframe XLoader_CframeIns = {0U}; /**< CFRAME Driver Instance */

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the Cframe driver.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_ERR_CFRAME_LOOKUP if CFrame lookup fails.
 * 			- XLOADER_ERR_CFRAME_CFG if CFrame configuration fails.
 *
 ******************************************************************************/
int XLoader_CframeInit(void)
{
	int Status = XST_FAILURE;
	XCframe_Config *Config;

	if (XLoader_CframeIns.IsReady != (u8)FALSE) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table, and
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig(XCFRAME_DEVICE);
	if (NULL == Config) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_CFRAME_LOOKUP, 0);
		goto END;
	}

	Status = XCframe_CfgInitialize(&XLoader_CframeIns, Config,
				Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_CFRAME_CFG, Status);
	}

	/** - Enable SLVERR for CFU. */
	XPlmi_UtilRMW(CFU_APB_CFU_CTL, CFU_APB_CFU_CTL_SLVERR_EN_MASK,
			CFU_APB_CFU_CTL_SLVERR_EN_MASK);

END:
	return Status;
}

#ifndef PLM_DEBUG_MODE
/*****************************************************************************/
/**
 * @brief	This function is used for Cframe error handling
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XLoader_CfiErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_CfiErrHandler(InstancePtr);
	XCframe_ClearCframeErr(&XLoader_CframeIns);
	XCfupmc_ClearIgnoreCfiErr(InstancePtr);

	/** Clear ISRs */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK);
	XCfupmc_ClearCfuIsr(InstancePtr);

	return;
}

/*****************************************************************************/
/**
 * @brief	This function is used for CFU error handling
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XLoader_CfuErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	/** CFU error checking and handling */
	XCfupmc_CfuErrHandler(InstancePtr);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
			PMC_GLOBAL_PMC_ERR1_STATUS_CFU_MASK);
	return;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function is used to check the CFU ISR and PMC_ERR1 and PMC_ERR2
 * 			status registers to check for any errors in PL and call
 * 			corresponding error	recovery functions if needed.
 *
 * @param	ImageId is Id of the image present in PDI
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XLoader_CframeErrorHandler(u32 ImageId)
{
	u32 Err1Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
	u32 Err2Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	u32 CfuIsrStatus = XPlmi_In32(CFU_APB_CFU_ISR);
	u32 CfuStatus = XPlmi_In32(CFU_APB_CFU_STATUS);
#ifndef PLM_DEBUG_MODE
	XCfupmc XLoader_CfuIns; /** CFU Driver Instance */
	u8 CfiErrStatus;
	u32 CountVal = XPlmi_In32(CFU_APB_CFU_QWORD_CNT);
	u8 PlHouseClean = (u8)(((CountVal != 0U) &&
		(ImageId == PM_DEV_PLD_0)) ? TRUE : FALSE);
#else
	(void)ImageId;
#endif

	XPlmi_Printf(DEBUG_GENERAL, "Error loading PL data: \n\r"
		"CFU_ISR: 0x%08x, CFU_STATUS: 0x%08x \n\r"
		"PMC ERR1: 0x%08x, PMC ERR2: 0x%08x\n\r",
		CfuIsrStatus, CfuStatus, Err1Status, Err2Status);

#ifndef PLM_DEBUG_MODE
	CfiErrStatus = (u8)(Err1Status & PMC_GLOBAL_PMC_ERR1_STATUS_CFRAME_MASK);
	if (CfiErrStatus == 0U) {
		CfiErrStatus = (u8)((Err2Status & PMC_GLOBAL_PMC_ERR2_STATUS_CFI_MASK)
			>> PMC_GLOBAL_PMC_ERR2_STATUS_CFI_SHIFT);
	}

	/**
	 * - Execute CfiErrorHandler.
	 */
	if ((CfiErrStatus != 0U) || (PlHouseClean == (u8)TRUE)) {
		XLoader_CfiErrHandler(&XLoader_CfuIns);
	}

	/**
	 * - Execute CFU Error Handler.
	 */
	XLoader_CfuErrHandler(&XLoader_CfuIns);

	/**
	 * - Retrigger PL house clean if it's a PLD0 image.
	 */
	if (PlHouseClean == (u8)TRUE) {
		(void)XPmPlDomain_RetriggerPlHouseClean();
	}
#endif
}

/*****************************************************************************/
/**
 * @brief	This function is used to check if the Cframe data is cleared
 * 			or not using CRC method. This function need to be called after
 * 			CFI house cleaning is done.
 *
 * @param	Cmd is pointer to the command. Command payload parameters are:
 * 			Block Type
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XLOADER_INVALID_BLOCKTYPE if block type passed is invalid. Only
 * 			Type 0, Type 1, and Type 2 are supported.
 * 			- XLOADER_CFI_CFRAME_IS_BUSY if CRAM self check fails as CFI
 * 			CFrame is busy.
 * 			- XLOADER_CFRAME_CRC_CHECK_FAILED if CFRAME CRC check fails.
 *
 *****************************************************************************/
int XLoader_CframeDataClearCheck(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 BlockType = Cmd->Payload[0U];
	u32 RowRange;
	u32 MaxRowRange = XPlmi_In32(CFU_APB_CFU_ROW_RANGE) & CFU_APB_CFU_ROW_RANGE_NUM_MASK;
	Xuint128 CframeData = {0x0U};

	XPLMI_EXPORT_CMD(XLOADER_CFRAME_DATACLEAR_CHECK_CMD_ID, XPLMI_MODULE_LOADER_ID,
			XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	/** - Check if the block type is valid */
	if (BlockType >= CFRAME_MAX_BLOCK_TYPE_COUNT) {
		Status = (int)XLOADER_INVALID_BLOCKTYPE;
		goto END;
	}

	/** - Initialize Cframe driver */
	Status = XLoader_CframeInit();
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	/** - Enable CRAM Self Check */
	CframeData.Word0 = CFRAME_BCAST_REG_TESTMODE_CRAM_SELF_CHECK_MASK;
	XCframe_WriteReg(&XLoader_CframeIns, CFRAME_BCAST_REG_TESTMODE_OFFSET,
			XCFRAME_FRAME_BCAST, &CframeData);

	/** - Reset CRC */
	XCframe_WriteCmd(&XLoader_CframeIns, XCFRAME_FRAME_BCAST, XCFRAME_CMD_REG_RCRC);

	/** - Set Frame Address Register with the Block type */
	CframeData.Word0 = BlockType << CFRAME_BCAST_REG_FAR_BLOCKTYPE_SHIFT;
	XCframe_WriteReg(&XLoader_CframeIns, CFRAME_BCAST_REG_FAR_OFFSET,
			XCFRAME_FRAME_BCAST, &CframeData);

	/** - Send RDALL command */
	XCframe_WriteCmd(&XLoader_CframeIns, XCFRAME_FRAME_BCAST, XCFRAME_CMD_REG_RDALL);

	/** - Check if Cframe is busy */
	Status = XPlmi_UtilPoll(CFU_APB_CFU_STATUS, CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK,
			XPLMI_ZERO, CFRAME_CRC_POLL_TIMEOUT, NULL);
	if (Status != XST_SUCCESS) {
		Status = (int)XLOADER_CFI_CFRAME_IS_BUSY;
		goto END;
	}

	for (RowRange = 0; RowRange < MaxRowRange; RowRange++) {
		XCframe_ReadReg(&XLoader_CframeIns, XCFRAME_CRC_OFFSET, (XCframe_FrameNo)RowRange, (u32 *)&CframeData);
		/** - Check CRC */
		if (CframeData.Word0 != XPLMI_ZERO) {
			Status = (int)XLOADER_CFRAME_CRC_CHECK_FAILED;
			goto END;
		}
	}

	/** - Clear CRAM Self Check */
	CframeData.Word0 = XPLMI_ZERO;
	XCframe_WriteReg(&XLoader_CframeIns, CFRAME_BCAST_REG_TESTMODE_OFFSET,
			XCFRAME_FRAME_BCAST, &CframeData);

END:
	return Status;
}
