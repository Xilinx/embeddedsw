/******************************************************************************
* Copyright (c) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_util.h
*
* This is the header file PMC FW utilities code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  bsv  04/18/2019 Added support for NPI readback and CFI readback
*       kc   04/26/2019 Updated Delay and Poll timeout based on timers
*       rm   06/27/2019 Added APIs for safety register writes
*       vnsl 07/19/2019 Added XPlmi_MemCmp API to check for PPK and SPK integrity
* 1.02  bsv  02/17/2020 Added 64-bit / 128-bit safety write APIs for xilsem
*       bsv  04/04/2020 Code clean up
* 1.03  kc   06/22/2020 Minor updates to PrintArray for better display
*       bsv  09/04/2020 Added checks to validate input params for XPlmi_Strcat
*                       and XPlmi_Strcpy
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
* 1.04  bm   03/04/2021 Add VerifyAddrRange API
* 1.05  bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/02/2021 Code clean up to reduce size
*       bsv  08/15/2021 Removed unwanted goto statements
*       bsv  09/05/2021 Disable prints in slave boot modes in case of error
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
* 1.07  bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/13/2023 Added BIT mask creation macro
*       dd	 09/11/2023 MISRA-C violation Rule 12.2 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_UTIL_H_
#define XPLMI_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_ARRAY_SIZE(x)	(u32)(sizeof(x) / sizeof(x[0U]))
#define MASK_ALL	(0XFFFFFFFFU)
#define MASK32_ALL_HIGH	(0xFFFFFFFFU)
#define XPLMI_TIME_OUT_DEFAULT	(0x10000000U)
#define XPLMI_WORD_LEN			(4U)
#define XPLMI_BIT(pos)			((u32)0x1U << (pos))
/************************** Function Prototypes ******************************/
void XPlmi_UtilRMW(u32 RegAddr, u32 Mask, u32 Value);
int XPlmi_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs);
int XPlmi_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs,
		void (*ClearHandler)(void));
int XPlmi_UtilPoll64(u64 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs);
void XPlmi_UtilWrite64(u32 HighAddr, u32 LowAddr, u32 Value);
int XPlmi_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask,
	u32 TimeOutInUs);
void XPlmi_PrintArray (u16 DebugType, const u64 BufAddr, u32 Len, const char *Str);
int XPlmi_UtilPollNs(u32 RegAddr, u32 Mask, u32 ExpectedValue, u64 TimeOutInNs,
		void (*ClearHandler)(void));

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_UTIL_H_ */
