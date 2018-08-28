/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xhdcp.c
*
* This file contains the main implementation of the Xilinx HDCP abstraction
* layer. The HDCP abstraction layer can support repeater topologies with a
* single upstream interface and up to 32 downstream interfaces. Both HDCP
* 1.4 and 2.2 protocols are supported. The interactions between the repeater
* upstream and downstream interface are implemented in the HDCP abstraction
* layer including: upstream topology propagation, and downstream stream
* management propagation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  MH   05/24/16 First Release
* 1.10  MG   10/21/16 Updated the Hdcp_Poll function
* 1.20  MG   10/26/16 Added interval in Hdcp_Poll function
* 1.30  MH   06/16/17 Removed authentication request flag.
*       GM   07/12/17 Changed printf usage to xil_printf
*                     Changed "\n\r" in xil_printf calls to "\r\n"
*       MH   08/04/17 Added ability to change HDCP capability
* 3.03  YB   08/14/18 Clubbing Repeater specific code under the
*                     'ENABLE_HDCP_REPEATER' macro.
*</pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xhdcp.h"
#include "xparameters.h"
#include "xhdmi_example.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
#ifdef USE_HDCP
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
static void XHdcp_EnforceBlank(XHdcp_Repeater *InstancePtr);
static void XHdcp_DownstreamAuthenticatedCallback(void *HdcpInstancePtr);
static void XHdcp_DownstreamUnauthenticatedCallback(void *HdcpInstancePtr);
#endif

