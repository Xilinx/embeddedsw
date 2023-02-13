/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_io.h"
#include "xpm_regs.h"
#include "xpm_common.h"
#include "xpm_debug.h"
#include "xplmi_ssit.h"

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/* Check if given device is on secondary SLR, if so, return which SLR */
inline u32 IsNodeOnSecondarySLR(u32 DeviceId, u32 *SlrIndex)
{
	*SlrIndex = (DeviceId >> NODE_SLR_IDX_SHIFT) & NODE_SLR_IDX_MASK_BITS;

	return (*SlrIndex != 0U) ? TRUE : FALSE;
}
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

/*****************************************************************************/
/**
 * This function is used to forward APIs to secondary SLR on SSIT devices
 *
 * @param ApiId		PM API ID
 * @param ArgBuf	Api specific arguments
 * @param NumArgs	Number of arguments in the buffer
 * @param CmdType	Optional argument. Secure/nonsecure command type
 * @param Response	Optional argument. Response to client.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *****************************************************************************/
XStatus XPm_SsitForwardApi(XPm_ApiId ApiId, const u32 *ArgBuf, u32 NumArgs,
				const u32 CmdType, u32 *const Response)
{
	XStatus Status = XST_DEVICE_NOT_FOUND;

	(void)ApiId;
	(void)ArgBuf;
	(void)NumArgs;
	(void)CmdType;
	(void)Response;

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	u32 SlrIndex = 0U;
	u32 DeviceId = ArgBuf[0];

	/**
	 * If we're on primary SLR, check if this command is intended for a
	 * secondary SLR; forward it if applicable
	 */
	if ((XPLMI_SSIT_MASTER_SLR_INDEX == XPlmi_GetSlrIndex()) &&
		(TRUE == IsNodeOnSecondarySLR(DeviceId, &SlrIndex))) {

		u32 ReqBuf[6U] = { 0 };
		u32 RespBuf[2U] = { 0 };

		/* Clear SLR index bits from the node id */
		u32 DevId = DeviceId & ~NODE_SLR_IDX_MASK;

		/* Pack payload to forward to secondary SLR */
		XPM_PACK_PAYLOAD(ReqBuf, XPM_HEADER(NumArgs, ApiId), DevId, ArgBuf[1], ArgBuf[2], ArgBuf[3], ArgBuf[4]);

		/* Set secure/nonsecure command type */
		if (NO_HEADER_CMDTYPE != CmdType) {
			XPM_HEADER_SET_CMDTYPE(ReqBuf, CmdType);
		}

		/* Forward message event to secondary SLR */
		PmDbg("Sending API: %x to SLR: <%u>\r\n", ApiId, SlrIndex);
		Status = XPlmi_SsitSendMsgEventAndGetResp((u8)SlrIndex,
				ReqBuf, ARRAY_SIZE(ReqBuf), RespBuf, ARRAY_SIZE(RespBuf),
				TIMEOUT_IOCTL_COMPL);
		if (XST_SUCCESS != Status) {
			PmDbg("SSIT API forward failed: 0x%x\r\n", Status);
			goto done;
		}

		Status = (XStatus)RespBuf[0];	/* First word contains status */
		PmDbg("Received: Status: 0x%x\r\n", Status);

		if (NULL != Response) {
			*Response = RespBuf[1];	/* Second word contains response */
			PmDbg("Received: Payload: 0x%x\r\n", *Response);
		}
	}

done:
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

	return Status;
}

/*****************************************************************************/
/**
 *  This function is used to set/clear bits in any NPI PCSR
 *
 *  @param BaseAddress	BaseAddress of device
 *  @param Mask			Mask to be written into PCSR_MASK register
 *  @param Value		Value to be written into PCSR_CONTROL register
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *****************************************************************************/
XStatus XPm_PcsrWrite(u32 BaseAddress, u32 Mask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Out32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	/* Blind write check */
	PmChkRegOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK;
		goto done;
	}

	XPm_Out32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
	/* Blind write check */
	PmChkRegMask32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Mask, Value, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

u8 XPm_PlatGetSlrIndex(void)
{
	return XPlmi_GetSlrIndex();
}
