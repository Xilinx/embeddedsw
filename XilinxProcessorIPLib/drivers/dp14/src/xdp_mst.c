/*******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdp_mst.c
 * @addtogroup dp_v7_6
 * @{
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial release. TX code merged from the dptx driver.
 * 2.0   als  06/08/15 Added MST functionality to RX.
 * 4.0   als  02/16/16 XDp_TxAllocatePayloadStreams now allocates payloads by
 *                     using an internally calculated starting timeslot for each
 *                     stream rather than invoking XDp_TxGetFirstAvailableTs.
 *                     XDp_TxAllocatePayloadVcIdTable now takes an additional
 *                     argument (StartTs, the starting timeslot).
 *                     Exposed RX MST API to application:
 *                         XDp_RxAllocatePayloadStream
 * 5.2  aad  01/24/16 XDp_RxAllocatePayloadStream now adjusts to timeslot
 *			   rearragement
 * 6.0	tu   05/30/17 Initialized variable in XDp_RxDeviceInfoToRawData
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "string.h"
#include "xdp.h"

/**************************** Constant Definitions ****************************/

/* The maximum length of a sideband message. Longer messages must be split into
 * multiple fragments. */
#define XDP_MAX_LENGTH_SBMSG 48

#if XPAR_XDPTXSS_NUM_INSTANCES
/* Error out if waiting for a sideband message reply or waiting for the payload
 * ID table to be updated takes more than 5000 AUX read iterations. */
#define XDP_TX_MAX_SBMSG_REPLY_TIMEOUT_COUNT 5000
/* Error out if waiting for the RX device to indicate that it has received an
 * ACT trigger takes more than 30 AUX read iterations. */
#define XDP_TX_VCP_TABLE_MAX_TIMEOUT_COUNT 30
#endif

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
	u8 MsgSequenceNum;		/**< Identifies individual message
						transactions to a given
						DisplayPort device. */
	u8 Crc;				/**< The cyclic-redundancy check (CRC)
						value of the header data. */

	u8 MsgHeaderLength;		/**< The number of data bytes stored as
						part of the sideband message
						header. */
} XDp_SidebandMsgHeader;

/**
 * This typedef stores the sideband message body.
 */
typedef struct
{
	u8 MsgData[256];		/**< The raw body data of the sideband
						message. */
	u8 MsgDataLength;		/**< The number of data bytes stored as
						part of the sideband message
						body. */
	u8 Crc;				/**< The cyclic-redundancy check (CRC)
						value of the body data. */
} XDp_SidebandMsgBody;

/**
 * This typedef stores the entire sideband message.
 */
typedef struct
{
	XDp_SidebandMsgHeader Header;	/**< The header segment of the sideband
						message. */
	XDp_SidebandMsgBody Body;	/**< The body segment of the sideband
						message. */
	u8 FragmentNum;			/**< Larger sideband messages need to be
						broken up into multiple
						fragments. For RX, this number
						indicates the fragment with
						which the current header
						corresponds to. */
} XDp_SidebandMsg;

/**
 * This typedef describes a sideband message reply.
 */
typedef struct
{
	u8 Length;			/**< The number of bytes of reply
						data. */
	u8 Data[256];			/**< The raw reply data. */
} XDp_SidebandReply;

