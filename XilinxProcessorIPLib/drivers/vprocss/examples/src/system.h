/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file system.h
* @addtogroup vprocss Overview
*
* This is header for top level resource file that will initialize all system
* level peripherals
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 0.01  rc   07/07/14 First release

* </pre>
*
******************************************************************************/
#ifndef XSYSTEM_H		 /* prevent circular inclusions */
#define XSYSTEM_H		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "periph.h"
#include "xvprocss.h"

/************************** Constant Definitions *****************************/
typedef enum
{
  XSYS_VPSS_STREAM_IN = 0,
  XSYS_VPSS_STREAM_OUT
}XSys_StreamDirection;

/************************** Structure Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Exported APIs ************************************/
int XSys_Init(XPeriph  *pPeriph, XVprocSs *pVprocss);
void XSys_ReportSystemInfo(XPeriph  *pPeriph, XVprocSs *pVprocss);
int XSys_SetStreamParam(XVprocSs *pVprocss, u16 Direction, u16 Width,
			u16 Height, XVidC_FrameRate FrameRate,
			XVidC_ColorFormat cfmt, u16 IsInterlaced);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
