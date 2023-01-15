/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_l.h
* @addtogroup wdttb Overview
* @{
*
* The xwdttb_l.h header file contains identifiers and basic driver functions (or
* macros) that can be used to access the device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b rpm  04/26/02 First release
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  22/10/09 The following macros defined in this file have been
*		      removed -
*		      XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt, XWdtTb_mRestartWdt
*		      XWdtTb_mGetTimebaseReg and XWdtTb_mHasReset.
*		      Added the XWdtTb_ReadReg and XWdtTb_WriteReg
*		      macros. User should XWdtTb_ReadReg/XWdtTb_WriteReg to
*		      achieve the desired functionality of the macros that
*		      were removed.
* 4.0   sha  12/17/15 Added Window WDT feature with basic mode.
*                     Removed extra include files.
*                     Moved constant and macro definitions to xwdttb_hw.h file.
* 4.3   srm  01/30/18 Added doxygen tags
* </pre>
*
******************************************************************************/

/** @cond INTERNAL */
#ifndef XWDTTB_L_H		/**< prevent circular inclusions */
#define XWDTTB_L_H		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xwdttb_hw.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif
/** @endcond */
/** @} */