/**************************** Function Prototypes *****************************/
#if XPAR_XDPRXSS_NUM_INSTANCES
static void XDp_RxSetLinkAddressReply(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static void XDp_RxSetClearPayloadIdReply(XDp_SidebandMsg *Msg);
static void XDp_RxSetAllocPayloadReply(XDp_SidebandMsg *Msg);
static void XDp_RxSetEnumPathResReply(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static void XDp_RxSetGenericNackReply(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static u32 XDp_RxSetRemoteDpcdReadReply(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static u32 XDp_RxSetRemoteIicReadReply(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static void XDp_RxDeviceInfoToRawData(XDp *InstancePtr, XDp_SidebandMsg *Msg);
static void XDp_RxSetAvailPbn(XDp *InstancePtr, XDp_SidebandMsg *Msg);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
static void XDp_TxIssueGuid(XDp *InstancePtr, u8 LinkCountTotal,
		u8 *RelativeAddress, XDp_TxTopology *Topology, u8 *Guid);
static void XDp_TxAddBranchToList(XDp *InstancePtr,
			XDp_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDp_TxAddSinkToList(XDp *InstancePtr,
			XDp_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress);
static void XDp_TxGetDeviceInfoFromSbMsgLinkAddress(
			XDp_SidebandReply *SbReply,
			XDp_SbMsgLinkAddressReplyDeviceInfo *FormatReply);
static u32 XDp_TxSendActTrigger(XDp *InstancePtr);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

static u32 XDp_SendSbMsgFragment(XDp *InstancePtr, XDp_SidebandMsg *Msg);

#if XPAR_XDPRXSS_NUM_INSTANCES
static void XDp_RxReadDownReq(XDp *InstancePtr, XDp_SidebandMsg *Msg);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
static u32 XDp_TxReceiveSbMsg(XDp *InstancePtr, XDp_SidebandReply *SbReply);
static u32 XDp_TxWaitSbReply(XDp *InstancePtr);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

static u32 XDp_Transaction2MsgFormat(u8 *Transaction, XDp_SidebandMsg *Msg);

#if XPAR_XDPRXSS_NUM_INSTANCES
static u32 XDp_RxWriteRawDownReply(XDp *InstancePtr, u8 *Data, u8 DataLength);
static u32 XDp_RxSendSbMsg(XDp *InstancePtr, XDp_SidebandMsg *Msg);
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

static u8 XDp_Crc4CalculateHeader(XDp_SidebandMsgHeader *Header);
static u8 XDp_Crc8CalculateBody(XDp_SidebandMsg *Msg);
static u8 XDp_CrcCalculate(const u8 *Data, u32 NumberOfBits, u8 Polynomial);

#if XPAR_XDPTXSS_NUM_INSTANCES
static u32 XDp_TxIsSameTileDisplay(u8 *DispIdSecTile0, u8 *DispIdSecTile1);
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/**************************** Variable Definitions ****************************/

/**
 * This table contains a list of global unique identifiers (GUIDs) that will be
 * issued when exploring the topology using the algorithm in the
 * XDp_TxFindAccessibleDpDevices function.
 */
u8 GuidTable[16][XDP_GUID_NBYTES] = {
	{0x78, 0x69, 0x6C, 0x61, 0x6E, 0x64, 0x72, 0x65,
				0x69, 0x6C, 0x73, 0x69, 0x6D, 0x69, 0x6F, 0x6E},
	{0x12, 0x34, 0x12, 0x34, 0x43, 0x21, 0x43, 0x21,
				0x56, 0x78, 0x56, 0x78, 0x87, 0x65, 0x87, 0x65},
	{0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0xEF, 0xDE, 0xAD,
				0x10, 0x01, 0x10, 0x01, 0xDA, 0xDA, 0xDA, 0xDA},
	{0xDA, 0xBA, 0xDA, 0xBA, 0x10, 0x01, 0x10, 0x01,
				0xBA, 0xDA, 0xBA, 0xDA, 0x5A, 0xD5, 0xAD, 0x5A},
	{0x12, 0x34, 0x56, 0x78, 0x43, 0x21, 0x43, 0x21,
				0xAB, 0xCD, 0xEF, 0x98, 0x87, 0x65, 0x87, 0x65},
	{0x12, 0x14, 0x12, 0x14, 0x41, 0x21, 0x41, 0x21,
				0x56, 0x78, 0x56, 0x78, 0x87, 0x65, 0x87, 0x65},
	{0xD1, 0xCD, 0xB1, 0x1F, 0xB1, 0x1F, 0xD1, 0xCD,
				0xFE, 0xBC, 0xDA, 0x90, 0xDC, 0xDC, 0xDC, 0xDC},
	{0xDC, 0xBC, 0xDC, 0xBC, 0xE0, 0x00, 0xE0, 0x00,
				0xBC, 0xDC, 0xBC, 0xDC, 0x5C, 0xD5, 0xCD, 0x5C},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
				0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
				0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
	{0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
				0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
	{0xAA, 0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFE, 0xBC, 0xDA, 0x90, 0xDC, 0xDC, 0xDC, 0xDC},
	{0xBB, 0xBB, 0xBB, 0xBB, 0xE0, 0x00, 0xE0, 0x00,
				0xFF, 0xFF, 0xFF, 0xFF, 0x5C, 0xD5, 0xCD, 0x5C},
	{0xCC, 0xCC, 0xCC, 0xCC, 0x11, 0x11, 0x11, 0x11,
				0x11, 0x11, 0x11, 0x11, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xDD, 0xDD, 0xDD, 0xDD, 0x22, 0x22, 0x22, 0x22,
				0xFF, 0xFF, 0xFF, 0xFF, 0x22, 0x22, 0x22, 0x22},
	{0xEE, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0xFF,
				0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}
};

/**************************** Function Definitions ****************************/

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will enable multi-stream transport (MST) mode for the driver.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxMstCfgModeEnable(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	InstancePtr->TxInstance.MstEnable = 1;
}

/******************************************************************************/
/**
 * This function will disable multi-stream transport (MST) mode for the driver.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	When disabled, the driver will behave in single-stream transport
 *		(SST) mode.
 *
*******************************************************************************/
void XDp_TxMstCfgModeDisable(XDp *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	InstancePtr->TxInstance.MstEnable = 0;
}

/******************************************************************************/
/**
 * This function will check if the immediate downstream RX device is capable of
 * multi-stream transport (MST) mode. A DisplayPort Configuration Data (DPCD)
 * version of 1.2 or higher is required and the MST capability bit in the DPCD
 * must be set for this function to return XST_SUCCESS.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxMstCapable(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	if (InstancePtr->Config.MstSupport == 0) {
		return XST_NO_FEATURE;
	}

	/* Check that the RX device has a DisplayPort Configuration Data (DPCD)
	 * version greater than or equal to 1.2 to be able to support MST
	 * functionality. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_REV, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}
	else if (AuxData < 0x12) {
		return XST_NO_FEATURE;
	}

	/* Check if the RX device has MST capabilities.. */
	Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_MSTM_CAP, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX read transaction failed. */
		return Status;
	}
	else if ((AuxData & XDP_DPCD_MST_CAP_MASK) !=
						XDP_DPCD_MST_CAP_MASK) {
		return XST_NO_FEATURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will enable multi-stream transport (MST) mode in both the
 * DisplayPort TX and the immediate downstream RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxMstEnable(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Check if the immediate downstream RX device has MST capabilities. */
	Status = XDp_TxMstCapable(InstancePtr);
	if (Status != XST_SUCCESS) {
		/* The RX device is not downstream capable. */
		return Status;
	}

	/* HPD long pulse used for upstream notification. */
	AuxData = 0;
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_BRANCH_DEVICE_CTRL, 1,
								&AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Enable MST in the immediate branch device and tell it that its
	 * upstream device is a source (the DisplayPort TX). */
	AuxData = XDP_DPCD_UP_IS_SRC_MASK | XDP_DPCD_UP_REQ_EN_MASK |
							XDP_DPCD_MST_EN_MASK;
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_MSTM_CTRL, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Enable MST in the DisplayPort TX. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_MST_CONFIG,
					XDP_TX_MST_CONFIG_MST_EN_MASK);

	XDp_TxMstCfgModeEnable(InstancePtr);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will disable multi-stream transport (MST) mode in both the
 * DisplayPort TX and the immediate downstream RX device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxMstDisable(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Disable MST mode in the immediate branch device. */
	AuxData = 0;
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_MSTM_CTRL, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Disable MST mode in the DisplayPort TX. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_MST_CONFIG, 0x0);

	XDp_TxMstCfgModeDisable(InstancePtr);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will check whether
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream ID to check for enable/disable status.
 *
 * @return
 *		- 1 if the specified stream is enabled.
 *		- 0 if the specified stream is disabled.
 *
 * @note	None.
 *
*******************************************************************************/
u8 XDp_TxMstStreamIsEnabled(XDp *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid((Stream == XDP_TX_STREAM_ID1) ||
						(Stream == XDP_TX_STREAM_ID2) ||
						(Stream == XDP_TX_STREAM_ID3) ||
						(Stream == XDP_TX_STREAM_ID4));

	return InstancePtr->TxInstance.
				MstStreamConfig[Stream - 1].MstStreamEnable;
}

/******************************************************************************/
/**
 * This function will configure the InstancePtr->TxInstance.MstStreamConfig
 * structure to enable the specified stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream ID that will be enabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxMstCfgStreamEnable(XDp *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Stream == XDP_TX_STREAM_ID1) ||
						(Stream == XDP_TX_STREAM_ID2) ||
						(Stream == XDP_TX_STREAM_ID3) ||
						(Stream == XDP_TX_STREAM_ID4));

	InstancePtr->TxInstance.MstStreamConfig[Stream - 1].MstStreamEnable = 1;
}

/******************************************************************************/
/**
 * This function will configure the InstancePtr->TxInstance.MstStreamConfig
 * structure to disable the specified stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream ID that will be disabled.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxMstCfgStreamDisable(XDp *InstancePtr, u8 Stream)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Stream == XDP_TX_STREAM_ID1) ||
						(Stream == XDP_TX_STREAM_ID2) ||
						(Stream == XDP_TX_STREAM_ID3) ||
						(Stream == XDP_TX_STREAM_ID4));

	InstancePtr->TxInstance.MstStreamConfig[Stream - 1].MstStreamEnable = 0;
}

/******************************************************************************/
/**
 * This function will map a stream to a downstream DisplayPort TX device that is
 * associated with a sink from the InstancePtr->TxInstance.Topology.SinkList.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream ID that will be mapped to a DisplayPort
 *		device.
 * @param	SinkNum is the sink ID in the sink list that will be mapped to
 *		the stream.
 *
 * @return	None.
 *
 * @note	The contents of the InstancePtr->TxInstance.
 *		MstStreamConfig[Stream] will be modified.
 * @note	The topology will need to be determined prior to calling this
 *		function using the XDp_TxFindAccessibleDpDevices.
 *
*******************************************************************************/
void XDp_TxSetStreamSelectFromSinkList(XDp *InstancePtr, u8 Stream, u8 SinkNum)
{
	u8 Index;
	XDp_TxMstStream *MstStream;
	XDp_TxTopology *Topology;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Stream == XDP_TX_STREAM_ID1) ||
						(Stream == XDP_TX_STREAM_ID2) ||
						(Stream == XDP_TX_STREAM_ID3) ||
						(Stream == XDP_TX_STREAM_ID4));

	MstStream = &InstancePtr->TxInstance.MstStreamConfig[Stream - 1];
	Topology = &InstancePtr->TxInstance.Topology;

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
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Stream is the stream number that will be mapped to a DisplayPort
 *		device.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the targeted downstream device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the targeted DisplayPort device.
 *
 * @return	None.
 *
 * @note	The contents of the InstancePtr->TxInstance.
 *		MstStreamConfig[Stream] will be modified.
 *
*******************************************************************************/
void XDp_TxSetStreamSinkRad(XDp *InstancePtr, u8 Stream, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u8 Index;
	XDp_TxMstStream *MstStream;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid((Stream == XDP_TX_STREAM_ID1) ||
						(Stream == XDP_TX_STREAM_ID2) ||
						(Stream == XDP_TX_STREAM_ID3) ||
						(Stream == XDP_TX_STREAM_ID4));
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));

	MstStream = &InstancePtr->TxInstance.MstStreamConfig[Stream - 1];

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
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the topology discovery is successful.
 *		- XST_FAILURE otherwise - if sending a LINK_ADDRESS sideband
 *		  message to one of the branch devices in the topology failed.
 *
 * @note	The contents of the InstancePtr->TxInstance.Topology structure
 *		will be modified.
 *
*******************************************************************************/
u32 XDp_TxDiscoverTopology(XDp *InstancePtr)
{
	u8 RelativeAddress[15];

	return XDp_TxFindAccessibleDpDevices(InstancePtr, 1, RelativeAddress);
}

/******************************************************************************/
/**
 * This function will explore the DisplayPort topology of downstream devices
 * starting from the branch device specified by the LinkCountTotal and
 * RelativeAddress parameters. It will recursively go through each branch
 * device, obtain its information by sending a LINK_ADDRESS sideband message,
 * and add this information to the the topology's node table. For each sink
 * device connected to a branch's downstream port, this function will obtain
 * the details of the sink, add it to the topology's node table, as well as
 * add it to the topology's sink list.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the total DisplayPort links connecting the
 *		DisplayPort TX to the current downstream device in the
 *		recursion.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the current target DisplayPort device in the
 *		recursion.
 *
 * @return
 *		- XST_SUCCESS if the topology discovery is successful.
 *		- XST_FAILURE otherwise - if sending a LINK_ADDRESS sideband
 *		  message to one of the branch devices in the topology failed.
 *
 * @note	The contents of the InstancePtr->TxInstance.Topology structure
 *		will be modified.
 *
*******************************************************************************/
u32 XDp_TxFindAccessibleDpDevices(XDp *InstancePtr, u8 LinkCountTotal,
							u8 *RelativeAddress)
{
	u32 Status;
	u8 Index;
	u8 NumDownBranches = 0;
	u8 OverallFailures = 0;
	XDp_TxTopology *Topology;
	XDp_SbMsgLinkAddressReplyPortDetail *PortDetails;
	static XDp_SbMsgLinkAddressReplyDeviceInfo DeviceInfo;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));

	Topology = &InstancePtr->TxInstance.Topology;

	/* Send a LINK_ADDRESS sideband message to the branch device in order to
	 * obtain information on it and its downstream devices. */
	Status = XDp_TxSendSbMsgLinkAddress(InstancePtr, LinkCountTotal,
						RelativeAddress, &DeviceInfo);
	if (Status != XST_SUCCESS) {
		/* The LINK_ADDRESS was sent to a device that cannot reply; exit
		 * from this recursion path. */
		return XST_FAILURE;
	}

	/* Write GUID to the branch device if it doesn't already have one. */
	XDp_TxIssueGuid(InstancePtr, LinkCountTotal, RelativeAddress, Topology,
							DeviceInfo.Guid);

	/* Add the branch device to the topology table. */
	XDp_TxAddBranchToList(InstancePtr, &DeviceInfo, LinkCountTotal,
							RelativeAddress);

	/* Downstream devices will be an extra link away from the source than
	 * this branch device. */
	LinkCountTotal++;

	u8 DownBranchesDownPorts[DeviceInfo.NumPorts];
	for (Index = 0; Index < DeviceInfo.NumPorts; Index++) {
		PortDetails = &DeviceInfo.PortDetails[Index];
		/* Any downstream device downstream device will have the RAD of
		 * the current branch device appended with the port number. */
		RelativeAddress[LinkCountTotal - 2] = PortDetails->PortNum;

		if ((PortDetails->InputPort == 0) &&
					(PortDetails->PeerDeviceType != 0x2) &&
					(PortDetails->DpDevPlugStatus == 1)) {

			if ((PortDetails->MsgCapStatus == 1) &&
					(PortDetails->DpcdRev >= 0x12)) {
				/* Write GUID to the branch device if it
				 * doesn't already have one. */
				XDp_TxIssueGuid(InstancePtr,
					LinkCountTotal, RelativeAddress,
					Topology, PortDetails->Guid);
			}

			XDp_TxAddSinkToList(InstancePtr, PortDetails,
					LinkCountTotal,
					RelativeAddress);
		}

		if (PortDetails->PeerDeviceType == 0x2) {
			DownBranchesDownPorts[NumDownBranches] =
							PortDetails->PortNum;
			NumDownBranches++;
		}
	}

	for (Index = 0; Index < NumDownBranches; Index++) {
		/* Any downstream device downstream device will have the RAD of
		 * the current branch device appended with the port number. */
		RelativeAddress[LinkCountTotal - 2] =
						DownBranchesDownPorts[Index];

		/* Found a branch device; recurse the algorithm to see what
		 * DisplayPort devices are connected to it with the appended
		 * RAD. */
		Status = XDp_TxFindAccessibleDpDevices(InstancePtr,
					LinkCountTotal, RelativeAddress);
		if (Status != XST_SUCCESS) {
			/* Keep trying to discover the topology, but the top
			 * level function call should indicate that a failure
			 * was detected. */
			OverallFailures++;
		}
	}

	if (OverallFailures != 0) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Swap the ordering of the sinks in the topology's sink list. All sink
 * information is preserved in the node table - the swapping takes place only on
 * the pointers to the sinks in the node table. The reason this swapping is done
 * is so that functions that use the sink list will act on the sinks in a
 * different order.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Index0 is the sink list's index of one of the sink pointers to
 *		be swapped.
 * @param	Index1 is the sink list's index of the other sink pointer to be
 *		swapped.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxTopologySwapSinks(XDp *InstancePtr, u8 Index0, u8 Index1)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	XDp_TxTopologyNode *TmpSink =
			InstancePtr->TxInstance.Topology.SinkList[Index0];

	InstancePtr->TxInstance.Topology.SinkList[Index0] =
			InstancePtr->TxInstance.Topology.SinkList[Index1];

	InstancePtr->TxInstance.Topology.SinkList[Index1] = TmpSink;
}

/******************************************************************************/
/**
 * Order the sink list with all sinks of the same tiled display being sorted by
 * 'tile order'. Refer to the XDp_TxGetDispIdTdtTileOrder macro on how to
 * determine the 'tile order'. Sinks of a tiled display will have an index in
 * the sink list that is lower than all indices of other sinks within that same
 * tiled display that have a greater 'tile order'.
 * When operations are done on the sink list, this ordering will ensure that
 * sinks within the same tiled display will be acted upon in a consistent
 * manner - with an incrementing sink list index, sinks with a lower 'tile
 * order' will be acted upon first relative to the other sinks in the same tiled
 * display. Multiple tiled displays may exist in the sink list.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxTopologySortSinksByTiling(XDp *InstancePtr)
{
	u32 Status;
	XDp_TxTopologyNode *CurrSink, *CmpSink;
	u8 CurrIndex, CmpIndex, NewIndex;
	u8 CurrEdidExt[128], CmpEdidExt[128];
	u8 *CurrTdt, *CmpTdt;
	u8 CurrTileOrder, CmpTileOrder;
	u8 SameTileDispCount, SameTileDispNum;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	for (CurrIndex = 0; CurrIndex <
			(InstancePtr->TxInstance.Topology.SinkTotal - 1);
			CurrIndex++) {
		CurrSink = InstancePtr->TxInstance.Topology.SinkList[CurrIndex];

		Status = XDp_TxGetRemoteTiledDisplayDb(InstancePtr, CurrEdidExt,
				CurrSink->LinkCountTotal,
				CurrSink->RelativeAddress, &CurrTdt);
		if (Status != XST_SUCCESS) {
			/* No Tiled Display Topology (TDT) data block exists. */
			continue;
		}

		/* Start by using the tiling parameters of the current sink
		 * index. */
		CurrTileOrder = XDp_TxGetDispIdTdtTileOrder(CurrTdt);
		NewIndex = CurrIndex;
		SameTileDispCount = 1;
		SameTileDispNum	= XDp_TxGetDispIdTdtNumTiles(CurrTdt);

		/* Try to find a sink that is part of the same tiled display,
		 * but has a smaller tile location - the sink with a smallest
		 * tile location should be ordered first in the topology's sink
		 * list. */
		for (CmpIndex = (CurrIndex + 1);
				(CmpIndex <
				InstancePtr->TxInstance.Topology.SinkTotal)
				&& (SameTileDispCount < SameTileDispNum);
				CmpIndex++) {
			CmpSink = InstancePtr->TxInstance.Topology.SinkList[
								CmpIndex];

			Status = XDp_TxGetRemoteTiledDisplayDb(
				InstancePtr, CmpEdidExt,
				CmpSink->LinkCountTotal,
				CmpSink->RelativeAddress, &CmpTdt);
			if (Status != XST_SUCCESS) {
				/* No TDT data block. */
				continue;
			}

			if (!XDp_TxIsSameTileDisplay(CurrTdt, CmpTdt)) {
				/* The sink under comparison does not belong to
				 * the same tiled display. */
				continue;
			}

			/* Keep track of the sink with a tile location that
			 * should be ordered first out of the remaining sinks
			 * that are part of the same tiled display. */
			CmpTileOrder = XDp_TxGetDispIdTdtTileOrder(CmpTdt);
			if (CurrTileOrder > CmpTileOrder) {
				CurrTileOrder = CmpTileOrder;
				NewIndex = CmpIndex;
				SameTileDispCount++;
			}
		}

		/* If required, swap the current sink with the sink that is a
		 * part of the same tiled display, but has a smaller tile
		 * location. */
		if (CurrIndex != NewIndex) {
			XDp_TxTopologySwapSinks(InstancePtr, CurrIndex,
								NewIndex);
		}
	}
}

