/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
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
#ifdef STDOUT_BASEADDRESS
#if (STDOUT_BASEADDRESS == 0xFF000000)
#define NODE_UART XPM_DEVID_UART_0
#elif (STDOUT_BASEADDRESS == 0xFF010000)
#define NODE_UART XPM_DEVID_UART_1
#endif
#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLM_DEFAULT_H */
