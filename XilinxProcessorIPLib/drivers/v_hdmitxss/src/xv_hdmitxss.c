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
* 1.10  MG     17/12/16 Fixed issue in function SetAudioChannels
*                       Updated function XV_HdmiTxSs_SendAuxInfoframe
* 1.2   yh     12/01/16 Check vtc existance before configuring it
* 1.3   yh     15/01/16 Add 3D Support
* 1.4   yh     20/01/16 Added remapper support
* 1.5   yh     01/02/16 Added set_ppc api
* 1.6   yh     01/02/16 Removed xil_print "Cable (dis)connected"
* 1.7   yh     15/02/16 Added default value to XV_HdmiTxSs_ConfigRemapper
* 1.8   MG     03/02/16 Added HDCP support
* 1.9   MG     09/03/16 Added XV_HdmiTxSs_SetHdmiMode and XV_HdmiTxSs_SetDviMode
*                       Removed reduced blanking support
* 1.10  MH     03/15/16 Moved HDCP 2.2 reset from stream up/down callback to
*                       connect callback
* 1.11  YH     18/03/16 Add XV_HdmiTxSs_SendGenericAuxInfoframe function
* 1.12  MH     23/04/16 1. HDCP 1.x driver now uses AXI timer 4.1, so updated
*                       to use AXI Timer config structure to determine timer
*                       clock frequency
*                       2. HDCP 1.x driver has fixed the problem where the
*                       reset for the receiver causes the entire DDC peripheral
*                       to get reset. Based on this change the driver has been
*                       updated to use XV_HdmiTxSs_HdcpReset and
*                       XV_HdmiRxSs_HdcpReset functions directly.
*                       3. Updated XV_HdmiTxSs_HdcpEnable and
*                       XV_HdmiRxSs_HdcpEnable functions to ensure that
*                       HDCP 1.4 and 2.2 are mutually exclusive.
*                       This fixes the problem where HDCP 1.4 and 2.2
*                       state machines are running simultaneously.
*
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
  XGpio RemapperReset;
  XTmrCtr HdcpTimer;
  XHdcp1x Hdcp14;
  XHdcp22_Tx  Hdcp22;
  XV_axi4s_remap Remapper;
  XV_HdmiTx HdmiTx;
  XVtc Vtc;
}XV_HdmiTxSs_SubCores;

/**************************** Local Global ***********************************/
XV_HdmiTxSs_SubCores XV_HdmiTxSs_SubCoreRepo[XPAR_XV_HDMITXSS_NUM_INSTANCES];
                /**< Define Driver instance of all sub-core
                                    included in the design */

XV_HdmiTx_VSIF VSIF;

/************************** Function Prototypes ******************************/
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr,
    u16 DevId);
