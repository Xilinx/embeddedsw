// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#ifndef _MBOX_INTERFACE_H_
#define _MBOX_INTERFACE_H_

#include "mbox_cmd.h"

/**
 * @brief Initializes the mailbox interface and its transport layer.
 *
 * @param[in] current_cpu_id Local core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_interface_init(MboxCoreId current_cpu_id);

/**
 * @brief Closes the mailbox interface and releases its buffers.
 */
void mbox_interface_close(void);

/**
 * @brief Builds, stamps, and sends a mailbox command.
 *
 * @param[in] cmd      Command identifier to send.
 * @param[in] data     Pointer to the command payload.
 * @param[in] size     Payload size in bytes.
 * @param[in] dest_cpu Destination core identifier.
 * @param[in] src_cpu  Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_interface_send_command(MBCmdId_E cmd, void *data, uint32_t size,
				   uint8_t dest_cpu, uint8_t src_cpu);

/**
 * @brief Builds, stamps, and sends a mailbox response.
 *
 * @param[in] res      Response identifier to send.
 * @param[in] data     Pointer to the response payload.
 * @param[in] size     Payload size in bytes.
 * @param[in] dest_cpu Destination core identifier.
 * @param[in] src_cpu  Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_interface_send_response(MBCmdId_E res, void *data, uint32_t size,
				    uint8_t dest_cpu, uint8_t src_cpu);

/**
 * @brief Sends a raw payload as a mailbox message without command framing.
 *
 * @param[in] msg_id   Message identifier to send.
 * @param[in] data     Pointer to the raw payload.
 * @param[in] size     Payload size in bytes.
 * @param[in] dest_cpu Destination core identifier.
 * @param[in] src_cpu  Source core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_interface_send_raw(uint32_t msg_id, void *data, uint32_t size,
			      uint8_t dest_cpu, uint8_t src_cpu);

/**
 * @brief Waits for the response that matches the given cookie.
 *
 * @param[in]  cookie Cookie identifying the awaited response.
 * @param[out] data   Buffer that receives the response payload.
 *
 * @return Response subcode on success, an error code otherwise.
 */
uint8_t mbox_interface_wait_for_ack(uint32_t cookie, void *data);

/**
 * @brief Drains and dispatches all pending inbound mailbox messages.
 *
 * @param[in] ipi_src_mask Interrupt source mask of the triggering IPI.
 */
void mbox_interface_rx_dispatch(uint32_t ipi_src_mask);

/**
 * @brief Returns the buffer holding the most recent inbound command.
 *
 * @return Pointer to the stored command message.
 */
MboxPostMsg *mbox_interface_get_command_msg(void);

#endif
