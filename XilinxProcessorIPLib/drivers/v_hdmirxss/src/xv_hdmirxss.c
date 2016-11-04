/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* 1.19  MG     31/10/16 Fixed issue with reference clock compensation in XV_HdmiRxSS_SetStream
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xenv.h"
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
  XGpio RemapperReset;
  XV_axi4s_remap Remapper;
} XV_HdmiRxSs_SubCores;

/**************************** Local Global ***********************************/
/** Define Driver instance of all sub-core included in the design */
XV_HdmiRxSs_SubCores XV_HdmiRxSs_SubCoreRepo[XPAR_XV_HDMIRXSS_NUM_INSTANCES];
/** Vendor-Specific InfoFrame structure */
XV_HdmiRx_VSIF VSIF;

/************************** Function Prototypes ******************************/
static void XV_HdmiRxSs_GetIncludedSubcores(XV_HdmiRxSs *HdmiRxSsPtr,
    u16 DevId);
static void XV_HdmiRxSs_WaitUs(XV_HdmiRxSs *InstancePtr, u32 MicroSeconds);
static void XV_HdmiRxSs_RetrieveVSInfoframe(XV_HdmiRx *HdmiRxPtr);
static int XV_HdmiRxSs_RegisterSubsysCallbacks(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiRxSs_AuxCallback(void *CallbackRef);
static void XV_HdmiRxSs_AudCallback(void *CallbackRef);
static void XV_HdmiRxSs_LnkStaCallback(void *CallbackRef);
static void XV_HdmiRxSs_DdcCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamDownCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamInitCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamUpCallback(void *CallbackRef);

static void XV_HdmiRxSs_ReportCoreInfo(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportTiming(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportLinkQuality(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportAudio(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportInfoFrame(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ReportSubcoreVersion(XV_HdmiRxSs *InstancePtr);

// HDCP specific
#ifdef USE_HDCP
static XV_HdmiRxSs_HdcpEvent XV_HdmiRxSs_HdcpGetEvent(XV_HdmiRxSs *InstancePtr);
static int XV_HdmiRxSs_HdcpProcessEvents(XV_HdmiRxSs *InstancePtr);
static int XV_HdmiRxSs_HdcpReset(XV_HdmiRxSs *InstancePtr);
static int XV_HdmiRxSs_HdcpSetTopologyDepth(XV_HdmiRxSs *InstancePtr, u32 Depth);
static int XV_HdmiRxSs_HdcpSetTopologyDeviceCnt(XV_HdmiRxSs *InstancePtr, u32 DeviceCnt);
static int XV_HdmiRxSs_HdcpSetTopologyMaxDevsExceeded(XV_HdmiRxSs *InstancePtr, u8 Value);
static int XV_HdmiRxSs_HdcpSetTopologyMaxCascadeExceeded(XV_HdmiRxSs *InstancePtr, u8 Value);
static int XV_HdmiRxSs_HdcpSetTopologyHdcp20RepeaterDownstream(XV_HdmiRxSs *InstancePtr, u8 Value);
static int XV_HdmiRxSs_HdcpSetTopologyHdcp1DeviceDownstream(XV_HdmiRxSs *InstancePtr, u8 Value);
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
static u32 XV_HdmiRxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
                                            u32 ClockFrequency);
static void XV_HdmiRxSs_HdcpTimerCallback(void *CallBackRef, u8 TimerChannel);
#endif

static void XV_HdmiRxSs_ResetRemapper(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ConfigRemapper(XV_HdmiRxSs *InstancePtr);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definition ******************************/

void XV_HdmiRxSs_ReportInfo(XV_HdmiRxSs *InstancePtr)
{
    xil_printf("------------\n\r");
    xil_printf("HDMI RX SubSystem\n\r");
	xil_printf("------------\n\r");
    XV_HdmiRxSs_ReportCoreInfo(InstancePtr);
    XV_HdmiRxSs_ReportSubcoreVersion(InstancePtr);
	xil_printf("\n\r");
    xil_printf("HDMI RX timing\n\r");
    xil_printf("------------\n\r");
    XV_HdmiRxSs_ReportTiming(InstancePtr);
    xil_printf("Link quality\n\r");
    xil_printf("---------\n\r");
    XV_HdmiRxSs_ReportLinkQuality(InstancePtr);
    xil_printf("Audio\n\r");
    xil_printf("---------\n\r");
    XV_HdmiRxSs_ReportAudio(InstancePtr);
    xil_printf("Infoframe\n\r");
    xil_printf("---------\n\r");
    XV_HdmiRxSs_ReportInfoFrame(InstancePtr);
    xil_printf("\n\r");
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
void XV_HdmiRxSs_ReportCoreInfo(XV_HdmiRxSs *InstancePtr)
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
 * This function installs a custom delay/sleep function to be used by the XV_HdmiRxSs
 * driver.
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
 * This function is the delay/sleep function for the XV_HdmiRxSs driver. For the Zynq
 * family, there exists native sleep functionality. For MicroBlaze however,
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

#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS_HdcpIntrHandler(XV_HdmiRxSs *InstancePtr)
{
    XHdcp1x_CipherIntrHandler(InstancePtr->Hdcp14Ptr);
}
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP Timer
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS_HdcpTimerIntrHandler(XV_HdmiRxSs *InstancePtr)
{
    XTmrCtr_InterruptHandler(InstancePtr->HdcpTimerPtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP 2.2 Timer
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS_Hdcp22TimerIntrHandler(XV_HdmiRxSs *InstancePtr)
{
  XTmrCtr *XTmrCtrPtr;

  XTmrCtrPtr = XHdcp22Rx_GetTimer(InstancePtr->Hdcp22Ptr);

  XTmrCtr_InterruptHandler(XTmrCtrPtr);
}
#endif

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
                          XV_HdmiRxSs_ConnectCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_AUX,
                          XV_HdmiRxSs_AuxCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_AUD,
                          XV_HdmiRxSs_AudCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_LNKSTA,
                          XV_HdmiRxSs_LnkStaCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_DDC,
                          XV_HdmiRxSs_DdcCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_DOWN,
                          XV_HdmiRxSs_StreamDownCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_INIT,
                          XV_HdmiRxSs_StreamInitCallback,
                          InstancePtr);

    XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                          XV_HDMIRX_HANDLER_STREAM_UP,
                          XV_HdmiRxSs_StreamUpCallback,
                          InstancePtr);
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
  HdmiRxSsPtr->HdmiRxPtr        = ((HdmiRxSsPtr->Config.HdmiRx.IsPresent) ?
                                   (&XV_HdmiRxSs_SubCoreRepo[DevId].HdmiRx) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
  HdmiRxSsPtr->Hdcp14Ptr        = ((HdmiRxSsPtr->Config.Hdcp14.IsPresent) ?
                                   (&XV_HdmiRxSs_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiRxSsPtr->HdcpTimerPtr     = ((HdmiRxSsPtr->Config.HdcpTimer.IsPresent) ?
                                  (&XV_HdmiRxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  HdmiRxSsPtr->Hdcp22Ptr        = ((HdmiRxSsPtr->Config.Hdcp22.IsPresent) ?
                                   (&XV_HdmiRxSs_SubCoreRepo[DevId].Hdcp22) : NULL);
#endif
  HdmiRxSsPtr->RemapperResetPtr = ((HdmiRxSsPtr->Config.RemapperReset.IsPresent) ?
                                   (&XV_HdmiRxSs_SubCoreRepo[DevId].RemapperReset) : NULL);
  HdmiRxSsPtr->RemapperPtr      = ((HdmiRxSsPtr->Config.Remapper.IsPresent) ?
                                   (&XV_HdmiRxSs_SubCoreRepo[DevId].Remapper) : NULL);
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

  // Remapper
  if (HdmiRxSsPtr->RemapperResetPtr) {
    if (XV_HdmiRxSs_SubcoreInitRemapperReset(HdmiRxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if (HdmiRxSsPtr->RemapperPtr) {
    if (XV_HdmiRxSs_SubcoreInitRemapper(HdmiRxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  /* Register Callbacks */
  XV_HdmiRxSs_RegisterSubsysCallbacks(HdmiRxSsPtr);

  /* Default value */
  HdmiRxSsPtr->HdcpIsReady = (FALSE);

#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when both HDCP cores are instantiated and all keys are loaded */
  if (HdmiRxSsPtr->Hdcp14Ptr && HdmiRxSsPtr->Hdcp22Ptr &&
      HdmiRxSsPtr->Hdcp22Lc128Ptr && HdmiRxSsPtr->Hdcp14KeyPtr && HdmiRxSsPtr->Hdcp22PrivateKeyPtr) {
    HdmiRxSsPtr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs_HdcpSetProtocol(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key is loaded */
  if (!HdmiRxSsPtr->HdcpIsReady && HdmiRxSsPtr->Hdcp14Ptr && HdmiRxSsPtr->Hdcp14KeyPtr) {
    HdmiRxSsPtr->HdcpIsReady = (TRUE);

    /* Set default HDCP content protection scheme */
    XV_HdmiRxSs_HdcpSetProtocol(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_14);
  }
#endif

#if defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the keys are loaded */
  if (!HdmiRxSsPtr->HdcpIsReady && HdmiRxSsPtr->Hdcp22Ptr && HdmiRxSsPtr->Hdcp22Lc128Ptr &&
      HdmiRxSsPtr->Hdcp22PrivateKeyPtr) {
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

  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_START, 0);
  /* Set RX hot plug detect */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, TRUE);

  /* Disable Audio Peripheral */
  XV_HdmiRx_AudioDisable(InstancePtr->HdmiRxPtr);
  XV_HdmiRx_AudioIntrDisable(InstancePtr->HdmiRxPtr);
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

  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_STOP, 0);
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

  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_RESET, 0);

  /* Assert RX reset */
  XV_HdmiRx_Reset(InstancePtr->HdmiRxPtr, TRUE);

  /* Release RX reset */
  XV_HdmiRx_Reset(InstancePtr->HdmiRxPtr, FALSE);
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
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_CONNECT, 0);

    // Set RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, TRUE);

    // Set stream connected flag
    HdmiRxSsPtr->IsStreamConnected = (TRUE);

#ifdef USE_HDCP
    // Push connect event to HDCP event queue
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_CONNECT_EVT);
#endif
  }

  // RX cable is disconnected
  else {
    XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_DISCONNECT, 0);

    // Clear RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, FALSE);

    // Set stream connected flag
    HdmiRxSsPtr->IsStreamConnected = (FALSE);

#ifdef USE_HDCP
    // Push disconnect event to HDCP event queue
    XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_DISCONNECT_EVT);
#endif

    XV_HdmiRx_SetScrambler(HdmiRxSsPtr->HdmiRxPtr, (FALSE)); // Disable scrambler
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

  // Retrieve Vendor Specific Info Frame
  XV_HdmiRxSs_RetrieveVSInfoframe(HdmiRxSsPtr->HdmiRxPtr);

  // Check if user callback has been registered
  if (HdmiRxSsPtr->AuxCallback) {
      HdmiRxSsPtr->AuxCallback(HdmiRxSsPtr->AuxRef);
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
static void XV_HdmiRxSs_RetrieveVSInfoframe(XV_HdmiRx *HdmiRx)
{
  if (HdmiRx->Aux.Header.Byte[0] == 0x81) {
      XV_HdmiRx_VSIF_ParsePacket(&HdmiRx->Aux, &VSIF);

      // Defaults
      HdmiRx->Stream.Video.Is3D = FALSE;
      HdmiRx->Stream.Video.Info_3D.Format = XVIDC_3D_UNKNOWN;

      if (VSIF.Format == XV_HDMIRX_VSIF_VF_3D) {
          HdmiRx->Stream.Video.Is3D = TRUE;
          HdmiRx->Stream.Video.Info_3D = VSIF.Info_3D.Stream;
      } else if (VSIF.Format == XV_HDMIRX_VSIF_VF_EXTRES) {
          switch(VSIF.HDMI_VIC) {
              case 1 :
                  HdmiRx->Stream.Vic = 95;
                  break;

              case 2 :
                  HdmiRx->Stream.Vic = 94;
                  break;

              case 3 :
                  HdmiRx->Stream.Vic = 93;
                  break;

              case 4 :
                  HdmiRx->Stream.Vic = 98;
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

  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_LINKSTATUS, 0);

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

  // Assert HDMI RX reset
  XV_HdmiRx_Reset(HdmiRxSsPtr->HdmiRxPtr, TRUE);

  /* Set stream up flag */
  HdmiRxSsPtr->IsStreamUp = (FALSE);

  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMDOWN, 0);

#ifdef USE_HDCP
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

  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMINIT, 0);

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

  XV_HdmiRxSs_LogWrite(HdmiRxSsPtr, XV_HDMIRXSS_LOG_EVT_STREAMUP, 0);

#ifdef USE_HDCP
  // Push stream-up event to HDCP event queue
  XV_HdmiRxSs_HdcpPushEvent(HdmiRxSsPtr, XV_HDMIRXSS_HDCP_STREAMUP_EVT);
#endif

  if (HdmiRxSsPtr->RemapperResetPtr) {
      /* Toggle AXI_GPIO to Reset Remapper */
      XV_HdmiRxSs_ResetRemapper(HdmiRxSsPtr);
  }

  if (HdmiRxSsPtr->RemapperPtr) {
      /* Configure Remapper according to HW setting and video format */
      XV_HdmiRxSs_ConfigRemapper(HdmiRxSsPtr);
  }

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
int XV_HdmiRxSs_SetCallback(XV_HdmiRxSs *InstancePtr, u32 HandlerType,
    void *CallbackFunc, void *CallbackRef)
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
            InstancePtr->DdcCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->DdcRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream down
        case (XV_HDMIRXSS_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream Init
        case (XV_HDMIRXSS_HANDLER_STREAM_INIT):
            InstancePtr->StreamInitCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamInitRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream up
        case (XV_HDMIRXSS_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback = (XV_HdmiRxSs_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
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
                                  (XHdcp1x_Callback)CallbackFunc,
                                  CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATED,
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
                                (XHdcp1x_Callback)CallbackFunc,
                                CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_UNAUTHENTICATED,
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
                                 XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
                                 CallbackFunc, CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_AUTHENTICATION_REQUEST,
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
                                  (XHdcp1x_Callback)CallbackFunc,
                                  CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_TOPOLOGY_UPDATE,
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
                                 (XHdcp1x_Callback)CallbackFunc,
                                 CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
            // Register HDCP 2.2 callbacks
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Rx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_RX_HANDLER_ENCRYPTION_UPDATE,
                                    (XHdcp22_Rx_RunHandler)CallbackFunc,
                                    CallbackRef);
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
        print("\n\r");
        print("Successfully loaded edid.\n\r");
    }
    else {
        print("\n\r");
        print("Error loading edid.\n\r");
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
        print("\n\r");
        print("Successfully loaded edid.\n\r");
    }
    else {
        print("\n\r");
        print("Error loading edid.\n\r");
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
  /* Drive HPD low */
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
  /* Drive HPD low */
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (FALSE));

  /* Wait 500 ms */
  XV_HdmiRxSs_WaitUs(InstancePtr, 500000);

  /* Drive HPD high */
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
XV_HdmiRx_Aux *XV_HdmiRxSs_GetAuxiliary(XV_HdmiRxSs *InstancePtr)
{
    return (&InstancePtr->HdmiRxPtr->Aux);
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

  /* Write log */
  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_SETSTREAM, 0);

  /* Set stream */
  XV_HdmiRx_SetStream(InstancePtr->HdmiRxPtr, InstancePtr->Config.Ppc, Clock);

  /* In case the TMDS clock ratio is 1/40 */
  /* The reference clock must be compensated */
  if (XV_HdmiRx_GetTmdsClockRatio(InstancePtr->HdmiRxPtr)) {
	  InstancePtr->HdmiRxPtr->Stream.RefClk = InstancePtr->HdmiRxPtr->Stream.RefClk * 4;
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

  XV_HdmiRxSs_LogWrite(InstancePtr, XV_HDMIRXSS_LOG_EVT_REFCLOCKCHANGE, 0);
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
void XV_HdmiRxSs_ReportTiming(XV_HdmiRxSs *InstancePtr)
{
    // Check is the RX stream is up
    if (XV_HdmiRx_IsStreamUp(InstancePtr->HdmiRxPtr)) {
        XV_HdmiRx_DebugInfo(InstancePtr->HdmiRxPtr);
        xil_printf("VIC: %0d\n\r", InstancePtr->HdmiRxPtr->Stream.Vic);
        xil_printf("Scrambled: %0d\n\r",
            (XV_HdmiRx_IsStreamScrambled(InstancePtr->HdmiRxPtr)));
        xil_printf("Audio channels: %0d\n\r",
            (XV_HdmiRx_GetAudioChannels(InstancePtr->HdmiRxPtr)));
        print("\n\r");
    }

    // No stream
    else {
      print("No HDMI RX stream\n\r\n\r");
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
void XV_HdmiRxSs_ReportLinkQuality(XV_HdmiRxSs *InstancePtr)
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

      xil_printf(" (%0d)\n\r", Errors);
  }

  /* Clear link error counters */
  XV_HdmiRx_ClearLinkStatus(InstancePtr->HdmiRxPtr);
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
void XV_HdmiRxSs_ReportAudio(XV_HdmiRxSs *InstancePtr)
{
  xil_printf("Channels : %d\n\r",
    XV_HdmiRx_GetAudioChannels(InstancePtr->HdmiRxPtr));
  xil_printf("ARC CTS : %d\n\r", XV_HdmiRx_GetAcrCts(InstancePtr->HdmiRxPtr));
  xil_printf("ARC N   : %d\n\r", XV_HdmiRx_GetAcrN(InstancePtr->HdmiRxPtr));
  print("\n\r");
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
void XV_HdmiRxSs_ReportInfoFrame(XV_HdmiRxSs *InstancePtr)
{
  xil_printf("RX header: %0x\n\r", InstancePtr->HdmiRxPtr->Aux.Header.Data);
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
     xil_printf("  HDMI RX version : %02d.%02d (%04x)\n\r",
     ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (InstancePtr->Hdcp14Ptr){
     Data = XHdcp1x_GetVersion(InstancePtr->Hdcp14Ptr);
     xil_printf("  HDCP 1.4 RX version : %02d.%02d (%04x)\n\r",
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

#ifdef XPAR_XHDCP_NUM_INSTANCES
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
static u32 XV_HdmiRxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
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
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
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
static void XV_HdmiRxSs_HdcpTimerCallback(void* CallBackRef, u8 TimerChannel)
{
  XHdcp1x* HdcpPtr = CallBackRef;

  XHdcp1x_HandleTimeout(HdcpPtr);
  return;
}
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
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
int XV_HdmiRxSs_HdcpTimerStart(void *InstancePtr, u16 TimeoutInMs)
{
  XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
  XTmrCtr *TimerPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

  u8 TimerChannel = 0;
  u32 TimerOptions = 0;
  u32 NumTicks = 0;

  /* Verify argument. */
  Xil_AssertNonvoid(TimerPtr != NULL);

  /* Determine NumTicks */
  NumTicks = XV_HdmiRxSs_HdcpTimerConvUsToTicks((TimeoutInMs*1000ul),
        TimerPtr->Config.SysClockFreqHz);

  /* Stop it */
  XTmrCtr_Stop(TimerPtr, TimerChannel);

  /* Configure the callback */
  XTmrCtr_SetHandler(TimerPtr, &XV_HdmiRxSs_HdcpTimerCallback,
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
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
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
int XV_HdmiRxSs_HdcpTimerStop(void *InstancePtr)
{
  XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
  XTmrCtr *TimerPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

  u8 TimerChannel = 0;

  /* Verify argument. */
  Xil_AssertNonvoid(TimerPtr != NULL);

  /* Stop it */
  XTmrCtr_Stop(TimerPtr, TimerChannel);

  return (XST_SUCCESS);
}
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
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
int XV_HdmiRxSs_HdcpTimerBusyDelay(void *InstancePtr, u16 DelayInMs)
{

  XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
  XTmrCtr *TimerPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

  u8 TimerChannel = 0;
  u32 TimerOptions = 0;
  u32 NumTicks = 0;

  /* Verify argument. */
	Xil_AssertNonvoid(TimerPtr != NULL);

  /* Determine NumTicks */
  NumTicks = XV_HdmiRxSs_HdcpTimerConvUsToTicks((DelayInMs*1000ul),
                TimerPtr->Config.SysClockFreqHz);

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
#endif

void XV_HdmiRxSs_ResetRemapper(XV_HdmiRxSs *InstancePtr) {

    XGpio *RemapperResetPtr = InstancePtr->RemapperResetPtr;

    XGpio_SetDataDirection(RemapperResetPtr, 1, 0);
    XGpio_DiscreteWrite(RemapperResetPtr, 1, 0);
    XGpio_DiscreteWrite(RemapperResetPtr, 1, 1);
}

void XV_HdmiRxSs_ConfigRemapper(XV_HdmiRxSs *InstancePtr) {

    XV_axi4s_remap *RemapperPtr = InstancePtr->RemapperPtr;

    XVidC_ColorFormat ColorFormat;
    XVidC_VideoMode VideoMode;

    XVidC_VideoStream *HdmiRxSsVidStreamPtr;
    HdmiRxSsVidStreamPtr = XV_HdmiRxSs_GetVideoStream(InstancePtr);

    ColorFormat = HdmiRxSsVidStreamPtr->ColorFormatId;
    VideoMode = HdmiRxSsVidStreamPtr->VmId;

    if (ColorFormat == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
        XV_axi4s_remap_Set_width(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.HActive);
        XV_axi4s_remap_Set_height(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.VActive);
        XV_axi4s_remap_Set_ColorFormat(RemapperPtr, XVIDC_CSF_YCRCB_420);
        XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
        XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
        XV_axi4s_remap_Set_inHDMI420(RemapperPtr, 1);
        XV_axi4s_remap_Set_outHDMI420(RemapperPtr, 0);
        XV_axi4s_remap_Set_inPixDrop(RemapperPtr, 0);
        XV_axi4s_remap_Set_outPixRepeat(RemapperPtr, 0);
        XV_axi4s_remap_EnableAutoRestart(RemapperPtr);
        XV_axi4s_remap_Start(RemapperPtr);
    }
    else {
        if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
            (VideoMode == XVIDC_VM_1440x576_50_I) )
        {
            /*********************************************************
             * NTSC/PAL Support
             *********************************************************/
            XV_axi4s_remap_Set_width(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.HActive);
            XV_axi4s_remap_Set_height(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.VActive);
            XV_axi4s_remap_Set_ColorFormat(RemapperPtr, ColorFormat);
            XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_inHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_outHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_inPixDrop(RemapperPtr, 1);
            XV_axi4s_remap_Set_outPixRepeat(RemapperPtr, 0);
            XV_axi4s_remap_EnableAutoRestart(RemapperPtr);
            XV_axi4s_remap_Start(RemapperPtr);
        }
        else {
            XV_axi4s_remap_Set_width(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.HActive);
            XV_axi4s_remap_Set_height(RemapperPtr, HdmiRxSsVidStreamPtr->Timing.VActive);
            XV_axi4s_remap_Set_ColorFormat(RemapperPtr, ColorFormat);
            XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiRxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_inHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_outHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_inPixDrop(RemapperPtr, 0);
            XV_axi4s_remap_Set_outPixRepeat(RemapperPtr, 0);
            XV_axi4s_remap_EnableAutoRestart(RemapperPtr);
            XV_axi4s_remap_Start(RemapperPtr);
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
    InstancePtr->Config.Ppc = Ppc;
    Id = Id; //squash unused variable compiler warning
}

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function pushes an event into the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Event is the event to be pushed in the queue.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpPushEvent(XV_HdmiRxSs *InstancePtr, XV_HdmiRxSs_HdcpEvent Event)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Event < XV_HDMIRXSS_HDCP_INVALID_EVT);

  /* Write event into the queue */
  InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Head] = Event;

  /* Update head pointer */
  if (InstancePtr->HdcpEventQueue.Head == (XV_HDMIRXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
    InstancePtr->HdcpEventQueue.Head = 0;
  }
  else {
    InstancePtr->HdcpEventQueue.Head++;
  }

  /* Check tail pointer. When the two pointer are equal, then the buffer
   * is full. In this case then increment the tail pointer as well to
   * remove the oldest entry from the buffer.
   */
  if (InstancePtr->HdcpEventQueue.Tail == InstancePtr->HdcpEventQueue.Head) {
    if (InstancePtr->HdcpEventQueue.Tail == (XV_HDMIRXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
      InstancePtr->HdcpEventQueue.Tail = 0;
    }
    else {
      InstancePtr->HdcpEventQueue.Tail++;
    }
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function gets an event from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return When the queue is filled, the next event is returned.
*         When the queue is empty, XV_HDMIRXSS_HDCP_NO_EVT is returned.
*
* @note   None.
*
******************************************************************************/
static XV_HdmiRxSs_HdcpEvent XV_HdmiRxSs_HdcpGetEvent(XV_HdmiRxSs *InstancePtr)
{
  XV_HdmiRxSs_HdcpEvent Event;

  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Check if there are any events in the queue */
  if (InstancePtr->HdcpEventQueue.Tail == InstancePtr->HdcpEventQueue.Head) {
    return XV_HDMIRXSS_HDCP_NO_EVT;
  }

  Event = InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Tail];

  /* Update tail pointer */
  if (InstancePtr->HdcpEventQueue.Tail == (XV_HDMIRXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
    InstancePtr->HdcpEventQueue.Tail = 0;
  }
  else {
    InstancePtr->HdcpEventQueue.Tail++;
  }

  return Event;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function clears all pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpClearEvents(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  InstancePtr->HdcpEventQueue.Head = 0;
  InstancePtr->HdcpEventQueue.Tail = 0;

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function processes pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpProcessEvents(XV_HdmiRxSs *InstancePtr)
{
  XV_HdmiRxSs_HdcpEvent Event;
  int Status = XST_SUCCESS;

  /* Verify argument */
  Xil_AssertNonvoid(InstancePtr != NULL);

  Event = XV_HdmiRxSs_HdcpGetEvent(InstancePtr);

  switch (Event) {

    // Stream up
    case XV_HDMIRXSS_HDCP_STREAMUP_EVT :
      break;

    // Stream down
    case XV_HDMIRXSS_HDCP_STREAMDOWN_EVT :
      break;

    // Connect
    case XV_HDMIRXSS_HDCP_CONNECT_EVT :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        // Set physical state
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, TRUE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
#endif
      XV_HdmiRxSs_HdcpSetProtocol(InstancePtr, InstancePtr->HdcpProtocol);
      break;

    // Disconnect
    // Enable the previous HDCP protocol
    case XV_HDMIRXSS_HDCP_DISCONNECT_EVT :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        // Set physical state
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, FALSE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
#endif
      break;

    // HDCP 1.4 protocol event
    // Enable HDCP 1.4
    case XV_HDMIRXSS_HDCP_1_PROT_EVT :
      if(XV_HdmiRxSs_HdcpSetProtocol(InstancePtr, XV_HDMIRXSS_HDCP_14) != XST_SUCCESS)
        XV_HdmiRxSs_HdcpSetProtocol(InstancePtr, XV_HDMIRXSS_HDCP_22);
      break;

    // HDCP 2.2 protocol event
    // Enable HDCP 2.2
    case XV_HDMIRXSS_HDCP_2_PROT_EVT :
      if(XV_HdmiRxSs_HdcpSetProtocol(InstancePtr, XV_HDMIRXSS_HDCP_22) != XST_SUCCESS)
        XV_HdmiRxSs_HdcpSetProtocol(InstancePtr, XV_HDMIRXSS_HDCP_14);
      break;

    default :
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function schedules the available HDCP cores. Only the active
* HDCP protocol poll function is executed. HDCP 1.4 and 2.2 poll
* functions should not execute in parallel.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpPoll(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Only poll when the HDCP is ready */
  if (InstancePtr->HdcpIsReady) {

    /* Process any pending events from the RX event queue */
    XV_HdmiRxSs_HdcpProcessEvents(InstancePtr);

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    if (InstancePtr->Hdcp22Ptr) {
      if (XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
        XHdcp22Rx_Poll(InstancePtr->Hdcp22Ptr);
      }
    }
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    if (InstancePtr->Hdcp14Ptr) {
      if (XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr)) {
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr);
      }
    }
#endif
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the active HDCP protocol and enables it.
* The protocol can be set to either HDCP 1.4, 2.2, or None.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Protocol is the requested content protection scheme of type
*        XV_HdmiRxSs_HdcpProtocol.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetProtocol(XV_HdmiRxSs *InstancePtr, XV_HdmiRxSs_HdcpProtocol Protocol)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((Protocol == XV_HDMIRXSS_HDCP_NONE) ||
                    (Protocol == XV_HDMIRXSS_HDCP_14)   ||
                    (Protocol == XV_HDMIRXSS_HDCP_22));

  int Status;

  /* Set requested protocol */
  InstancePtr->HdcpProtocol = Protocol;

  /* Reset both protocols */
  Status = XV_HdmiRxSs_HdcpReset(InstancePtr);
  if (Status != XST_SUCCESS) {
    InstancePtr->HdcpProtocol = XV_HDMIRXSS_HDCP_NONE;
    return XST_FAILURE;
  }

  /* Enable the requested protocol */
  Status = XV_HdmiRxSs_HdcpEnable(InstancePtr);
  if (Status != XST_SUCCESS) {
    InstancePtr->HdcpProtocol = XV_HDMIRXSS_HDCP_NONE;
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function gets the active HDCP content protection scheme.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  @RequestedScheme is the requested content protection scheme.
*
* @note   None.
*
******************************************************************************/
XV_HdmiRxSs_HdcpProtocol XV_HdmiRxSs_HdcpGetProtocol(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return InstancePtr->HdcpProtocol;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function enables the requested HDCP protocol. This function
* ensures that the HDCP protocols are mutually exclusive such that
* either HDCP 1.4 or HDCP 2.2 is enabled and active at any given time.
* When the protocol is set to None, both HDCP protocols are disabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpEnable(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status1 = XST_SUCCESS;
  int Status2 = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol) {

    /* Disable HDCP 1.4 and HDCP 2.2 */
    case XV_HDMIRXSS_HDCP_NONE :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
      }
#endif
      break;

    /* Enable HDCP 1.4 and disable HDCP 2.2 */
    case XV_HDMIRXSS_HDCP_14 :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Enable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      else {
        Status1 = XST_FAILURE;
      }

      /* Set DDC peripheral to HDCP 1.4 mode */
      XV_HdmiRx_DdcHdcp14Mode(InstancePtr->HdmiRxPtr);
#else
      Status1 = XST_FAILURE;
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
      }
#endif
      break;

    /* Enable HDCP 2.2 and disable HDCP 1.4 */
    case XV_HDMIRXSS_HDCP_22 :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Enable(InstancePtr->Hdcp22Ptr);
      }
      else {
        Status2 = XST_FAILURE;
      }

      /* Set DDC peripheral to HDCP 2.2 mode */
      XV_HdmiRx_DdcHdcp22Mode(InstancePtr->HdmiRxPtr);
#else
      Status2 = XST_FAILURE;
#endif
      break;

    default :
      return XST_FAILURE;
  }

  return (Status1 == XST_SUCCESS && Status2 == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function disables both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpDisable(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  // HDCP 1.4
#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (InstancePtr->Hdcp14Ptr) {
    Status = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
    Status = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }
#endif

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function resets both HDCP 1.4 and 2.2 protocols. This function also
* disables both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpReset(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4
  // Resetting HDCP 1.4 causes the state machine to be enabled, therefore
  // disable must be called immediately after reset is called.
  if (InstancePtr->Hdcp14Ptr) {
    Status = XHdcp1x_Reset(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
    if (Status != XST_SUCCESS)
      return XST_FAILURE;

    Status = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
    Status = XHdcp22Rx_Reset(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;

    Status = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }
#endif

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol is enabled
*  - FALSE if active protocol is disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsEnabled(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_14 :
      return XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_22 :
      return XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is authenticated.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol is authenticated
*  - FALSE if active protocol is not authenticated
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsAuthenticated(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_14 :
      return XHdcp1x_IsAuthenticated(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_22 :
      return XHdcp22Rx_IsAuthenticated(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol has encryption enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol has encryption enabled
*  - FALSE if active protocol has encryption disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsEncrypted(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_14 :
      return XHdcp1x_IsEncrypted(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_22 :
      return XHdcp22Rx_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is busy authenticating.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol is busy authenticating
*  - FALSE if active protocol is not busy authenticating
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsInProgress(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_14 :
      return XHdcp1x_IsInProgress(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS_HDCP_22 :
      return XHdcp22Rx_IsInProgress(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is in computations state.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol is in computations state
*  - FALSE if active protocol is not in computations state
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsInComputations(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
	case XV_HDMIRXSS_HDCP_NONE :
	  return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
	case XV_HDMIRXSS_HDCP_14 :
	  return XHdcp1x_IsInComputations(InstancePtr->Hdcp14Ptr);
#endif

	case XV_HDMIRXSS_HDCP_22 :
	  return FALSE;

	default :
	  return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is in wait-for-ready state.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*  - TRUE if active protocol is in computations state
*  - FALSE if active protocol is not in computations state
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsInWaitforready(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
	case XV_HDMIRXSS_HDCP_NONE :
	  return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
	case XV_HDMIRXSS_HDCP_14 :
	  return XHdcp1x_IsInWaitforready(InstancePtr->Hdcp14Ptr);
#endif

	case XV_HDMIRXSS_HDCP_22 :
	  return FALSE;

	default :
	  return FALSE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets pointers to the HDCP 1.4 and HDCP 2.2 keys.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_HdcpSetKey(XV_HdmiRxSs *InstancePtr, XV_HdmiRxSs_HdcpKeyType KeyType, u8 *KeyPtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((KeyType == XV_HDMIRXSS_KEY_HDCP22_LC128)   ||
                   (KeyType == XV_HDMIRXSS_KEY_HDCP22_PRIVATE)   ||
                    (KeyType == XV_HDMIRXSS_KEY_HDCP14));

  switch (KeyType) {

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2 LC128
    case XV_HDMIRXSS_KEY_HDCP22_LC128 :
      InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
      break;

    // HDCP 2.2 Private key
    case XV_HDMIRXSS_KEY_HDCP22_PRIVATE :
      InstancePtr->Hdcp22PrivateKeyPtr = KeyPtr;
      break;
#endif

    // HDCP 1.4
    case XV_HDMIRXSS_KEY_HDCP14 :
      InstancePtr->Hdcp14KeyPtr = KeyPtr;
      break;

    default :
      break;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function reports the HDCP information.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_HdcpInfo(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_NONE :
      xil_printf("\n\rHDCP RX is disabled\n\r");
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        if (XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr)) {
          xil_printf("\n\rHDCP 1.4 RX Info\n\r");

          xil_printf("Encryption : ");
          if (XHdcp1x_IsEncrypted(InstancePtr->Hdcp14Ptr)) {
            xil_printf("Enabled.\n\r");
          } else {
            xil_printf("Disabled.\n\r");
          }

          // Route debug output to xil_printf
          XHdcp1x_SetDebugPrintf(xil_printf);

          // Display info
          XHdcp1x_Info(InstancePtr->Hdcp14Ptr);
        }
        else {
          xil_printf("\n\rHDCP 1.4 RX is disabled\n\r");
        }
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        if (XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
          XHdcp22Rx_LogDisplay(InstancePtr->Hdcp22Ptr);

          xil_printf("HDCP 2.2 RX Info\n\r");
          XHdcp22Rx_Info(InstancePtr->Hdcp22Ptr);
        }
        else {
          xil_printf("\n\rHDCP 2.2 RX is disabled\n\r");
        }
      }
      break;
#endif

    default:
      xil_printf("\n\rHDCP info unknown?\n\r");
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP logging level.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Verbose is set to TRUE to enable detailed logging.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_HdcpSetInfoDetail(XV_HdmiRxSs *InstancePtr, u8 Verbose)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);

  if (Verbose) {
#ifdef XPAR_XHDCP_NUM_INSTANCES
   // HDCP 1.4
   if (InstancePtr->Hdcp14Ptr) {
     XHdcp1x_SetDebugLogMsg(xil_printf);
   }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
   // HDCP 2.2
   if (InstancePtr->Hdcp22Ptr) {
     XHdcp22Rx_LogReset(InstancePtr->Hdcp22Ptr, TRUE);
   }
#endif
  }

  else {
#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    if (InstancePtr->Hdcp14Ptr) {
      XHdcp1x_SetDebugLogMsg(NULL);
    }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    if (InstancePtr->Hdcp22Ptr) {
      XHdcp22Rx_LogReset(InstancePtr->Hdcp22Ptr, FALSE);
    }
#endif
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function copies the HDCP repeater topology for the active protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param TopologyPtr is a pointer to the topology structure.
*
* @return .
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetTopology(XV_HdmiRxSs *InstancePtr, void *TopologyPtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(TopologyPtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopology(InstancePtr->Hdcp14Ptr, ((XHdcp1x_RepeaterExchange*)(TopologyPtr)));
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopology(InstancePtr->Hdcp22Ptr, (XHdcp22_Rx_Topology*)(TopologyPtr));
      } else {
        Status = XST_FAILURE;
      }
    break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function copies the HDCP repeater topology list for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param ListPtr is a pointer to the Receiver ID list.
* @param ListSize is the number of Receiver IDs in the list.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetTopologyReceiverIdList(XV_HdmiRxSs *InstancePtr, u8 *ListPtr, u32 ListSize)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(ListPtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyKSVList(InstancePtr->Hdcp14Ptr, ListPtr,	ListSize);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyReceiverIdList(InstancePtr->Hdcp22Ptr, ListPtr, ListSize);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default :
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets various fields inside the HDCP repeater topology.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Field indicates what field of the topology structure to update.
* @param Value is the value assigned to the field of the topology structure.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetTopologyField(XV_HdmiRxSs *InstancePtr,
      XV_HdmiRxSs_HdcpTopologyField Field, u32 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Field < XV_HDMIRXSS_HDCP_TOPOLOGY_INVALID);

  switch (Field)
  {
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_DEPTH:
      return XV_HdmiRxSs_HdcpSetTopologyDepth(InstancePtr, Value);
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_DEVICECNT:
      return XV_HdmiRxSs_HdcpSetTopologyDeviceCnt(InstancePtr, Value);
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_MAXDEVSEXCEEDED:
      return XV_HdmiRxSs_HdcpSetTopologyMaxDevsExceeded(InstancePtr, Value);
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_MAXCASCADEEXCEEDED:
      return XV_HdmiRxSs_HdcpSetTopologyMaxCascadeExceeded(InstancePtr, Value);
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_HDCP20REPEATERDOWNSTREAM:
      return XV_HdmiRxSs_HdcpSetTopologyHdcp20RepeaterDownstream(InstancePtr, Value);
	  case XV_HDMIRXSS_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM:
      return XV_HdmiRxSs_HdcpSetTopologyHdcp1DeviceDownstream(InstancePtr, Value);
    default:
      return XST_FAILURE;
  }
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology depth for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Depth is the Repeater cascade depth.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyDepth(XV_HdmiRxSs *InstancePtr, u32 Depth)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_DEPTH, Depth);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_DEPTH, Depth);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology device count for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param DeviceCnt is the Total number of connected downstream devices.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyDeviceCnt(XV_HdmiRxSs *InstancePtr, u32 DeviceCnt)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_DEVICECNT, DeviceCnt);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_DEVICECNT, DeviceCnt);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology maximum devices exceeded
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyMaxDevsExceeded(XV_HdmiRxSs *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_MAXDEVSEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_MAXDEVSEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology maximum cascade exceeded
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyMaxCascadeExceeded(XV_HdmiRxSs *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_MAXCASCADEEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_MAXCASCADEEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology HDCP 2.0 repeater
* downstream flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyHdcp20RepeaterDownstream(XV_HdmiRxSs *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_HDCP20REPEATERDOWNSTREAM, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology HDCP 1.x repeater downstream
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs_HdcpSetTopologyHdcp1DeviceDownstream(XV_HdmiRxSs *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_HDCP1DEVICEDOWNSTREAM, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology update flag, indicating that
* the topology is ready for upstream propagation.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetTopologyUpdate(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    // None
    case XV_HDMIRXSS_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyUpdate(InstancePtr->Hdcp14Ptr);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyUpdate(InstancePtr->Hdcp22Ptr);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

    default:
      Status = XST_FAILURE;
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function gets the repeater management content stream type.
* When the protocol is HDCP 1.4 the stream type is always Type 0.
* For HDCP 2.2 the stream type is extracted from the HDCP 2.2
* stream manage message.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XV_HdmiRxSs_HdcpContentStreamType XV_HdmiRxSs_HdcpGetContentStreamType(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int StreamType;

  switch (InstancePtr->HdcpProtocol)
  {
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      StreamType = XV_HDMIRXSS_HDCP_STREAMTYPE_0;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        StreamType = XHdcp22Rx_GetContentStreamType(InstancePtr->Hdcp22Ptr);
      } else {
        StreamType = XV_HDMIRXSS_HDCP_STREAMTYPE_0;
      }
      break;
#endif

    default:
      StreamType = XV_HDMIRXSS_HDCP_STREAMTYPE_0;
  }

  return StreamType;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function checks if the HDMI receiver is an HDCP repeater
* upstream interface for the active protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
*
* @return
*   - TRUE if repeater upstream interface.
*   - FALSE if not repeater upstream interface.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpIsRepeater(XV_HdmiRxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = (int)FALSE;

  switch (InstancePtr->HdcpProtocol)
  {
#ifdef XPAR_XHDCP_NUM_INSTANCES
    // HDCP 1.4
    case XV_HDMIRXSS_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        Status = XHdcp1x_IsRepeater(InstancePtr->Hdcp14Ptr);
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    // HDCP 2.2
    case XV_HDMIRXSS_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        Status = XHdcp22Rx_IsRepeater(InstancePtr->Hdcp22Ptr);
      }
      break;
#endif

    default:
      Status = (int)FALSE;
  }

  return Status;
}
#endif

#ifdef USE_HDCP
/*****************************************************************************/
/**
*
* This function enables the Repeater functionality for the HDCP protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs instance.
* @param Set is TRUE to enable and FALSE to disable repeater.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs_HdcpSetRepeater(XV_HdmiRxSs *InstancePtr, u8 Set)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4
  if (InstancePtr->Hdcp14Ptr) {
    XHdcp1x_SetRepeater(InstancePtr->Hdcp14Ptr, Set);
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
    XHdcp22Rx_SetRepeater(InstancePtr->Hdcp22Ptr, Set);
  }
#endif

  return XST_SUCCESS;
}
#endif
