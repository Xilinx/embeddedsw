
#include <stdio.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "system.h"
#include "xvprocss_vdma.h"

#define XVPROCSS_SW_VER "v2.00"
#define VERBOSE_MODE 1
#define VIDEO_MONITOR_LOCK_TIMEOUT (2000000)
#define USECASE_PAUSE_TIMEOUT (10000000)

static void check_usecase(XVprocSs *VpssPtr,
                  u16 *width_in,
				  u16 *height_in,
                  u16 *width_out,
				  u16 *height_out);

/************************** Variable Definitions *****************************/
XPeriph  PeriphInst;
XVprocSs VprocInst;

/***************************************************************************
*  This is the main thread that will do all initializations.
*  It will call configure functions for all subsystems and system level
*  peripherals
***************************************************************************/
int main(void)
{
  XPeriph *PeriphPtr;
  XVprocSs *VpssPtr;
  int status;
  u32 Timeout;
  static int Lock = FALSE;

  u16 width_in = 1920;
  u16 height_in = 1080;
  XVidC_ColorFormat Cformat_in = XVIDC_CSF_YCRCB_444;
  u16 Pattern = XTPG_BKGND_COLOR_BARS;
  u16 IsInterlaced = FALSE;

  u16 width_out = 1920;
  u16 height_out= 1080;
  XVidC_ColorFormat Cformat_out = XVIDC_CSF_YCRCB_444;

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
  xil_printf("  (c) 2015 by Xilinx Inc.\r\n");
  xil_printf("--------------------------------------------------------\r\n");

  status = XSys_Init(PeriphPtr, VpssPtr);
  if(status != XST_SUCCESS)
  {
	 xil_printf("CRITICAL ERR:: System Init Failed. Cannot recover from this error. Check HW\n\r");
  }

  /* Based on the customized Video Processing Subsystem functionality
   * an appropriate video input and output formats are chosen.
   */

  switch (VpssPtr->Config.Topology) {
    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
      // In Scaler-only mode only the picture size may change

      // Choose video format based on the "422 Enabled" option
      // Video In: 1920x1080_60_P Video Out: 3840x2160_60_P
      width_in = 1280;
      height_in = 720;
      Cformat_in = XV_HscalerIs422Enabled(VpssPtr->HscalerPtr)?
                   XVIDC_CSF_YCRCB_422:
                   XVIDC_CSF_YCRCB_444;
      IsInterlaced = FALSE;
      width_out = 1920;
      height_out = 1080;
      Cformat_out = Cformat_in;
      break;

    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
      // Full Fledged mode may deinterlace, change picture size and/or color format

      // In the Full Fledged configuration, the presence of a sub-core
      //   is indicated by a non-NULL pointer to the sub-core driver instance.

      // If there is no Deinterlacer OR if 420 input is supported,
      //   choose progressive 420 input format
      if ((VpssPtr->DeintPtr      == NULL) ||
          (VpssPtr->VcrsmplrInPtr != NULL)) {
      // Video In: 720P 420  Video Out: 1080P RGB
        width_in = 1280;
        height_in = 720;
        Cformat_in = XVIDC_CSF_YCRCB_420;
        IsInterlaced = FALSE;

      // If the Deinterlacer is present AND 422 input is supported,
      //   choose 480i interlaced input format, 422 or 444
      } else {
      // Video In: 480i 422  Video Out: 1080P RGB
        width_in = 720;
        height_in = 240;
        Cformat_in = (VpssPtr->HcrsmplrPtr != NULL)?
			XVIDC_CSF_YCRCB_422 : XVIDC_CSF_YCRCB_444;
        IsInterlaced = TRUE;
	  }
      width_out = 1920;
      height_out = 1080;
      Cformat_out = XVIDC_CSF_RGB;

      break;

    case XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY:
      // In this mode only deinterlacing may be done
      // Video In: 1920x1080_60_I 422  Video Out: 1920x1080_60_P 422
      width_in = 1920;
      height_in = 540;
      Cformat_in = XVIDC_CSF_YCRCB_422;
      IsInterlaced = TRUE;
      width_out = width_in;
      height_out = height_in * 2;
      Cformat_out = Cformat_in;
      break;

    case XVPROCSS_TOPOLOGY_CSC_ONLY:
      // In CSC-only mode only the color format may change
      // Video In: 1920x1080_60_P 444  Video Out: 1920x1080_60_P RGB
      width_in = 1920;
      height_in = 1080;
      Cformat_in = XVIDC_CSF_YCRCB_444;
      IsInterlaced = FALSE;
      width_out = width_in;
      height_out = height_in;
      Cformat_out = XVIDC_CSF_RGB;
      break;

    case XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY:
      // In this mode only vertical chroma resampling may be done
      // Video In: 1920x1080_60_P 420  Video Out: 1920x1080_60_P 422
      width_in = 1920;
      height_in = 1080;
      Cformat_in = XVIDC_CSF_YCRCB_420;
      IsInterlaced = FALSE;
      width_out = width_in;
      height_out = height_in;
      Cformat_out = XVIDC_CSF_YCRCB_422;
      break;

    case XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY:
      // In this mode only horizontal chroma resampling may be done
      // Video In: 1920x1080_60_P 422  Video Out: 1920x1080_60_P 444
      width_in = 1920;
      height_in = 1080;
      Cformat_in = XVIDC_CSF_YCRCB_422;
      IsInterlaced = FALSE;
      width_out = width_in;
      height_out = height_in;
      Cformat_out = XVIDC_CSF_YCRCB_444;
      break;

    default:
      break;
  }

  // depending on HW config, optionally modify the in/out formats
  check_usecase(VpssPtr,
                &width_in, &height_in,
				&width_out, &height_out);

    //Set Test Pattern Generator parameters
  printf ("\r\nConfigure TPG.\r\n");
  XPeriph_SetTpgParams(PeriphPtr,
                       width_in,
                       height_in,
                       Cformat_in,
                       Pattern,
                       IsInterlaced);

  //Set Video Input AXI Stream to TPG settings
  //Note that framerate is hardwired to 60Hz in the example design
  XSys_SetStreamParam(VpssPtr,
		              XSYS_VPSS_STREAM_IN,
		              PeriphInst.TpgConfig.Width,
		              PeriphInst.TpgConfig.Height,
		              PeriphInst.TpgConfig.ColorFmt,
		              PeriphInst.TpgConfig.IsInterlaced);

  //Set Video Output AXI Stream
  // Note that output video is always progressive
  XSys_SetStreamParam(VpssPtr,
                      XSYS_VPSS_STREAM_OUT,
                      width_out,
                      height_out,
                      Cformat_out,
                      FALSE);

  // Disable the Video input, then reset VPSS IP block
  // (done only for single-IP VPSS cases)
  if (XVprocSs_IsConfigModeCscOnly(VpssPtr)          ||
      XVprocSs_IsConfigModeDeinterlaceOnly (VpssPtr) ||
      XVprocSs_IsConfigModeHCResampleOnly(VpssPtr)   ||
      XVprocSs_IsConfigModeVCResampleOnly(VpssPtr)
     ) {
    XPeriph_DisableVidIn(PeriphPtr);
    //XPeriph_ResetHlsIp(PeriphPtr); // wip - reset problem
    XPeriph_EnableVidIn(PeriphPtr);
  }

  // Configure and Start the VPSS IP
  // (reset logic for multi-IP VPSS cases is done in this routine)
  status = XVprocSs_SetSubsystemConfig(VpssPtr);

  // wip - reset problem.................................
  // Enable the Video input
  // (done only for single-IP VPSS cases)
  //if (XVprocSs_IsConfigModeCscOnly(VpssPtr)          ||
  //    XVprocSs_IsConfigModeDeinterlaceOnly (VpssPtr) ||
  //    XVprocSs_IsConfigModeHCResampleOnly(VpssPtr)   ||
  //    XVprocSs_IsConfigModeVCResampleOnly(VpssPtr)
  //   ) {
  //  XPeriph_EnableVidIn(PeriphPtr);
  //}

  //Query video processing subsystem configuration
  XVprocSs_ReportSubsystemConfig(VpssPtr);

  if(status == XST_SUCCESS)
  {
    //Configure VTC with output timing
	XPeriph_ConfigVtc(PeriphPtr,
			          &VpssPtr->VidOut,
			          VprocInst.Config.PixPerClock);

    //Config TPG for AXIS In
    XPeriph_ConfigTpg(PeriphPtr);

    /* vtc is running at 9Mhz essentially providing < 2fps frame rate
     * Need to wait for 3-4 frames (~2sec) for vidout to acquire lock
     */
    xil_printf("\r\nWaiting for output to lock: ");
    MB_Sleep(2000);

    /* check for output lock */
    Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;
    while(!Lock && Timeout) {
      if(XPeriph_IsVideoLocked(PeriphPtr)) {
        xil_printf("Locked\r\n");
        Lock = TRUE;
      }
      --Timeout;
    }

    if(!Timeout) {
      xil_printf("\r\nTEST FAILED\r\n");
    } else {
      xil_printf("\r\nTEST PASSED\r\n");
    }
  } else {
    xil_printf("\r\nERR:: VProcss Configuration Failed. \r\n");
	xil_printf("\r\nTEST FAILED\r\n");
  }

#if VERBOSE_MODE
  XVprocSs_LogDisplay(VpssPtr);
#endif

  while(1) {
	 //NOP
  }

  return 0;
}

static void check_usecase(XVprocSs *VpssPtr,
            u16 *width_in, u16 *height_in,
            u16 *width_out, u16 *height_out)
{
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
