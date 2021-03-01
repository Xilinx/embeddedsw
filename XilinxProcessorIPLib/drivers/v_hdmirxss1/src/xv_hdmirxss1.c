/******************************************************************************
# Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1.c
*
* This is main code of Xilinx HDMI Receiver Subsystem device driver.
* Please see xv_hdmirxss1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
******************************************************************************/

/***************************** Include Files *********************************/
#include "sleep.h"
#include "xv_hdmirxss1.h"
#include "xv_hdmirxss1_coreinit.h"

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
  XV_HdmiRx1 HdmiRx1;
} XV_HdmiRxSs1_SubCores;

/**************************** Local Global ***********************************/
/** Define Driver instance of all sub-core included in the design */
XV_HdmiRxSs1_SubCores XV_HdmiRxSs1_SubCoreRepo[XPAR_XV_HDMIRXSS1_NUM_INSTANCES];

/************************** Function Prototypes ******************************/
static void XV_HdmiRxSs1_GetIncludedSubcores(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
    u16 DevId);
static void XV_HdmiRxSs1_WaitUs(XV_HdmiRxSs1 *InstancePtr, u32 MicroSeconds);
static void XV_HdmiRxSs1_RetrieveVSInfoframe(XV_HdmiRxSs1 *HdmiRxSs1);
static int XV_HdmiRxSs1_RegisterSubsysCallbacks(XV_HdmiRxSs1 *InstancePtr);
static void XV_HdmiRxSs1_ConnectCallback(void *CallbackRef);
static void XV_HdmiRxSs1_BrdgOverflowCallback(void *CallbackRef);
static void XV_HdmiRxSs1_AuxCallback(void *CallbackRef);
static void XV_HdmiRxSs1_AudCallback(void *CallbackRef);
static void XV_HdmiRxSs1_LnkStaCallback(void *CallbackRef);
static void XV_HdmiRxSs1_DdcCallback(void *CallbackRef);
static void XV_HdmiRxSs1_StreamDownCallback(void *CallbackRef);
static void XV_HdmiRxSs1_StreamInitCallback(void *CallbackRef);
static void XV_HdmiRxSs1_StreamUpCallback(void *CallbackRef);
static void XV_HdmiRxSs1_SyncLossCallback(void *CallbackRef);
static void XV_HdmiRxSs1_ModeCallback(void *CallbackRef);
static void XV_HdmiRxSs1_TmdsClkRatioCallback(void *CallbackRef);
static void XV_HdmiRxSs1_PhyResetCallback(void *CallbackRef);
static void XV_HdmiRxSs1_VicErrorCallback(void *CallbackRef);
static void XV_HdmiRxSs1_LnkRdyErrorCallback(void *CallbackRef);
static void XV_HdmiRxSs1_VidRdyErrorCallback(void *CallbackRef);
static void XV_HdmiRxSs1_SkewLockErrorCallback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLtsLCallback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLts1Callback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLts2Callback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLts3Callback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLts4Callback(void *CallbackRef);
static void XV_HdmiRxSs1_FrlLtsPCallback(void *CallbackRef);
static void XV_HdmiRxSs1_VfpChanged(void *CallbackRef);
static void XV_HdmiRxSs1_VrrReady(void *CallbackRef);
static void XV_HdmiRxSs1_DynHdrEvtCallback(void *CallbackRef);

static void XV_HdmiRxSs1_ConfigBridgeMode(XV_HdmiRxSs1 *InstancePtr);

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This macros selects the bridge YUV420 mode
*
* @param  InstancePtr is a pointer to the HDMI RX Subsystem
*
*****************************************************************************/
#define XV_HdmiRxSs1_BridgeYuv420(InstancePtr,Enable) \
{ \
    XV_HdmiRx1_Bridge_yuv420(InstancePtr->HdmiRx1Ptr, Enable); \
} \

/*****************************************************************************/
/**
* This macros selects the bridge pixel repeat mode
*
* @param  InstancePtr is a pointer to the HDMI RX Subsystem
*
*****************************************************************************/
#define XV_HdmiRxSs1_BridgePixelDrop(InstancePtr,Enable) \
{ \
    XV_HdmiRx1_Bridge_pixel(InstancePtr->HdmiRx1Ptr, Enable); \
}

/************************** Function Definition ******************************/

