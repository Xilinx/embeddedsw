/******************************************************************************
# Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1_hdcp.c
*
* This is main code of Xilinx HDMI Receiver Subsystem for HDCP Functionality.
* Please see xv_hdmirxss1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  YB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmirxss1.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
/* HDCP specific*/
#ifdef USE_HDCP_RX
static XV_HdmiRxSs1_HdcpEvent XV_HdmiRxSs1_HdcpGetEvent(XV_HdmiRxSs1 *InstancePtr);
static int XV_HdmiRxSs1_HdcpProcessEvents(XV_HdmiRxSs1 *InstancePtr);
static int XV_HdmiRxSs1_HdcpReset(XV_HdmiRxSs1 *InstancePtr);
static int XV_HdmiRxSs1_HdcpSetTopologyDepth(XV_HdmiRxSs1 *InstancePtr, u32 Depth);
static int XV_HdmiRxSs1_HdcpSetTopologyDeviceCnt(XV_HdmiRxSs1 *InstancePtr, u32 DeviceCnt);
static int XV_HdmiRxSs1_HdcpSetTopologyMaxDevsExceeded(XV_HdmiRxSs1 *InstancePtr, u8 Value);
static int XV_HdmiRxSs1_HdcpSetTopologyMaxCascadeExceeded(XV_HdmiRxSs1 *InstancePtr, u8 Value);
static int XV_HdmiRxSs1_HdcpSetTopologyHdcp2LegacyDeviceDownstream(XV_HdmiRxSs1 *InstancePtr, u8 Value);
static int XV_HdmiRxSs1_HdcpSetTopologyHdcp1DeviceDownstream(XV_HdmiRxSs1 *InstancePtr, u8 Value);
#endif
#ifdef XPAR_XHDCP_NUM_INSTANCES
static u32 XV_HdmiRxSs1_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
                                            u32 ClockFrequency);
static void XV_HdmiRxSs1_HdcpTimerCallback(void *CallBackRef, u8 TimerChannel);
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definition ******************************/
#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDCP
 *
 * @param  InstancePtr is a pointer to the HDMI RX Subsystem
 *
 *****************************************************************************/
