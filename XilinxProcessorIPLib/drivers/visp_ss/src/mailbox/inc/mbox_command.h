// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#ifndef _MBOX_COMMAND_H_
#define _MBOX_COMMAND_H_

#include "mbox_cmd.h"

/**
 * @brief Sets the response acknowledgment flag.
 *
 * @param[in] ack New value of the acknowledgment flag.
 */
void mbox_command_set_ack(bool ack);

/**
 * @brief Sets the flag that marks response payload data as ready.
 *
 * @param[in] ready New value of the data-ready flag.
 */
void mbox_command_set_data_ready(bool ready);

/**
 * @brief Stores the cookie of the most recent response.
 *
 * @param[in] cookie Cookie value carried by the response.
 */
void mbox_command_set_cookie(uint32_t cookie);

/**
 * @brief Returns whether the response acknowledgment flag is set.
 *
 * @return true if the acknowledgment flag is set, false otherwise.
 */
bool mbox_command_is_ack_set(void);

/**
 * @brief Returns whether response payload data is ready.
 *
 * @return true if response data is ready, false otherwise.
 */
bool mbox_command_is_data_ready(void);

/**
 * @brief Returns the cookie of the most recent response.
 *
 * @return Stored response cookie value.
 */
uint32_t mbox_command_get_cookie(void);

/**
 * @brief Marks the pending command as an asynchronous command.
 */
void mbox_command_mark_async_cmd(void);

/**
 * @brief Marks the pending command as an asynchronous FuSa command.
 */
void mbox_command_mark_async_fusa_cmd(void);

/**
 * @brief Processes any pending inbound mailbox command.
 */
void Apu_Mbox_Check_Command(void);

/**
 * @brief Processes any pending inbound FuSa mailbox command.
 */
void Apu_Mbox_Check_FusaCommand(void);

/**
 * @brief Dispatches a received mailbox message to its handler.
 *
 * @param[in] cmd     Command identifier of the received message.
 * @param[in] data    Pointer to the received payload.
 * @param[in] size    Payload size in bytes.
 * @param[in] core_id Destination core identifier.
 * @param[in] src_cpu Source core identifier.
 *
 * @return Command-specific status code.
 */
uint32_t ParseCommand(MBCmdId_E cmd, void *data, uint32_t size, MboxCoreId core_id,
		      MboxCoreId src_cpu);

/**
 * @brief Processes a standard mailbox command on the APU.
 *
 * @param[in] cmd     Command identifier to process.
 * @param[in] data    Pointer to the command payload.
 * @param[in] size    Payload size in bytes.
 * @param[in] core_id Destination core identifier.
 * @param[in] src_cpu Source core identifier.
 *
 * @return Command-specific status code.
 */
uint32_t ApuProcessCommand(MBCmdId_E cmd, void *data, uint32_t size, MboxCoreId core_id,
			   MboxCoreId src_cpu);

/**
 * @brief Processes a FuSa mailbox command on the APU.
 *
 * @param[in] cmd     Command identifier to process.
 * @param[in] data    Pointer to the command payload.
 * @param[in] size    Payload size in bytes.
 * @param[in] core_id Destination core identifier.
 * @param[in] src_cpu Source core identifier.
 *
 * @return Command-specific status code.
 */
uint32_t ApuProcessFusaCommand(MBCmdId_E cmd, void *data, uint32_t size, MboxCoreId core_id,
			       MboxCoreId src_cpu);

#endif
