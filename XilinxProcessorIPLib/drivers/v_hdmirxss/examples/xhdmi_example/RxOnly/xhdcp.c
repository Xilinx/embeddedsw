/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
static void XHdcp_UpstreamAuthenticatedCallback(void *HdcpInstancePtr);
static void XHdcp_UpstreamUnauthenticatedCallback(void *HdcpInstancePtr);
static void XHdcp_UpstreamEncryptionUpdateCallback(void *HdcpInstancePtr);
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  InstancePtr->UpstreamInstanceBinded = 0;
  InstancePtr->UpstreamInstanceConnected = 0;
  InstancePtr->UpstreamInstanceStreamUp = 0;
#endif

  /* Instance is ready only after upstream and at least one downstream
     has been binded */
  InstancePtr->IsReady = (FALSE);

  return (XST_SUCCESS);
}

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is used to bind an HDMI Receiver instance as
* the repeater upstream interface. This function should be called
* once per repeater topology to set the upstream interface.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    UpstreamInstancePtr is a pointer to the HDMI receiver instance.
*
* @return   - XST_SUCCESS if upstream interface registered successfully.
*           - XST_FAILURE if upstream interface could not be registered.
*
* @note	    None.
*
******************************************************************************/
int XHdcp_SetUpstream(XHdcp_Repeater *InstancePtr,
      XV_HdmiRxSs *UpstreamInstancePtr)
{
  int Status;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(UpstreamInstancePtr != NULL);

  /* Bind upstream interface */
  InstancePtr->UpstreamInstancePtr = UpstreamInstancePtr;


  Status = XV_HdmiRxSs_SetCallback(UpstreamInstancePtr,
    XV_HDMIRXSS_HANDLER_HDCP_AUTHENTICATED,
	(void *)XHdcp_UpstreamAuthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  Status = XV_HdmiRxSs_SetCallback(UpstreamInstancePtr,
    XV_HDMIRXSS_HANDLER_HDCP_UNAUTHENTICATED,
	(void *)XHdcp_UpstreamUnauthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  Status = XV_HdmiRxSs_SetCallback(UpstreamInstancePtr,
    XV_HDMIRXSS_HANDLER_HDCP_ENCRYPTION_UPDATE,
	(void *)XHdcp_UpstreamEncryptionUpdateCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  /* Indicate upstream interface has been binded */
  InstancePtr->UpstreamInstanceBinded = (TRUE);

  /* RX Only */
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


  if (InstancePtr->IsReady) {
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
    /* Call the upstream interface Poll function */
    XV_HdmiRxSs_HdcpPoll(InstancePtr->UpstreamInstancePtr);
#endif


  }
}



#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called to set the HDCP capability on the upstream
* interface. HPD is toggled to get the attention of the transmitter.
*
* @param    InstancePtr is a pointer to the XHdcp_Repeater instance.
* @param    Protocol is defined by XV_HdmiRxSs_HdcpProtocol type with the
*           following valid options:
*           - XV_HDMIRXSS_HDCP_NONE sets protocol to none
*           - XV_HDMIRXSS_HDCP_BOTH set protocol to both
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XHdcp_SetUpstreamCapability(XHdcp_Repeater *InstancePtr, int Protocol)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if (InstancePtr->IsReady) {

    /* Set desired upstream capability */
    XV_HdmiRxSs_HdcpSetCapability(InstancePtr->UpstreamInstancePtr, Protocol);

    /* Toggle HPD to get attention of upstream transmitter */
    XV_HdmiRxSs_ToggleHpd(InstancePtr->UpstreamInstancePtr);
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
#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  xil_printf("Upstream Binded: %d\r\n", InstancePtr->UpstreamInstanceBinded);
  xil_printf("Upstream Connected: %d\r\n", InstancePtr->UpstreamInstanceConnected);
  xil_printf("Upstream Stream-Up: %d\r\n", InstancePtr->UpstreamInstanceStreamUp);
#endif
}



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

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  /* Upstream interface stream up */
  if (XV_HdmiRxSs_IsStreamUp(InstancePtr->UpstreamInstancePtr)) {


    InstancePtr->UpstreamInstanceStreamUp = TRUE;
  }
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

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
  /* Upstream interface stream down */
  if (!XV_HdmiRxSs_IsStreamUp(InstancePtr->UpstreamInstancePtr)) {
    InstancePtr->UpstreamInstanceStreamUp = FALSE;
  }
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

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES

  /* Check if upstream interface is connected */
  if (XV_HdmiRxSs_IsStreamConnected(InstancePtr->UpstreamInstancePtr)) {
    if (!(InstancePtr->UpstreamInstanceConnected)) {
      InstancePtr->UpstreamInstanceStreamUp = FALSE;
    }
    InstancePtr->UpstreamInstanceConnected = TRUE;
  }
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

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES

  /* Check if upstream interface is disconnected */
  if (!(XV_HdmiRxSs_IsStreamConnected(InstancePtr->UpstreamInstancePtr))) {
    InstancePtr->UpstreamInstanceStreamUp = FALSE;
    InstancePtr->UpstreamInstanceConnected = FALSE;
  }
#endif



}


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the upstream interface transitions to the
* authenticated state. Low value content output is set for downstream
* interfaces that are not in the authenticated state.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_UpstreamAuthenticatedCallback(void *HdcpInstancePtr)
{
  XHdcp_Repeater *InstancePtr =  (XHdcp_Repeater *)HdcpInstancePtr;
  int HdcpProtocol;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  /* xil_printf message */
  HdcpProtocol = XV_HdmiRxSs_HdcpGetProtocol(InstancePtr->UpstreamInstancePtr);
  switch (HdcpProtocol) {
    case XV_HDMIRXSS_HDCP_22:
      xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 2.2 upstream authenticated\r\n");
      break;
    case XV_HDMIRXSS_HDCP_14:
      xdbg_printf(XDBG_DEBUG_GENERAL, "HDCP 1.4 upstream authenticated\r\n");
      break;
    default : ;
  }

}
#endif


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the upstream interface transitions from
* an authenticated state to an unauthenticated state.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_UpstreamUnauthenticatedCallback(void *HdcpInstancePtr)
{
}
#endif


#ifdef XPAR_XV_HDMIRXSS_NUM_INSTANCES
/*****************************************************************************/
/**
*
* This function is called when the upstream interface encryption status
* has changed.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp_UpstreamEncryptionUpdateCallback(void *HdcpInstancePtr)
{
}
#endif









#endif /* USE_HDCP */