/******************************************************************************/
/**
 * This function performs a remote DisplayPort Configuration Data (DPCD) read
 * by sending a sideband message. In case message is directed at the RX device
 * connected immediately to the TX, the message is issued over the AUX channel.
 * The read message will be divided into multiple transactions which read a
 * maximum of 16 bytes each.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	DpcdAddress is the starting address to read from the RX device.
 * @param	BytesToRead is the number of bytes to read.
 * @param	ReadData is a pointer to the data buffer that will be filled
 *		with read data.
 *
 * @return
 *		- XST_SUCCESS if the DPCD read has successfully completed (has
 *		  been acknowledged).
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
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
u32 XDp_TxRemoteDpcdRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(ReadData != NULL);

	/* Target RX device is immediately connected to the TX. */
	if (LinkCountTotal == 1) {
		Status = XDp_TxAuxRead(InstancePtr, DpcdAddress, BytesToRead,
								ReadData);
		return Status;
	}

	u32 BytesLeft = BytesToRead;
	u8 CurrBytesToRead;

	/* Send read message in 16 byte chunks. */
	while (BytesLeft > 0) {
		/* Read a maximum of 16 bytes. */
		if (BytesLeft > 16) {
			CurrBytesToRead = 16;
		}
		/* Read the remaining number of bytes as requested. */
		else {
			CurrBytesToRead = BytesLeft;
		}

		/* Send remote DPCD read sideband message. */
		Status = XDp_TxSendSbMsgRemoteDpcdRead(InstancePtr,
			LinkCountTotal, RelativeAddress, DpcdAddress,
			CurrBytesToRead, ReadData);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Previous DPCD read was 16 bytes; prepare for next read. */
		if (BytesLeft > 16) {
			BytesLeft -= 16;
			DpcdAddress += 16;
			ReadData += 16;
		}
		/* Last DPCD read. */
		else {
			BytesLeft = 0;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This function performs a remote DisplayPort Configuration Data (DPCD) write
 * by sending a sideband message. In case message is directed at the RX device
 * connected immediately to the TX, the message is issued over the AUX channel.
 * The write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	DpcdAddress is the starting address to write to the RX device.
 * @param	BytesToWrite is the number of bytes to write.
 * @param	WriteData is a pointer to a buffer which will be used as the
 *		data source for the write.
 *
 * @return
 *		- XST_SUCCESS if the DPCD write has successfully completed (has
 *		  been acknowledged).
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_DATA_LOST if the requested number of BytesToWrite does not
 *		  equal that actually received.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxRemoteDpcdWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(WriteData != NULL);

	/* Target RX device is immediately connected to the TX. */
	if (LinkCountTotal == 1) {
		Status = XDp_TxAuxWrite(InstancePtr, DpcdAddress, BytesToWrite,
								WriteData);
		return Status;
	}

	u32 BytesLeft = BytesToWrite;
	u8 CurrBytesToWrite;

	/* Send write message in 16 byte chunks. */
	while (BytesLeft > 0) {
		/* Write a maximum of 16 bytes. */
		if (BytesLeft > 16) {
			CurrBytesToWrite = 16;
		}
		/* Write the remaining number of bytes as requested. */
		else {
			CurrBytesToWrite = BytesLeft;
		}

		/* Send remote DPCD write sideband message. */
		Status = XDp_TxSendSbMsgRemoteDpcdWrite(InstancePtr,
			LinkCountTotal, RelativeAddress, DpcdAddress,
			CurrBytesToWrite, WriteData);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Previous DPCD write was 16 bytes; prepare for next read. */
		if (BytesLeft > 16) {
			BytesLeft -= 16;
			DpcdAddress += 16;
			WriteData += 16;
		}
		/* Last DPCD write. */
		else {
			BytesLeft = 0;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This function performs a remote I2C read by sending a sideband message. In
 * case message is directed at the RX device connected immediately to the TX,
 * the message is sent over the AUX channel. The read message will be divided
 * into multiple transactions which read a maximum of 16 bytes each. The segment
 * pointer is automatically incremented and the offset is calibrated as needed.
 * E.g. For an overall offset of:
 *	- 128, an I2C read is done on segptr=0; offset=128.
 *	- 256, an I2C read is done on segptr=1; offset=0.
 *	- 384, an I2C read is done on segptr=1; offset=128.
 *	- 512, an I2C read is done on segptr=2; offset=0.
 *	- etc.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	Offset is the offset at the specified address of the targeted
 *		I2C device that the read will start from.
 * @param	BytesToRead is the number of bytes to read.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		I2C read data.
 *
 * @return
 *		- XST_SUCCESS if the I2C read has successfully completed with no
 *		  errors.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
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
u32 XDp_TxRemoteIicRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u16 Offset, u16 BytesToRead,
	u8 *ReadData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(ReadData != NULL);

	/* Target RX device is immediately connected to the TX. */
	if (LinkCountTotal == 1) {
		Status = XDp_TxIicRead(InstancePtr, IicAddress, Offset,
							BytesToRead, ReadData);
		return Status;
	}

	u8 SegPtr;
	u16 NumBytesLeftInSeg;
	u16 BytesLeft = BytesToRead;
	u8 CurrBytesToRead;

	/* Reposition based on a segment length of 256 bytes. */
	SegPtr = 0;
	if (Offset > 255) {
		SegPtr += Offset / 256;
		Offset %= 256;
	}
	NumBytesLeftInSeg = 256 - Offset;

	/* Set the segment pointer to 0. */
	Status = XDp_TxRemoteIicWrite(InstancePtr, LinkCountTotal,
		RelativeAddress, XDP_SEGPTR_ADDR, 1, &SegPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Send I2C read message in 16 byte chunks. */
	while (BytesLeft > 0) {
		/* Read a maximum of 16 bytes. */
		if ((NumBytesLeftInSeg >= 16) && (BytesLeft >= 16)) {
			CurrBytesToRead = 16;
		}
		/* Read the remaining number of bytes as requested. */
		else if (NumBytesLeftInSeg >= BytesLeft) {
			CurrBytesToRead = BytesLeft;
		}
		/* Read the remaining data in the current segment boundary. */
		else {
			CurrBytesToRead = NumBytesLeftInSeg;
		}

		/* Send remote I2C read sideband message. */
		Status = XDp_TxSendSbMsgRemoteIicRead(InstancePtr,
			LinkCountTotal, RelativeAddress, IicAddress, Offset,
			BytesLeft, ReadData);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Previous I2C read was 16 bytes; prepare for next read. */
		if (BytesLeft > CurrBytesToRead) {
			BytesLeft -= CurrBytesToRead;
			Offset += CurrBytesToRead;
			ReadData += CurrBytesToRead;
			NumBytesLeftInSeg -= CurrBytesToRead;
		}
		/* Last I2C read. */
		else {
			BytesLeft = 0;
		}

		/* Increment the segment pointer to access more I2C address
		 * space. */
		if ((NumBytesLeftInSeg == 0) && (BytesLeft > 0)) {
			SegPtr++;
			Offset %= 256;
			NumBytesLeftInSeg = 256;

			Status = XDp_TxRemoteIicWrite(InstancePtr,
				LinkCountTotal, RelativeAddress,
				XDP_SEGPTR_ADDR, 1, &SegPtr);
			if (Status != XST_SUCCESS) {
				return Status;
			}
		}
	}

	/* Reset the segment pointer to 0. */
	SegPtr = 0;
	Status = XDp_TxRemoteIicWrite(InstancePtr, LinkCountTotal,
				RelativeAddress, XDP_SEGPTR_ADDR, 1, &SegPtr);

	return Status;
}

/******************************************************************************/
/**
 * This function performs a remote I2C write by sending a sideband message. In
 * case message is directed at the RX device connected immediately to the TX,
 * the message is sent over the AUX channel.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	IicAddress is the address on the I2C bus of the target device.
 * @param	BytesToWrite is the number of bytes to write.
 * @param	WriteData is a pointer to a buffer which will be used as the
 *		data source for the write.
 *
 * @return
 *		- XST_SUCCESS if the I2C write has successfully completed with
 *		  no errors.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_ERROR_COUNT_MAX if either waiting for a reply, or an AUX
 *		  request timed out.
 *		- XST_DATA_LOST if the requested number of BytesToWrite does not
 *		  equal that actually received.
 *		- XST_FAILURE otherwise - if an AUX read or write transaction
 *		  failed, the header or body CRC of the sideband message did not
 *		  match the calculated value, or the a reply was negative
 *		  acknowledged (NACK'ed).
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_TxRemoteIicWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicAddress, u8 BytesToWrite,
	u8 *WriteData)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(WriteData != NULL);

	/* Target RX device is immediately connected to the TX. */
	if (LinkCountTotal == 1) {
		Status = XDp_TxIicWrite(InstancePtr, IicAddress, BytesToWrite,
								WriteData);
	}
	/* Send remote I2C sideband message. */
	else {
		Status = XDp_TxSendSbMsgRemoteIicWrite(InstancePtr,
			LinkCountTotal, RelativeAddress, IicAddress,
			BytesToWrite, WriteData);
	}

	return Status;
}

/******************************************************************************/
/**
 * This function will allocate bandwidth for all enabled stream.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxAllocatePayloadStreams(XDp *InstancePtr)
{
	u32 Status;
	u8 StreamIndex;
	u8 StartTs = 1;
	XDp_TxMstStream *MstStream;
	XDp_TxMainStreamAttributes *MsaConfig;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	/* Allocate the payload table for each stream in both the DisplayPort TX
	 * and RX device. */
	for (StreamIndex = 0; StreamIndex < XDP_TX_STREAM_ID4; StreamIndex++) {
		if (!XDp_TxMstStreamIsEnabled(InstancePtr,
					StreamIndex + XDP_TX_STREAM_ID1)) {
			continue;
		}
		MsaConfig = &InstancePtr->TxInstance.MsaConfig[StreamIndex];

		Status = XDp_TxAllocatePayloadVcIdTable(InstancePtr,
			StreamIndex + XDP_TX_STREAM_ID1,
			MsaConfig->TransferUnitSize, StartTs);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		StartTs += MsaConfig->TransferUnitSize;
	}

	/* Generate an ACT event. */
	Status = XDp_TxSendActTrigger(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Send ALLOCATE_PAYLOAD request. */
	for (StreamIndex = 0; StreamIndex < XDP_TX_STREAM_ID4; StreamIndex++) {
		if (!XDp_TxMstStreamIsEnabled(InstancePtr,
					StreamIndex + XDP_TX_STREAM_ID1)) {
			continue;
		}
		MstStream =
			&InstancePtr->TxInstance.MstStreamConfig[StreamIndex];

		Status = XDp_TxSendSbMsgAllocatePayload(InstancePtr,
			MstStream->LinkCountTotal, MstStream->RelativeAddress,
			StreamIndex + XDP_TX_STREAM_ID1, MstStream->MstPbn);
		if (Status != XST_SUCCESS) {
			return Status;
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
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	VcId is the unique virtual channel ID to allocate into the
 *		payload ID tables.
 * @param	Ts is the number of timeslots to allocate in the payload ID
 *		tables.
 * @param	StartTs is the starting time slot to allocate the VcId to.
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
u32 XDp_TxAllocatePayloadVcIdTable(XDp *InstancePtr, u8 VcId, u8 Ts, u8 StartTs)
{
	u32 Status;
	u8 AuxData[3];
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(Ts <= 64);

	/* Clear the VC payload ID table updated bit. */
	AuxData[0] = 0x1;
	Status = XDp_TxAuxWrite(InstancePtr,
			XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Check that there are enough time slots available. */
	if ((VcId != 0) && (((63 - StartTs + 1) < Ts) || (StartTs == 0))) {
		/* Payload ID table needs to be cleared to. */
		return XST_BUFFER_TOO_SMALL;
	}

	/* Allocate timeslots in TX. */
	for (Index = StartTs; Index < (StartTs + Ts); Index++) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
			(XDP_TX_VC_PAYLOAD_BUFFER_ADDR + (4 * Index)), VcId);
	}

	XDp_WaitUs(InstancePtr, 1000);

	/* Allocate timeslots in sink. */

	/* Allocate VC with VcId. */
	AuxData[0] = VcId;
	/* Start timeslot for VC with VcId. */
	AuxData[1] = StartTs;
	/* Timeslot count for VC with VcId. */
	if (VcId == 0) {
		AuxData[2] = 0x3F;
	}
	else {
		AuxData[2] = Ts;
	}
	Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_PAYLOAD_ALLOCATE_SET, 3,
								AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	/* Wait for the VC table to be updated. */
	do {
		Status = XDp_TxAuxRead(InstancePtr,
			XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}
	} while ((AuxData[0] & 0x01) != 0x01);

	XDp_WaitUs(InstancePtr, 1000);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will clear the virtual channel payload ID table in both the
 * DisplayPort TX and all downstream DisplayPort devices.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxClearPayloadVcIdTable(XDp *InstancePtr)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	Status = XDp_TxAllocatePayloadVcIdTable(InstancePtr, 0, 64, 0);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Send CLEAR_PAYLOAD_ID_TABLE request. */
	Status = XDp_TxSendSbMsgClearPayloadIdTable(InstancePtr);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_DPCD_WRITE sideband message which will write
 * some data to the specified DisplayPort Configuration Data (DPCD) address of a
 * downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgRemoteDpcdWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToWrite, u8 *WriteData)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToWrite <= 0xFFFFF);
	Xil_AssertNonvoid(WriteData != NULL);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_REMOTE_DPCD_WRITE;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
						4) | (DpcdAddress >> 16);
	Msg.Body.MsgData[2] = (DpcdAddress & 0x0000FF00) >> 8;
	Msg.Body.MsgData[3] = (DpcdAddress & 0x000000FF);
	Msg.Body.MsgData[4] = BytesToWrite;
	for (Index = 0; Index < BytesToWrite; Index++) {
		Msg.Body.MsgData[5 + Index] = WriteData[Index];
	}
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the REMOTE_DPCD_WRITE transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_DPCD_READ sideband message which will read
 * from the specified DisplayPort Configuration Data (DPCD) address of a
 * downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgRemoteDpcdRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u32 DpcdAddress, u32 BytesToRead, u8 *ReadData)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DpcdAddress <= 0xFFFFF);
	Xil_AssertNonvoid(BytesToRead <= 0xFFFFF);
	Xil_AssertNonvoid(ReadData != NULL);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_REMOTE_DPCD_READ;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
						4) | (DpcdAddress >> 16);
	Msg.Body.MsgData[2] = (DpcdAddress & 0x0000FF00) >> 8;
	Msg.Body.MsgData[3] = (DpcdAddress & 0x000000FF);
	Msg.Body.MsgData[4] = BytesToRead;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the REMOTE_DPCD_READ transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);
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
 * This function will send a REMOTE_I2C_WRITE sideband message which will write
 * to the specified I2C address of a downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	IicDeviceId is the address on the I2C bus of the target device.
 * @param	BytesToWrite is the number of bytes to write to the I2C address.
 * @param	WriteData is a pointer to a buffer that will be written.
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
u32 XDp_TxSendSbMsgRemoteIicWrite(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 BytesToWrite, u8 *WriteData)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(WriteData != NULL);

	Msg.FragmentNum = 0;

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = LinkCountTotal - 1;
	for (Index = 0; Index < (Msg.Header.LinkCountTotal - 1); Index++) {
		Msg.Header.RelativeAddress[Index] = RelativeAddress[Index];
	}
	Msg.Header.LinkCountRemaining = Msg.Header.LinkCountTotal - 1;
	Msg.Header.BroadcastMsg = 0;
	Msg.Header.PathMsg = 0;
	Msg.Header.MsgBodyLength = 5 + BytesToWrite;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_REMOTE_I2C_WRITE;
	Msg.Body.MsgData[1] = RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4;
	Msg.Body.MsgData[2] = IicDeviceId; /* Write I2C device ID. */
	Msg.Body.MsgData[3] = BytesToWrite; /* Number of bytes to write. */
	for (Index = 0; Index < BytesToWrite; Index++) {
		Msg.Body.MsgData[Index + 4] = WriteData[Index];
	}
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the REMOTE_I2C_WRITE transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a REMOTE_I2C_READ sideband message which will read
 * from the specified I2C address of a downstream DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	IicDeviceId is the address on the I2C bus of the target device.
 * @param	Offset is the offset at the specified address of the targeted
 *		I2C device that the read will start from.
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
u32 XDp_TxSendSbMsgRemoteIicRead(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, u8 IicDeviceId, u8 Offset, u8 BytesToRead,
	u8 *ReadData)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(ReadData != NULL);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_REMOTE_I2C_READ;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4) | 1;
	Msg.Body.MsgData[2] = IicDeviceId; /* Write I2C device ID. */
	Msg.Body.MsgData[3] = 1; /* Number of bytes to write. */
	Msg.Body.MsgData[4] = Offset;
	Msg.Body.MsgData[5] = (0 << 4) | 0;
	Msg.Body.MsgData[6] = IicDeviceId; /* Read I2C device ID. */
	Msg.Body.MsgData[7] = BytesToRead;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the REMOTE_I2C_READ transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);
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
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgLinkAddress(XDp *InstancePtr, u8 LinkCountTotal,
	u8 *RelativeAddress, XDp_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(DeviceInfo != NULL);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_LINK_ADDRESS;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the LINK_ADDRESS transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}

	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);
	if (Status != XST_SUCCESS) {
		/* Either the reply indicates a NACK, an AUX read or write
		 * transaction failed, there was a time out waiting for a reply,
		 * or a CRC check failed. */
		return Status;
	}
	XDp_TxGetDeviceInfoFromSbMsgLinkAddress(&SbMsgReply, DeviceInfo);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will send an ENUM_PATH_RESOURCES sideband message which will
 * determine the available payload bandwidth number (PBN) for a path to a target
 * device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgEnumPathResources(XDp *InstancePtr, u8 LinkCountTotal,
			u8 *RelativeAddress, u16 *AvailPbn, u16 *FullPbn)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(AvailPbn != NULL);
	Xil_AssertNonvoid(FullPbn != NULL);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_ENUM_PATH_RESOURCES;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4);
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the ENUM_PATH_RESOURCES transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);
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
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgAllocatePayload(XDp *InstancePtr, u8 LinkCountTotal,
					u8 *RelativeAddress, u8 VcId, u16 Pbn)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;
	u8 Index;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(VcId > 0);
	Xil_AssertNonvoid(Pbn > 0);

	Msg.FragmentNum = 0;

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
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_ALLOCATE_PAYLOAD;
	Msg.Body.MsgData[1] = (RelativeAddress[Msg.Header.LinkCountTotal - 1] <<
									4);
	Msg.Body.MsgData[2] = VcId;
	Msg.Body.MsgData[3] = (Pbn >> 8);
	Msg.Body.MsgData[4] = (Pbn & 0xFFFFFFFF);
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the ALLOCATE_PAYLOAD transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will send a CLEAR_PAYLOAD_ID_TABLE sideband message which will
 * de-allocate all virtual channel payload ID tables.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
