/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file main.c
*
* This is main file for the Video Processing Subsystem example design.
*
* The VPSS HW configuration is detected and several use cases are selected to
* test it.  These VPSS HW characteristics are configurable:
*  Topology:             6 cases - Full fledged, Scaler-only, ...
*  Pixels/clock:         3 cases - 1, 2, 4
*  Component/Pixel:      1 case - 3
*  Data Width/Component: 4 cases - 8, 10, 12, 16
*  Interlaced input:     2 cases - Allowed, Not allowed
*  Allow color formats:  3 cases - (RGB,444,422,420), (RGB,444,422), (RGB,444)
*  Max Width:  range is  64...3840
*  Max Height: range is  64...2160
*
* The video pipeline in the Example Design HW consists of the Test Pattern
* Generator driving the VPSS input.  The VPSS output is checked for video lock.
*
* On start-up the program reads the HW config and initializes its internal
* data structures.  Based on the HW capabilities, 2 use cases are selected.
* Testing a use case is done by:
*  1) Select an appropriate video input format, and program the Test Pattern
*     Generator and the VPSS input stage for this format.
*  2) Select an appropriate video output format, and program the VPSS output
*     stage and the Video Timing Controller for that format.
*  3) Start the HW, and poll the Lock status bit waiting for video Lock.
*     If you get Lock, the test reports "PASSED".  If there is no Lock
*     after several seconds the test reports "FAILED"
*  4) Optionally, go back and set up the next use case, repeating steps 1,2,3.
*
******************************************************************************/

#include <stdio.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "system.h"
#include "xvprocss_vdma.h"

/************************** Local Constants *********************************/
#define XVPROCSS_SW_VER "v2.00"
#define VERBOSE_MODE 0
#define TOPOLOGY_COUNT 6
#define USECASE_COUNT 2
#define VIDEO_MONITOR_LOCK_TIMEOUT (2000000)

/************************** Local Typedefs **********************************/
typedef struct {
  u16 width_in;
  u16 height_in;
  XVidC_ColorFormat Cformat_in;
  u16 Pattern;
  u16 IsInterlaced;

  u16 width_out;
  u16 height_out;
  XVidC_ColorFormat Cformat_out;
} vpssVideo;

/************************** Local Routines **********************************/
static void check_usecase(XVprocSs *VpssPtr, vpssVideo *useCase);

static int setup_video_io(
                 XPeriph *PeriphPtr, XVprocSs *VpssPtr, vpssVideo *useCase);

static int start_system(XPeriph *PeriphPtr, XVprocSs *VpssPtr);

/************************** Variable Definitions *****************************/
XPeriph  PeriphInst;
XVprocSs VprocInst;
const char topo_name[XVPROCSS_TOPOLOGY_NUM_SUPPORTED][32]={
        "Scaler-only",
        "Full",
        "Deint-only",
        "Csc-only",
        "Vcr-only",
        "Hcr-only"};

vpssVideo useCase[TOPOLOGY_COUNT][USECASE_COUNT] =
  //scaler only
  {{{1280,  720, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_444                             },
    {1280,  720, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
      720,  480, XVIDC_CSF_YCRCB_420                             }},
  //full fledged
   {{ 720,  240, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, TRUE,
     1920, 1080, XVIDC_CSF_RGB                                   },
    {1280,  720, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_420                             }},
  //deinterlacer only
   {{1920,  540, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, TRUE,
     1920, 1080, XVIDC_CSF_YCRCB_444                             },
    { 720,  240, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, TRUE,
      720,  480, XVIDC_CSF_YCRCB_444                             }},
   //color space conversion only
   {{1920, 1080, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_RGB                                   },
    {1280,  720, XVIDC_CSF_RGB, XTPG_BKGND_COLOR_BARS,       FALSE,
     1280,  720, XVIDC_CSF_YCRCB_444                             }},
   //vertical chroma resample only
   {{1920, 1080, XVIDC_CSF_YCRCB_420, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_422                             },
    {1920, 1080, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_420                             }},
   //horizontal chroma resample only
   {{1920, 1080, XVIDC_CSF_YCRCB_422, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_444                             },
    {1920, 1080, XVIDC_CSF_YCRCB_444, XTPG_BKGND_COLOR_BARS, FALSE,
     1920, 1080, XVIDC_CSF_YCRCB_422                             }}};

