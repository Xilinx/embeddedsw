/******************************************************************************\
|* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) <2021> by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include <string.h>

#include "mbox_cmd.h"

#include <stdio.h>

#include "mbox_command.h"
#include "mbox_interface.h"
#include "xil_printf.h"
#include "sensor_cmd.h"
#include "vlog.h"
#include "xparameters.h"
#if ENABLE_VMIX_MACRO
	#include "xv_mix_l2.h"
	#include "xvidc.h"
#endif

#ifdef XPAR_I2C0_BASEADDR
#include "control.h"
#endif

#define LOGTAG "MCMD"
extern long int apu_parsecontrolenter;

/*-------------------for set param to RPU-------------*/
static volatile bool ACK = false;
static volatile bool mb_data_flag = false;
static volatile uint32_t payload_ret_cookie = 0;
static volatile bool ACK_PROC = false;
static volatile bool ACK_PROC_FUSA = false;

/**
 * @brief Sets the response acknowledgment flag.
 */
void mbox_command_set_ack(bool ack)
{
	ACK = ack;
}

/**
 * @brief Sets the response data-ready flag.
 */
void mbox_command_set_data_ready(bool ready)
{
	mb_data_flag = ready;
}

/**
 * @brief Stores the cookie of the most recent response.
 */
void mbox_command_set_cookie(uint32_t cookie)
{
	payload_ret_cookie = cookie;
}

/**
 * @brief Returns whether the response acknowledgment flag is set.
 */
bool mbox_command_is_ack_set(void)
{
	return ACK;
}

/**
 * @brief Returns whether response payload data is ready.
 */
bool mbox_command_is_data_ready(void)
{
	return mb_data_flag;
}

/**
 * @brief Returns the cookie of the most recent response.
 */
uint32_t mbox_command_get_cookie(void)
{
	return payload_ret_cookie;
}

/**
 * @brief Marks the pending command as an asynchronous command.
 */
void mbox_command_mark_async_cmd(void)
{
	ACK_PROC = true;
}

/**
 * @brief Marks the pending command as an asynchronous FuSa command.
 */
void mbox_command_mark_async_fusa_cmd(void)
{
	ACK_PROC_FUSA = true;
}

/**
 * @brief Initializes the mailbox for the given core.
 *
 * @param[in] cpu Local core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mailbox_init(uint32_t cpu)
{
	return mbox_interface_init((MboxCoreId)cpu);
}

/**
 * @brief Closes the mailbox interface.
 */
void mailbox_close()
{
	mbox_interface_close();
}

/**
 * @brief Initializes the mailbox IPI transport on the local core.
 *
 * @return 0 on success, -1 on failure.
 */
int init_mailbox_ipi()
{
	return (mailbox_init(src_cpu_id) == VPI_SUCCESS) ? 0 : -1;
}

/**
 * @brief Sends a mailbox command to a remote core.
 *
 * @param[in] cmd      Command identifier to send.
 * @param[in] data     Pointer to the command payload.
 * @param[in] size     Payload size in bytes.
 * @param[in] dest_cpu Destination core identifier.
 * @param[in] src_cpu  Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
	return mbox_interface_send_command(cmd, data, size, dest_cpu, src_cpu);
}

/**
 * @brief Sends a mailbox response to a remote core.
 *
 * @param[in] res      Response identifier to send.
 * @param[in] data     Pointer to the response payload.
 * @param[in] size     Payload size in bytes.
 * @param[in] dest_cpu Destination core identifier.
 * @param[in] src_cpu  Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int Send_Response(MBCmdId_E res, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
	return mbox_interface_send_response(res, data, size, dest_cpu, src_cpu);
}

/**
 * @brief Sends a raw mailbox message without command framing.
 *
 * @param[in] cmdId       Message identifier to send.
 * @param[in] struct_msg  Pointer to the raw payload.
 * @param[in] size        Payload size in bytes.
 * @param[in] receiver_id Destination core identifier.
 * @param[in] core_id     Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t write_mboxcmd(uint32_t cmdId, void *struct_msg, uint32_t size, MboxCoreId receiver_id,
		      MboxCoreId core_id)
{
	return mbox_interface_send_raw(cmdId, struct_msg, size,
				      (uint8_t)receiver_id, (uint8_t)core_id);
}

/**
 * @brief Waits for the response that matches the given cookie.
 *
 * @param[in]  cookie Cookie identifying the awaited response.
 * @param[out] data   Buffer that receives the response payload.
 *
 * @return Response subcode on success, an error code otherwise.
 */