u32 XDp_TxSendSbMsgClearPayloadIdTable(XDp *InstancePtr)
{
	u32 Status;
	XDp_SidebandMsg Msg;
	XDp_SidebandReply SbMsgReply;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_TX);

	Msg.FragmentNum = 0;

	/* Prepare the sideband message header. */
	Msg.Header.LinkCountTotal = 1;
	Msg.Header.LinkCountRemaining = 6;
	Msg.Header.BroadcastMsg = 1;
	Msg.Header.PathMsg = 1;
	Msg.Header.MsgBodyLength = 2;
	Msg.Header.StartOfMsgTransaction = 1;
	Msg.Header.EndOfMsgTransaction = 1;
	Msg.Header.MsgSequenceNum = 0;
	Msg.Header.Crc = XDp_Crc4CalculateHeader(&Msg.Header);

	/* Prepare the sideband message body. */
	Msg.Body.MsgData[0] = XDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE;
	Msg.Body.MsgDataLength = Msg.Header.MsgBodyLength - 1;
	Msg.Body.Crc = XDp_Crc8CalculateBody(&Msg);

	/* Submit the CLEAR_PAYLOAD_ID_TABLE transaction message request. */
	Status = XDp_SendSbMsgFragment(InstancePtr, &Msg);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction used to send the sideband message
		 * failed. */
		return Status;
	}
	Status = XDp_TxReceiveSbMsg(InstancePtr, &SbMsgReply);

	return Status;
}

