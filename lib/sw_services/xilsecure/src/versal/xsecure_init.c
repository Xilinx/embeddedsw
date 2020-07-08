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
#define PMC_GLOBAL_IER_REG_ADDR			(0xF1110018U)
#define PMC_GLOBAL_IER_TAMPER_INT		(0x00000008U)
#define XPLMI_EVENT_ERROR_PMC_ERR2		(0x28104000U)
#define	XPLMI_NODEIDX_ERROR_PMCAPB		(32U)

/************************** Function Prototypes ******************************/
static u32 XSecure_RegisterTampIntHandler(void);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt.
 *
 * @param	None
 *
 * @return
 *		- Returns XST_SUCCESS on success.
 *		- Returns error code on failure
 *
 *****************************************************************************/
u32 XSecure_Init(void)
{
	u32 Status = (u32)XST_FAILURE;

	Status = XSecure_RegisterTampIntHandler();

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the handler for tamper interrupt.
 *
 * @param	None
 *
 * @return
 *		- Returns XST_SUCCESS on success.
 *		- Returns error code on failure
 *
 *****************************************************************************/
static u32 XSecure_RegisterTampIntHandler(void)
{
	u32 Status = (u32)XST_FAILURE;
	/**
	 * Register handler
	 */
	Status = (u32)XPlmi_EmSetAction(XPLMI_EVENT_ERROR_PMC_ERR2,
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
 * @brief	This is the handler for tamper interrupt.
 *
 * @param	ErrorNodeId - Node Identifier
 * @param	ErrorMask - Mask Identifier
 *
 * @return
 *		- Returns XST_SUCCESS on success.
 *		- Returns error code on failure
 *
 *****************************************************************************/
void XSecure_TamperInterruptHandler(const u32 ErrorNodeId, const u32 ErrorMask)
{

	(void)ErrorNodeId;
	(void)ErrorMask;

	(void)XSecure_ProcessTamperResponse();
}