uint8_t apu_wait_for_ACK(uint32_t cookie, void *data)
{
	return mbox_interface_wait_for_ack(cookie, data);
}

/**
 * @brief Dispatches pending inbound mailbox messages for the given IPI source.
 *
 * @param[in] IpiSrcMask Interrupt source mask of the triggering IPI.
 */
void apu_mailbox_read(uint32_t IpiSrcMask)
{
	mbox_interface_rx_dispatch(IpiSrcMask);
}

/**
 * @brief Processes a pending asynchronous command received from a remote core.
 */
void Apu_Mbox_Check_Command(void)
{
	MboxPostMsg *rmsg_command_to_apu = mbox_interface_get_command_msg();
	uint32_t msg_id_;
	uint32_t size_;
	Payload_packet proc_cmd_packet;

	if (rmsg_command_to_apu == NULL)
		return;

	if (ACK_PROC == true) {
		LOGI("\r\n processing command from RPU \r\n");
		ACK_PROC = false;
		msg_id_ = rmsg_command_to_apu->msg_id;
		size_ = rmsg_command_to_apu->size;
		if (size_ > sizeof(Payload_packet)) {
			LOGE("Mbox cmd size %u exceeds max %u, dropping\r\n",
			     size_, (uint32_t)sizeof(Payload_packet));
			return;
		}
		memcpy(&proc_cmd_packet, &rmsg_command_to_apu->payload, size_);
		ApuProcessCommand(msg_id_, &proc_cmd_packet, size_, src_cpu_id, dest_cpu_id);
	}
}


/**
 * @brief Processes a pending asynchronous FuSa command received from a remote core.
 */
void Apu_Mbox_Check_FusaCommand(void)
{
	MboxPostMsg *rmsg_command_to_apu = mbox_interface_get_command_msg();
	uint32_t msg_id_;
	uint32_t size_;
	Payload_packet proc_cmd_packet;

	if (rmsg_command_to_apu == NULL)
		return;

	if (ACK_PROC_FUSA == true) {
		ACK_PROC_FUSA = false;
		msg_id_ = rmsg_command_to_apu->msg_id;
		size_ = rmsg_command_to_apu->size;
		if (size_ > sizeof(Payload_packet)) {
			LOGE("Mbox fusa cmd size %u exceeds max %u, dropping\r\n",
			     size_, (uint32_t)sizeof(Payload_packet));
			return;
		}
		memcpy(&proc_cmd_packet, &rmsg_command_to_apu->payload, size_);
		ApuProcessFusaCommand(msg_id_, &proc_cmd_packet, size_, src_cpu_id, dest_cpu_id);
	}
}

#if ENABLE_VMIX_MACRO
	VMix_input_buffer Vmix_buff;
	extern XV_Mix_l2  mix;
	extern XVidC_VideoStream  VidStream;
