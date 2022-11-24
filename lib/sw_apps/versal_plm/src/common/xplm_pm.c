/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_pm.c
*
* This file contains the wrapper code xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
* 1.01  rp   08/08/2019 Added code to send PM notify callback through IPI
* 1.02  kc   03/23/2020 Minor code cleanup
* 1.03  kc   08/04/2020 Initialized IpiMask to zero for PMC CDO commands
*       kc   08/04/2020 Added default NPLL configuration for master SLR devices
* 1.04  ma   01/12/2021 Initialize SlrType to invalid SLR type
*                       Call PMC state clear function when error occurs while
*                        processing PLM CDO
*       bm   02/08/2021 Added SysmonInit after processing PMC CDO
*       skd  03/16/2021 Added code to monitor if psm is alive or not
*       rama 03/22/2021 Added hook for STL periodic execution and
*                       FTTI configuration support for keep alive task
*       bm   04/10/2021 Updated scheduler function calls
*       bsv  04/16/2021 Add provision to store Subsystem Id in XilPlmi
*       bm   04/27/2021 Updated priority of XPlm_KeepAliveTask
* 1.05  td   07/08/2021 Fix doxygen warnings
*       ma   07/12/2021 Register NULL error handler for XPlm_KeepAliveTask
*       bm   08/09/2021 Cleared PMC CDO buffer by default after processing
*       bsv  08/13/2021 Remove unwanted header file
*       bsv  08/13/2021 Removed unwanted goto statements
*       kpt  09/09/2021 Fixed SW-BP-BLIND-WRITE in XLoader_SecureClear
* 1.06  skd  11/18/2021 Added time stamps in XPlm_ProcessPmcCdo
*       bm   01/05/2022 Fixed ZEROIZE-PRIORITY for XLoader_SecureClear
* 1.07  skd  04/20/2022 Misra-C violation Rule 17.7 fixed
* 1.08  bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.09  ng   11/11/2022 Updated doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xpm_api.h"
#include "xpm_ipi.h"
#include "xpm_psm.h"
#include "xpm_subsystem.h"
#include "xplmi_scheduler.h"
#include "xplmi_util.h"
#include "xloader.h"
#include "xloader_secure.h"
#include "xplmi.h"
#include "xplmi_sysmon.h"
#include "xplmi_task.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_cdo.h"
#include "xplm_plat.h"
#ifdef PLM_ENABLE_STL
#include "xplm_stl.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XPlm_PmRequestCb(const u32 IpiMask, const XPmApiCbId_t EventId, u32 *Payload);
#ifdef XPLMI_IPI_DEVICE_ID
static int XPlm_SendKeepAliveEvent(void);
static int XPlm_KeepAliveTask(void *Arg);
#endif /* XPLMI_IPI_DEVICE_ID */

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function is registered with XilPm to send any data to IPI master
* when a event comes.
*
* @param	IpiMask IPI master ID
* @param	EventId Id of the event as defined in XilPm
* @param	Payload is pointer to the Data that needs to be sent to the IPI
* 		master
*
* @return	None
*
*****************************************************************************/
static void XPlm_PmRequestCb(const u32 IpiMask, const XPmApiCbId_t EventId, u32 *Payload)
{
#ifdef XPLMI_IPI_DEVICE_ID
	XStatus Status = XST_FAILURE;

	if ((PM_INIT_SUSPEND_CB == EventId) || (PM_NOTIFY_CB == EventId)) {
		Status = XPlmi_IpiWrite(IpiMask, Payload, XPLMI_CMD_RESP_SIZE,
					XIPIPSU_BUF_TYPE_MSG);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL,
			 "%s Error in IPI write: %d\r\n", __func__, Status);
		} else {
			Status = XPlmi_IpiTrigger(IpiMask);
			if (XST_SUCCESS != Status) {
				XPlmi_Printf(DEBUG_GENERAL,
				"%s Error in IPI trigger: %d\r\n", __func__, Status);
			}
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL,
		 "%s Error: Unsupported EventId: %d\r\n", __func__, EventId);
	}
#else
	XPlmi_Printf(DEBUG_GENERAL, "%s Error: IPI is not defined\r\n", __func__);
#endif /* XPLMI_IPI_DEVICE_ID */
}

