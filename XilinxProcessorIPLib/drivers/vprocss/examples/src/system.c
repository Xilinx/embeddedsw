/******************************************************************************
 *
 * (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
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
#if defined(__MICROBLAZE__)
#define DDR_BASEADDR XPAR_MIG7SERIES_0_BASEADDR
#else
#define DDR_BASEADDR XPAR_DDR_MEM_BASEADDR
#endif
#define USR_FRAME_BUF_BASEADDR     (DDR_BASEADDR + (0x20000000))

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