#endif
uint32_t ParseCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
		      MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif
	switch (cmd) {
		/*These cases handle the commands sent by the RPU's*/
		case MB_CMD_RES_SUCCESS:
			mbox_command_set_cookie(packet->cookie);
			mbox_command_set_ack(true);
			break;

		case MB_CMD_GET_SUCCESS:
			mbox_command_set_cookie(packet->cookie);
			mbox_command_set_ack(true);
			mbox_command_set_data_ready(true);

			break;
		case RPU_2_APU_MB_CMD_ISP_ERR_REPORT:
		case RPU_2_APU_MB_CMD_IsiCreateIss:
		case RPU_2_APU_MB_CMD_IsiReleaseIss:
		case RPU_2_APU_MB_CMD_IsiEnumModeIss:
		case RPU_2_APU_MB_CMD_IsiCheckConnectionIss:
		case RPU_2_APU_MB_CMD_IsiOpenIss:
		case RPU_2_APU_MB_CMD_IsiCloseIss:
		case RPU_2_APU_MB_CMD_IsiGetModeIss:
		case	RPU_2_APU_MB_CMD_IsiGetCapsIss:
		case	RPU_2_APU_MB_CMD_IsiGetRevisionIss:
		case	RPU_2_APU_MB_CMD_IsiSetStreamingIss:
		case	RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss:
		case	RPU_2_APU_MB_CMD_IsiGetAGainIss:
		case	RPU_2_APU_MB_CMD_IsiGetDGainIss:
		case	RPU_2_APU_MB_CMD_IsiSetAGainIss:
		case	RPU_2_APU_MB_CMD_IsiSetDGainIss:
		case	RPU_2_APU_MB_CMD_IsiGetIntTimeIss:
		case	RPU_2_APU_MB_CMD_IsiSetIntTimeIss:
		case	RPU_2_APU_MB_CMD_IsiGetFpsIss:
		case	RPU_2_APU_MB_CMD_IsiSetFpsIss:
		case	RPU_2_APU_MB_CMD_IsiGetIspStatusIss:
		case	RPU_2_APU_MB_CMD_IsiSetWBIss:
		case	RPU_2_APU_MB_CMD_IsiGetWBIss:
		case	RPU_2_APU_MB_CMD_IsiSetBlcIss:
		case	RPU_2_APU_MB_CMD_IsiGetBlcIss:
		case	RPU_2_APU_MB_CMD_IsiSetTpgIss:
		case	RPU_2_APU_MB_CMD_IsiGetTpgIss:
		case	RPU_2_APU_MB_CMD_IsiGetExpandCurveIss:
		case	RPU_2_APU_MB_CMD_IsiWriteRegIss:
		case	RPU_2_APU_MB_CMD_IsiReadRegIss:

			mbox_command_mark_async_cmd();
			break;

		case RPU_2_APU_MB_CMD_FUSA_EVENT_CB:
			mbox_command_mark_async_fusa_cmd();
			break;

		case RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM:
			//LOGI("RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM coreid %d\r\n", core_id);
			CtrlFullBufferInform(data);
			break;


		case RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE:
			// LOGI("RPU %d requested RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE \n",src_cpu);
			break;

		case MB_CMD_END:
		case APU_2_RPU_MB_CMB_INIT_FIRMWARE:
			break;

		/*Following cases handle the responses sent by the RPU*/

		case MB_CMD_RES_ERR:
			//LOGI("APU MB_CMD_RES_ERR %d, cookie 0x%x,cmd id %d,src_cpu  %d \n", core_id,buf_resp->cookie,buf_resp->cmdid,src_cpu);
			LOGI("APU MB_CMD_RES_ERR %d, cookie 0x%x,src_cpu  %d \n", core_id, packet->cookie, src_cpu);
			break;
		case MB_CMD_RES_TIMEOUT:
			//LOGI("APU MB_CMD_RES_TIMEOUT %d, cookie 0x%x,cmd id %d,src_cpu %d \n", core_id,buf_resp->cookie,buf_resp->cmdid,src_cpu);
			LOGI("APU MB_CMD_RES_TIMEOUT %d, cookie 0x%x,src_cpu %d \n", core_id, packet->cookie, src_cpu);
			break;
		case MB_CMD_BUF_RET:
			//LOGI("FUN: %s\t buff response from RPU0 0x%x\r\n",__func__,buf_resp->resp_payload[0]);
			//Vmix_buff.baseAddress = buf_resp->resp_payload[0];
#if ENABLE_VMIX_MACRO
			Vmix_buff.baseAddress = packet->payload_data[0];
			Status = VMix_User_defined(&VidStream);  //check this!??
			if (Status == XST_FAILURE) {
				LOGE("VMIX User defined failed.\n\r");
				return XST_FAILURE;
			} else
				LOGI("\r\n\r\n VMIX User defined is Done \n");
#endif
			//LOGI("@RPU Response error: 0x%x cmd:%d cookie:%d load:%d\r\n",buf_resp->error_subcode, buf_resp->cmdid, buf_resp->cookie,buf_resp->payload_type) ;
			break;
	}
	//TODO:Send response tacket
	return ret;
}

