/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_tamper.c
*
* This file contains the tamper response processing routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   rpo 06/25/2020 Initial release
* 4.3   rpo 06/25/2020 Updated file version to sync with library version
*       am  09/24/2020 Resolved MISRA C violations
*       har 10/12/2020 Addressed security review comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_tamper.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PMC_GLOBAL_IER_REG_ADDR		(0xF1110018U)
#define PMC_GLOBAL_IER_TAMPER_INT	(0x00000008U)
#define PMC_GLOBAL_ISR_REG_ADDR		(0xF1110010U)
#define PMC_GLOBAL_ISR_TAMPER_INT	(0x00000008U)
#define PMC_GLOBAL_TAMPER_RESP_0	(0xF1110530U)
#define PMC_GLOBAL_SLD_MASK		((1U<<2U)|(1U<<3U))
#define PMC_GLOBAL_TAMPER_TRIG		(0xF1110570U)
#define PMC_GLOBAL_TAMPER_TRIG_VAL	(1U)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function enables the tamper interrupt in PMC_GLOBAL
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XSecure_EnableTamperInterrupt(void)
 {
	/**
	 * Enable tamper interrupt in PMC GLOBAL
	 */
	Xil_Out32(PMC_GLOBAL_IER_REG_ADDR, PMC_GLOBAL_IER_TAMPER_INT);
 }

/*****************************************************************************/
/**
 * @brief	This function processes the tamper response
 *
 * @param	None
 *
 * @return 	- XST_SUCCESS - Always
 *
 *****************************************************************************/
int XSecure_ProcessTamperResponse(void)
{
	int Status = XST_FAILURE;
	u32 TamperResponse;

	/**
	 * Clear the tamper interrupt in PMC_GLOBAL
	 */
	Xil_Out32(PMC_GLOBAL_ISR_REG_ADDR, PMC_GLOBAL_ISR_TAMPER_INT);

	/**
	 * Check the reason for interrupt
	 */
	if ((Xil_In32(PMC_GLOBAL_ISR_REG_ADDR) &
		PMC_GLOBAL_ISR_TAMPER_INT) ==
		PMC_GLOBAL_ISR_TAMPER_INT) {
		TamperResponse = Xil_In32(PMC_GLOBAL_TAMPER_RESP_0);
		if ((TamperResponse & PMC_GLOBAL_SLD_MASK) != 0U) {
			XSecure_SecureLockDown();

			/**
			 * Trigger software tamper event to ROM to execute lockdown
			 * for PMC
			 */
			Xil_Out32(PMC_GLOBAL_TAMPER_TRIG, PMC_GLOBAL_TAMPER_TRIG_VAL);

			/**
			 * Wait forever; ROM to complete secure lock down
			 */
			while(1U) {
			 ;
			};
		}
	}

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function executes the secure lock down for LPD/FPD/PL/NoC
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
 void XSecure_SecureLockDown(void)
 {
	 return;
 }
