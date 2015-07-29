/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_port.c
*
* This contains the main implementation file for the Xilinx HDCP Port driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xil_assert.h"
#include "xil_types.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

#if defined(XPAR_XHDMI_TX_NUM_INSTANCES) && (XPAR_XHDMI_TX_NUM_INSTANCES > 0)
#define INCLUDE_HDMI_TX
#endif
#if defined(XPAR_XHDMI_RX_NUM_INSTANCES) && (XPAR_XHDMI_RX_NUM_INSTANCES > 0)
#define INCLUDE_HDMI_RX
#endif
#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_NUM_INSTANCES > 0)
#define INCLUDE_DP_TX
#define INCLUDE_DP_RX
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Extern Declarations ******************************/

#if defined(INCLUDE_HDMI_TX)
extern const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortHdmiTxAdaptor;
#endif
#if defined(INCLUDE_HDMI_RX)
extern const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortHdmiRxAdaptor;
#endif
#if defined(INCLUDE_DP_TX)
extern const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpTxAdaptor;
#endif
#if defined(INCLUDE_DP_RX)
extern const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpRxAdaptor;
#endif

/*************************** Function Prototypes *****************************/

static const XHdcp1x_PortPhyIfAdaptor *DetermineAdaptor(
		const XHdcp1x_Port *InstancePtr);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes a port device.
