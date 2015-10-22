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
#include "xv_hdmirxss.h"
#include "xv_hdmirxss_coreinit.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
  XTmrCtr HdcpTimer;
  XHdcp1x Hdcp;
  XV_HdmiRx HdmiRx;
}XV_HdmiRxSs_SubCores;

/**************************** Local Global ***********************************/
XV_HdmiRxSs_SubCores XV_HdmiRxSs_SubCoreRepo[XPAR_XV_HDMIRXSS_NUM_INSTANCES];
                               /**< Define Driver instance of all sub-core
                                    included in the design */


/************************** Function Prototypes ******************************/
static void XV_HdmiRxSs_GetIncludedSubcores(XV_HdmiRxSs *HdmiRxSsPtr,
       u16 DevId);
static XV_HdmiRxSs_SubCores *XV_HdmiRxSs_GetSubSysStruct(void *SubCorePtr);
static void XV_HdmiRxSs_WaitUs(XV_HdmiRxSs *InstancePtr, u32 MicroSeconds);
static int XV_HdmiRxSs_RegisterSubsysCallbacks(XV_HdmiRxSs *InstancePtr);
static void XV_HdmiRxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiRxSs_AuxCallback(void *CallbackRef);
static void XV_HdmiRxSs_AudCallback(void *CallbackRef);
static void XV_HdmiRxSs_LnkStaCallback(void *CallbackRef);
static void XV_HdmiRxSs_DdcCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamDownCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamInitCallback(void *CallbackRef);
static void XV_HdmiRxSs_StreamUpCallback(void *CallbackRef);
static u32 XV_HdmiRxSs_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
                                                                                       u32 ClockFrequency);
static void XV_HdmiRxSs_HdcpTimerCallback(void *CallBackRef, u8 TimerChannel);

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
void XV_HdmiRxSs_ReportCoreInfo(XV_HdmiRxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->HDMI RX Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if(InstancePtr->HdmiRxPtr)
  {
    xil_printf("    : HDMI RX \r\n");
  }

  if(InstancePtr->HdcpPtr)
  {
    xil_printf("    : HDCP RX \r\n");
  }

  if(InstancePtr->HdcpTimerPtr)
  {
    xil_printf("    : HDCP: AXIS Timer\r\n");
  }
}