/*****************************************************************************/
/**
* @brief It calls the XilPm initialization API to initialize its structures.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_PmInit(void)
{
	int Status = XST_FAILURE;

	/**
	 * Initialize the XilPm component. It registers the callback handlers,
	 * variables, events
	 */
	Status = XPm_Init(XPlm_PmRequestCb, &XLoader_RestartImage);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_PM_MOD, Status);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief This function executes the PMC CDO present in PMC RAM.
*
* @param	Arg Not used in the function
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_ProcessPmcCdo(void *Arg)
{
	int Status = XST_FAILURE;
	int SStatus = XST_FAILURE;
	XPlmiCdo Cdo;
	u32 SlrType = XLOADER_SSIT_INVALID_SLR;
#ifdef PLM_PRINT_PERF_CDO_PROCESS
	u64 TaskStartTime;
	XPlmi_PerfTime PerfTime;
#endif

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	(void )Arg;

#ifdef PLM_PRINT_PERF_CDO_PROCESS
	TaskStartTime = XPlmi_GetTimerValue();
#endif

	/**
	 * Configure NoC frequency equivalent to the frequency ROM sets in
	 * Slave devices
	 */
	SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK;
	if (SlrType == XLOADER_SSIT_MASTER_SLR) {
		Status = XPlm_ConfigureDefaultNPll();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 *  Pass the PLM CDO to CDO parser, PLM CDO contains
	 *  - Device topology
	 *  - PMC block configuration
	 */

	/* Process the PLM CDO */
	Status = XPlmi_InitCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Cdo.BufPtr = (u32 *)XPLMI_PMCRAM_BASEADDR;
	Cdo.BufLen = XPLMI_PMCRAM_LEN;
	Cdo.SubsystemId = PM_SUBSYS_PMC;
	Status = XPlmi_ProcessCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		(void)XLoader_SecureClear();
		goto END;
	}

#ifdef PLM_PRINT_PERF_CDO_PROCESS
	XPlmi_MeasurePerfTime(TaskStartTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF, "%u.%03u ms: PMC CDO processing time\n\r",
			(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
#endif

	Status = XPlmi_SysMonInit();

END:
	/** Clear PMC CDO memory irrespective of success or failure */
	SStatus = XPlmi_MemSet(XPLMI_PMCRAM_BASEADDR, XPLMI_DATA_INIT_PZM,
			XPLMI_PMC_CDO_MAX_WORD_LEN);
	if (XST_SUCCESS != SStatus) {
		SStatus = XLoader_SecureClear();
		if (Status == XST_SUCCESS) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_PMC_RAM_MEMSET,
					SStatus);
		}
	}

	return Status;
}