*
* @param	InstancePtr is the device to initialize.
* @param	CfgPtr is the HDCP configuration structure.
* @param	PhyIfPtr is pointer to the underlying physical interface.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_NO_FEATURE if the port lacks an Init function.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortCfgInitialize(XHdcp1x_Port *InstancePtr,
                const XHdcp1x_Config *CfgPtr, void *PhyIfPtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(PhyIfPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady != XIL_COMPONENT_IS_READY);

	/* Initialize InstancePtr */
	memset(InstancePtr, 0, sizeof(XHdcp1x_Port));
	InstancePtr->CfgPtr = CfgPtr;
	InstancePtr->PhyIfPtr = PhyIfPtr;
	InstancePtr->Adaptor = DetermineAdaptor(InstancePtr);

	/* Sanity Check */
	if (InstancePtr->Adaptor == NULL) {
		Status = XST_NO_FEATURE;
	}
	/* Invoke adaptor function if present */
	else if (InstancePtr->Adaptor->Init != NULL) {
		Status = (*(InstancePtr->Adaptor->Init))(InstancePtr);
	}

	/* Set IsReady */
	if (Status == XST_SUCCESS) {
		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function enables a port device.
*
* @param	InstancePtr is the device to enables.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_NO_FEATURE if the port lacks an Enable function.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortEnable(XHdcp1x_Port *InstancePtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Sanity Check */
	if (Adaptor == NULL) {
		Status = XST_NO_FEATURE;
	}
	/* Invoke adaptor function if present */
	else if (Adaptor->Enable != NULL) {
		Status = (*(Adaptor->Enable))(InstancePtr);
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function disables a port device.
*
* @param	InstancePtr is the device to disables.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_NO_FEATURE if the port lacks a Disable function.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortDisable(XHdcp1x_Port *InstancePtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Sanity Check */
	if (Adaptor == NULL) {
		Status = XST_NO_FEATURE;
	}
	/* Invoke adaptor function if present */
	else if (Adaptor->Disable != NULL) {
		Status = (*(Adaptor->Disable))(InstancePtr);
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function queries a port device to determine if hdcp is supported.
*
* @param	InstancePtr is the device to query.
*
* @return	Truth value.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortIsCapable(const XHdcp1x_Port *InstancePtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int IsCapable = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Invoke adaptor function if present */
	if ((Adaptor != NULL) && (Adaptor->IsCapable != NULL)) {
		IsCapable = (*(Adaptor->IsCapable))(InstancePtr);
	}

	return (IsCapable);
}

/*****************************************************************************/
/**
* This function queries a port device to determine if it is connected to a
* repeater.
*
* @param	InstancePtr is the device to query.
*
* @return	Truth value.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortIsRepeater(const XHdcp1x_Port *InstancePtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int IsRepeater = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Invoke adaptor function if present */
	if ((Adaptor != NULL) && (Adaptor->IsRepeater != NULL)) {
		IsRepeater = (*(Adaptor->IsRepeater))(InstancePtr);
	}

	return (IsRepeater);
}

/*****************************************************************************/
/**
* This function retrieves the repeater information from the connected device.
*
* @param	InstancePtr is the device to query.
* @param	InfoPtr is the repeater info.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_NO_FEATURE if the port lacks a GetRepeaterInfo function.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortGetRepeaterInfo(XHdcp1x_Port *InstancePtr, u16 *InfoPtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InfoPtr != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Sanity Check */
	if (Adaptor == NULL) {
		Status = XST_NO_FEATURE;
	}
	/* Invoke adaptor function if present */
	else if (Adaptor->GetRepeaterInfo != NULL) {
		Status = (*(Adaptor->GetRepeaterInfo))(InstancePtr, InfoPtr);
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function reads a register from a HDCP port device.
*
* @param	InstancePtr is the device to read from.
* @param	Offset is the offset to start reading from.
* @param	Buf is the buffer to copy the data read.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes read.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortRead(const XHdcp1x_Port *InstancePtr, u8 Offset, void *Buf,
                u32 BufSize)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int NumRead = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Invoke adaptor function if present */
	if ((Adaptor != NULL) && (Adaptor->Read != NULL)) {
		NumRead = (*(Adaptor->Read))(InstancePtr, Offset, Buf,
				BufSize);
	}

	return (NumRead);
}

/*****************************************************************************/
/**
* This function writes a register within a HDCP port device.
*
* @param	InstancePtr is the device to write to.
* @param	Offset is the offset to start writing at.
* @param	Buf is the buffer containing the data to write.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes written.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortWrite(XHdcp1x_Port *InstancePtr, u8 Offset, const void *Buf,
                u32 BufSize)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	int NumWritten = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine Adaptor */
	Adaptor = InstancePtr->Adaptor;

	/* Invoke adaptor function if present */
	if ((Adaptor != NULL) && (Adaptor->Write != NULL)) {
		NumWritten = (*(Adaptor->Write))(InstancePtr, Offset, Buf,
		                BufSize);
	}

	return (NumWritten);
}

/*****************************************************************************/
/**
*
* This function determines the adaptor for a specified port device.
*
* @param	InstancePtr is the device whose adaptor is to be determined.
*
* @return	A pointer to the adaptor table. NULL if not found.
*
* @note		None.
*
******************************************************************************/
static const XHdcp1x_PortPhyIfAdaptor *DetermineAdaptor(
		const XHdcp1x_Port *InstancePtr)
{
	const XHdcp1x_PortPhyIfAdaptor *Adaptor = NULL;
	const XHdcp1x_Config *CfgPtr = InstancePtr->CfgPtr;

#if defined(INCLUDE_HDMI_RX)
	/* Check for HDMI Rx */
	if ((CfgPtr->IsRx) && (CfgPtr->IsHDMI)) {
		Adaptor = &XHdcp1x_PortHdmiRxAdaptor;
	}
	else
#endif
#if defined(INCLUDE_HDMI_TX)
	/* Check for HDMI Tx */
	if (!(CfgPtr->IsRx) && (CfgPtr->IsHDMI)) {
		Adaptor = &XHdcp1x_PortHdmiTxAdaptor;
	}
	else
#endif
#if defined(INCLUDE_DP_RX)
	/* Check for DP Rx */
	if ((CfgPtr->IsRx) && !(CfgPtr->IsHDMI)) {
		Adaptor = &XHdcp1x_PortDpRxAdaptor;
	}
	else
#endif
#if defined(INCLUDE_DP_TX)
	/* Check for DP Tx */
	if (!(CfgPtr->IsRx) && !(CfgPtr->IsHDMI)) {
		Adaptor = &XHdcp1x_PortDpTxAdaptor;
	}
	else
#endif
	{
		Adaptor = NULL;
	}

	return (Adaptor);
}
