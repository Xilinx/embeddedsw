// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#include <string.h>

#include "mbox_interface.h"

#include "mbox_command.h"
#include "mbox_error_code.h"
#include "mbox_integrity.h"
#include "mbox_phy.h"
#include "oslayer.h"
#include "vlog.h"
#include "xil_printf.h"

/** Log tag used by the mailbox interface layer. */
#define LOGTAG "MIFC"

/** Maximum time to wait for a command acknowledgment, in microseconds. */
#define ACK_TIMEOUT_US 180000000U
/** Polling interval while waiting for an acknowledgment, in microseconds. */
#define ACK_POLL_INTERVAL_US 100U

static MboxPostMsg *rx_msg;
static MboxPostMsg *rx_response;
static MboxPostMsg *rx_command;

/**
 * @brief Returns the used byte count of a mailbox message.
 *
 * @param[in] msg Message to measure.
 *
 * @return Size of the message header plus the used payload, in bytes.
 */
static inline uint32_t mbox_copy_size(const MboxPostMsg *msg)
{
	return sizeof(MboxPostMsg) - MAX_ITEM + msg->payload.payload_size;
}

/**
 * @brief Allocates the receive buffers used by the mailbox interface.
 *
 * @return VPI_SUCCESS on success, VPI_ERR_NOMEM otherwise.
 */
static int32_t mbox_interface_allocate_buffers(void)
{
	rx_msg = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rx_msg == NULL)
		goto err_nomem;

	rx_response = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rx_response == NULL)
		goto err_free_rx;

	rx_command = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rx_command == NULL)
		goto err_free_resp;

	return VPI_SUCCESS;

err_free_resp:
	osFree(rx_response);
	rx_response = NULL;
err_free_rx:
	osFree(rx_msg);
	rx_msg = NULL;
err_nomem:
	return VPI_ERR_NOMEM;
}

/**
 * @brief Initializes the mailbox interface and its transport layer.
 */
int32_t mbox_interface_init(MboxCoreId current_cpu_id)
{
	int32_t status;

	status = mbox_interface_allocate_buffers();
	if (status != VPI_SUCCESS)
		return status;

	mbox_integrity_init();
	status = mbox_phy_init(current_cpu_id);
	if (status != VPI_SUCCESS) {
		osFree(rx_msg);
		osFree(rx_response);
		osFree(rx_command);
		rx_msg = NULL;
		rx_response = NULL;
		rx_command = NULL;
		return status;
	}

	return VPI_SUCCESS;
}

/**
 * @brief Closes the mailbox interface and releases its buffers.
 */
void mbox_interface_close(void)
{
	mbox_phy_close();

	osFree(rx_msg);
	osFree(rx_response);
	osFree(rx_command);

	rx_msg = NULL;
	rx_response = NULL;
	rx_command = NULL;
}

/**
 * @brief Builds, stamps, and sends a mailbox command.
 */
int32_t mbox_interface_send_command(MBCmdId_E cmd, void *data, uint32_t size,
				   uint8_t dest_cpu, uint8_t src_cpu)
{
	Payload_packet *payload = (Payload_packet *)data;
	MboxPostMsg tmp_msg;
	int32_t ret;

	if (payload == NULL)
		return VPI_ERR_INVALID;

	payload->resp_field.error_subcode_t = RET_SUCCESS;

	(void)size;
	memset(&tmp_msg, 0, sizeof(tmp_msg));
	tmp_msg.msg_id = cmd;
	tmp_msg.size = sizeof(Payload_packet) - MAX_ITEM + payload->payload_size;
	memcpy(&tmp_msg.payload, payload, tmp_msg.size);

	mbox_integrity_stamp_outbound(dest_cpu, &tmp_msg);

	ret = mbox_phy_send(&tmp_msg, (MboxCoreId)dest_cpu, (MboxCoreId)src_cpu);
	if (ret == VPI_ERR_FULL)
		LOGE("VPI_ERR_FULL while sending cmd=%u to core=%u\n", cmd, dest_cpu);

	return ret;
}

/**
 * @brief Builds, stamps, and sends a mailbox response.
 */