#ifdef XPLMI_IPI_DEVICE_ID
/*****************************************************************************/
/**
* @brief	This function sends keep alive IPI event to PSM
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
static int XPlm_SendKeepAliveEvent(void)
{
	int Status = XST_FAILURE;
	u32 Payload[XPLMI_IPI_MAX_MSG_LEN] = {0U};

	/** Assign PSM keep alive API ID to IPI payload[0] */
	Payload[0U] = XPLM_PSM_API_KEEP_ALIVE;

	/** Send IPI for keep alive event to PSM */
	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf(DEBUG_GENERAL, "%s Error in IPI send: %0x\r\n",
				__func__, Status);
		/* Update status in case of error */
		Status = XPlmi_UpdateStatus(XPLM_ERR_IPI_SEND, Status);
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if PSM is alive and healthy.
*
* @param	Arg Not used in the function currently
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
static int XPlm_KeepAliveTask(void *Arg)
{
	int Status = XST_FAILURE;
	u32 ActualCounterValue;
#ifdef PLM_ENABLE_STL
	int StlStatus = XST_FAILURE;
#endif

	(void)Arg;

	/**
	 * Check if PSM is running and PSMFW is loaded and no error occurred
	 * from PSM Keep alive event.
	 */
	if (((u8)TRUE == XPmPsm_FwIsPresent()) && (XPLM_PSM_ALIVE_ERR !=
	    XPlm_SetPsmAliveStsVal(XPLM_PSM_ALIVE_RETURN))) {
		/**
		 * Check if the keep alive task called for first time then skip
		 * comparing keep alive counter value.
		 */
		if (XPLM_PSM_ALIVE_STARTED == XPlm_SetPsmAliveStsVal(XPLM_PSM_ALIVE_RETURN)) {
			/**
			 * Read keep alive counter value from RTCA(Run time
			 * configuration area) register.
			 */
			ActualCounterValue = XPlmi_In32(XPLM_PSM_ALIVE_COUNTER_ADDR);
			/* Increment expected keep alive counter value */
			(void)XPlm_UpdatePsmCounterVal(XPLM_PSM_COUNTER_INCREMENT);
			/**
			 * Check if PSM incremented keep alive counter value or
			 * not. Return error if counter value is not matched
			 * with expected value.
			 */
			if (ActualCounterValue != XPlm_UpdatePsmCounterVal(XPLM_PSM_COUNTER_RETURN)) {
				XPlmi_Printf(DEBUG_GENERAL, "%s ERROR: PSM is not alive\r\n",
						__func__);
				/* Clear RTCA register */
				XPlmi_Out32(XPLM_PSM_ALIVE_COUNTER_ADDR,
						0U);
				/* Clear expected counter value */
				(void)XPlm_UpdatePsmCounterVal(XPLM_PSM_COUNTER_CLEAR);
				/* Update PSM keep alive status for error */
				(void)XPlm_SetPsmAliveStsVal(XPLM_PSM_ALIVE_ERR);
				/* Remove Keep alive task in case of error */
				Status = XPlm_RemoveKeepAliveTask();
				/* Update the error status */
				Status = XPlmi_UpdateStatus(XPLM_ERR_PSM_NOT_ALIVE,
								Status);
				goto END;
			}
		}

		/** Send keep alive IPI event to PSM */
		Status = XPlm_SendKeepAliveEvent();
		if (XST_SUCCESS != Status) {
			/* Remove Keep alive task in case of error */
			(void)XPlm_RemoveKeepAliveTask();
			goto END;
		}

		/* Update PSM keep alive status as successfully started */
		(void)XPlm_SetPsmAliveStsVal(XPLM_PSM_ALIVE_STARTED);
	}

	Status = XST_SUCCESS;

END:
#ifdef PLM_ENABLE_STL
	/* Execute STL periodic Tasks */
	StlStatus = XPlm_PeriodicStlHook();
	if (XST_SUCCESS == Status) {
		Status = StlStatus;
	}
#endif
	return Status;
}

/*****************************************************************************/
/**
* @brief This function creates keep alive scheduler task
*
* @param PtrMilliSeconds periodicity of the task (must be > 10ms)
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_CreateKeepAliveTask(void *PtrMilliSeconds)
{
	int Status = XST_FAILURE;
	u32 MilliSeconds = *(u32*)PtrMilliSeconds;

	/**
	 * Validate input parameter (MilliSeconds) which needs to be greater
	 * than minimum FTTI time (10ms).
	 */
	if (XPLM_MIN_FTTI_TIME > MilliSeconds) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_KEEP_ALIVE_TASK_CREATE,
						XST_INVALID_PARAM);
		goto END;
	}

	/* Clear keep alive counter and status as not started */
	XPlmi_Out32(XPLM_PSM_ALIVE_COUNTER_ADDR, 0U);
	(void)XPlm_UpdatePsmCounterVal(XPLM_PSM_COUNTER_CLEAR);
	(void)XPlm_SetPsmAliveStsVal(XPLM_PSM_ALIVE_NOT_STARTED);

	/**
	 * Add keep alive task in scheduler which runs at every
	 * XPLM_DEFAULT_FTTI_TIME period.
	 */
	Status = XPlmi_SchedulerAddTask(XPLM_PSM_HEALTH_CHK, XPlm_KeepAliveTask,
			NULL, MilliSeconds, XPLM_TASK_PRIORITY_0, NULL,
			XPLMI_PERIODIC_TASK);
	if (XST_SUCCESS != Status) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_KEEP_ALIVE_TASK_CREATE,
						Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief This function remove keep alive scheduler task
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_RemoveKeepAliveTask(void)
{
	int Status = XST_FAILURE;

	/* Remove keep alive task from scheduler */
	Status = XPlmi_SchedulerRemoveTask(XPLM_PSM_HEALTH_CHK,
			XPlm_KeepAliveTask, 0U, NULL);
	if (XST_SUCCESS != Status) {
		/* Update minor error value to status */
		Status = (int)XPLM_PSM_ALIVE_REMOVE_TASK_ERR;
	}

	return Status;
}
#endif /* XPLMI_IPI_DEVICE_ID */
