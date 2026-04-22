/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_sem_init.c
*
* This file contains the SEM Init APIs to initialize the XilSEM modules.
* Also, supports Beam testing requirement task which prints periodically
* total correctable error count for every 20 seconds.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  kc   02/10/2020 Updated scheduler to add/remove tasks
*       kc   02/17/2020 Added configurable priority for scheduler tasks
*       kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
*       hb   10/29/2020 Updated OwnerId for NPI scan scheduler
* 1.02  rb   10/30/2020 Updated XilSEM Init API
* 1.03  rb   01/28/2021 Added Sem PreInit API to have CDO command handler
*                       initialization, removed unused header file
*       rb   03/09/2021 Updated Sem Init API call
* 1.03  ga   05/03/2023 Removed XPlm_SemInit function and updated
*                       copyright information
* 1.10  ng   11/20/2023 Fixed doxygen grouping
* 2.00  gam  04/08/2026 Add support for Beam testing and protect the code with
*                       macro PLM_SEM_BEAM_TESTING.
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xplm_apis Versal PLM APIs
 * @{
 * @cond xplm_internal
 */

/***************************** Include Files *********************************/

#include "xplm_sem_init.h"

#ifdef XPLM_SEM
#include "xilsem.h"
#ifdef PLM_SEM_BEAM_TESTING
#include "xplmi_scheduler.h"
#include "xplmi_debug.h"
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
#include "xplmi_ssit.h"
#endif /* End of PLM_ENABLE_PLM_TO_PLM_COMM */
#endif /* End of PLM_SEM_BEAM_TESTING */
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef PLM_SEM_BEAM_TESTING
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/*****************************************************************************/
/**
 * @brief		This function Reads the Get SLR address from XPLMI API.
 *
 * @param[in]	SlrIndex : Target SLR in which address need to be read.
 * @param[in]	Addr     : Register address to be read.
 *
 * @return
 *			- Register value of SSIT slave address.
 *
 *****************************************************************************/
static u32 XPlm_SsitReadSlrAddr(u32 SlrIndex, u32 Addr)
{
	u64 TargetRegAddr = 0U;

	TargetRegAddr = XPlmi_SsitGetSlrAddr((u32)Addr, (u8)SlrIndex);

	return XPlmi_In64(TargetRegAddr);
}
#endif /* End of PLM_ENABLE_PLM_TO_PLM_COMM */

/*****************************************************************************/
/**
 * @brief		This functions prints Correctable Error count.
 *
 * @param		Data is not used
 *
 * @return
 *			- Success
 *
 *****************************************************************************/
int XPlm_SemCfrErrCntPrint(void *Data)
{
	(void) Data;
	u32 Slrtype = 0U;
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	u32 SlrTcCnt[4];
	u32 TargetSlr = 1U;
	u32 SlrId;
#endif
		Slrtype = Xil_In32(PMC_TAP_SLR_TYPE_ADDR);
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	if (Slrtype == PMC_SLR_MASTER) {
		/* Read Master SLR Total Correctable Error count */
		SlrTcCnt[0] = Xil_In32(PMC_RAM_SEM_CRAM_COR_BITCNT);
		/* Read Slave SLR Total Correctable Error count */
		for (SlrId = 1U; SlrId < XSEM_SSIT_MAX_SLR_CNT; SlrId++) {
			SlrTcCnt[SlrId] = XPlm_SsitReadSlrAddr(TargetSlr, PMC_RAM_SEM_CRAM_COR_BITCNT);
			TargetSlr++;
		}
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS,"TCM%d S1%d S2%d S3%d\n", SlrTcCnt[0], SlrTcCnt[1], \
									SlrTcCnt[2], SlrTcCnt[3]);
	}
#else
	if (Slrtype == PMC_SLR_MASTER) {
		/* Read Master SLR total Correctable Error count */
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS,"TCM%d \n", Xil_In32(PMC_RAM_SEM_CRAM_COR_BITCNT));
	}
#endif /* End of PLM_ENABLE_PLM_TO_PLM_COMM */
	return 0;
}
#endif /* End of PLM_SEM_BEAM_TESTING */

/*****************************************************************************/
/**
 * @brief	This function calls the scan init functions of XilSEM.
 * As a part of this, XilSEM library can initiate scan on both CRAM and NPI.
 * Scan will be decided based on the CIPS params in xparameters.h.
 * Also, supports Beam testing requirement
 *
 * @param	Arg is not used
 *
 * @return
 *			- XST_SUCCESS on success
 *			- XST_FAILURE on failure
 *			- Status as defined in XilSEM library
 *			- If Beam testing enabled, Status as defined in Xplmi Scheduler
 *
 *****************************************************************************/
int XPlm_SemScanInit(void *Arg)
{
	int Status = XST_FAILURE;
	(void)Arg;
#ifdef PLM_SEM_BEAM_TESTING
	void * volatile CramStatusAddr = (void *)PMC_RAM_SEM_CRAM_STATUS;
	/* Clear PMC RAM space for CRAM scan */
	(void)Xil_SMemSet(CramStatusAddr, ((u32)PMC_RAM_CRAM_REGCNT * 4U),
				(u8)0x00U, ((u32)PMC_RAM_CRAM_REGCNT * 4U));
#endif
	Status = XSem_InitScan();
	if (XST_SUCCESS != Status) {
		goto END;
	}
#ifdef PLM_SEM_BEAM_TESTING
	/* Add periodic print task to the PLM scheduler */
	Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_SEM_ID, \
			(XPlmi_Callback_t)&XPlm_SemCfrErrCntPrint, NULL, ERR_PRINT_TIMEOUT, \
					XPLM_TASK_PRIORITY_0, NULL, XPLMI_PERIODIC_TASK);
	if (XST_SUCCESS != Status) {
		XPlmi_Printf_WoTS(DEBUG_PRINT_ALWAYS,"Error Correction cnt print task creation failed\n");
		goto END;
	}
#endif

END:
	return Status;
}
#endif
