/******************************************************************************
*
* (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file system.h
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
void XSys_ReportSystemInfo(XPeriph  *pPeriph,
		                   XVprocSs *pVprocss);

void XSys_SetStreamParam(XVprocSs *pVprocss,
		                 u16 Direction,
		                 u16 Width,
		                 u16 Height,
		                 XVidC_ColorFormat cfmt,
		                 u16 IsInterlaced);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
