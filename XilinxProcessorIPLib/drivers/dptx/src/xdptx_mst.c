/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_mst.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a als  08/03/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "string.h"
#include "xdptx.h"
#include "xstatus.h"

/**************************** Constant Definitions ****************************/

/* Error out if waiting for a sideband message reply or waiting for the payload
 * ID table to be updated takes more than 100 AUX read iterations. */
#define XDPTX_MAX_TIMEOUT_COUNT 100

/****************************** Type Definitions ******************************/

/**
 * This typedef stores the sideband message header.
 */
typedef struct
{
	u8 LinkCountTotal;		/**< The total number of DisplayPort
						links connecting the device
						device that this sideband
						message is targeted from the
						DisplayPort TX. */
	u8 LinkCountRemaining;		/**< The remaining link count until
						the sideband message reaches
						the target device. */
	u8 RelativeAddress[15];		/**< The relative address from the
						DisplayPort TX to the target
						device. */
	u8 BroadcastMsg;		/**< Specifies that this message is
						a broadcast message, to be
						handled by all downstream
						devices. */
	u8 PathMsg;			/**< Specifies that this message is
						a path message, to be handled by
						all the devices between the
						origin and the target device. */
	u8 MsgBodyLength;		/**< The total number of data bytes that
						are stored in the sideband
						message body. */
	u8 StartOfMsgTransaction;	/**< This message is the first sideband
						message in the transaction. */
	u8 EndOfMsgTransaction;		/**< This message is the last sideband
						message in the transaction. */
	u8 MsgSequenceNum;		/**< Identifies invidiual message
						transactions to a given
						DisplayPort device. */
	u8 Crc;				/**< The cyclic-redundancy check (CRC)
						value of the header data. */

	u8 MsgHeaderLength;		/**< The number of data bytes stored as
						part of the sideband message
						header. */
} XDptx_SidebandMsgHeader;

/**
 * This typedef stores the sideband message body.
 */
typedef struct
{
	u8 MsgData[62];			/**< The raw body data of the sideband
						message. */
	u8 MsgDataLength;		/**< The number of data bytes stored as
						part of the sideband message
						body. */
	u8 Crc;				/**< The cyclic-redundancy check (CRC)
						value of the body data. */
} XDptx_SidebandMsgBody;

/**
 * This typedef stores the entire sideband message.
 */
typedef struct
{
	XDptx_SidebandMsgHeader Header;	/**< The header segment of the sideband
						message. */
	XDptx_SidebandMsgBody Body;	/**< The body segment of the sideband
						message. */
} XDptx_SidebandMsg;

/**
 * This typedef describes a sideband message reply.
 */
typedef struct
{
	u8 Length;			/**< The number of bytes of reply
						data. */
	u8 Data[256];			/**< The raw reply data. */
} XDptx_SidebandReply;

/**************************** Function Prototypes *****************************/

static void XDptx_IssueGuid(XDptx *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, XDptx_Topology *Topology,
			u32 *Guid);
static void XDptx_AddBranchToList(XDptx *InstancePtr,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDptx_AddSinkToList(XDptx *InstancePtr,
			XDptx_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDptx_GetDeviceInfoFromSbMsgLinkAddress(
			XDptx_SidebandReply *SbReply,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *FormatReply);
static u32 XDptx_GetFirstAvailableTs(XDptx *InstancePtr, u8 *FirstTs);
static u32 XDptx_SendActTrigger(XDptx *InstancePtr);
static u32 XDptx_SendSbMsg(XDptx *InstancePtr, XDptx_SidebandMsg *Msg);
static u32 XDptx_ReceiveSbMsg(XDptx *InstancePtr, XDptx_SidebandReply *SbReply);
static u32 XDptx_WaitSbReply(XDptx *InstancePtr);
static u32 XDptx_Transaction2MsgFormat(u8 *Transaction, XDptx_SidebandMsg *Msg);
static u8 XDptx_Crc4CalculateHeader(XDptx_SidebandMsgHeader *Header);
static u8 XDptx_Crc8CalculateBody(XDptx_SidebandMsgBody *Body);
static u8 XDptx_CrcCalculate(const u8 *Data, u32 NumberOfBits, u8 Polynomial);

/**************************** Variable Definitions ****************************/

/**
 * This table contains a list of global unique identifiers (GUIDs) that will be
 * issued when exploring the topology using the algorithm in the
 * XDptx_FindAccessibleDpDevices function.
 */
u32 GuidTable[16][4] = {
	{0x12341234, 0x43214321, 0x56785678, 0x87658765},
	{0xDEADBEEF, 0xBEEFDEAD, 0x10011001, 0xDADADADA},
	{0xDABADABA, 0x10011001, 0xBADABADA, 0x5AD5AD5A},
	{0x12345678, 0x43214321, 0xABCDEF98, 0x87658765},
	{0x12141214, 0x41214121, 0x56785678, 0x87658765},
	{0xD1CDB11F, 0xB11FD1CD, 0xFEBCDA90, 0xDCDCDCDC},
	{0xDCBCDCBC, 0xE000E000, 0xBCDCBCDC, 0x5CD5CD5C},
	{0x11111111, 0x11111111, 0x11111111, 0x11111111},
	{0x22222222, 0x22222222, 0x22222222, 0x22222222},
	{0x33333333, 0x33333333, 0x33333333, 0x33333333},
	{0xAAAAAAAA, 0xFFFFFFFF, 0xFEBCDA90, 0xDCDCDCDC},
	{0xBBBBBBBB, 0xE000E000, 0xFFFFFFFF, 0x5CD5CD5C},
	{0xCCCCCCCC, 0x11111111, 0x11111111, 0xFFFFFFFF},
	{0xDDDDDDDD, 0x22222222, 0xFFFFFFFF, 0x22222222},
	{0xEEEEEEEE, 0xFFFFFFFF, 0x33333333, 0x33333333},
	{0x12145678, 0x41214121, 0xCBCD1F98, 0x87658765}
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function will enable multi-stream transport (MST) mode for the driver.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_MstCfgModeEnable(XDptx *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->MstEnable = 1;
}

/******************************************************************************/
/**
 * This function will disable multi-stream transport (MST) mode for the driver.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return	None.
 *
 * @note	When disabled, the driver will behave in single-stream transport
 *		(SST) mode.
 *
*******************************************************************************/
void XDptx_MstCfgModeDisable(XDptx *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->MstEnable = 0;
}

/******************************************************************************/
/**
 * This function will check if the immediate downstream RX device is capable of
 * multi-stream transport (MST) mode. A DisplayPort Configuration Data (DPCD)
 * version of 1.2 or higher is required and the MST capability bit in the DPCD
 * must be set for this function to return XST_SUCCESS.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the RX device is MST capable.
 *		- XST_NO_FEATURE if the RX device does not support MST.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if an AUX read request timed out.
 *		- XST_FAILURE otherwise - if an AUX read transaction failed.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_MstCapable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check that the RX device has a DisplayPort Configuration Data (DPCD)
	 * version greater than or equal to 1.2 to be able to support MST
	 * functionality. */
	Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_REV, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}
	else if (AuxData < 0x12) {
		return XST_NO_FEATURE;
	}

	/* Check if the RX device has MST capabilities.. */
	Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_MSTM_CAP, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}
	else if ((AuxData & XDPTX_DPCD_MST_CAP_MASK) !=
						XDPTX_DPCD_MST_CAP_MASK) {
		return XST_NO_FEATURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will enable multi-stream transport (MST) mode in both the
 * DisplayPort TX and the immediate downstream RX device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if MST mode has been successful enabled in
 *		  hardware.
 *		- XST_NO_FEATURE if the immediate downstream RX device does not
 *		  support MST - that is, if its DisplayPort Configuration Data
 *		  (DPCD) version is less than 1.2, or if the DPCD indicates that
 *		  it has no DPCD capabilities.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if an AUX request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_MstEnable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check if the immediate downstream RX device has MST capabilities. */
	Status = XDptx_MstCapable(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* The RX device is not downstream capable. */
		return Status;
	}

	/* HPD long pulse used for upstream notification. */
	AuxData = 0;
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_BRANCH_DEVICE_CTRL, 1,
								&AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Enable MST in the immediate branch device and tell it that its
	 * upstream device is a source (the DisplayPort TX). */
	AuxData = XDPTX_DPCD_UP_IS_SRC_MASK | XDPTX_DPCD_UP_REQ_EN_MASK |
							XDPTX_DPCD_MST_EN_MASK;
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_MSTM_CTRL, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Enable MST in the DisplayPort TX. */
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_TX_MST_CONFIG,
					XDPTX_TX_MST_CONFIG_MST_EN_MASK);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will disable multi-stream transport (MST) mode in both the
 * DisplayPort TX and the immediate downstream RX device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if MST mode has been successful disabled in
 *		  hardware.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX write request timed out.
 *		- XST_FAILURE otherwise - if the AUX write transaction failed.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_MstDisable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Disable MST mode in the immediate branch device. */
	AuxData = 0;
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_MSTM_CTRL, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Disable MST mode in the DisplayPort TX. */
	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_TX_MST_CONFIG, 0x0);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will check whether
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream ID to check for enable/disable status.
 *
 * @return
 *		- 1 if the specified stream is enabled.
 *		- 0 if the specified stream is disabled.
 *
 * @note	None.
 *
