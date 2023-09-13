/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_tamper.c
*
* This file contains the tamper response processing routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ma   07/08/2022 Initial release
*       ma   07/19/2022 Disable interrupts before calling secure lockdown proc
*                       and continue secure lockdown irrespective of the status
*       kpt  07/21/2022 Added XPlmi_IfHaltBootTriggerSLD
*       ma   07/25/2022 Enhancements to secure lockdown code
* 1.01  ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Create Secure Lockdown as a Critical Priority Task
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
* 1.02  skd  04/10/2023 Fix third party review comments
* 1.03  sk   07/18/2023 Added NULL check in RegisterTamperIntrHandler
*       sk   08/17/2023 Updated XPlmi_EmSetAction arguments
*       dd   09/12/2023 MISRA-C violation Rule 17.7 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_tamper.h"
#include "xplmi_err_common.h"
#include "xplmi_status.h"
#include "xplmi_hw.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PMC_ANALOG_GD_CTRL_REG		(0xF1160000U)
					/**< PMC_ANALOG base address */
#define PMC_ANALOG_GD0_RST_STATUS_REG_MASK	(0x200U)
					/**< Glitch detector0 status mask */
#define PMC_ANALOG_GD1_RST_STATUS_REG_MASK	(0x2000000U)
					/**< Glitch detector1 status mask */
#define PMC_ANALOG_GD_STATUS 		(PMC_ANALOG_GD0_RST_STATUS_REG_MASK | \
					 PMC_ANALOG_GD1_RST_STATUS_REG_MASK)
					/**< Glitch detector status mask */
#define CRP_CFU_REF_CTRL				(0xF1260108U)
					/**< CRP CFU_REF_CTRL register address */
#define CRP_CFU_REF_CTRL_DIVISOR_MASK	(0x3FF00U)
					/**< CRP CFU_REF_CTRL Divisor mask */
#define CFU_REF_CTRL_DIVISOR_INCREASE	(0x2U)
					/**< CRP CFU_REF_CTRL Divisor increase */

/************************** Function Prototypes ******************************/
static void XPlmi_PmcApbErrorHandler(const u32 ErrorNodeId,
		const u32 ErrorMask);
static int XPlmi_ProcessTamperResponse(void *Data);

/************************** Variable Definitions *****************************/
static u32 SldState = XPLMI_SLD_NOT_TRIGGERED;
static u32 TamperResponse;
static XPlmi_TaskNode *TamperTask = NULL;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function returns Secure Lockdown State
 *
 * @return	Secure Lockdown State
 *
 *****************************************************************************/
u32 XPlmi_SldState(void)
{
	return SldState;
}

/*****************************************************************************/
/**
 * @brief	This functions Triggers Tamper Response processing immediately
 *		or as a task.
 *
 * @param	Response is the Tamper Response that has to occur
 * @param	Flag denotes whether processing has to occur immediately or as
 *		a task. The possible values are: XPLMI_TRIGGER_TAMPER_TASK,
 *		XPLMI_TRIGGER_TAMPER_IMMEDIATE.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_TriggerTamperResponse(u32 Response, u32 Flag)
{
	/** If Tamper Task is already triggered, Do nothing */
	if (XPlmi_SldState() != XPLMI_SLD_NOT_TRIGGERED) {
		goto END;
	}
	TamperResponse = Response;
	SldState = XPLMI_SLD_TRIGGERED;

	/** For versal SSIT devices, Notify slave SLRs about SLD */
	if ((TamperResponse & XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK) != 0x0U) {
		XPlmi_NotifySldSlaveSlrs();
	}

	if (Flag == XPLMI_TRIGGER_TAMPER_TASK) {
		if (TamperTask != NULL) {
			/** Trigger tamper response task */
			XPlmi_TaskTriggerNow(TamperTask);
		}
		else {
			XPlmi_Printf(DEBUG_GENERAL, "Tamper Trigger Task not created yet\n\r");
		}
	}
	else {
		/** Process tamper response immediately */
		(void)XPlmi_ProcessTamperResponse(NULL);
	}