void XV_HdmiRxSS1_HdcpIntrHandler(XV_HdmiRxSs1 *InstancePtr)
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
void XV_HdmiRxSS1_HdcpTimerIntrHandler(XV_HdmiRxSs1 *InstancePtr)
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
void XV_HdmiRxSS1_Hdcp22TimerIntrHandler(XV_HdmiRxSs1 *InstancePtr)
{
  XTmrCtr *XTmrCtrPtr;

  XTmrCtrPtr = XHdcp22Rx_GetTimer(InstancePtr->Hdcp22Ptr);

  XTmrCtr_InterruptHandler(XTmrCtrPtr);
}
#endif


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
static u32 XV_HdmiRxSs1_HdcpTimerConvUsToTicks(u32 TimeoutInUs,
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
static void XV_HdmiRxSs1_HdcpTimerCallback(void* CallBackRef, u8 TimerChannel)
{
  XHdcp1x* HdcpPtr = (XHdcp1x*) CallBackRef;

  TimerChannel = TimerChannel;
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
int XV_HdmiRxSs1_HdcpTimerStart(void *InstancePtr, u16 TimeoutInMs)
{
  XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
  XTmrCtr *TimerPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

  u8 TimerChannel = 0;
  u32 TimerOptions = 0;
  u32 NumTicks = 0;

  /* Verify argument. */
  Xil_AssertNonvoid(TimerPtr != NULL);

  /* Determine NumTicks */
  NumTicks = XV_HdmiRxSs1_HdcpTimerConvUsToTicks((TimeoutInMs*1000ul),
        TimerPtr->Config.SysClockFreqHz);

  /* Stop it */
  XTmrCtr_Stop(TimerPtr, TimerChannel);

  /* Configure the callback */
  XTmrCtr_SetHandler(TimerPtr, &XV_HdmiRxSs1_HdcpTimerCallback,
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
int XV_HdmiRxSs1_HdcpTimerStop(void *InstancePtr)
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
int XV_HdmiRxSs1_HdcpTimerBusyDelay(void *InstancePtr, u16 DelayInMs)
{

  XHdcp1x *HdcpPtr = (XHdcp1x *)InstancePtr;
  XTmrCtr *TimerPtr = (XTmrCtr *)HdcpPtr->Hdcp1xRef;

  u8 TimerChannel = 0;
  u32 TimerOptions = 0;
  u32 NumTicks = 0;

  /* Verify argument. */
	Xil_AssertNonvoid(TimerPtr != NULL);

  /* Determine NumTicks */
  NumTicks = XV_HdmiRxSs1_HdcpTimerConvUsToTicks((DelayInMs*1000ul),
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function pushes an event into the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Event is the event to be pushed in the queue.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpPushEvent(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpEvent Event)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Event < XV_HDMIRXSS1_HDCP_INVALID_EVT);

  /* Write event into the queue */
  InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Head] = Event;

  /* Update head pointer */
  if (InstancePtr->HdcpEventQueue.Head == (XV_HDMIRXSS1_HDCP_MAX_QUEUE_SIZE - 1)) {
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
    if (InstancePtr->HdcpEventQueue.Tail == (XV_HDMIRXSS1_HDCP_MAX_QUEUE_SIZE - 1)) {
      InstancePtr->HdcpEventQueue.Tail = 0;
    }
    else {
      InstancePtr->HdcpEventQueue.Tail++;
    }
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function gets an event from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return When the queue is filled, the next event is returned.
*         When the queue is empty, XV_HDMIRXSS1_HDCP_NO_EVT is returned.
*
* @note   None.
*
******************************************************************************/
static XV_HdmiRxSs1_HdcpEvent XV_HdmiRxSs1_HdcpGetEvent(XV_HdmiRxSs1 *InstancePtr)
{
  XV_HdmiRxSs1_HdcpEvent Event;

  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Check if there are any events in the queue */
  if (InstancePtr->HdcpEventQueue.Tail == InstancePtr->HdcpEventQueue.Head) {
    return XV_HDMIRXSS1_HDCP_NO_EVT;
  }

  Event = InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Tail];

  /* Update tail pointer */
  if (InstancePtr->HdcpEventQueue.Tail == (XV_HDMIRXSS1_HDCP_MAX_QUEUE_SIZE - 1)) {
    InstancePtr->HdcpEventQueue.Tail = 0;
  }
  else {
    InstancePtr->HdcpEventQueue.Tail++;
  }

  return Event;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function clears all pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpClearEvents(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  InstancePtr->HdcpEventQueue.Head = 0;
  InstancePtr->HdcpEventQueue.Tail = 0;

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function processes pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpProcessEvents(XV_HdmiRxSs1 *InstancePtr)
{
  XV_HdmiRxSs1_HdcpEvent Event;
  int Status = XST_SUCCESS;

  /* Verify argument */
  Xil_AssertNonvoid(InstancePtr != NULL);

  Event = XV_HdmiRxSs1_HdcpGetEvent(InstancePtr);

  switch (Event) {

    /* Stream up*/
    case XV_HDMIRXSS1_HDCP_STREAMUP_EVT :
      break;

    /* Stream down*/
    case XV_HDMIRXSS1_HDCP_STREAMDOWN_EVT :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, FALSE);
      }
#endif
      break;

    /* Connect*/
    case XV_HDMIRXSS1_HDCP_CONNECT_EVT :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        /* Set physical state*/
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, TRUE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
      }
#endif
      XV_HdmiRxSs1_HdcpSetProtocol(InstancePtr, InstancePtr->HdcpProtocol);
      break;

    /* Disconnect*/
    /* Enable the previous HDCP protocol*/
    case XV_HDMIRXSS1_HDCP_DISCONNECT_EVT :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        /* Clear HDMI mode*/
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, FALSE);

        /* Set physical state*/
        XHdcp1x_SetPhysicalState(InstancePtr->Hdcp14Ptr, FALSE);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
      }
#endif
      break;

    /* DVI mode event*/
    case XV_HDMIRXSS1_HDCP_DVI_MODE_EVT:
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, FALSE);
      }
#endif
      break;

    /* HDMI mode event*/
    case XV_HDMIRXSS1_HDCP_HDMI_MODE_EVT:
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, TRUE);
      }
