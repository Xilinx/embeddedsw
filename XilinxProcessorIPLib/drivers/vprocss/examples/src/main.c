
#include <stdio.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "system.h"
#include "xvprocss_vdma.h"
#include "microblaze_sleep.h"

#define XVPROCSS_SW_VER "v2.00"
#define VERBOSE_MODE 0
#define VIDEO_MONITOR_LOCK_TIMEOUT (2000000)

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

  xil_printf("\r\nInitialize System Design...\r\n");
  status = XSys_Init(PeriphPtr, VpssPtr);
  if(status != XST_SUCCESS)
  {
	 xil_printf("CRITICAL ERR:: System Init Failed. Cannot recover from this error. Check HW\n\r");
  }

#if (VERBOSE_MODE == 1)
  xil_printf("\r\nINFO> Setting up VPSS AXIS In/Out\r\n");
#endif

  //Set Test Pattern Generator parameters
  switch (VpssPtr->Config.Topology) {
    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
      // In this mode only the picture size may change
      // Video In: 1920x1080_60_P 422  Video Out: 3840x2160_60_P 422
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   1080,
		                   XVIDC_CSF_YCRCB_422,
		                   XTPG_BKGND_COLOR_BARS,
		                   FALSE);
      break;

    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
      // This mode may deinterlace, change picture size and/or color format
      // Video In: 1920x1080_60_I 422  Video Out: 3840x2160_60_P RGB
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   540,
						   XVIDC_CSF_YCRCB_422,
		                   XTPG_BKGND_COLOR_BARS,
		                   TRUE);
      break;

    case XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY:
      // In this mode only deinterlacing may be done
      // Video In: 1920x1080_60_I 422  Video Out: 1920x1080_60_P 422
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   540,
		                   XVIDC_CSF_YCRCB_422,
		                   XTPG_BKGND_COLOR_BARS,
		                   TRUE);
      break;

    case XVPROCSS_TOPOLOGY_CSC_ONLY:
      // In this mode only the color format may change
      // Video In: 1920x1080_60_P 444  Video Out: 1920x1080_60_P RGB
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   1080,
						   XVIDC_CSF_YCRCB_444,
		                   XTPG_BKGND_COLOR_BARS,
		                   FALSE);
      break;

    case XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY:
      // In this mode only vertical chroma resampling may be done
      // Video In: 1920x1080_60_P 420  Video Out: 1920x1080_60_P 422
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   1080,
		                   XVIDC_CSF_YCRCB_420,
		                   XTPG_BKGND_COLOR_BARS,
		                   FALSE);
      break;

    case XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY:
      // In this mode only horizontal chroma resampling may be done
      // Video In: 1920x1080_60_P 422  Video Out: 1920x1080_60_P 444
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   1080,
		                   XVIDC_CSF_YCRCB_422,
		                   XTPG_BKGND_COLOR_BARS,
		                   FALSE);
      break;

    default:
	  /* Shouldn't come here */
      XPeriph_SetTpgParams(PeriphPtr,
		                   1920,
		                   1080,
		                   XVIDC_CSF_RGB,
		                   XTPG_BKGND_COLOR_BARS,
		                   FALSE);
      break;
  }

  //Set Video Input AXI Stream to TPG settings
  XSys_SetStreamParam(VpssPtr,
		              XSYS_VPSS_STREAM_IN,
		              PeriphInst.TpgConfig.Width,
		              PeriphInst.TpgConfig.Height,
		              PeriphInst.TpgConfig.ColorFmt,
		              PeriphInst.TpgConfig.IsInterlaced);

  //Set Video Output AXI Stream Out
  switch (VpssPtr->Config.Topology) {
    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
      // Video In: 1920x1080_60_P 422  Video Out: 3840x2160_60_P 422
      XSys_SetStreamParam(VpssPtr,
		                  XSYS_VPSS_STREAM_OUT,
		                  3840,
		                  2160,
		                  PeriphInst.TpgConfig.ColorFmt,
		                  FALSE);
      break;

    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
      // Video In: 1920x1080_60_I 422  Video Out: 3840x2160_60_P RGB
	  XSys_SetStreamParam(VpssPtr,
		                  XSYS_VPSS_STREAM_OUT,
				          3840,
				          2160,
						  XVIDC_CSF_RGB,
				          FALSE);
      break;

    case XVPROCSS_TOPOLOGY_DEINTERLACE_ONLY:
      // Video In: 1920x1080_60_I 422  Video Out: 1920x1080_60_P 422
	  XSys_SetStreamParam(VpssPtr,
	                      XSYS_VPSS_STREAM_OUT,
			              PeriphInst.TpgConfig.Width,
			              PeriphInst.TpgConfig.Height*2,
			              PeriphInst.TpgConfig.ColorFmt,
			              FALSE);
	  break;

    case XVPROCSS_TOPOLOGY_CSC_ONLY:
      // Video In: 1920x1080_60_P 444  Video Out: 1920x1080_60_P RGB
	  XSys_SetStreamParam(VpssPtr,
	                      XSYS_VPSS_STREAM_OUT,
			              PeriphInst.TpgConfig.Width,
			              PeriphInst.TpgConfig.Height,
						  XVIDC_CSF_RGB,
			              FALSE);
      break;

    case XVPROCSS_TOPOLOGY_VCRESAMPLE_ONLY:
	  // Video In: 1920x1080_60_P 420  Video Out: 1920x1080_60_P 422
	  XSys_SetStreamParam(VpssPtr,
	                      XSYS_VPSS_STREAM_OUT,
			              PeriphInst.TpgConfig.Width,
			              PeriphInst.TpgConfig.Height,
			              XVIDC_CSF_YCRCB_422,
			              FALSE);
      break;

    case XVPROCSS_TOPOLOGY_HCRESAMPLE_ONLY:
	  // Video In: 1920x1080_60_P 422  Video Out: 1920x1080_60_P 444
	  XSys_SetStreamParam(VpssPtr,
	                      XSYS_VPSS_STREAM_OUT,
			              PeriphInst.TpgConfig.Width,
			              PeriphInst.TpgConfig.Height,
			              XVIDC_CSF_YCRCB_444,
			              FALSE);
      break;

    default:
	  /* Shouldn't come here */
	  XSys_SetStreamParam(VpssPtr,
	                      XSYS_VPSS_STREAM_OUT,
			              PeriphInst.TpgConfig.Width,
			              PeriphInst.TpgConfig.Height,
			              PeriphInst.TpgConfig.ColorFmt,
			              FALSE);
      break;
  }

  //Configure video processing subsystem
  status = XVprocSs_SetSubsystemConfig(VpssPtr);

  //Query vpss configuration
  XVprocSs_ReportSubsystemConfig(VpssPtr);

  if(status == XST_SUCCESS)
  {
    //Configure VTC with output timing
	XPeriph_ConfigVtc(PeriphPtr,
			          &VpssPtr->VidOut,
			          VprocInst.Config.PixPerClock);

    //Config TPG for AXIS In
    XPeriph_ConfigTpg(PeriphPtr);

#if (VERBOSE_MODE == 1)
    XPeriph_TpgDbgReportStatus(PeriphPtr);
#endif

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

  while(1) {
	 //NOP
  }

  return 0;
}