/******************************************************************************/
/**
 * This function installs a custom delay/sleep function to be used by the XV_HdmiRxSs
 * driver.
 *
 * @param      InstancePtr is a pointer to the XDptx instance.
 * @param      CallbackFunc is the address to the callback function.
 * @param      CallbackRef is the user data item (microseconds to delay) that
 *             will be passed to the custom sleep/delay function when it is
 *             invoked.
 *
 * @return     None.
 *
 * @note       None.
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
 * This function is the delay/sleep function for the XV_HdmiTxSs driver. For the Zynq
 * family, there exists native sleep functionality. For MicroBlaze however,
 * there does not exist such functionality. In the MicroBlaze case, the default
 * method for delaying is to use a predetermined amount of loop iterations. This
 * method is prone to inaccuracy and dependent on system configuration; for
 * greater accuracy, the user may supply their own delay/sleep handler, pointed
 * to by InstancePtr->UserTimerWaitUs, which may have better accuracy if a
 * hardware timer is used.
 *
 * @param      InstancePtr is a pointer to the XDptx instance.
 * @param      MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return     None.
 *
 * @note       None.
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
 * This function calls the interrupt handler for HDCP
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS_HdcpIntrHandler(XV_HdmiRxSs *InstancePtr)
{
       XHdcp1x_CipherIntrHandler(InstancePtr->HdcpPtr);
}

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
  int Status;

  //Register HDMI Rx ISR
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

  //Register HDCP RX ISR
  if (HdmiRxSsPtr->HdcpPtr) {
  }

  //Register HDCP RX Timer ISR
  if (HdmiRxSsPtr->HdcpTimerPtr) {
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
  HdmiRxSsPtr->HdmiRxPtr     = ((HdmiRxSsPtr->Config.HdmiRx.IsPresent)    \
                        ? (&XV_HdmiRxSs_SubCoreRepo[DevId].HdmiRx) : NULL);
  HdmiRxSsPtr->HdcpPtr       = ((HdmiRxSsPtr->Config.Hdcp.IsPresent) \
                        ? (&XV_HdmiRxSs_SubCoreRepo[DevId].Hdcp) : NULL);
  HdmiRxSsPtr->HdcpTimerPtr  = ((HdmiRxSsPtr->Config.HdcpTimer.IsPresent) \
                        ? (&XV_HdmiRxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
}

/*****************************************************************************/
/**
* This function searches for the XV_HdmiRxSs_SubCores pointer
*
* @param  SubCorePtr address of reference subcore
*
* @return Pointer to XV_HdmiRxSs_SubCoreRepo
*
******************************************************************************/
static XV_HdmiRxSs_SubCores *XV_HdmiRxSs_GetSubSysStruct(void *SubCorePtr)
{
       int i;

       for (i=0; i<XPAR_XV_HDMIRXSS_NUM_INSTANCES;i++){
               if(&XV_HdmiRxSs_SubCoreRepo[i].HdmiRx == SubCorePtr){
                       return &XV_HdmiRxSs_SubCoreRepo[i];
               }

               if(&XV_HdmiRxSs_SubCoreRepo[i].Hdcp == SubCorePtr){
                       return &XV_HdmiRxSs_SubCoreRepo[i];
               }

               if(&XV_HdmiRxSs_SubCoreRepo[i].HdcpTimer == SubCorePtr){
                       return &XV_HdmiRxSs_SubCoreRepo[i];
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
int XV_HdmiRxSs_CfgInitialize(XV_HdmiRxSs *InstancePtr,
       XV_HdmiRxSs_Config *CfgPtr,
    u32 EffectiveAddr)
{
  XV_HdmiRxSs *HdmiRxSsPtr = InstancePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiRxSsPtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

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

  if(HdmiRxSsPtr->HdcpPtr)
  {
       if(XV_HdmiRxSs_SubcoreInitHdcp(HdmiRxSsPtr) != XST_SUCCESS)
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

  /* Register Callbacks */
  XV_HdmiRxSs_RegisterSubsysCallbacks(HdmiRxSsPtr);

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
//  u8 *pStartCore;

  Xil_AssertVoid(InstancePtr != NULL);

//  pStartCore = &InstancePtr->idata.startCore[0];
  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Start HDMI RX Subsystem.... \r\n");

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

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Stop HDMI RX Subsystem.... \r\n");
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

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Reset HDMI RX Subsystem.... \r\n");

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
    xil_printf("RX cable is connected\n\r");

    // Set RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, TRUE);

    HdmiRxSsPtr->IsStreamConnected = (TRUE);
  }

  // RX cable is disconnected
  else {
    xil_printf("RX cable is disconnected\n\r");

    // Clear RX hot plug detect
    XV_HdmiRx_SetHpd(HdmiRxSsPtr->HdmiRxPtr, FALSE);

    XV_HdmiRx_SetScrambler(HdmiRxSsPtr->HdmiRxPtr, (FALSE)); // Disable scrambler
    HdmiRxSsPtr->IsStreamConnected = (FALSE);
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

  // Check if user callback has been registered
  if (HdmiRxSsPtr->AuxCallback) {
         HdmiRxSsPtr->AuxCallback(HdmiRxSsPtr->AuxRef);
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

    xil_printf("Active audio channels %d\n\r", Channels);
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

  if (HdmiRxSsPtr->HdcpPtr) {

         XV_HdmiRx_DdcHdcpDisable(HdmiRxSsPtr->HdmiRxPtr);

         /* Set the RX HDCP state to down */
         XHdcp1x_SetPhysicalState(HdmiRxSsPtr->HdcpPtr, FALSE);
  }

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
  u32 count = 0;

  /* Clear link Status error counters */
  XV_HdmiRx_ClearLinkStatus(HdmiRxSsPtr->HdmiRxPtr);

  if (HdmiRxSsPtr->HdcpPtr) {

         XV_HdmiRx_DdcHdcpEnable(HdmiRxSsPtr->HdmiRxPtr);

         do{
                 count++;
         }while (XHdcp1x_CipherXorInProgress(HdmiRxSsPtr->HdcpPtr));

         /* Set the RX HDCP state to up */
         XHdcp1x_SetPhysicalState(HdmiRxSsPtr->HdcpPtr, TRUE);
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
* (XV_HDMIRXSS_HANDLER_CONNECT)       HpdCallback
* (XV_HDMIRXSS_HANDLER_VS)            VsCallback
* (XV_HDMIRXSS_HANDLER_STREAM_DOWN)   SreamDownCallback
* (XV_HDMIRXSS_HANDLER_STREAM_UP)     SreamUpCallback
* </pre>
*
* @param       InstancePtr is a pointer to the HDMI RX Subsystem instance.
* @param       HandlerType specifies the type of handler.
* @param       CallbackFunc is the address of the callback function.
* @param       CallbackRef is a user data item that will be passed to the
*              callback function when it is invoked.
*
* @return
*              - XST_SUCCESS if callback function installed successfully.
*              - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note                Invoking this function for a handler that already has been
*              installed replaces it with the new handler.
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
    // Load new EDID
    XV_HdmiRx_DdcLoadEdid(InstancePtr->HdmiRxPtr, InstancePtr->EdidPtr,
                                                                                               InstancePtr->EdidLength);

    print("\n\r");
    print("Succesfully loaded edid.\n\r");
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
    // Load new EDID
    XV_HdmiRx_DdcLoadEdid(InstancePtr->HdmiRxPtr, EdidDataPtr, Length);

    print("\n\r");
    print("Succesfully loaded edid.\n\r");
}

/*****************************************************************************/
/**
*
* This function toggles the HPD on the HDMI RX
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs_ToggleHpd(XV_HdmiRxSs *InstancePtr)
{
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (FALSE));   // Drive HPD low
  XV_HdmiRxSs_WaitUs(InstancePtr, 500000); // Wait for 500 ms
  //MB_Sleep(500);                    // Wait for 500 ms
  XV_HdmiRx_SetHpd(InstancePtr->HdmiRxPtr, (TRUE));    // Drive HPD high
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
u32 XV_HdmiRxSs_SetStream(XV_HdmiRxSs *InstancePtr, XVidC_PixelsPerClock Ppc,
               u32 Clock, u32 LineRate)
{
  // Check line rate
  // For 4k60p select 4 pixels per clock
  if (LineRate > 3400) {
    XV_HdmiRx_SetStream(InstancePtr->HdmiRxPtr, XVIDC_PPC_4, Clock);
    InstancePtr->HdmiRxPtr->Stream.PixelClk = Clock * 4;
  }

  // Other line rate 2 pixels per clock
  else
  {
    XV_HdmiRx_SetStream(InstancePtr->HdmiRxPtr, XVIDC_PPC_2, Clock);
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

  if (InstancePtr->HdcpPtr){
        Data = XHdcp1x_GetVersion(InstancePtr->HdcpPtr);
        xil_printf("  HDCP RX version : %02d.%02d (%04x)\n\r",
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
void XV_HdmiRxSs_HdcpEnable(XV_HdmiRxSs *InstancePtr, u8 Enable)
{
       XV_HdmiRxSs *HdmiRxSsPtr = InstancePtr;

       if(Enable){
               /* Enable hdcp interface */
               XHdcp1x_Enable(HdmiRxSsPtr->HdcpPtr);

               /* Set the RX HDCP state to up */
               XHdcp1x_SetPhysicalState(HdmiRxSsPtr->HdcpPtr, TRUE);
       }
       else {
           /* Set the RX HDCP state to down */
           XHdcp1x_SetPhysicalState(HdmiRxSsPtr->HdcpPtr, FALSE);

               /* Disable hdcp interface */
               XHdcp1x_Disable(HdmiRxSsPtr->HdcpPtr);
       }
}

/*****************************************************************************/
/**
*
* This function polls the hdcp example module and determines if the video
* stream is encrypted. The traffic is encrypted if the encryption bit map
* is non-zero and the interface is authenticated.
*
* @return      Truth value indicating encrypted (TRUE) or not (FALSE).
*
* @note
*   This function is intended to be called from within the main loop of the
*   software utilizing this module.
*
******************************************************************************/
u8 XV_HdmiRxSs_HdcpPoll(XV_HdmiRxSs *InstancePtr)
{
       /* Locals */
       XV_HdmiRxSs *HdmiRxSsPtr = InstancePtr;

       /* Poll hdcp interface */
       XHdcp1x_Poll(HdmiRxSsPtr->HdcpPtr);

       /* Return */
       return ((u8) XHdcp1x_IsEncrypted(HdmiRxSsPtr->HdcpPtr));
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
int XV_HdmiRxSs_HdcpTimerStart(const XHdcp1x* InstancePtr, u16 TimeoutInMs)
{
       XV_HdmiRxSs_SubCores *SubCorePtr;
       XTmrCtr *TimerPtr;
       u8 TimerChannel = 0;
       u32 TimerOptions = 0;
       u32 NumTicks = 0;

       SubCorePtr = XV_HdmiRxSs_GetSubSysStruct((void*)InstancePtr);
       TimerPtr = &SubCorePtr->HdcpTimer;

       /* Determine NumTicks */
       NumTicks = XV_HdmiRxSs_HdcpTimerConvUsToTicks((TimeoutInMs*1000ul),
                                                                                       XPAR_CPU_CORE_CLOCK_FREQ_HZ);

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
int XV_HdmiRxSs_HdcpTimerStop(const XHdcp1x* InstancePtr)
{
       XV_HdmiRxSs_SubCores *SubCorePtr;
       XTmrCtr *TimerPtr;
       u8 TimerChannel = 0;

       SubCorePtr = XV_HdmiRxSs_GetSubSysStruct((void*)InstancePtr);
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
int XV_HdmiRxSs_HdcpTimerBusyDelay(const XHdcp1x* InstancePtr, u16 DelayInMs)
{
       XV_HdmiRxSs_SubCores *SubCorePtr;
       XTmrCtr *TimerPtr;
       u8 TimerChannel = 0;
       u32 TimerOptions = 0;
       u32 NumTicks = 0;

       SubCorePtr = XV_HdmiRxSs_GetSubSysStruct((void*)InstancePtr);
       TimerPtr = &SubCorePtr->HdcpTimer;

       /* Determine NumTicks */
       NumTicks = XV_HdmiRxSs_HdcpTimerConvUsToTicks((DelayInMs*1000ul),
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
