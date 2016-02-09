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
* @file system.c
*
* This is top level resource file that will initialize all system level
* peripherals
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- --------  -------------------------------------------------------
* 0.01  rc   07/07/14  First release
* 1.00  dmc  01/25/16  Initialize the VPSS Event Logging system in XSys_Init()
*
* </pre>
*
******************************************************************************/
#include "xparameters.h"
#include "system.h"

/************************** Constant Definitions *****************************/
#define USR_FRAME_BUF_BASEADDR     (XPAR_MIG7SERIES_0_BASEADDR + (0x20000000))

/**************************** Type Definitions *******************************/


/**************************** Local Global *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/
/*****************************************************************************/
/**
*
* This function is the system level initialization routine. It in turn calls
* each of the included subsystem initialization function
*
* @param	pPeriph is a pointer to the peripherals instance
* @param	pVprocss is a pointer to the video proc subsystem instance
*
* @return
*		- XST_SUCCESS if init. was successful.
*		- XST_FAILURE if init. was unsuccessful.
*
* @note		None.
*
******************************************************************************/
int XSys_Init(XPeriph  *pPeriph, XVprocSs *pVprocss)
{
  int status;
  XVprocSs_Config *VprocSsConfigPtr;

  Xil_AssertNonvoid(pPeriph  != NULL);
  Xil_AssertNonvoid(pVprocss != NULL);

  //Init all instance variables to 0
  memset(pPeriph,  0, sizeof(XPeriph));
  memset(pVprocss, 0, sizeof(XVprocSs));

  status = XPeriph_PowerOnInit(pPeriph);
  if(status != XST_SUCCESS)
  {
	 xil_printf("ERR:: System Peripheral Init. error\n\r");
	 return(XST_FAILURE);
  }

  VprocSsConfigPtr = XVprocSs_LookupConfig(XPAR_V_PROC_SS_0_DEVICE_ID);
  if(VprocSsConfigPtr == NULL)
  {
	xil_printf("ERR:: VprocSs device not found\r\n");
    return (XST_DEVICE_NOT_FOUND);
  }

  switch (VprocSsConfigPtr->Topology)
  {
    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
    case XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY:
	  XVprocSs_SetFrameBufBaseaddr(pVprocss, USR_FRAME_BUF_BASEADDR);
      break;

    default:
      break;
  }

  /* Start capturing event log. */
  XVprocSs_LogReset(pVprocss);

  status = XVprocSs_CfgInitialize(pVprocss,
		                          VprocSsConfigPtr,
		                          VprocSsConfigPtr->BaseAddress);

  if(status != XST_SUCCESS)
  {
	 xil_printf("ERR:: Video Processing Subsystem Init. error\n\r");
	 return(XST_FAILURE);
  }

  return(status);
}

/*****************************************************************************/
/**
*
* This function is to set vpss stream parameters
*
* @param	pVprocss is a pointer to the video proc subsystem instance
* @param    Direction defined if parameters are to be applied to Input or
*           output stream
* @param    Width is stream width
* @param    Height is stream height
* @param    cfmt is stream color format
*
* @return   None
*
* @note		None.
*
******************************************************************************/
void XSys_SetStreamParam(XVprocSs *pVprocss,
		                 u16 Direction,
		                 u16 Width,
		                 u16 Height,
		                 XVidC_ColorFormat cfmt,
		                 u16 IsInterlaced)
{
  XVidC_VideoMode resId;
  XVidC_VideoStream Stream;
  XVidC_VideoTiming const *TimingPtr;

  resId = XVidC_GetVideoModeId(Width, Height, XVIDC_FR_60HZ, IsInterlaced);
  TimingPtr = XVidC_GetTimingInfo(resId);

  //Setup Video Processing Subsystem
  Stream.VmId           = resId;
  Stream.Timing         = *TimingPtr;
  Stream.ColorFormatId  = cfmt;
  Stream.ColorDepth     = pVprocss->Config.ColorDepth;
  Stream.PixPerClk      = pVprocss->Config.PixPerClock;
  Stream.FrameRate      = XVIDC_FR_60HZ;
  Stream.IsInterlaced   = IsInterlaced;

  if(Direction == XSYS_VPSS_STREAM_IN)
  {
    XVprocSs_SetVidStreamIn(pVprocss, &Stream);
  }
  else
  {
    XVprocSs_SetVidStreamOut(pVprocss, &Stream);
  }
}


/*****************************************************************************/
/**
*
* This function reports all the IP's included in the system design.
* It calls each subsystem API to report its child nodes
*
* @param	pPeriph is a pointer to the peripherals instance
* @param	pVprocss is a pointer to the video proc subsystem instance
*
* @return   None
*
* @note		None.
*
******************************************************************************/
void XSys_ReportSystemInfo(XPeriph  *pPeriph, XVprocSs *pVprocss)
{
  Xil_AssertVoid(pPeriph  != NULL);
  Xil_AssertVoid(pVprocss != NULL);

  xil_printf("\r\n\r\r****Reporting System Design Info****\r\n");

  //Report Peripherals found at System level
  XPeriph_ReportDeviceInfo(pPeriph);

  //Report cores in Video Processing subsystem
  XVprocSs_ReportSubsystemCoreInfo(pVprocss);
}
