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

typedef struct
{
	u8 LinkCountTotal;
	u8 LinkCountRemaining;
	u8 RelativeAddress[14];
	u8 BroadcastMsg;
	u8 PathMsg;
	u8 MsgBodyLength;
	u8 StartOfMsgTransaction;
	u8 EndOfMsgTransaction;
	u8 MsgSequenceNum;
	u8 Crc;

	u8 MsgHeaderLength;
} XDptx_SidebandMsgHeader;

typedef struct
{
	u8 MsgData[62];
	u8 MsgDataLength;
	u8 Crc;
} XDptx_SidebandMsgBody;

typedef struct
{
	XDptx_SidebandMsgHeader Header;
	XDptx_SidebandMsgBody Body;
} XDptx_SidebandMsg;

typedef struct
{
	u8 Length;
	u8 Data[256];
} XDptx_SidebandReply;

static u32 XDptx_SendActTrigger(XDptx *InstancePtr);
static u32 XDptx_AllocatePayloadVcIdTable(XDptx *InstancePtr, u8 LinkCountTotal,
				u8 *RelativeAddress, u8 VcId, u16 Pbn, u8 Ts);
static u32 XDptx_GetFirstAvailableTs(XDptx *InstancePtr, u8 *FirstTs);
static void XDptx_AddBranchToList(XDptx_Topology *Topology,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDptx_AddSinkToList(XDptx_Topology *Topology,
			XDptx_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDptx_IssueGuid(XDptx *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, XDptx_Topology *Topology,
			u32 *Guid);
static u32 XDptx_SendSbMsg(XDptx *InstancePtr, XDptx_SidebandMsg *Msg);
static u32 XDptx_WaitSbReply(XDptx *InstancePtr);
static void XDptx_GetDeviceInfoFromSbMsgLinkAddress(
			XDptx_SidebandReply *SbReply,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *FormatReply);
static u8 XDptx_CrcCalculate(const u8 *Data, u32 NumberOfBits, u8 Polynomial);
static u8 XDptx_Crc4CalculateHeader(XDptx_SidebandMsgHeader *Header);
static u8 XDptx_Crc8CalculateBody(XDptx_SidebandMsgBody *Body);
static u32 XDptx_Transaction2MsgFormat(u8 *Transaction, XDptx_SidebandMsg *Msg);
static u32 XDptx_ReceiveSbMsg(XDptx *InstancePtr, XDptx_SidebandReply *SbReply);

u32 GuidTable[11][4] = {
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
                {0x12145678, 0x41214121, 0xCBCD1F98, 0x87658765}
};

/**************************** Function Definitions ****************************/

void XDptx_MstCfgModeEnable(XDptx *InstancePtr)
{
	InstancePtr->MstEnable = 1;
}

void XDptx_MstCfgModeDisable(XDptx *InstancePtr)
{
	InstancePtr->MstEnable = 0;
}

void XDptx_MstCfgStreamEnable(XDptx *InstancePtr, u8 Stream)
{
	InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable = 1;
}

void XDptx_MstCfgStreamDisable(XDptx *InstancePtr, u8 Stream)
{
	InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable = 0;
}

u8 XDptx_MstStreamIsEnabled(XDptx *InstancePtr, u8 Stream)
{
	return InstancePtr->MstStreamConfig[Stream - 1].MstStreamEnable;
}

u32 XDptx_MstEnable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;

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

u32 XDptx_MstDisable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData;

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

void XDptx_SetStreamSelectFromSinkList(XDptx *InstancePtr, u8 Stream, u8
									SinkNum)
{
	u8 Index;
	XDptx_MstStream *MstStream = &InstancePtr->MstStreamConfig[Stream];
	XDptx_Topology *Topology = &InstancePtr->Topology;

	/* Check sink count. */

	MstStream->LinkCountTotal = Topology->SinkList[SinkNum]->LinkCountTotal;
	for (Index = 0; Index < MstStream->LinkCountTotal - 1; Index++) {
		MstStream->RelativeAddress[Index] =
			Topology->SinkList[SinkNum]->RelativeAddress[Index];
	}
}

void XDptx_SetStreamSinkRad(XDptx *InstancePtr, u8 Stream, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u8 Index;
	XDptx_MstStream *MstStream = &InstancePtr->MstStreamConfig[Stream];

	MstStream->LinkCountTotal = LinkCountTotal;
	for (Index = 0; Index < MstStream->LinkCountTotal - 1; Index++) {
		MstStream->RelativeAddress[Index] = RelativeAddress[Index];
	}
}

u32 XDptx_AllocatePayloadStreams(XDptx *InstancePtr)
{
	u32 Status;
	u8 StreamIndex;

	Status = XDptx_ClearPayloadVcIdTable(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* Either a AUX read or write transaction failed trying to clear
		 * the payload ID table or read the update status, or waiting
		 * for an indication that the payload ID table was updated
		 * timed out. */
		return Status;
	}

	/* Allocate the payload table for each stream in both the DisplayPort TX
	 * and RX device. */
	for (StreamIndex = 0; StreamIndex < 4; StreamIndex++) {
		if (XDptx_MstStreamIsEnabled(InstancePtr, StreamIndex + 1)) {
			Status = XDptx_AllocatePayloadVcIdTable(InstancePtr,
		InstancePtr->MstStreamConfig[StreamIndex].LinkCountTotal,
		InstancePtr->MstStreamConfig[StreamIndex].RelativeAddress,
							StreamIndex + 1,
		InstancePtr->MstStreamConfig[StreamIndex].MstPbn,
		InstancePtr->MsaConfig[StreamIndex].TransferUnitSize);
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
		if (XDptx_MstStreamIsEnabled(InstancePtr, StreamIndex + 1)) {
			Status = XDptx_SendSbMsgAllocatePayload(InstancePtr,
		InstancePtr->MstStreamConfig[StreamIndex].LinkCountTotal,
		InstancePtr->MstStreamConfig[StreamIndex].RelativeAddress,
							StreamIndex + 1,
		InstancePtr->MstStreamConfig[StreamIndex].MstPbn);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
	}

	return XST_SUCCESS;
}

static u32 XDptx_AllocatePayloadVcIdTable(XDptx *InstancePtr, u8 LinkCountTotal,
				u8 *RelativeAddress, u8 VcId, u16 Pbn, u8 Ts)
{
	u32 Status;
	u8 AuxData[3];
	u8 Index;
	u8 StartTs;

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

static u32 XDptx_SendActTrigger(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData[1];

	XDptx_WriteReg(InstancePtr->Config.BaseAddr, XDPTX_TX_MST_CONFIG, 0x3);

	do {
		Status = XDptx_AuxRead(InstancePtr,
			XDPTX_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}
	} while ((AuxData[0] & 0x02) != 0x02);

	return XST_SUCCESS;
}

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
		if (AuxData[Index] == 0) {
			*FirstTs = (Index + 1);
			return XST_SUCCESS;
		}
	}

	/* No free time slots available. */
	*FirstTs = 0;
	return XST_SUCCESS;
}

u32 XDptx_ClearPayloadVcIdTable(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData[3];
	u8 Index;

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

void XDptx_FindAccessibleDpDevices(XDptx *InstancePtr, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u8 Index;
	u8 RadIndex;
	XDptx_SbMsgLinkAddressReplyDeviceInfo DeviceInfo;
	XDptx_Topology *Topology = &InstancePtr->Topology;

	XDptx_SendSbMsgLinkAddress(InstancePtr, LinkCountTotal, RelativeAddress,
								&DeviceInfo);

	/* Write GUID to the branch device if it doesn't already have one. */
	XDptx_IssueGuid(InstancePtr, LinkCountTotal, RelativeAddress, Topology,
							DeviceInfo.Guid);

	/* Add the branch device to the topology table. */
	XDptx_AddBranchToList(Topology, &DeviceInfo, LinkCountTotal,
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
		if (DeviceInfo.PortDetails[Index].InputPort == 0) {
			/* Append the port number to the RAD of the branch
			 * device. */
			DownstreamRelativeAddress[RadIndex] =
					DeviceInfo.PortDetails[Index].PortNum;

			if (DeviceInfo.PortDetails[Index].PeerDeviceType ==
									0x2) {
				/* Found a branch device; recurse the algorithm
				 * to see what DisplayPort devices are connected
				 * to it with the appended RAD. */
				XDptx_FindAccessibleDpDevices(InstancePtr,
						LinkCountTotal,
						DownstreamRelativeAddress);
			}
			else if (DeviceInfo.PortDetails[Index].DpDevPlugStatus
									== 1) {
				if ((DeviceInfo.PortDetails[Index].MsgCapStatus
		== 1) && (DeviceInfo.PortDetails[Index].DpcdRev >= 0x12)) {
					/* Write GUID to the branch device if it
					 * doesn't already have one. */
					XDptx_IssueGuid(InstancePtr,
						LinkCountTotal, RelativeAddress,
						Topology, DeviceInfo.Guid);
				}

				XDptx_AddSinkToList(Topology,
						&DeviceInfo.PortDetails[Index],
						LinkCountTotal,
						DownstreamRelativeAddress);
			}
		}
	}
}

void XDptx_GetGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 *Guid)
{
	u8 Index;
	u8 Data[30];

	if (LinkCountTotal == 1) {
		XDptx_AuxRead(InstancePtr, 0x30, 16, Data);

		memset(Guid, 0, 16);
		for (Index = 0; Index < 16; Index++) {
			Guid[Index / 4] <<= 8;
			Guid[Index / 4] |= Data[Index];
		}
	}
	else {
		XDptx_SendSbMsgRemoteDpcdRead(InstancePtr, LinkCountTotal,
					RelativeAddress, 0x30, 16, Data);

		memset(Guid, 0, 16);
		for (Index = 0; Index < 16; Index++) {
			Guid[Index / 4] <<= 8;
			Guid[Index / 4] |= Data[Index + 3];
		}
	}
}

void XDptx_WriteGuid(XDptx *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u32 Guid[4])
{
	u8 AuxData[16];
	u8 Index;

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

u32 XDptx_SendSbMsgRemoteDpcdWrite(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u8 BytesToWrite, u8 *WriteData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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

u32 XDptx_SendSbMsgRemoteDpcdRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u8 BytesToRead, u8 *ReadData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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
	for (Index = 0; Index < SbMsgReply.Length; Index++) {
		ReadData[Index] = SbMsgReply.Data[Index];
	}

	return XST_SUCCESS;
}

u32 XDptx_SendSbMsgRemoteIicRead(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 IicDeviceId, u8 BytesToRead, u8 *ReadData)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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
	for (Index = 0; Index < SbMsgReply.Length; Index++) {
		ReadData[Index] = SbMsgReply.Data[Index];
	}

	return Status;
}

u32 XDptx_SendSbMsgLinkAddress(XDptx *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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

	XDptx_ReceiveSbMsg(InstancePtr, &SbMsgReply);
	XDptx_GetDeviceInfoFromSbMsgLinkAddress(&SbMsgReply, DeviceInfo);

	return Status;
}

u32 XDptx_SendSbMsgEnumPathResources(XDptx *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, u16 *AvailPbn, u16 *FullPbn)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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

u32 XDptx_SendSbMsgAllocatePayload(XDptx *InstancePtr, u8 LinkCountTotal,
					u8 *RelativeAddress, u8 VcId, u16 Pbn)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;
	u8 Index;

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

u32 XDptx_SendSbMsgClearPayloadIdTable(XDptx *InstancePtr)
{
	u32 Status;
	XDptx_SidebandMsg Msg;
	XDptx_SidebandReply SbMsgReply;

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

static void XDptx_AddBranchToList(XDptx_Topology *Topology,
			XDptx_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;

	for (Index = 0; Index < 4; Index++) {
		Topology->NodeTable[Topology->NodeTotal].Guid[Index] =
							DeviceInfo->Guid[Index];
	}
	for (Index = 0; Index < (LinkCountTotal - 1); Index++) {
		Topology->NodeTable[Topology->NodeTotal].RelativeAddress[Index]
						= RelativeAddress[Index];
	}
	Topology->NodeTable[Topology->NodeTotal].DeviceType = 0x02;
	Topology->NodeTable[Topology->NodeTotal].LinkCountTotal =
								LinkCountTotal;
	Topology->NodeTable[Topology->NodeTotal].DpcdRev = 0x12;
	Topology->NodeTable[Topology->NodeTotal].MsgCapStatus = 1;

	Topology->NodeTotal++;
}

static void XDptx_AddSinkToList(XDptx_Topology *Topology,
			XDptx_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;

	/* Copy the GUID of the sink for the new entry in the topology node
	 * table. */
	for (Index = 0; Index < 4; Index++) {
		Topology->NodeTable[Topology->NodeTotal].Guid[Index] =
							SinkDevice->Guid[Index];
	}
	/* Copy the RAD of the sink for the new entry in the topology node
	 * table. */
	for (Index = 0; Index < (LinkCountTotal - 2); Index++) {
		Topology->NodeTable[Topology->NodeTotal].RelativeAddress[Index]
						= RelativeAddress[Index];
	}
	Topology->NodeTable[Topology->NodeTotal].RelativeAddress[Index] =
						SinkDevice->PortNum;
	Topology->NodeTable[Topology->NodeTotal].DeviceType =
						SinkDevice->PeerDeviceType;
	Topology->NodeTable[Topology->NodeTotal].LinkCountTotal =
						LinkCountTotal;
	Topology->NodeTable[Topology->NodeTotal].DpcdRev =
						SinkDevice->DpcdRev;
	Topology->NodeTable[Topology->NodeTotal].MsgCapStatus =
						SinkDevice->MsgCapStatus;

	Topology->SinkList[Topology->SinkTotal] =
				&Topology->NodeTable[Topology->NodeTotal++];
	Topology->SinkTotal++;
}

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

static u32 XDptx_WaitSbReply(XDptx *InstancePtr)
{
	u32 Status;
	u8 AuxData[1];

	do {
		Status = XDptx_AuxRead(InstancePtr,
				XDPTX_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}
	}
	while ((AuxData[0] & 0x10) != 0x10);

	return XST_SUCCESS;
}

static void XDptx_GetDeviceInfoFromSbMsgLinkAddress(XDptx_SidebandReply
		*SbReply, XDptx_SbMsgLinkAddressReplyDeviceInfo *FormatReply)
{
	u8 ReplyIndex = 0;
	u8 Index, Index2;

	FormatReply->ReplyType = (SbReply->Data[ReplyIndex] >> 7);
	FormatReply->RequestId = (SbReply->Data[ReplyIndex++] & 0x7F);

	memset(FormatReply->Guid, 0, 16);
	for (Index = 0; Index < 16; Index++) {
		FormatReply->Guid[Index / 4] <<= 8;
		FormatReply->Guid[Index / 4] |= SbReply->Data[ReplyIndex++];
	}

	FormatReply->NumPorts = SbReply->Data[ReplyIndex++];

	for (Index = 0; Index < FormatReply->NumPorts; Index++) {
		FormatReply->PortDetails[Index].InputPort =
				(SbReply->Data[ReplyIndex] >> 7);
		FormatReply->PortDetails[Index].PeerDeviceType =
				((SbReply->Data[ReplyIndex] & 0x70) >> 4);
		FormatReply->PortDetails[Index].PortNum =
				(SbReply->Data[ReplyIndex++] & 0x0F);
		FormatReply->PortDetails[Index].MsgCapStatus =
				(SbReply->Data[ReplyIndex] >> 7);
		FormatReply->PortDetails[Index].DpDevPlugStatus =
				((SbReply->Data[ReplyIndex] & 0x40) >> 6);

		if (FormatReply->PortDetails[Index].InputPort == 0) {
			FormatReply->PortDetails[Index].LegacyDevPlugStatus =
				((SbReply->Data[ReplyIndex++] & 0x20) >> 5);
			FormatReply->PortDetails[Index].DpcdRev =
						(SbReply->Data[ReplyIndex++]);

			memset(FormatReply->PortDetails[Index].Guid, 0, 16);
			for (Index2 = 0; Index2 < 16; Index2++) {
				FormatReply->PortDetails[Index].Guid[Index2 / 4]
									<<= 8;
				FormatReply->PortDetails[Index].Guid[Index2 / 4]
						|= SbReply->Data[ReplyIndex++];
			}

			FormatReply->PortDetails[Index].NumSdpStreams =
					(SbReply->Data[ReplyIndex] >> 4);
			FormatReply->PortDetails[Index].NumSdpStreamSinks =
					(SbReply->Data[ReplyIndex++] & 0x0F);
		}
		else {
			ReplyIndex++;
		}
	}
}

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
		return XST_CRC_ERROR;
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
		return XST_CRC_ERROR;
	}

	return XST_SUCCESS;
}

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

static u8 XDptx_Crc8CalculateBody(XDptx_SidebandMsgBody *Body)
{
	return XDptx_CrcCalculate(Body->MsgData, 8 * Body->MsgDataLength, 8);
}

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
			/* Either the AUX read transaction used to check the
			 * reply status failed, or waiting for a reply timed
			 * out. */
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
			return XST_CRC_ERROR;
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
		/* Reply with the reason for NACK. See xdptx.h
		 * XDPTX_SBREPLY_NACK_* for a list of possible values (range is
		 * (XST_SBREPLY_NACK + 0x01) to (XST_SBREPLY_NACK + 0x0A)). */
		return (XST_SBREPLY_NACK + SbReply->Data[17]);
	}

	return XST_SUCCESS;
}
