/*******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_mst.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * MST sideband messaging (DOWN_REQ / DOWN_REP over AUX) and topology
 * discovery, so streams can be routed through MST branch devices.
 *
 * @note	Requests are single-chunk; replies may span several chunks.
 *		Replies are polled through DPCD ESI0, so the caller must not run
 *		other AUX traffic concurrently.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   arm  10/22/26 Initial release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <string.h>
#include <xstatus.h>
#include <xil_assert.h>
#include <xil_printf.h>

#include "xmmidp.h"

/**************************** Constant Definitions ****************************/

/** @name Sideband message transport
 * @{
 */
#define XMMIDP_SBMSG_MAX_BYTES		48	/**< One message transaction. */
#define XMMIDP_SBMSG_REPLY_MAX_BYTES	320	/**< Largest LINK_ADDRESS reply. */
#define XMMIDP_SBMSG_AUX_CHUNK		16	/**< Bytes per AUX transfer. */
#define XMMIDP_SBMSG_REPLY_POLL_US	1000	/**< Reply poll interval. */
#define XMMIDP_SBMSG_REPLY_TIMEOUT	5000	/**< Polls of REPLY_POLL_US. */
#define XMMIDP_SBMSG_BROADCAST_LCR	6	/**< LCR of a broadcast. */
#define XMMIDP_SBMSG_NAK_MASK		0x80	/**< Reply type NAK bit. */
#define XMMIDP_SBMSG_REQ_ID_MASK	0x7F	/**< Request ID bits. */
#define XMMIDP_SBMSG_SOMT_MASK		0x80	/**< Start of transaction. */
#define XMMIDP_SBMSG_EOMT_MASK		0x40	/**< End of transaction. */
#define XMMIDP_SBMSG_BODY_LEN_MASK	0x3F	/**< Body length bits. */
#define XMMIDP_SBMSG_NAK_REASON_IDX	17	/**< After request ID and GUID. */
#define XMMIDP_SBMSG_CRC4_POLY		0x13	/**< x^4 + x + 1 */
#define XMMIDP_SBMSG_CRC8_POLY		0x1D5	/**< x^8+x^7+x^6+x^4+x^2+1 */
/** @} */

/** Discovery skips the ports of a branch at this LCT or deeper. */
#define XMMIDP_MST_MAX_LCT		15

/**************************** Type Definitions ********************************/

/**
 * Sideband reply body reassembled from message transactions.
 */
typedef struct {
	u16 Length;	/**< Valid bytes in Data[]. */
	u8 Data[XMMIDP_SBMSG_REPLY_MAX_BYTES];	/**< Reply body. */
} XMmiDp_SbReply;

/************************** Variable Definitions ******************************/

/**
 * Reassembled reply body, kept off the stack because it is too large for the
 * stacks these designs run on.
 *
 * One buffer is shared by every sideband transaction, so the transport is not
 * reentrant. A transaction started from the HPD interrupt handler while a
 * main-loop transaction is in flight would overwrite the reassembly in place.
 * SbBusy rejects the nested call instead.
 */
static XMmiDp_SbReply SbReply;

/**
 * Set while a sideband transaction owns SbReply.
 */
static volatile u8 SbBusy;