/*****************************************************************************/
/**
*
* This function is used to initialize the HDCP repeater instance.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   - XST_SUCCESS or XST_FAILURE
*
* @note	    None.
*
******************************************************************************/
int XHdcp_Initialize(XHdcp_Repeater *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Set default values */
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  InstancePtr->DownstreamInstanceBinded = 0;
  InstancePtr->DownstreamInstanceConnected = 0;
  InstancePtr->DownstreamInstanceStreamUp = 0;
#endif
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  InstancePtr->EnforceBlocking = (TRUE);
  InstancePtr->StreamType = XV_HDMITXSS_HDCP_STREAMTYPE_0;
#endif

  /* Instance is ready only after upstream and at least one downstream
     has been binded */
  InstancePtr->IsReady = (FALSE);

  return (XST_SUCCESS);
}


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is used to bind an HDMI transmitter instance as
* a repeater downstream interface. This function should be called
* for each downstream interface in a repeater topology. The maximum
* downstream interfaced is indicated by XHDCP_MAX_DOWNSTREAM_INTERFACES.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    UpstreamInstancePtr is a pointer to the HDMI receiver instance.
*
* @return   - XST_SUCCESS if downstream interface registered successfully.
*           - XST_FAILURE if downstream interface could not be registered.
*
* @note	    None.
*
******************************************************************************/
int XHdcp_SetDownstream(XHdcp_Repeater *InstancePtr,
      XV_HdmiTxSs *DownstreamInstancePtr)
{
  int Status;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(DownstreamInstancePtr != NULL);

  /* Bind downstream interface */
  if (InstancePtr->DownstreamInstanceBinded <= XHDCP_MAX_DOWNSTREAM_INTERFACES) {
    InstancePtr->DownstreamInstancePtr[InstancePtr->DownstreamInstanceBinded] =
      DownstreamInstancePtr;
  } else {
    return (XST_FAILURE);
  }


  Status = XV_HdmiTxSs_SetCallback(DownstreamInstancePtr,
    XV_HDMITXSS_HANDLER_HDCP_UNAUTHENTICATED,
	(void *)XHdcp_DownstreamUnauthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  Status = XV_HdmiTxSs_SetCallback(DownstreamInstancePtr,
    XV_HDMITXSS_HANDLER_HDCP_AUTHENTICATED,
	(void *)XHdcp_DownstreamAuthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  /* Increment downstream interface count */
  (InstancePtr->DownstreamInstanceBinded)++;

  /* TX Only */
  InstancePtr->IsReady = (TRUE);

  return (XST_SUCCESS);
}
#endif

/*****************************************************************************/
/**
*
* This function is responsible for executing the state machine for the
* upstream interface and each connected downstream interface. The
* state machines are executed using round robin scheduling. Interface
* poll functions are non-blocking, so starvation should not occur, but
* fairness is not guaranteed.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_Poll(XHdcp_Repeater *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /*
    The stream-up even pushes an authenticaiton request, but
    some sinks are not immediately HDCP capable; therefore,
    we must periodically attempt to authenticate. We delay the
    authentication using a interval count to avoid stalling
    the processor with excessive I2C transactions.
  */
  static int IntervalCounter = 0;
#endif

  if (InstancePtr->IsReady) {

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
    /* Call each downstream interface Poll function */
    for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
      XV_HdmiTxSs_HdcpPoll(InstancePtr->DownstreamInstancePtr[i]);
      }

    /* Trigger authentication */
    if (IntervalCounter == 0) {
      XHdcp_Authenticate(InstancePtr);
      XHdcp_EnforceBlank(InstancePtr);
#if defined (ARMR5) || (__aarch64__) || (__arm__)
      IntervalCounter = 1000000;
#else
      IntervalCounter = 100000;
#endif
    } else  {
      IntervalCounter--;
    }
#endif

  }
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called to trigger authentication for each downstream
* interface.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_Authenticate(XHdcp_Repeater *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if (InstancePtr->IsReady) {

    /* Downstream interface stream up */
    for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {

      if (InstancePtr->DownstreamInstanceStreamUp & (0x1 << i)) {

        /* Trigger authentication on Idle */
        if (!(XV_HdmiTxSs_HdcpIsAuthenticated(InstancePtr->DownstreamInstancePtr[i])) &&
            !(XV_HdmiTxSs_HdcpIsInProgress(InstancePtr->DownstreamInstancePtr[i]))) {

          XV_HdmiTxSs_HdcpPushEvent(InstancePtr->DownstreamInstancePtr[i], XV_HDMITXSS_HDCP_AUTHENTICATE_EVT);
        }

        /* Trigger authentication on Toggle */
        else if(XV_HdmiTxSs_IsStreamToggled(InstancePtr->DownstreamInstancePtr[i])) {
          XV_HdmiTxSs_HdcpPushEvent(InstancePtr->DownstreamInstancePtr[i], XV_HDMITXSS_HDCP_AUTHENTICATE_EVT);
        }
      }
    }
  }
}
#endif

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called to set the HDCP capablility on the
* downstream interfaces. After the capability is set authentication
* is initiated.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Protocol is defined by XV_HdmiTxSs_HdcpProtocol type with the
*           following valid options:
*           - XV_HDMITXSS_HDCP_NONE sets protocol to none
*             and disables downstream content blocking. This option
*             should be used for debug purposes only.
*           - XV_HDMITXSS_HDCP_14 set protocol to 1.4
*           - XV_HDMITXSS_HDCP_22 set protocol to 2.2
*           - XV_HDMITXSS_HDCP_BOTH set protocol to both
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdcp_SetDownstreamCapability(XHdcp_Repeater *InstancePtr, int Protocol)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if (InstancePtr->IsReady) {

    /* Set HDCP protocol on downstream interfaces */
    for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {

      /* Set desired downstream capability */
      XV_HdmiTxSs_HdcpSetCapability(InstancePtr->DownstreamInstancePtr[i],
        Protocol);

      /* Push authentication request */
      XV_HdmiTxSs_HdcpPushEvent(InstancePtr->DownstreamInstancePtr[i],
        XV_HDMITXSS_HDCP_AUTHENTICATE_EVT);
    }
  }
}
#endif