*******************************************************************************/
u8 XDptx_MstStreamIsEnabled(XDptx *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	return InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable;
}

/******************************************************************************/
/**
 * This function will configure the InstancePtr->MstStreamConfig structure to
 * enable the specified stream.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream ID that will be enabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_MstCfgStreamEnable(XDptx *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable = 1;
}

/******************************************************************************/
/**
 * This function will configure the InstancePtr->MstStreamConfig structure to
 * disable the specified stream.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream ID that will be disabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_MstCfgStreamDisable(XDptx *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable = 0;
}

/******************************************************************************/
/**
 * This function will map a stream to a downstream DisplayPort TX device that is
 * associated with a sink from the InstancePtr->Topology.SinkList.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream ID that will be mapped to a DisplayPort
 *		device.
 * @param	SinkNum is the sink ID in the sink list that will be mapped to
 *		the stream.
 *
 * @return	None.
 *
 * @note	The contents of the InstancePtr->MstStreamConfig[Stream] will be
 *		modified.
 * @note	The topology will need to be determined prior to calling this
 *		function using the XDptx_FindAccessibleDpDevices.
 *
*******************************************************************************/
void XDptx_SetStreamSelectFromSinkList(XDptx *InstancePtr, u8 Stream, u8
									SinkNum)
{
	u8 Index;
	XDptx_MstStream *MstStream;
	XDptx_Topology *Topology;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));

	MstStream = &InstancePtr->MstStreamConfig[Stream - 1];
	Topology = &InstancePtr->Topology;

	MstStream->LinkCountTotal = Topology->SinkList[SinkNum]->LinkCountTotal;
	for (Index = 0; Index < MstStream->LinkCountTotal - 1; Index++) {
		MstStream->RelativeAddress[Index] =
			Topology->SinkList[SinkNum]->RelativeAddress[Index];
	}
}

/******************************************************************************/
/**
 * This function will map a stream to a downstream DisplayPort TX device
 * determined by the relative address.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Stream is the stream number that will be mapped to a DisplayPort
 *		device.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the targeted downstream device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the targeted DisplayPort device.
 *
 * @return	None.
 *
 * @note	The contents of the InstancePtr->MstStreamConfig[Stream] will be
 *		modified.
 *
*******************************************************************************/
void XDptx_SetStreamSinkRad(XDptx *InstancePtr, u8 Stream, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u8 Index;
	XDptx_MstStream *MstStream;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Stream == XDPTX_STREAM_ID1) ||
		(Stream == XDPTX_STREAM_ID2) || (Stream == XDPTX_STREAM_ID3) ||
		(Stream == XDPTX_STREAM_ID4));
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid(RelativeAddress != NULL);

	MstStream = &InstancePtr->MstStreamConfig[Stream - 1];

	MstStream->LinkCountTotal = LinkCountTotal;
	for (Index = 0; Index < MstStream->LinkCountTotal - 1; Index++) {
		MstStream->RelativeAddress[Index] = RelativeAddress[Index];
	}
}