uint32_t ApuProcessCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			   MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	//Response_packet resp_pckt, *buf_resp = data; //response packet to be sent to APU; //Received response packet
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif

	switch (cmd) {
		case RPU_2_APU_MB_CMD_ISP_ERR_REPORT:
			u32 isp_mis_read = packet->payload_data[0] |
					   (packet->payload_data[1] << 8) |
					   (packet->payload_data[2] << 16) |
					   (packet->payload_data[3] << 24);
			LOGI("APU RPU_2_APU_MB_CMD_ISP_ERR_REPORT, core_id %d, cookie 0x%x, src_xpu %d, isp_mis_read 0x%08x\n",
			     core_id, packet->cookie, src_cpu, isp_mis_read);

			// memset(packet, 0, sizeof(Payload_packet));
			// packet->cookie = 0;
			// packet->type = CMD;
			// packet->payload_size = 0;
			// packet->payload_size += sizeof(uint32_t);
			// Send_Command (APU_2_RPU_MB_CMD_SOFT_RESET, &packet,sizeof(Payload_packet),dest_cpu_id, src_cpu_id);
			break;

		/*---------------------------------------sensor function--------------------------------------*/
		case RPU_2_APU_MB_CMD_IsiCreateIss:
			LOGI("RPU_2_APU_MB_CMD_IsiCreateIss coreid %d\r\n", core_id);

			ret = ControlIsiSensorDrvHandleRegisterIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiReleaseIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiSensorDrvHandleUnRegisterIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;

		case RPU_2_APU_MB_CMD_IsiEnumModeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiEnumModeIss coreid %d\r\n", core_id);

			ret = ControlIsiEnumModeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiCheckConnectionIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiCheckConnectionIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiOpenIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiOpenIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiCloseIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiCloseIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiWriteRegIss:
			LOGI("RPU_2_APU_MB_CMD_IsiWriteRegIss coreid %d\r\n", core_id);

			ret = ControlIsiWriteRegIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiReadRegIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReadRegIss coreid %d\r\n", core_id);

			ret = ControlIsiReadRegIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetModeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetModeIss coreid %d\r\n", core_id);

			ret = ControlIsiGetModeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetCapsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetCapsIss coreid %d\r\n", core_id);

			ret = ControlIsiGetCapsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetRevisionIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetRevisionIss coreid %d\r\n", core_id);

			ret = ControlIsiGetRevisionIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetStreamingIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetStreamingIss coreid %d\r\n", core_id);

			ret = ControlIsiSetStreamingIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss coreid %d\r\n", core_id);

			ret = ControlIsiGetAeBaseInfoIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetAGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetAGainIss coreid %d\r\n", core_id);

			ret = ControlIsiGetAGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetDGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetDGainIss coreid %d\r\n", core_id);

			ret = ControlIsiGetDGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetAGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetAGainIss coreid %d\r\n", core_id);

			ret = ControlIsiSetAGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetDGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetDGainIss coreid %d\r\n", core_id);

			ret = ControlIsiSetDGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetIntTimeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetIntTimeIss coreid %d\r\n", core_id);

			ret = ControlIsiGetIntTimeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetIntTimeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetIntTimeIss coreid %d\r\n", core_id);

			ret = ControlIsiSetIntTimeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetFpsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetFpsIss coreid %d\r\n", core_id);

			ret = ControlIsiGetFpsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetFpsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetFpsIss coreid %d\r\n", core_id);

			ret = ControlIsiSetFpsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetIspStatusIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetIspStatusIss coreid %d\r\n", core_id);

			ret = ControlIsiGetIspStatusIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetWBIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetWBIss coreid %d\r\n", core_id);

			ret = ControlIsiSetWBIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetWBIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetWBIss coreid %d\r\n", core_id);

			ret = ControlIsiGetWBIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetBlcIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetBlcIss coreid %d\r\n", core_id);

			ret = ControlIsiSetBlcIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;

		case RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM:
			CtrlFullBufferInform(data);
			break;


		case RPU_2_APU_MB_CMD_IsiGetBlcIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetBlcIss coreid %d\r\n", core_id);

			ret = ControlIsiGetBlcIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);

			break;
		case RPU_2_APU_MB_CMD_IsiSetTpgIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetTpgIss coreid %d\r\n", core_id);

			ret = ControlIsiSetTpgIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetTpgIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetTpgIss coreid %d\r\n", core_id);

			ret = ControlIsiGetTpgIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetExpandCurveIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetExpandCurveIss coreid %d\r\n", core_id);

			ret = ControlIsiGetExpandCurveIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;


		case RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE:
			// LOGI("RPU %d requested RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE \n",src_cpu);
			break;
		default:
			break;
	}
	//TODO:Send response tacket
	return ret;
}


uint32_t ApuProcessFusaCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			       MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	//Response_packet resp_pckt, *buf_resp = data; //response packet to be sent to APU; //Received response packet
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif

	switch (cmd) {
		case RPU_2_APU_MB_CMD_FUSA_EVENT_CB:
			xil_printf("RPU_2_APU_MB_CMD_FUSA_EVENT_CB coreid %d\r\n", core_id);
			CtrlCamDeviceFusaEventCb(data);

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				xil_printf("can not send response, the result is %d.\r\n", ret);
			break;

		default:
			break;
	}
	//TODO:Send response tacket
	return ret;
}
