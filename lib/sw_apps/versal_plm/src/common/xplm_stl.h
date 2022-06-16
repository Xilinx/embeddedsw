/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_stl.h
*
* This file contains the header functions of wrapper Xilstl
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rama 08/12/2020 Initial release
* 1.01  rama 03/22/2021 Updated hook for periodic STL execution and FTTI
*                       configuration
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_STL_H
#define XPLM_STL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLM_ENABLE_STL

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
#define DEFAULT_FTTI_TIME (90U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlm_StlInit(void);
int XPlm_PeriodicStlHook(void);

/************************** Variable Definitions *****************************/

#endif  /* PLM_ENABLE_STL */

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_STL_H */