/******************************************************************************/
/**
 * This function will explore the DisplayPort topology of downstream devices
 * connected to the DisplayPort TX. It will recursively go through each branch
 * device, obtain its information by sending a LINK_ADDRESS sideband message,
 * and add this information to the the topology's node table. For each sink
 * device connected to a branch's downstream port, this function will obtain
 * the details of the sink, add it to the topology's node table, as well as
 * add it to the topology's sink list.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the current downstream device in the
 *		recursion.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the current target DisplayPort device in the
 *		recursion.
 *
 * @return	None - the recursion will exit from nodes that have an error
 *		when replying to the LINK_ADDRESS sideband message.
 *
 * @note	The contents of the InstancePtr->Topology structure will be
 *		modified.
 *
*******************************************************************************/
void XDptx_FindAccessibleDpDevices(XDptx *InstancePtr, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u32 Status;
	u8 Index;
	u8 RadIndex;
	XDptx_SbMsgLinkAddressReplyDeviceInfo DeviceInfo;
	XDptx_SbMsgLinkAddressReplyPortDetail *PortDetails;
	XDptx_Topology *Topology;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));

	Topology = &InstancePtr->Topology;

	/* Send a LINK_ADDRESS sideband message to the branch device in order to
	 * obtain information on it and its downstream devices. */
	Status = XDptx_SendSbMsgLinkAddress(InstancePtr, LinkCountTotal,
						RelativeAddress, &DeviceInfo);
	if (Status != XST_SUCCESS) {
		/* The LINK_ADDRESS was sent to a device that cannot reply;
		 * exit from this recursion path. */
		return;
	}

	/* Write GUID to the branch device if it doesn't already have one. */
	XDptx_IssueGuid(InstancePtr, LinkCountTotal, RelativeAddress, Topology,
							DeviceInfo.Guid);

	/* Add the branch device to the topology table. */
	XDptx_AddBranchToList(InstancePtr, &DeviceInfo, LinkCountTotal,
							RelativeAddress);

	/* Downstream devices will be an extra link away from the source than
	 * this branch device. */
	LinkCountTotal++;

	/* Any downstream device downstream device will have the RAD of the
	 * current branch device appended with the port number. */
	u8 DownstreamRelativeAddress[LinkCountTotal - 1];
	/* Copy the branch device's RAD. */
	for (RadIndex = 0; RadIndex < (LinkCountTotal - 2); RadIndex++) {
		DownstreamRelativeAddress[RadIndex] = RelativeAddress[RadIndex];
	}

	for (Index = 0; Index < DeviceInfo.NumPorts; Index++) {
		PortDetails = &DeviceInfo.PortDetails[Index];

		if (PortDetails->InputPort == 0) {
			/* Append the port number to the RAD of the branch
			 * device. */
			DownstreamRelativeAddress[RadIndex] =
							PortDetails->PortNum;

			if (PortDetails->PeerDeviceType == 0x2) {
				/* Found a branch device; recurse the algorithm
				 * to see what DisplayPort devices are connected
				 * to it with the appended RAD. */
				XDptx_FindAccessibleDpDevices(InstancePtr,
						LinkCountTotal,
						DownstreamRelativeAddress);
			}
			else if (PortDetails->DpDevPlugStatus == 1) {
				if ((PortDetails->MsgCapStatus == 1) &&
					(PortDetails->DpcdRev >= 0x12)) {
					/* Write GUID to the branch device if it
					 * doesn't already have one. */
					XDptx_IssueGuid(InstancePtr,
						LinkCountTotal, RelativeAddress,
						Topology, PortDetails->Guid);
				}

				XDptx_AddSinkToList(InstancePtr, PortDetails,
						LinkCountTotal,
						DownstreamRelativeAddress);
			}
		}
	}
}

