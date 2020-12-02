/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_init.c
*
* This file contains the initialization functions to be called by PLM. This
* file will only be part of XilSecure when it is compiled with PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rpo 06/25/2020 Initial release
* 4.3   rpo 06/25/2020 Updated file version to sync with library version
*	rpo 08/19/2020 Clear the tamper interrupt source
*	am  09/24/2020 Resolved MISRA C violations
*       har 10/12/2020 Addressed security review comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_util.h"
#include "xplmi_err.h"
#include "xsecure_init.h"
#include "xsecure_tamper.h"

/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_EVENT_ERROR_PMC_ERR2	(0x28104000U)
#define	XPLMI_NODEIDX_ERROR_PMCAPB	(32U)
#define XSECURE_TAMPER_INT_MASK		(8U)
#define XSECURE_GD0_GLITCH_STATUS_MASK	(0x200U)
#define XSECURE_GD1_GLITCH_STATUS_MASK	(0x2000000U)
#define XSECURE_GD_STATUS 		(XSECURE_GD1_GLITCH_STATUS_MASK | \
					 XSECURE_GD0_GLITCH_STATUS_MASK)
#define PMC_ANALOG_GD_CTRL_REG		(0xF1160000U)
#define PMC_GLOBAL_ISR_REG		(0xF1110010U)

/************************** Function Prototypes ******************************/
static int XSecure_RegisterTampIntHandler(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt
 *
 * @param	None
 *
 * @return	- XST_SUCCESS - On success
 *     		- XPLMI_INVALID_ERROR_ID      - On invalid ID
 *      	- XPLMI_INVALID_ERROR_HANDLER - On Null handler
 *
 *****************************************************************************/
int XSecure_Init(void)
{
	int Status = XST_FAILURE;

	Status = XSecure_RegisterTampIntHandler();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt
 *
 * @param	None
 *
 * @return	- XST_SUCCESS - On success
 *     		- XPLMI_INVALID_ERROR_ID      - On invalid ID
 *      	- XPLMI_INVALID_ERROR_HANDLER - On Null handler
 *
 *****************************************************************************/
static int XSecure_RegisterTampIntHandler(void)
{
	int Status = XST_FAILURE;

	/**
	 * Register handler
	 */
	Status = XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PMC_ERR2,
				   XPLMI_NODEIDX_ERROR_PMCAPB,
				   XPLMI_EM_ACTION_CUSTOM,
				   XSecure_TamperInterruptHandler);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Enable tamper interrupt in PMC GLOBAL
	 */
	XSecure_EnableTamperInterrupt();

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
 * @return	None
 *
 *****************************************************************************/
void XSecure_TamperInterruptHandler(const u32 ErrorNodeId, const u32 ErrorMask)
{
	(void)ErrorNodeId;
	(void)ErrorMask;

	(void)XSecure_ProcessTamperResponse();

	/**
	 * Clear the interrupt source
	 */
	Xil_UtilRMW32(PMC_ANALOG_GD_CTRL_REG,	XSECURE_GD_STATUS, XSECURE_GD_STATUS);
	Xil_Out32(PMC_GLOBAL_ISR_REG, XSECURE_TAMPER_INT_MASK);
	Xil_UtilRMW32(PMC_ANALOG_GD_CTRL_REG, XSECURE_GD_STATUS, 0U);

	/**
	 * Once the interrupt is detected by PLM, disables the interrupt and
	 * calls the handler. So it is necessary to re-enable the interrupt
	 */
	(void)XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PMC_ERR2,
				XPLMI_NODEIDX_ERROR_PMCAPB,
				XPLMI_EM_ACTION_CUSTOM,
				XSecure_TamperInterruptHandler);
}
