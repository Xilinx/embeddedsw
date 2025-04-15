/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_intr.c
*
* This file contains the code related to the interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   03/25/2019 Initial release
* 1.01  kc   04/09/2019 Added support for PCIe secondary boot mode and
*						partial PDI load
*       kc   09/13/2019 SBI reset is removed for SMAP boot mode to ensure smap
*						bus width value remains unchanged
* 1.02  kc   02/17/2020 Added APIs to add services to task queues in interrupt
*						context
*       kc   06/03/2020 Moved PLM GIC interrupt enablement to GIC handlers
*       bsv  04/09/2020 Code clean up of Xilloader
*       bsv  08/12/2020 Remove misleading comments
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       bm   10/14/2020 Code clean up
*       ana  10/19/2020 Added doxygen comments
* 1.03  bsv  07/19/2021 Disable UART prints when invalid header is encountered
*                       in slave boot modes
*       bsv  08/02/2021 Updated function return type as part of code clean up
*       bsv  09/05/2021 Disable prints in slave boot modes in case of error
* 1.06  am   11/24/2021 Fixed doxygen warnings
* 1.07  ma   05/10/2022 Enable SSIT interrupts for Slave SLRs
*       bm   07/06/2022 Refactor versal and versal_net code
*       bm   07/18/2022 Shutdown modules gracefully during update
*       bm   07/24/2022 Set PlmLiveStatus during boot time
* 1.08  ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Switch to SSIT Events as soon as basic Noc path is
*                       configured
*       sk   02/22/2023 Added EoPDI SYNC logic to handle Slave PDI load errors
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*       sk   05/18/2023 Deprecate copy to memory feature,removed SubsystemPdiIns
*       sk   08/18/2023 Fixed security review comments
*       dd	 09/11/2023 MISRA-C violation Rule 17.7 fixed
*       sk   03/13/24 Fixed doxygen comments format
*       pre  03/02/2025 Added task based event notification functionality for partial PDI
*       nb   04/09/2025 Add CPM PCIE isolation removal which has to come after
*                       PDI load is complete
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_hw.h"
#include "xloader.h"
#include "xplmi_proc.h"
#include "xplmi.h"
#include "xplmi_err.h"
#include "xloader_plat.h"
#include "xplmi_plat.h"
#include "xplmi_wdt.h"
#include "xloader_secure.h"
#include "xsecure_resourcehandling.h"
#include "xpm_domain_iso.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *****************************************/
#define XLOADER_SBI_DELAY_IN_MICROSEC		(5000U) /**< Flag indicates SBI
                                                         * delay in micro second
							 */
#define XLOADER_LOG_LEVEL_MASK		(0xF0U) /**< Flag indicates Log level
                                                 * mask */
#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET))
#define XLOADER_PDILOAD_DEFAULT     (0x0U) /**< State to represent default sbi
                                    pdi load which is triggered from interrupt*/
#define XLOADER_PDILOAD_TRIGGERED     (0x1U) /**< State to represent pdi load
                                      which is triggered from queued event */
#define XLOADER_PPDI_IPI_CMDID        (0x701) /**< Cmd Id for partial PDI load from IPI */
#define XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE	(0x8U) /**< SBI PCIE PDI load */

/************************** Function Prototypes **************************************************/
/**
 * @{
 * @cond xloader_internal
 */
static int XLoader_TriggerPartialPdiEvent(void);

/************************** Variable Definitions *****************************/
static XSecure_PartialPdiEventParams PpdiEventVars = {.PartialPdiEventSts = XSECURE_EVENT_CLEAR,
	.TriggerPartialPdiEvent = XLoader_TriggerPartialPdiEvent,
};
u32 XSecure_PdiLoadState = XLOADER_PDILOAD_DEFAULT;
#endif

static int XLoader_SbiLoadPdi(void *Data);

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	This function initializes the loader instance and registers loader
 * 			commands with PLM.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure.
 *
 *****************************************************************************/