/******************************************************************************/
/**
 * This function will allocate bandwidth for all enabled stream.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the payload ID tables were successfully updated
 *		  with the new allocation.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, waiting for
 *		  the payload ID table to be cleared or updated, or an AUX
 *		  request timed out.
 *		- XST_BUFFER_TOO_SMALL if there is not enough free timeslots in
 *		  the payload ID table for the requested Ts.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of a sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_AllocatePayloadStreams(XDptx *InstancePtr)
{
	u32 Status;
	u8 StreamIndex;
	XDptx_MstStream *MstStream;
	XDptx_MainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Clear the payload ID table first. */
	Status = XDptx_ClearPayloadVcIdTable(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Allocate the payload table for each stream in both the DisplayPort TX
	 * and RX device. */
	for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
		MstStream = &InstancePtr->MstStreamConfig[StreamIndex];
		MsaConfig = &InstancePtr->MsaConfig[StreamIndex];

		if (XDptx_MstStreamIsEnabled(InstancePtr, StreamIndex + 1)) {
			Status = XDptx_AllocatePayloadVcIdTable(InstancePtr,
				MstStream->LinkCountTotal,
				MstStream->RelativeAddress, StreamIndex + 1,
				MstStream->MstPbn, MsaConfig->TransferUnitSize);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
	}

	/* Generate an ACT event. */
	Status = XDptx_SendActTrigger(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Send ALLOCATE_PAYLOAD request. */
	for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
		MstStream = &InstancePtr->MstStreamConfig[StreamIndex];

		if (XDptx_MstStreamIsEnabled(InstancePtr, StreamIndex + 1)) {
			Status = XDptx_SendSbMsgAllocatePayload(InstancePtr,
				MstStream->LinkCountTotal,
				MstStream->RelativeAddress, StreamIndex + 1,
				MstStream->MstPbn);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will allocate a bandwidth for a virtual channel in the payload
 * ID table in both the DisplayPort TX and the downstream DisplayPort devices
 * on the path to the target device specified by LinkCountTotal and
 * RelativeAddress.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	VcId is the unique virtual channel ID to allocate into the
 *		payload ID tables.
 * @param	Pbn is the payload bandwidth number that determines how much
 *		bandwidth will be allocated for the virtual channel.
 * @param	Ts is the number of timeslots to allocate in the payload ID
 *		tables.
 *
 * @return
 *		- XST_SUCCESS if the payload ID tables were successfully updated
 *		  with the new allocation.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_BUFFER_TOO_SMALL if there is not enough free timeslots in
 *		  the payload ID table for the requested Ts.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of a sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_AllocatePayloadVcIdTable(XDptx *InstancePtr, u8 LinkCountTotal,
				u8 *RelativeAddress, u8 VcId, u16 Pbn, u8 Ts)
{
	u32 Status;
	u8 AuxData[3];
	u8 Index;
	u8 StartTs;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(VcId > 0);
	Xil_AssertNonvoid(Pbn > 0);
	Xil_AssertNonvoid(Ts > 0);

	/* Find next available timeslot. */
	Status = XDptx_GetFirstAvailableTs(InstancePtr, &StartTs);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}

	/* Check that there are enough time slots available. */
	if (((63 - StartTs + 1) < Ts) ||
			(StartTs == 0)) {
		/* Clearing the payload ID table is required to re-allocate
		 * streams. */
		return XST_BUFFER_TOO_SMALL;
	}

	/* Allocate timeslots in TX. */
	for (Index = StartTs; Index < (StartTs + Ts); Index++) {
		XDptx_WriteReg(InstancePtr->Config.BaseAddr,
			(XDPTX_VC_PAYLOAD_BUFFER_ADDR + (4 * Index)), VcId);
	}

	XDptx_WaitUs(InstancePtr, 1000);

	/* Allocate timeslots in sink. */

	/* Clear the VCP table update bit. */
	AuxData[0] = 0x01;
	Status = XDptx_AuxWrite(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Allocate VC with VcId. */
	AuxData[0] = VcId;
	/* Start timeslot for VC with VcId. */
	AuxData[1] = StartTs;
	/* Timeslot count for VC with VcId. */
	AuxData[2] = Ts;
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_PAYLOAD_ALLOCATE_SET, 3,
								AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Wait for the VC table to be updated. */
	do {
		Status = XDptx_AuxRead(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}
	} while ((AuxData[0] & 0x01) != 0x01);

	XDptx_WaitUs(InstancePtr, 1000);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will clear the virtual channel payload ID table in both the
 * DisplayPort TX and all downstream DisplayPort devices.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the payload ID tables were successfully
 *		  cleared.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of a sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_ClearPayloadVcIdTable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData[3];
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Clear the payload table in the transmitter. */
	for (Index = 0; Index < 63; Index++) {
		XDptx_WriteReg(InstancePtr->Config.BaseAddr,
			(XDPTX_VC_PAYLOAD_BUFFER_ADDR + (4 * Index)), 0);
	}

	/* Clear the payload table in the immediate downstream branch device. */

	/* Clear the VCP table update bit. */
	AuxData[0] = 0x01;
	Status = XDptx_AuxWrite(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Allocate VC with VcId. */
	AuxData[0] = 0;
	/* Start timeslot for VC with VcId. */
	AuxData[1] = 0;
	/* Timeslot count for VC with VcId. */
	AuxData[2] = 0x3F;
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_PAYLOAD_ALLOCATE_SET, 3,
								AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Wait for the VC table to be updated. */
	do {
		Status = XDptx_AuxRead(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}
	} while ((AuxData[0] & 0x01) != 0x01);

	Status = XDptx_SendActTrigger(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Send CLEAR_PAYLOAD_ID_TABLE request. */
	Status = XDptx_SendSbMsgClearPayloadIdTable(InstancePtr);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_DPCD_WRITE sideband message which will write
 * some data to the specified DisplayPort Configuration Data (DPCD) address of a
 * downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	DpcdAddress is the DPCD address of the target device that data
 *		will be written to.
 * @param	BytesToWrite is the number of bytes to write to the specified
 *		DPCD address.
 * @param	WriteData is a pointer to a buffer that stores the data to write
 *		to the DPCD location.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgRemoteDpcdWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToWrite <= 0xFFFFF);
	Xil_AssertNonvoid(WriteData != NULL);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (Msg.Header.LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 0;
	Msg.Header.MsgBodyLength = 6 + BytesToWrite;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_REMOTE_DPCD_WRITE;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
						4) | (DpcdAddress >> 16);
	Msg.Body.MsgData[2] = (DpcdAddress & 0x0000FF00) >> 8;
	Msg.Body.MsgData[3] = (DpcdAddress & 0x000000FF);
	Msg.Body.MsgData[4] = BytesToWrite;
	for (Index = 0; Index < BytesToWrite; Index++) {
		Msg.Body.MsgData[5 + Index] = WriteData[Index];
	}
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the REMOTE_DPCD_WRITE transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_DPCD_READ sideband message which will read
 * from the specified DisplayPort Configuration Data (DPCD) address of a
 * downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	DpcdAddress is the DPCD address of the target device that data
 *		will be read from.
 * @param	BytesToRead is the number of bytes to read from the specified
 *		DPCD address.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		DPCD read data.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_DATA_LOST if the requested number of BytesToRead does not
 *		  equal that actually received.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgRemoteDpcdRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToRead <= 0xFFFFF);
	Xil_AssertNonvoid(ReadData != NULL);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (Msg.Header.LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 0;
	Msg.Header.MsgBodyLength = 6;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_REMOTE_DPCD_READ;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
						4) | (DpcdAddress >> 16);
	Msg.Body.MsgData[2] = (DpcdAddress & 0x0000FF00) >> 8;
	Msg.Body.MsgData[3] = (DpcdAddress & 0x000000FF);
	Msg.Body.MsgData[4] = BytesToRead;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the REMOTE_DPCD_READ transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);
	if (Status != XST_SUCCESS) {
		/* Either the reply indicates a NACK, an AUX read or write
		 * transaction failed, there was a time out waiting for a reply,
		 * or a CRC check failed. */
		return Status;
	}

	/* Collect body data into an array. */
	for (Index = 3; Index < SbMsgReply.Length; Index++) {
		ReadData[Index - 3] = SbMsgReply.Data[Index];
	}

	/* The number of bytes actually read does not match that requested. */
	if (Index < BytesToRead) {
		return XST_DATA_LOST;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_I2C_READ sideband message which will read
 * from the specified I2C address of a downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	IicDeviceId is the address on the I2C bus of the target device.
 * @param	BytesToRead is the number of bytes to read from the I2C address.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		I2C read data.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_DATA_LOST if the requested number of BytesToRead does not
 *		  equal that actually received.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgRemoteIicRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 BytesToRead, u8 *ReadData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(IicDeviceId <= 0xFF);
	Xil_AssertNonvoid(BytesToRead <= 0xFF);
	Xil_AssertNonvoid(ReadData != NULL);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (Msg.Header.LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 0;
	Msg.Header.MsgBodyLength = 9;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_REMOTE_I2C_READ;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4) | 1;
	Msg.Body.MsgData[2] = IicDeviceId; /* Write I2C device ID. */
	Msg.Body.MsgData[3] = 1; /* Number of bytes to write. */
	Msg.Body.MsgData[4] = 0; /* Write byte[0]. */
	Msg.Body.MsgData[5] = (0 << 4) | 0;
	Msg.Body.MsgData[6] = IicDeviceId; /* Read I2C device ID. */
	Msg.Body.MsgData[7] = BytesToRead;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the REMOTE_I2C_READ transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);
	if (Status != XST_SUCCESS) {
		/* Either the reply indicates a NACK, an AUX read or write
		 * transaction failed, there was a time out waiting for a reply,
		 * or a CRC check failed. */
		return Status;
	}

	/* Collect body data into an array. */
	for (Index = 3; Index < SbMsgReply.Length; Index++) {
		ReadData[Index - 3] = SbMsgReply.Data[Index];
	}

	/* The number of bytes actually read does not match that requested. */
	if (Index < BytesToRead) {
		return XST_DATA_LOST;
	}

	return Status;
}

/******************************************************************************/
/**
 * This function will send a LINK_ADDRESS sideband message to a target
 * DisplayPort branch device. It is used to determine the resources available
 * for that device and some device information for each of the ports connected
 * to the branch device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort branch device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort branch device.
 * @param	DeviceInfo is a pointer to the device information structure
 *		whose contents will be filled in with the information obtained
 *		by the LINK_ADDRESS sideband message.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	The contents of the DeviceInfo structure will be modified with
 *		the information obtained from the LINK_ADDRESS sideband message
 *		reply.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgLinkAddress(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DeviceInfo != NULL);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal;
	for (Index = 0; Index < (LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 0;
	Msg.Header.MsgBodyLength = 2;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_LINK_ADDRESS;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the LINK_ADDRESS transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}

	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);
	if (Status != XST_SUCCESS) {
		/* Either the reply indicates a NACK, an AUX read or write
		 * transaction failed, there was a time out waiting for a reply,
		 * or a CRC check failed. */
		return Status;
	}
	XDptx_GetDeviceInfoFromSbMsgLinkAddress(&SbMsgReply, DeviceInfo);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send an ENUM_PATH_RESOURCES sideband message which will
 * determine the available payload bandwidth number (PBN) for a path to a target
 * device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	AvailPbn is a pointer to the available PBN of the path whose
 *		value will be filled in by this function.
 * @param	FullPbn is a pointer to the total PBN of the path whose value
 *		will be filled in by this function.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	ENUM_PATH_RESOURCES is a path message that will be serviced by
 *		all downstream DisplayPort devices connecting the DisplayPort TX
 *		and the target device.
 * @note	AvailPbn will be modified with the available PBN from the reply.
 * @note	FullPbn will be modified with the total PBN of the path from the
 *		reply.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgEnumPathResources(XDptx *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, u16 *AvailPbn, u16 *FullPbn)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(AvailPbn != NULL);
	Xil_AssertNonvoid(FullPbn != NULL);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 1;
	Msg.Header.MsgBodyLength = 3;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_ENUM_PATH_RESOURCES;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4);
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the LINK_ADDRESS transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);
	if (Status != XST_SUCCESS) {
		/* Either the reply indicates a NACK, an AUX read or write
		 * transaction failed, there was a time out waiting for a reply,
		 * or a CRC check failed. */
		return Status;
	}

	*AvailPbn = ((SbMsgReply.Data[4] << 8) | SbMsgReply.Data[5]);
	*FullPbn = ((SbMsgReply.Data[2] << 8) | SbMsgReply.Data[3]);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send an ALLOCATE_PAYLOAD sideband message which will
 * allocate bandwidth for a virtual channel in the payload ID tables of the
 * downstream devices connecting the DisplayPort TX to the target device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	VcId is the unique virtual channel ID to allocate into the
 *		payload ID tables.
 * @param	Pbn is the payload bandwidth number that determines how much
 *		bandwidth will be allocated for the virtual channel.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	ALLOCATE_PAYLOAD is a path message that will be serviced by all
 *		downstream DisplayPort devices connecting the DisplayPort TX and
 *		the target device.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgAllocatePayload(XDptx *InstancePtr, u8 LinkCountTotal,
					u8 *RelativeAddress, u8 VcId, u16 Pbn)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(VcId > 0);
	Xil_AssertNonvoid(Pbn > 0);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 1;
	Msg.Header.MsgBodyLength = 6;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_ALLOCATE_PAYLOAD;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4);
	Msg.Body.MsgData[2] = VcId;
	Msg.Body.MsgData[3] = (Pbn >> 8);
	Msg.Body.MsgData[4] = (Pbn & 0xFFFFFFFF);
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the ALLOCATE_PAYLOAD transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a CLEAR_PAYLOAD_ID_TABLE sideband message which will
 * de-allocate all virtual channel payload ID tables.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the reply to the sideband message was
 *		  successfully obtained and it indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	CLEAR_PAYLOAD_ID_TABLE is a broadcast message sent to all
 *		downstream devices.
 *