int main(void)
{
  XPeriph *PeriphPtr;
  XVprocSs *VpssPtr;

  vpssVideo *thisCase;
  int status, cnt;
  u32 Timeout;
  static int Lock = FALSE;

  /* Bind instance pointer with definition */
  PeriphPtr = &PeriphInst;
  VpssPtr   = &VprocInst;

  /* Initialize ICache */
  Xil_ICacheInvalidate();
  Xil_ICacheEnable();

  /* Initialize DCache */
  Xil_DCacheInvalidate();
  Xil_DCacheEnable();

  xil_printf("\r\n--------------------------------------------------------\r\n");
  xil_printf("  Video Processing Subsystem Example Design %s\r\n", XVPROCSS_SW_VER);
  xil_printf("  (c) 2015, 2016 by Xilinx Inc.\r\n");

  status = XSys_Init(PeriphPtr, VpssPtr);
  if(status != XST_SUCCESS) {
     xil_printf("CRITICAL ERROR:: System Init Failed. Cannot recover from this error. Check HW\n\r");
  }

  /* Based on the customized Video Processing Subsystem functionality
   * the video input and output formats are chosen.
   */
  cnt = 0;
  while (cnt < USECASE_COUNT) {
    xil_printf("--------------------------------------------------------\r\n");
    printf("Topology is %s, case %d\r\n",topo_name[VpssPtr->Config.Topology],cnt+1);

    thisCase = &useCase[VpssPtr->Config.Topology][cnt];

    switch (VpssPtr->Config.Topology) {
      case XVPROCSS_TOPOLOGY_SCALER_ONLY:
        // Choose video format based on the "422, 420, and CSC Enabled" option
        // Video In: 720P Video Out: 1080P
        thisCase->Cformat_in = XV_HscalerIs420Enabled(VpssPtr->HscalerPtr)?
                     XVIDC_CSF_YCRCB_420 :
                     (XV_HscalerIs422Enabled(VpssPtr->HscalerPtr) ? XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444);
        thisCase->Cformat_out = XV_HscalerIsCscEnabled(VpssPtr->HscalerPtr)? XVIDC_CSF_RGB : XVIDC_CSF_YCRCB_444;
        break;

      case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
        // Full Fledged mode may deinterlace, change picture size and/or color format.
        // In the Full Fledged configuration, the presence of a sub-core
        //   is indicated by a non-NULL pointer to the sub-core driver instance.
	//Check if FULL topology has any sub-cores excluded
	if((VpssPtr->DeintPtr      == NULL) ||  //No interlaced support
	   (VpssPtr->VcrsmplrInPtr == NULL) ||  //No 420 support
		   (VpssPtr->HcrsmplrPtr   == NULL)) {  //No 422 support

          //Overwrite default test case with specific ones
          // If there is no Deinterlacer AND 420 input is supported (Vcr present),
          //   choose progressive 420 input format
          if ((VpssPtr->DeintPtr      == NULL) &&
              (VpssPtr->VcrsmplrInPtr != NULL)) {
          // Video In: 720P 420  Video Out: 1080P RGB
            thisCase->width_in = 1280;
            thisCase->height_in = 720;
            thisCase->Cformat_in = XVIDC_CSF_YCRCB_420;
            thisCase->IsInterlaced = FALSE;

          // If the Deinterlacer is present,
          //   choose 480i interlaced input 422 (Hcr present) or 444 (Hcr absent)
          } else {
            if (VpssPtr->DeintPtr != NULL) {
            // Video In: 480i YUV  Video Out: 1080P RGB
              thisCase->width_in = 720;
              thisCase->height_in = 240;
              thisCase->Cformat_in = (VpssPtr->HcrsmplrPtr != NULL)?
                XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444;
              thisCase->IsInterlaced = TRUE;
            }
          }
	} else {
	  //NOP - use default test cases

	}
        break;

      default:
        break;
    }

    printf ("Set up Video Input and Output streams.\r\n");
    status = setup_video_io(PeriphPtr, VpssPtr, thisCase);
    if (status != XST_SUCCESS) {
	xil_printf ("Failed to setup io\r\n");
	goto INFINITE_LOOP;
    }

    printf ("Start VPSS.\r\n");
    status = start_system(PeriphPtr, VpssPtr);

    //Query video processing subsystem configuration
    XVprocSs_ReportSubsystemConfig(VpssPtr);

    if(status == XST_SUCCESS)
    {
      //Configure and start VTC with output timing
      printf ("Start VTC.\r\n");
      XPeriph_ConfigVtc(PeriphPtr,
                      &VpssPtr->VidOut,
                      VprocInst.Config.PixPerClock);

      //Configure and start the TPG
      printf ("Start TPG.\r\n");
      XPeriph_ConfigTpg(PeriphPtr);

      /* check for output lock */
      xil_printf("Waiting for lock... ");
      Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;
      while(!Lock && Timeout) {
        if(XPeriph_IsVideoLocked(PeriphPtr)) {
          xil_printf("Locked.\r\n");
          Lock = TRUE;
        }
        --Timeout;
      }
      if(!Timeout) {
        xil_printf("\r\nERROR:: Test Failed\r\n");
      } else {
        xil_printf("\r\nTest Completed Successfully\r\n\r\n");
      }

      xil_printf("Stop... ");
      XVprocSs_Stop(VpssPtr);

      // In the Deint-only configuration, it is necessary to allow
      // some time for aximm traffic to stop, and the core to become idle
      if (XVprocSs_IsConfigModeDeinterlaceOnly(VpssPtr)) {
        if (XV_DeintWaitForIdle(VpssPtr->DeintPtr) == XST_SUCCESS)
          xil_printf ("Deint subcore IDLE.\r\n");
      else
          xil_printf ("ERROR:: Deint subcore NOT IDLE.\r\n");
      }

    } else {
        xil_printf("\r\nERROR:: Test Failed\r\n");
        xil_printf("    ->VProcss Configuration Failed. \r\n");
    }

#if VERBOSE_MODE
  XVprocSs_LogDisplay(VpssPtr);
#endif

    xil_printf ("End testing this use case.\r\n");
    Lock = FALSE;
    cnt++;
  }

INFINITE_LOOP:
  while(1) {
    //NOP
  }

  return 0;
}