/******************************************************************************/
/**
 * Claims the shared sideband reply buffer for one transaction.
 *
 * @return	XST_SUCCESS when the caller owns the buffer, XST_DEVICE_BUSY
 *		when a sideband transaction is already in flight.
 *
 * @note	The test and the set are not atomic, so this is not a formal
 *		critical section. It rejects the case that occurs in practice,
 *		an HPD interrupt handler starting a transaction while the main
 *		loop is inside one.
 *
*******************************************************************************/
static u32 XMmiDp_SbClaim(void)
{
	if (SbBusy != 0U)
		return XST_DEVICE_BUSY;

	SbBusy = 1U;

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Releases the shared sideband reply buffer.
 *
*******************************************************************************/
static void XMmiDp_SbRelease(void)
{
	SbBusy = 0U;
}

/******************************************************************************/
/**
 * Bitwise CRC over Width-bit units packed MSB first: CRC4 over header
 * nibbles, CRC8 over body bytes (DP 1.4 section 2.11.3).
 *
 * @param	Data is the packed input.
 * @param	NumBits is the number of input bits.
 * @param	Poly is the generator polynomial including its top bit.
 * @param	Width is 4 for CRC4 or 8 for CRC8.
 *
 * @return	The CRC value.
 *
*******************************************************************************/
static u8 XMmiDp_SbCrc(const u8 *Data, u32 NumBits, u16 Poly, u8 Width)
{
	u16 TopBit = (u16)(1U << Width);
	u16 Remainder = 0U;
	u32 BitIdx;

	for (BitIdx = 0U; BitIdx < NumBits; BitIdx++) {
		Remainder = (u16)((Remainder << 1) |
				  ((Data[BitIdx >> 3] >> (7U - (BitIdx & 7U))) &
				   1U));
		if ((Remainder & TopBit) != 0U)
			Remainder ^= Poly;
	}

	for (BitIdx = 0U; BitIdx < Width; BitIdx++) {
		Remainder = (u16)(Remainder << 1);
		if ((Remainder & TopBit) != 0U)
			Remainder ^= Poly;
	}

	return (u8)(Remainder & (TopBit - 1U));
}

/******************************************************************************/
/**
 * Writes a single-chunk sideband message header into Buf and fills in its
 * CRC4.
 *
 * @param	Buf receives the header bytes.
 * @param	LinkCountTotal is the header LCT.
 * @param	LinkCountRemaining is the header LCR.
 * @param	RelativeAddress holds LinkCountTotal - 1 port numbers.
 * @param	Broadcast is the broadcast message flag.
 * @param	PathMsg is the path message flag.
 * @param	BodyLength is the body length including the CRC8 byte.
 *
 * @return	The header length in bytes.
 *
*******************************************************************************/
static u8 XMmiDp_SbBuildHeader(u8 *Buf, u8 LinkCountTotal,
			       u8 LinkCountRemaining, const u8 *RelativeAddress,
			       u8 Broadcast, u8 PathMsg, u8 BodyLength)
{
	u8 Len = 0U;
	u8 Idx;

	Buf[Len++] = (u8)((LinkCountTotal << 4) | (LinkCountRemaining & 0x0FU));
	for (Idx = 0U; (Idx + 1U) < LinkCountTotal; Idx += 2U) {
		Buf[Len] = (u8)(RelativeAddress[Idx] << 4);
		if ((Idx + 2U) < LinkCountTotal)
			Buf[Len] |= RelativeAddress[Idx + 1U] & 0x0FU;
		Len++;
	}
	Buf[Len++] = (u8)((Broadcast << 7) | (PathMsg << 6) |
			  (BodyLength & XMMIDP_SBMSG_BODY_LEN_MASK));
	Buf[Len++] = XMMIDP_SBMSG_SOMT_MASK | XMMIDP_SBMSG_EOMT_MASK;

	/* CRC4 covers every header nibble except the CRC nibble itself. */
	Buf[Len - 1U] |= XMmiDp_SbCrc(Buf, ((Len * 2U) - 1U) * 4U,
				      XMMIDP_SBMSG_CRC4_POLY, 4U);

	return Len;
}

/******************************************************************************/
/**
 * Acknowledges DOWN_REP_MSG_RDY in DPCD ESI0 so the branch can post the next
 * reply chunk.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	The AUX write status.
 *
*******************************************************************************/
static u32 XMmiDp_SbAckDownRep(XMmiDp *InstancePtr)
{
	u8 AuxData = XMMIDP_DPCD_ESI0_DOWN_REP_MSG_RDY_MASK;

	return XMmiDp_AuxWrite(InstancePtr,
			       XMMIDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
			       1, &AuxData);
}

/******************************************************************************/
/**
 * Writes a sideband request to DPCD DOWN_REQ.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the target branch.
 * @param	RelativeAddress is the RAD of the target branch.
 * @param	Broadcast is the broadcast message flag.
 * @param	PathMsg is the path message flag.
 * @param	Body is the request body without CRC8.
 * @param	BodyBytes is the number of bytes in Body.
 *
 * @return	XST_SUCCESS, XST_FAILURE if the request does not fit one
 *		chunk, or the failing AUX status.
 *
*******************************************************************************/
static u32 XMmiDp_SbSendRequest(XMmiDp *InstancePtr, u8 LinkCountTotal,
				const u8 *RelativeAddress, u8 Broadcast,
				u8 PathMsg, const u8 *Body, u8 BodyBytes)
{
	u8 Msg[XMMIDP_SBMSG_MAX_BYTES];
	u8 LinkCountRemaining;
	u8 Esi0;
	u8 Len;
	u32 Status;

	/* Drop a stale reply so it is not taken as this request's reply. */
	Status = XMmiDp_AuxRead(InstancePtr,
				XMMIDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, &Esi0);
	if (Status != XST_SUCCESS)
		return Status;
	if ((Esi0 & XMMIDP_DPCD_ESI0_DOWN_REP_MSG_RDY_MASK) != 0U) {
		Status = XMmiDp_SbAckDownRep(InstancePtr);
		if (Status != XST_SUCCESS)
			return Status;
	}

	LinkCountRemaining = (Broadcast != 0U) ? XMMIDP_SBMSG_BROADCAST_LCR :
			     (u8)(LinkCountTotal - 1U);
	Len = XMmiDp_SbBuildHeader(Msg, LinkCountTotal, LinkCountRemaining,
				   RelativeAddress, Broadcast, PathMsg,
				   (u8)(BodyBytes + 1U));
	if (((u32)Len + BodyBytes + 1U) > XMMIDP_SBMSG_MAX_BYTES)
		return XST_FAILURE;

	memcpy(&Msg[Len], Body, BodyBytes);
	Msg[Len + BodyBytes] = XMmiDp_SbCrc(Body, (u32)BodyBytes * 8U,
					    XMMIDP_SBMSG_CRC8_POLY, 8U);
	Len += BodyBytes + 1U;

	return XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_DOWN_REQ, Len, Msg);
}