*******************************************************************************/
u32 XDptx_SendSbMsgClearPayloadIdTable(XDptx *InstancePtr)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = 1;
	Msg.Header.LinkCountRemaining = 6;
	Msg.Header.BroadcastMsg = 1;
	Msg.Header.PathMsg = 1;
	Msg.Header.MsgBodyLength = 2;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDptx_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDPTX_SBMSG_CLEAR_PAYLOAD_ID_TABLE;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDptx_Crc8CalculateBody(&Msg.Body);

	/* Submit the CLEAR_PAYLOAD_ID_TABLE transaction message request. */
	Status = XDptx_SendSbMsg(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will write a global unique identifier (GUID) to the target
 * DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target device.
 * @param	Guid is a the GUID to write to the target device.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_WriteGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 Guid[4])
{
	u8 AuxData[16];
	u8 Index;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertVoid((Guid[0] != 0) || (Guid[1] != 0) || (Guid[2] != 0) ||
								(Guid[3] != 0));

	memset(AuxData, 0, 16);
	for (Index = 0; Index < 16; Index++) {
		AuxData[Index] = (Guid[Index / 4] >> ((3 - (Index % 4)) * 8)) &
									0xFF;
	}

	if (LinkCountTotal == 1) {
		XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_GUID, 16, AuxData);
	}
	else {
		XDptx_SendSbMsgRemoteDpcdWrite(InstancePtr, LinkCountTotal,
				RelativeAddress, XDPTX_DPCD_GUID, 16, AuxData);
	}
}

/******************************************************************************/
/**
 * This function will obtain the global unique identifier (GUID) for the target
 * DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target device.
 * @param	Guid is a pointer to the GUID that will store the existing GUID
 *		of the target device.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDptx_GetGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 *Guid)
{
	u8 Index;
	u8 Data[16];

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertVoid(Guid != NULL);

	if (LinkCountTotal == 1) {
		XDptx_AuxRead(InstancePtr, XDPTX_DPCD_GUID, 16, Data);
	}
	else {
		XDptx_SendSbMsgRemoteDpcdRead(InstancePtr, LinkCountTotal,
				RelativeAddress, XDPTX_DPCD_GUID, 16, Data);
	}

	memset(Guid, 0, 16);
	for (Index = 0; Index < 16; Index++) {
		Guid[Index / 4] <<= 8;
		Guid[Index / 4] |= Data[Index];
	}
}

/******************************************************************************/
/**
 * This function retrieves a remote RX device's Extended Display Identification
 * Data (EDID).
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	A pointer to the Edid buffer to save to.
 *
 * @return
 *		- XST_SUCCESS if the I2C transactions to read the EDID were
 *		  successful.
 *		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_GetRemoteEdid(XDptx *InstancePtr, u8 LinkCountTotal,
						u8 *RelativeAddress, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(Edid != NULL);

	if (LinkCountTotal == 1) {
		Status = XDptx_GetEdid(InstancePtr, Edid);
	}
	else {
		Status = XDptx_SendSbMsgRemoteIicRead(InstancePtr,
				LinkCountTotal, RelativeAddress,
				XDPTX_EDID_ADDR, XDPTX_EDID_SIZE, Edid);
	}

	return Status;
}

/******************************************************************************/
/**
 * This function will check whether or not a DisplayPort device has a global
 * unique identifier (GUID). If it doesn't (the GUID is all zeros), then it will
 * issue one.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target device.
 * @param	Topology is a pointer to the downstream topology.
 * @param	Guid is a pointer to the GUID that will store the new, or
 *		existing GUID for the target device.
 *
 * @return	None.
 *
 * @note	The GUID will be issued from the GuidTable.
 *
*******************************************************************************/
static void XDptx_IssueGuid(XDptx *InstancePtr, u8 LinkCountTotal,
		u8 *RelativeAddress, XDptx_Topology *Topology, u32 *Guid)
{
	XDptx_GetGuid(InstancePtr, LinkCountTotal, RelativeAddress, Guid);
	if ((Guid[0] == 0) && (Guid[1] == 0) && (Guid[2] == 0) &&
							(Guid[3] == 0)) {
		XDptx_WriteGuid(InstancePtr, LinkCountTotal, RelativeAddress,
						GuidTable[Topology->NodeTotal]);

		XDptx_GetGuid(InstancePtr, LinkCountTotal, RelativeAddress,
									Guid);
	}
}

