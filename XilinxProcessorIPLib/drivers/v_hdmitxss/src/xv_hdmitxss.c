/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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
* @file xv_hdmitxss.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmitxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xenv.h"
#if defined(__MICROBLAZE__)
#include "microblaze_sleep.h"
#elif defined(__arm__)
#include "sleep.h"
#endif
#include "xv_hdmitxss.h"
#include "xv_hdmitxss_coreinit.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
  XTmrCtr HdcpTimer;
  XHdcp1x Hdcp;
  XV_HdmiTx HdmiTx;
  XVtc Vtc;
}XV_HdmiTxSs_SubCores;

/**************************** Local Global ***********************************/
XV_HdmiTxSs_SubCores XV_HdmiTxSs_SubCoreRepo[XPAR_XV_HDMITXSS_NUM_INSTANCES];
				/**< Define Driver instance of all sub-core
                                    included in the design */


/************************** Function Prototypes ******************************/
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr,
	u16 DevId);
static XV_HdmiTxSs_SubCores *XV_HdmiTxSs_GetSubSysStruct(void *SubCorePtr);
static void XV_HdmiTxSs_WaitUs(XV_HdmiTxSs *InstancePtr, u32 MicroSeconds);
static int XV_HdmiTxSs_RegisterSubsysCallbacks(XV_HdmiTxSs *InstancePtr);
static int XV_HdmiTxSs_VtcSetup(XVtc *XVtcPtr, XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendAviInfoframe(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendGeneralControlPacket(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiTxSs_VsCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamUpCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamDownCallback(void *CallbackRef);
static u32 XV_HdmiTxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
	u32 ClockFrequency);

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Definition ******************************/


/*****************************************************************************/
/**
* This function reports list of cores included in Video Processing Subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_ReportCoreInfo(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->HDMI TX Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if (InstancePtr->HdmiTxPtr) {
    xil_printf("    : HDMI TX \r\n");
  }

  if (InstancePtr->VtcPtr) {
    xil_printf("    : VTC Core \r\n");
  }

  if (InstancePtr->HdcpPtr) {
    xil_printf("    : HDCP TX \r\n");
  }

  if (InstancePtr->HdcpTimerPtr) {
    xil_printf("    : HDCP: AXIS Timer\r\n");
  }
}

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the XV_HdmiTxSs
 * driver.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	CallbackFunc is the address to the callback function.
 * @param	CallbackRef is the user data item (microseconds to delay) that
 *		will be passed to the custom sleep/delay function when it is
 *		invoked.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XV_HdmiTxSs_SetUserTimerHandler(XV_HdmiTxSs *InstancePtr,
			XVidC_DelayHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->UserTimerWaitUs = CallbackFunc;
	InstancePtr->UserTimerPtr = CallbackRef;
}

/******************************************************************************/
/**
 * This function is the delay/sleep function for the XV_HdmiTxSs driver. For the Zynq
 * family, there exists native sleep functionality. For MicroBlaze however,
 * there does not exist such functionality. In the MicroBlaze case, the default
 * method for delaying is to use a predetermined amount of loop iterations. This
 * method is prone to inaccuracy and dependent on system configuration; for
 * greater accuracy, the user may supply their own delay/sleep handler, pointed
 * to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
 * hardware timer is used.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XV_HdmiTxSs_WaitUs(XV_HdmiTxSs *InstancePtr, u32 MicroSeconds)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (MicroSeconds == 0) {
		return;
	}

#if defined(__MICROBLAZE__)
	if (InstancePtr->UserTimerWaitUs != NULL) {
		/* Use the timer handler specified by the user for better
		 * accuracy. */
		InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
	}
	else {
		/* MicroBlaze sleep only has millisecond accuracy. Round up. */
		u32 MilliSeconds = (MicroSeconds + 999) / 1000;
		MB_Sleep(MilliSeconds);
	}
#elif defined(__arm__)
	/* Wait the requested amount of time. */
	usleep(MicroSeconds);
#endif
}

///*****************************************************************************/
///**
//* This function registers the user defined delay/sleep function with subsystem
//*
//* @param  InstancePtr is a pointer to the Subsystem instance
//* @param  waitmsec is the function pointer to the user defined delay function
//* @param  pTimer is the pointer to timer instance used by the delay function
//*
//* @return None
//*
//******************************************************************************/
//void XV_HdmiTxSs_RegisterDelayHandler(XV_HdmiTxSs *InstancePtr,
//                                   XVidC_DelayHandler WaitMsec,
//                                   void *TimerPtr)
//{
//  Xil_AssertVoid(InstancePtr != NULL);
//
//  InstancePtr->UsrDelaymsec = WaitMsec;
//  InstancePtr->UsrTmrPtr    = TimerPtr;
//}