int XLoader_IntrInit(void)
{
	int Status = XST_FAILURE;

	/**
	 * - Register the SBI RDY interrupt to enable the PDI loading from
	 * SBI interface.
	 */
	Status = XPlmi_GicRegisterHandler(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX,
		XLoader_SbiLoadPdi, (void *)0U);

	if (XPlmi_IsPlmUpdateDone() == (u8)TRUE) {
		XPlmi_GicIntrEnable(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
	}

	return Status;
}

/**
 * @}
 * @endcond
 */
/*****************************************************************************/
/**
 * @brief	This function is the interrupt handler for SBI data ready.
 * 			In this handler, PDI is loadeed through SBI interface.
 * 			SBI interface setting for JTAG/SMAP/AXI/HSDP should be set before
 * 			this handler.
 *
 * @param	Data Not used as of now, present as a part of general interrupt
 *			handler definition.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure.
 *
 * @note
 *		SBI interface setting for JTAG/SMAP/AXI/HSDP should be set before
 *		this handler.
 *****************************************************************************/
static int XLoader_SbiLoadPdi(void *Data)
{
	int Status = XST_FAILURE;
	PdiSrc_t PdiSrc;
	u64 PdiAddr;
	u32 RegVal;
#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET))
    u32 Response[XPLMI_CMD_RESP_SIZE] = {0U};
#endif
	XilPdi* PdiPtr = XLoader_GetPdiInstance();
	(void)Data;

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/**
	 * - Disable the SBI RDY interrupt so that PDI load does not
	 * interrupt itself.
	 */
	XPlmi_GicIntrDisable(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);

	/** In-Place Update is applicable only for versal_net */
	if (XPlmi_IsPlmUpdateInProgress() == (u8)TRUE) {
		XPlmi_Printf(DEBUG_GENERAL, "ERROR: Update in Progress\n\r");
		Status = XST_SUCCESS;
		goto END1;
	}

#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET))
	if (XSecure_PdiLoadState == XLOADER_PDILOAD_TRIGGERED) {
		PdiPtr->IpiMask = PpdiEventVars.IpiMask;
		PdiSrc = PpdiEventVars.PdiSrc;
		PdiAddr = PpdiEventVars.PdiAddr;
		XSecure_PdiLoadState = XLOADER_PDILOAD_DEFAULT;
	}
	else
#endif
	{
		RegVal = XPlmi_In32(SLAVE_BOOT_SBI_CTRL) &
			SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK;
		if (RegVal == 0U) {
			PdiSrc = XLOADER_PDI_SRC_SMAP;
		}
		else {
			PdiSrc = XLOADER_PDI_SRC_SBI;
		}

		PdiAddr = 0U;
		PdiPtr->IpiMask = 0U;

		XPlmi_Printf(DEBUG_GENERAL, "SBI PDI Load: Started\n\r");
	}

	PdiPtr->PdiType = XLOADER_PDI_TYPE_PARTIAL;

	if (PdiPtr->DiscardUartLogs == (u8)TRUE) {
		DebugLog->LogLevel &= XLOADER_LOG_LEVEL_MASK;
	}


	/**
	 * - Queue partial PDI if any of SHA and AES resources is busy
	*/
	Status = XLoader_PpdiEventHandling(PdiSrc, PdiAddr, PdiPtr->IpiMask);
	if (Status != XST_SUCCESS) {
		if (Status == (int)XPLMI_CMD_IN_PROGRESS) {
			Status = XST_SUCCESS;
		}
		goto END1;
	}

	/**
	 * - Load partial PDI.
	*/
	Status = XLoader_LoadPdi(PdiPtr, PdiSrc, PdiAddr);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Remove remaining PCIE isolations if it is PCIE load.
	 */
	if (XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE ==
	    (XPlmi_In32(SLAVE_BOOT_SBI_CTRL) & XLOADER_SBI_CTRL_INTERFACE_AXI_SLAVE)) {
		XPmDomainIso_CpmPcieIsoRemoval();
	}

	if (PdiPtr->IpiMask == 0U ) {
		XPlmi_Printf(DEBUG_GENERAL, "SBI PDI Load: Done\n\r");
	}
	else {
		XPlmi_Printf(DEBUG_GENERAL, "Subsystem PDI Load: Done\n\r");
	}

END:
	if (Status != XST_SUCCESS) {
		/* Update the error code */
		XPlmi_ErrMgr(Status);
		PdiPtr->DiscardUartLogs = (u8)TRUE;
		DebugLog->LogLevel |= (DebugLog->LogLevel >> XPLMI_LOG_LEVEL_SHIFT);
		XPlmi_SetPlmLiveStatus();
		usleep(XLOADER_SBI_DELAY_IN_MICROSEC);
	}

#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET))
	if (PdiPtr->IpiMask != 0U) {
		Response[XLOADER_RESP_CMD_LOAD_PDI_STATUS_INDEX] = (u32)Status;
		if (Status != XST_SUCCESS) {
			Status = XPlmi_UpdateStatus((XPlmiStatus_t)XPlmi_GetCdoErr(XLOADER_PPDI_IPI_CMDID), Status);
		}
		Response[XLOADER_RESP_CMD_EXEC_STATUS_INDEX] = (u32)Status;

		/**
		 * Send response to caller and ack IPI
		 */
		XPlmi_SendResponseandAck(PdiPtr->IpiMask, &Response[0]);
	}
#endif

	/**
	 * - Clear SBI RDY interrupt.
	*/
	XLoader_ClearIntrSbiDataRdy();
	(void)Xloader_SsitEoPdiSync(PdiPtr);
END1:
	return Status;
}

#if (defined(PLM_ENABLE_SHA_AES_EVENTS_QUEUING) || defined(VERSAL_NET))
/*****************************************************************************/
/**
 * @brief	This function handles the partial PDI event based on status of
 *          resources
 *
 * @param	PdiPtr is the instance pointer that points to PDI details
 * @param	PdiSrc is source of PDI.
 * @param	PdiAddr is the address at which PDI is located in the PDI source
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure.
*
 *****************************************************************************/
int XLoader_PpdiEventHandling(PdiSrc_t PdiSrc, u64 PdiAddr, u32 IpiMask)
{
	int Status = XST_FAILURE;
	XSecure_ResourceSts ResourceSts = XSECURE_RES_BUSY;

	/**
	 * - Get status of resources
	*/
	Status = XSecure_GetShaAndAesSts(&ResourceSts);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Notify partial PDI event when SHA or AES is busy
	*/
	if (ResourceSts == XSECURE_RES_BUSY) {
		PpdiEventVars.PartialPdiEventSts = XSECURE_EVENT_PENDING;
		PpdiEventVars.IpiMask = IpiMask;
		PpdiEventVars.PdiSrc = PdiSrc;
		PpdiEventVars.PdiAddr = PdiAddr;
		Status = XPLMI_CMD_IN_PROGRESS;
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function triggers the partial PDI event
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure.
 *****************************************************************************/
static int XLoader_TriggerPartialPdiEvent(void)
{
	int Status = XST_FAILURE;
	XPlmi_TaskNode *PpdiEvent = NULL;

	/** Return success if partial PDI event is not pending */
	if (PpdiEventVars.PartialPdiEventSts != XSECURE_EVENT_PENDING) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** Get task for partial PDI */
	PpdiEvent = XPlmi_GetTaskInstance(XLoader_SbiLoadPdi, (void *)0U, XPLMI_INVALID_INTR_ID);
	if (PpdiEvent == NULL) {
		XPlmi_Printf(DEBUG_INFO, "Task get instance failed \n\r");
		goto END;
	}

	/** Clear partial PDI event */
	PpdiEventVars.PartialPdiEventSts = XSECURE_EVENT_CLEAR;

	/** Change state of Pdi Load */
	XSecure_PdiLoadState = XLOADER_PDILOAD_TRIGGERED;

	/**
	 * Disable interrupts and enable after triggering task to avoid concurrent access
	 * since task queue is being accessed in interrupts
	 */
	microblaze_disable_interrupts();
	XPlmi_TaskTriggerNow(PpdiEvent);
	microblaze_enable_interrupts();

	Status = XST_SUCCESS;

END:
	return Status;
}

/************************************************************************************/
/**
 * @brief	This function is used to get the functions related to partial PDI event
 *
 * @return
 * 			- Functions related to partial PDI event
 ************************************************************************************/
XSecure_PartialPdiEventParams *XLoader_PpdiEventParamsPtr(void)
{
	return (&PpdiEventVars);
}

#else
/*****************************************************************************/
/**
 * @brief	This function handles the partial PDI event based on status of
 *          resources and is applicable only when queuinh mechanism is enabled
 *
 * @param	PdiPtr is the instance pointer that points to PDI details
 * @param	PdiSrc is source of PDI.
 * @param	PdiAddr is the address at which PDI is located in the PDI source
 *
 * @return
 * 			- XST_SUCCESS
*
 *****************************************************************************/
int XLoader_PpdiEventHandling(PdiSrc_t PdiSrc, u64 PdiAddr, u32 IpiMask)
{
	(void)PdiSrc;
	(void)PdiAddr;
	(void)IpiMask;
	return XST_SUCCESS;
}

/************************************************************************************/
/**
 * @brief	This function is used to get the functions related to partial PDI event
 *
 * @return
 * 			- NULL
 ************************************************************************************/
XSecure_PartialPdiEventParams *XLoader_PpdiEventParamsPtr(void)
{
	return NULL;
}
#endif

/**
 * @{
 * @cond xloader_internal
 */
/*****************************************************************************/
/**
 * @brief	This function clears the previous SBI data ready
 * 			and enables IRQ for next interrupt.
 *
 *****************************************************************************/
void XLoader_ClearIntrSbiDataRdy(void)
{
	/**
	 * - Clear the SBI interrupt.
	*/
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_STATUS,
		SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK,
		SLAVE_BOOT_SBI_IRQ_STATUS_DATA_RDY_MASK);
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_IRQ_ENABLE,
		SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK,
		SLAVE_BOOT_SBI_IRQ_ENABLE_DATA_RDY_MASK);

	/**
	 * - Clear and Enable GIC interrupt.
	*/
	XPlmi_GicIntrClearStatus(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
	XPlmi_GicIntrEnable(XPLMI_SBI_GICP_INDEX, XPLMI_SBI_GICPX_INDEX);
}

/**
 * @}
 * @endcond
 */

/** @} */