#endif
      break;

    /* Sync loss event*/
    case XV_HDMIRXSS1_HDCP_SYNC_LOSS_EVT:
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, FALSE);
      }
#endif
      break;

      /* Sync est/recover event*/
      case XV_HDMIRXSS1_HDCP_SYNC_EST_EVT:
  #ifdef XPAR_XHDCP_NUM_INSTANCES
        if (InstancePtr->Hdcp14Ptr) {
          XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, TRUE);
        }
  #endif
        break;

    default :
      break;
  }

  return Status;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function schedules the available HDCP cores. Only the active
* HDCP protocol poll function is executed. HDCP 1.4 and 2.2 poll
* functions should not execute in parallel.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpPoll(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Only poll when the HDCP is ready */
  if (InstancePtr->HdcpIsReady) {

    /* Process any pending events from the RX event queue */
    XV_HdmiRxSs1_HdcpProcessEvents(InstancePtr);

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    if (InstancePtr->Hdcp22Ptr) {
      if (XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
        XHdcp22Rx_Poll(InstancePtr->Hdcp22Ptr);
      }
    }
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the active HDCP protocol and enables it.
* The protocol can be set to either HDCP 1.4, 2.2, or None.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Protocol is the requested content protection scheme of type
*        XV_HdmiRxSs1_HdcpProtocol.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetProtocol(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpProtocol Protocol)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid((Protocol == XV_HDMIRXSS1_HDCP_NONE) ||
                    (Protocol == XV_HDMIRXSS1_HDCP_14)   ||
                    (Protocol == XV_HDMIRXSS1_HDCP_22));

  int Status;

  /* Set requested protocol */
  InstancePtr->HdcpProtocol = Protocol;

  /* Reset both protocols */
  Status = XV_HdmiRxSs1_HdcpReset(InstancePtr);
  if (Status != XST_SUCCESS) {
    InstancePtr->HdcpProtocol = XV_HDMIRXSS1_HDCP_NONE;
    return XST_FAILURE;
  }

  /* Enable the requested protocol */
  Status = XV_HdmiRxSs1_HdcpEnable(InstancePtr);
  if (Status != XST_SUCCESS) {
    InstancePtr->HdcpProtocol = XV_HDMIRXSS1_HDCP_NONE;
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the desired HDCP capability and enables it.
* The protocol can be set to either both or none. Selective
* enablement of HDCP 1.4 and 2.2 is not yet supported.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Protocol is the requested content protection scheme of type
*        XV_HdmiRxSs1_HdcpProtocol. Only options currently supported
*        is both and none.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetCapability(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpProtocol Protocol)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Protocol <= XV_HDMIRXSS1_HDCP_BOTH);

    /* Enable HDCP capable for both protocols*/
    /* DDC slave is enabled*/
    switch (Protocol) {

    /* Disable HDCP capable for both protocols*/
    /* DDC slave is disabled*/
    case XV_HDMIRXSS1_HDCP_NONE:
      XV_HdmiRx1_DdcIntrDisable(InstancePtr->HdmiRx1Ptr);
      XV_HdmiRx1_DdcHdcpDisable(InstancePtr->HdmiRx1Ptr);
      XV_HdmiRxSs1_HdcpDisable(InstancePtr);
      break;

    /* Enable HDCP capable for both protocols*/
    /* DDC slave is enabled*/
    default :
      XV_HdmiRx1_DdcIntrEnable(InstancePtr->HdmiRx1Ptr);
      XV_HdmiRx1_DdcHdcpEnable(InstancePtr->HdmiRx1Ptr);
      break;
  }

  return XST_SUCCESS;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function gets the active HDCP content protection scheme.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  @RequestedScheme is the requested content protection scheme.
*
* @note   None.
*
******************************************************************************/
XV_HdmiRxSs1_HdcpProtocol XV_HdmiRxSs1_HdcpGetProtocol(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return InstancePtr->HdcpProtocol;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function enables the requested HDCP protocol. This function
* ensures that the HDCP protocols are mutually exclusive such that
* either HDCP 1.4 or HDCP 2.2 is enabled and active at any given time.
* When the protocol is set to None, both HDCP protocols are disabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpEnable(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status1 = XST_SUCCESS;
  int Status2 = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol) {

    /* Disable HDCP 1.4 and HDCP 2.2 */
    case XV_HDMIRXSS1_HDCP_NONE :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP14, 0);
#endif
      }
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP22, 0);
#endif
      }