/*****************************************************************************/
/**
*
* This function displays the repeater layer instance information.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Verbose can be set to TRUE to display device list.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_DisplayInfo(XHdcp_Repeater *InstancePtr, u8 Verbose)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("HDCP Repeater Info\r\n");
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  xil_printf("Downstream Binded: %d\r\n", InstancePtr->DownstreamInstanceBinded);
  xil_printf("Downstream Connected: 0x%08x\r\n", InstancePtr->DownstreamInstanceConnected);
  xil_printf("Downstream Stream-Up: 0x%08x\r\n", InstancePtr->DownstreamInstanceStreamUp);
#endif
}

#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function enables/disables encryption for each authenticated downstream
* interface.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Set is TRUE to enable or FALSE to disable repeater.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_EnableEncryption(XHdcp_Repeater *InstancePtr, u8 Set)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if (InstancePtr->IsReady) {
    for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
      if (Set)
      XV_HdmiTxSs_HdcpEnableEncryption(InstancePtr->DownstreamInstancePtr[i]);
      else
      XV_HdmiTxSs_HdcpDisableEncryption(InstancePtr->DownstreamInstancePtr[i]);
    }
  }
}
#endif


/*****************************************************************************/
/**
*
* This function is called by the stream up event for an
* interface. The function initiates authentication with each
* connected downstream interface that is not in the authenticated
* state. This function also sets the default content stream management
* type to zero when the upstream interface is HDCP 1.4.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_StreamUpCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Downstream interface stream up */
  for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
	  if (XV_HdmiTxSs_IsStreamUp(InstancePtr->DownstreamInstancePtr[i])) {

      /* Trigger authentication */
      if (!(InstancePtr->DownstreamInstanceStreamUp & (0x1 << i))) {
        XV_HdmiTxSs_HdcpPushEvent(InstancePtr->DownstreamInstancePtr[i],
			XV_HDMITXSS_HDCP_AUTHENTICATE_EVT);
      }

      InstancePtr->DownstreamInstanceStreamUp |= (0x1 << i);
    }
  }

  /* Enforce blanking */
  XHdcp_EnforceBlank(InstancePtr);
#endif
}

/*****************************************************************************/
/**
*
* This function is called by the stream down event for an interface.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_StreamDownCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Downstream interface stream down */
  for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
    if (!XV_HdmiTxSs_IsStreamUp(InstancePtr->DownstreamInstancePtr[i])) {
      InstancePtr->DownstreamInstanceStreamUp &= ~(0x1 << i);
    }
  }

  /* Enforce blanking */
  XHdcp_EnforceBlank(InstancePtr);
#endif
}

/*****************************************************************************/
/**
*
* This function is called by the stream connect event for an interface.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_StreamConnectCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  u32 DownstreamInstanceConnected = 0;
#endif

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Check if downstream interface is connected */
  for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
    if (XV_HdmiTxSs_IsStreamConnected(InstancePtr->DownstreamInstancePtr[i])) {
      if (!(InstancePtr->DownstreamInstanceConnected & (0x1 << i))) {
        InstancePtr->DownstreamInstanceStreamUp &= ~(0x1 << i);
      }
      DownstreamInstanceConnected |= (0x1 << i);
    }
  }
#endif


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Update the connected flag */
  InstancePtr->DownstreamInstanceConnected = DownstreamInstanceConnected;
#endif
}

/*****************************************************************************/
/**
*
* This function is called by the stream disconnect event for an interface.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
void XHdcp_StreamDisconnectCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;
#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  u32 DownstreamInstanceConnected = 0;
#endif

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Check if downstream interface is disconnected */
  for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
    if (XV_HdmiTxSs_IsStreamConnected(InstancePtr->DownstreamInstancePtr[i])) {
      DownstreamInstanceConnected |= (0x1 << i);
    } else {
      InstancePtr->DownstreamInstanceStreamUp &= ~(0x1 << i);
    }
  }
#endif


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
  /* Update the connected flag */
  InstancePtr->DownstreamInstanceConnected = DownstreamInstanceConnected;
#endif
}



#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the downstream interface transitions to the
* authenticated state. Encryption will be enabled for a downstream
* interface based on the downstream interface protocol and content stream
* type setting.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_DownstreamAuthenticatedCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;
  int HdcpProtocol;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  /* After authentication start encryption */
  for (int i = 0; (i < InstancePtr->DownstreamInstanceBinded); i++) {

    if (XV_HdmiTxSs_HdcpIsAuthenticated(InstancePtr->DownstreamInstancePtr[i])) {

      /* Check the downstream interface protocol */
      HdcpProtocol = XV_HdmiTxSs_HdcpGetProtocol(InstancePtr->DownstreamInstancePtr[i]);

      switch (HdcpProtocol) {
        /* HDCP 2.2 */
        case XV_HDMITXSS_HDCP_22:
          xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 2.2 downstream authenticated\r\n");
          XV_HdmiTxSs_HdcpEnableEncryption(InstancePtr->DownstreamInstancePtr[i]);
          break;

        /* HDCP 1.4 */
        case XV_HDMITXSS_HDCP_14:
          xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 1.4 downstream authenticated\r\n");
          XV_HdmiTxSs_HdcpEnableEncryption(InstancePtr->DownstreamInstancePtr[i]);
          break;
      }
    }
  }

  /* Enforce blanking */
  XHdcp_EnforceBlank(InstancePtr);
}
#endif


