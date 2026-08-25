// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#ifndef _MBOX_PHY_H_
#define _MBOX_PHY_H_

#include <stdint.h>

#include "mbox_cmd.h"

/**
 * @brief Initializes the mailbox transport layer for the local core.
 *
 * @param[in] current_cpu_id Local core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_phy_init(MboxCoreId current_cpu_id);

/**
 * @brief Closes the mailbox transport layer and frees its buffers.
 */
void mbox_phy_close(void);

/**
 * @brief Posts a fully stamped message over the mailbox transport.
 *
 * @param[in] msg         Message to post.
 * @param[in] receiver_id Destination core identifier.
 * @param[in] core_id     Sending core identifier.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_phy_send(MboxPostMsg *msg, MboxCoreId receiver_id, MboxCoreId core_id);

/**
 * @brief Pops the next received message from the mailbox FIFO.
 *
 * @param[out] out_msg Buffer that receives the message.
 *
 * @return VPI_SUCCESS on success, an error code otherwise.
 */
int32_t mbox_phy_pop_rx(MboxPostMsg *out_msg);

/**
 * @brief Returns whether the mailbox FIFO has a pending message.
 *
 * @return true if a message is available, false otherwise.
 */
bool mbox_phy_has_rx(void);

#endif