/******************************************************************************/
/**
 * This function will copy the branch device's information into the topology's
 * node table.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	DeviceInfo is a pointer to the device information of the branch
 *		device to add to the list.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the branch device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the branch device.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDptx_AddBranchToList(XDptx *InstancePtr,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;
	XDptx_TopologyNode *TopologyNode;

	/* Add this node to the topology's node list. */
	TopologyNode = &InstancePtr->Topology.NodeTable[
					InstancePtr->Topology.NodeTotal];

	for (Index = 0; Index < 4; Index++) {
		TopologyNode->Guid[Index] = DeviceInfo->Guid[Index];
	}
	for (Index = 0; Index < (LinkCountTotal - 1); Index++) {
		TopologyNode->RelativeAddress[Index] = RelativeAddress[Index];
	}
	TopologyNode->DeviceType = 0x02;
	TopologyNode->LinkCountTotal = LinkCountTotal;
	TopologyNode->DpcdRev = 0x12;
	TopologyNode->MsgCapStatus = 1;

	/* The branch device has been added to the topology node list. */
	InstancePtr->Topology.NodeTotal++;
}

/******************************************************************************/
/**
 * This function will copy the sink device's information into the topology's
 * node table and also add this entry into the topology's sink list.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	SinkDevice is a pointer to the device information of the sink
 *		device to add to the lists.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the sink device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the sink device.
 *
 * @return	None.
 *
 * @note	The sink device is added to both the node and sink list of the
 *		topology structure.
 *
*******************************************************************************/
static void XDptx_AddSinkToList(XDptx *InstancePtr,
			XDptx_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;
	XDptx_Topology *Topology = &InstancePtr->Topology;
	XDptx_TopologyNode *TopologyNode;

	/* Add this node to the topology's node list. */
	TopologyNode = &Topology->NodeTable[Topology->NodeTotal];

	/* Copy the GUID of the sink for the new entry in the topology node
	 * table. */
	for (Index = 0; Index < 4; Index++) {
		TopologyNode->Guid[Index] = SinkDevice->Guid[Index];
	}
	/* Copy the RAD of the sink for the new entry in the topology node
	 * table. */
	for (Index = 0; Index < (LinkCountTotal - 2); Index++) {
		TopologyNode->RelativeAddress[Index] = RelativeAddress[Index];
	}
	TopologyNode->RelativeAddress[Index] = SinkDevice->PortNum;
	TopologyNode->DeviceType = SinkDevice->PeerDeviceType;
	TopologyNode->LinkCountTotal = LinkCountTotal;
	TopologyNode->DpcdRev = SinkDevice->DpcdRev;
	TopologyNode->MsgCapStatus = SinkDevice->MsgCapStatus;

	/* Add this node to the sink list by linking it to the appropriate node
	 * in the topology's node list. */
	Topology->SinkList[Topology->SinkTotal] = TopologyNode;

	/* This node is a sink device, so both the node and sink total are
	 * incremented. */
	Topology->NodeTotal++;
	Topology->SinkTotal++;
}

/******************************************************************************/
/**
 * This function will fill in a device information structure from data obtained
 * by reading the sideband reply from a LINK_ADDRESS sideband message.
 *
 * @param	SbReply is a pointer to the sideband reply structure that stores
 *		the reply data.
 * @param	FormatReply is a pointer to the device information structure
 *		that will filled in with the sideband reply data.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDptx_GetDeviceInfoFromSbMsgLinkAddress(XDptx_SidebandReply
		*SbReply, XDptx_SbMsgLinkAddressReplyDeviceInfo *FormatReply)
{
	u8 ReplyIndex = 0;
	u8 Index, Index2;
	XDptx_SbMsgLinkAddressReplyPortDetail *PortDetails;

	/* Determine the device information from the sideband message reply
	 * structure. */
	FormatReply->ReplyType = (SbReply->Data[ReplyIndex] >> 7);
	FormatReply->RequestId = (SbReply->Data[ReplyIndex++] & 0x7F);

	memset(FormatReply->Guid, 0, 16);
	for (Index = 0; Index < 16; Index++) {
		FormatReply->Guid[Index / 4] <<= 8;
		FormatReply->Guid[Index / 4] |= SbReply->Data[ReplyIndex++];
	}

	FormatReply->NumPorts = SbReply->Data[ReplyIndex++];

	/* For each port of the current device, obtain the details. */
	for (Index = 0; Index < FormatReply->NumPorts; Index++) {
		PortDetails = &FormatReply->PortDetails[Index];

		PortDetails->InputPort = (SbReply->Data[ReplyIndex] >> 7);
		PortDetails->PeerDeviceType =
				((SbReply->Data[ReplyIndex] & 0x70) >> 4);
		PortDetails->PortNum = (SbReply->Data[ReplyIndex++] & 0x0F);
		PortDetails->MsgCapStatus = (SbReply->Data[ReplyIndex] >> 7);
		PortDetails->DpDevPlugStatus =
				((SbReply->Data[ReplyIndex] & 0x40) >> 6);

		if (PortDetails->InputPort == 0) {
			/* Get the port details of the downstream device. */
			PortDetails->LegacyDevPlugStatus =
				((SbReply->Data[ReplyIndex++] & 0x20) >> 5);
			PortDetails->DpcdRev = (SbReply->Data[ReplyIndex++]);

			memset(PortDetails->Guid, 0, 16);
			for (Index2 = 0; Index2 < 16; Index2++) {
				PortDetails->Guid[Index2 / 4] <<= 8;
				PortDetails->Guid[Index2 / 4] |=
						SbReply->Data[ReplyIndex++];
			}

			PortDetails->NumSdpStreams =
					(SbReply->Data[ReplyIndex] >> 4);
			PortDetails->NumSdpStreamSinks =
					(SbReply->Data[ReplyIndex++] & 0x0F);
		}
		else {
			ReplyIndex++;
		}
	}
}

