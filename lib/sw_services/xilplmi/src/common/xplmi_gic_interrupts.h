/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_gic_interrupts.h
*
* This is the header file for xplm_gic_interrupts.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   10/08/2018 Initial release
* 1.01  kc   04/09/2019 Added code to register/enable/disable interrupts
* 1.02  bsv  04/04/2020 Code clean up
* 1.03  bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bm   04/03/2021 Move task creation out of interrupt context
* 1.05  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       ma   08/05/2021 Add separate task for each IPI channel
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_GIC_INTERRUPTS_H
#define XPLMI_GIC_INTERRUPTS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_ipi.h"
#include "xplmi_task.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_NO_OF_BITS_IN_REG		(32U)

/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/* Handler Table Structure */
typedef int (*GicIntHandler_t)(void *Data);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* Functions defined in xplm_main.c */
void XPlmi_GicIntrHandler(void *CallbackRef);
int XPlmi_GicRegisterHandler(u32 GicPVal, u32 GicPxVal, GicIntHandler_t Handler,
	void *Data);
void XPlmi_GicIntrEnable(u32 GicPVal, u32 GicPxVal);
void XPlmi_GicIntrDisable(u32 GicPVal, u32 GicPxVal);
void XPlmi_GicIntrClearStatus(u32 GicPVal, u32 GicPxVal);
void XPlmi_GicIntrAddTask(u32 Index);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_GIC_INTERRUPTS_H */