/******************************************************************************/
/**
 * This function will write a global unique identifier (GUID) to the target
 * DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target device.
 * @param	Guid is a pointer to the GUID to write to the target device.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_TxWriteGuid(XDp *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u8 *Guid)
{
	u8 AuxData[XDP_GUID_NBYTES];
	u8 Index;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertVoid(Guid != NULL);

	memset(AuxData, 0, XDP_GUID_NBYTES);
	for (Index = 0; Index < XDP_GUID_NBYTES; Index++) {
		AuxData[Index] = Guid[Index];
	}

	XDp_TxRemoteDpcdWrite(InstancePtr, LinkCountTotal, RelativeAddress,
				XDP_DPCD_GUID, XDP_GUID_NBYTES, AuxData);
}

/******************************************************************************/
/**
 * This function will obtain the global unique identifier (GUID) for the target
 * DisplayPort device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
void XDp_TxGetGuid(XDp *InstancePtr, u8 LinkCountTotal, u8 *RelativeAddress,
								u8 *Guid)
{
	u8 Index;
	u8 Data[XDP_GUID_NBYTES];

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_TX);
	Xil_AssertVoid(LinkCountTotal > 0);
	Xil_AssertVoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertVoid(Guid != NULL);

	XDp_TxRemoteDpcdRead(InstancePtr, LinkCountTotal, RelativeAddress,
					XDP_DPCD_GUID, XDP_GUID_NBYTES, Data);

	memset(Guid, 0, XDP_GUID_NBYTES);
	for (Index = 0; Index < XDP_GUID_NBYTES; Index++) {
		Guid[Index] = Data[Index];
	}
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will handle incoming sideband messages. It will
 * 1) Read the contents of the down request registers,
 * 2) Delegate control depending on the request type, and
 * 3) Send a down reply.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return
 *		- XST_SUCCESS if the entire message was sent successfully.
 *		- XST_DEVICE_NOT_FOUND if no device is connected.
 *		- XST_ERROR_COUNT_MAX if sending one of the message fragments
 *		  timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_RxHandleDownReq(XDp *InstancePtr)
{
	XDp_SidebandMsg Msg;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_RX);

	XDp_RxReadDownReq(InstancePtr, &Msg);

	switch (Msg.Body.MsgData[0]) {
	case XDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE:
		XDp_RxSetClearPayloadIdReply(&Msg);
		XDp_RxSetAvailPbn(InstancePtr, &Msg);
		break;

	case XDP_SBMSG_LINK_ADDRESS:
		XDp_RxSetLinkAddressReply(InstancePtr, &Msg);
		break;

	case XDP_SBMSG_REMOTE_I2C_READ:
		XDp_RxSetRemoteIicReadReply(InstancePtr, &Msg);
		break;

	case XDP_SBMSG_REMOTE_DPCD_READ:
		XDp_RxSetRemoteDpcdReadReply(InstancePtr, &Msg);
		break;

	case XDP_SBMSG_ENUM_PATH_RESOURCES:
		XDp_RxSetEnumPathResReply(InstancePtr, &Msg);
		break;

	case XDP_SBMSG_ALLOCATE_PAYLOAD:
		XDp_RxSetAllocPayloadReply(&Msg);
		XDp_RxSetAvailPbn(InstancePtr, &Msg);
		break;

	default:
		XDp_RxSetGenericNackReply(InstancePtr, &Msg);
		break;
	}

	return XDp_RxSendSbMsg(InstancePtr, &Msg);
}

/******************************************************************************/
/**
 * This function returns a pointer to the I2C map entry at the supplied I2C
 * address for the specified port.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number for which to obtain the I2C map entry
 *		for.
 * @param	IicAddress is the I2C address of the map entry.
 *
 * @return
 *		- NULL if no entry exists in the I2C map corresponding to the
 *		  supplied I2C address for the given port.
 *		- Otherwise, a pointer to the I2C map entry with the specified
 *		  I2C address.
 *
 * @note	None.
 *
*******************************************************************************/
XDp_RxIicMapEntry *XDp_RxGetIicMapEntry(XDp *InstancePtr, u8 PortNum,
								u8 IicAddress)
{
	u8 Index;
	XDp_RxIicMapEntry *IicMap;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertNonvoid(PortNum < XDP_MAX_NPORTS);

	IicMap = InstancePtr->RxInstance.Topology.Ports[PortNum].IicMap;

	for (Index = 0; Index < XDP_RX_NUM_I2C_ENTRIES_PER_PORT; Index++) {
		/* Return a pointer to the specified I2C address, or the first
		 * empty slot. */
		if ((IicMap[Index].IicAddress == IicAddress) ||
					(IicMap[Index].IicAddress == 0)) {
			return &IicMap[Index];
		}
	}

	/* No entry with the specified I2C address has been found and there are
	 * no empty slots. */
	return NULL;
}

/******************************************************************************/
/**
 * This function adds an entry into the I2C map for a given port. The user
 * provides a pointer to the data to be used for the specified I2C address.
 * When an upstream device issues a REMOTE_I2C_READ sideband message, this I2C
 * map will be searched for an entry matching the requested I2C address read.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number for which to set the I2C map entry.
 * @param	IicAddress is the I2C address for which to set the data.
 * @param	ReadNumBytes is number of bytes available for reading from the
 *		associated IicAddress.
 * @param	ReadData is a pointer to a user-defined data structure that will
 *		be used as read data when an upstream device issues an I2C read.
 *
 * @return
 *		- XST_SUCCESS if there is an available slot in the I2C map for a
 *		  new entry and the I2C address isn't taken.
 *		- XST_FAILURE, otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDp_RxSetIicMapEntry(XDp *InstancePtr, u8 PortNum, u8 IicAddress,
						u16 ReadNumBytes, u8 *ReadData)
{
	XDp_RxIicMapEntry *IicMapEntry;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertNonvoid(PortNum < XDP_MAX_NPORTS);
	Xil_AssertNonvoid(ReadData != NULL);

	IicMapEntry = XDp_RxGetIicMapEntry(InstancePtr, PortNum, IicAddress);
	if (!IicMapEntry) {
		return XST_FAILURE;
	}

	/* Definition at specified IicAddress exists. */
	IicMapEntry->IicAddress = IicAddress;
	IicMapEntry->WriteVal = 0;
	IicMapEntry->ReadNumBytes = ReadNumBytes;
	IicMapEntry->ReadData = ReadData;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function specified the DPCD address space for a given port. The user
 * provides a pointer to the data to be used. When an upstream device issues a
 * REMOTE_DPCD_READ sideband message, the contents of this DPCD structure will
 * be used as the reply's data.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number for which to set the DPCD.
 * @param	StartAddr is the starting address for which to define the DPCD.
 * @param	NumBytes is the total number of bytes defined by the DPCD.
 * @param	DpcdMap is a pointer to a user-defined data structure that will
 *		be used as read data when an upstream device issues a DPCD read.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxSetDpcdMap(XDp *InstancePtr, u8 PortNum, u32 StartAddr, u32 NumBytes,
								u8 *DpcdMap)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(PortNum < XDP_MAX_NPORTS);
	Xil_AssertVoid(DpcdMap != NULL);

	InstancePtr->RxInstance.Topology.Ports[PortNum].DpcdMap.DataPtr =
								DpcdMap;
	InstancePtr->RxInstance.Topology.Ports[PortNum].DpcdMap.NumBytes =
								NumBytes;
	InstancePtr->RxInstance.Topology.Ports[PortNum].DpcdMap.StartAddr =
								StartAddr;
}

/******************************************************************************/
/**
 * This function allows the user to select which ports will be exposed when
 * replying to a LINK_ADDRESS sideband message. The number of ports will also
 * be set.
 * When an upstream device sends a LINK_ADDRESS sideband message, the RX will
 * respond by forming a reply message containing port information for directly
 * connected ports.
 * If exposed, this information will be provided in the LINK_ADDRESS reply.
 * Otherwise, the LINK_ADDRESS reply will not contain this information, hiding
 * the port from the TX.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number to enable or disable exposure.
 * @param	Expose will expose the port at the specified PortNum as part of
 *		the LINK_ADDRESS reply when set to 1. Hidden otherwise.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxMstExposePort(XDp *InstancePtr, u8 PortNum, u8 Expose)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(PortNum < XDP_MAX_NPORTS);
	Xil_AssertVoid((Expose == 0) || (Expose == 1));

	InstancePtr->RxInstance.Topology.Ports[PortNum].Exposed = Expose;

	if (Expose) {
		InstancePtr->RxInstance.Topology.LinkAddressInfo.NumPorts++;
	}
	else if (InstancePtr->RxInstance.Topology.LinkAddressInfo.NumPorts) {
		InstancePtr->RxInstance.Topology.LinkAddressInfo.NumPorts--;
	}

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_SINK_COUNT,
		InstancePtr->RxInstance.Topology.LinkAddressInfo.NumPorts);
}

/******************************************************************************/
/**
 * This function sets the port information that is contained in the driver
 * instance structure for the specified port number, to be copied from the
 * supplied port details structure.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number to set the port details for.
 * @param	PortDetails is a pointer to the user-defined port structure,
 *		whose information is to be copied into the driver instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxMstSetPort(XDp *InstancePtr, u8 PortNum,
			XDp_SbMsgLinkAddressReplyPortDetail *PortDetails)
{
	XDp_SbMsgLinkAddressReplyPortDetail *Port;
	u8 GuidIndex;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(PortNum < XDP_MAX_NPORTS);
	Xil_AssertVoid(PortDetails != NULL);

	Port = &InstancePtr->RxInstance.Topology.LinkAddressInfo.
							PortDetails[PortNum];

	/* Keep internal port number the same as the index. */
	Port->PortNum = PortNum;

	/* Copy port details into the structure. */
	Port->InputPort = PortDetails->InputPort;
	Port->PeerDeviceType = PortDetails->PeerDeviceType;
	Port->MsgCapStatus = PortDetails->MsgCapStatus;
	Port->DpDevPlugStatus = PortDetails->DpDevPlugStatus;
	Port->LegacyDevPlugStatus = PortDetails->LegacyDevPlugStatus;
	Port->DpcdRev = PortDetails->DpcdRev;
	for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
		Port->Guid[GuidIndex] = PortDetails->Guid[GuidIndex];
	}
	Port->NumSdpStreams = PortDetails->NumSdpStreams;
	Port->NumSdpStreamSinks = PortDetails->NumSdpStreamSinks;
}

/******************************************************************************/
/**
 * This function, for an input port, sets the port information that is contained
 * in the driver instance structure for the specified port number.
 * Some default values will be used if no port structure is supplied.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number to set the input port for.
 * @param	PortOverride is a pointer to the user-defined port structure,
 *		whose information is to be copied into the driver instance. If
 *		set to NULL, default values for the input port will be used.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxMstSetInputPort(XDp *InstancePtr, u8 PortNum,
			XDp_SbMsgLinkAddressReplyPortDetail *PortOverride)
{
	XDp_SbMsgLinkAddressReplyDeviceInfo *Branch;
	XDp_SbMsgLinkAddressReplyPortDetail *Port;
	u8 GuidIndex;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(PortNum < XDP_MAX_NPORTS);

	Branch = &InstancePtr->RxInstance.Topology.LinkAddressInfo;
	Port = &Branch->PortDetails[PortNum];

	if (!PortOverride) {
		/* Use default values. */
		Port->InputPort = 1;
		Port->PeerDeviceType = 0x1;
		Port->PortNum = PortNum;
		Port->MsgCapStatus = 1;
		Port->DpDevPlugStatus = 1;
		for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
			Branch->Guid[GuidIndex] = GuidTable[0][GuidIndex];
		}
	}
	else {
		XDp_RxMstSetPort(InstancePtr, PortNum, PortOverride);
		for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
			Branch->Guid[GuidIndex] = PortOverride->Guid[GuidIndex];
		}
	}

	XDp_RxMstExposePort(InstancePtr, PortNum, 1);
}

