/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi.h
*
* This file contains declarations PLMI module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/07/2019 Initial release
* 1.01  ma   08/01/2019 Added LPD init code
* 1.02  kc   02/19/2020 Moved code to support PLM banner from PLM app
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  07/07/2020 Made functions used in single transaltion unit as
*						static
*       kc   07/28/2020 Added WDT MACRO to indicate WDT initialized
*	    skd  07/29/2020 Added device copy macros
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_H
#define XPLMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xplmi_gic_interrupts.h"
#include "xplmi_proc.h"
#include "xplmi_generic.h"
#include "xplmi_util.h"
#include "xplmi_task.h"

/************************** Constant Definitions *****************************/
/* SDK release version */
#define SDK_RELEASE_YEAR	"2020"
#define SDK_RELEASE_QUARTER	"2"

/*
 * Device Copy flag related macros
 */
#define XPLMI_DEVICE_COPY_STATE_MASK		(0x7U << 5U)
#define XPLMI_DEVICE_COPY_STATE_BLK			(0x0U << 5U)
#define XPLMI_DEVICE_COPY_STATE_INITIATE	(0x1U << 5U)
#define XPLMI_DEVICE_COPY_STATE_WAIT_DONE	(0x2U << 5U)

/**************************** Type Definitions *******************************/
#define UART_INITIALIZED	((u8)(1U << 0U))
#define LPD_INITIALIZED		((u8)(1U << 1U))
#define LPD_WDT_INITIALIZED	((u8)(1U << 2U))

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * Using FW_IS_PRESENT to indicate Boot PDI loading is completed
 */
#define XPlmi_SetBootPdiDone()	XPlmi_UtilRMW(PMC_GLOBAL_GLOBAL_CNTRL, \
					PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK, \
					PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)

#define XPlmi_IsLoadBootPdiDone() (((XPlmi_In32(PMC_GLOBAL_GLOBAL_CNTRL) & \
				PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) == \
				PMC_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) ? \
					(TRUE) : (FALSE))

/************************** Function Prototypes ******************************/
int XPlmi_Init(void);
void XPlmi_LpdInit(void);
void XPlmi_ResetLpdInitialized(void);

/************************** Variable Definitions *****************************/
extern u8 LpdInitialized;

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_H */
