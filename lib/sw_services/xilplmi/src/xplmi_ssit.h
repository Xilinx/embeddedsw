/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_ssit.h
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains declarations for SSIT functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   08/13/2019 Initial release
*       ma   08/24/2019 Added SSIT commands
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
*       ana  10/19/2020 Added doxygen comments
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XPLMI_SSIT_H
#define XPLMI_SSIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xplmi.h"

/************************** Constant Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */

/**
 * SSIT defines
 */
#define PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK	(1U)
#define PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK	(4U)
#define PMC_GLOBAL_SSIT_ERR_MASK		(0xE0000000U)

#define SSIT_SLAVE_0_MASK			(1U)
#define SSIT_SLAVE_1_MASK			(2U)
#define SSIT_SLAVE_2_MASK			(4U)
/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd);
int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd);
int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_SSIT_H */

/** @} */