void XV_HdmiRxSs1_ReportInfo(XV_HdmiRxSs1 *InstancePtr)
{
    xil_printf("------------\r\n");
    xil_printf("HDMI RX SubSystem\r\n");
    xil_printf("------------\r\n");
    XV_HdmiRxSs1_ReportCoreInfo(InstancePtr);
    XV_HdmiRxSs1_ReportSubcoreVersion(InstancePtr);
    xil_printf("\r\n");
    xil_printf("------------\r\n");
    xil_printf("HDMI RX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiRxSs1_ReportTiming(InstancePtr);
    xil_printf("Link quality\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportLinkQuality(InstancePtr);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportAudio(InstancePtr);
    xil_printf("Static HDR DRM Info frame\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportDRMInfo(InstancePtr);
    xil_printf("InfoFrame\r\n");
    xil_printf("---------\r\n");
    XV_HdmiRxSs1_ReportInfoFrame(InstancePtr);
    xil_printf("\r\n");
}

/*****************************************************************************/
/**
* This function prints out the sub-core register dump
*
* @param	InstancePtr  Instance Pointer to the main data structure
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiRxSs1_RegisterDebug(XV_HdmiRxSs1 *InstancePtr)
{
	/* HDMI RX Core */
	XV_HdmiRx1_RegisterDebug(InstancePtr->HdmiRx1Ptr);
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
void XV_HdmiRxSs1_ReportCoreInfo(XV_HdmiRxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->HDMI RX Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if (InstancePtr->HdmiRx1Ptr)
  {
    xil_printf("    : HDMI RX \r\n");
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (InstancePtr->Hdcp14Ptr)
  {
    xil_printf("    : HDCP 1.4 RX \r\n");
  }

  if (InstancePtr->HdcpTimerPtr)
  {
    xil_printf("    : HDCP: AXIS Timer\r\n");
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  if (InstancePtr->Hdcp22Ptr)
  {
    xil_printf("    : HDCP 2.2 RX \r\n");
  }
#endif
}

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the
 *  XV_HdmiRxSs1 driver.
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
void XV_HdmiRxSs1_SetUserTimerHandler(XV_HdmiRxSs1 *InstancePtr,
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
 * This function is the delay/sleep function for the XV_HdmiRxSs1 driver. For the
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
static void XV_HdmiRxSs1_WaitUs(XV_HdmiRxSs1 *InstancePtr, u32 MicroSeconds)
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
void XV_HdmiRxSS1_HdmiRxIntrHandler(XV_HdmiRxSs1 *InstancePtr)
{
    XV_HdmiRx1_IntrHandler(InstancePtr->HdmiRx1Ptr);
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
static int XV_HdmiRxSs1_RegisterSubsysCallbacks(XV_HdmiRxSs1 *InstancePtr)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = InstancePtr;

  /*Register HDMI callbacks*/
  if (HdmiRxSs1Ptr->HdmiRx1Ptr) {
    /*
     * Register call back for Rx Core Interrupts.
     */
    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_CONNECT,
                          (void *)XV_HdmiRxSs1_ConnectCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_BRDG_OVERFLOW,
                          (void *)XV_HdmiRxSs1_BrdgOverflowCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_AUX,
                          (void *)XV_HdmiRxSs1_AuxCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_AUD,
                          (void *)XV_HdmiRxSs1_AudCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_LNKSTA,
                          (void *)XV_HdmiRxSs1_LnkStaCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_DDC,
                          (void *)XV_HdmiRxSs1_DdcCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_STREAM_DOWN,
                          (void *)XV_HdmiRxSs1_StreamDownCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_STREAM_INIT,
                          (void *)XV_HdmiRxSs1_StreamInitCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_STREAM_UP,
                          (void *)XV_HdmiRxSs1_StreamUpCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_SYNC_LOSS,
                          (void *)XV_HdmiRxSs1_SyncLossCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_MODE,
                          (void *)XV_HdmiRxSs1_ModeCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_TMDS_CLK_RATIO,
                          (void *)XV_HdmiRxSs1_TmdsClkRatioCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_VIC_ERROR,
                          (void *)XV_HdmiRxSs1_VicErrorCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_PHY_RESET,
                          (void *)XV_HdmiRxSs1_PhyResetCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_LNK_RDY_ERR,
                          (void *)XV_HdmiRxSs1_LnkRdyErrorCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_VID_RDY_ERR,
                          (void *)XV_HdmiRxSs1_VidRdyErrorCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                          XV_HDMIRX1_HANDLER_SKEW_LOCK_ERR,
                          (void *)XV_HdmiRxSs1_SkewLockErrorCallback,
                          (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_CONFIG,
			  (void *)XV_HdmiRxSs1_FrlConfigCallback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_START,
			  (void *)XV_HdmiRxSs1_FrlStartCallback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_TMDS_CONFIG,
			  (void *)XV_HdmiRxSs1_TmdsConfigCallback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTSL,
			  (void *)XV_HdmiRxSs1_FrlLtsLCallback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTS1,
			  (void *)XV_HdmiRxSs1_FrlLts1Callback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTS2,
			  (void *)XV_HdmiRxSs1_FrlLts2Callback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTS3,
			  (void *)XV_HdmiRxSs1_FrlLts3Callback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTS4,
			  (void *)XV_HdmiRxSs1_FrlLts4Callback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_FRL_LTSP,
			  (void *)XV_HdmiRxSs1_FrlLtsPCallback,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_VFP_CHANGE,
			  (void *)XV_HdmiRxSs1_VfpChanged,
			  (void *)InstancePtr);

    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			  XV_HDMIRX1_HANDLER_VRR_RDY,
			  (void *)XV_HdmiRxSs1_VrrReady,
			  (void *)InstancePtr);
    XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			   XV_HDMIRX1_HANDLER_DYN_HDR,
			  (void *)XV_HdmiRxSs1_DynHdrEvtCallback,
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
* @param  HdmiRxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void XV_HdmiRxSs1_GetIncludedSubcores(XV_HdmiRxSs1 *HdmiRxSs1Ptr,
		u16 DevId)
{
  HdmiRxSs1Ptr->HdmiRx1Ptr   =((HdmiRxSs1Ptr->Config.HdmiRx1.IsPresent) ?
                            (&XV_HdmiRxSs1_SubCoreRepo[DevId].HdmiRx1) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
  HdmiRxSs1Ptr->Hdcp14Ptr   =((HdmiRxSs1Ptr->Config.Hdcp14.IsPresent) ?
                            (&XV_HdmiRxSs1_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiRxSs1Ptr->HdcpTimerPtr=((HdmiRxSs1Ptr->Config.HdcpTimer.IsPresent) ?
                            (&XV_HdmiRxSs1_SubCoreRepo[DevId].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  HdmiRxSs1Ptr->Hdcp22Ptr   =((HdmiRxSs1Ptr->Config.Hdcp22.IsPresent) ?
                            (&XV_HdmiRxSs1_SubCoreRepo[DevId].Hdcp22) : NULL);
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
int XV_HdmiRxSs1_CfgInitialize(XV_HdmiRxSs1 *InstancePtr,
    XV_HdmiRxSs1_Config *CfgPtr,
    UINTPTR EffectiveAddr)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = InstancePtr;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiRxSs1Ptr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

  /* Setup the instance */
  memcpy((void *)&(HdmiRxSs1Ptr->Config), (const void *)CfgPtr,
    sizeof(XV_HdmiRxSs1_Config));
  HdmiRxSs1Ptr->Config.BaseAddress = EffectiveAddr;

  /* Determine sub-cores included in the provided instance of subsystem */
  XV_HdmiRxSs1_GetIncludedSubcores(HdmiRxSs1Ptr, CfgPtr->DeviceId);

  /* Initialize all included sub_cores */
  if (HdmiRxSs1Ptr->HdmiRx1Ptr)
  {
    if (XV_HdmiRxSs1_SubcoreInitHdmiRx1(HdmiRxSs1Ptr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
    XV_HdmiRx1_SetAxiClkFreq(HdmiRxSs1Ptr->HdmiRx1Ptr,
                            HdmiRxSs1Ptr->Config.AxiLiteClkFreq);
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  /* HDCP 1.4*/
  if (HdmiRxSs1Ptr->Hdcp14Ptr)
  {
    if (XV_HdmiRxSs1_SubcoreInitHdcp14(HdmiRxSs1Ptr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if (HdmiRxSs1Ptr->HdcpTimerPtr)
  {
    if (XV_HdmiRxSs1_SubcoreInitHdcpTimer(HdmiRxSs1Ptr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  /* HDCP 2.2*/
  if (HdmiRxSs1Ptr->Hdcp22Ptr)
  {
    if (XV_HdmiRxSs1_SubcoreInitHdcp22(HdmiRxSs1Ptr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }
#endif

  /* Register Callbacks */
  XV_HdmiRxSs1_RegisterSubsysCallbacks(HdmiRxSs1Ptr);

#ifdef USE_HDCP_RX
  /* Default value */
  HdmiRxSs1Ptr->HdcpIsReady = (FALSE);
  XV_HdmiRxSs1_HdcpSetCapability(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_BOTH);
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when both HDCP cores are instantiated and all keys are
     loaded */
  if (HdmiRxSs1Ptr->Hdcp14Ptr && HdmiRxSs1Ptr->Hdcp22Ptr &&
      HdmiRxSs1Ptr->Hdcp22Lc128Ptr && HdmiRxSs1Ptr->Hdcp14KeyPtr &&
      HdmiRxSs1Ptr->Hdcp22PrivateKeyPtr) {
    HdmiRxSs1Ptr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key is
     loaded */
  if (!HdmiRxSs1Ptr->HdcpIsReady && HdmiRxSs1Ptr->Hdcp14Ptr &&
       HdmiRxSs1Ptr->Hdcp14KeyPtr) {
    HdmiRxSs1Ptr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the keys are
     loaded */
  if (!HdmiRxSs1Ptr->HdcpIsReady && HdmiRxSs1Ptr->Hdcp22Ptr &&
       HdmiRxSs1Ptr->Hdcp22Lc128Ptr && HdmiRxSs1Ptr->Hdcp22PrivateKeyPtr) {
    HdmiRxSs1Ptr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_22);
  }
#endif

  /* Reset the hardware and set the flag to indicate the
     subsystem is ready
   */
  XV_HdmiRxSs1_Reset(HdmiRxSs1Ptr);
  HdmiRxSs1Ptr->IsReady = XIL_COMPONENT_IS_READY;

  /* Initialize the application version with 0 <default value>.
   * Application need to set the this variable properly to let driver know
   * what version of application is being used.
   */
  HdmiRxSs1Ptr->AppMajVer = 0;
  HdmiRxSs1Ptr->AppMinVer = 0;

  /* Disable the Logging */
  HdmiRxSs1Ptr->EnableHDCPLogging = (FALSE);
  HdmiRxSs1Ptr->EnableHDMILogging = (FALSE);

  /* Initialise the static HDR struct */
  DrmInfoFramePtr = XV_HdmiRxSs1_GetDrmInfoframe(HdmiRxSs1Ptr);
  DrmInfoFramePtr->Static_Metadata_Descriptor_ID = 0xFF;
  DrmInfoFramePtr->EOTF = 0xFF;

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
void XV_HdmiRxSs1_Start(XV_HdmiRxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_START, 0);
#endif

  XV_HdmiRx1_Start(InstancePtr->HdmiRx1Ptr);
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
void XV_HdmiRxSs1_Stop(XV_HdmiRxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx1_Stop(InstancePtr->HdmiRx1Ptr);

  /* Clear SCDC variables */
  XV_HdmiRx1_DdcScdcClear(InstancePtr->HdmiRx1Ptr);

  /* Disable the scrambler */
  XV_HdmiRx1_SetScrambler(InstancePtr->HdmiRx1Ptr, (FALSE));

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_STOP, 0);
#endif

  XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr->HdmiRx1Ptr);
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
void XV_HdmiRxSs1_Reset(XV_HdmiRxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_RESET, 0);
#endif

  /* Assert HDMI RX core resets */
  XV_HdmiRxSs1_RXCore_VRST(InstancePtr, TRUE);
  XV_HdmiRxSs1_RXCore_LRST(InstancePtr, TRUE);

  /* Assert SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs1_SYSRST(InstancePtr, TRUE);

  /* Release HDMI RX core resets */
  XV_HdmiRxSs1_RXCore_VRST(InstancePtr, FALSE);
  XV_HdmiRxSs1_RXCore_LRST(InstancePtr, FALSE);

  /* Release SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs1_SYSRST(InstancePtr, FALSE);
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
void XV_HdmiRxSs1_RXCore_VRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx1_INT_VRST(InstancePtr->HdmiRx1Ptr, Reset);
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
void XV_HdmiRxSs1_RXCore_LRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx1_INT_LRST(InstancePtr->HdmiRx1Ptr, Reset);
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
void XV_HdmiRxSs1_VRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx1_EXT_VRST(InstancePtr->HdmiRx1Ptr, Reset);
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
void XV_HdmiRxSs1_SYSRST(XV_HdmiRxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiRx1_EXT_SYSRST(InstancePtr->HdmiRx1Ptr, Reset);
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
static void XV_HdmiRxSs1_BrdgOverflowCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: RX Bridge Overflow\r\n");

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->BrdgOverflowCallback) {
    HdmiRxSs1Ptr->BrdgOverflowCallback(HdmiRxSs1Ptr->BrdgOverflowRef);
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
static void XV_HdmiRxSs1_ConnectCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  /* Is the cable connected?*/
  if (XV_HdmiRx1_IsStreamConnected(HdmiRxSs1Ptr->HdmiRx1Ptr)) {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_CONNECT, 0);
#endif
    /* Set RX hot plug detect*/
    XV_HdmiRx1_SetHpd(HdmiRxSs1Ptr->HdmiRx1Ptr, TRUE);

    /* Set stream connected flag*/
    HdmiRxSs1Ptr->IsStreamConnected = (TRUE);

#ifdef USE_HDCP_RX
    /* Push connect event to HDCP event queue*/
    XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_CONNECT_EVT);
#endif
  }

  /* RX cable is disconnected*/
  else {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_DISCONNECT, 0);
#endif
    /* Clear RX hot plug detect*/
    XV_HdmiRx1_SetHpd(HdmiRxSs1Ptr->HdmiRx1Ptr, FALSE);

    /* Set stream connected flag*/
    HdmiRxSs1Ptr->IsStreamConnected = (FALSE);

#ifdef USE_HDCP_RX
    /* Push disconnect event to HDCP event queue*/
    XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_DISCONNECT_EVT);
#endif

    XV_HdmiRx1_SetScrambler(HdmiRxSs1Ptr->HdmiRx1Ptr, (FALSE)); /*Disable scrambler*/
  }


  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->ConnectCallback) {
    HdmiRxSs1Ptr->ConnectCallback(HdmiRxSs1Ptr->ConnectRef);
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
static void XV_HdmiRxSs1_AuxCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;
  XHdmiC_Aux *AuxPtr;
  XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
  XHdmiC_GeneralControlPacket *GeneralControlPacketPtr;
  XHdmiC_AudioInfoFrame *AudioInfoFramePtr;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

  AviInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(HdmiRxSs1Ptr);
  GeneralControlPacketPtr = XV_HdmiRxSs1_GetGCP(HdmiRxSs1Ptr);
  AudioInfoFramePtr = XV_HdmiRxSs1_GetAudioInfoframe(HdmiRxSs1Ptr);
  DrmInfoFramePtr = XV_HdmiRxSs1_GetDrmInfoframe(HdmiRxSs1Ptr);

  AuxPtr = XV_HdmiRxSs1_GetAuxiliary(HdmiRxSs1Ptr);

  if (AuxPtr->Header.Byte[0] == AUX_VSIF_TYPE){
	  /* Retrieve Vendor Specific Info Frame*/
	  XV_HdmiRxSs1_RetrieveVSInfoframe(HdmiRxSs1Ptr);
  } else if (AuxPtr->Header.Byte[0] == AUX_AVI_INFOFRAME_TYPE){
	  /* Reset Avi InfoFrame*/
	  (void)memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));
	  /* Parse Aux to retrieve Avi InfoFrame*/
	  XV_HdmiC_ParseAVIInfoFrame(AuxPtr, AviInfoFramePtr);
	  HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.Video.ColorFormatId =
				XV_HdmiRx1_GetAviColorSpace(HdmiRxSs1Ptr->HdmiRx1Ptr);
	  HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.Vic =
				XV_HdmiRx1_GetAviVic(HdmiRxSs1Ptr->HdmiRx1Ptr);
	  HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.Video.AspectRatio =
				XV_HdmiC_IFAspectRatio_To_XVidC(HdmiRxSs1Ptr->AVIInfoframe.PicAspectRatio);
  } else if (AuxPtr->Header.Byte[0] == AUX_GENERAL_CONTROL_PACKET_TYPE) {
	  /* Reset General Control Packet*/
	  (void)memset((void *)GeneralControlPacketPtr, 0, sizeof(XHdmiC_GeneralControlPacket));
	  /* Parse Aux to retrieve General Control Packet*/
	  XV_HdmiC_ParseGCP(AuxPtr, GeneralControlPacketPtr);
	  /* Stream.Video.ColorDepth is updated from the core during AUX INTR*/
  } else if (AuxPtr->Header.Byte[0] == AUX_AUDIO_INFOFRAME_TYPE) {
	  /* Reset Audio InfoFrame*/
	  (void)memset((void *)AudioInfoFramePtr, 0, sizeof(XHdmiC_AudioInfoFrame));
	  /* Parse Aux to retrieve Audio InfoFrame*/
	  XV_HdmiC_ParseAudioInfoFrame(AuxPtr, AudioInfoFramePtr);
  } else if (AuxPtr->Header.Byte[0] == AUX_DRM_INFOFRAME_TYPE) {
	  (void)memset((void *)DrmInfoFramePtr, 0, sizeof(XHdmiC_DRMInfoFrame));
	  XV_HdmiC_ParseDRMIF(AuxPtr, DrmInfoFramePtr);
  }

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->AuxCallback) {
      HdmiRxSs1Ptr->AuxCallback(HdmiRxSs1Ptr->AuxRef);
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
static void XV_HdmiRxSs1_SyncLossCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  if (HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.SyncStatus ==
		  XV_HDMIRX1_SYNCSTAT_SYNC_LOSS) {
  /* Push sync loss event to HDCP event queue*/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_SYNCLOSS, 0);
#endif

#ifdef USE_HDCP_RX
  XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_SYNC_LOSS_EVT);
#endif
  }
  /* Sync is recovered/establish*/
  else if (HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.SyncStatus ==
		  XV_HDMIRX1_SYNCSTAT_SYNC_EST) {
	  /* Push sync loss event to HDCP event queue*/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
	  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_SYNCEST, 0);
#endif

#ifdef USE_HDCP_RX
	  XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr,
			  XV_HDMIRXSS1_HDCP_SYNC_EST_EVT);
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
static void XV_HdmiRxSs1_ModeCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  /* HDMI mode*/
  if (XV_HdmiRxSs1_GetVideoStreamType(HdmiRxSs1Ptr)) {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_HDMIMODE, 0);
#endif
#ifdef USE_HDCP_RX
    XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_HDMI_MODE_EVT);
#endif
  }

  /* DVI mode*/
  else {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_DVIMODE, 0);
#endif
#ifdef USE_HDCP_RX
    XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_DVI_MODE_EVT);
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
static void XV_HdmiRxSs1_TmdsClkRatioCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->TmdsClkRatioCallback) {
      HdmiRxSs1Ptr->TmdsClkRatioCallback(HdmiRxSs1Ptr->TmdsClkRatioRef);
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
static void XV_HdmiRxSs1_VicErrorCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_VICERROR, 0);
#endif

  /* Check if user callback has been registered */
  if (HdmiRxSs1Ptr->VicErrorCallback) {
    HdmiRxSs1Ptr->VicErrorCallback(HdmiRxSs1Ptr->VicErrorRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when the Link Ready error happens.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs1_LnkRdyErrorCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_LNKRDYERROR,
		  HdmiRxSs1Ptr->HdmiRx1Ptr->DBMessage);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the Video Ready error happens.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs1_VidRdyErrorCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_VIDRDYERROR,
		  HdmiRxSs1Ptr->HdmiRx1Ptr->DBMessage);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the Skew Lock error happens.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs1_SkewLockErrorCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_SKEWLOCKERROR,
		  HdmiRxSs1Ptr->HdmiRx1Ptr->DBMessage);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the RX Phy needs to be reset.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiRxSs1_PhyResetCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

	/* Check if user callback has been registered*/
	if (HdmiRxSs1Ptr->PhyResetCallback) {
		      HdmiRxSs1Ptr->PhyResetCallback(HdmiRxSs1Ptr->PhyResetRef);
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
static void XV_HdmiRxSs1_RetrieveVSInfoframe(XV_HdmiRxSs1 *HdmiRxSs1)
{
  /** Vendor-Specific InfoFrame structure */
  XHdmiC_VSIF *VSIFPtr;
  VSIFPtr = XV_HdmiRxSs1_GetVSIF(HdmiRxSs1);

  if (HdmiRxSs1->HdmiRx1Ptr->Aux.Header.Byte[0] == AUX_VSIF_TYPE) {
	  /* Reset Vendor Specific InfoFrame*/
	  (void)memset((void *)VSIFPtr, 0, sizeof(XHdmiC_VSIF));

	  XV_HdmiC_VSIF_ParsePacket(&HdmiRxSs1->HdmiRx1Ptr->Aux, VSIFPtr);

      /* Defaults*/
      HdmiRxSs1->HdmiRx1Ptr->Stream.Video.Is3D = FALSE;
      HdmiRxSs1->HdmiRx1Ptr->Stream.Video.Info_3D.Format = XVIDC_3D_UNKNOWN;

      if (VSIFPtr->Format == XHDMIC_VSIF_VF_3D) {
          HdmiRxSs1->HdmiRx1Ptr->Stream.Video.Is3D = TRUE;
          HdmiRxSs1->HdmiRx1Ptr->Stream.Video.Info_3D =
			  VSIFPtr->Info_3D.Stream;
      } else if (VSIFPtr->Format == XHDMIC_VSIF_VF_EXTRES) {
          switch(VSIFPtr->HDMI_VIC) {
              case 1 :
                  HdmiRxSs1->HdmiRx1Ptr->Stream.Vic = 95;
                  break;

              case 2 :
                  HdmiRxSs1->HdmiRx1Ptr->Stream.Vic = 94;
                  break;

              case 3 :
                  HdmiRxSs1->HdmiRx1Ptr->Stream.Vic = 93;
                  break;

              case 4 :
                  HdmiRxSs1->HdmiRx1Ptr->Stream.Vic = 98;
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
static void XV_HdmiRxSs1_AudCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  u8 Channels;

  if (XV_HdmiRx1_IsAudioActive(HdmiRxSs1Ptr->HdmiRx1Ptr)) {

    /* Get audio channels*/
    Channels = XV_HdmiRx1_GetAudioChannels(HdmiRxSs1Ptr->HdmiRx1Ptr);
    HdmiRxSs1Ptr->AudioChannels = Channels;
  }

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->AudCallback) {
      HdmiRxSs1Ptr->AudCallback(HdmiRxSs1Ptr->AudRef);
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
static void XV_HdmiRxSs1_LnkStaCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  HdmiRxSs1Ptr->IsLinkStatusErrMax =
    XV_HdmiRx1_IsLinkStatusErrMax(HdmiRxSs1Ptr->HdmiRx1Ptr);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_LINKSTATUS, 0);
#endif
  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->LnkStaCallback) {
      HdmiRxSs1Ptr->LnkStaCallback(HdmiRxSs1Ptr->LnkStaRef);
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
static void XV_HdmiRxSs1_DdcCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->DdcCallback) {
      HdmiRxSs1Ptr->DdcCallback(HdmiRxSs1Ptr->DdcRef);
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
static void XV_HdmiRxSs1_StreamDownCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;
  XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
  XV_HdmiC_VideoTimingExtMeta *ExtMetaPtr;
  XV_HdmiC_SrcProdDescIF *SpdIfPtr;
  XVidC_VideoStream *VidCStream;
  XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

  if (HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
	  /* Assert HDMI RX core resets */
	  XV_HdmiRxSs1_RXCore_VRST(HdmiRxSs1Ptr, TRUE);
	  XV_HdmiRxSs1_RXCore_LRST(HdmiRxSs1Ptr, TRUE);
  }
  HdmiRxSs1Ptr->HdmiRx1Ptr->IsFirstVtemReceived = FALSE;

  /* Assert SYSCLK VID_IN bridge reset */
  XV_HdmiRxSs1_SYSRST(HdmiRxSs1Ptr, TRUE);

  /* Set stream up flag */
  HdmiRxSs1Ptr->IsStreamUp = (FALSE);

  AviInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(HdmiRxSs1Ptr);
  memset((void *)AviInfoFramePtr, 0, sizeof(XHdmiC_AVI_InfoFrame));

  DrmInfoFramePtr = XV_HdmiRxSs1_GetDrmInfoframe(HdmiRxSs1Ptr);
  DrmInfoFramePtr->Static_Metadata_Descriptor_ID = 0xFF;
  DrmInfoFramePtr->EOTF = 0xFF;

  XV_HdmiRx1_SetVrrIfType(HdmiRxSs1Ptr->HdmiRx1Ptr,
		  XV_HDMIC_VRRINFO_TYPE_NONE);
  ExtMetaPtr = XV_HdmiRx1_GetVidTimingExtMeta(HdmiRxSs1Ptr->HdmiRx1Ptr);
  memset((void *)ExtMetaPtr, 0, sizeof(XV_HdmiC_VideoTimingExtMeta));

  SpdIfPtr = XV_HdmiRx1_GetSrcProdDescIF(HdmiRxSs1Ptr->HdmiRx1Ptr);
  memset((void *)SpdIfPtr, 0, sizeof(XV_HdmiC_SrcProdDescIF));

  VidCStream = XV_HdmiRxSs1_GetVideoStream(HdmiRxSs1Ptr);
  memset(&(VidCStream->BaseTiming), 0, sizeof(XVidC_VideoTiming));

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_STREAMDOWN, 0);
#endif
#ifdef USE_HDCP_RX
  /* Push stream-down event to HDCP event queue*/
  XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_STREAMDOWN_EVT);
#endif

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->StreamDownCallback) {
      HdmiRxSs1Ptr->StreamDownCallback(HdmiRxSs1Ptr->StreamDownRef);
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
static void XV_HdmiRxSs1_StreamInitCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_STREAMINIT, 0);
#endif

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->StreamInitCallback) {
      HdmiRxSs1Ptr->StreamInitCallback(HdmiRxSs1Ptr->StreamInitRef);
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
static void XV_HdmiRxSs1_StreamUpCallback(void *CallbackRef)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

  /* Clear link Status error counters */
  XV_HdmiRx1_ClearLinkStatus(HdmiRxSs1Ptr->HdmiRx1Ptr);

  /* Set stream up flag */
  HdmiRxSs1Ptr->IsStreamUp = (TRUE);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_STREAMUP, 0);
#endif
#ifdef USE_HDCP_RX
  /* Push stream-up event to HDCP event queue*/
  XV_HdmiRxSs1_HdcpPushEvent(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_STREAMUP_EVT);
#endif

  /* Configure Remapper according to HW setting and video format */
  XV_HdmiRxSs1_ConfigBridgeMode(HdmiRxSs1Ptr);

  /* Check if user callback has been registered*/
  if (HdmiRxSs1Ptr->StreamUpCallback) {
      HdmiRxSs1Ptr->StreamUpCallback(HdmiRxSs1Ptr->StreamUpRef);
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
* (XV_HDMIRXSS1_HANDLER_CONNECT)             HpdCallback
* (XV_HDMIRXSS1_HANDLER_VS)                  VsCallback
* (XV_HDMIRXSS1_HANDLER_STREAM_DOWN)         StreamDownCallback
* (XV_HDMIRXSS1_HANDLER_STREAM_UP)           StreamUpCallback
* (XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATED)
* (XV_HDMIRXSS1_HANDLER_HDCP_UNAUTHENTICATED)
* (XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATION_REQUEST)
* (XV_HDMIRXSS1_HANDLER_HDCP_STREAM_MANAGE_REQUEST)
* (XV_HDMIRXSS1_HANDLER_HDCP_TOPOLOGY_UPDATE)
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
int XV_HdmiRxSs1_SetCallback(XV_HdmiRxSs1 *InstancePtr,
		XV_HdmiRxSs1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XV_HDMIRXSS1_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {

        case (XV_HDMIRXSS1_HANDLER_CONNECT):
            InstancePtr->ConnectCallback = (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->ConnectRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS1_HANDLER_BRDGOVERFLOW):
            InstancePtr->BrdgOverflowCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->BrdgOverflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS1_HANDLER_AUX):
            InstancePtr->AuxCallback = (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->AuxRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS1_HANDLER_AUD):
            InstancePtr->AudCallback = (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->AudRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        case (XV_HDMIRXSS1_HANDLER_LNKSTA):
            InstancePtr->LnkStaCallback = (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->LnkStaRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Ddc */
        case (XV_HDMIRXSS1_HANDLER_DDC):
            InstancePtr->DdcCallback =(XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->DdcRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Stream down */
        case (XV_HDMIRXSS1_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Stream Init */
        case (XV_HDMIRXSS1_HANDLER_STREAM_INIT):
            InstancePtr->StreamInitCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->StreamInitRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Stream up */
        case (XV_HDMIRXSS1_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Phy reset */
        case (XV_HDMIRXSS1_HANDLER_PHY_RESET):
            InstancePtr->PhyResetCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->PhyResetRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* TMDS_CLK_RATIO */
        case (XV_HDMIRXSS1_HANDLER_TMDS_CLK_RATIO):
            InstancePtr->TmdsClkRatioCallback =
                                  (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->TmdsClkRatioRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* VIC_ERROR */
        case (XV_HDMIRXSS1_HANDLER_VIC_ERROR):
            InstancePtr->VicErrorCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->VicErrorRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

	/* FRL Config */
	case (XV_HDMIRXSS1_HANDLER_FRL_CONFIG):
            InstancePtr->FrlConfigCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->FrlConfigRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

	/* FRL Start */
	case (XV_HDMIRXSS1_HANDLER_FRL_START):
            InstancePtr->FrlStartCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->FrlStartRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

	/* TMDS Config */
	case (XV_HDMIRXSS1_HANDLER_TMDS_CONFIG):
            InstancePtr->TmdsConfigCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->TmdsConfigRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* HDCP */
        case (XV_HDMIRXSS1_HANDLER_HDCP):
            InstancePtr->HdcpCallback = (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->HdcpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* HDCP authenticated */
        case (XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                  XHDCP1X_HANDLER_AUTHENTICATED,
                                  (void *)(XHdcp1x_Callback)CallbackFunc,
                                  (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATED,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        /* HDCP authenticated */
        case (XV_HDMIRXSS1_HANDLER_HDCP_UNAUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                XHDCP1X_HANDLER_UNAUTHENTICATED,
                                (void *)(XHdcp1x_Callback)CallbackFunc,
                                (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_UNAUTHENTICATED,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        /* HDCP authentication request */
        case (XV_HDMIRXSS1_HANDLER_HDCP_AUTHENTICATION_REQUEST):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
        /*Register the hdcp trigger downstream authentication callback */
               XHdcp1x_SetCallBack(InstancePtr->Hdcp14Ptr,
                                   (XHdcp1x_HandlerType) XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
                                   (void *) (XHdcp1x_Callback)CallbackFunc,
                                   (void *) CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATION_REQUEST,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        /* HDCP stream management request */
        case (XV_HDMIRXSS1_HANDLER_HDCP_STREAM_MANAGE_REQUEST):
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks*/
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_STREAM_MANAGE_REQUEST,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        /* HDCP topology update request */
        case (XV_HDMIRXSS1_HANDLER_HDCP_TOPOLOGY_UPDATE):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks*/
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                  XHDCP1X_HANDLER_TOPOLOGY_UPDATE,
                                  (void *)(XHdcp1x_Callback)CallbackFunc,
                                  (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_TOPOLOGY_UPDATE,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        /* HDCP encryption status update */
        case (XV_HDMIRXSS1_HANDLER_HDCP_ENCRYPTION_UPDATE):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                 XHDCP1X_HANDLER_ENCRYPTION_UPDATE,
                                 (void *)(XHdcp1x_Callback)CallbackFunc,
                                 (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_ENCRYPTION_UPDATE,
                                    (void *)(XHdcp22_Rx_RunHandler)CallbackFunc,
                                    (void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

	/* Vfp Change */
	case (XV_HDMIRXSS1_HANDLER_VFP_CH):
            InstancePtr->VfpChangeCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->VfpChangeRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

	/* VRR ready */
	case (XV_HDMIRXSS1_HANDLER_VRR_RDY):
            InstancePtr->VrrRdyCallback =
			    (XV_HdmiRxSs1_Callback)CallbackFunc;
            InstancePtr->VrrRdyRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

	/* Dynamic HDR  */
	case (XV_HDMIRXSS1_HANDLER_DYN_HDR):
	    InstancePtr->DynHdrCallback =
	    (XV_HdmiRxSs1_Callback)CallbackFunc;
	    InstancePtr->DynHdrRef = CallbackRef;
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
* This function installs an asynchronous callback function for the LogWrite
* API:
*
* @param    InstancePtr is a pointer to the HDMI RX Subsystem instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when callback function is not installed
*                successfully.
*
******************************************************************************/
int XV_HdmiRxSs1_SetLogCallback(XV_HdmiRxSs1 *InstancePtr,
    u64 *CallbackFunc,
    void *CallbackRef)
{
	u32 Status = (XST_FAILURE);

    InstancePtr->LogWriteCallback = (XV_HdmiRxSs1_LogCallback)CallbackFunc;
    InstancePtr->LogWriteRef = CallbackRef;
    Status = (XST_SUCCESS);

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
void XV_HdmiRxSs1_SetEdidParam(XV_HdmiRxSs1 *InstancePtr, u8 *EdidDataPtr,
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
void XV_HdmiRxSs1_LoadDefaultEdid(XV_HdmiRxSs1 *InstancePtr)
{
    u32 Status;

    /* Load new EDID*/
    Status = XV_HdmiRx1_DdcLoadEdid(InstancePtr->HdmiRx1Ptr, InstancePtr->EdidPtr,
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
void XV_HdmiRxSs1_LoadEdid(XV_HdmiRxSs1 *InstancePtr, u8 *EdidDataPtr,
		u16 Length)
{
    u32 Status;

    /* Load new EDID*/
    Status = XV_HdmiRx1_DdcLoadEdid(InstancePtr->HdmiRx1Ptr, EdidDataPtr,
		    Length);

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
void XV_HdmiRxSs1_SetHpd(XV_HdmiRxSs1 *InstancePtr, u8 Value)
{
  /* Drive HDMI RX HPD based on the input value */
  XV_HdmiRx1_SetHpd(InstancePtr->HdmiRx1Ptr, Value);
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
void XV_HdmiRxSs1_ToggleHpd(XV_HdmiRxSs1 *InstancePtr)
{
  /* Clear SCDC variables */
  XV_HdmiRx1_DdcScdcClear(InstancePtr->HdmiRx1Ptr);

  /* Disable the scrambler */
  XV_HdmiRx1_SetScrambler(InstancePtr->HdmiRx1Ptr, (FALSE));

  /* Drive HDMI RX HPD Low */
  XV_HdmiRx1_SetHpd(InstancePtr->HdmiRx1Ptr, (FALSE));

  /* Wait 500 ms */
  XV_HdmiRxSs1_WaitUs(InstancePtr, 500000);

  /* Drive HDMI RX HPD High */
  XV_HdmiRx1_SetHpd(InstancePtr->HdmiRx1Ptr, (TRUE));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS Aux structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux *XV_HdmiRxSs1_GetAuxiliary(XV_HdmiRxSs1 *InstancePtr)
{
    return (&(InstancePtr->HdmiRx1Ptr->Aux));
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
XHdmiC_AVI_InfoFrame *XV_HdmiRxSs1_GetAviInfoframe(XV_HdmiRxSs1 *InstancePtr)
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
XHdmiC_GeneralControlPacket *XV_HdmiRxSs1_GetGCP(XV_HdmiRxSs1 *InstancePtr)
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
XHdmiC_AudioInfoFrame *XV_HdmiRxSs1_GetAudioInfoframe(XV_HdmiRxSs1 *InstancePtr)
{
    return (&(InstancePtr->AudioInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI 2.1 RX SS DRM InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return XHdmiC_DRMInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_DRMInfoFrame *XV_HdmiRxSs1_GetDrmInfoframe(XV_HdmiRxSs1 *InstancePtr)
{
    return &InstancePtr->DrmInfoframe;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS Vendor Specific InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return XHdmiC_VSIF pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_VSIF *XV_HdmiRxSs1_GetVSIF(XV_HdmiRxSs1 *InstancePtr)
{
    return (&(InstancePtr->VSIF));
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
u32 XV_HdmiRxSs1_SetStream(XV_HdmiRxSs1 *InstancePtr,
        u32 Clock, u32 LineRate)
{

    LineRate = LineRate;

    if (Clock == 0) {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  /* Write log */
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_SETSTREAM_ERR, 0);
#endif

        return (XST_FAILURE);
    }

#ifdef XV_HDMIRXSS1_LOG_ENABLE
  /* Write log */
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_SETSTREAM, 0);
#endif
  /* Set stream */
  XV_HdmiRx1_SetStream(InstancePtr->HdmiRx1Ptr,
		  InstancePtr->Config.Ppc, Clock);

  /* In case the TMDS clock ratio is 1/40 */
  /* The reference clock must be compensated */
  if (XV_HdmiRx1_GetTmdsClockRatio(InstancePtr->HdmiRx1Ptr)) {
      InstancePtr->HdmiRx1Ptr->Stream.RefClk =
                   InstancePtr->HdmiRx1Ptr->Stream.RefClk * 4;
  }

  return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_HdmiRxSs1_GetVideoStream(XV_HdmiRxSs1 *InstancePtr)
{
    return (&InstancePtr->HdmiRx1Ptr->Stream.Video);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return VIC
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs1_GetVideoIDCode(XV_HdmiRxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiRx1Ptr->Stream.Vic);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Stream Type  1:HDMI 0:DVI
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs1_GetVideoStreamType(XV_HdmiRxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiRx1Ptr->Stream.IsHdmi);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Transport Mode  1:FRL 0:TMDS
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs1_GetTransportMode(XV_HdmiRxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiRx1Ptr->Stream.IsFrl);
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI RX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Stream Type  1:IsScrambled 0: not Scrambled
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs1_GetVideoStreamScramblingFlag(XV_HdmiRxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiRx1Ptr->Stream.IsScrambled);
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS number of active audio channels
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiRxSs1_GetAudioChannels(XV_HdmiRxSs1 *InstancePtr)
{
    return (InstancePtr->AudioChannels);
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS audio format
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
XV_HdmiRx1_AudioFormatType XV_HdmiRxSs1_GetAudioFormat(XV_HdmiRxSs1 *InstancePtr)
{
    return ((XV_HdmiRx1_AudioFormatType)(InstancePtr->HdmiRx1Ptr->AudFormat));
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS CTS value
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiRxSs1_GetAudioAcrCtsVal(XV_HdmiRxSs1 *InstancePtr)
{
    return XV_HdmiRx1_GetAcrCts(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
*
* This function returns the HDMI RX SS CTS value
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return Channels
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiRxSs1_GetAudioAcrNVal(XV_HdmiRxSs1 *InstancePtr)
{
    return XV_HdmiRx1_GetAcrN(InstancePtr->HdmiRx1Ptr);
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
void XV_HdmiRxSs1_RefClockChangeInit(XV_HdmiRxSs1 *InstancePtr)
{
  /* Set TMDS Clock ratio*/
  InstancePtr->TMDSClockRatio =
    XV_HdmiRx1_GetTmdsClockRatio(InstancePtr->HdmiRx1Ptr);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
  XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_REFCLOCKCHANGE, 0);
#endif
}

/******************************************************************************/
/**
*
* This function prints debug information on STDIO/UART console.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_DebugInfo(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_DebugInfo(InstancePtr->HdmiRx1Ptr);
}

/******************************************************************************/
/**
*
* This function prints out RX's SCDC registers and values on STDIO/UART
* console.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_DdcRegDump(XV_HdmiRxSs1 *InstancePtr)
{
	XV_HdmiRx1_DdcRegDump(InstancePtr->HdmiRx1Ptr);
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
void XV_HdmiRxSs1_ReportTiming(XV_HdmiRxSs1 *InstancePtr)
{
	if (InstancePtr->HdmiRx1Ptr->Stream.IsHdmi == FALSE) {
		xil_printf("HDMI RX Mode - DVI");
	} else {
		xil_printf("HDMI RX Mode - HDMI ");
		if (InstancePtr->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
			xil_printf("TMDS");
		} else {
			xil_printf("FRL (%d lanes @ %d Gbps)",
					InstancePtr->HdmiRx1Ptr->Stream.Frl.Lanes,
					InstancePtr->HdmiRx1Ptr->Stream.Frl.LineRate);
		}
	}

	xil_printf("\r\n");

    /* Check is the RX stream is up*/
    if (XV_HdmiRx1_IsStreamUp(InstancePtr->HdmiRx1Ptr)) {
        XV_HdmiRx1_Info(InstancePtr->HdmiRx1Ptr);
        xil_printf("VIC: %0d\r\n", InstancePtr->HdmiRx1Ptr->Stream.Vic);
        xil_printf("Scrambled: %0d\r\n",
            (XV_HdmiRx1_IsStreamScrambled(InstancePtr->HdmiRx1Ptr)));
    }

    /* No stream*/
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
void XV_HdmiRxSs1_ReportLinkQuality(XV_HdmiRxSs1 *InstancePtr)
{
	u8 Channel;
	u32 Errors;
	u16 Data = 0;

	if (InstancePtr->HdmiRx1Ptr->Stream.IsFrl != TRUE) {
		for (Channel = 0; Channel < 3; Channel++)
		{
			Errors = XV_HdmiRx1_GetLinkStatus(InstancePtr->HdmiRx1Ptr,
					Channel);

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
	} else {
		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_MSB));
		xil_printf("Channel 0 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (Data & 0x80) {
			Data &= 0x7F;
			Data = (Data << 8) |
					XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH0_ERRCNT_LSB));
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_MSB));
		xil_printf("Channel 1 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (Data & 0x80) {
			Data &= 0x7F;
			Data = (Data << 8) |
					XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH1_ERRCNT_LSB));
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_MSB));
		xil_printf("Channel 2 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (Data & 0x80) {
			Data &= 0x7F;
			Data = (Data << 8) |
					XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH2_ERRCNT_LSB));
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_MSB));
		xil_printf("Channel 3 CED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (Data & 0x80) {
			Data &= 0x7F;
			Data = (Data << 8) |
					XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_CH3_ERRCNT_LSB));
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}

		Data = XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_RSCCNT_MSB));
		xil_printf("RSED: ");
		/* Valid only when the valid bit (0x80) is set */
		if (Data & 0x80) {
			Data &= 0x7F;
			Data = (Data << 8) |
					XV_HdmiRx1_FrlDdcReadField(InstancePtr->HdmiRx1Ptr,
					(XV_HDMIRX1_SCDCFIELD_RSCCNT_LSB));
			xil_printf("%d\r\n", Data);
		} else {
			xil_printf("Invalid\r\n", Data);
		}
	}

	xil_printf("\r\n");

	/* Clear link error counters */
	XV_HdmiRx1_ClearLinkStatus(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
*
* This function prints the HDMI 2.1 RX SS static HDR infoframe information
*
* @param  InstancePtr pointer to XV_HdmiRxSs1 instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_ReportDRMInfo(XV_HdmiRxSs1 *InstancePtr)
{
	XHdmiC_DRMInfoFrame *DrmInfoFramePtr;

	Xil_AssertVoid(InstancePtr);

	DrmInfoFramePtr = XV_HdmiRxSs1_GetDrmInfoframe(InstancePtr);
	if (DrmInfoFramePtr->EOTF != 0xFF)
		xil_printf("eotf: %d\r\n", DrmInfoFramePtr->EOTF);

	if (DrmInfoFramePtr->Static_Metadata_Descriptor_ID == 0xFF) {
		xil_printf("No DRM info\r\n");
		return;
	}

	xil_printf("DRM IF info:\n");
	xil_printf("desc id: %d\r\n",
		   DrmInfoFramePtr->Static_Metadata_Descriptor_ID);
	xil_printf("display primaries x0, y0, x1, y1, x2, y2: %d %d %d %d %d %d\r\n",
		   DrmInfoFramePtr->disp_primaries[0].x,
		   DrmInfoFramePtr->disp_primaries[0].y,
		   DrmInfoFramePtr->disp_primaries[1].x,
		   DrmInfoFramePtr->disp_primaries[1].y,
		   DrmInfoFramePtr->disp_primaries[2].x,
		   DrmInfoFramePtr->disp_primaries[2].y);
	xil_printf("white point x, y: %d %d\r\n",
		   DrmInfoFramePtr->white_point.x,
		   DrmInfoFramePtr->white_point.y);
	xil_printf("min/max display mastering luminance: %d %d\r\n",
		   DrmInfoFramePtr->Min_Disp_Mastering_Luminance,
		   DrmInfoFramePtr->Max_Disp_Mastering_Luminance);
	xil_printf("Max_CLL: %d\r\n",
		   DrmInfoFramePtr->Max_Content_Light_Level);
	xil_printf("max_fall: %d\r\n",
		   DrmInfoFramePtr->Max_Frame_Average_Light_Level);
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
void XV_HdmiRxSs1_ReportAudio(XV_HdmiRxSs1 *InstancePtr)
{
  xil_printf("Format   : ");
  switch (XV_HdmiRxSs1_GetAudioFormat(InstancePtr)) {
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
	  default:
		  break;
  }
  xil_printf("Channels : %d\r\n",
  XV_HdmiRx1_GetAudioChannels(InstancePtr->HdmiRx1Ptr));
  xil_printf("ACR CTS  : %d\r\n",
		  XV_HdmiRx1_GetAcrCts(InstancePtr->HdmiRx1Ptr));
  xil_printf("ACR N    : %d\r\n",
		  XV_HdmiRx1_GetAcrN(InstancePtr->HdmiRx1Ptr));
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
void XV_HdmiRxSs1_ReportInfoFrame(XV_HdmiRxSs1 *InstancePtr)
{
  xil_printf("RX header: %0x\r\n", InstancePtr->HdmiRx1Ptr->Aux.Header.Data);
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
void XV_HdmiRxSs1_ReportSubcoreVersion(XV_HdmiRxSs1 *InstancePtr)
{
  u32 Data;

  if (InstancePtr->HdmiRx1Ptr)
  {
     Data = XV_HdmiRx1_GetVersion(InstancePtr->HdmiRx1Ptr);
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
int XV_HdmiRxSs1_IsStreamUp(XV_HdmiRxSs1 *InstancePtr)
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
int XV_HdmiRxSs1_IsStreamConnected(XV_HdmiRxSs1 *InstancePtr)
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
static void XV_HdmiRxSs1_ConfigBridgeMode(XV_HdmiRxSs1 *InstancePtr) {
    XVidC_VideoStream *HdmiRxSs1VidStreamPtr;
    HdmiRxSs1VidStreamPtr = XV_HdmiRxSs1_GetVideoStream(InstancePtr);

    XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
    AviInfoFramePtr = XV_HdmiRxSs1_GetAviInfoframe(InstancePtr);

    if ((!InstancePtr->HdmiRx1Ptr->Stream.IsHdmi) &&
		    HdmiRxSs1VidStreamPtr->IsInterlaced) {
	if ((HdmiRxSs1VidStreamPtr->Timing.HActive == 1440) &&
			((HdmiRxSs1VidStreamPtr->Timing.VActive == 288) ||
			 (HdmiRxSs1VidStreamPtr->Timing.VActive == 240))) {
             XV_HdmiRxSs1_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiRxSs1_BridgePixelDrop(InstancePtr, TRUE);

	     return;
	}
    }

    /* Pixel Repetition factor of 3 and above are not supported by the bridge*/
    if (AviInfoFramePtr->PixelRepetition > XHDMIC_PIXEL_REPETITION_FACTOR_2) {
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    	XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_PIX_REPEAT_ERR,
    			AviInfoFramePtr->PixelRepetition);
#endif

    	return;
    }

    if (HdmiRxSs1VidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
         XV_HdmiRxSs1_BridgePixelDrop(InstancePtr, FALSE);
         XV_HdmiRxSs1_BridgeYuv420(InstancePtr, TRUE);
    }
    else {
        if (AviInfoFramePtr->PixelRepetition ==
				XHDMIC_PIXEL_REPETITION_FACTOR_2)
        {
            /*********************************************************
             * NTSC/PAL Support
             *********************************************************/
             XV_HdmiRxSs1_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiRxSs1_BridgePixelDrop(InstancePtr, TRUE);
        }
        else {
            XV_HdmiRxSs1_BridgeYuv420(InstancePtr, FALSE);
            XV_HdmiRxSs1_BridgePixelDrop(InstancePtr, FALSE);
        }
    }
}

/*****************************************************************************/
/**
* This function will set the default in HDF.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
* @param    Id is the XV_HdmiRxSs1 ID to operate on.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_SetDefaultPpc(XV_HdmiRxSs1 *InstancePtr, u8 Id) {
    extern XV_HdmiRxSs1_Config XV_HdmiRxSs1_ConfigTable[XPAR_XV_HDMIRXSS1_NUM_INSTANCES];
    InstancePtr->Config.Ppc = XV_HdmiRxSs1_ConfigTable[Id].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
* @param    Id is the XV_HdmiRxSs1 ID to operate on.
* @param    Ppc is the PPC to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiRxSs1_SetPpc(XV_HdmiRxSs1 *InstancePtr, u8 Id, u8 Ppc) {
    InstancePtr->Config.Ppc = (XVidC_PixelsPerClock) Ppc;
    Id = Id; /*squash unused variable compiler warning*/
}

/*****************************************************************************/
/**
* This function will set the major and minor application version in RXSs struct
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 core instance.
* @param    maj is the major version of the application.
* @param    min is the minor version of the application.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiRxSS1_SetAppVersion(XV_HdmiRxSs1 *InstancePtr, u8 maj, u8 min)
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
void XV_HdmiRxSs1_AudioMute(XV_HdmiRxSs1 *InstancePtr, u8 Enable)
{
  /*Audio Mute Mode*/
  if (Enable){
	XV_HdmiRx1_AudioDisable(InstancePtr->HdmiRx1Ptr);
  }
  else{
	XV_HdmiRx1_AudioEnable(InstancePtr->HdmiRx1Ptr);
  }
}

/*****************************************************************************/
/**
*
* This function controls HDMI RX VFP event enable/disable
*
* @param  Enable 0: disable VFP event 1: enable VFP event.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_VfpControl(XV_HdmiRxSs1 *InstancePtr, u8 Enable)
{
	XV_HdmiRx1_VtdVfpEvent(InstancePtr->HdmiRx1Ptr, Enable);
}

/*****************************************************************************/
/**
*
* This function returns VRR infoframe type
*
* @param    InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return XV_HdmiRx1_VrrInfoframeType
*
* @note   None.
*
******************************************************************************/
XV_HdmiC_VrrInfoFrame *XV_HdmiRxSs1_GetVrrIf(XV_HdmiRxSs1 *InstancePtr)
{
	return &(InstancePtr->HdmiRx1Ptr->VrrIF);
}

/*****************************************************************************/
/**
*
* This function returns core ppc value
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return Core pixel per clock value
*
* @note   None.
*
******************************************************************************/
XVidC_PixelsPerClock XV_HdmiRxSs1_GetCorePpc(XV_HdmiRxSs1 *InstancePtr)
{
	return InstancePtr->HdmiRx1Ptr->Stream.CorePixPerClk;
}

/*****************************************************************************/
/**
*
* This function enables the Data mover in Dynamic HDR
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_DynHDR_DM_Enable(XV_HdmiRxSs1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRxSs1 Dynamic HDR disabled\r\n");
		return;
	}

	XV_HdmiRx1_DynHDR_DM_Enable(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
*
* This function disables the Data mover in Dynamic HDR
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_DynHDR_DM_Disable(XV_HdmiRxSs1 *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRxSs1 Dynamic HDR disabled\r\n");
		return;
	}

	XV_HdmiRx1_DynHDR_DM_Disable(InstancePtr->HdmiRx1Ptr);
}

/*****************************************************************************/
/**
*
* This function sets the buffer address for Dyanamic HDR
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
* @param  Addr 64 bit address
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_DynHDR_SetAddr(XV_HdmiRxSs1 *InstancePtr, u64 Addr)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(Addr);
	/* 64 bit aligned address */
	Xil_AssertVoid(!(Addr & 0x3F));

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRxSs1 Dynamic HDR disabled\r\n");
		return;
	}

	XV_HdmiRx1_DynHDR_SetAddr(InstancePtr->HdmiRx1Ptr, Addr);
}

/*****************************************************************************/
/**
*
* This function sets the buffer address for Dyanamic HDR
*
* @param  InstancePtr pointer to XV_HdmiRXSs instance
* @param  RxDynHdrInfo pointer to dynamic hdr info structure. This is passed
*	  by the application.
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_DynHDR_GetInfo(XV_HdmiRxSs1 *InstancePtr,
				 XV_HdmiRxSs1_DynHDR_Info *RxDynHdrInfo)
{
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(RxDynHdrInfo);

	if (!InstancePtr->Config.DynamicHDR) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			    "\r\nWarning: HdmiRxSs1 Dynamic HDR disabled\r\n");
		return;
	}

	XV_HdmiRx1_DynHDR_GetInfo(InstancePtr->HdmiRx1Ptr,
				  (XV_HdmiRx1_DynHDR_Info *)RxDynHdrInfo);
}

static void XV_HdmiRxSs1_FrlLtsLCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTSL, 0);
#endif
}

static void XV_HdmiRxSs1_FrlLts1Callback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTS1, 0);
#endif
}

static void XV_HdmiRxSs1_FrlLts2Callback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTS2, 0);
#endif
}

static void XV_HdmiRxSs1_FrlLts3Callback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTS3,
			(HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.Frl.FfeLevels << 4) |
			HdmiRxSs1Ptr->HdmiRx1Ptr->Stream.Frl.LineRate);
#endif
}

static void XV_HdmiRxSs1_FrlLts4Callback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTS4, 0);
#endif
}

static void XV_HdmiRxSs1_FrlLtsPCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_FRL_LTSP, 0);
#endif
}

static void XV_HdmiRxSs1_VfpChanged(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

	if (HdmiRxSs1Ptr->VfpChangeCallback)
		HdmiRxSs1Ptr->VfpChangeCallback(HdmiRxSs1Ptr->VfpChangeRef);
}

static void XV_HdmiRxSs1_VrrReady(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;
#ifdef XV_HDMIRXSS1_LOG_ENABLE
	XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_VRR_RDY, 0);
#endif
	if (HdmiRxSs1Ptr->VrrRdyCallback)
		HdmiRxSs1Ptr->VrrRdyCallback(HdmiRxSs1Ptr->VrrRdyRef);
}

static void XV_HdmiRxSs1_DynHdrEvtCallback(void *CallbackRef)
{
	XV_HdmiRxSs1 *HdmiRxSs1Ptr = (XV_HdmiRxSs1 *)CallbackRef;

	if (HdmiRxSs1Ptr->DynHdrCallback)
		HdmiRxSs1Ptr->DynHdrCallback(HdmiRxSs1Ptr->DynHdrRef);
}
