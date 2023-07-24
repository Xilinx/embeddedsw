/******************************************************************************
* Copyright (c) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdeint_i.h
* @addtogroup deinterlacer Overview
* @{
*
* This code contains internal functions of the Xilinx Video Deinterlacer core.
* The application should not need the functions in this code to control
* the Video Deinterlacer core. Read xdeint.h for detailed information about
* the core.
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a rjh  07/10/11 First release.
* 2.00a rjh  18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk  02/13/14 Added Doxygen support.
* </pre>
*
******************************************************************************/

#ifndef XDEINT_I_H
#define XDEINT_I_H		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/


/************************** Constant Definitions *****************************/

/* Base address fetch */
#define XDeint_BaseAddr(InstancePtr) ((InstancePtr)->Config.BaseAddress)

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif	/* End of protection macro */
/** @} */
