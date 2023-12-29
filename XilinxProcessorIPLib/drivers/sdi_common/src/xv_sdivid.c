/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xv_sdivid.c
 * @addtogroup sdi_common Overview
 * @{
 *
 * Contains common utility variables that are used by SDI IP
 * drivers and applications.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   jsr  07/17/17 Initial release.
 * 1.1   jsr  10/08/18 Removed redundant declarations
 *                     Added SDI specific timing modes
 * </pre>
 *
*******************************************************************************/


#ifndef XV_SDIVID_C_  /* Prevent circular inclusions by using protection macros. */
#define XV_SDIVID_C_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/

#include "xil_types.h"
#include "xvidc.h"
#include "xv_sdivid.h"

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions ******************************/

/**
 * SDI Specific 3GB timing modes.
 */
const XVidC_VideoTimingMode XVidC_SdiVidTimingModes[XVIDC_SDICUSTOM_NUM_SUPPORTED] =
{
	/* Interlaced modes. */
	{ XVIDC_VM_1920x1080_96_I, "1920x1080@96Hz (I)", XVIDC_FR_96HZ,
		{1920, 371, 88, 371, 2750, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
	{ XVIDC_VM_1920x1080_100_I, "1920x1080@100Hz (I)", XVIDC_FR_100HZ,
		{1920, 528, 44, 148, 2640, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
	{ XVIDC_VM_1920x1080_120_I, "1920x1080@120Hz (I)", XVIDC_FR_120HZ,
		{1920, 88, 44, 148, 2200, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
	{ XVIDC_VM_2048x1080_96_I, "2048x1080@96Hz (I)", XVIDC_FR_96HZ,
		{2048, 329, 44, 329, 2750, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
	{ XVIDC_VM_2048x1080_100_I, "2048x1080@100Hz (I)", XVIDC_FR_100HZ,
		{2048, 274, 44, 274, 2640, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
	{ XVIDC_VM_2048x1080_120_I, "2048x1080@120Hz (I)", XVIDC_FR_120HZ,
		{2048, 66, 20, 66, 2200, 1,
		1080, 5, 10, 30, 1125, 5, 10, 30, 1125, 1} },
};

/****************************** Type Definitions ******************************/

/*************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* XV_SDIVID_C_ */
/** @} */