/******************************************************************************/
/**
 * Polls DPCD ESI0 until the branch posts a reply chunk in DOWN_REP.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	XST_SUCCESS, XST_ERROR_COUNT_MAX on timeout, or the failing AUX
 *		status.
 *
*******************************************************************************/
static u32 XMmiDp_SbWaitDownRep(XMmiDp *InstancePtr)
{
	u32 Poll;
	u32 Status;
	u8 Esi0;

	for (Poll = 0U; Poll < XMMIDP_SBMSG_REPLY_TIMEOUT; Poll++) {
		Status = XMmiDp_AuxRead(InstancePtr,
				XMMIDP_DPCD_SINK_DEVICE_SERVICE_IRQ_VECTOR_ESI0,
				1, &Esi0);
		if (Status != XST_SUCCESS)
			return Status;
		if ((Esi0 & XMMIDP_DPCD_ESI0_DOWN_REP_MSG_RDY_MASK) != 0U)
			return XST_SUCCESS;
		XMmiDp_WaitUs(InstancePtr, XMMIDP_SBMSG_REPLY_POLL_US);
	}

	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * Receives a complete sideband reply into SbReply, reassembling chunks until
 * EOMT, and checks that it is an ACK for ReqId.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	ReqId is the request identifier the reply must match.
 *
 * @return	XST_SUCCESS on ACK, XST_FAILURE on NAK, CRC error, malformed
 *		or mismatched reply, or the wait/AUX status.
 *
*******************************************************************************/
static u32 XMmiDp_SbReceiveReply(XMmiDp *InstancePtr, u8 ReqId)
{
	u8 Chunk[XMMIDP_SBMSG_MAX_BYTES];
	u8 HdrLen;
	u8 BodyLen;
	u8 MsgLen;
	u8 Eomt;
	u32 Status;

	SbReply.Length = 0U;

	do {
		Status = XMmiDp_SbWaitDownRep(InstancePtr);
		if (Status != XST_SUCCESS)
			return Status;

		Status = XMmiDp_AuxRead(InstancePtr, XMMIDP_DPCD_DOWN_REP,
					XMMIDP_SBMSG_AUX_CHUNK, Chunk);
		if (Status != XST_SUCCESS)
			return Status;

		/* LCT/LCR byte, (LCT - 1) RAD nibbles, two fixed bytes. */
		HdrLen = (u8)(3U + ((Chunk[0] >> 4) / 2U));
		BodyLen = Chunk[HdrLen - 2U] & XMMIDP_SBMSG_BODY_LEN_MASK;
		MsgLen = HdrLen + BodyLen;
		if ((BodyLen == 0U) || (MsgLen > XMMIDP_SBMSG_MAX_BYTES))
			return XST_FAILURE;

		if (MsgLen > XMMIDP_SBMSG_AUX_CHUNK) {
			Status = XMmiDp_AuxRead(InstancePtr,
					XMMIDP_DPCD_DOWN_REP +
					XMMIDP_SBMSG_AUX_CHUNK,
					MsgLen - XMMIDP_SBMSG_AUX_CHUNK,
					&Chunk[XMMIDP_SBMSG_AUX_CHUNK]);
			if (Status != XST_SUCCESS)
				return Status;
		}

		Status = XMmiDp_SbAckDownRep(InstancePtr);
		if (Status != XST_SUCCESS)
			return Status;

		if (((Chunk[HdrLen - 1U] & 0x0FU) !=
		     XMmiDp_SbCrc(Chunk, ((HdrLen * 2U) - 1U) * 4U,
				  XMMIDP_SBMSG_CRC4_POLY, 4U)) ||
		    (Chunk[MsgLen - 1U] !=
		     XMmiDp_SbCrc(&Chunk[HdrLen], (BodyLen - 1U) * 8U,
				  XMMIDP_SBMSG_CRC8_POLY, 8U))) {
			xil_printf("[MSTM] ERROR: Sideband reply CRC mismatch\r\n");
			return XST_FAILURE;
		}
		Eomt = Chunk[HdrLen - 1U] & XMMIDP_SBMSG_EOMT_MASK;

		if ((SbReply.Length + BodyLen - 1U) > XMMIDP_SBMSG_REPLY_MAX_BYTES)
			return XST_FAILURE;
		memcpy(&SbReply.Data[SbReply.Length], &Chunk[HdrLen],
		       BodyLen - 1U);
		SbReply.Length += BodyLen - 1U;
	} while (Eomt == 0U);

	if (SbReply.Length == 0U)
		return XST_FAILURE;

	if ((SbReply.Data[0] & XMMIDP_SBMSG_NAK_MASK) != 0U) {
		xil_printf("[MSTM] ERROR: Sideband request 0x%02X NAK "
			   "(reason 0x%02X)\r\n", ReqId,
			   (SbReply.Length > XMMIDP_SBMSG_NAK_REASON_IDX) ?
			   SbReply.Data[XMMIDP_SBMSG_NAK_REASON_IDX] : 0U);
		return XST_FAILURE;
	}

	if ((SbReply.Data[0] & XMMIDP_SBMSG_REQ_ID_MASK) != ReqId) {
		xil_printf("[MSTM] ERROR: Sideband reply 0x%02X does not match "
			   "request 0x%02X\r\n", SbReply.Data[0], ReqId);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Sends one sideband request and waits for its ACK.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the target branch.
 * @param	RelativeAddress is the RAD of the target branch.
 * @param	Broadcast is the broadcast message flag.
 * @param	PathMsg is the path message flag.
 * @param	Body is the request body; Body[0] is the request identifier.
 * @param	BodyBytes is the number of bytes in Body.
 *
 * @return	XST_SUCCESS when the reply is an ACK; otherwise the failing
 *		status.
 *
*******************************************************************************/
static u32 XMmiDp_SbTransact(XMmiDp *InstancePtr, u8 LinkCountTotal,
			     const u8 *RelativeAddress, u8 Broadcast,
			     u8 PathMsg, const u8 *Body, u8 BodyBytes)
{
	u32 Status;

	Status = XMmiDp_SbSendRequest(InstancePtr, LinkCountTotal,
				      RelativeAddress, Broadcast, PathMsg,
				      Body, BodyBytes);
	if (Status == XST_SUCCESS)
		Status = XMmiDp_SbReceiveReply(InstancePtr, Body[0]);

	if (Status != XST_SUCCESS)
		xil_printf("[MSTM] ERROR: Sideband request 0x%02X (LCT %u) "
			   "failed, status %u\r\n", Body[0], LinkCountTotal,
			   Status);

	return Status;
}

/******************************************************************************/
/**
 * Sends LINK_ADDRESS to a branch device and decodes its port list.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the branch (1 = directly attached).
 * @param	RelativeAddress is the RAD of the branch; unused when LCT is 1.
 * @param	Reply receives the decoded reply.
 *
 * @return	XST_SUCCESS, or XST_FAILURE / AUX status on error.
 *
*******************************************************************************/
static u32 XMmiDp_SbMsgLinkAddress(XMmiDp *InstancePtr, u8 LinkCountTotal,
				   const u8 *RelativeAddress,
				   XMmiDp_SbLinkAddressReply *Reply)
{
	u8 Body[1] = { XMMIDP_SBMSG_LINK_ADDRESS };
	XMmiDp_SbLinkAddressPort *Port;
	u32 Status;
	u16 Idx;
	u8 PortIdx;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((LinkCountTotal >= 1U) &&
			  (LinkCountTotal <= XMMIDP_MST_MAX_LCT));
	Xil_AssertNonvoid((LinkCountTotal == 1U) || (RelativeAddress != NULL));
	Xil_AssertNonvoid(Reply != NULL);

	Status = XMmiDp_SbTransact(InstancePtr, LinkCountTotal,
				   RelativeAddress, 0U, 0U, Body, sizeof(Body));
	if (Status != XST_SUCCESS)
		return Status;

	/*
	 * Request ID, GUID, port count, then 2 bytes per input port and
	 * 20 bytes per output port.
	 */
	Idx = 1U;
	if (SbReply.Length < (Idx + XMMIDP_GUID_NBYTES + 1U))
		return XST_FAILURE;
	memcpy(Reply->Guid, &SbReply.Data[Idx], XMMIDP_GUID_NBYTES);
	Idx += XMMIDP_GUID_NBYTES;
	Reply->NumPorts = SbReply.Data[Idx++] & 0x0FU;

	for (PortIdx = 0U; PortIdx < Reply->NumPorts; PortIdx++) {
		Port = &Reply->Ports[PortIdx];
		memset(Port, 0, sizeof(*Port));

		if ((Idx + 2U) > SbReply.Length)
			return XST_FAILURE;
		Port->InputPort = SbReply.Data[Idx] >> 7;
		Port->PeerDeviceType = (SbReply.Data[Idx] >> 4) & 0x07U;
		Port->PortNum = SbReply.Data[Idx++] & 0x0FU;
		Port->MsgCapStatus = SbReply.Data[Idx] >> 7;
		Port->DpDevPlugStatus = (SbReply.Data[Idx] >> 6) & 0x01U;
		if (Port->InputPort != 0U) {
			Idx++;
			continue;
		}
		Port->LegacyDevPlugStatus = (SbReply.Data[Idx++] >> 5) & 0x01U;

		if ((Idx + 2U + XMMIDP_GUID_NBYTES) > SbReply.Length)
			return XST_FAILURE;
		Port->DpcdRev = SbReply.Data[Idx++];
		Idx += XMMIDP_GUID_NBYTES;
		Port->NumSdpStreams = SbReply.Data[Idx] >> 4;
		Port->NumSdpStreamSinks = SbReply.Data[Idx++] & 0x0FU;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Sends LINK_ADDRESS to a branch device and decodes its port list.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the branch (1 = directly attached).
 * @param	RelativeAddress is the RAD of the branch; unused when LCT is 1.
 * @param	Reply receives the decoded reply.
 *
 * @return	XST_SUCCESS, XST_DEVICE_BUSY when another sideband transaction
 *		is in flight, or XST_FAILURE / AUX status on error.
 *
*******************************************************************************/
u32 XMmiDp_SendSbMsgLinkAddress(XMmiDp *InstancePtr, u8 LinkCountTotal,
				const u8 *RelativeAddress,
				XMmiDp_SbLinkAddressReply *Reply)
{
	u32 Status;

	Status = XMmiDp_SbClaim();
	if (Status != XST_SUCCESS)
		return Status;

	Status = XMmiDp_SbMsgLinkAddress(InstancePtr, LinkCountTotal,
					 RelativeAddress, Reply);
	XMmiDp_SbRelease();

	return Status;
}

/******************************************************************************/
/**
 * Sends ENUM_PATH_RESOURCES for the path to a sink.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the sink (>= 2).
 * @param	RelativeAddress is the RAD of the sink.
 * @param	AvailPbn receives the available PBN of the path.
 * @param	FullPbn receives the full PBN of the path.
 *
 * @return	XST_SUCCESS, or XST_FAILURE / AUX status on error.
 *
*******************************************************************************/
static u32 XMmiDp_SbMsgEnumPathResources(XMmiDp *InstancePtr, u8 LinkCountTotal,
					 const u8 *RelativeAddress,
					 u16 *AvailPbn, u16 *FullPbn)
{
	u8 Body[2];
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((LinkCountTotal >= XMMIDP_MST_SINK_MIN_LCT) &&
			  (LinkCountTotal <= XMMIDP_MST_MAX_LCT));
	Xil_AssertNonvoid(RelativeAddress != NULL);
	Xil_AssertNonvoid((AvailPbn != NULL) && (FullPbn != NULL));

	/* Addressed to the branch that owns the sink's port. */
	Body[0] = XMMIDP_SBMSG_ENUM_PATH_RESOURCES;
	Body[1] = (u8)(RelativeAddress[LinkCountTotal - 2U] << 4);

	Status = XMmiDp_SbTransact(InstancePtr, LinkCountTotal - 1U,
				   RelativeAddress, 0U, 1U, Body, sizeof(Body));
	if (Status != XST_SUCCESS)
		return Status;

	/* Request ID, port, full PBN, available PBN. */
	if (SbReply.Length < 6U)
		return XST_FAILURE;
	*FullPbn = (u16)((SbReply.Data[2] << 8) | SbReply.Data[3]);
	*AvailPbn = (u16)((SbReply.Data[4] << 8) | SbReply.Data[5]);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Sends ENUM_PATH_RESOURCES for the path to a sink.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the sink (>= 2).
 * @param	RelativeAddress is the RAD of the sink.
 * @param	AvailPbn receives the available PBN of the path.
 * @param	FullPbn receives the full PBN of the path.
 *
 * @return	XST_SUCCESS, XST_DEVICE_BUSY when another sideband transaction
 *		is in flight, or XST_FAILURE / AUX status on error.
 *
*******************************************************************************/
u32 XMmiDp_SendSbMsgEnumPathResources(XMmiDp *InstancePtr, u8 LinkCountTotal,
				      const u8 *RelativeAddress,
				      u16 *AvailPbn, u16 *FullPbn)
{
	u32 Status;

	Status = XMmiDp_SbClaim();
	if (Status != XST_SUCCESS)
		return Status;

	Status = XMmiDp_SbMsgEnumPathResources(InstancePtr, LinkCountTotal,
					       RelativeAddress, AvailPbn,
					       FullPbn);
	XMmiDp_SbRelease();

	return Status;
}

/******************************************************************************/
/**
 * Sends ALLOCATE_PAYLOAD so the branch that owns the sink's port maps
 * virtual channel VcId to that port with Pbn bandwidth.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the sink (>= 2).
 * @param	RelativeAddress is the RAD of the sink.
 * @param	VcId is the virtual channel payload ID (the stream ID).
 * @param	Pbn is the payload bandwidth number; 0 deallocates.
 *
 * @return	XST_SUCCESS, or XST_FAILURE / AUX status on error.
 *
 * @note	Call after the upstream payload table update and ACT.
 *
*******************************************************************************/
static u32 XMmiDp_SbMsgAllocatePayload(XMmiDp *InstancePtr, u8 LinkCountTotal,
				       const u8 *RelativeAddress, u8 VcId,
				       u16 Pbn)
{
	u8 Body[5];
	u16 AllocPbn;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((LinkCountTotal >= XMMIDP_MST_SINK_MIN_LCT) &&
			  (LinkCountTotal <= XMMIDP_MST_MAX_LCT));
	Xil_AssertNonvoid(RelativeAddress != NULL);

	Body[0] = XMMIDP_SBMSG_ALLOCATE_PAYLOAD;
	Body[1] = (u8)(RelativeAddress[LinkCountTotal - 2U] << 4);
	Body[2] = VcId & 0x7FU;
	Body[3] = (u8)(Pbn >> 8);
	Body[4] = (u8)(Pbn & 0xFFU);

	Status = XMmiDp_SbTransact(InstancePtr, LinkCountTotal - 1U,
				   RelativeAddress, 0U, 1U, Body, sizeof(Body));
	if (Status != XST_SUCCESS)
		return Status;

	/* Request ID, port, VC ID, allocated PBN. */
	if (SbReply.Length >= 5U) {
		AllocPbn = (u16)((SbReply.Data[3] << 8) | SbReply.Data[4]);
		if (AllocPbn != Pbn)
			xil_printf("[MSTM] WARNING: VC %u requested PBN %u, "
				   "branch allocated %u\r\n", VcId, Pbn,
				   AllocPbn);
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Sends ALLOCATE_PAYLOAD so the branch that owns the sink's port maps
 * virtual channel VcId to that port with Pbn bandwidth.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	LinkCountTotal is the LCT of the sink (>= 2).
 * @param	RelativeAddress is the RAD of the sink.
 * @param	VcId is the virtual channel payload ID (the stream ID).
 * @param	Pbn is the payload bandwidth number; 0 deallocates.
 *
 * @return	XST_SUCCESS, XST_DEVICE_BUSY when another sideband transaction
 *		is in flight, or XST_FAILURE / AUX status on error.
 *
 * @note	Call after the upstream payload table update and ACT.
 *
*******************************************************************************/
u32 XMmiDp_SendSbMsgAllocatePayload(XMmiDp *InstancePtr, u8 LinkCountTotal,
				    const u8 *RelativeAddress, u8 VcId,
				    u16 Pbn)
{
	u32 Status;

	Status = XMmiDp_SbClaim();
	if (Status != XST_SUCCESS)
		return Status;

	Status = XMmiDp_SbMsgAllocatePayload(InstancePtr, LinkCountTotal,
					     RelativeAddress, VcId, Pbn);
	XMmiDp_SbRelease();

	return Status;
}

/******************************************************************************/
/**
 * Broadcasts CLEAR_PAYLOAD_ID_TABLE so every downstream branch drops its
 * payload allocations.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	XST_SUCCESS, XST_DEVICE_BUSY when another sideband transaction
 *		is in flight, or XST_FAILURE / AUX status on error.
 *
*******************************************************************************/
u32 XMmiDp_SendSbMsgClearPayloadIdTable(XMmiDp *InstancePtr)
{
	u8 Body[1] = { XMMIDP_SBMSG_CLEAR_PAYLOAD_ID_TABLE };
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XMmiDp_SbClaim();
	if (Status != XST_SUCCESS)
		return Status;

	Status = XMmiDp_SbTransact(InstancePtr, 1U, NULL, 1U, 1U, Body,
				   sizeof(Body));
	XMmiDp_SbRelease();

	return Status;
}

/******************************************************************************/
/**
 * Discovers the MST topology with LINK_ADDRESS, breadth first from the
 * directly attached branch, and records every plugged DP monitor (SST sink)
 * in InstancePtr->MstTopology in port order.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return	XST_SUCCESS, or the status of the failing LINK_ADDRESS.
 *
 * @note	Branches beyond XMMIDP_MST_MAX_BRANCHES and sinks beyond
 *		XMMIDP_MST_MAX_SINKS are ignored.
 *
*******************************************************************************/
u32 XMmiDp_MstDiscoverTopology(XMmiDp *InstancePtr)
{
	XMmiDp_MstTopology *Topology;
	XMmiDp_SbLinkAddressReply Reply;
	XMmiDp_SbLinkAddressPort *Port;
	XMmiDp_MstSink *Sink;
	u8 BranchLct[XMMIDP_MST_MAX_BRANCHES];
	u8 BranchRad[XMMIDP_MST_MAX_BRANCHES][XMMIDP_MST_MAX_RAD] = { { 0U } };
	u8 Head = 0U;
	u8 Tail = 0U;
	u8 Lct;
	u8 PortIdx;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Topology = &InstancePtr->MstTopology;
	memset(Topology, 0, sizeof(*Topology));

	BranchLct[Tail++] = 1U;

	while (Head < Tail) {
		Lct = BranchLct[Head];

		Status = XMmiDp_SendSbMsgLinkAddress(InstancePtr, Lct,
						     BranchRad[Head], &Reply);
		if (Status != XST_SUCCESS)
			return Status;
		Topology->BranchTotal++;

		for (PortIdx = 0U; PortIdx < Reply.NumPorts; PortIdx++) {
			Port = &Reply.Ports[PortIdx];
			if ((Port->InputPort != 0U) ||
			    (Port->DpDevPlugStatus == 0U) ||
			    (Lct >= XMMIDP_MST_MAX_LCT))
				continue;

			if ((Port->PeerDeviceType ==
			     XMMIDP_PEER_DEVICE_MST_BRANCHING_UNIT) &&
			    (Port->MsgCapStatus != 0U)) {
				if (Tail < XMMIDP_MST_MAX_BRANCHES) {
					BranchLct[Tail] = Lct + 1U;
					memcpy(BranchRad[Tail], BranchRad[Head],
					       Lct - 1U);
					BranchRad[Tail][Lct - 1U] = Port->PortNum;
					Tail++;
				}
				continue;
			}

			/*
			 * Only DP monitors are routed; legacy converters and
			 * branches without messaging are not supported.
			 */
			if ((Port->PeerDeviceType !=
			     XMMIDP_PEER_DEVICE_SST_SINK) ||
			    (Topology->SinkTotal >= XMMIDP_MST_MAX_SINKS))
				continue;

			Sink = &Topology->Sinks[Topology->SinkTotal++];
			Sink->LinkCountTotal = Lct + 1U;
			memcpy(Sink->RelativeAddress, BranchRad[Head], Lct - 1U);
			Sink->RelativeAddress[Lct - 1U] = Port->PortNum;
			Sink->PeerDeviceType = Port->PeerDeviceType;
			Sink->DpcdRev = Port->DpcdRev;
		}

		Head++;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * Routes a stream to a sink found by XMmiDp_MstDiscoverTopology(); the sink
 * address is used by XMmiDp_AllocatePayloadStream() for ALLOCATE_PAYLOAD.
 *
 * @param	InstancePtr is a pointer to the XMmiDp instance.
 * @param	Stream is the stream ID (XMMIDP_STREAM_ID1..ID4).
 * @param	SinkIdx is the index into InstancePtr->MstTopology.Sinks.
 *
 * @return	XST_SUCCESS, or XST_FAILURE if SinkIdx is out of range.
 *
*******************************************************************************/
u32 XMmiDp_MstSetStreamSink(XMmiDp *InstancePtr, u8 Stream, u8 SinkIdx)
{
	XMmiDp_MstStream *MstStream;
	XMmiDp_MstSink *Sink;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Stream >= XMMIDP_STREAM_ID1) &&
			  (Stream <= XMMIDP_STREAM_ID4));

	if (SinkIdx >= InstancePtr->MstTopology.SinkTotal)
		return XST_FAILURE;

	Sink = &InstancePtr->MstTopology.Sinks[SinkIdx];
	MstStream = &InstancePtr->MstStreamConfig[Stream - 1U];
	MstStream->LinkCountTotal = Sink->LinkCountTotal;
	memcpy(MstStream->RelativeAddress, Sink->RelativeAddress,
	       sizeof(MstStream->RelativeAddress));

	return XST_SUCCESS;
}
/** @} */