#endif
      break;

    /* Enable HDCP 1.4 and disable HDCP 2.2 */
    case XV_HDMIRXSS1_HDCP_14 :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Enable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP14, 1);
#endif
      }
      else {
        Status1 = XST_FAILURE;
      }

      /* Set DDC peripheral to HDCP 1.4 mode */
      XV_HdmiRx1_DdcHdcp14Mode(InstancePtr->HdmiRx1Ptr);
#else
      Status1 = XST_FAILURE;
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Disable(InstancePtr->Hdcp22Ptr);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP22, 0);
#endif
      }
#endif
      break;

    /* Enable HDCP 2.2 and disable HDCP 1.4 */
    case XV_HDMIRXSS1_HDCP_22 :
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (InstancePtr->Hdcp14Ptr) {
        Status1 = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
        XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP14, 0);
#endif
      }
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (InstancePtr->Hdcp22Ptr) {
        Status2 = XHdcp22Rx_Enable(InstancePtr->Hdcp22Ptr);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
        XV_HdmiRxSs1_LogWrite(InstancePtr, XV_HDMIRXSS1_LOG_EVT_HDCP22, 1);
#endif
      }
      else {
        Status2 = XST_FAILURE;
      }

      /* Set DDC peripheral to HDCP 2.2 mode */
      XV_HdmiRx1_DdcHdcp22Mode(InstancePtr->HdmiRx1Ptr);
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function disables both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpDisable(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  /* Set protocol to NONE then reset/disable HDCP 1.4 and 2.2 */
  InstancePtr->HdcpProtocol = XV_HDMIRXSS1_HDCP_NONE;
  Status = XV_HdmiRxSs1_HdcpReset(InstancePtr);

  return Status;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function resets both HDCP 1.4 and 2.2 protocols. This function also
* disables both HDCP 1.4 and 2.2 protocols.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpReset(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

#ifdef XPAR_XHDCP_NUM_INSTANCES
  /* HDCP 1.4*/
  /* Resetting HDCP 1.4 causes the state machine to be enabled, therefore*/
  /* disable must be called immediately after reset is called.*/
  if (InstancePtr->Hdcp14Ptr) {
    Status = XHdcp1x_Reset(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
    if (Status != XST_SUCCESS)
      return XST_FAILURE;

    Status = XHdcp1x_Disable(InstancePtr->Hdcp14Ptr);
    XHdcp1x_Poll(InstancePtr->Hdcp14Ptr); /* This is needed to ensure that the previous command is executed.*/
    if (Status != XST_SUCCESS)
      return XST_FAILURE;
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  /* HDCP 2.2*/
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol is enabled
*  - FALSE if active protocol is disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsEnabled(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS1_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_14 :
      return XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_22 :
      return XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is authenticated.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol is authenticated
*  - FALSE if active protocol is not authenticated
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsAuthenticated(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS1_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_14 :
      return XHdcp1x_IsAuthenticated(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_22 :
      return XHdcp22Rx_IsAuthenticated(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol has encryption enabled.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol has encryption enabled
*  - FALSE if active protocol has encryption disabled
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsEncrypted(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS1_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_14 :
      return XHdcp1x_IsEncrypted(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_22 :
      return XHdcp22Rx_IsEncryptionEnabled(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is busy authenticating.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol is busy authenticating
*  - FALSE if active protocol is not busy authenticating
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsInProgress(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS1_HDCP_NONE :
      return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_14 :
      return XHdcp1x_IsInProgress(InstancePtr->Hdcp14Ptr);
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    case XV_HDMIRXSS1_HDCP_22 :
      return XHdcp22Rx_IsInProgress(InstancePtr->Hdcp22Ptr);
#endif

    default :
      return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is in computations state.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol is in computations state
*  - FALSE if active protocol is not in computations state
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsInComputations(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
	case XV_HDMIRXSS1_HDCP_NONE :
	  return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
	case XV_HDMIRXSS1_HDCP_14 :
	  return XHdcp1x_IsInComputations(InstancePtr->Hdcp14Ptr);
#endif

	case XV_HDMIRXSS1_HDCP_22 :
	  return FALSE;

	default :
	  return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the active HDCP protocol is in wait-for-ready state.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*  - TRUE if active protocol is in computations state
*  - FALSE if active protocol is not in computations state
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsInWaitforready(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
	case XV_HDMIRXSS1_HDCP_NONE :
	  return FALSE;

#ifdef XPAR_XHDCP_NUM_INSTANCES
	case XV_HDMIRXSS1_HDCP_14 :
	  return XHdcp1x_IsInWaitforready(InstancePtr->Hdcp14Ptr);
#endif

	case XV_HDMIRXSS1_HDCP_22 :
	  return FALSE;

	default :
	  return FALSE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets pointers to the HDCP 1.4 and HDCP 2.2 keys.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_HdcpSetKey(XV_HdmiRxSs1 *InstancePtr, XV_HdmiRxSs1_HdcpKeyType KeyType, u8 *KeyPtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((KeyType == XV_HDMIRXSS1_KEY_HDCP22_LC128)   ||
                   (KeyType == XV_HDMIRXSS1_KEY_HDCP22_PRIVATE)   ||
                    (KeyType == XV_HDMIRXSS1_KEY_HDCP14));

  switch (KeyType) {

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2 LC128*/
    case XV_HDMIRXSS1_KEY_HDCP22_LC128 :
      InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
      break;

    /* HDCP 2.2 Private key*/
    case XV_HDMIRXSS1_KEY_HDCP22_PRIVATE :
      InstancePtr->Hdcp22PrivateKeyPtr = KeyPtr;
      break;
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_KEY_HDCP14 :
      InstancePtr->Hdcp14KeyPtr = KeyPtr;
#endif
      break;

    default :
      break;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function reports the HDCP information.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_HdcpInfo(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);

  switch (InstancePtr->HdcpProtocol) {
    case XV_HDMIRXSS1_HDCP_NONE :
      xil_printf("\r\nHDCP RX is disabled\r\n");
      xil_printf("---------\r\n");
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        if (XHdcp1x_IsEnabled(InstancePtr->Hdcp14Ptr)) {
          xil_printf("\r\nHDCP 1.4 RX Info\r\n");
          xil_printf("---------\r\n");

          xil_printf("Encryption : ");
          if (XHdcp1x_IsEncrypted(InstancePtr->Hdcp14Ptr)) {
            xil_printf("Enabled.\r\n");
          } else {
            xil_printf("Disabled.\r\n");
          }

          /* Route debug output to xil_printf*/
          XHdcp1x_SetDebugPrintf(xil_printf);

          /* Display info*/
          XHdcp1x_Info(InstancePtr->Hdcp14Ptr);
        }
        else {
          xil_printf("\r\nHDCP 1.4 RX is disabled\r\n");
          xil_printf("---------\r\n");
        }
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        if (XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
            if (InstancePtr->EnableHDCPLogging == TRUE) {
              XHdcp22Rx_LogDisplay(InstancePtr->Hdcp22Ptr);
            }

          xil_printf("\r\nHDCP 2.2 RX Info\r\n");
          xil_printf("---------\r\n");
          XHdcp22Rx_Info(InstancePtr->Hdcp22Ptr);
        }
        else {
          xil_printf("\r\nHDCP 2.2 RX is disabled\r\n");
          xil_printf("---------\r\n");
        }
      }
      break;
#endif

    default:
      xil_printf("\r\nHDCP info unknown?\r\n");
      xil_printf("---------\r\n");
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP logging level.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Verbose is set to TRUE to enable detailed logging.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
void XV_HdmiRxSs1_HdcpSetInfoDetail(XV_HdmiRxSs1 *InstancePtr, u8 Verbose)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);

  if (Verbose) {
#ifdef XPAR_XHDCP_NUM_INSTANCES
   /* HDCP 1.4*/
   if (InstancePtr->Hdcp14Ptr) {
     XHdcp1x_SetDebugLogMsg(xil_printf);
   }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
   /* HDCP 2.2*/
   if (InstancePtr->Hdcp22Ptr) {
     XHdcp22Rx_LogReset(InstancePtr->Hdcp22Ptr, TRUE);
   }
#endif
  }

  else {
#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    if (InstancePtr->Hdcp14Ptr) {
      XHdcp1x_SetDebugLogMsg(NULL);
    }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    if (InstancePtr->Hdcp22Ptr) {
      XHdcp22Rx_LogReset(InstancePtr->Hdcp22Ptr, FALSE);
    }
#endif
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function copies the HDCP repeater topology for the active protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param TopologyPtr is a pointer to the topology structure.
*
* @return .
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetTopology(XV_HdmiRxSs1 *InstancePtr, void *TopologyPtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(TopologyPtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopology(InstancePtr->Hdcp14Ptr, ((XHdcp1x_RepeaterExchange*)(TopologyPtr)));
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function copies the HDCP repeater topology list for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param ListPtr is a pointer to the Receiver ID list.
* @param ListSize is the number of Receiver IDs in the list.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetTopologyReceiverIdList(XV_HdmiRxSs1 *InstancePtr, u8 *ListPtr, u32 ListSize)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(ListPtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyKSVList(InstancePtr->Hdcp14Ptr, ListPtr,	ListSize);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets various fields inside the HDCP repeater topology.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Field indicates what field of the topology structure to update.
* @param Value is the value assigned to the field of the topology structure.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetTopologyField(XV_HdmiRxSs1 *InstancePtr,
      XV_HdmiRxSs1_HdcpTopologyField Field, u32 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Field < XV_HDMIRXSS1_HDCP_TOPOLOGY_INVALID);

  switch (Field)
  {
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_DEPTH:
      return XV_HdmiRxSs1_HdcpSetTopologyDepth(InstancePtr, Value);
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_DEVICECNT:
      return XV_HdmiRxSs1_HdcpSetTopologyDeviceCnt(InstancePtr, Value);
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_MAXDEVSEXCEEDED:
      return XV_HdmiRxSs1_HdcpSetTopologyMaxDevsExceeded(InstancePtr, Value);
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_MAXCASCADEEXCEEDED:
      return XV_HdmiRxSs1_HdcpSetTopologyMaxCascadeExceeded(InstancePtr, Value);
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_HDCP2LEGACYDEVICEDOWNSTREAM:
      return XV_HdmiRxSs1_HdcpSetTopologyHdcp2LegacyDeviceDownstream(InstancePtr, Value);
	  case XV_HDMIRXSS1_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM:
      return XV_HdmiRxSs1_HdcpSetTopologyHdcp1DeviceDownstream(InstancePtr, Value);
    default:
      return XST_FAILURE;
  }
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology depth for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Depth is the Repeater cascade depth.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyDepth(XV_HdmiRxSs1 *InstancePtr, u32 Depth)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_DEPTH, Depth);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology device count for the active
* protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param DeviceCnt is the Total number of connected downstream devices.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyDeviceCnt(XV_HdmiRxSs1 *InstancePtr, u32 DeviceCnt)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_DEVICECNT, DeviceCnt);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology maximum devices exceeded
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyMaxDevsExceeded(XV_HdmiRxSs1 *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_MAXDEVSEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology maximum cascade exceeded
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyMaxCascadeExceeded(XV_HdmiRxSs1 *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyField(InstancePtr->Hdcp14Ptr,
          XHDCP1X_TOPOLOGY_MAXCASCADEEXCEEDED, Value);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology HDCP2 legacy device
* downstream flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyHdcp2LegacyDeviceDownstream(XV_HdmiRxSs1 *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  Value = Value;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        XHdcp22Rx_SetTopologyField(InstancePtr->Hdcp22Ptr,
          XHDCP22_RX_TOPOLOGY_HDCP2LEGACYDEVICEDOWNSTREAM, Value);
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology HDCP 1.x repeater downstream
* flag.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiRxSs1_HdcpSetTopologyHdcp1DeviceDownstream(XV_HdmiRxSs1 *InstancePtr, u8 Value)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  Value = Value;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function sets the HDCP repeater topology update flag, indicating that
* the topology is ready for upstream propagation.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Value is either TRUE or FALSE.
*
* @return XST_SUCCESS or XST_FAILURE.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetTopologyUpdate(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = XST_SUCCESS;

  switch (InstancePtr->HdcpProtocol)
  {
    /* None*/
    case XV_HDMIRXSS1_HDCP_NONE:
      Status = XST_FAILURE;
      break;

#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetTopologyUpdate(InstancePtr->Hdcp14Ptr);
      } else {
        Status = XST_FAILURE;
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function gets the repeater management content stream type.
* When the protocol is HDCP 1.4 the stream type is always Type 0.
* For HDCP 2.2 the stream type is extracted from the HDCP 2.2
* stream manage message.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
XV_HdmiRxSs1_HdcpContentStreamType XV_HdmiRxSs1_HdcpGetContentStreamType(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int StreamType;

  switch (InstancePtr->HdcpProtocol)
  {
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      StreamType = XV_HDMIRXSS1_HDCP_STREAMTYPE_0;
      break;

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
      if (InstancePtr->Hdcp22Ptr) {
        StreamType = XHdcp22Rx_GetContentStreamType(InstancePtr->Hdcp22Ptr);
      } else {
        StreamType = XV_HDMIRXSS1_HDCP_STREAMTYPE_0;
      }
      break;
#endif

    default:
      StreamType = XV_HDMIRXSS1_HDCP_STREAMTYPE_0;
  }

  return (XV_HdmiRxSs1_HdcpContentStreamType) StreamType;
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function checks if the HDMI receiver is an HDCP repeater
* upstream interface for the active protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
*
* @return
*   - TRUE if repeater upstream interface.
*   - FALSE if not repeater upstream interface.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpIsRepeater(XV_HdmiRxSs1 *InstancePtr)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  int Status = (int)FALSE;

  switch (InstancePtr->HdcpProtocol)
  {
#ifdef XPAR_XHDCP_NUM_INSTANCES
    /* HDCP 1.4*/
    case XV_HDMIRXSS1_HDCP_14:
      if (InstancePtr->Hdcp14Ptr) {
        Status = XHdcp1x_IsRepeater(InstancePtr->Hdcp14Ptr);
      }
      break;
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
    /* HDCP 2.2*/
    case XV_HDMIRXSS1_HDCP_22:
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

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
*
* This function enables the Repeater functionality for the HDCP protocol.
*
* @param InstancePtr is a pointer to the XV_HdmiRxSs1 instance.
* @param Set is TRUE to enable and FALSE to disable repeater.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XV_HdmiRxSs1_HdcpSetRepeater(XV_HdmiRxSs1 *InstancePtr, u8 Set)
{
  /* Verify argument. */
  Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef XPAR_XHDCP_NUM_INSTANCES
  /* HDCP 1.4*/
  if (InstancePtr->Hdcp14Ptr) {
    XHdcp1x_SetRepeater(InstancePtr->Hdcp14Ptr, Set);
  }
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
  /* HDCP 2.2*/
  if (InstancePtr->Hdcp22Ptr) {
    XHdcp22Rx_SetRepeater(InstancePtr->Hdcp22Ptr, Set);
  }
#endif

  return XST_SUCCESS;
}
#endif
