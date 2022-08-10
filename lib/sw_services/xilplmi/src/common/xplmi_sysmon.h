/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_sysmon.h
*
* This is the header file which contains definitions for the SysMon manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  sn   07/01/2019 Initial release
*       sn   07/04/2019 Enabled SysMon's over-temperature interrupt
*       sn   08/03/2019 Added code to wait until over-temperature condition
*                       gets resolved before restart
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 1.03  bm   02/08/2021 Add GetSysmonInst API
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       is   01/10/2022 Updated XPlmi_SysMonOTDetect API to pass wait time arg
*       is   01/10/2022 Updated Copyright Year to 2022
*       ma   08/08/2022 Fix SW-BP-MAGIC-NUM warning
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_SYSMON_H
#define XPLMI_SYSMON_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsysmonpsv.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PCSR_UNLOCK_VAL       (0xF9E8D7C6U)
#define XPLMI_SYSMON_NO_WAIT_TIME	(0x0U)

/************************** Function Prototypes ******************************/
int XPlmi_SysMonInit(void);
void XPlmi_SysMonOTDetect(u32 WaitInMSec);
XSysMonPsv* XPlmi_GetSysmonInst(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_SYSMON_H */
