/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss.c
*
* This is main code of Xilinx HDMI Receiver Subsystem device driver.
* Please see xv_hdmirxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.
* 1.1   yh     15/01/16 Added 3D Video support
* 1.2   yh     20/01/16 Added remapper support
* 1.3   yh     01/02/16 Added set_ppc api
* 1.4   yh     01/02/16 Removed xil_print "Cable (dis)connected"
* 1.5   yh     01/02/16 Removed xil_printf("Active audio channels...)
* 1.6   yh     15/02/16 Added default value to XV_HdmiRxSs_ConfigRemapper
* 1.7   MG     03/02/16 Added HDCP support
* 1.8   MG     10/02/16 Moved HDCP 2.2 reset from stream up/down callback
*                       to connect callback
* 1.9   MH     15/03/16 Added HDCP authenticated callback support
* 1.10  MH     23/04/16 1. HDCP 1.x driver now uses AXI timer 4.1, so updated
*                       to use AXI Timer config structure to determine timer
*                       clock frequency
*                       2. HDCP 1.x driver has fixed the problem where the
*                       reset for the receiver causes the entire DDC peripheral
*                       to get reset. Based on this change the driver has been
*                       updated to use XV_HdmiRxSs_HdcpReset and
*                       XV_HdmiRxSs_HdcpReset functions directly.
*                       3. Updated XV_HdmiRxSs_HdcpEnable and
*                       XV_HdmiRxSs_HdcpEnable functions to ensure that
*                       HDCP 1.4 and 2.2 are mutually exclusive.
*                       This fixes the problem where HDCP 1.4 and 2.2
*                       state machines are running simultaneously.
* 1.11  MG     13/05/16 Added DDC peripheral HDCP mode selection to XV_HdmiRxSs_HdcpEnable
* 1.12  MH     23/06/16 Added HDCP repeater support.
* 1.13  YH     18/07/16 1. Replace xil_print with xdbg_printf.
*                       2. Replace MB_Sleep() with usleep()
* 1.14  YH     25/07/16 Used UINTPTR instead of u32 for BaseAddress
*                       XV_HdmiRxSs_CfgInitialize
* 1.15  MH     26/07/16 Updates for automatic protocol switching
* 1.16  MH     05/08/16 Updates to optimize out HDCP when excluded
* 1.17  YH     17/08/16 Remove sleep in XV_HdmiRxSs_ResetRemapper
*                       squash unused variable compiler warning
*                       Added Event Log
* 1.18  MH     08/10/16 Improve HDCP 1.4 authentication
* 1.19  MG     31/10/16 Fixed issue with reference clock compensation in
*                           XV_HdmiRxSS_SetStream
* 1.20  YH     14/11/16 Added API to enable/disable YUV420/Pixel Drop Mode
*                       for video bridge
* 1.21  YH     14/11/16 Remove Remapper APIs
*                       Replace XV_HdmiRxSs_ConfigRemapper API with
*                       XV_HdmiRxSs_ConfigBridgeMode API as remapper feature is
*                           moved to video bridge and controlled by HDMI core
* 1.22  MMO    03/01/17 Add compiler option(XV_HDMIRXSS_LOG_ENABLE) to enable
*                           Log
*                       Move global variable XV_HdmiRx_VSIF VSIF to local
*                           XV_HdmiRxSs_RetrieveVSInfoframe API
*                       Move HDCP related API's to hdmirxss_hdcp.c
* 1.23  MMO    10/02/17 Added Sync Loss and HDMI/DVI Interrupt Support
*
* 1.4   YH     07/07/17 Add new log type XV_HDMIRXSS_LOG_EVT_SETSTREAM_ERR
*                       Report HDMI/DVI mode in HDMI example design info log
*
* 1.41  MMO    21/07/17 CR-979900 (Fix)
*                       Removed the HDCP Push Event API Call when the
*                       Aux Callback event happen
*       MH     09/08/17 Added function XV_HdmiRxSs_HdcpSetCapability
* 1.42  YH     06/10/17 Added function XV_HdmiRxSs_GetAudioFormat
*       EB     10/10/17 Updated function XV_HdmiRxSs_ReportAudio to report
*                           audio format
* 5.00  YH     16/11/17 Added dedicated reset for each clock domain
*              16/11/17 Added bridge overflow interrupt
*       EB     16/01/18 Added parsing of InfoFrames during AuxCallback
*                       Changed XV_HdmiRxSs_RetrieveVSInfoframe's input
*                           parameter type
*                       Added function XV_HdmiRxSs_GetAviInfoframe,
*                           XV_HdmiRxSs_GetGCP, XV_HdmiRxSs_GetAudioInfoframe,
*                           XV_HdmiRxSs_GetVSIF
*                       Updated XV_HdmiRxSs_ConfigBridgeMode so Pixel
*                           Pepetition is based on received AVI InfoFrame
*       SM     28/02/18 Added definition of XV_HdmiRxSS_SetAppVersion() API
* 5.10  MMO    06/04/18 Updated XV_HdmiRxSs_ToggleHpd and XV_HdmiRxSs_Stop
*                           for cleaner HPD flow during transition from HDMI2.0
*                           to HDMI1.4
*       YH     13/04/18 Fixed a bug in XV_HdmiRxSs_BrdgOverflowCallback
* 5.20	EB     03/08/18 Added function XV_HdmiRxSs_AudioMute
*                       Added TMDS Clock Ratio callback support
* 5.40  EB     06/08/19 Added Vic and Video Timing mismatch callback support
******************************************************************************/

/***************************** Include Files *********************************/
#include "sleep.h"
#include "xv_hdmirxss.h"
#include "xv_hdmirxss_coreinit.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
#ifdef XPAR_XHDCP_NUM_INSTANCES
  XTmrCtr HdcpTimer;
  XHdcp1x Hdcp14;
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  XHdcp22_Rx Hdcp22;
#endif
  XV_HdmiRx HdmiRx;
} XV_HdmiRxSs_SubCores;

/**************************** Local Global ***********************************/
/** Define Driver instance of all sub-core included in the design */
XV_HdmiRxSs_SubCores XV_HdmiRxSs_SubCoreRepo[XPAR_XV_HDMIRXSS_NUM_INSTANCES];

/************************** Function Prototypes ******************************/
static void XV_HdmiRxSs_GetIncludedSubcores(XV_HdmiRxSs *HdmiRxSsPtr,
    u16 DevId);