static XV_HdmiTxSs_SubCores *XV_HdmiTxSs_GetSubSysStruct(void *SubCorePtr);
static void XV_HdmiTxSs_WaitUs(XV_HdmiTxSs *InstancePtr, u32 MicroSeconds);
static int XV_HdmiTxSs_RegisterSubsysCallbacks(XV_HdmiTxSs *InstancePtr);
static int XV_HdmiTxSs_VtcSetup(XVtc *XVtcPtr, XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendAviInfoframe(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendGeneralControlPacket(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendVSInfoframe(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiTxSs_VsCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamUpCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamDownCallback(void *CallbackRef);
static u32 XV_HdmiTxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
    u32 ClockFrequency);

// HDCP specific
static XV_HdmiTxSs_HdcpEvent XV_HdmiTxSs_HdcpGetEvent(XV_HdmiTxSs *InstancePtr);
static int XV_HdmiTxSs_HdcpProcessEvents(XV_HdmiTxSs *InstancePtr);
static int XV_HdmiTxSs_HdcpReset(XV_HdmiTxSs *InstancePtr);
static u8 XV_HdmiTxSs_IsSinkHdcp14Capable(XV_HdmiTx *HdmiInstPtr);
static u8 XV_HdmiTxSs_IsSinkHdcp22Capable(XV_HdmiTx *HdmiInstPtr);

static void XV_HdmiTxSs_ResetRemapper(XV_HdmiTxSs *InstancePtr);
static void XV_HdmiTxSs_ConfigRemapper(XV_HdmiTxSs *InstancePtr);
static void XV_HdmiTxSs_SetDefaultPpc(XV_HdmiTxSs *InstancePtr, u8 Id);
static void XV_HdmiTxSs_SetPpc(XV_HdmiTxSs *InstancePtr, u8 Id, u8 Ppc);

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function sets the core into HDMI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_SetHdmiMode(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_SetHdmiMode(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
 * This function sets the core into DVI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_SetDviMode(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_SetDviMode(InstancePtr->HdmiTxPtr);
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

  if (InstancePtr->Hdcp14Ptr) {
    xil_printf("    : HDCP 1.4 TX \r\n");
  }

  if (InstancePtr->Hdcp22Ptr) {
    xil_printf("    : HDCP 2.2 TX \r\n");
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
 * @param   InstancePtr is a pointer to the XDptx instance.
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
 * @param   InstancePtr is a pointer to the XDptx instance.
 * @param   MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return  None.
 *
 * @note    None.
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
    XHdcp1x_CipherIntrHandler(InstancePtr->Hdcp14Ptr);
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
 * This function calls the interrupt handler for HDCP 2.2 Timer
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_Hdcp22TimerIntrHandler(XV_HdmiTxSs *InstancePtr)
{
	XTmrCtr *XTmrCtrPtr;

	XTmrCtrPtr = XHdcp22Tx_GetTimer(InstancePtr->Hdcp22Ptr);

    XTmrCtr_InterruptHandler(XTmrCtrPtr);
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

  /** Register HDCP 1.4 TX ISR */
  if (HdmiTxSsPtr->Hdcp14Ptr) {
  }

  /** Register HDCP 2.2 TX ISR */
  if (HdmiTxSsPtr->Hdcp22Ptr) {
    // Authenticated callback

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
  // HDCP 1.4
  HdmiTxSsPtr->Hdcp14Ptr       = ((HdmiTxSsPtr->Config.Hdcp14.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiTxSsPtr->HdcpTimerPtr  = ((HdmiTxSsPtr->Config.HdcpTimer.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
  // HDCP 2.2
  HdmiTxSsPtr->Hdcp22Ptr       = ((HdmiTxSsPtr->Config.Hdcp22.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Hdcp22) : NULL);

  HdmiTxSsPtr->RemapperResetPtr  = ((HdmiTxSsPtr->Config.RemapperReset.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].RemapperReset) : NULL);
  HdmiTxSsPtr->RemapperPtr  = ((HdmiTxSsPtr->Config.Remapper.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Remapper) : NULL);
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

        // HDCP 1.4
        if (&XV_HdmiTxSs_SubCoreRepo[i].Hdcp14 == SubCorePtr){
            return &XV_HdmiTxSs_SubCoreRepo[i];
        }

        if (&XV_HdmiTxSs_SubCoreRepo[i].HdcpTimer == SubCorePtr){
            return &XV_HdmiTxSs_SubCoreRepo[i];
        }

        // HDCP 2.2
        if (&XV_HdmiTxSs_SubCoreRepo[i].Hdcp22 == SubCorePtr){
            return &XV_HdmiTxSs_SubCoreRepo[i];
        }

        if (&XV_HdmiTxSs_SubCoreRepo[i].RemapperReset == SubCorePtr){
            return &XV_HdmiTxSs_SubCoreRepo[i];
        }

        if (&XV_HdmiTxSs_SubCoreRepo[i].Remapper == SubCorePtr){
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

  // HDCP 1.4
  if (HdmiTxSsPtr->Hdcp14Ptr) {
    if (XV_HdmiTxSs_SubcoreInitHdcp14(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->HdmiTxPtr) {
    if (XV_HdmiTxSs_SubcoreInitHdmiTx(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  // HDCP 2.2
  if (HdmiTxSsPtr->Hdcp22Ptr) {
    if (XV_HdmiTxSs_SubcoreInitHdcp22(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->VtcPtr) {
    if (XV_HdmiTxSs_SubcoreInitVtc(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->RemapperResetPtr) {
    if (XV_HdmiTxSs_SubcoreInitRemapperReset(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->RemapperPtr) {
    if (XV_HdmiTxSs_SubcoreInitRemapper(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  /* Register Callbacks */
  XV_HdmiTxSs_RegisterSubsysCallbacks(HdmiTxSsPtr);

  /* Set default HDCP protocol */
  HdmiTxSsPtr->HdcpProtocol = XV_HDMITXSS_HDCP_NONE;

  /* HDCP ready flag */

  /* Default value */
  HdmiTxSsPtr->HdcpIsReady = (FALSE);

  /* HDCP is ready when both HDCP cores are instantiated and both keys are loaded */
  if (HdmiTxSsPtr->Hdcp14Ptr && HdmiTxSsPtr->Hdcp22Ptr &&
      HdmiTxSsPtr->Hdcp22Lc128Ptr && HdmiTxSsPtr->Hdcp14KeyPtr) {
        HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }

  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key is loaded */
  else if (HdmiTxSsPtr->Hdcp14Ptr && HdmiTxSsPtr->Hdcp14KeyPtr) {
        HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }

  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the key is loaded */
  else if (HdmiTxSsPtr->Hdcp22Ptr && HdmiTxSsPtr->Hdcp22Lc128Ptr) {
        HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }

  /* Set the flag to indicate the subsystem is ready */
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

  if (InstancePtr->VtcPtr) {
    /* Disable VTC */
    XVtc_DisableGenerator(InstancePtr->VtcPtr);
  }
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

  /* Assert TX reset */
  XV_HdmiTx_Reset(InstancePtr->HdmiTxPtr, TRUE);

  /* Release TX reset */
  XV_HdmiTx_Reset(InstancePtr->HdmiTxPtr, FALSE);
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
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_CONNECT_EVT);
  }

  // TX cable is disconnected
  else {
    HdmiTxSsPtr->IsStreamConnected = (FALSE);
    xil_printf("TX cable is disconnected\n\r");
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_DISCONNECT_EVT);
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

  // Vendor-Specific InfoFrame
  XV_HdmiTxSs_SendVSInfoframe(HdmiTxSsPtr->HdmiTxPtr);

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

  if (!XVidC_IsStream3D(&HdmiTx->Stream.Video) &&
      (HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_24_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_25_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_30_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_4096x2160_24_P)) {
    HdmiTx->Aux.Data.Byte[4] = 0;
  }
  else {
      HdmiTx->Aux.Data.Byte[4] = HdmiTx->Stream.Vic;
  }

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
* This function sends the Vendor Specific Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_SendVSInfoframe(XV_HdmiTx *HdmiTx)
{
    VSIF.Version = 0x1;
    VSIF.IEEE_ID = 0xC03;

    if (XVidC_IsStream3D(&HdmiTx->Stream.Video)) {
        VSIF.Format = XV_HDMITX_VSIF_VF_3D;
        VSIF.Info_3D.Stream = HdmiTx->Stream.Video.Info_3D;
        VSIF.Info_3D.MetaData.IsPresent = FALSE;
    }
    else if (HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_24_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_25_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_30_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_4096x2160_24_P) {
        VSIF.Format = XV_HDMITX_VSIF_VF_EXTRES;

        /* Set HDMI VIC */
        switch(HdmiTx->Stream.Video.VmId) {
            case XVIDC_VM_4096x2160_24_P :
                VSIF.HDMI_VIC = 4;
                break;
            case XVIDC_VM_3840x2160_24_P :
                VSIF.HDMI_VIC = 3;
                break;
            case XVIDC_VM_3840x2160_25_P :
                VSIF.HDMI_VIC = 2;
                break;
            case XVIDC_VM_3840x2160_30_P :
                VSIF.HDMI_VIC = 1;
                break;
            default :
                break;
        }
    }
    else {
        VSIF.Format = XV_HDMITX_VSIF_VF_NOINFO;
    }

    XV_HdmiTx_VSIF_GeneratePacket(&VSIF,(XV_HdmiTx_Aux *)&HdmiTx->Aux);

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

  if (HdmiTxSsPtr->VtcPtr) {
    /* Setup VTC */
    XV_HdmiTxSs_VtcSetup(HdmiTxSsPtr->VtcPtr, HdmiTxSsPtr->HdmiTxPtr);
  }

  if (HdmiTxSsPtr->AudioEnabled) {
      /* HDMI TX unmute audio */
      HdmiTxSsPtr->AudioMute = (FALSE);
      XV_HdmiTx_AudioUnmute(HdmiTxSsPtr->HdmiTxPtr);
  }

  // HDCP 1.4
  if (HdmiTxSsPtr->Hdcp14Ptr) {
    /* Set the TX HDCP state to up */
    /* However because this callback might be executed from an interrupt
    and the HDCP driver is not thread safe, therefore this event queue is used.
    In the next HDCP poll the HDCP state is set */
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_STREAMUP_EVT);
  }

  if (HdmiTxSsPtr->RemapperResetPtr) {
      /* Toggle AXI_GPIO to Reset Remapper */
      XV_HdmiTxSs_ResetRemapper(HdmiTxSsPtr);
  }

  if (HdmiTxSsPtr->RemapperPtr) {
      /* Configure Remapper according to HW setting and video format */
      XV_HdmiTxSs_ConfigRemapper(HdmiTxSsPtr);
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

  // HDCP 1.4
  if (HdmiTxSsPtr->Hdcp14Ptr) {
    /* Set the TX HDCP state to down */
    /* However because this callback might be executed from an interrupt
    and the HDCP driver is not thread safe, therefore this event queue is used.
    In the next HDCP poll the HDCP state is set */
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_STREAMDOWN_EVT);
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
* @param    InstancePtr is a pointer to the HDMI TX Subsystem instance.
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

        // HDCP authenticated
        case (XV_HDMITXSS_HANDLER_HDCP_AUTHENTICATE):
          InstancePtr->HdcpAuthenticateCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
          InstancePtr->HdcpAuthenticateRef = CallbackRef;
          Status = (XST_INVALID_PARAM);

          // Register HDCP 1.4 callback
          if (InstancePtr->Hdcp14Ptr) {
            Status = XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                XHDCP1X_HANDLER_AUTHENTICATED,
                (XHdcp1x_Callback)CallbackFunc,
                CallbackRef);
          }

          // Register HDCP 2.2 callback
          if (InstancePtr->Hdcp22Ptr) {
            Status = XHdcp22Tx_SetCallback(InstancePtr->Hdcp22Ptr,
                XHDCP22_TX_HANDLER_AUTHENTICATED,
                (XHdcp22_Tx_Callback)CallbackFunc,
                CallbackRef);
          }
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
      if (tx_aux->Header.Byte[0] == 0x84) {
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
* This function sends generic info frames.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SendGenericAuxInfoframe(XV_HdmiTxSs *InstancePtr, void *Aux)
{
  u8 Index;
  XV_HdmiTx_Aux *tx_aux = Aux;

  // Header
  InstancePtr->HdmiTxPtr->Aux.Header.Data = tx_aux->Header.Data;

  // Data
  for (Index = 0; Index < 8; Index++) {
    InstancePtr->HdmiTxPtr->Aux.Data.Data[Index] =
	tx_aux->Data.Data[Index];
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
    XV_HdmiTx_SetAudioChannels(InstancePtr->HdmiTxPtr, AudioChannels);
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
        XVidC_ColorDepth Bpc, XVidC_3DInfo *Info3D)
{
  u32 TmdsClock = 0;

  if (VideoMode < XVIDC_VM_NUM_SUPPORTED) {
          TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr, VideoMode,
            ColorFormat, Bpc, InstancePtr->Config.Ppc, Info3D);

  }

  // Default 1080p colorbar
  else {
    TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr,
        XVIDC_VM_1920x1080_60_P, XVIDC_CSF_RGB, XVIDC_BPC_8, InstancePtr->Config.Ppc,
        Info3D);
  }

  if(TmdsClock == 0) {
    xil_printf("\nWarning: Sink does not support HDMI 2.0\r\n");
    xil_printf("         Connect to HDMI 2.0 Sink or \r\n");
    xil_printf("         Change to HDMI 1.4 video format\r\n\n");
}

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
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
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

  // HDCP 1.4
  if (InstancePtr->Hdcp14Ptr){
     Data = XHdcp1x_GetVersion(InstancePtr->Hdcp14Ptr);
     xil_printf("  HDCP 1.4 TX version : %02d.%02d (%04x)\n\r",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
   Data = XHdcp22Tx_GetVersion(InstancePtr->Hdcp22Ptr);
   xil_printf("  HDCP 2.2 TX version : %02d.%02d (%04x)\n\r",
    ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

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
									TimerPtr->Config.SysClockFreqHz);


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


void XV_HdmiTxSs_ResetRemapper(XV_HdmiTxSs *InstancePtr) {

    XGpio *RemapperResetPtr = InstancePtr->RemapperResetPtr;

    XGpio_SetDataDirection(RemapperResetPtr, 1, 0);
    XGpio_DiscreteWrite(RemapperResetPtr, 1, 0);
    XV_HdmiTxSs_WaitUs(InstancePtr, 1000);
    XGpio_DiscreteWrite(RemapperResetPtr, 1, 1);
    XV_HdmiTxSs_WaitUs(InstancePtr, 1000);
}

void XV_HdmiTxSs_ConfigRemapper(XV_HdmiTxSs *InstancePtr) {
    XV_axi4s_remap *RemapperPtr = InstancePtr->RemapperPtr;

    XVidC_ColorFormat ColorFormat;
    XVidC_VideoMode VideoMode;

    XVidC_VideoStream *HdmiTxSsVidStreamPtr;
    HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(InstancePtr);

    ColorFormat = HdmiTxSsVidStreamPtr->ColorFormatId;
    VideoMode = HdmiTxSsVidStreamPtr->VmId;

    if (ColorFormat == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
        XV_axi4s_remap_Set_width(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.HActive);
        XV_axi4s_remap_Set_height(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.VActive);
        XV_axi4s_remap_Set_ColorFormat(RemapperPtr, XVIDC_CSF_YCRCB_420);
        XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
        XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
        XV_axi4s_remap_Set_inHDMI420(RemapperPtr, 0);
        XV_axi4s_remap_Set_outHDMI420(RemapperPtr, 1);
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
            XV_axi4s_remap_Set_width(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.HActive/2);
            XV_axi4s_remap_Set_height(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.VActive);
            XV_axi4s_remap_Set_ColorFormat(RemapperPtr, 0);
            XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_inHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_outHDMI420(RemapperPtr, 0);
            XV_axi4s_remap_Set_inPixDrop(RemapperPtr, 0);
            XV_axi4s_remap_Set_outPixRepeat(RemapperPtr, 1);
            XV_axi4s_remap_EnableAutoRestart(RemapperPtr);
            XV_axi4s_remap_Start(RemapperPtr);
        }
        else {
            XV_axi4s_remap_Set_width(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.HActive);
            XV_axi4s_remap_Set_height(RemapperPtr, HdmiTxSsVidStreamPtr->Timing.VActive);
            XV_axi4s_remap_Set_ColorFormat(RemapperPtr, ColorFormat);
            XV_axi4s_remap_Set_inPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
            XV_axi4s_remap_Set_outPixClk(RemapperPtr, HdmiTxSsVidStreamPtr->PixPerClk);
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
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    Id is the XV_HdmiTxSs ID to operate on.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs_SetDefaultPpc(XV_HdmiTxSs *InstancePtr, u8 Id) {
    extern XV_HdmiTxSs_Config XV_HdmiTxSs_ConfigTable[XPAR_XV_HDMITXSS_NUM_INSTANCES];
    InstancePtr->Config.Ppc = XV_HdmiTxSs_ConfigTable[Id].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    Id is the XV_HdmiTxSs ID to operate on.
* @param    Ppc is the PPC to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs_SetPpc(XV_HdmiTxSs *InstancePtr, u8 Id, u8 Ppc) {
    InstancePtr->Config.Ppc = Ppc;
}

/*****************************************************************************/
/**
*
* This function pushes an event into the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
* @param Event is the event to be pushed in the queue.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpPushEvent(XV_HdmiTxSs *InstancePtr, XV_HdmiTxSs_HdcpEvent Event)
{
  u16 EventQueueSize;

  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Event < XV_HDMITXSS_HDCP_INVALID_EVT);

  /* Write event into the queue */
  InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Head] = Event;

  /* Update head pointer */
  EventQueueSize = sizeof(InstancePtr->HdcpEventQueue)/sizeof(XV_HdmiTxSs_HdcpEvent);
  if (InstancePtr->HdcpEventQueue.Head == (EventQueueSize - 1)) {
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
    if (InstancePtr->HdcpEventQueue.Tail == (EventQueueSize - 1)) {
      InstancePtr->HdcpEventQueue.Tail = 0;
    }
    else {
      InstancePtr->HdcpEventQueue.Tail++;
    }
  }

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets an event from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return When the queue is filled, the next event is returned.
*         When the queue is empty, XV_HDMITXSS_HDCP_NO_EVT is returned.
*
* @note   None.
*
******************************************************************************/
static XV_HdmiTxSs_HdcpEvent XV_HdmiTxSs_HdcpGetEvent(XV_HdmiTxSs *InstancePtr)
{
  u16 EventQueueSize;
  XV_HdmiTxSs_HdcpEvent Event;

  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Check if there are any events in the queue */
  if (InstancePtr->HdcpEventQueue.Tail == InstancePtr->HdcpEventQueue.Head) {
    return XV_HDMITXSS_HDCP_NO_EVT;
  }

  Event = InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Tail];

  /* Update tail pointer */
  EventQueueSize = sizeof(InstancePtr->HdcpEventQueue.Queue)/sizeof(XV_HdmiTxSs_HdcpEvent);
  if (InstancePtr->HdcpEventQueue.Tail == (EventQueueSize - 1)) {
    InstancePtr->HdcpEventQueue.Tail = 0;
  }
  else {
    InstancePtr->HdcpEventQueue.Tail++;
  }

  return Event;
}

/*****************************************************************************/
/**
*
* This function clears all pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpClearEvents(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  InstancePtr->HdcpEventQueue.Head = 0;
  InstancePtr->HdcpEventQueue.Tail = 0;

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function processes pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiTxSs_HdcpProcessEvents(XV_HdmiTxSs *InstancePtr)
{
  XV_HdmiTxSs_HdcpEvent Event;
  int Status = XST_SUCCESS;

  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  Event = XV_HdmiTxSs_HdcpGetEvent(InstancePtr);
  switch (Event) {

    // Stream up
    case XV_HDMITXSS_HDCP_STREAMUP_EVT :
      if (InstancePtr->Hdcp14Ptr) {
        // Set physical state
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, TRUE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      break;

    // Stream down
    // Reset both HDCP protocols, then enable only the active HDCP protocol
    case XV_HDMITXSS_HDCP_STREAMDOWN_EVT :
      if (InstancePtr->Hdcp14Ptr) {
        // Set physical state
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, FALSE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      XV_HdmiTxSs_HdcpReset(InstancePtr);
      XV_HdmiTxSs_HdcpEnable(InstancePtr);
      break;

    // Connect
    // Enable only the active HDCP protocol
    case XV_HDMITXSS_HDCP_CONNECT_EVT :
      XV_HdmiTxSs_HdcpEnable(InstancePtr);
      break;

    // Disconnect
    // Reset both HDCP protocols
    case XV_HDMITXSS_HDCP_DISCONNECT_EVT :
      XV_HdmiTxSs_HdcpReset(InstancePtr);
      break;

    default :
      break;
  }

  return Status;
}

/*****************************************************************************/
/**
*
* This function schedules the available HDCP cores. Only the active
* HDCP protocol poll function is executed. HDCP 1.4 and 2.2 poll
* functions should not execute in parallel.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpPoll(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Only poll when the HDCP is ready */
  if (InstancePtr->HdcpIsReady) {

    /* Process any pending events from the TX event queue */
    XV_HdmiTxSs_HdcpProcessEvents(InstancePtr);

    // HDCP 2.2
    if (InstancePtr->Hdcp22Ptr) {
      if (XHdcp22Tx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
       XHdcp22Tx_Poll(InstancePtr->Hdcp22Ptr);
      }
    }

    // HDCP 1.4
    if (InstancePtr->Hdcp14Ptr) {
      if (XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr)) {
       XHdcp1x_Poll(InstancePtr->Hdcp14Ptr);
      }
    }
  }

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the active HDCP protocol and enables it.
* The protocol can be set to either HDCP 1.4, 2.2, or None.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
* @param Protocol is the requested content protection scheme of type
*        XV_HdmiTxSs_HdcpProtocol.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpSetProtocol(XV_HdmiTxSs *InstancePtr, XV_HdmiTxSs_HdcpProtocol Protocol)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((Protocol == XV_HDMITXSS_HDCP_NONE)   ||
                    (Protocol == XV_HDMITXSS_HDCP_14) ||
                    (Protocol == XV_HDMITXSS_HDCP_22));

  int Status;

  // Set protocol
  InstancePtr->HdcpProtocol = Protocol;

  // Reset both protocols
  Status = XV_HdmiTxSs_HdcpReset(InstancePtr);
  if (Status != XST_SUCCESS)
    return Status;

  // Enable the requested protocol
  Status = XV_HdmiTxSs_HdcpEnable(InstancePtr);
  if (Status != XST_SUCCESS)
    return Status;

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets the active HDCP content protection scheme.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  @RequestedScheme is the requested content protection scheme.
*
* @note   None.
*
******************************************************************************/
XV_HdmiTxSs_HdcpProtocol XV_HdmiTxSs_HdcpGetProtocol(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return InstancePtr->HdcpProtocol;
}

/*****************************************************************************/
/**
*
* This function enables the requested HDCP protocol. This function
* ensures that the HDCP protocols are mutually exclusive such that
* either HDCP 1.4 or HDCP 2.2 is enabled and active at any given time.
* When the protocol is set to None, both HDCP protocols are disabled.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpEnable(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status1 = XST_SUCCESS;
  int Status2 = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol) {

    /* Disable HDCP 1.4 and HDCP 2.2 */
    case XV_HDMITXSS_HDCP_NONE :
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Tx_Disable(InstancePtr->Hdcp22Ptr);
      }
      break;

    /* Enable HDCP 1.4 and disable HDCP 2.2 */
    case XV_HDMITXSS_HDCP_14 :
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Enable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      else {
        Status1 = XST_FAILURE;
      }
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Tx_Disable(InstancePtr->Hdcp22Ptr);
      }
      break;

    /* Enable HDCP 2.2 and disable HDCP 1.4 */
    case XV_HDMITXSS_HDCP_22 :
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
      }
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Tx_Enable(InstancePtr->Hdcp22Ptr);
      }
      else {
        Status2 = XST_FAILURE;
      }
      break;

    default :
      return XST_FAILURE;
  }

  return (Status1 == XST_SUCCESS && Status2 == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function disables both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpDisable(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_FAILURE;

  // HDCP 1.4
  if (InstancePtr->Hdcp14Ptr) {
    Status = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); // This is needed to ensure that the previous command is executed.
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }

  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
    Status = XHdcp22Tx_Disable(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }

  return Status;
}

/*****************************************************************************/
/**
*
* This function resets both HDCP 1.4 and 2.2 protocols. This function
* also disables the both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiTxSs_HdcpReset(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_FAILURE;

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

  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
    Status = XHdcp22Tx_Reset(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;

    Status = XHdcp22Tx_Disable(InstancePtr->Hdcp22Ptr);
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }

  return Status;
}

/*****************************************************************************/
/**
*
* This function sends an authentication request for the active HDCP Protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpAuthRequest(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_FAILURE;

  /* Always disable encryption */
  Status = XV_HdmiTxSs_HdcpDisableEncryption(InstancePtr);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /* Verify if sink is attached */
  if (!XV_HdmiTx_IsStreamConnected(InstancePtr->HdmiTxPtr)) {
    xil_printf("No sink is attached\n\r");
    return XST_FAILURE;
  }

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      break;

    case XV_HDMITXSS_HDCP_14 :
      if (XV_HdmiTxSs_IsSinkHdcp14Capable(InstancePtr->HdmiTxPtr)) {
        xil_printf("Starting HDCP 1.4 authentication\n\r");
        Status = XHdcp1x_Authenticate(InstancePtr->Hdcp14Ptr);
      }
      else {
        xil_printf("Sink is not HDCP 1.4 capable\n\r");
      }
      break;

    case XV_HDMITXSS_HDCP_22 :
      if (XV_HdmiTxSs_IsSinkHdcp22Capable(InstancePtr->HdmiTxPtr)) {
        xil_printf("Starting HDCP 2.2 authentication\n\r");
        Status = XHdcp22Tx_Authenticate(InstancePtr->Hdcp22Ptr);
      }
      else {
        xil_printf("Sink is not HDCP 2.2 capable\n\r");
      }
      break;

    default :
      break;
  }

  return (Status == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function enables encryption for active HDCP protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpEnableEncryption(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      return XST_SUCCESS;

    case XV_HDMITXSS_HDCP_14 :
      return XHdcp1x_EnableEncryption(InstancePtr->Hdcp14Ptr, 0x1);
      break;

    case XV_HDMITXSS_HDCP_22 :
      return XHdcp22Tx_EnableEncryption(InstancePtr->Hdcp22Ptr);
      break;

    default :
      /* Do nothing */
      break;
  }

  return XST_FAILURE;
}

/*****************************************************************************/
/**
*
* This function disables encryption for active HDCP protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpDisableEncryption(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_FAILURE;

  if (InstancePtr->Hdcp14Ptr) {
    Status = XHdcp1x_DisableEncryption(InstancePtr->Hdcp14Ptr, 0x1);

    if (Status != XST_SUCCESS) {
      return XST_FAILURE;
    }
  }

  if (InstancePtr->Hdcp22Ptr) {
    Status = XHdcp22Tx_DisableEncryption(InstancePtr->Hdcp22Ptr);

    if (Status != XST_SUCCESS) {
      return XST_FAILURE;
    }
  }

  return Status;
}

/*****************************************************************************/
/**
*
* This function determines if the connected HDMI sink has HDCP1.4 capabilities.
*
* @param HdmiInstPtr is a pointer to the XHdmi_Tx core instance.
*
* @return
*  - TRUE if sink is HDCP1.4 capable
*  - FALSE if sink does not support HDCP1.4
*
******************************************************************************/
static u8 XV_HdmiTxSs_IsSinkHdcp14Capable(XV_HdmiTx *HdmiInstPtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(HdmiInstPtr != NULL);

  int status;
  u8 buffer[5];
  u8 temp = 0;
  int zero_count = 0;
  int one_count = 0;
  int i,j;

  buffer[0] = 0x0; // XHDCP14_BKSV_REG
  status = XV_HdmiTx_DdcWrite(HdmiInstPtr, 0x3A, 1, (u8*)&buffer, FALSE);
  if (status != XST_SUCCESS)
    return FALSE;

  /* Read the receiver KSV */
  status = XV_HdmiTx_DdcRead(HdmiInstPtr, 0x3A, 5, (u8*)&buffer, TRUE);
  if (status != XST_SUCCESS)
    return FALSE;

  for(i = 0; i < 5; i++) {
    temp = buffer[i];

    /* Count the amount of ones and zeros */
    for(j = 0; j < 8; j++) {
      if(temp & 0x1)
        one_count++;
      else
        zero_count++;

      temp = temp >> 1;
    }
  }

  /* A valid KSV contains 20 ones and 20 zeros */
  if (one_count == 20 && zero_count == 20)
    return TRUE;
  else
    return FALSE;
}

/*****************************************************************************/
/**
*
* This function determines if the connected HDMI sink has HDCP2.2 capabilities.
*
* @param HdmiInstPtr is a pointer to the XHdmi_Tx core instance.
*
* @return
*  - TRUE if sink is HDCP2.2 capable
*  - FALSE if sink does not support HDCP2.2
*
******************************************************************************/
static u8 XV_HdmiTxSs_IsSinkHdcp22Capable(XV_HdmiTx *HdmiInstPtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(HdmiInstPtr != NULL);

  int status;
  u8 data = 0x50; // XHDCP2_VERSION_REG

  /* Write the register offset */
  status = XV_HdmiTx_DdcWrite(HdmiInstPtr, 0x3A, 1, (u8*)&data, FALSE);
  if (status != XST_SUCCESS)
    return FALSE;

  /* Read the HDCP2 version */
  status = XV_HdmiTx_DdcRead(HdmiInstPtr, 0x3A, 1, (u8*)&data, TRUE);
  if (status != XST_SUCCESS)
    return FALSE;

  /* Check the HDCP2.2 version */
  if(data & 0x4)
    return TRUE;
  else
    return FALSE;
}

/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - TRUE if active protocol is enabled
*  - FALSE if active protocol is disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpIsEnabled(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      return FALSE;

    case XV_HDMITXSS_HDCP_14 :
      return XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr);

    case XV_HDMITXSS_HDCP_22 :
      return XHdcp22Tx_IsEnabled(InstancePtr->Hdcp22Ptr);

    default :
      return FALSE;
  }
}

/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is authenticated.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - TRUE if active protocol is authenticated
*  - FALSE if active protocol is not authenticated
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpIsAuthenticated(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      return FALSE;

    case XV_HDMITXSS_HDCP_14 :
      return XHdcp1x_IsAuthenticated(InstancePtr->Hdcp14Ptr);

    case XV_HDMITXSS_HDCP_22 :
      return XHdcp22Tx_IsAuthenticated(InstancePtr->Hdcp22Ptr);

    default :
      return FALSE;
  }
}

/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol has encryption enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return
*  - TRUE if active protocol has encryption enabled
*  - FALSE if active protocol has encryption disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_HdcpIsEncrypted(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      return FALSE;

    case XV_HDMITXSS_HDCP_14 :
      return XHdcp1x_IsEncrypted(InstancePtr->Hdcp14Ptr);

    case XV_HDMITXSS_HDCP_22 :
      return XHdcp22Tx_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr);

    default :
      return FALSE;
  }
}

/*****************************************************************************/
/**
*
* This function sets pointers to the HDCP 1.4 and HDCP 2.2 keys.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_HdcpSetKey(XV_HdmiTxSs *InstancePtr, XV_HdmiTxSs_HdcpKeyType KeyType, u8 *KeyPtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((KeyType == XV_HDMITXSS_KEY_HDCP22_LC128)   ||
                    (KeyType == XV_HDMITXSS_KEY_HDCP14));

  switch (KeyType) {

    // HDCP 2.2 LC128
    case XV_HDMITXSS_KEY_HDCP22_LC128 :
      InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
      break;

    // HDCP 1.4
    case XV_HDMITXSS_KEY_HDCP14 :
      InstancePtr->Hdcp14KeyPtr = KeyPtr;
      break;

    default :
      break;
  }
}

/*****************************************************************************/
/**
*
* This function reports the HDCP info
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_HdcpInfo(XV_HdmiTxSs *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMITXSS_HDCP_NONE :
      xil_printf("\n\rHDCP is disabled\n\r");
      break;

    // HDCP 1.4
    case XV_HDMITXSS_HDCP_14 :
      xil_printf("\n\rHDCP 1.4 Info\n\r");
      if (XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr)) {

        // Route debug output to xil_printf
        XHdcp1x_SetDebugPrintf(xil_printf);

        // Display info
        XHdcp1x_Info(InstancePtr->Hdcp14Ptr);
      }

      else {
        xil_printf("Core is disabled\n\r");
      }

      break;

    // HDCP 2.2
    case XV_HDMITXSS_HDCP_22 :
      xil_printf("\n\rHDCP 2.2 Info\n\r");

      if (XHdcp22Tx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
        xil_printf("State : ");
        switch (InstancePtr->Hdcp22Ptr->Info.AuthenticationStatus) {
          case XHDCP22_TX_INCOMPATIBLE_RX :
            xil_printf("RX is incompatible.\n\r");
            break;

          case XHDCP22_TX_AUTHENTICATION_BUSY :
            xil_printf("Busy Authentication.\n\r");
            break;

          case XHDCP22_TX_REAUTHENTICATE_REQUESTED :
            xil_printf("Re-authentication Requested.\n\r");
            break;

          case XHDCP22_TX_UNAUTHENTICATED :
            xil_printf("Not Authenticated.\n\r");
            break;

          case XHDCP22_TX_AUTHENTICATED :
            xil_printf("Authenticated.\n\r");
            break;

          default :
            xil_printf("Unknown state.\n\r");
            break;
        }

        xil_printf("Encryption : ");
        if (XHdcp22Tx_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr)) {
          xil_printf("Enabled.\n\r");
        } else {
          xil_printf("Disabled.\n\r");
        }

        XHdcp22Tx_LogDisplay(InstancePtr->Hdcp22Ptr);

      } else {
        xil_printf("Core is disabled.\n\r");
      }

      break;
  }
}

/*****************************************************************************/
/**
*
* This function sets the logging level.
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_HdcpSetInfoDetail(XV_HdmiTxSs *InstancePtr, u8 Verbose)
{
  /* Verify argument. */
    Xil_AssertVoid(InstancePtr != NULL);

    if (Verbose) {
      // HDCP 1.4
      if (InstancePtr->Hdcp14Ptr) {
      XHdcp1x_SetDebugLogMsg(xil_printf);
      }

      // HDCP 2.2
      if (InstancePtr->Hdcp22Ptr) {
      XHdcp22Tx_LogReset(InstancePtr->Hdcp22Ptr, TRUE);
      }
  }

  else {
      // HDCP 1.4
      if (InstancePtr->Hdcp14Ptr) {
      XHdcp1x_SetDebugLogMsg(NULL);
      }

      // HDCP 2.2
      if (InstancePtr->Hdcp22Ptr) {
      XHdcp22Tx_LogReset(InstancePtr->Hdcp22Ptr, FALSE);
      }
  }
}
