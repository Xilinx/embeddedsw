
#include <stdio.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "system.h"
#include "xvprocss_vdma.h"
#include "microblaze_sleep.h"

#define XVPROCSS_SW_VER  "v1.00"
#define VERBOSE_MODE     0
#define VIDEO_MONITOR_LOCK_TIMEOUT   (1000000)

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
  //Set TPG default parameters
  XPeriph_SetTpgParams(PeriphPtr,
		               1920,
		               1080,
		               XVIDC_CSF_RGB,
		               XTPG_BKGND_COLOR_BARS,
		               FALSE);

  //Set AXIS In to TPG settings
  XSys_SetStreamParam(VpssPtr,
		              XSYS_VPSS_STREAM_IN,
		              PeriphInst.TpgConfig.Width,
		              PeriphInst.TpgConfig.Height,
		              PeriphInst.TpgConfig.ColorFmt,
		              PeriphInst.TpgConfig.IsInterlaced);

  if(VpssPtr->Config.Topology == XVPROCSS_TOPOLOGY_SCALER_ONLY)
  {
	/* Only Scaling Ratio can be changed. Stream out color format
	 * must be same as stream in
	 */
    //Set AXIS Out
    XSys_SetStreamParam(VpssPtr,
		                XSYS_VPSS_STREAM_OUT,
		                3840,
		                2160,
		                PeriphInst.TpgConfig.ColorFmt,
		                FALSE);
  }
  else //FULL_FLEDGED
  {
	//Set AXIS Out
	XSys_SetStreamParam(VpssPtr,
	                    XSYS_VPSS_STREAM_OUT,
			            3840,
			            2160,
			            XVIDC_CSF_YCRCB_422,
			            FALSE);
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
    while(!Lock && Timeout)
    {
      if(XPeriph_IsVideoLocked(PeriphPtr))
      {
        xil_printf("Locked\r\n");
        Lock = TRUE;
      }
      --Timeout;
    }

    if(!Timeout)
    {
      xil_printf("\r\nTEST FAILED\r\n");
    }
    else
    {
      xil_printf("\r\nTEST PASSED\r\n");
    }
  }
  else
  {
    xil_printf("\r\nERR:: VProcss Configuration Failed. \r\n");
	xil_printf("\r\nTEST FAILED\r\n");
  }

  while(1)
  {
	 //NOP
  }

  /* Clean up DCache. For writeback caches, the disable_dcache routine
  internally does the flush and invalidate. For write through caches,
  an explicit invalidation must be performed on the entire cache. */
#if XPAR_MICROBLAZE_DCACHE_USE_WRITEBACK == 0
  Xil_DCacheInvalidate ();
#endif

  Xil_DCacheDisable ();

  /* Clean up ICache */
  Xil_ICacheInvalidate ();
  Xil_ICacheDisable ();

  return 0;
}