/******************************************************************************/
/**
 * This function will set the available payload bandwidth number (PBN) of the
 * specified port that is available for allocation, and the full PBN that the
 * port is capable of using.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	PortNum is the port number to set the PBN values for.
 * @param	PbnVal is the value to set the port's available and full PBN to.
 *
 * @return	None.
 *
 * @note	The available PBN is set to 100% of the full PBN.
 *
*******************************************************************************/
void XDp_RxMstSetPbn(XDp *InstancePtr, u8 PortNum, u16 PbnVal)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(XDp_GetCoreType(InstancePtr) == XDP_RX);
	Xil_AssertVoid(PortNum < XDP_MAX_NPORTS);

	InstancePtr->RxInstance.Topology.Ports[PortNum].FullPbn = PbnVal;
	InstancePtr->RxInstance.Topology.Ports[PortNum].AvailPbn = PbnVal;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to a LINK_ADDRESS down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxSetLinkAddressReply(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 0;
	Msg->Header.PathMsg = 0;
	Msg->Header.MsgHeaderLength = 3;

	XDp_RxDeviceInfoToRawData(InstancePtr, Msg);
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to a CLEAR_PAYLOAD down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxSetClearPayloadIdReply(XDp_SidebandMsg *Msg)
{
	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 1;
	Msg->Header.PathMsg = 1;
	Msg->Header.MsgBodyLength = 2;
	Msg->Header.MsgHeaderLength = 3;

	Msg->Body.MsgData[0] = XDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE;
	Msg->Body.MsgDataLength = 1;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to an ALLOCATE_PAYLOAD down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxSetAllocPayloadReply(XDp_SidebandMsg *Msg)
{
	u8 ReplyIndex = 0;
	u8 PortNum;
	u8 VcId;
	u16 Pbn;

	PortNum = Msg->Body.MsgData[1] >> 4;
	VcId = Msg->Body.MsgData[2];
	Pbn = (Msg->Body.MsgData[3] << 8) | Msg->Body.MsgData[4];

	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 0;
	Msg->Header.PathMsg = 0;
	Msg->Header.MsgHeaderLength = 3;

	Msg->Body.MsgData[ReplyIndex++] = XDP_SBMSG_ALLOCATE_PAYLOAD;
	Msg->Body.MsgData[ReplyIndex++] = PortNum << 4;
	Msg->Body.MsgData[ReplyIndex++] = VcId;
	Msg->Body.MsgData[ReplyIndex++] = Pbn >> 8;
	Msg->Body.MsgData[ReplyIndex++] = Pbn & 0xFF;

	Msg->Body.MsgDataLength = ReplyIndex;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to an ENUMERATE_PATH_RESOURCES down request.
 *
 * @param       Msg is a pointer to the message to be formatted.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
static void XDp_RxSetEnumPathResReply(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
        u8 ReplyIndex = 0;
        u8 PortNum;

        Msg->Header.LinkCountTotal = 1;
        Msg->Header.LinkCountRemaining = 0;
        Msg->Header.BroadcastMsg = 0;
        Msg->Header.PathMsg = 0;
        Msg->Header.MsgHeaderLength = 3;

        Msg->Body.MsgData[ReplyIndex++] = XDP_SBMSG_ENUM_PATH_RESOURCES;

	PortNum = Msg->Body.MsgData[ReplyIndex++] >> 4;
        Msg->Body.MsgData[ReplyIndex++] =
                InstancePtr->RxInstance.Topology.Ports[PortNum].FullPbn >> 8;
        Msg->Body.MsgData[ReplyIndex++] =
                InstancePtr->RxInstance.Topology.Ports[PortNum].FullPbn & 0xFF;
        Msg->Body.MsgData[ReplyIndex++] =
                InstancePtr->RxInstance.Topology.Ports[PortNum].AvailPbn >> 8;
        Msg->Body.MsgData[ReplyIndex++] =
                InstancePtr->RxInstance.Topology.Ports[PortNum].AvailPbn & 0xFF;

        Msg->Body.MsgDataLength = ReplyIndex;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to a REMOTE_DPCD_READ down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxSetRemoteDpcdReadReply(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u8 NumReadBytes;
	u8 ReplyIndex = 0;
	u8 Index;
	u32 DpcdReadIndex;
	u8 PortNum;
	u32 DpcdReadAddress;
	XDp_RxDpcdMap *DpcdMap;
	u32 DpcdMapEndAddr;

	PortNum = Msg->Body.MsgData[1] >> 4;
	DpcdReadAddress = ((Msg->Body.MsgData[1] & 0xF) << 16) |
			(Msg->Body.MsgData[2] << 8) | Msg->Body.MsgData[3];
	NumReadBytes = Msg->Body.MsgData[4];

	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 0;
	Msg->Header.PathMsg = 0;
	Msg->Header.MsgHeaderLength = 3;

	Msg->Body.MsgData[ReplyIndex++] = XDP_SBMSG_REMOTE_DPCD_READ;
	Msg->Body.MsgData[ReplyIndex++] = PortNum;
	Msg->Body.MsgData[ReplyIndex++] = NumReadBytes;

	DpcdMap = &InstancePtr->RxInstance.Topology.Ports[PortNum].DpcdMap;

	if (!DpcdMap->DataPtr) {
		/* Supply garbage data if the targeted port has no associated
		 * DPCD map. */
		memset(&Msg->Body.MsgData[ReplyIndex], 0x00, NumReadBytes);
			Msg->Body.MsgDataLength = ReplyIndex + NumReadBytes;
		return XST_FAILURE;
	}
	DpcdMapEndAddr = (DpcdMap->StartAddr + DpcdMap->NumBytes - 1);

	for (Index = 0; Index < NumReadBytes; Index++) {
		DpcdReadIndex = DpcdReadAddress + Index;

		if ((DpcdReadIndex >= DpcdMap->StartAddr) &&
					(DpcdReadIndex <= DpcdMapEndAddr)) {
			Msg->Body.MsgData[ReplyIndex++] = DpcdMap->DataPtr[
					DpcdReadIndex - DpcdMap->StartAddr];
		}
		else {
			/* Supply garbage data if the read is out of range. */
			Msg->Body.MsgData[ReplyIndex++] = 0x00;
		}
	}

	Msg->Body.MsgDataLength = ReplyIndex;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to a REMOTE_I2C_READ down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxSetRemoteIicReadReply(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u8 Index;
	u8 RequestIndex;
	u8 PortNum;
	u8 ReadIicAddr;
	u8 ReadNumBytes;
	u8 WriteIicAddr;
	u8 WriteNumBytes;
	u8 NumIicWriteTransactions;
	XDp_RxIicMapEntry *MyIicMapEntry;

	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 0;
	Msg->Header.PathMsg = 0;
	Msg->Header.MsgHeaderLength = 3;

	NumIicWriteTransactions = Msg->Body.MsgData[1] & 0x03;

	RequestIndex = 1;
	PortNum = Msg->Body.MsgData[RequestIndex++] >> 4;

	for (Index = 0; Index < NumIicWriteTransactions; Index++) {
		WriteIicAddr = Msg->Body.MsgData[RequestIndex++] & 0x7F;
		WriteNumBytes = Msg->Body.MsgData[RequestIndex++];

		MyIicMapEntry = XDp_RxGetIicMapEntry(InstancePtr, PortNum,
								WriteIicAddr);
		if (!MyIicMapEntry || !MyIicMapEntry->IicAddress) {
			/* There is no I2C entry defined with the specified I2C
			 * address in the port's I2C map. */
			return XST_FAILURE;
		}
		else if (WriteNumBytes == 1) {
			/* The driver only supports 1 byte writes to an I2C
			 * address. Otherwise, ignore the write. */
			MyIicMapEntry->WriteVal =
						Msg->Body.MsgData[RequestIndex];
		}
		RequestIndex += WriteNumBytes + 1;
	}

	ReadIicAddr = Msg->Body.MsgData[RequestIndex++];
	ReadNumBytes = Msg->Body.MsgData[RequestIndex];

	MyIicMapEntry = XDp_RxGetIicMapEntry(InstancePtr, PortNum, ReadIicAddr);
	if (!MyIicMapEntry || !MyIicMapEntry->IicAddress) {
		/* There is no I2C entry defined with the specified I2C address
		 * in the port's I2C map. */
		return XST_FAILURE;
	}

	Msg->Body.MsgData[0] = XDP_SBMSG_REMOTE_I2C_READ;
	Msg->Body.MsgData[1] = PortNum;
	Msg->Body.MsgData[2] = ReadNumBytes;
	for (Index = 0; Index < ReadNumBytes; Index++) {
		if ((Index + MyIicMapEntry->WriteVal) <
						MyIicMapEntry->ReadNumBytes) {
			Msg->Body.MsgData[3 + Index] = MyIicMapEntry->ReadData[
					Index + MyIicMapEntry->WriteVal];
		}
		else {
			/* Supply garbage data if the read is out of range. */
			Msg->Body.MsgData[3 + Index] = 0x00;
		}
	}
	Msg->Body.MsgDataLength = 3 + Index;
	Msg->Header.MsgBodyLength = Msg->Body.MsgDataLength + 1;

	return XST_SUCCESS;
}


/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * with a NACK.
 *
 * @param       Msg is a pointer to the message to be formatted.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
static void XDp_RxSetGenericNackReply(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u8 ReplyIndex = 0;
	u8 GuidIndex;

	Msg->Header.LinkCountTotal = 1;
	Msg->Header.LinkCountRemaining = 0;
	Msg->Header.BroadcastMsg = 0;
	Msg->Header.PathMsg = 0;
	Msg->Header.MsgHeaderLength = 3;

	/* Reply type for NACK. */
	Msg->Body.MsgData[ReplyIndex++] |= (1 << 7);
	/* 16 bytes of GUID. */
	for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
		Msg->Body.MsgData[ReplyIndex++] = InstancePtr->RxInstance.
				Topology.LinkAddressInfo.Guid[GuidIndex];
	}
	/* Reason for NACK. */
	Msg->Body.MsgData[ReplyIndex++] = XDP_SBMSG_NAK_REASON_WRITE_FAILURE;
	/* NACK Data */
	Msg->Body.MsgData[ReplyIndex++] = 0x00;

	Msg->Body.MsgDataLength = ReplyIndex;
}

/******************************************************************************/
/**
 * This function will set and format a sideband message structure for replying
 * to a REMOTE_I2C_READ down request.
 *
 * @param	Msg is a pointer to the message to be formatted.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxDeviceInfoToRawData(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u8 ReplyIndex = 0;
	u8 PortIndex;
	u8 GuidIndex;
	XDp_RxTopology *Topology;
	XDp_SbMsgLinkAddressReplyPortDetail *PortDetails;

	Topology = &InstancePtr->RxInstance.Topology;

	/* Determine the device information from the sideband message reply
	* structure. */

	Msg->Body.MsgData[ReplyIndex] = XDP_SBMSG_LINK_ADDRESS;
	ReplyIndex++;

	for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
		Msg->Body.MsgData[ReplyIndex++] =
				Topology->LinkAddressInfo.Guid[GuidIndex];
	}

	Msg->Body.MsgData[ReplyIndex++] = Topology->LinkAddressInfo.NumPorts;

	/* For each port of the current device, obtain the details. */
	for (PortIndex = 0; PortIndex < XDP_MAX_NPORTS; PortIndex++) {
		if (!Topology->Ports[PortIndex].Exposed) {
			/* Current port is not exposed. */
			continue;
		}

		PortDetails = &Topology->LinkAddressInfo.PortDetails[PortIndex];

		Msg->Body.MsgData[ReplyIndex] = (PortDetails->InputPort << 7);
		Msg->Body.MsgData[ReplyIndex] |=
				((PortDetails->PeerDeviceType & 0x07) << 4);
		Msg->Body.MsgData[ReplyIndex++] |=
				(PortDetails->PortNum & 0x0F);
		Msg->Body.MsgData[ReplyIndex] =
				(PortDetails->MsgCapStatus << 7);
		Msg->Body.MsgData[ReplyIndex] |=
				((PortDetails->DpDevPlugStatus & 0x01) << 6);

		if (PortDetails->InputPort) {
			/* Input port does not carry more information. */
			ReplyIndex++;
			continue;
		}

		/* Get the port details of the downstream device. */
		Msg->Body.MsgData[ReplyIndex++] |=
			((PortDetails->LegacyDevPlugStatus & 0x01) << 5);
		Msg->Body.MsgData[ReplyIndex++] = PortDetails->DpcdRev;

		for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
			Msg->Body.MsgData[ReplyIndex++] =
						PortDetails->Guid[GuidIndex];
		}

		Msg->Body.MsgData[ReplyIndex] =
					(PortDetails->NumSdpStreams << 4);
		Msg->Body.MsgData[ReplyIndex++] |=
					(PortDetails->NumSdpStreamSinks & 0x0F);
	}

	Msg->Body.MsgDataLength = ReplyIndex;
}

/******************************************************************************/
/**
 * This function will set the virtual channel payload table both in software and
 * in the DisplayPort RX core's hardware registers based on the MST allocation
 * values from ALLOCATE_PAYLOAD and CLEAR_PAYLOAD sideband message requests.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
void XDp_RxAllocatePayloadStream(XDp *InstancePtr)
{
	u8 Index;
	u8 IndexTsEnd = 0;
        u8 *PayloadTable;
        u32 RegVal;
        u8 StreamId;
        u8 StartTs;
        u8 NumTs;
        PayloadTable = &InstancePtr->RxInstance.Topology.PayloadTable[0];
        RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_MST_ALLOC);
        StreamId = (RegVal & XDP_RX_MST_ALLOC_VCP_ID_MASK);
        StartTs = (RegVal & XDP_RX_MST_ALLOC_START_TS_MASK) >>
		XDP_RX_MST_ALLOC_START_TS_SHIFT;
        NumTs = (RegVal & XDP_RX_MST_ALLOC_COUNT_TS_MASK) >>
		XDP_RX_MST_ALLOC_COUNT_TS_SHIFT;
        /* Set the virtual channel payload table in software using the
	 * MST allocation values. */
	memset(&PayloadTable[StartTs], StreamId, NumTs);
        if ((StreamId == 0) && (NumTs == 63)) {
		/* CLEAR_PAYLOAD request. */
                PayloadTable[63] = 0;
        }
        for (Index = 0; Index < 64; Index++) {
		if ((NumTs == 0) && (PayloadTable[Index] == StreamId)) {
			/* ALLOCATE_PAYLOAD, stream deletion. */
                        PayloadTable[Index] = 0;
                        IndexTsEnd = Index-1;
                }
                /* Write payload table as configured in software
		 to hardware. */
                XDp_WriteReg(InstancePtr->Config.BaseAddr,
                XDP_RX_VC_PAYLOAD_TABLE + (Index * 4), PayloadTable[Index]);
        }
        /* Adjust timeslots after initial streams are deallocated*/
        if(NumTs == 0) {
		for (Index = StartTs; Index < 64; Index++) {
			if(Index+StartTs+IndexTsEnd+2 < 64) {
				/* ALLOCATE_PAYLOAD, stream deletion. */
                                PayloadTable[Index] =
					PayloadTable[Index-StartTs+IndexTsEnd+2];
                               /* Write payload table as configured in
				* software to hardware. */
                               XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_VC_PAYLOAD_TABLE + (Index * 4),
				 PayloadTable[(Index-StartTs)+IndexTsEnd+2]);
                        }
                        else {
				PayloadTable[Index] = 0x00;
                                /* Clear up end timeslots */
                                XDp_WriteReg(InstancePtr->Config.BaseAddr,
					XDP_RX_VC_PAYLOAD_TABLE + (Index * 4),
					0x00);
                        }
		}
       }
       /* Indicate that the virtual channel payload table has been updated. */
       RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr, XDP_RX_MST_CAP);
       RegVal |= XDP_RX_MST_CAP_VCP_UPDATE_MASK;
       XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_MST_CAP, RegVal);
}

