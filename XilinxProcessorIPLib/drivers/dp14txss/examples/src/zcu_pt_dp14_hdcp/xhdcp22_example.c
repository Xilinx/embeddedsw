/*******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
*******************************************************************************/

/*
 * xhdcp22_example.c
 *
 *  Created on: Dec 17, 2018
 *      Author: jbaniset
 */
#include "xhdcp22_example.h"

#ifdef XPAR_XDPRXSS_NUM_INSTANCES

XHdcp22_Repeater     Hdcp22Repeater;

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
static void XHdcp22_UpstreamAuthenticatedCallback(void *HdcpInstancePtr)
{
	HdcpInstancePtr = HdcpInstancePtr;

//	xil_printf("HDCP 2.2 upstream authenticated\r\n");

	/* Nothing to be done here for now Implement
	 * enforce blanking if TX HDCP is present*/
}

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
static void XHdcp22_UpstreamUnauthenticatedCallback(void *HdcpInstancePtr)
{
	/* Nothing to be done here for now Implement
	 * enforce blanking, topology and stream type reset
	 * if TX HDCP is present*/
}

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
static void XHdcp22_UpstreamEncryptionUpdateCallback(void *HdcpInstancePtr)
{
	/* Nothing to be done here for now Implement
	 * enforce blanking if TX HDCP is present*/
}

/*****************************************************************************/
/**
*
* This function is used to bind an DP Receiver instance as
* the repeater upstream interface. This function should be called
* once per repeater topology to set the upstream interface.
*
* @param    InstancePtr is a pointer to the XHdcp22_Repeater instance.
* @param    UpstreamInstancePtr is a pointer to the DP receiver instance.
*
* @return   - XST_SUCCESS if upstream interface registered successfully.
*           - XST_FAILURE if upstream interface could not be registered.
*
* @note	    As the current driver is for only receiver this function
* 			registers only receiver related callbacks.
*
******************************************************************************/
int XHdcp22_SetUpstream(XHdcp22_Repeater *InstancePtr,
      XDpRxSs *UpstreamInstancePtr)
{
  int Status;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(UpstreamInstancePtr != NULL);

  /* Bind upstream interface */
  InstancePtr->UpstreamInstancePtr = UpstreamInstancePtr;

  /*Register authentication done callback*/
#if XPAR_DPRXSS_0_HDCP22_ENABLE
  Status = XDpRxSs_SetCallBack(UpstreamInstancePtr,
    XDPRXSS_HANDLER_HDCP22_AUTHENTICATED,
	(void *)XHdcp22_UpstreamAuthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  /*Register Unauthenticated callback*/
  Status = XDpRxSs_SetCallBack(UpstreamInstancePtr,
    XDPRXSS_HANDLER_HDCP22_UNAUTHENTICATED,
	(void *)XHdcp22_UpstreamUnauthenticatedCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }

  /*Register Encryption update callback*/
  Status = XDpRxSs_SetCallBack(UpstreamInstancePtr,
    XDPRXSS_HANDLER_HDCP22_ENCRYPTION_UPDATE,
	(void *)XHdcp22_UpstreamEncryptionUpdateCallback,
    (void *)InstancePtr);

  if (Status != XST_SUCCESS) {
    return (XST_FAILURE);
  }
#endif

  /* Indicate upstream interface has been binded */
  InstancePtr->UpstreamInstanceBinded = (TRUE);

#if 0 // XPAR_XDPTXSS_NUM_INSTANCES
  /* Set ready when the upstream interface and at least one downstream interface is binded */
  if (InstancePtr->DownstreamInstanceBinded > 0) {
    InstancePtr->IsReady = (TRUE);
  }
#else
  /* RX Only */
  InstancePtr->IsReady = (TRUE);
#endif

  return (XST_SUCCESS);
}
#endif

void XHdcp22_Poll(XHdcp22_Repeater *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	if (InstancePtr->IsReady) {
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)

		/* Call the upstream interface Poll function */
	    XDpRxSs_Hdcp22Poll(InstancePtr->UpstreamInstancePtr);
#endif
#if  (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
		/* Call the upstream interface Poll function */
		for (int i = 0; i < InstancePtr->DownstreamInstanceBinded; i++) {
			XDpTxSs_HdcpPoll(InstancePtr->DownstreamInstancePtr[i]);
		}
#endif
	}
}