int32_t mbox_interface_send_response(MBCmdId_E res, void *data, uint32_t size,
				    uint8_t dest_cpu, uint8_t src_cpu)
{
	Payload_packet *payload = (Payload_packet *)data;
	MboxPostMsg tmp_msg;

	if (payload == NULL)
		return VPI_ERR_INVALID;

	(void)size;
	memset(&tmp_msg, 0, sizeof(tmp_msg));
	tmp_msg.msg_id = res;
	tmp_msg.size = sizeof(Payload_packet) - MAX_ITEM + payload->payload_size;
	memcpy(&tmp_msg.payload, payload, tmp_msg.size);

	mbox_integrity_stamp_outbound(dest_cpu, &tmp_msg);
	return mbox_phy_send(&tmp_msg, (MboxCoreId)dest_cpu, (MboxCoreId)src_cpu);
}

/**
 * @brief Sends a raw payload as a mailbox message without command framing.
 */
int32_t mbox_interface_send_raw(uint32_t msg_id, void *data, uint32_t size,
			       uint8_t dest_cpu, uint8_t src_cpu)
{
	MboxPostMsg tmp_msg;

	if (data == NULL)
		return VPI_ERR_INVALID;

	if (size > sizeof(Payload_packet))
		return VPI_ERR_INVALID;

	memset(&tmp_msg, 0, sizeof(tmp_msg));
	tmp_msg.msg_id = msg_id;
	tmp_msg.size = size;
	memcpy(&tmp_msg.payload, data, size);

	mbox_integrity_stamp_outbound(dest_cpu, &tmp_msg);
	return mbox_phy_send(&tmp_msg, (MboxCoreId)dest_cpu, (MboxCoreId)src_cpu);
}

/**
 * @brief Waits for the response that matches the given cookie.
 */
uint8_t mbox_interface_wait_for_ack(uint32_t cookie, void *data)
{
	volatile Payload_packet *packet;
	uint32_t elapsed_us = 0;

	if (rx_response == NULL)
		return (uint8_t)VPI_ERR_UNINITED;

	packet = &rx_response->payload;
	while (!mbox_command_is_ack_set() || cookie != mbox_command_get_cookie()) {
		Apu_Mbox_Check_Command();

		if (!mbox_command_is_ack_set() || cookie != mbox_command_get_cookie()) {
			usleep(ACK_POLL_INTERVAL_US);
			elapsed_us += ACK_POLL_INTERVAL_US;
			if (elapsed_us >= ACK_TIMEOUT_US) {
				LOGE("ACK timeout (%u us) for cookie 0x%x", ACK_TIMEOUT_US, cookie);
				mbox_command_set_ack(false);
				mbox_command_set_data_ready(false);
				return (uint8_t)VPI_ERR_TIMEOUT;
			}
		}
	}

	if (mbox_command_is_data_ready()) {
		uint32_t safe_size = packet->payload_size;
		if (safe_size > MAX_ITEM)
			safe_size = MAX_ITEM;
		memcpy(data, packet->payload_data, safe_size);
	}

	mbox_command_set_ack(false);
	mbox_command_set_data_ready(false);
	return packet->resp_field.error_subcode_t;
}

/**
 * @brief Drains and dispatches all pending inbound mailbox messages.
 */
void mbox_interface_rx_dispatch(uint32_t ipi_src_mask)
{
	int32_t ret;
	(void)ipi_src_mask;

	if (rx_msg == NULL || rx_response == NULL || rx_command == NULL)
		return;

	while (mbox_phy_has_rx()) {
		ret = mbox_phy_pop_rx(rx_msg);
		if (ret != VPI_SUCCESS)
			break;

		ret = mbox_integrity_validate_inbound(src_cpu_id, rx_msg);
		if (ret != VPI_SUCCESS)
			continue;

		if ((rx_msg->msg_id == MB_CMD_RES_SUCCESS) || (rx_msg->msg_id == MB_CMD_GET_SUCCESS)) {
			memcpy(rx_response, rx_msg, mbox_copy_size(rx_msg));
		} else {
			memcpy(rx_command, rx_msg, mbox_copy_size(rx_msg));
		}

		ParseCommand(rx_msg->msg_id, &rx_msg->payload, rx_msg->size, src_cpu_id, dest_cpu_id);
	}
}

/**
 * @brief Returns the buffer holding the most recent inbound command.
 */
MboxPostMsg *mbox_interface_get_command_msg(void)
{
	return rx_command;
}