/******************************************************************************/
/**
 * This function will set the available and full payload bandwidth numbers (PBN)
 * based on CLEAR_PAYLOAD and ALLOCATE_PAYLOAD sideband messages.
 *
 * @param       InstancePtr is a pointer to the XDp instance.
 * @param       Msg is a pointer to the structure holding the CLEAR_PAYLOAD or
 *              ALLOCATE_PAYLOAD sideband message.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
static void XDp_RxSetAvailPbn(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
        u8 Index;
        u8 PortNum;
        u16 PbnReq;
        XDp_RxTopology *Topology;

        Topology = &InstancePtr->RxInstance.Topology;

        if (Msg->Body.MsgData[0] == XDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE) {
                for (Index = 0; Index < XDP_MAX_NPORTS; Index++) {
                        Topology->Ports[Index].AvailPbn =
                                                Topology->Ports[Index].FullPbn;
                }
        }
        else if (Msg->Body.MsgData[0] == XDP_SBMSG_ALLOCATE_PAYLOAD) {
                PortNum = Msg->Body.MsgData[1] >> 4;
                PbnReq = (Msg->Body.MsgData[3] << 8) | Msg->Body.MsgData[4];

                if (PbnReq) {
                        Topology->Ports[PortNum].AvailPbn = 0;
                }
                else {
                        Topology->Ports[PortNum].AvailPbn =
                                        Topology->Ports[PortNum].FullPbn;
                }
        }
}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will check whether or not a DisplayPort device has a global
 * unique identifier (GUID). If it doesn't (the GUID is all zeros), then it will
 * issue one.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
static void XDp_TxIssueGuid(XDp *InstancePtr, u8 LinkCountTotal,
		u8 *RelativeAddress, XDp_TxTopology *Topology, u8 *Guid)
{
	u8 GuidIndex;

	for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
		if (Guid[GuidIndex]) {
			return;
		}
	}
	/* The current GUID is all 0's; issue a GUID to the device. */
	XDp_TxWriteGuid(InstancePtr, LinkCountTotal, RelativeAddress,
						GuidTable[Topology->NodeTotal]);

	for (GuidIndex = 0; GuidIndex < XDP_GUID_NBYTES; GuidIndex++) {
		Guid[GuidIndex] = GuidTable[Topology->NodeTotal][GuidIndex];
	}
}

/******************************************************************************/
/**
 * This function will copy the branch device's information into the topology's
 * node table.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
static void XDp_TxAddBranchToList(XDp *InstancePtr,
			XDp_SbMsgLinkAddressReplyDeviceInfo *DeviceInfo,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;
	XDp_TxTopologyNode *TopologyNode;

	/* Add this node to the topology's node list. */
	TopologyNode = &InstancePtr->TxInstance.Topology.NodeTable[
				InstancePtr->TxInstance.Topology.NodeTotal];

	for (Index = 0; Index < XDP_GUID_NBYTES; Index++) {
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
	InstancePtr->TxInstance.Topology.NodeTotal++;
}