/*****************************************************************************/
/**
*
* @local routine setup_video_io()
*
*  1) Program the TPG and the VPSS input stage to the input video format.
*  2) Program the VPSS output stage to the output video format.
*
*  @return Returns XST_SUCCESS, if stream configured properly,
*  	   otherwise XST_FAILURE.
*
******************************************************************************/

static int setup_video_io(XPeriph *PeriphPtr, XVprocSs *VpssPtr,
			   vpssVideo *useCase)
{
	int status = XST_FAILURE;

	/* depending on HW config, optionally modify the in/out formats */
	check_usecase(VpssPtr, useCase);

	/*
	 * Test Pattern Generator is the video source for the example design
	 * Set Test Pattern Generator parameters
	 */
	XPeriph_SetTpgParams(PeriphPtr, useCase->width_in, useCase->height_in,
			     useCase->Cformat_in, useCase->Pattern,
			     useCase->IsInterlaced);

	/*
	 * Set VPSS Video Input AXI Stream to match the TPG
	 * Note that framerate is hardwired to 60Hz in the example design
	 */
	status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_IN,
				     PeriphInst.TpgConfig.Width,
				     PeriphInst.TpgConfig.Height, XVIDC_FR_60HZ,
				     PeriphInst.TpgConfig.ColorFmt,
				     PeriphInst.TpgConfig.IsInterlaced);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Set VPSS Video Output AXI Stream
	 * Note that output video is always progressive
	 */
	status = XSys_SetStreamParam(VpssPtr, XSYS_VPSS_STREAM_OUT,
				     useCase->width_out, useCase->height_out,
				     XVIDC_FR_60HZ, useCase->Cformat_out,
				     FALSE);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	return status;
}

/*****************************************************************************/
/**
*
* @local routine start_system()
*
*  Configure and Start the video system.
*
*  @return Returns XST_SUCCESS for successful configuration, else XST_FAILURE.
*
******************************************************************************/
static int start_system(XPeriph *PeriphPtr, XVprocSs *VpssPtr)
{
    int status;
      // For single-IP VPSS cases only, reset is handled outside vpss
      if (XVprocSs_IsConfigModeCscOnly(VpssPtr)          ||
          XVprocSs_IsConfigModeDeinterlaceOnly (VpssPtr) ||
          XVprocSs_IsConfigModeHCResampleOnly(VpssPtr)   ||
          XVprocSs_IsConfigModeVCResampleOnly(VpssPtr)
         ) {
        XPeriph_ResetHlsIp(PeriphPtr);
      }

      // Configure and Start the VPSS IP
      // (reset logic for multi-IP VPSS cases is done here)
      status = XVprocSs_SetSubsystemConfig(VpssPtr);

      return status;
}

/*****************************************************************************/
/**
*
* @local routine check_usecase()
*
*  Confine the useCase to the Height and Width restrictions of the HW.
*  Height and Width data in proposed useCase are altered if necessary.
*
*  @return Returns void.
*
******************************************************************************/
static void check_usecase(XVprocSs *VpssPtr, vpssVideo *useCase)
{
  u16 *width_in   = &useCase->width_in;
  u16 *width_out  = &useCase->width_out;
  u16 *height_in  = &useCase->height_in;
  u16 *height_out = &useCase->height_out;

    // check/correct Max Width
    if(*width_in > VpssPtr->Config.MaxWidth)
      *width_in = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_in % VpssPtr->Config.PixPerClock) != 0)
    *width_in -= (*width_in % VpssPtr->Config.PixPerClock);

  // check/correct Max Width
  if(*width_out > VpssPtr->Config.MaxWidth)
    *width_out = VpssPtr->Config.MaxWidth;

  // check/correct Width divisible by pix/clk
  if((*width_out % VpssPtr->Config.PixPerClock) != 0)
    *width_out -= (*width_out % VpssPtr->Config.PixPerClock);

  // check/correct Max Height
  if(*height_in > VpssPtr->Config.MaxHeight)
    *height_in = VpssPtr->Config.MaxHeight;

  // check/correct Max Height
  if(*height_out > VpssPtr->Config.MaxHeight)
    *height_out = VpssPtr->Config.MaxHeight;
}