END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function processes the tamper response
 *
 * @param	Data is unused
 *
 * @return	None
 *
 *****************************************************************************/
static int XPlmi_ProcessTamperResponse(void *Data)
{
	int Status = XST_FAILURE;
	u32 CfuDivisor;
	u32 CfuRefCtrl;

	(void)Data;

	if ((TamperResponse & XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK) != 0x0U) {
		/**
		 * Set SldState to XPLMI_SLD_IN_PROGRESS
		 */
		SldState = XPLMI_SLD_IN_PROGRESS;
		/*
		 * For versal SSIT devices, Handshake on SSIT ERR lines has to be
		 * performed before performing secure lockdown.
		 */
		XPlmi_InterSlrSldHandshake();

		/**
		 * Reset LpdInitialized variable
		 */
		XPlmi_UnSetLpdInitialized(UART_INITIALIZED);
		/**
		 * Disable interrupts to Microblaze
		 */
		microblaze_disable_interrupts();
		/**
		 * Disable PMC EAM interrupts
		 */
		(void)XPlmi_EmDisablePmcErrors(XPLMI_PMC_PSM_ERR1_REG_OFFSET,
			MASK32_ALL_HIGH);
		(void)XPlmi_EmDisablePmcErrors(XPLMI_PMC_PSM_ERR2_REG_OFFSET,
			MASK32_ALL_HIGH);
		/**
		 * Reduce PL frequency by half
		 */
		CfuRefCtrl = XPlmi_In32(CRP_CFU_REF_CTRL);
		CfuDivisor = (CfuRefCtrl & CRP_CFU_REF_CTRL_DIVISOR_MASK) *
				CFU_REF_CTRL_DIVISOR_INCREASE;
		CfuRefCtrl = (CfuRefCtrl & ~CRP_CFU_REF_CTRL_DIVISOR_MASK) | CfuDivisor;
		XPlmi_Out32(CRP_CFU_REF_CTRL, CfuRefCtrl);
		/**
		 * Execute secure lockdown proc
		 */

		Status = XPlmi_ExecuteProc(XPLMI_SLD_PROC_ID);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "Secure Lockdown failed with 0x%x "
					"error\r\n", Status);
		}
	}

	/**
	 * Configure TAMPER_RESP_0 with the received response
	 */
	Xil_Out32(PMC_GLOBAL_TAMPER_RESP_0, TamperResponse);

	/**
	 * Trigger software tamper event to ROM to execute lockdown
	 * for PMC
	 */
	Xil_Out32(PMC_GLOBAL_TAMPER_TRIG, PMC_GLOBAL_TAMPER_TRIG_VAL);

	/**
	 * Wait forever; ROM to complete secure lockdown
	 */
	while(1U) {
		;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt
 *
 * @return
 *	-	XST_SUCCESS - On success
 *	-	XPLMI_INVALID_ERROR_ID - On invalid ID
 *	-	XPLMI_INVALID_ERROR_HANDLER - On Null handler
 *
 *****************************************************************************/
int XPlmi_RegisterTamperIntrHandler(void)
{
	int Status = XST_FAILURE;

	/* Check if the task is already created */
	TamperTask = XPlmi_GetTaskInstance(XPlmi_ProcessTamperResponse, NULL,
				XPLMI_INVALID_INTR_ID);
	if (TamperTask == NULL) {
		/* Create task if it is not already created */
		TamperTask = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_CRITICAL,
				XPlmi_ProcessTamperResponse, NULL);
		if (TamperTask == NULL) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
			goto END;
		}
		TamperTask->IntrId = XPLMI_INVALID_INTR_ID;
	}

	/**
	 * Register handler
	 */
	Status = XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
			XIL_EVENT_ERROR_MASK_PMCAPB, XPLMI_EM_ACTION_CUSTOM,
			XPlmi_PmcApbErrorHandler, XPLMI_INVALID_SUBSYSTEM_ID);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Enable tamper interrupt in PMC GLOBAL
	 */
	Xil_Out32(PMC_GLOBAL_IER, PMC_GLOBAL_TAMPER_INT_MASK);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is the handler for tamper interrupt
 *
 * @param	ErrorNodeId - Node Identifier
 * @param	ErrorMask   - Mask Identifier
 *
 * @return  None
 *
 *****************************************************************************/