/******************************************************************************/
/**
 * This function will read the payload ID table from the immediate downstream RX
 * device and determine what the first available time slot is in the table.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the immediate downstream RX device's payload ID
 *		  table was successfully read (regardless of whether there are
 *		  any available time slots or not).
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if waiting for the the AUX read request to
 *		  read the RX device's payload ID table timed out.
 *		- XST_FAILURE if the AUX read transaction failed while trying to
 *		  read the RX device's payload ID table.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_GetFirstAvailableTs(XDptx *InstancePtr, u8 *FirstTs)
{
	u32 Status;
	u8 Index;
	u8 AuxData[64];

	Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_VC_PAYLOAD_ID_SLOT(1),
								64, AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}

	for (Index = 0; Index < 64; Index++) {
		/* A zero in the payload ID table indicates that the timeslot is
		 * available. */
		if (AuxData[Index] == 0) {
			*FirstTs = (Index + 1);
			return XST_SUCCESS;
		}
	}

	/* No free time slots available. */
	*FirstTs = 0;
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send a sideband message by creating a data array from the
 * supplied sideband message structure and submitting an AUX write transaction.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if the RX device indicates that the ACT trigger
 *		  has taken effect and the payload ID table has been updated.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for the payload ID table
 *		  to indicate that it has been updated, or the AUX read request
 *		  timed out.
 *		- XST_FAILURE if the AUX read transaction failed while accessing
 *		  the RX device.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_SendActTrigger(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;
	u8 TimeoutCount = 0;

	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_TX_MST_CONFIG, 0x3);

	do {
		Status = XDptx_AuxRead(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, &AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Error out if timed out. */
		if (TimeoutCount > XDPTX_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		TimeoutCount++;
		XDptx_WaitUs(InstancePtr, 1000);
	} while ((AuxData & 0x02) != 0x02);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send a sideband message by creating a data array from the
 * supplied sideband message structure and submitting an AUX write transaction.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Msg is a pointer to the sideband message structure that holds
 *		the contents of the data to be submitted.
 *
 * @return
 *		- XST_SUCCESS if the AUX write transaction used to transmit the
 *		  sideband message was successful.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if the AUX write request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_SendSbMsg(XDptx *InstancePtr, XDptx_SidebandMsg *Msg)
{
	u32 Status;
	u8 AuxData[10+63];
	XDptx_SidebandMsgHeader *Header = &Msg->Header;
	XDptx_SidebandMsgBody *Body = &Msg->Body;
	u8 Index;

	/* Add the header to the sideband message transaction. */
	Msg->Header.MsgHeaderLength = 0;
	AuxData[Msg->Header.MsgHeaderLength++] =
			(Msg->Header.LinkCountTotal << 4) |
						Msg->Header.LinkCountRemaining;
	for (Index = 0; Index < (Header->LinkCountTotal - 1); Index += 2) {
		AuxData[Header->MsgHeaderLength] =
					(Header->RelativeAddress[Index] << 4);

		if ((Index + 1) < (Header->LinkCountTotal - 1)) {
			AuxData[Header->MsgHeaderLength] |=
					Header->RelativeAddress[Index + 1];
		}
		/* Else, the lower (4-bit) nibble is all zeros (for
		 * byte-alignment). */

		Header->MsgHeaderLength++;
	}
	AuxData[Header->MsgHeaderLength++] = (Header->BroadcastMsg << 7) |
				(Header->PathMsg << 6) | Header->MsgBodyLength;
	AuxData[Header->MsgHeaderLength++] = (Header->StartOfMsgTransaction <<
				7) | (Header->EndOfMsgTransaction << 6) |
				(Header->MsgSequenceNum << 4) | Header->Crc;

	/* Add the body to the transaction. */
	for (Index = 0; Index < Body->MsgDataLength; Index++) {
		AuxData[Index + Header->MsgHeaderLength] = Body->MsgData[Index];
	}
	AuxData[Index + Header->MsgHeaderLength] = Body->Crc;

	/* Submit the LINK_ADDRESS transaction message request. */
	Status = XDptx_AuxWrite(InstancePtr, XDPTX_DPCD_DOWN_REQ,
			Msg->Header.MsgHeaderLength + Msg->Header.MsgBodyLength,
			AuxData);

	return Status;
}

/******************************************************************************/
/**
 * This function will wait for a sideband message reply and fill in the SbReply
 * structure with the reply data for use by higher-level functions.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	SbReply is a pointer to the reply structure that this function
 *		will fill in for use by higher-level functions.
 *
 * @return
 *		- XST_SUCCESS if the reply was successfully obtained and it
 *		  indicates an acknowledge.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC did not match the calculated
 *		  value, or the reply was negative acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_ReceiveSbMsg(XDptx *InstancePtr, XDptx_SidebandReply *SbReply)
{
	u32 Status;
	u8 Index = 0;
	u8 AuxData[80];
	XDptx_SidebandMsg Msg;

	SbReply->Length = 0;

	do {
		/* Wait for a reply. */
		Status = XDptx_WaitSbReply(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Receive reply. */
		Status = XDptx_AuxRead(InstancePtr, XDPTX_DPCD_DOWN_REP, 80,
								AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Convert the reply transaction into XDptx_SidebandReply
		 * format. */
		Status = XDptx_Transaction2MsgFormat(AuxData, &Msg);
		if (Status != XST_SUCCESS) {
			/* The CRC of the header or the body did not match the
			 * calculated value. */
			return XST_FAILURE;
		}

		/* Collect body data into an array. */
		for (Index = 0; Index < Msg.Body.MsgDataLength; Index++) {
			SbReply->Data[SbReply->Length++] =
							Msg.Body.MsgData[Index];
		}

		/* Clear. */
		AuxData[0] = 0x10;
		Status = XDptx_AuxWrite(InstancePtr,
				XDPTX_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX write transaction failed. */
			return Status;
		}
	}
	while (Msg.Header.EndOfMsgTransaction == 0);

	/* Check if the reply indicates a NACK. */
	if ((SbReply->Data[0] & 0x80) == 0x80) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will wait until the RX device directly downstream to the
 * DisplayPort TX indicates that a sideband reply is ready to be received by
 * the source.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 *
 * @return
 *		- XST_SUCCESS if a reply message is ready.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or the AUX
 *		  read request timed out.
 *		- XST_FAILURE if the AUX read transaction failed while accessing
 *		  the RX device.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_WaitSbReply(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;
	u8 TimeoutCount = 0;

	do {
		Status = XDptx_AuxRead(InstancePtr,
				XDPTX_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, &AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Error out if timed out. */
		if (TimeoutCount > XDPTX_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		TimeoutCount++;
		XDptx_WaitUs(InstancePtr, 1000);
	}
	while ((AuxData & 0x10) != 0x10);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will take a byte array and convert it into a sideband message
 * format by filling in the XDptx_SidebandMsg structure with the array data.
 *
 * @param	Transaction is the pointer to the data used to fill in the
 *		sideband message structure.
 * @param	Msg is a pointer to the sideband message structure that will be
 *		filled in with the transaction data.
 *
 * @return
 *		- XST_SUCCESS if the transaction data was successfully converted
 *		  into the sideband message structure format.
 *		- XST_FAILURE otherwise, if the calculated header or body CRC
 *		  does not match that contained in the transaction data.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDptx_Transaction2MsgFormat(u8 *Transaction, XDptx_SidebandMsg *Msg)
{
	XDptx_SidebandMsgHeader *Header = &Msg->Header;
	XDptx_SidebandMsgBody *Body = &Msg->Body;

	u8 Index = 0;
	u8 CrcCheck;

	/* Fill the header structure from the reply transaction. */
	Header->MsgHeaderLength = 0;
	/* Byte 0. */
	Header->LinkCountTotal = Transaction[Header->MsgHeaderLength] >> 4;
	Header->LinkCountRemaining = Transaction[Header->MsgHeaderLength] &
									0x0F;
	Header->MsgHeaderLength++;
	/* If LinkCountTotal > 1, Byte 1 to Byte (_(LinkCountTotal / 2)_) */
	for (Index = 0; Index < (Header->LinkCountTotal - 1); Index += 2) {
		Header->RelativeAddress[Index] =
				Transaction[Header->MsgHeaderLength] >> 4;

		if ((Index + 1) < (Header->LinkCountTotal - 1)) {
			Header->RelativeAddress[Index + 1] =
				Transaction[Header->MsgHeaderLength] & 0x0F;
		}

		Header->MsgHeaderLength++;
	}
	/* Byte (1 + _(LinkCountTotal / 2)_). */
	Header->BroadcastMsg = Transaction[Header->MsgHeaderLength] >> 7;
	Header->PathMsg = (Transaction[Header->MsgHeaderLength] & 0x40) >> 6;
	Header->MsgBodyLength = Transaction[Header->MsgHeaderLength] & 0x3F;
	Header->MsgHeaderLength++;
	/* Byte (2 + _(LinkCountTotal / 2)_). */
	Header->StartOfMsgTransaction = Transaction[Header->MsgHeaderLength] >>
									7;
	Header->EndOfMsgTransaction = (Transaction[Header->MsgHeaderLength] &
								0x40) >> 6;
	Header->MsgSequenceNum = (Transaction[Header->MsgHeaderLength] &
								0x10) >> 4;
	Header->Crc = Transaction[Header->MsgHeaderLength] & 0x0F;
	Header->MsgHeaderLength++;
	/* Verify the header CRC. */
	CrcCheck = XDptx_Crc4CalculateHeader(Header);
	if (CrcCheck != Header->Crc) {
		/* The calculated CRC for the header did not match the
		 * response. */
		return XST_FAILURE;
	}

	/* Fill the body structure from the reply transaction. */
	Body->MsgDataLength = Header->MsgBodyLength - 1;
	for (Index = 0; Index < Body->MsgDataLength; Index++) {
		Body->MsgData[Index] = Transaction[Header->MsgHeaderLength +
									Index];
	}
	Body->Crc = Transaction[Header->MsgHeaderLength + Index];
	/* Verify the body CRC. */
	CrcCheck = XDptx_Crc8CalculateBody(Body);
	if (CrcCheck != Body->Crc) {
		/* The calculated CRC for the body did not match the
		 * response. */
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will perform a cyclic redundancy check (CRC) on the header of a
 * sideband message using a generator polynomial of 4.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Header is a pointer sideband message header that the CRC
 *		algorithm is to be run on.
 *
 * @return	The CRC value obtained by running the algorithm on the sideband
 *		message header.
 *
 * @note	The header is divided into 4-bit nibbles for use by the lower-
 *		level XDptx_CrcCalculate function.
 *
*******************************************************************************/
static u8 XDptx_Crc4CalculateHeader(XDptx_SidebandMsgHeader *Header)
{
	u8 Nibbles[20];
	u8 RadOffset = 0;

	/* Arrange header into nibbles for the CRC. */
	Nibbles[0] = Header->LinkCountTotal;
	Nibbles[1] = Header->LinkCountRemaining;

	for (RadOffset = 0; RadOffset < (Header->LinkCountTotal - 1);
							RadOffset += 2) {
		Nibbles[2 + RadOffset] = Header->RelativeAddress[RadOffset];

		/* Byte (8-bits) align the nibbles (4-bits). */
		if ((RadOffset + 1) < (Header->LinkCountTotal - 1)) {
			Nibbles[2 + RadOffset + 1] =
					Header->RelativeAddress[RadOffset + 1];
		}
		else {
			Nibbles[2 + RadOffset + 1] = 0;
		}
	}

	Nibbles[2 + RadOffset] = (Header->BroadcastMsg << 3) |
		(Header->PathMsg << 2) | ((Header->MsgBodyLength & 0x30) >> 4);
	Nibbles[3 + RadOffset] = Header->MsgBodyLength & 0x0F;
	Nibbles[4 + RadOffset] = (Header->StartOfMsgTransaction << 3) |
		(Header->EndOfMsgTransaction << 2) | Header->MsgSequenceNum;

	return XDptx_CrcCalculate(Nibbles, 4 * (5 + RadOffset), 4);
}

/******************************************************************************/
/**
 * This function will perform a cyclic redundancy check (CRC) on the body of a
 * sideband message using a generator polynomial of 8.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Body is a pointer sideband message body that the CRC algorithm
 *		is to be run on.
 *
 * @return	The CRC value obtained by running the algorithm on the sideband
 *		message body.
 *
 * @note	None.
 *
*******************************************************************************/
static u8 XDptx_Crc8CalculateBody(XDptx_SidebandMsgBody *Body)
{
	return XDptx_CrcCalculate(Body->MsgData, 8 * Body->MsgDataLength, 8);
}

/******************************************************************************/
/**
 * This function will run a cyclic redundancy check (CRC) algorithm on some data
 * given a generator polynomial.
 *
 * @param	Data is a pointer to the data that the algorithm is to run on.
 * @param	NumberOfBits is the total number of data bits that the algorithm
 *		is to run on.
 * @param	Polynomial is the generator polynomial for the CRC algorithm and
 *		will be used as the divisor in the polynomial long division.
 *
 * @return	The CRC value obtained by running the algorithm on the data
 *		using the specified polynomial.
 *
 * @note	None.
 *
*******************************************************************************/
static u8 XDptx_CrcCalculate(const u8 *Data, u32 NumberOfBits, u8 Polynomial)
{
	u8 BitMask;
	u8 BitShift;
	u8 ArrayIndex = 0;
	u16 Remainder = 0;

	if (Polynomial == 4) {
		/* For CRC4, expecting nibbles (4-bits). */
		BitMask = 0x08;
		BitShift = 3;
	}
	else {
		/* For CRC8, expecting bytes (8-bits). */
		BitMask = 0x80;
		BitShift = 7;
	}

	while (NumberOfBits != 0) {
		NumberOfBits--;

		Remainder <<= 1;
		Remainder |= (Data[ArrayIndex] & BitMask) >> BitShift;

		BitMask >>= 1;
		BitShift--;

		if (BitMask == 0) {
			if (Polynomial == 4) {
				BitMask = 0x08;
				BitShift = 3;
			}
			else {
				BitMask = 0x80;
				BitShift = 7;
			}
			ArrayIndex++;
		}

		if ((Remainder & (1 << Polynomial)) != 0) {
			if (Polynomial == 4) {
				Remainder ^= 0x13;
			}
			else {
				Remainder ^= 0xD5;
			}
		}
	}

	NumberOfBits = Polynomial;
	while (NumberOfBits != 0) {
		NumberOfBits--;

		Remainder <<= 1;

		if ((Remainder & (1 << Polynomial)) != 0) {
			if (Polynomial == 4) {
				Remainder ^= 0x13;
				Remainder &= 0xFF;
			}
			else {
				Remainder ^= 0xD5;
			}
		}
	}

	return Remainder & 0xFF;
}
