/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_default.h
*
* This is the default header file which contains definitions for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/12/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLM_DEFAULT_H
#define XPLM_DEFAULT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
/* BSP Hdrs */
#include "xil_io.h"
#include "xil_types.h"
#include "mb_interface.h"
#include "xparameters.h"

/* XPLMI headers */
#include "xplmi_debug.h"
#include "xplmi_cdo.h"
#include "xplmi_config.h"
#include "xplmi_hw.h"
#include "xplmi_task.h"
#include "xplmi_status.h"
#include "xplmi.h"

/* library headers */
#include "xpm_device.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLM_DEFAULT_H */