static void XPlmi_PmcApbErrorHandler(const u32 ErrorNodeId,
		const u32 ErrorMask)
{
	volatile u32 TamperResp = XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK;
	volatile u32 TamperRespTmp = XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK;
	volatile u32 IsrVal = PMC_GLOBAL_TAMPER_INT_MASK;
	volatile u32 IsrValTmp = PMC_GLOBAL_TAMPER_INT_MASK;

	(void)ErrorNodeId;
	(void)ErrorMask;

	/**
	 * Check the reason for interrupt
	 */
	IsrVal = Xil_In32(PMC_GLOBAL_ISR);
	IsrValTmp = Xil_In32(PMC_GLOBAL_ISR);
	if (((IsrVal & PMC_GLOBAL_TAMPER_INT_MASK) ==
			PMC_GLOBAL_TAMPER_INT_MASK) ||
		((IsrValTmp & PMC_GLOBAL_TAMPER_INT_MASK) ==
				PMC_GLOBAL_TAMPER_INT_MASK)) {
		TamperResp = Xil_In32(XPLMI_RTCFG_TAMPER_RESP);
		TamperRespTmp = Xil_In32(XPLMI_RTCFG_TAMPER_RESP);
		if (((TamperResp & XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK) != 0x0U) ||
			((TamperRespTmp & XPLMI_RTCFG_TAMPER_RESP_SLD_0_1_MASK) != 0x0U)) {
			XSECURE_REDUNDANT_IMPL(XPlmi_TriggerTamperResponse, TamperResp, XPLMI_TRIGGER_TAMPER_TASK);
		} else {
			XPlmi_Printf(DEBUG_GENERAL, "Warning: Invalid Tamper Response. "
					"Configured Tamper Response at RTCA: 0x%x\r\n"
					"For SYSTEM_INERRUPT response, user need to register for "
					"notification of PMC_APB error\r\n",
					TamperResp);
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL, "Received PMC_APB interrupt is other than "
				"tamper interrupt. PMC_GLOBAL_ISR: 0x%x\r\n", IsrVal);
	}

	/**
	 * Clear the interrupt source
	 */
	Xil_UtilRMW32(PMC_ANALOG_GD_CTRL_REG, PMC_ANALOG_GD_STATUS,
			PMC_ANALOG_GD_STATUS);
	Xil_Out32(PMC_GLOBAL_ISR, PMC_GLOBAL_TAMPER_INT_MASK);
	Xil_UtilRMW32(PMC_ANALOG_GD_CTRL_REG, PMC_ANALOG_GD_STATUS, 0U);

	/**
	 * Once the interrupt is received, PLM disables the interrupt and
	 * calls the handler. So it is necessary to re-enable the interrupt.
	 */
	(void)XPlmi_EmSetAction(XIL_NODETYPE_EVENT_ERROR_PMC_ERR2,
			XIL_EVENT_ERROR_MASK_PMCAPB, XPLMI_EM_ACTION_CUSTOM,
			XPlmi_PmcApbErrorHandler, XPLMI_INVALID_SUBSYSTEM_ID);
}

/******************************************************************************/
/**
 * @brief	This function triggers secure lockdown if haltboot efuses are programmed.
 *
 * @return	None
 *
 ******************************************************************************/
void XPlmi_TriggerSLDOnHaltBoot(u32 Flag)
{
	u32 HaltBoot = XPlmi_In32(EFUSE_CACHE_MISC_CTRL) &
			EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK;
	u32 HaltBootTmp = XPlmi_In32(EFUSE_CACHE_MISC_CTRL) &
			EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK;

	if ((HaltBoot != 0U) || (HaltBootTmp != 0U)) {
		XSECURE_REDUNDANT_IMPL(XPlmi_TriggerTamperResponse, XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK, Flag);
	}
}