extern u8 XHdcp22Lc128[];
extern u8 XHdcp22RxPrivateKey[];
int XHdcp22_LoadKeys_rx(uint8_t *Hdcp22Lc128, uint32_t Hdcp22Lc128Size,
		uint8_t *Hdcp22RxPrivateKey, uint32_t Hdcp22RxPrivateKeySize)
{
	/* This is a Dummy function, This should load the keys from its
	 * stored location and return those.
	 * For HDMI there is an EEPROM in which all the encrypted keys have stored,
	 * For DP this is from the keys.c file*/

	/* HDCP 2.2 LC128 */
	memcpy(Hdcp22Lc128, XHdcp22Lc128, Hdcp22Lc128Size);
	/* Certificate */
	memcpy(Hdcp22RxPrivateKey, XHdcp22RxPrivateKey, Hdcp22RxPrivateKeySize);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets pointers to the HDCP 2.2 keys.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_DpRxSs_Hdcp22SetKey(XDpRxSs *InstancePtr, XV_DpRxSs_Hdcp22KeyType KeyType, u8 *KeyPtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((KeyType == XV_DPRXSS_KEY_HDCP22_LC128)   ||
                   (KeyType == XV_DPRXSS_KEY_HDCP22_PRIVATE))
  switch (KeyType) {
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
    // HDCP 2.2 LC128
    case XV_DPRXSS_KEY_HDCP22_LC128 :
      InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
      break;

    // HDCP 2.2 Private key
    case XV_DPRXSS_KEY_HDCP22_PRIVATE :
      InstancePtr->Hdcp22PrivateKeyPtr = KeyPtr;
      break;
#endif
    default :
      break;
  }
}

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
int XHdcp_Initialize(XHdcp22_Repeater *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Set default values */
#if 0
#ifdef XPAR_XDPRXSS_NUM_INSTANCES
	InstancePtr->UpstreamInstanceBinded = 0;
	InstancePtr->UpstreamInstanceConnected = 0;
	InstancePtr->UpstreamInstanceStreamUp = 0;
#endif
#endif /*#if 0*/
#ifdef XPAR_XDPTXSS_NUM_INSTANCES
	InstancePtr->DownstreamInstanceBinded = 0;
	InstancePtr->DownstreamInstanceConnected = 0;
	InstancePtr->DownstreamInstanceStreamUp = 0;
#endif
#ifdef XPAR_XDPTXSS_NUM_INSTANCES
	InstancePtr->EnforceBlocking = (TRUE);
	InstancePtr->StreamType = XDPTXSS_HDCP_STREAMTYPE_0;
#endif
#if defined (XPAR_XDPTXSS_NUM_INSTANCES) && defined (XPAR_XDPRXSS_NUM_INSTANCES)
	memset(&InstancePtr->Topology, 0, sizeof(XHdcp_Topology));
#endif

	/* Instance is ready only after upstream and at least one downstream
	 has been binded */
	InstancePtr->IsReady = (FALSE);

	return (XST_SUCCESS);
}


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
static void XHdcp_EnforceBlank(XHdcp22_Repeater *InstancePtr)
{
	InstancePtr = InstancePtr;
	/*This should be implemented when both RX and Tx drivers integrated into
	 * same driver.
	 * For Tx only driver, nothing to be done here.*/
}

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
static void XHdcp22_DownstreamUnauthenticatedCallback(void *HdcpInstancePtr)
{
  XHdcp22_Repeater *InstancePtr =  (XHdcp22_Repeater *)HdcpInstancePtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enforce blanking */
  XHdcp_EnforceBlank(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function is called when the downstream interface transitions to the
* authenticated state. Encryption will be enabled for a downstream
* interface based on the downstream interface protocol and content stream
* type setting.
*
* @param    HdcpInstancePtr is a pointer to the XHdcp22_Repeater instance.
*
* @return   None.
*
* @note	    None.
*
******************************************************************************/
static void XHdcp22_DownstreamAuthenticatedCallback(void *HdcpInstancePtr)
{
	XHdcp22_Repeater *InstancePtr =  (XHdcp22_Repeater *)HdcpInstancePtr;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
//	xil_printf ("Authenticated\r\n");

	extern XDpTxSs DpTxSsInst;
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_SOFT_RESET, 0x100);
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_SOFT_RESET, 0x0);

	/* After authentication start encryption */
	for (int i = 0; (i < InstancePtr->DownstreamInstanceBinded); i++) {
//		xil_printf("HDCP 2.2 downstream authenticated\r\n");
		XDpTxSs_EnableEncryption((void *)InstancePtr->DownstreamInstancePtr[i], 1);
	}
//
//	/* Enforce blanking */
//	XHdcp_EnforceBlank(InstancePtr);
}


/*****************************************************************************/
/**
*
* This function sets pointers to the HDCP 2.2 keys.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_DpTxSs_Hdcp22SetKey(XDpTxSs *InstancePtr, XV_DpTxSs_Hdcp22KeyType KeyType, u8 *KeyPtr)
{
  /* Verify argument. */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((KeyType == XV_DPTXSS_KEY_HDCP22_LC128)   ||
                   (KeyType == XV_DPTXSS_KEY_HDCP22_SRM))
  switch (KeyType) {
#if (XPAR_XHDCP22_TX_NUM_INSTANCES > 0)
    // HDCP 2.2 LC128
    case XV_DPTXSS_KEY_HDCP22_LC128 :
      InstancePtr->Hdcp22Lc128Ptr = KeyPtr;
      break;

    // HDCP 2.2 Private key
    case XV_DPTXSS_KEY_HDCP22_SRM:
      InstancePtr->Hdcp22SrmPtr = KeyPtr;
      break;
#endif
    default :
      break;
  }
}

