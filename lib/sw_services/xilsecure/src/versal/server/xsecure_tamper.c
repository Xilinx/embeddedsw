/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
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
* 4.4   kpt 01/27/2021 Fixed bug in clearing tamper interrupt
* 4.5   kpt 02/04/2021 Added redundancy for tamper interrupt and response
*                      checks
* 4.6   har 07/14/2021 Fixed doxygen warnings
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsecure_tamper.h"
#include "xil_util.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name PMC_GLOBAL_IER register
 * @{
 */
/**< PMC_GLOBAL_IER register offset and definitions */
#define PMC_GLOBAL_IER_REG_ADDR		(0xF1110018U)
#define PMC_GLOBAL_IER_TAMPER_INT	(0x00000008U)
/** @} */

/**
 * @name PMC_GLOBAL_ISR register
 * @{
 */
/**< PMC_GLOBAL_ISR register offset and definitions */
#define PMC_GLOBAL_ISR_REG_ADDR		(0xF1110010U)
#define PMC_GLOBAL_ISR_TAMPER_INT	(0x00000008U)
/** @} */

#define PMC_GLOBAL_TAMPER_RESP_0	(0xF1110530U)
					/**< TAMPER_RESP_0 register offset */

#define PMC_GLOBAL_SLD_MASK		((1U<<2U)|(1U<<3U))
					/**< Secure Lockdown mask */

/**
 * @name TAMPER_TRIG register
 * @{
 */
/**< TAMPER_TRIG register offset and definitions */
#define PMC_GLOBAL_TAMPER_TRIG		(0xF1110570U)
#define PMC_GLOBAL_TAMPER_TRIG_VAL	(1U)
/** @} */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function enables the tamper interrupt in PMC_GLOBAL
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
 * @return 	- XST_SUCCESS - Always
 *
 *****************************************************************************/
int XSecure_ProcessTamperResponse(void)
{
	int Status = XST_FAILURE;
	volatile u32 TamperResponse = PMC_GLOBAL_SLD_MASK;
	volatile u32 TamperResponseTmp = PMC_GLOBAL_SLD_MASK;
	volatile u32 IntVal = PMC_GLOBAL_ISR_TAMPER_INT;
	volatile u32 IntValTmp = PMC_GLOBAL_ISR_TAMPER_INT;

	/**
	 * Check the reason for interrupt
	 */
	IntVal = Xil_In32(PMC_GLOBAL_ISR_REG_ADDR) &
		PMC_GLOBAL_ISR_TAMPER_INT;
	IntValTmp = Xil_In32(PMC_GLOBAL_ISR_REG_ADDR) &
		PMC_GLOBAL_ISR_TAMPER_INT;
	if ((IntVal == PMC_GLOBAL_ISR_TAMPER_INT) ||
		(IntValTmp == PMC_GLOBAL_ISR_TAMPER_INT)) {
		TamperResponse = Xil_In32(PMC_GLOBAL_TAMPER_RESP_0) &
				PMC_GLOBAL_SLD_MASK;
		TamperResponseTmp = Xil_In32(PMC_GLOBAL_TAMPER_RESP_0) &
				PMC_GLOBAL_SLD_MASK;
		if ((TamperResponse != 0U) || (TamperResponseTmp != 0U)) {
			XSecure_SecureLockDown();

			/**
			 * Trigger software tamper event to ROM to execute lockdown
			 * for PMC
			 */
			Status = Xil_SecureOut32(PMC_GLOBAL_TAMPER_TRIG,
					PMC_GLOBAL_TAMPER_TRIG_VAL);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			/**
			 * Wait forever; ROM to complete secure lock down
			 */
			while(1U) {
			 ;
			};
		}
		else {
			Status = (int)XSECURE_NO_TAMPER_RESPONSE;
			goto END;
		}
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function executes the secure lock down for LPD/FPD/PL/NoC
 *
 * @return	None
 *
 *****************************************************************************/
 void XSecure_SecureLockDown(void)
 {
	 return;
 }