#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the downstream interface transitions from
* an authenticated state to an unauthenticated state.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_DownstreamUnauthenticatedCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enforce blanking */
  XHdcp_EnforceBlank(InstancePtr);
}
#endif








#ifdef XPAR_XV_HDMITXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function enforces downstream content blocking based on the upstream
* encryption status and stream type information. When the content is required
* to be blocked, cipher output blanking is enabled.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   - XST_SUCCESS if blocking enforced successfully.
*           - XST_FAILURE if blocking could not be enforced.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_EnforceBlank(XHdcp_Repeater *InstancePtr)
{
  int Status;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);


  /* Enforce downstream content blocking */
  for (int i = 0; (i < InstancePtr->DownstreamInstanceBinded); i++) {
      if (XV_HdmiTxSs_HdcpIsAuthenticated(InstancePtr->DownstreamInstancePtr[i])) {
        /* Check the downstream interface protocol */
        Status = XV_HdmiTxSs_HdcpGetProtocol(InstancePtr->DownstreamInstancePtr[i]);

        switch (Status) {

          /* HDCP 2.2
             Allow content under the following conditons:
             - Stream type is 0
             - Stream type is 1, and no HDCP 1.x devices are downstream,
               and no HDCP 2.0 repeaters are downstream. */
          case XV_HDMITXSS_HDCP_22:
            Status = XV_HdmiTxSs_HdcpGetTopologyField(
                       InstancePtr->DownstreamInstancePtr[i],
                       XV_HDMITXSS_HDCP_TOPOLOGY_HDCP20REPEATERDOWNSTREAM);
            Status |= XV_HdmiTxSs_HdcpGetTopologyField(
                        InstancePtr->DownstreamInstancePtr[i],
                        XV_HDMITXSS_HDCP_TOPOLOGY_HDCP1DEVICEDOWNSTREAM);
            if ((InstancePtr->StreamType == XV_HDMITXSS_HDCP_STREAMTYPE_0) ||
                (Status == FALSE)) {
              XV_HdmiTxSs_HdcpDisableBlank(InstancePtr->DownstreamInstancePtr[i]);
            } else {
              Status = XV_HdmiTxSs_HdcpEnableBlank(InstancePtr->DownstreamInstancePtr[i]);
              if (Status != XST_SUCCESS) {
                xdbg_printf(XDBG_DEBUG_GENERAL, "Error: Problem blocking downstream content.\r\n");
              }
            }
            break;

          /* HDCP 1.4
             Allow content when the stream type is 0 */
          case XV_HDMITXSS_HDCP_14:
            if (InstancePtr->StreamType == XV_HDMITXSS_HDCP_STREAMTYPE_0) {
              XV_HdmiTxSs_HdcpDisableBlank(InstancePtr->DownstreamInstancePtr[i]);
            } else {
              Status = XV_HdmiTxSs_HdcpEnableBlank(InstancePtr->DownstreamInstancePtr[i]);
              if (Status != XST_SUCCESS) {
                xdbg_printf(XDBG_DEBUG_GENERAL, "Error: Problem blocking downstream content.\r\n");
              }
            }
            break;
        }
      } else {
        Status = XV_HdmiTxSs_HdcpEnableBlank(InstancePtr->DownstreamInstancePtr[i]);
        if (Status != XST_SUCCESS) {
          xdbg_printf(XDBG_DEBUG_GENERAL, "Error: Problem blocking downstream content.\r\n");
        }
      }
  }
}
#endif


#endif /* USE_HDCP */