static void XV_HdmiRxSs_WaitUs(XV_HdmiRxSs *InstancePtr, u32 MicroSeconds);
static void XV_HdmiRxSs_RetrieveVSInfoframe(XV_HdmiRxSs *HdmiRxSs);
static int XV_HdmiRxSs_RegisterSubsysCallbacks(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiRxSs_BrdgOverflowCallback(void *CallbackRef);
static void XV_HdmiRxSs_AuxCallback(void *CallbackRef);
static void XV_HdmiRxSs_AudCallback(void *CallbackRef);
static void XV_HdmiRxSs_LnkStaCallback(void *CallbackRef);
static void XV_HdmiRxSs_DdcCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamDownCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamInitCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamUpCallback(void *CallbackRef);
static void XV_HdmiRxSs_SyncLossCallback(void *CallbackRef);
static void XV_HdmiRxSs_ModeCallback(void *CallbackRef);
static void XV_HdmiRxSs_TmdsClkRatioCallback(void *CallbackRef);
static void XV_HdmiRxSs_VicErrorCallback(void *CallbackRef);

static void XV_HdmiRxSs_ReportCoreInfo(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportTiming(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportLinkQuality(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportDRMInfo(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportAudio(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportInfoFrame(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportSubcoreVersion(XV_HdmiRxSs *InstancePtr);

static void XV_HdmiRxSs_ConfigBridgeMode(XV_HdmiRxSs *InstancePtr);

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This macros selects the bridge YUV420 mode
*
* @param  InstancePtr is a pointer to the HDMI RX Subsystem
*
*****************************************************************************/
#define XV_HdmiRxSs_BridgeYuv420(InstancePtr,Enable) \
{ \
    XV_HdmiRx_Bridge_yuv420(InstancePtr->HdmiRxPtr, Enable); \
} \

/*****************************************************************************/
/**
* This macros selects the bridge pixel repeat mode
*
* @param  InstancePtr is a pointer to the HDMI RX Subsystem
*
*****************************************************************************/
#define XV_HdmiRxSs_BridgePixelDrop(InstancePtr,Enable) \
{ \
    XV_HdmiRx_Bridge_pixel(InstancePtr->HdmiRxPtr, Enable); \
}

/************************** Function Definition ******************************/

void XV_HdmiRxSs_ReportInfo(XV_HdmiRxSs *InstancePtr)
{
    xil_printf("------------\r\n");
    xil_printf("HDMI RX SubSystem\r\n");
    xil_printf("------------\r\n");
    XV_HdmiRxSs_ReportCoreInfo(InstancePtr);
    XV_HdmiRxSs_ReportSubcoreVersion(InstancePtr);
    xil_printf("\r\n");
    xil_printf("HDMI RX Mode - ");
    if (InstancePtr->HdmiRxPtr->Stream.IsHdmi == (TRUE)) {
        xil_printf("HDMI\r\n");
    }
    else {
        xil_printf("DVI\r\n");
    }
    xil_printf("------------\r\n");
    xil_printf("HDMI RX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiRxSs_ReportTiming(InstancePtr);
    xil_printf("Link quality\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs_ReportLinkQuality(InstancePtr);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs_ReportAudio(InstancePtr);
    xil_printf("DRM info frame\r\n");
    xil_printf("--------------\r\n");
    XV_HdmiRxSs_ReportDRMInfo(InstancePtr);
    xil_printf("Infoframe\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs_ReportInfoFrame(InstancePtr);
    xil_printf("\r\n");
}

/*****************************************************************************/
/**
* This function reports list of cores included in Video Processing Subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
static void XV_HdmiRxSs_ReportCoreInfo(XV_HdmiRxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->HDMI RX Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if(InstancePtr->HdmiRxPtr)
  {
    xil_printf("    : HDMI RX \r\n");
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  if(InstancePtr->Hdcp14Ptr)
  {
    xil_printf("    : HDCP 1.4 RX \r\n");
  }

  if(InstancePtr->HdcpTimerPtr)
  {
    xil_printf("    : HDCP: AXIS Timer\r\n");
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  if(InstancePtr->Hdcp22Ptr)
  {
    xil_printf("    : HDCP 2.2 RX \r\n");
  }
#endif
}

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the
 *  XV_HdmiRxSs driver.
 *
 * @param   InstancePtr is a pointer to the HdmiSsRx instance.
 * @param   CallbackFunc is the address to the callback function.
 * @param   CallbackRef is the user data item (microseconds to delay) that
 *      will be passed to the custom sleep/delay function when it is
 *      invoked.
 *
 * @return  None.
 *
 * @note    None.
 *
*******************************************************************************/
void XV_HdmiRxSs_SetUserTimerHandler(XV_HdmiRxSs *InstancePtr,
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
 * This function is the delay/sleep function for the XV_HdmiRxSs driver. For the
 * Zynq family, there exists native sleep functionality. For MicroBlaze however,
 * there does not exist such functionality. In the MicroBlaze case, the default
 * method for delaying is to use a predetermined amount of loop iterations. This
 * method is prone to inaccuracy and dependent on system configuration; for
 * greater accuracy, the user may supply their own delay/sleep handler, pointed
 * to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
 * hardware timer is used.
 *
 * @param   InstancePtr is a pointer to the HdmiSsRx instance.
 * @param   MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return  None.
 *
 * @note    None.
 *
*******************************************************************************/
static void XV_HdmiRxSs_WaitUs(XV_HdmiRxSs *InstancePtr, u32 MicroSeconds)
{
    /* Verify arguments. */
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    if (MicroSeconds == 0) {
        return;
    }

    if (InstancePtr->UserTimerWaitUs != NULL) {
        /* Use the timer handler specified by the user for better
         * accuracy. */
        InstancePtr->UserTimerWaitUs(InstancePtr, MicroSeconds);
    }
    else {
        usleep(MicroSeconds);
    }
}

/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDMI RX
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS_HdmiRxIntrHandler(XV_HdmiRxSs *InstancePtr)
{
    XV_HdmiRx_IntrHandler(InstancePtr->HdmiRxPtr);
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
static int XV_HdmiRxSs_RegisterSubsysCallbacks(XV_HdmiRxSs *InstancePtr)
{
  XV_HdmiRxSs *HdmiRxSsPtr = InstancePtr;

  //Register HDMI callbacks
  if(HdmiRxSsPtr->HdmiRxPtr) {
    /*
     * Register call back for Rx Core Interrupts.
     */
    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_CONNECT,
                          (void *)XV_HdmiRxSs_ConnectCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_BRDG_OVERFLOW,
                          (void *)XV_HdmiRxSs_BrdgOverflowCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_AUX,
                          (void *)XV_HdmiRxSs_AuxCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_AUD,
                          (void *)XV_HdmiRxSs_AudCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_LNKSTA,
                          (void *)XV_HdmiRxSs_LnkStaCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_DDC,
                          (void *)XV_HdmiRxSs_DdcCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_DOWN,
                          (void *)XV_HdmiRxSs_StreamDownCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_INIT,
                          (void *)XV_HdmiRxSs_StreamInitCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_UP,
                          (void *)XV_HdmiRxSs_StreamUpCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_SYNC_LOSS,
                          (void *)XV_HdmiRxSs_SyncLossCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_MODE,
                          (void *)XV_HdmiRxSs_ModeCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_TMDS_CLK_RATIO,
                          (void *)XV_HdmiRxSs_TmdsClkRatioCallback,
                          (void *)InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_VIC_ERROR,
                          (void *)XV_HdmiRxSs_VicErrorCallback,
                          (void *)InstancePtr);
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
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void XV_HdmiRxSs_GetIncludedSubcores(XV_HdmiRxSs *HdmiRxSsPtr, u16 DevId)
{
  HdmiRxSsPtr->HdmiRxPtr   =((HdmiRxSsPtr->Config.HdmiRx.IsPresent) ?
                            (&XV_HdmiRxSs_SubCoreRepo[DevId].HdmiRx) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
  HdmiRxSsPtr->Hdcp14Ptr   =((HdmiRxSsPtr->Config.Hdcp14.IsPresent) ?
                            (&XV_HdmiRxSs_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiRxSsPtr->HdcpTimerPtr=((HdmiRxSsPtr->Config.HdcpTimer.IsPresent) ?
                            (&XV_HdmiRxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  HdmiRxSsPtr->Hdcp22Ptr   =((HdmiRxSsPtr->Config.Hdcp22.IsPresent) ?
                            (&XV_HdmiRxSs_SubCoreRepo[DevId].Hdcp22) : NULL);
#endif
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
int XV_HdmiRxSs_CfgInitialize(XV_HdmiRxSs *InstancePtr,
    XV_HdmiRxSs_Config *CfgPtr,
    UINTPTR EffectiveAddr)
{
  XV_HdmiRxSs *HdmiRxSsPtr = InstancePtr;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiRxSsPtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

  /* Setup the instance */
  memcpy((void *)&(HdmiRxSsPtr->Config), (const void *)CfgPtr,
    sizeof(XV_HdmiRxSs_Config));
  HdmiRxSsPtr->Config.BaseAddress = EffectiveAddr;

  /* Determine sub-cores included in the provided instance of subsystem */
  XV_HdmiRxSs_GetIncludedSubcores(HdmiRxSsPtr, CfgPtr->DeviceId);

  /* Initialize all included sub_cores */
  if(HdmiRxSsPtr->HdmiRxPtr)
  {
    if(XV_HdmiRxSs_SubcoreInitHdmiRx(HdmiRxSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4
  if(HdmiRxSsPtr->Hdcp14Ptr)
  {
    if(XV_HdmiRxSs_SubcoreInitHdcp14(HdmiRxSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(HdmiRxSsPtr->HdcpTimerPtr)
  {
    if(XV_HdmiRxSs_SubcoreInitHdcpTimer(HdmiRxSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  // HDCP 2.2
  if(HdmiRxSsPtr->Hdcp22Ptr)
  {
    if(XV_HdmiRxSs_SubcoreInitHdcp22(HdmiRxSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }
#endif

  /* Register Callbacks */
  XV_HdmiRxSs_RegisterSubsysCallbacks(HdmiRxSsPtr);

#ifdef USE_HDCP_RX
  /* Default value */
  HdmiRxSsPtr->HdcpIsReady = (FALSE);
  XV_HdmiRxSs_HdcpSetCapability(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_BOTH);
  HdmiRxSsPtr->UserHdcpProt = XV_HDMIRXSS_HDCP_NOUSERPREF;
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when both HDCP cores are instantiated and all keys are
     loaded */
  if (HdmiRxSsPtr->Hdcp14Ptr && HdmiRxSsPtr->Hdcp22Ptr &&
      HdmiRxSsPtr->Hdcp22Lc128Ptr && HdmiRxSsPtr->Hdcp14KeyPtr &&
      HdmiRxSsPtr->Hdcp22PrivateKeyPtr) {
    HdmiRxSsPtr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs_HdcpSetProtocol(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key is
     loaded */
  if (!HdmiRxSsPtr->HdcpIsReady && HdmiRxSsPtr->Hdcp14Ptr &&
       HdmiRxSsPtr->Hdcp14KeyPtr) {
    HdmiRxSsPtr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs_HdcpSetProtocol(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the keys are
     loaded */
  if (!HdmiRxSsPtr->HdcpIsReady && HdmiRxSsPtr->Hdcp22Ptr &&
       HdmiRxSsPtr->Hdcp22Lc128Ptr && HdmiRxSsPtr->Hdcp22PrivateKeyPtr) {
    HdmiRxSsPtr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs_HdcpSetProtocol(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_22);
  }
#endif

  /* Reset the hardware and set the flag to indicate the
     subsystem is ready
   */
  XV_HdmiRxSs_Reset(HdmiRxSsPtr);
  HdmiRxSsPtr->IsReady = XIL_COMPONENT_IS_READY;

  /* Initialize the application version with 0 <default value>.
   * Application need to set the this variable properly to let driver know
   * what version of application is being used.
   */
  HdmiRxSsPtr->AppMajVer = 0;
  HdmiRxSsPtr->AppMinVer = 0;
  DrmInfoFramePtr = XV_HdmiRxSs_GetDrmInfoframe(HdmiRxSsPtr);

  DrmInfoFramePtr->Static_Metadata_Descriptor_ID = 0xFF;
  DrmInfoFramePtr->EOTF = 0xff;

  return(XST_SUCCESS);
}

/****************************************************************************/
/**
* This function starts the HDMI RX subsystem including all sub-cores that are
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
void XV_HdmiRxSs_Start(XV_HdmiRxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_START, 0);
#endif
  /* Drive HDMI RX HPD High */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, TRUE);
}

/*****************************************************************************/
/**
* This function stops the HDMI RX subsystem including all sub-cores
* Stop the video pipe starting from front to back
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiRxSs_Stop(XV_HdmiRxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Clear SCDC variables */
  XV_HdmiRx_DdcScdcClear(InstancePtr->HdmiRxPtr);

  /* Disable the scrambler */
  XV_HdmiRx_SetScrambler(InstancePtr->HdmiRxPtr, (FALSE));

  /* Drive HDMI RX HPD Low */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (FALSE));

#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_STOP, 0);
#endif
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
void XV_HdmiRxSs_Reset(XV_HdmiRxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_RESET, 0);
#endif

  /* Assert HDMI RX core resets */
  XV_HdmiRxSs_RXCore_VRST(InstancePtr, TRUE);
  XV_HdmiRxSs_RXCore_LRST(InstancePtr, TRUE);

  /* Assert SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs_SYSRST(InstancePtr, TRUE);

  /* Release HDMI RX core resets */
  XV_HdmiRxSs_RXCore_VRST(InstancePtr, FALSE);
  XV_HdmiRxSs_RXCore_LRST(InstancePtr, FALSE);

  /* Release SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs_SYSRST(InstancePtr, FALSE);
}

/*****************************************************************************/
/**
* This function asserts or releases the Internal Video reset
* of the HDMI subcore within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiRxSs_RXCore_VRST(XV_HdmiRxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx_INT_VRST(InstancePtr->HdmiRxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the Internal Link reset
* of the HDMI subcore within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiRxSs_RXCore_LRST(XV_HdmiRxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx_INT_LRST(InstancePtr->HdmiRxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the video reset of other
* blocks within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiRxSs_VRST(XV_HdmiRxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx_EXT_VRST(InstancePtr->HdmiRxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the system reset of other
* blocks within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiRxSs_SYSRST(XV_HdmiRxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx_EXT_SYSRST(InstancePtr->HdmiRxPtr, Reset);
}

/*****************************************************************************/
/**
*
* This function is called when a Bridge overflow event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_BrdgOverflowCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: RX Bridge Overflow\r\n");

  // Check if user callback has been registered
  if (HdmiRxSsPtr->BrdgOverflowCallback) {
    HdmiRxSsPtr->BrdgOverflowCallback(HdmiRxSsPtr->BrdgOverflowRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when a RX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ConnectCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  // Is the cable connected?
  if (XV_HdmiRx_IsStreamConnected(HdmiRxSsPtr->HdmiRxPtr)) {
#ifdef XV_HDMIRXSS_LOG_ENABLE
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_CONNECT, 0);
#endif
    // Set RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, TRUE);

    // Set stream connected flag
    HdmiRxSsPtr->IsStreamConnected = (TRUE);

#ifdef USE_HDCP_RX
    // Push connect event to HDCP event queue
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_CONNECT_EVT);
#endif
  }

  // RX cable is disconnected
  else {
#ifdef XV_HDMIRXSS_LOG_ENABLE
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_DISCONNECT, 0);
#endif
    // Clear RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, FALSE);

    // Set stream connected flag
    HdmiRxSsPtr->IsStreamConnected = (FALSE);

#ifdef USE_HDCP_RX
    // Push disconnect event to HDCP event queue
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_DISCONNECT_EVT);
#endif

    XV_HdmiRx_SetScrambler(HdmiRxSsPtr->HdmiRxPtr, (FALSE)); //Disable scrambler
  }


  // Check if user callback has been registered
  if (HdmiRxSsPtr->ConnectCallback) {
    HdmiRxSsPtr->ConnectCallback(HdmiRxSsPtr->ConnectRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when a RX AUX IRQ has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_AuxCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
  XHdmiC_Aux *AuxPtr;
  XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
  XHdmiC_GeneralControlPacket *GeneralControlPacketPtr;
  XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

  AviInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(HdmiRxSsPtr);
  GeneralControlPacketPtr = XV_HdmiRxSs_GetGCP(HdmiRxSsPtr);
  AudioInfoFramePtr = XV_HdmiRxSs_GetAudioInfoframe(HdmiRxSsPtr);
  DrmInfoFramePtr = XV_HdmiRxSs_GetDrmInfoframe(HdmiRxSsPtr);
  AuxPtr = XV_HdmiRxSs_GetAuxiliary(HdmiRxSsPtr);

  if(AuxPtr->Header.Byte[0] == AUX_VSIF_TYPE){
	  // Retrieve Vendor Specific Info Frame
	  XV_HdmiRxSs_RetrieveVSInfoframe(HdmiRxSsPtr);
  } else if(AuxPtr->Header.Byte[0] == AUX_AVI_INFOFRAME_TYPE){
	  // Reset Avi InfoFrame
	  (void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	  // Parse Aux to retrieve Avi InfoFrame
	  XV_HdmiC_ParseAVIInfoFrame(AuxPtr, AviInfoFramePtr);
	  HdmiRxSsPtr->HdmiRxPtr->Stream.Video.ColorFormatId =
	  			XV_HdmiRx_GetAviColorSpace(HdmiRxSsPtr->HdmiRxPtr);
	  HdmiRxSsPtr->HdmiRxPtr->Stream.Vic =
				XV_HdmiRx_GetAviVic(HdmiRxSsPtr->HdmiRxPtr);
	  HdmiRxSsPtr->HdmiRxPtr->Stream.Video.AspectRatio =
				XV_HdmiC_IFAspectRatio_To_XVidC(HdmiRxSsPtr->AVIInfoframe.PicAspectRatio);
  } else if(AuxPtr->Header.Byte[0] == AUX_GENERAL_CONTROL_PACKET_TYPE) {
	  // Reset General Control Packet
	  (void)memset((void *)GeneralControlPacketPtr, 0, sizeof(XHdmiC_GeneralControlPacket));
	  // Parse Aux to retrieve General Control Packet
	  XV_HdmiC_ParseGCP(AuxPtr, GeneralControlPacketPtr);
	  // Stream.Video.ColorDepth is updated from the core during AUX INTR
  } else if(AuxPtr->Header.Byte[0] == AUX_AUDIO_INFOFRAME_TYPE) {
	  // Reset Audio InfoFrame
	  (void)memset((void *)AudioInfoFramePtr, 0, sizeof(XHdmiC_AudioInfoFrame));
	  // Parse Aux to retrieve Audio InfoFrame
	  XV_HdmiC_ParseAudioInfoFrame(AuxPtr, AudioInfoFramePtr);
  } else if(AuxPtr->Header.Byte[0] == AUX_DRM_INFOFRAME_TYPE) {
	  // Reset HDR InfoFrame
	  (void)memset((void *)DrmInfoFramePtr, 0, sizeof(XHdmiC_DRMInfoFrame));
	  // Parse Aux to retrieve HDR InfoFrame
	  XV_HdmiC_ParseDRMIF(AuxPtr, DrmInfoFramePtr);
  }

  // Check if user callback has been registered
  if (HdmiRxSsPtr->AuxCallback) {
      HdmiRxSsPtr->AuxCallback(HdmiRxSsPtr->AuxRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a RX Sync Loss IRQ has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_SyncLossCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  if (HdmiRxSsPtr->HdmiRxPtr->Stream.SyncStatus ==
		  	  	  	  	  	  	  	  	  	  	XV_HDMIRX_SYNCSTAT_SYNC_LOSS) {
  // Push sync loss event to HDCP event queue
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_SYNCLOSS, 0);
#endif

#ifdef USE_HDCP_RX
  XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_SYNC_LOSS_EVT);
#endif
  }
  // Sync is recovered/establish
  else if (HdmiRxSsPtr->HdmiRxPtr->Stream.SyncStatus ==
		  	  	  	  	  	  	  	  	  	  	 XV_HDMIRX_SYNCSTAT_SYNC_EST) {
	  // Push sync loss event to HDCP event queue
#ifdef XV_HDMIRXSS_LOG_ENABLE
	  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_SYNCEST, 0);
#endif

#ifdef USE_HDCP_RX
	  XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_SYNC_EST_EVT);
#endif
  }
}

/*****************************************************************************/
/**
*
* This function is called when the mode has transitioned from DVI to HDMI or
* vice versa.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ModeCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  // HDMI mode
  if (XV_HdmiRxSs_GetVideoStreamType(HdmiRxSsPtr )) {
#ifdef XV_HDMIRXSS_LOG_ENABLE
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_HDMIMODE, 0);
#endif
#ifdef USE_HDCP_RX
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_HDMI_MODE_EVT);
#endif
  }

  // DVI mode
  else {
#ifdef XV_HDMIRXSS_LOG_ENABLE
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_DVIMODE, 0);
#endif
#ifdef USE_HDCP_RX
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_DVI_MODE_EVT);
#endif
  }
}

/*****************************************************************************/
/**
*
* This function is called when the TMDS CLK ratio changes.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_TmdsClkRatioCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  // Check if user callback has been registered
  if (HdmiRxSsPtr->TmdsClkRatioCallback) {
      HdmiRxSsPtr->TmdsClkRatioCallback(HdmiRxSsPtr->TmdsClkRatioRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when the Vic does not match with the detected video
* timing.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_VicErrorCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_VICERROR, 0);
#endif

  // Check if user callback has been registered
  if (HdmiRxSsPtr->VicErrorCallback) {
	HdmiRxSsPtr->VicErrorCallback(HdmiRxSsPtr->VicErrorRef);
  }
}

/*****************************************************************************/
/**
*
* This function retrieves the Vendor Specific Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_RetrieveVSInfoframe(XV_HdmiRxSs *HdmiRxSs)
{
  /** Vendor-Specific InfoFrame structure */
  XHdmiC_VSIF *VSIFPtr;
  VSIFPtr = XV_HdmiRxSs_GetVSIF(HdmiRxSs);

  if (HdmiRxSs->HdmiRxPtr->Aux.Header.Byte[0] == AUX_VSIF_TYPE) {
	  // Reset Vendor Specific InfoFrame
	  (void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

	  XV_HdmiC_VSIF_ParsePacket(&HdmiRxSs->HdmiRxPtr->Aux, VSIFPtr);

      // Defaults
      HdmiRxSs->HdmiRxPtr->Stream.Video.Is3D = FALSE;
      HdmiRxSs->HdmiRxPtr->Stream.Video.Info_3D.Format = XVIDC_3D_UNKNOWN;

      if (VSIFPtr->Format == XHDMIC_VSIF_VF_3D) {
          HdmiRxSs->HdmiRxPtr->Stream.Video.Is3D = TRUE;
          HdmiRxSs->HdmiRxPtr->Stream.Video.Info_3D = VSIFPtr->Info_3D.Stream;
      } else if (VSIFPtr->Format == XHDMIC_VSIF_VF_EXTRES) {
          switch(VSIFPtr->HDMI_VIC) {
              case 1 :
                  HdmiRxSs->HdmiRxPtr->Stream.Vic = 95;
                  break;

              case 2 :
                  HdmiRxSs->HdmiRxPtr->Stream.Vic = 94;
                  break;

              case 3 :
                  HdmiRxSs->HdmiRxPtr->Stream.Vic = 93;
                  break;

              case 4 :
                  HdmiRxSs->HdmiRxPtr->Stream.Vic = 98;
                  break;

              default :
                  break;
          }
      }
  }
}

/*****************************************************************************/
/**
*
* This function is called when a RX Audio IRQ has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_AudCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  u8 Channels;

  if (XV_HdmiRx_IsAudioActive(HdmiRxSsPtr->HdmiRxPtr)) {

    // Get audio channels
    Channels = XV_HdmiRx_GetAudioChannels(HdmiRxSsPtr->HdmiRxPtr);
    HdmiRxSsPtr->AudioChannels = Channels;
  }

  // Check if user callback has been registered
  if (HdmiRxSsPtr->AudCallback) {
      HdmiRxSsPtr->AudCallback(HdmiRxSsPtr->AudRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a RX Link Status IRQ has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_LnkStaCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  HdmiRxSsPtr->IsLinkStatusErrMax =
    XV_HdmiRx_IsLinkStatusErrMax(HdmiRxSsPtr->HdmiRxPtr);
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_LINKSTATUS, 0);
#endif
  // Check if user callback has been registered
  if (HdmiRxSsPtr->LnkStaCallback) {
      HdmiRxSsPtr->LnkStaCallback(HdmiRxSsPtr->LnkStaRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a RX DDC IRQ has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_DdcCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  // Check if user callback has been registered
  if (HdmiRxSsPtr->DdcCallback) {
      HdmiRxSsPtr->DdcCallback(HdmiRxSsPtr->DdcRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_StreamDownCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;
  XHdmiC_AVI_InfoFrame *AviInfoFramePtr;

  /* Assert HDMI RX core resets */
  XV_HdmiRxSs_RXCore_VRST(HdmiRxSsPtr, TRUE);
  XV_HdmiRxSs_RXCore_LRST(HdmiRxSsPtr, TRUE);

  /* Assert SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs_SYSRST(HdmiRxSsPtr, TRUE);

  /* Set stream up flag */
  HdmiRxSsPtr->IsStreamUp = (FALSE);

  DrmInfoFramePtr = XV_HdmiRxSs_GetDrmInfoframe(HdmiRxSsPtr);

  DrmInfoFramePtr->Static_Metadata_Descriptor_ID = 0xFF;
  DrmInfoFramePtr->EOTF = 0xff;

  AviInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(HdmiRxSsPtr);
  memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));

#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMDOWN, 0);
#endif
#ifdef USE_HDCP_RX
  // Push stream-down event to HDCP event queue
  XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_STREAMDOWN_EVT);
#endif

  // Check if user callback has been registered
  if (HdmiRxSsPtr->StreamDownCallback) {
      HdmiRxSsPtr->StreamDownCallback(HdmiRxSsPtr->StreamDownRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when the RX stream init .
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_StreamInitCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMINIT, 0);
#endif
  // Check if user callback has been registered
  if (HdmiRxSsPtr->StreamInitCallback) {
      HdmiRxSsPtr->StreamInitCallback(HdmiRxSsPtr->StreamInitRef);
  }

}

/*****************************************************************************/
/**
*
* This function is called when the RX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_StreamUpCallback(void *CallbackRef)
{
  XV_HdmiRxSs *HdmiRxSsPtr = (XV_HdmiRxSs *)CallbackRef;

  /* Clear link Status error counters */
  XV_HdmiRx_ClearLinkStatus(HdmiRxSsPtr->HdmiRxPtr);

  /* Set stream up flag */
  HdmiRxSsPtr->IsStreamUp = (TRUE);
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMUP, 0);
#endif
#ifdef USE_HDCP_RX
  // Push stream-up event to HDCP event queue
  XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_STREAMUP_EVT);
#endif

  /* Configure Remapper according to HW setting and video format */
  XV_HdmiRxSs_ConfigBridgeMode(HdmiRxSsPtr);

  // Check if user callback has been registered
  if (HdmiRxSsPtr->StreamUpCallback) {
      HdmiRxSsPtr->StreamUpCallback(HdmiRxSsPtr->StreamUpRef);
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
* -----------------------         --------------------------------------------------
* (XV_HDMIRXSS_HANDLER_CONNECT)             HpdCallback
* (XV_HDMIRXSS_HANDLER_VS)                  VsCallback
* (XV_HDMIRXSS_HANDLER_STREAM_DOWN)         StreamDownCallback
* (XV_HDMIRXSS_HANDLER_STREAM_UP)           StreamUpCallback
* (XV_HDMIRXSS_HANDLER_HDCP_AUTHENTICATED)
* (XV_HDMIRXSS_HANDLER_HDCP_UNAUTHENTICATED)
* (XV_HDMIRXSS_HANDLER_HDCP_AUTHENTICATION_REQUEST)
* (XV_HDMIRXSS_HANDLER_HDCP_STREAM_MANAGE_REQUEST)
* (XV_HDMIRXSS_HANDLER_HDCP_TOPOLOGY_UPDATE)
* </pre>
*
* @param    InstancePtr is a pointer to the HDMI RX Subsystem instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_HdmiRxSs_SetCallback(XV_HdmiRxSs *InstancePtr,
		XV_HdmiRxSs_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XV_HDMIRXSS_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {

        case (XV_HDMIRXSS_HANDLER_CONNECT):
            InstancePtr->ConnectCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->ConnectRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS_HANDLER_BRDGOVERFLOW):
            InstancePtr->BrdgOverflowCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->BrdgOverflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS_HANDLER_AUX):
            InstancePtr->AuxCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->AuxRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS_HANDLER_AUD):
            InstancePtr->AudCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->AudRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS_HANDLER_LNKSTA):
            InstancePtr->LnkStaCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->LnkStaRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Ddc
        case (XV_HDMIRXSS_HANDLER_DDC):
            InstancePtr->DdcCallback =(XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->DdcRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream down
        case (XV_HDMIRXSS_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback =(XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream Init
        case (XV_HDMIRXSS_HANDLER_STREAM_INIT):
            InstancePtr->StreamInitCallback =(XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamInitRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream up
        case (XV_HDMIRXSS_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // TMDS_CLK_RATIO
        case (XV_HDMIRXSS_HANDLER_TMDS_CLK_RATIO):
            InstancePtr->TmdsClkRatioCallback =
                                  (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->TmdsClkRatioRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // VIC_ERROR
        case (XV_HDMIRXSS_HANDLER_VIC_ERROR):
            InstancePtr->VicErrorCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->VicErrorRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // HDCP
        case (XV_HDMIRXSS_HANDLER_HDCP):
            InstancePtr->HdcpCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->HdcpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // HDCP authenticated
        case (XV_HDMIRXSS_HANDLER_HDCP_AUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            // Register HDCP 1.4 callbacks
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                  XHDCP1X_HANDLER_AUTHENTICATED,
                                  (void *)(XHdcp1x_Callback)CallbackFunc,
                                  (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATED,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP authenticated
        case (XV_HDMIRXSS_HANDLER_HDCP_UNAUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            // Register HDCP 1.4 callbacks
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                XHDCP1X_HANDLER_UNAUTHENTICATED,
                                (void *)(XHdcp1x_Callback)CallbackFunc,
                                (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_UNAUTHENTICATED,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP authentication request
        case (XV_HDMIRXSS_HANDLER_HDCP_AUTHENTICATION_REQUEST):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            // Register HDCP 1.4 callbacks
            if (InstancePtr->Hdcp14Ptr) {
        //Register the hdcp trigger downstream authentication callback
               XHdcp1x_SetCallBack(InstancePtr->Hdcp14Ptr,
                                   (XHdcp1x_HandlerType) XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
                                   (void *) (XHdcp1x_Callback)CallbackFunc,
                                   (void *) CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATION_REQUEST,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP stream management request
        case (XV_HDMIRXSS_HANDLER_HDCP_STREAM_MANAGE_REQUEST):
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_STREAM_MANAGE_REQUEST,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP topology update request
        case (XV_HDMIRXSS_HANDLER_HDCP_TOPOLOGY_UPDATE):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            // Register HDCP 1.4 callbacks
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                  XHDCP1X_HANDLER_TOPOLOGY_UPDATE,
                                  (void *)(XHdcp1x_Callback)CallbackFunc,
                                  (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_TOPOLOGY_UPDATE,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP encryption status update
        case (XV_HDMIRXSS_HANDLER_HDCP_ENCRYPTION_UPDATE):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            // Register HDCP 1.4 callbacks
            if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                 XHDCP1X_HANDLER_ENCRYPTION_UPDATE,
                                 (void *)(XHdcp1x_Callback)CallbackFunc,
                                 (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_ENCRYPTION_UPDATE,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
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
* This function Sets the EDID parameters in the HDMI RX SS struct
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_SetEdidParam(XV_HdmiRxSs *InstancePtr, u8 *EdidDataPtr,
                                                                u16 Length)
{
    InstancePtr->EdidPtr = EdidDataPtr;
    InstancePtr->EdidLength = Length;
}

/*****************************************************************************/
/**
*
* This function loads the default EDID to the HDMI RX
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_LoadDefaultEdid(XV_HdmiRxSs *InstancePtr)
{
    u32 Status;

    // Load new EDID
    Status = XV_HdmiRx_DdcLoadEdid(InstancePtr->HdmiRxPtr, InstancePtr->EdidPtr,
            InstancePtr->EdidLength);
    if (Status == XST_SUCCESS) {
        xil_printf("\r\nSuccessfully loaded edid.\r\n");
    }
    else {
        xil_printf("\r\nError loading edid.\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function loads the default EDID to the HDMI RX
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_LoadEdid(XV_HdmiRxSs *InstancePtr, u8 *EdidDataPtr,
                                                                u16 Length)
{
    u32 Status;

    // Load new EDID
    Status = XV_HdmiRx_DdcLoadEdid(InstancePtr->HdmiRxPtr, EdidDataPtr, Length);

    if (Status == XST_SUCCESS) {
        xil_printf("\r\nSuccessfully loaded edid.\r\n");
    }
    else {
        xil_printf("\r\nError loading edid.\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function sets the HPD on the HDMI RX.
*
* @param  Value is a flag used to set the HPD.
*   - TRUE drives HPD high
*   - FALSE drives HPD low
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_SetHpd(XV_HdmiRxSs *InstancePtr, u8 Value)
{
  /* Drive HDMI RX HPD based on the input value */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, Value);
}

/*****************************************************************************/
/**
*
* This function toggles the HPD on the HDMI RX.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_ToggleHpd(XV_HdmiRxSs *InstancePtr)
{
  /* Clear SCDC variables */
  XV_HdmiRx_DdcScdcClear(InstancePtr->HdmiRxPtr);

  /* Disable the scrambler */
  XV_HdmiRx_SetScrambler(InstancePtr->HdmiRxPtr, (FALSE));

  /* Drive HDMI RX HPD Low */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (FALSE));

  /* Wait 500 ms */
  XV_HdmiRxSs_WaitUs(InstancePtr, 500000);

  /* Drive HDMI RX HPD High */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (TRUE));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS Aux structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux *XV_HdmiRxSs_GetAuxiliary(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->HdmiRxPtr->Aux));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS AVI InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return XHdmiC_AVI_InfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AVI_InfoFrame *XV_HdmiRxSs_GetAviInfoframe(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->AVIInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS General Control Packet
* structure
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return XHdmiC_GeneralControlPacket pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_GeneralControlPacket *XV_HdmiRxSs_GetGCP(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->GCP));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS Audio InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return XHdmiC_AudioInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AudioInfoFrame *XV_HdmiRxSs_GetAudioInfoframe(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->AudioInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS Vendor Specific InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return XHdmiC_VSIF pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_VSIF *XV_HdmiRxSs_GetVSIF(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->VSIF));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS DRM InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return XHdmiC_DRMInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_DRMInfoFrame *XV_HdmiRxSs_GetDrmInfoframe(XV_HdmiRxSs *InstancePtr)
{
    return (&(InstancePtr->DrmInfoframe));
}

/*****************************************************************************/
/**
*
* This function set HDMI RX susbsystem stream parameters
*
* @param  None.
*
* @return Calculated TMDS Clock
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiRxSs_SetStream(XV_HdmiRxSs *InstancePtr,
        u32 Clock, u32 LineRate)
{

    LineRate = LineRate;

    if (Clock == 0) {
#ifdef XV_HDMIRXSS_LOG_ENABLE
  /* Write log */
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_SETSTREAM_ERR, 0);
#endif

        return (XST_FAILURE);
    }

#ifdef XV_HDMIRXSS_LOG_ENABLE
  /* Write log */
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_SETSTREAM, 0);
#endif
  /* Set stream */
  XV_HdmiRx_SetStream(InstancePtr->HdmiRxPtr, InstancePtr->Config.Ppc, Clock);

  /* In case the TMDS clock ratio is 1/40 */
  /* The reference clock must be compensated */
  if (XV_HdmiRx_GetTmdsClockRatio(InstancePtr->HdmiRxPtr)) {
      InstancePtr->HdmiRxPtr->Stream.RefClk =
                   InstancePtr->HdmiRxPtr->Stream.RefClk * 4;
  }

  return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_HdmiRxSs_GetVideoStream(XV_HdmiRxSs *InstancePtr)
{
    return (&InstancePtr->HdmiRxPtr->Stream.Video);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return VIC
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs_GetVideoIDCode(XV_HdmiRxSs *InstancePtr)
{
    return (InstancePtr->HdmiRxPtr->Stream.Vic);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return Stream Type  1:HDMI 0:DVI
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs_GetVideoStreamType(XV_HdmiRxSs *InstancePtr)
{
    return (InstancePtr->HdmiRxPtr->Stream.IsHdmi);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return Stream Type  1:IsScrambled 0: not Scrambled
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs_GetVideoStreamScramblingFlag(XV_HdmiRxSs *InstancePtr)
{
    return (InstancePtr->HdmiRxPtr->Stream.IsScrambled);
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS number of active audio channels
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs_GetAudioChannels(XV_HdmiRxSs *InstancePtr)
{
    return (InstancePtr->AudioChannels);
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS audio format
*
* @param  InstancePtr pointer to XV_HdmiRxSs instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
XV_HdmiRx_AudioFormatType XV_HdmiRxSs_GetAudioFormat(XV_HdmiRxSs *InstancePtr)
{
    return ((XV_HdmiRx_AudioFormatType)(InstancePtr->HdmiRxPtr->AudFormat));
}

/*****************************************************************************/
/**
*
* This function is called when HDMI RX SS TMDS clock changes
*
* @param  None.
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_RefClockChangeInit(XV_HdmiRxSs *InstancePtr)
{
  // Set TMDS Clock ratio
  InstancePtr->TMDSClockRatio =
    XV_HdmiRx_GetTmdsClockRatio(InstancePtr->HdmiRxPtr);
#ifdef XV_HDMIRXSS_LOG_ENABLE
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_REFCLOCKCHANGE, 0);
#endif
}

/*****************************************************************************/
/**
*
* This function prints the HDMI RX SS timing information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ReportTiming(XV_HdmiRxSs *InstancePtr)
{
    // Check is the RX stream is up
    if (XV_HdmiRx_IsStreamUp(InstancePtr->HdmiRxPtr)) {
        XV_HdmiRx_DebugInfo(InstancePtr->HdmiRxPtr);
        xil_printf("VIC: %0d\r\n", InstancePtr->HdmiRxPtr->Stream.Vic);
        xil_printf("Scrambled: %0d\r\n",
            (XV_HdmiRx_IsStreamScrambled(InstancePtr->HdmiRxPtr)));
    }

    // No stream
    else {
      xil_printf("No HDMI RX stream\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function reports the link quality based on the link error counter
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ReportLinkQuality(XV_HdmiRxSs *InstancePtr)
{
  u8 Channel;
  u32 Errors;

  for (Channel = 0; Channel < 3; Channel++)
  {
      Errors = XV_HdmiRx_GetLinkStatus(InstancePtr->HdmiRxPtr, Channel);

      xil_printf("Link quality channel %0d : ", Channel);

      if (Errors == 0) {
        xil_printf("excellent");
      }

      else if ((Errors > 0) && (Errors < 1024)) {
        xil_printf("good");
      }

      else if ((Errors > 1024) && (Errors < 16384)) {
        xil_printf("average");
      }

      else {
        xil_printf("bad");
      }

      xil_printf(" (%0d)\r\n", Errors);
  }

  /* Clear link error counters */
  XV_HdmiRx_ClearLinkStatus(InstancePtr->HdmiRxPtr);
}

/*****************************************************************************/
/**
*
* This function prints the HDMI RX SS DRM If information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ReportDRMInfo(XV_HdmiRxSs *InstancePtr)
{
	XHdmiC_DRMInfoFrame *DrmInfoFramePtr;
	DrmInfoFramePtr = XV_HdmiRxSs_GetDrmInfoframe(InstancePtr);

	if (DrmInfoFramePtr->EOTF != 0xff)
		xil_printf("eotf: %d\r\n", DrmInfoFramePtr->EOTF);

	if (DrmInfoFramePtr->Static_Metadata_Descriptor_ID == 0xFF) {
		xil_printf("No DRM info\r\n");
		return;
	}

	xil_printf("DRM IF info:\n");
	xil_printf("desc id: %d\r\n", DrmInfoFramePtr->Static_Metadata_Descriptor_ID);
	xil_printf("display primaries x0, y0, x1, y1, x2, y2: %d %d %d %d %d %d\r\n",
			DrmInfoFramePtr->disp_primaries[0].x, DrmInfoFramePtr->disp_primaries[0].y,
			DrmInfoFramePtr->disp_primaries[1].x, DrmInfoFramePtr->disp_primaries[1].y,
			DrmInfoFramePtr->disp_primaries[2].x, DrmInfoFramePtr->disp_primaries[2].y
		  );
	xil_printf("white point x, y: %d %d\r\n",
			DrmInfoFramePtr->white_point.x, DrmInfoFramePtr->white_point.y);
	xil_printf("min/max display mastering luminance: %d %d\r\n",
			DrmInfoFramePtr->Min_Disp_Mastering_Luminance,
			DrmInfoFramePtr->Max_Disp_Mastering_Luminance);
	xil_printf("Max_CLL: %d\r\n", DrmInfoFramePtr->Max_Content_Light_Level);
	xil_printf("max_fall: %d\r\n", DrmInfoFramePtr->Max_Frame_Average_Light_Level);
}


/*****************************************************************************/
/**
*
* This function prints the HDMI RX SS audio information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ReportAudio(XV_HdmiRxSs *InstancePtr)
{
  xil_printf("Format   : ");
  switch (XV_HdmiRxSs_GetAudioFormat(InstancePtr)) {
	  case 0:
		xil_printf("Unknown\r\n");
		break;
	  case 1:
		xil_printf("L-PCM\r\n");
		break;
	  case 2:
		xil_printf("HBR\r\n");
		break;
	  case 3:
		xil_printf("3D\r\n");
		break;
	  default:
		break;
  }
  xil_printf("Channels : %d\r\n",
  XV_HdmiRx_GetAudioChannels(InstancePtr->HdmiRxPtr));
  xil_printf("ACR CTS  : %d\r\n", XV_HdmiRx_GetAcrCts(InstancePtr->HdmiRxPtr));
  xil_printf("ACR N    : %d\r\n", XV_HdmiRx_GetAcrN(InstancePtr->HdmiRxPtr));
}

/*****************************************************************************/
/**
*
* This function prints the HDMI RX SS audio information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ReportInfoFrame(XV_HdmiRxSs *InstancePtr)
{
  xil_printf("RX header: %0x\r\n", InstancePtr->HdmiRxPtr->Aux.Header.Data);
}

/*****************************************************************************/
/**
*
* This function prints the HDMI RX SS subcore versions
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_ReportSubcoreVersion(XV_HdmiRxSs *InstancePtr)
{
  u32 Data;

  if(InstancePtr->HdmiRxPtr)
  {
     Data = XV_HdmiRx_GetVersion(InstancePtr->HdmiRxPtr);
     xil_printf("  HDMI RX version : %02d.%02d (%04x)\r\n",
     ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (InstancePtr->Hdcp14Ptr){
     Data = XHdcp1x_GetVersion(InstancePtr->Hdcp14Ptr);
     xil_printf("  HDCP 1.4 RX version : %02d.%02d (%04x)\r\n",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }
#endif

}

/*****************************************************************************/
/**
*
* This function checks if the video stream is up.
*
* @param  None.
*
* @return
*   - TRUE if stream is up.
*   - FALSE if stream is down.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_IsStreamUp(XV_HdmiRxSs *InstancePtr)
{
  return (InstancePtr->IsStreamUp);
}

/*****************************************************************************/
/**
*
* This function checks if the interface is connected.
*
* @param  None.
*
* @return
*   - TRUE if interface is connected.
*   - FALSE if interface is not connected.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_IsStreamConnected(XV_HdmiRxSs *InstancePtr)
{
  return (InstancePtr->IsStreamConnected);
}

/******************************************************************************/
/**
*
* This function configures the Bridge for YUV420 functionality and repeater
*
* @param InstancePtr  Instance Pointer to the main data structure
* @param None
*
* @return
*
* @note
*   None.
*
******************************************************************************/
static void XV_HdmiRxSs_ConfigBridgeMode(XV_HdmiRxSs *InstancePtr) {
    XVidC_VideoStream *HdmiRxSsVidStreamPtr;
    HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(InstancePtr);

    XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
    AviInfoFramePtr = XV_HdmiRxSs_GetAviInfoframe(InstancePtr);

    if ((!InstancePtr->HdmiRxPtr->Stream.IsHdmi) &&
		    HdmiRxSsVidStreamPtr->IsInterlaced) {
	if ((HdmiRxSsVidStreamPtr->Timing.HActive == 1440) &&
			((HdmiRxSsVidStreamPtr->Timing.VActive == 288) ||
			 (HdmiRxSsVidStreamPtr->Timing.VActive == 240))) {
             XV_HdmiRxSs_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiRxSs_BridgePixelDrop(InstancePtr, TRUE);

	     return;
	}
    }

    // Pixel Repetition factor of 3 and above are not supported by the bridge
    if (AviInfoFramePtr->PixelRepetition > XHDMIC_PIXEL_REPETITION_FACTOR_2) {
#ifdef XV_HDMIRXSS_LOG_ENABLE
    	XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_PIX_REPEAT_ERR,
    			AviInfoFramePtr->PixelRepetition);
#endif

    	return;
    }

    if (HdmiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
         XV_HdmiRxSs_BridgePixelDrop(InstancePtr, FALSE);
         XV_HdmiRxSs_BridgeYuv420(InstancePtr, TRUE);
    }
    else {
        if (AviInfoFramePtr->PixelRepetition ==
				XHDMIC_PIXEL_REPETITION_FACTOR_2)
        {
            /*********************************************************
             * NTSC/PAL Support
             *********************************************************/
             XV_HdmiRxSs_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiRxSs_BridgePixelDrop(InstancePtr, TRUE);
        }
        else {
            XV_HdmiRxSs_BridgeYuv420(InstancePtr, FALSE);
            XV_HdmiRxSs_BridgePixelDrop(InstancePtr, FALSE);
        }
    }
}

/*****************************************************************************/
/**
* This function will set the default in HDF.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs core instance.
* @param    Id is the XV_HdmiRxSs ID to operate on.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs_SetDefaultPpc(XV_HdmiRxSs *InstancePtr, u8 Id) {
    extern XV_HdmiRxSs_Config XV_HdmiRxSs_ConfigTable[XPAR_XV_HDMIRXSS_NUM_INSTANCES];
    InstancePtr->Config.Ppc = XV_HdmiRxSs_ConfigTable[Id].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs core instance.
* @param    Id is the XV_HdmiRxSs ID to operate on.
* @param    Ppc is the PPC to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs_SetPpc(XV_HdmiRxSs *InstancePtr, u8 Id, u8 Ppc) {
    InstancePtr->Config.Ppc = (XVidC_PixelsPerClock) Ppc;
    Id = Id; //squash unused variable compiler warning
}

/*****************************************************************************/
/**
* This function will set the major and minor application version in RXSs struct
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs core instance.
* @param    maj is the major version of the application.
* @param    min is the minor version of the application.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiRxSS_SetAppVersion(XV_HdmiRxSs *InstancePtr, u8 maj, u8 min)
{
	InstancePtr->AppMajVer = maj;
	InstancePtr->AppMinVer = min;
}

/*****************************************************************************/
/**
*
* This function set HDMI RX audio parameters
*
* @param  Enable 0: Unmute the audio 1: Mute the audio.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_AudioMute(XV_HdmiRxSs *InstancePtr, u8 Enable)
{
  //Audio Mute Mode
  if (Enable){
	XV_HdmiRx_AudioDisable(InstancePtr->HdmiRxPtr);
  }
  else{
	XV_HdmiRx_AudioEnable(InstancePtr->HdmiRxPtr);
  }
}