int XHdcp22_LoadKeys_tx(u8 *Hdcp22Lc128, u32 Hdcp22Lc128Size)
{
	/* This is a Dummy function, This should load the keys from its
	 * stored location and return those.
	 * For HDMI there is an EEPROM in which all the encrypted keys have stored,
	 * For DP this is from the keys.c file*/

	/* HDCP 2.2 LC128 */
	memcpy(Hdcp22Lc128, XHdcp22Lc128, Hdcp22Lc128Size);

	return XST_SUCCESS;
}

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
int XHdcp_SetDownstream(XHdcp22_Repeater *InstancePtr,
      XDpTxSs *DownstreamInstancePtr)
{
	int Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DownstreamInstancePtr != NULL);

	/* Bind downstream interface */
	if (InstancePtr->DownstreamInstanceBinded <=
			XHDCP_MAX_DOWNSTREAM_INTERFACES) {
		InstancePtr->DownstreamInstancePtr[InstancePtr->
										   DownstreamInstanceBinded] =
												   DownstreamInstancePtr;
	} else {
		return (XST_FAILURE);
	}
#if XPAR_DPTXSS_0_HDCP22_ENABLE
	Status = XDpTxSs_SetCallBack(DownstreamInstancePtr,
			XDPTXSS_HANDLER_HDCP22_UNAUTHENTICATED,
			(void *)XHdcp22_DownstreamUnauthenticatedCallback,
			(void *)InstancePtr);

	if (Status != XST_SUCCESS) {
		return (XST_FAILURE);
	}

	Status = XDpTxSs_SetCallBack(DownstreamInstancePtr,
			XDPTXSS_HANDLER_HDCP22_AUTHENTICATED,
			(void *)XHdcp22_DownstreamAuthenticatedCallback,
			(void *)InstancePtr);

	if (Status != XST_SUCCESS) {
		return (XST_FAILURE);
	}
#endif
  /* Increment downstream interface count */
  InstancePtr->DownstreamInstanceBinded++;

#if 0 /*def XPAR_XDPRXSS_NUM_INSTANCES*/
  /* Set ready when the upstream interface and
     at least one downstream interface is binded */
  if ((InstancePtr->UpstreamInstanceBinded == TRUE) &&
      (InstancePtr->DownstreamInstanceBinded > 0)) {
    InstancePtr->IsReady = (TRUE);
  }
#else
  /* TX Only */
  InstancePtr->IsReady = (TRUE);
#endif

  return (XST_SUCCESS);
}