/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDMI TX
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_HdmiTxIntrHandler(XV_HdmiTxSs *InstancePtr)
{
	XV_HdmiTx_IntrHandler(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_HdcpIntrHandler(XV_HdmiTxSs *InstancePtr)
{
	XHdcp1x_CipherIntrHandler(InstancePtr->HdcpPtr);
}

/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP Timer
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_HdcpTimerIntrHandler(XV_HdmiTxSs *InstancePtr)
{
	XTmrCtr_InterruptHandler(InstancePtr->HdcpTimerPtr);
}

/*****************************************************************************/
/**
 * This function register's all sub-core ISR's with interrupt controller and
 * any subsystem level call back function with requisite sub-core
 *
 * @param  InstancePtr is a pointer to the Subsystem instance to be
 *       worked on.
 *
 *****************************************************************************/
static int XV_HdmiTxSs_RegisterSubsysCallbacks(XV_HdmiTxSs *InstancePtr)
{
  XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;
  int Status;

  /** Register HDMI Tx ISR */
  if (HdmiTxSsPtr->HdmiTxPtr) {
	/*
	 * Register call back for Tx Core Interrupts.
	 */
	XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
	                      XV_HDMITX_HANDLER_CONNECT,
	                      XV_HdmiTxSs_ConnectCallback,
			              InstancePtr);

	XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
			              XV_HDMITX_HANDLER_VS,
			              XV_HdmiTxSs_VsCallback,
			              InstancePtr);

	XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
			              XV_HDMITX_HANDLER_STREAM_UP,
			              XV_HdmiTxSs_StreamUpCallback,
			              InstancePtr);

	XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
			              XV_HDMITX_HANDLER_STREAM_DOWN,
			              XV_HdmiTxSs_StreamDownCallback,
			              InstancePtr);
  }

  /** Register HDCP TX Timer ISR */
  if (HdmiTxSsPtr->HdcpTimerPtr) {
  }

  /** Register HDCP TX ISR */
  if (HdmiTxSsPtr->HdcpPtr) {
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr, u16 DevId)
{
  HdmiTxSsPtr->HdmiTxPtr     = ((HdmiTxSsPtr->Config.HdmiTx.IsPresent)    \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].HdmiTx) : NULL);
  HdmiTxSsPtr->VtcPtr        = ((HdmiTxSsPtr->Config.Vtc.IsPresent)  \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Vtc) : NULL);
  HdmiTxSsPtr->HdcpPtr       = ((HdmiTxSsPtr->Config.Hdcp.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Hdcp) : NULL);
  HdmiTxSsPtr->HdcpTimerPtr  = ((HdmiTxSsPtr->Config.HdcpTimer.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
}

/*****************************************************************************/
/**
* This function searches for the XV_HdmiTxSs_SubCores pointer
*
* @param  SubCorePtr address of reference subcore
*
* @return Pointer to XV_HdmiTxSs_SubCoreRepo
*
******************************************************************************/
static XV_HdmiTxSs_SubCores *XV_HdmiTxSs_GetSubSysStruct(void *SubCorePtr)
{
	int i;

	for (i=0; i<XPAR_XV_HDMITXSS_NUM_INSTANCES;i++){
		if (&XV_HdmiTxSs_SubCoreRepo[i].HdmiTx == SubCorePtr){
			return &XV_HdmiTxSs_SubCoreRepo[i];
		}

		if (&XV_HdmiTxSs_SubCoreRepo[i].Vtc == SubCorePtr){
			return &XV_HdmiTxSs_SubCoreRepo[i];
		}

		if (&XV_HdmiTxSs_SubCoreRepo[i].Hdcp == SubCorePtr){
			return &XV_HdmiTxSs_SubCoreRepo[i];
		}

		if (&XV_HdmiTxSs_SubCoreRepo[i].HdcpTimer == SubCorePtr){
			return &XV_HdmiTxSs_SubCoreRepo[i];
		}
	}
	return (NULL);
}

/*****************************************************************************/
/**
* This function initializes the video subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CfgPtr points to the configuration structure associated with the
*         subsystem instance.
* @param  EffectiveAddr is the base address of the device. If address
*         translation is being used, then this parameter must reflect the
*         virtual base address. Otherwise, the physical address should be
*         used.
*
* @return XST_SUCCESS if initialization is successful else XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_CfgInitialize(XV_HdmiTxSs *InstancePtr,
	XV_HdmiTxSs_Config *CfgPtr,
    u32 EffectiveAddr)
{
  XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiTxSsPtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

  /* Setup the instance */
  memcpy((void *)&(HdmiTxSsPtr->Config), (const void *)CfgPtr,
	sizeof(XV_HdmiTxSs_Config));
  HdmiTxSsPtr->Config.BaseAddress = EffectiveAddr;

  /* Determine sub-cores included in the provided instance of subsystem */
  XV_HdmiTxSs_GetIncludedSubcores(HdmiTxSsPtr, CfgPtr->DeviceId);

  /* Initialize all included sub_cores */
  if (HdmiTxSsPtr->HdcpTimerPtr) {
	if (XV_HdmiTxSs_SubcoreInitHdcpTimer(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->HdcpPtr) {
	if (XV_HdmiTxSs_SubcoreInitHdcp(HdmiTxSsPtr) != XST_SUCCESS){
	  return(XST_FAILURE);
	}
  }

  if (HdmiTxSsPtr->HdmiTxPtr) {
	if (XV_HdmiTxSs_SubcoreInitHdmiTx(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->VtcPtr) {
	if (XV_HdmiTxSs_SubcoreInitVtc(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  /* Register Callbacks */
  XV_HdmiTxSs_RegisterSubsysCallbacks(HdmiTxSsPtr);

  /* Reset the hardware and set the flag to indicate the
     subsystem is ready
   */
  XV_HdmiTxSs_Reset(HdmiTxSsPtr);
  HdmiTxSsPtr->IsReady = XIL_COMPONENT_IS_READY;

  return(XST_SUCCESS);
}

/****************************************************************************/
/**
* This function starts the HDMI TX subsystem including all sub-cores that are
* included in the processing pipeline for a given use-case. Video pipe is
* started from back to front
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
* @note Cores are started only if the corresponding start flag in the scratch
*       pad memory is set. This allows to selectively start only those cores
*       included in the processing chain
******************************************************************************/
void XV_HdmiTxSs_Start(XV_HdmiTxSs *InstancePtr)
{
//  u8 *pStartCore;

  Xil_AssertVoid(InstancePtr != NULL);

//  pStartCore = &InstancePtr->idata.startCore[0];
  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Start HDMI TX Subsystem.... \r\n");

}

/*****************************************************************************/
/**
* This function stops the HDMI TX subsystem including all sub-cores
* Stop the video pipe starting from front to back
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_Stop(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Stop HDMI TX Subsystem.... \r\n");

  /* Disable VTC */
  XVtc_DisableGenerator(InstancePtr->VtcPtr);
}

/*****************************************************************************/
/**
* This function resets the video subsystem sub-cores. There are 2 reset
* networks within the subsystem
*  - For cores that are on AXIS interface
*  - For cores that are on AXI-MM interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_Reset(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Reset HDMI TX Subsystem.... \r\n");

  XVtc_Reset(InstancePtr->VtcPtr);
}

/*****************************************************************************/
/**
*
* This function configures Video Timing Controller (VTC).
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiTxSs_VtcSetup(XVtc *XVtcPtr, XV_HdmiTx *HdmiTxPtr)
{
  /* Polarity configuration */
  XVtc_Polarity Polarity;
  XVtc_SourceSelect SourceSelect;
  XVtc_Timing VideoTiming;
  u32 HdmiTx_Hblank;
  u32 Vtc_Hblank;

  /* Disable Generator */
  XVtc_Reset(XVtcPtr);
  XVtc_DisableGenerator(XVtcPtr);
  XVtc_Disable(XVtcPtr);

  /* Set up source select */
  memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

  /* 1 = Generator registers, 0 = Detector registers */
  SourceSelect.VChromaSrc = 1;
  SourceSelect.VActiveSrc = 1;
  SourceSelect.VBackPorchSrc = 1;
  SourceSelect.VSyncSrc = 1;
  SourceSelect.VFrontPorchSrc = 1;
  SourceSelect.VTotalSrc = 1;
  SourceSelect.HActiveSrc = 1;
  SourceSelect.HBackPorchSrc = 1;
  SourceSelect.HSyncSrc = 1;
  SourceSelect.HFrontPorchSrc = 1;
  SourceSelect.HTotalSrc = 1;

  XVtc_SetSource(XVtcPtr, &SourceSelect);

  VideoTiming.HActiveVideo = HdmiTxPtr->Stream.Video.Timing.HActive;
  VideoTiming.HFrontPorch = HdmiTxPtr->Stream.Video.Timing.HFrontPorch;
  VideoTiming.HSyncWidth = HdmiTxPtr->Stream.Video.Timing.HSyncWidth;
  VideoTiming.HBackPorch = HdmiTxPtr->Stream.Video.Timing.HBackPorch;
  VideoTiming.HSyncPolarity = HdmiTxPtr->Stream.Video.Timing.HSyncPolarity;

  /* Vertical Timing */
  VideoTiming.VActiveVideo = HdmiTxPtr->Stream.Video.Timing.VActive;

  VideoTiming.V0FrontPorch = HdmiTxPtr->Stream.Video.Timing.F0PVFrontPorch;
  VideoTiming.V0BackPorch = HdmiTxPtr->Stream.Video.Timing.F0PVBackPorch;
  VideoTiming.V0SyncWidth = HdmiTxPtr->Stream.Video.Timing.F0PVSyncWidth;

  VideoTiming.V1FrontPorch = HdmiTxPtr->Stream.Video.Timing.F1VFrontPorch;
  VideoTiming.V1SyncWidth = HdmiTxPtr->Stream.Video.Timing.F1VSyncWidth;
  VideoTiming.V1BackPorch = HdmiTxPtr->Stream.Video.Timing.F1VBackPorch;

  VideoTiming.VSyncPolarity = HdmiTxPtr->Stream.Video.Timing.VSyncPolarity;

  VideoTiming.Interlaced = HdmiTxPtr->Stream.Video.IsInterlaced;

	/* 4 pixels per clock */
	if (HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
	  VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/4;
	  VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/4;
	  VideoTiming.HBackPorch = VideoTiming.HBackPorch/4;
	  VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/4;
	}

	/* 2 pixels per clock */
	else if (HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
	  VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
	  VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
	  VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
	  VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
	}

	/* 1 pixels per clock */
	else {
	  VideoTiming.HActiveVideo = VideoTiming.HActiveVideo;
	  VideoTiming.HFrontPorch = VideoTiming.HFrontPorch;
	  VideoTiming.HBackPorch = VideoTiming.HBackPorch;
	  VideoTiming.HSyncWidth = VideoTiming.HSyncWidth;
	}

    /* For YUV420 the line width is double there for double the blanking */
    if (HdmiTxPtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
	  VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
	  VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
	  VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
	  VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
    }

/** When compensating the vtc horizontal timing parameters for the pixel mode
* (quad or dual) rounding errors might be introduced (due to the divide)
* If this happens, the vtc total horizontal blanking is less than the hdmi tx
* horizontal blanking.
* As a result the hdmi tx vid out bridge is not able to lock to
* the incoming video stream.
* This process will check the horizontal blank timing and compensate
* for this condition.
* Calculate hdmi tx horizontal blanking */

  HdmiTx_Hblank = HdmiTxPtr->Stream.Video.Timing.HFrontPorch +
	HdmiTxPtr->Stream.Video.Timing.HSyncWidth +
	HdmiTxPtr->Stream.Video.Timing.HBackPorch;

  do {
    // Calculate vtc horizontal blanking
    Vtc_Hblank = VideoTiming.HFrontPorch +
		VideoTiming.HBackPorch +
		VideoTiming.HSyncWidth;

    // Quad pixel mode
    if (HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
      Vtc_Hblank *= 4;
    }

    // Dual pixel mode
    else if (HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
      Vtc_Hblank *= 2;
    }

	// Single pixel mode
    else {
      //Vtc_Hblank *= 1;
    }

    /* For YUV420 the line width is double there for double the blanking */
    if (HdmiTxPtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
	    Vtc_Hblank *= 2;
    }

    // If the horizontal total blanking differs,
    // then increment the Vtc horizontal front porch.
    if (Vtc_Hblank != HdmiTx_Hblank) {
      VideoTiming.HFrontPorch++;
    }

  } while (Vtc_Hblank < HdmiTx_Hblank);

  if (Vtc_Hblank != HdmiTx_Hblank) {
	  xil_printf("Error! Current format with total Hblank (%d) cannot \r\n",
				HdmiTx_Hblank);
	  xil_printf("       be transmitted with pixels per clock = %d\r\n",
				HdmiTxPtr->Stream.Video.PixPerClk);
	  return (XST_FAILURE);
  }

  XVtc_SetGeneratorTiming(XVtcPtr, &VideoTiming);

  /* Set up Polarity of all outputs */
  memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
  Polarity.ActiveChromaPol = 1;
  Polarity.ActiveVideoPol = 1;

  //Polarity.FieldIdPol = 0;
  if (VideoTiming.Interlaced) {
    Polarity.FieldIdPol = 1;
  }
  else {
    Polarity.FieldIdPol = 0;
  }

  Polarity.VBlankPol = VideoTiming.VSyncPolarity;
  Polarity.VSyncPol = VideoTiming.VSyncPolarity;
  Polarity.HBlankPol = VideoTiming.HSyncPolarity;
  Polarity.HSyncPol = VideoTiming.HSyncPolarity;

  XVtc_SetPolarity(XVtcPtr, &Polarity);

  /* VTC driver does not take care of the setting of the VTC in
   * interlaced operation. As a work around the register
   * is set manually */
  if (VideoTiming.Interlaced) {
    /* Interlaced mode */
    XVtc_WriteReg(XVtcPtr->Config.BaseAddress, 0x68, 0x42);
  }
  else {
    /* Progressive mode */
    XVtc_WriteReg(XVtcPtr->Config.BaseAddress, 0x68, 0x2);
  }

  /* Enable generator module */
  XVtc_Enable(XVtcPtr);
  XVtc_EnableGenerator(XVtcPtr);
  XVtc_RegUpdateEnable(XVtcPtr);

  return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is called when a TX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ConnectCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  // Is the cable connected?
  if (XV_HdmiTx_IsStreamConnected(HdmiTxSsPtr->HdmiTxPtr)) {
    HdmiTxSsPtr->IsStreamConnected = (TRUE);
	xil_printf("TX cable is connected\n\r");
  }

  // TX cable is disconnected
  else {
    HdmiTxSsPtr->IsStreamConnected = (FALSE);
    xil_printf("TX cable is disconnected\n\r");
  }

  // Check if user callback has been registered
  if (HdmiTxSsPtr->ConnectCallback) {
	  HdmiTxSsPtr->ConnectCallback(HdmiTxSsPtr->ConnectRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when a TX vsync has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_VsCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  // AVI infoframe
  XV_HdmiTxSs_SendAviInfoframe(HdmiTxSsPtr->HdmiTxPtr);

  // General control packet
  XV_HdmiTxSs_SendGeneralControlPacket(HdmiTxSsPtr->HdmiTxPtr);

  // Check if user callback has been registered
  if (HdmiTxSsPtr->VsCallback) {
	  HdmiTxSsPtr->VsCallback(HdmiTxSsPtr->VsRef);
  }
}

/*****************************************************************************/
/**
*
* This function sends AVI info frames.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_SendAviInfoframe(XV_HdmiTx *HdmiTx)
{
  u8 Index;
  u8 Data;
  u8 Crc;

  /* Header, Packet type*/
  HdmiTx->Aux.Header.Byte[0] = 0x82;

  /* Version */
  HdmiTx->Aux.Header.Byte[1] = 0x02;

  /* Length */
  HdmiTx->Aux.Header.Byte[2] = 13;

  /* Checksum (this will be calculated by the HDMI TX IP) */
  HdmiTx->Aux.Header.Byte[3] = 0;

  /* Data */
  switch (HdmiTx->Stream.Video.ColorFormatId) {
    case XVIDC_CSF_YCRCB_422:
      Data = 1 << 5;
      break;

    case XVIDC_CSF_YCRCB_444:
      Data = 2 << 5;
      break;

    case XVIDC_CSF_YCRCB_420:
      Data = 3 << 5;
      break;

    default:
      Data = 0;
      break;
  }

  HdmiTx->Aux.Data.Byte[1] = Data;

  HdmiTx->Aux.Data.Byte[2] = 0;
  HdmiTx->Aux.Data.Byte[3] = 0;
  HdmiTx->Aux.Data.Byte[4] = HdmiTx->Stream.Vic;

  for (Index = 5; Index < 32; Index++) {
    HdmiTx->Aux.Data.Byte[Index] = 0;
  }

  /* Calculate AVI infoframe checksum */
  Crc = 0;

  /* Header */
  for (Index = 0; Index < 3; Index++) {
    Crc += HdmiTx->Aux.Header.Byte[Index];
  }

  /* Data */
  for (Index = 1; Index < 5; Index++) {
    Crc += HdmiTx->Aux.Data.Byte[Index];
  }

  Crc = 256 - Crc;

  HdmiTx->Aux.Data.Byte[0] = Crc;

  XV_HdmiTx_AuxSend(HdmiTx);
}

/*****************************************************************************/
/**
*
* This function sends the general control packet.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_SendGeneralControlPacket(XV_HdmiTx *HdmiTx)
{
  u8 Index;
  u8 Data;

  // Pre-Process SB1 data
  // Pixel Packing Phase
  switch (XV_HdmiTx_GetPixelPackingPhase(HdmiTx)) {

    case 1 :
      Data = 1;
      break;

    case 2 :
      Data = 2;
      break;

    case 3 :
      Data = 3;
      break;

    default :
      Data = 0;
      break;
  }

  /**< Shift pixel packing phase to the upper nibble */
  Data <<= 4;

  /** In HDMI the colordepth in YUV422 is always 12 bits,  although on the
  * link itself it is being transmitted as 8-bits. Therefore if the colorspace
  * is YUV422, then force the colordepth to 8 bits. */
  if (HdmiTx->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_422) {
    Data |= 0;
  }

  else {

    // Colordepth
    switch (HdmiTx->Stream.Video.ColorDepth) {

      // 10 bpc
      case XVIDC_BPC_10:
        // Color depth
        Data |= 5;
        break;

      // 12 bpc
      case XVIDC_BPC_12:
        // Color depth
        Data |= 6;
        break;

      // 16 bpc
      case XVIDC_BPC_16:
        // Color depth
        Data |= 7;
        break;

      // Not indicated
      default:
        Data = 0;
        break;
    }
  }

  // Packet type
  HdmiTx->Aux.Header.Byte[0] = 0x3;

  // Reserved
  HdmiTx->Aux.Header.Byte[1] = 0;

  // Reserved
  HdmiTx->Aux.Header.Byte[2] = 0;

  // Checksum (this will be calculated by the HDMI TX IP)
  HdmiTx->Aux.Header.Byte[3] = 0;

  // Data
  // The packet contains four identical subpackets
  for (Index = 0; Index < 4; Index++) {
    // SB0
    HdmiTx->Aux.Data.Byte[(Index*8)] = 0;

    // SB1
    HdmiTx->Aux.Data.Byte[(Index*8)+1] = Data;

    // SB2
    HdmiTx->Aux.Data.Byte[(Index*8)+2] = 0;

    // SB3
    HdmiTx->Aux.Data.Byte[(Index*8)+3] = 0;

    // SB4
    HdmiTx->Aux.Data.Byte[(Index*8)+4] = 0;

    // SB5
    HdmiTx->Aux.Data.Byte[(Index*8)+5] = 0;

    // SB6
    HdmiTx->Aux.Data.Byte[(Index*8)+6] = 0;

    // SB ECC
    HdmiTx->Aux.Data.Byte[(Index*8)+7] = 0;

  }

  XV_HdmiTx_AuxSend(HdmiTx);
}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_StreamUpCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  // Check if user callback has been registered
  if (HdmiTxSsPtr->StreamUpCallback) {
	  HdmiTxSsPtr->StreamUpCallback(HdmiTxSsPtr->StreamUpRef);
  }

  /* Set TX sample rate */
  XV_HdmiTx_SetSampleRate(HdmiTxSsPtr->HdmiTxPtr, HdmiTxSsPtr->SamplingRate);

  /* Release HDMI TX reset */
  XV_HdmiTx_Reset(HdmiTxSsPtr->HdmiTxPtr, FALSE);

  /* Setup VTC */
  XV_HdmiTxSs_VtcSetup(HdmiTxSsPtr->VtcPtr, HdmiTxSsPtr->HdmiTxPtr);

  if (HdmiTxSsPtr->AudioEnabled) {
	  /* HDMI TX unmute audio */
	  HdmiTxSsPtr->AudioMute = (FALSE);
	  XV_HdmiTx_AudioUnmute(HdmiTxSsPtr->HdmiTxPtr);
  }


  if (HdmiTxSsPtr->HdcpPtr) {
	  /* Set the TX HDCP state to up */
	  XHdcp1x_SetPhysicalState(HdmiTxSsPtr->HdcpPtr, TRUE);
  }

}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_StreamDownCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  // Assert HDMI TX reset
  XV_HdmiTx_Reset(HdmiTxSsPtr->HdmiTxPtr, TRUE);

  if (HdmiTxSsPtr->HdcpPtr) {
	  /* Set the TX HDCP state to down */
	  XHdcp1x_SetPhysicalState(HdmiTxSsPtr->HdcpPtr, FALSE);
  }

  // Check if user callback has been registered
  if (HdmiTxSsPtr->StreamDownCallback) {
	  HdmiTxSsPtr->StreamDownCallback(HdmiTxSsPtr->StreamDownRef);
  }

}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                     Callback Function Type
* -----------------------         ---------------------------------------------
* (XV_HDMITXSS_HANDLER_CONNECT)       HpdCallback
* (XV_HDMITXSS_HANDLER_VS)            VsCallback
* (XV_HDMITXSS_HANDLER_STREAM_DOWN)   SreamDownCallback
* (XV_HDMITXSS_HANDLER_STREAM_UP)     SreamUpCallback
* </pre>
*
* @param	InstancePtr is a pointer to the HDMI TX Subsystem instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
int XV_HdmiTxSs_SetCallback(XV_HdmiTxSs *InstancePtr,
	u32 HandlerType,
	void *CallbackFunc,
	void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(HandlerType >= (XV_HDMITXSS_HANDLER_CONNECT));
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Check for handler type */
	switch (HandlerType) {
		case (XV_HDMITXSS_HANDLER_CONNECT):
			InstancePtr->ConnectCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
			InstancePtr->ConnectRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		case (XV_HDMITXSS_HANDLER_VS):
			InstancePtr->VsCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
			InstancePtr->VsRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		// Stream down
		case (XV_HDMITXSS_HANDLER_STREAM_DOWN):
			InstancePtr->StreamDownCallback =
				(XV_HdmiTxSs_Callback)CallbackFunc;
			InstancePtr->StreamDownRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		// Stream up
		case (XV_HDMITXSS_HANDLER_STREAM_UP):
			InstancePtr->StreamUpCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
			InstancePtr->StreamUpRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		default:
			Status = (XST_INVALID_PARAM);
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function reads the HDMI Sink EDID.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_ReadEdid(XV_HdmiTxSs *InstancePtr, u8 *Buffer)
{
    u32 Status;

    // Default
    Status = (XST_FAILURE);

    // Check if a sink is connected
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      *Buffer = 0x00;   // Offset zero
      Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x50, 1, Buffer,
		(FALSE));

      // Check if write was successful
      if (Status == (XST_SUCCESS)) {
        // Read edid
        Status = XV_HdmiTx_DdcRead(InstancePtr->HdmiTxPtr, 0x50, 256, Buffer,
			(TRUE));
      }
    }

    else {
      print("No sink is connected.\n\r");
      print("Please connect a HDMI sink.\n\r");
    }
  return Status;
}

/*****************************************************************************/
/**
*
* This function shows the HDMI source edid.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr)
{
    u8 Buffer[256];
    u8 Row;
    u8 Column;
    u8 Valid;
    u32 Status;
    u8 EdidManName[4];

    // Check if a sink is connected
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      // Default
      Valid = (FALSE);

      // Read TX edid
      Status = XV_HdmiTxSs_ReadEdid(InstancePtr, (u8*)&Buffer);

      // Check if read was successful
      if (Status == (XST_SUCCESS)) {

        print("\n\r");

        XVidC_EdidGetManName(&Buffer[0], (char *) EdidManName);
        xil_printf("Manufacturer name : %S\n\r", EdidManName);

        print("\n\rRaw data\n\r");
        print("----------------------------------------------------\n\r");
        for (Row = 0; Row < 16; Row++) {
          xil_printf("%02X : ", (Row*16));
          for (Column = 0; Column < 16; Column++) {
            xil_printf("%02X ", Buffer[(Row*16)+Column]);
          }
        print("\n\r");
        }
        Valid = (TRUE);
      }

      if (!Valid) {
        print("Error reading EDID\n\r");
      }
    }

    else {
      print("No sink is connected.\n\r");
      print("Please connect a HDMI sink.\n\r");
    }
}


/*****************************************************************************/
/**
*
* This function starts the HDMI TX stream
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_StreamStart(XV_HdmiTxSs *InstancePtr)
{
  // Set TX pixel rate
  XV_HdmiTx_SetPixelRate(InstancePtr->HdmiTxPtr);

  // Set TX color depth
  XV_HdmiTx_SetColorDepth(InstancePtr->HdmiTxPtr);

  // Set TX color format
  XV_HdmiTx_SetColorFormat(InstancePtr->HdmiTxPtr);

  // Set TX scrambler
  XV_HdmiTx_Scrambler(InstancePtr->HdmiTxPtr);

  // Set TX clock ratio
  XV_HdmiTx_ClockRatio(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function sends audio info frames.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SendAuxInfoframe(XV_HdmiTxSs *InstancePtr, void *Aux)
{
  u8 Index;
  u8 Crc;
  XV_HdmiTx_Aux *tx_aux = Aux;

  if (Aux == (NULL)) {
      /* Header, Packet type */
	  InstancePtr->HdmiTxPtr->Aux.Header.Byte[0] = 0x84;

	  /* Version */
	  InstancePtr->HdmiTxPtr->Aux.Header.Byte[1] = 0x01;

	  /* Length */
	  InstancePtr->HdmiTxPtr->Aux.Header.Byte[2] = 10;

	  /* Checksum (this will be calculated by the HDMI TX IP) */
	  InstancePtr->HdmiTxPtr->Aux.Header.Byte[3] = 0;

	  /* 2 Channel count. Audio coding type refer to stream */
	  InstancePtr->HdmiTxPtr->Aux.Data.Byte[1] = 0x1;

	  for (Index = 2; Index < 32; Index++) {
		InstancePtr->HdmiTxPtr->Aux.Data.Byte[Index] = 0;
	  }

	  /* Calculate AVI infoframe checksum */
	  Crc = 0;

	  /* Header */
	  for (Index = 0; Index < 3; Index++) {
		Crc += InstancePtr->HdmiTxPtr->Aux.Header.Byte[Index];
	  }

	  /* Data */
	  for (Index = 1; Index < 5; Index++) {
		Crc += InstancePtr->HdmiTxPtr->Aux.Data.Byte[Index];
	  }

	  Crc = 256 - Crc;
	  InstancePtr->HdmiTxPtr->Aux.Data.Byte[0] = Crc;

	  XV_HdmiTx_AuxSend(InstancePtr->HdmiTxPtr);

  }

  else {
	  // Copy Audio Infoframe
	  if (InstancePtr->HdmiTxPtr->Aux.Header.Byte[0] == 0x84) {
		// Header
		InstancePtr->HdmiTxPtr->Aux.Header.Data = tx_aux->Header.Data;

		// Data
		for (Index = 0; Index < 8; Index++) {
		  InstancePtr->HdmiTxPtr->Aux.Data.Data[Index] =
			tx_aux->Data.Data[Index];
		}
	  }
  }

  /* Send packet */
  XV_HdmiTx_AuxSend(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS number of active audio channels
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  AudioChannels
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetAudioChannels(XV_HdmiTxSs *InstancePtr, u8 AudioChannels)
{
	InstancePtr->AudioChannels = AudioChannels;
}

/*****************************************************************************/
/**
*
* This function set HDMI TX audio parameters
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_AudioMute(XV_HdmiTxSs *InstancePtr, u8 Enable)
{
  //Audio Mute Mode
  if (Enable){
    XV_HdmiTx_AudioMute(InstancePtr->HdmiTxPtr);
  }
  else{
    XV_HdmiTx_AudioUnmute(InstancePtr->HdmiTxPtr);
  }
}

/*****************************************************************************/
/**
*
* This function set HDMI TX susbsystem stream parameters
*
* @param  None.
*
* @return Calculated TMDS Clock
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_SetStream(XV_HdmiTxSs *InstancePtr,
		XVidC_VideoMode VideoMode, XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc)
{
  u32 TmdsClock = 0;
  u32 Loop;
  u8 ReachedEnd = 0;

  if (VideoMode < XVIDC_VM_NUM_SUPPORTED) {
	Loop = 0;
    do
    {
      switch (Loop) {
        case 0 :
          TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr, VideoMode,
			ColorFormat, Bpc, InstancePtr->Config.Ppc);
          break;

        // Try YCrCb 4:2:0 Colorspace at 4 pixels per clock
        case 1 :
          TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr, VideoMode,
			XVIDC_CSF_YCRCB_420,  XVIDC_BPC_8, XVIDC_PPC_2);
          break;

        default :
          ReachedEnd = (TRUE);
      }

      if (ReachedEnd)
        break;

      Loop++;
    }
    while (TmdsClock == 0);
  }

  // Default 1080p colorbar
  else {
    TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr,
		XVIDC_VM_1920x1080_60_P, XVIDC_CSF_RGB, XVIDC_BPC_8, XVIDC_PPC_2);
  }

  return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function set HDMI TX susbsystem stream reduced blanking parameters
*
* @param  None.
*
* @return Calculated TMDS Clock
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_SetStreamReducedBlanking(XV_HdmiTxSs *InstancePtr)
{
  u32 TmdsClock = 0;

  TmdsClock = XV_HdmiTx_SetStreamReducedBlanking(InstancePtr->HdmiTxPtr);

  return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_HdmiTxSs_GetVideoStream(XV_HdmiTxSs *InstancePtr)
{
	return (&InstancePtr->HdmiTxPtr->Stream.Video);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStream(XV_HdmiTxSs *InstancePtr,
									XVidC_VideoStream VidStream)
{
	InstancePtr->HdmiTxPtr->Stream.Video = VidStream;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  SamplingRate Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetSamplingRate(XV_HdmiTxSs *InstancePtr, u8 SamplingRate)
{
	InstancePtr->SamplingRate = SamplingRate;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  InstancePtr VIC Flag Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoIDCode(XV_HdmiTxSs *InstancePtr, u8 Vic)
{
	InstancePtr->HdmiTxPtr->Stream.Vic = Vic;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  InstancePtr VIC Value 1:HDMI 0:DVI
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStreamType(XV_HdmiTxSs *InstancePtr, u8 StreamType)
{
	InstancePtr->HdmiTxPtr->Stream.IsHdmi = StreamType;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  IsScrambled 1:IsScrambled 0: not Scrambled
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStreamScramblingFlag(XV_HdmiTxSs *InstancePtr,
															u8 IsScrambled)
{
	InstancePtr->HdmiTxPtr->Stream.IsScrambled = IsScrambled;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS TMDS Cock Ratio
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  Ratio 0 - 1/10, 1 - 1/40
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetTmdsClockRatio(XV_HdmiTxSs *InstancePtr, u8 Ratio)
{
	InstancePtr->HdmiTxPtr->Stream.TMDSClockRatio = Ratio;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  SamplingRate Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_GetTmdsClockFreqHz(XV_HdmiTxSs *InstancePtr)
{
	return (InstancePtr->HdmiTxPtr->Stream.TMDSClock);
}

/*****************************************************************************/
/**
*
* This function detects connected sink is a HDMI 2.0/HDMI 1.4 sink device
*
* @param	InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return
*		- XST_SUCCESS if HDMI 2.0
*		- XST_FAILURE if HDMI 1.4
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_DetectHdmi20(XV_HdmiTxSs *InstancePtr)
{
	  return (XV_HdmiTx_DetectHdmi20(InstancePtr->HdmiTxPtr));
}

/*****************************************************************************/
/**
*
* This function is called when HDMI TX SS TMDS clock changes
*
* @param  None.
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_RefClockChangeInit(XV_HdmiTxSs *InstancePtr)
{
	  /* Assert HDMI TX reset */
	  XV_HdmiTx_Reset(InstancePtr->HdmiTxPtr, TRUE);

	  /* Clear variables */
	  XV_HdmiTx_Clear(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS timing information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ReportTiming(XV_HdmiTxSs *InstancePtr)
{
	  XV_HdmiTx_DebugInfo(InstancePtr->HdmiTxPtr);
	  xil_printf("Scrambled: %0d\n\r",
		(XV_HdmiTx_IsStreamScrambled(InstancePtr->HdmiTxPtr)));
	  xil_printf("Sample rate: %0d\n\r",
		(XV_HdmiTx_GetSampleRate(InstancePtr->HdmiTxPtr)));
	  xil_printf("Audio channels: %0d\n\r",
		(XV_HdmiTx_GetAudioChannels(InstancePtr->HdmiTxPtr)));
	  print("\r\n");

}

/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS subcore versions
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ReportSubcoreVersion(XV_HdmiTxSs *InstancePtr)
{
  u32 Data;

  if (InstancePtr->HdmiTxPtr) {
	 Data = XV_HdmiTx_GetVersion(InstancePtr->HdmiTxPtr);
	 xil_printf("  HDMI TX version : %02d.%02d (%04x)\n\r",
		((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

  if (InstancePtr->VtcPtr){
	 Data = XVtc_GetVersion(InstancePtr->VtcPtr);
	 xil_printf("  VTC version     : %02d.%02d (%04x)\n\r",
		((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

  if (InstancePtr->HdcpPtr){
	 Data = XHdcp1x_GetVersion(InstancePtr->HdcpPtr);
	 xil_printf("  HDCP TX version : %02d.%02d (%04x)\n\r",
		((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }
}

/*****************************************************************************/
/**
*
* This function enables the hdcp module
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
void XV_HdmiTxSs_HdcpEnable(XV_HdmiTxSs *InstancePtr, u8 Enable)
{
	XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;

	if (Enable)	{
		/* Enable hdcp interface */
		XHdcp1x_Enable(HdmiTxSsPtr->HdcpPtr);

		/* Set the TX HDCP state to up */
		XHdcp1x_SetPhysicalState(HdmiTxSsPtr->HdcpPtr, TRUE);
	}
	else {
	    /* Set the TX HDCP state to down */
	    XHdcp1x_SetPhysicalState(HdmiTxSsPtr->HdcpPtr, FALSE);

		/* Disable hdcp interface */
		XHdcp1x_Disable(HdmiTxSsPtr->HdcpPtr);
	}
}

/*****************************************************************************/
/**
*
* This function polls the hdcp example module and determines if the video
* stream is encrypted. The traffic is encrypted if the encryption bit map
* is non-zero and the interface is authenticated.
*
* @return	Truth value indicating encrypted (TRUE) or not (FALSE).
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
u8 XV_HdmiTxSs_HdcpPoll(XV_HdmiTxSs *InstancePtr)
{
	/* Locals */
	XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;

	/* Poll hdcp interface */
	XHdcp1x_Poll(HdmiTxSsPtr->HdcpPtr);

	/* Return */
	return (XHdcp1x_IsEncrypted(HdmiTxSsPtr->HdcpPtr));
}


/*****************************************************************************/
/**
*
* This function initiates HDCP authentication and enable encryption
*
* @return Truth value indicating encryption was enabled (TRUE) or not (FALSE).
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
u8 XV_HdmiTxSs_HdcpStart(XV_HdmiTxSs *InstancePtr)
{
	/* Locals */
	XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;
	u8 retval = FALSE;

	if (XHdcp1x_TxIsAuthenticated(HdmiTxSsPtr->HdcpPtr)==0)
	{
		XHdcp1x_Authenticate(HdmiTxSsPtr->HdcpPtr);
		retval = FALSE;
	}
	else
	{
		XHdcp1x_EnableEncryption(HdmiTxSsPtr->HdcpPtr,0x1);
		retval = TRUE;
	}

	return retval;
}

/******************************************************************************/
/**
*
* This function converts from microseconds to timer ticks
*
* @param TimeoutInUs  the timeout to convert
* @param ClockFrequency  the clock frequency to use in the conversion
*
* @return
*   The number of "ticks"
*
* @note
*   None.
*
******************************************************************************/
static u32 XV_HdmiTxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
													u32 ClockFrequency)
{
	u32 TimeoutFreq = 0;
	u32 NumTicks = 0;

	/* Check for greater than one second */
	if (TimeoutInUs > 1000000ul) {
		u32 NumSeconds = 0;

		/* Determine theNumSeconds */
		NumSeconds = (TimeoutInUs/1000000ul);

		/* Update theNumTicks */
		NumTicks = (NumSeconds*ClockFrequency);

		/* Adjust theTimeoutInUs */
		TimeoutInUs -= (NumSeconds*1000000ul);
	}

	/* Convert TimeoutFreq to a frequency */
	TimeoutFreq  = 1000;
	TimeoutFreq *= 1000;
	TimeoutFreq /= TimeoutInUs;

	/* Update NumTicks */
	NumTicks += ((ClockFrequency / TimeoutFreq) + 1);

	return (NumTicks);
}

/*****************************************************************************/
/**
*
* This function serves as the timer callback function
*
* @param CallBackRef  the callback reference value
* @param TimerChannel  the channel within the timer that expired
*
* @return
*   void
*
* @note
*   None
*
******************************************************************************/
void XV_HdmiTxSs_HdcpTimerCallback(void* CallBackRef, u8 TimerChannel)
{
	XHdcp1x* HdcpPtr = CallBackRef;

	XHdcp1x_HandleTimeout(HdcpPtr);
	return;
}

/******************************************************************************/
/**
*
* This function starts a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
* @param TimeoutInMs  the timer duration in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpTimerStart(const XHdcp1x* InstancePtr, u16 TimeoutInMs)
{
	XV_HdmiTxSs_SubCores *SubCorePtr;
	XTmrCtr *TimerPtr;
	u8 TimerChannel = 0;
	u32 TimerOptions = 0;
	u32 NumTicks = 0;

	SubCorePtr = XV_HdmiTxSs_GetSubSysStruct((void*)InstancePtr);
	TimerPtr = &SubCorePtr->HdcpTimer;

	/* Determine NumTicks */
	NumTicks = XV_HdmiTxSs_HdcpTimerConvUsToTicks((TimeoutInMs*1000ul),
												XPAR_CPU_CORE_CLOCK_FREQ_HZ);

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	/* Configure the callback */
	XTmrCtr_SetHandler(TimerPtr, &XV_HdmiTxSs_HdcpTimerCallback,
												(void*) InstancePtr);

	/* Configure the timer options */
	TimerOptions  = XTmrCtr_GetOptions(TimerPtr, TimerChannel);
	TimerOptions |=  XTC_DOWN_COUNT_OPTION;
	TimerOptions |=  XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TimerPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TimerPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TimerPtr, TimerChannel);

	return (XST_SUCCESS);
}


/******************************************************************************/
/**
*
* This function stops a timer on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpTimerStop(const XHdcp1x* InstancePtr)
{
	XV_HdmiTxSs_SubCores *SubCorePtr;
	XTmrCtr *TimerPtr;
	u8 TimerChannel = 0;

	SubCorePtr = XV_HdmiTxSs_GetSubSysStruct((void*)InstancePtr);
	TimerPtr = &SubCorePtr->HdcpTimer;

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	return (XST_SUCCESS);
}


/******************************************************************************/
/**
*
* This function busy waits for an interval on behalf of an hdcp interface
*
* @param InstancePtr  the hdcp interface
* @param DelayInMs  the delay duration in milliseconds
*
* @return
*   XST_SUCCESS if successful
*
* @note
*   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpTimerBusyDelay(const XHdcp1x* InstancePtr, u16 DelayInMs)
{
	XV_HdmiTxSs_SubCores *SubCorePtr;
	XTmrCtr *TimerPtr;
	u8 TimerChannel = 0;
	u32 TimerOptions = 0;
	u32 NumTicks = 0;

	SubCorePtr = XV_HdmiTxSs_GetSubSysStruct((void*)InstancePtr);
	TimerPtr = &SubCorePtr->HdcpTimer;

	/* Determine NumTicks */
	NumTicks = XV_HdmiTxSs_HdcpTimerConvUsToTicks((DelayInMs*1000ul),
											XPAR_CPU_CORE_CLOCK_FREQ_HZ);

	/* Stop it */
	XTmrCtr_Stop(TimerPtr, TimerChannel);

	/* Configure the timer options */
	TimerOptions  = XTmrCtr_GetOptions(TimerPtr, TimerChannel);
	TimerOptions |=  XTC_DOWN_COUNT_OPTION;
	TimerOptions &= ~XTC_INT_MODE_OPTION;
	TimerOptions &= ~XTC_AUTO_RELOAD_OPTION;
	XTmrCtr_SetOptions(TimerPtr, TimerChannel, TimerOptions);

	/* Set the timeout and start */
	XTmrCtr_SetResetValue(TimerPtr, TimerChannel, NumTicks);
	XTmrCtr_Start(TimerPtr, TimerChannel);

	/* Wait until done */
	while (!XTmrCtr_IsExpired(TimerPtr, TimerChannel));

	return (XST_SUCCESS);
}