/******************************************************************************/
/**
 * This function will copy the sink device's information into the topology's
 * node table and also add this entry into the topology's sink list.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
static void XDp_TxAddSinkToList(XDp *InstancePtr,
			XDp_SbMsgLinkAddressReplyPortDetail *SinkDevice,
			u8 LinkCountTotal, u8 *RelativeAddress)
{
	u8 Index;
	XDp_TxTopology *Topology = &InstancePtr->TxInstance.Topology;
	XDp_TxTopologyNode *TopologyNode;

	/* Add this node to the topology's node list. */
	TopologyNode = &Topology->NodeTable[Topology->NodeTotal];

	/* Copy the GUID of the sink for the new entry in the topology node
	 * table. */
	for (Index = 0; Index < XDP_GUID_NBYTES; Index++) {
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
static void XDp_TxGetDeviceInfoFromSbMsgLinkAddress(XDp_SidebandReply
		*SbReply, XDp_SbMsgLinkAddressReplyDeviceInfo *FormatReply)
{
	u8 ReplyIndex = 1;
	u8 Index, Index2;
	XDp_SbMsgLinkAddressReplyPortDetail *PortDetails;

	/* Determine the device information from the sideband message reply
	 * structure. */

	memset(FormatReply->Guid, 0, XDP_GUID_NBYTES);
	for (Index = 0; Index < XDP_GUID_NBYTES; Index++) {
		FormatReply->Guid[Index] = SbReply->Data[ReplyIndex++];
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

			memset(PortDetails->Guid, 0, XDP_GUID_NBYTES);
			for (Index2 = 0; Index2 < XDP_GUID_NBYTES; Index2++) {
				PortDetails->Guid[Index] =
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
 * This function will send a sideband message by creating a data array from the
 * supplied sideband message structure and submitting an AUX write transaction.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
static u32 XDp_TxSendActTrigger(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;
	u8 TimeoutCount = 0;

	XDp_WaitUs(InstancePtr, 10000);

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_MST_CONFIG, 0x3);

	do {
		Status = XDp_TxAuxRead(InstancePtr,
			XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, &AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Error out if timed out. */
		if (TimeoutCount > XDP_TX_VCP_TABLE_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		TimeoutCount++;
		XDp_WaitUs(InstancePtr, 1000);
	} while ((AuxData & 0x02) != 0x02);

	/* Clear the ACT event received bit. */
	AuxData = 0x2;
	Status = XDp_TxAuxWrite(InstancePtr,
			XDP_DPCD_PAYLOAD_TABLE_UPDATE_STATUS, 1, &AuxData);
	if (Status != XST_SUCCESS) {
		/* The AUX write transaction failed. */
		return Status;
	}

	return XST_SUCCESS;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */


/******************************************************************************/
/**
 * Operating in TX mode, this function will send a sideband message by creating
 * a data array from the supplied sideband message structure and submitting an
 * AUX write transaction.
 * In RX mode, the data array will be written as a down reply.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Msg is a pointer to the sideband message structure that holds
 *		the contents of the data to be submitted.
 *
 * @return
 *		- XST_SUCCESS if the write transaction used to transmit the
 *		  sideband message was successful.
 *		- XST_DEVICE_NOT_FOUND if no device is connected.
 *		- XST_ERROR_COUNT_MAX if the request timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_SendSbMsgFragment(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u32 Status;
	u8 Data[XDP_MAX_LENGTH_SBMSG];
	XDp_SidebandMsgHeader *Header = &Msg->Header;
	XDp_SidebandMsgBody *Body = &Msg->Body;
	u8 FragmentOffset;
	u8 Index;

	XDp_WaitUs(InstancePtr, InstancePtr->TxInstance.SbMsgDelayUs);

#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		/* First, clear the DOWN_REP_MSG_RDY in case the RX device is in
		 * a weird state. */
		Data[0] = 0x10;
		Status = XDp_TxAuxWrite(InstancePtr,
				XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0, 1,
				Data);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

	/* Add the header to the sideband message transaction. */
	Msg->Header.MsgHeaderLength = 0;
	Data[Msg->Header.MsgHeaderLength++] =
					(Msg->Header.LinkCountTotal << 4) |
					Msg->Header.LinkCountRemaining;
	for (Index = 0; Index < (Header->LinkCountTotal - 1); Index += 2) {
		Data[Header->MsgHeaderLength] =
					(Header->RelativeAddress[Index] << 4);

		if ((Index + 1) < (Header->LinkCountTotal - 1)) {
			Data[Header->MsgHeaderLength] |=
					Header->RelativeAddress[Index + 1];
		}
		/* Else, the lower (4-bit) nibble is all zeros (for
		 * byte-alignment). */

		Header->MsgHeaderLength++;
	}
	Data[Header->MsgHeaderLength++] = (Header->BroadcastMsg << 7) |
				(Header->PathMsg << 6) | Header->MsgBodyLength;
	Data[Header->MsgHeaderLength++] = (Header->StartOfMsgTransaction << 7) |
				(Header->EndOfMsgTransaction << 6) |
				(Header->MsgSequenceNum << 4) | Header->Crc;

	/* Add the body to the transaction. */
	FragmentOffset = (Msg->FragmentNum * (XDP_MAX_LENGTH_SBMSG -
					Msg->Header.MsgHeaderLength - 1));
	for (Index = 0; Index < Body->MsgDataLength; Index++) {
		Data[Index + Header->MsgHeaderLength] =
					Body->MsgData[Index + FragmentOffset];
	}
	Data[Index + Header->MsgHeaderLength] = Body->Crc;

	/* Submit the message. */
#if XPAR_XDPTXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_TX) {
		Status = XDp_TxAuxWrite(InstancePtr, XDP_DPCD_DOWN_REQ,
			Msg->Header.MsgHeaderLength + Msg->Header.MsgBodyLength,
			Data);
	} else
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */
#if XPAR_XDPRXSS_NUM_INSTANCES
	if (XDp_GetCoreType(InstancePtr) == XDP_RX) {
		Status = XDp_RxWriteRawDownReply(InstancePtr, Data,
						Msg->Header.MsgHeaderLength +
						Msg->Header.MsgBodyLength);
	}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */
	{
		/* Nothing. */
	}

	return Status;
}

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will read the raw sideband message down request and format it
 * into the message structure.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Msg is a pointer to the message structure to be filled with the
 *		read data.
 *
 * @return	None.
 *
 * @note	None.
 *
*******************************************************************************/
static void XDp_RxReadDownReq(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u8 Data[XDP_MAX_LENGTH_SBMSG];
	u8 Index;

	for (Index = 0; Index < XDP_MAX_LENGTH_SBMSG; Index++) {
		Data[Index] = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_DOWN_REQ + (Index * 4));
	}

	Msg->FragmentNum = 0;
	XDp_Transaction2MsgFormat(Data, Msg);
}
#endif /* XPAR_XDPrXSS_NUM_INSTANCES */

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will wait for a sideband message reply and fill in the SbReply
 * structure with the reply data for use by higher-level functions.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
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
static u32 XDp_TxReceiveSbMsg(XDp *InstancePtr, XDp_SidebandReply *SbReply)
{
	u32 Status;
	u8 Index = 0;
	u8 AuxData[80];
	XDp_SidebandMsg Msg;

	Msg.FragmentNum = 0;
	SbReply->Length = 0;

	do {
		XDp_WaitUs(InstancePtr, InstancePtr->TxInstance.SbMsgDelayUs);

		/* Wait for a reply. */
		Status = XDp_TxWaitSbReply(InstancePtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Receive reply. */
		Status = XDp_TxAuxRead(InstancePtr, XDP_DPCD_DOWN_REP, 80,
								AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Convert the reply transaction into XDp_SidebandReply
		 * format. */
		Status = XDp_Transaction2MsgFormat(AuxData, &Msg);
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
		Status = XDp_TxAuxWrite(InstancePtr,
				XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
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
 * @param	InstancePtr is a pointer to the XDp instance.
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
static u32 XDp_TxWaitSbReply(XDp *InstancePtr)
{
	u32 Status;
	u8 AuxData;
	u16 TimeoutCount = 0;

	do {
		Status = XDp_TxAuxRead(InstancePtr,
				XDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, &AuxData);
		if (Status != XST_SUCCESS) {
			/* The AUX read transaction failed. */
			return Status;
		}

		/* Error out if timed out. */
		if (TimeoutCount > XDP_TX_MAX_SBMSG_REPLY_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		TimeoutCount++;
		XDp_WaitUs(InstancePtr, 1000);
	}
	while ((AuxData & 0x10) != 0x10);

	return XST_SUCCESS;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */

/******************************************************************************/
/**
 * This function will take a byte array and convert it into a sideband message
 * format by filling in the XDp_SidebandMsg structure with the array data.
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
static u32 XDp_Transaction2MsgFormat(u8 *Transaction, XDp_SidebandMsg *Msg)
{
	XDp_SidebandMsgHeader *Header = &Msg->Header;
	XDp_SidebandMsgBody *Body = &Msg->Body;

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
	CrcCheck = XDp_Crc4CalculateHeader(Header);
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
	CrcCheck = XDp_Crc8CalculateBody(Msg);
	if (CrcCheck != Body->Crc) {
		/* The calculated CRC for the body did not match the
		 * response. */
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#if XPAR_XDPRXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * This function will write a new down reply message to be read by the upstream
 * device.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Data is the raw message to be written as a down reply.
 * @param	DataLength is the number of bytes in the down reply.
 *
 * @return
 *		- XST_SUCCESS if the down reply message was read by the upstream
 *		  device.
 *		- XST_ERROR_COUNT_MAX if the RX times out waiting for it's new
 *		  down reply bit to be cleared by the upstream device.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxWriteRawDownReply(XDp *InstancePtr, u8 *Data, u8 DataLength)
{
	u8 Index;
	u16 TimeoutCount = 0;
	u8 RegVal;

	/* Write the down reply message. */
	for (Index = 0; Index < DataLength; Index++) {
		XDp_WriteReg(InstancePtr->Config.BaseAddr,
				XDP_RX_DOWN_REP + (Index * 4), Data[Index]);
	}

	/* Indicate and alert that a new down reply is ready. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_DEVICE_SERVICE_IRQ,
				XDP_RX_DEVICE_SERVICE_IRQ_NEW_DOWN_REPLY_MASK);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_RX_HPD_INTERRUPT,
				XDP_RX_HPD_INTERRUPT_ASSERT_MASK);

	/* Wait until the upstream device reads the new down reply. */
	while (TimeoutCount < 5000) {
		RegVal = XDp_ReadReg(InstancePtr->Config.BaseAddr,
						XDP_RX_DEVICE_SERVICE_IRQ);
		if (!(RegVal & XDP_RX_DEVICE_SERVICE_IRQ_NEW_DOWN_REPLY_MASK)) {
			return XST_SUCCESS;
		}
		XDp_WaitUs(InstancePtr, 1000);
		TimeoutCount++;
	}

	/* The upstream device hasn't cleared the new down reply bit to indicate
	 * that the reply was read. */
	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * Given a sideband message structure, this function will issue a down reply
 * sideband message. If the overall sideband message is too large to fit into
 * the XDP_MAX_LENGTH_SBMSG byte maximum, it will split it up into multiple
 * sideband message fragments.
 * The header's Start/EndOfMsg bits, the header and body length, and CRC values
 * will be calculated and set accordingly for each sideband message fragment.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Msg is a pointer to the message to be sent.
 *
 * @return
 *		- XST_SUCCESS if the entire message was sent successfully.
 *		- XST_DEVICE_NOT_FOUND if no device is connected.
 *		- XST_ERROR_COUNT_MAX if sending one of the message fragments
 *		  timed out.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_RxSendSbMsg(XDp *InstancePtr, XDp_SidebandMsg *Msg)
{
	u32 Status;
	u8 BodyMsgDataCount = Msg->Body.MsgDataLength;
	u8 BodyMsgDataLeft = BodyMsgDataCount;

	Msg->FragmentNum = 0;
	while (BodyMsgDataLeft) {
		if (BodyMsgDataLeft == BodyMsgDataCount) {
			Msg->Header.StartOfMsgTransaction = 1;
		}
		else {
			Msg->Header.StartOfMsgTransaction = 0;
		}

		if (BodyMsgDataLeft > (XDP_MAX_LENGTH_SBMSG -
					Msg->Header.MsgHeaderLength - 1)) {
			Msg->Body.MsgDataLength = XDP_MAX_LENGTH_SBMSG -
					Msg->Header.MsgHeaderLength - 1;
		}
		else {
			Msg->Body.MsgDataLength = BodyMsgDataLeft;
		}
		BodyMsgDataLeft -= Msg->Body.MsgDataLength;
		Msg->Header.MsgBodyLength = Msg->Body.MsgDataLength + 1;

		if (BodyMsgDataLeft) {
			Msg->Header.EndOfMsgTransaction = 0;
		}
		else {
			Msg->Header.EndOfMsgTransaction = 1;
		}

		Msg->Header.Crc = XDp_Crc4CalculateHeader(&Msg->Header);
		Msg->Body.Crc = XDp_Crc8CalculateBody(Msg);

		Status = XDp_SendSbMsgFragment(InstancePtr, Msg);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		Msg->FragmentNum++;
	}

	return XST_SUCCESS;
}
#endif /* XPAR_XDPRXSS_NUM_INSTANCES */

/******************************************************************************/
/**
 * This function will perform a cyclic redundancy check (CRC) on the header of a
 * sideband message using a generator polynomial of 4.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Header is a pointer sideband message header that the CRC
 *		algorithm is to be run on.
 *
 * @return	The CRC value obtained by running the algorithm on the sideband
 *		message header.
 *
 * @note	The header is divided into 4-bit nibbles for use by the lower-
 *		level XDp_CrcCalculate function.
 *
*******************************************************************************/
static u8 XDp_Crc4CalculateHeader(XDp_SidebandMsgHeader *Header)
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

	return XDp_CrcCalculate(Nibbles, 4 * (5 + RadOffset), 4);
}

/******************************************************************************/
/**
 * This function will perform a cyclic redundancy check (CRC) on the body of a
 * sideband message using a generator polynomial of 8.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	Body is a pointer sideband message body that the CRC algorithm
 *		is to be run on.
 *
 * @return	The CRC value obtained by running the algorithm on the sideband
 *		message body.
 *
 * @note	None.
 *
*******************************************************************************/
static u8 XDp_Crc8CalculateBody(XDp_SidebandMsg *Msg)
{
	XDp_SidebandMsgBody *Body = &Msg->Body;
	u8 StartIndex;

	StartIndex = Msg->FragmentNum * (XDP_MAX_LENGTH_SBMSG -
					Msg->Header.MsgHeaderLength - 1);

	return XDp_CrcCalculate(&Body->MsgData[StartIndex],
					8 * Body->MsgDataLength, 8);
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
static u8 XDp_CrcCalculate(const u8 *Data, u32 NumberOfBits, u8 Polynomial)
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

#if XPAR_XDPTXSS_NUM_INSTANCES
/******************************************************************************/
/**
 * Check whether or not the two specified Tiled Display Topology (TDT) data
 * blocks represent devices that are part of the same tiled display. This is
 * done by comparing the TDT ID descriptor (tiled display manufacturer/vendor
 * ID, product ID and serial number fields).
 *
 * @param	TileDisp0 is one of the TDT data blocks to be compared.
 * @param	TileDisp1 is the other TDT data block to be compared.
 *
 * @return
 *		- 1 if the two TDT sections represent devices that are part of
 *		  the same tiled display.
 *		- 0 otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
static u32 XDp_TxIsSameTileDisplay(u8 *TileDisp0, u8 *TileDisp1)
{
	if ((TileDisp0[XDP_TX_DISPID_TDT_VENID0] !=
				TileDisp1[XDP_TX_DISPID_TDT_VENID0]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_VENID1] !=
				TileDisp1[XDP_TX_DISPID_TDT_VENID1]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_VENID2] !=
				TileDisp1[XDP_TX_DISPID_TDT_VENID2]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_PCODE0] !=
				TileDisp1[XDP_TX_DISPID_TDT_PCODE0]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_PCODE1] !=
				TileDisp1[XDP_TX_DISPID_TDT_PCODE1]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_SN0] !=
				TileDisp1[XDP_TX_DISPID_TDT_SN0]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_SN1] !=
				TileDisp1[XDP_TX_DISPID_TDT_SN1]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_SN2] !=
				TileDisp1[XDP_TX_DISPID_TDT_SN2]) ||
			(TileDisp0[XDP_TX_DISPID_TDT_SN3] !=
				TileDisp1[XDP_TX_DISPID_TDT_SN3]) ) {
		return 0;
	}

	return 1;
}
#endif /* XPAR_XDPTXSS_NUM_INSTANCES */
/** @} */
