// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#ifndef _MBOX_INTEGRITY_H_
#define _MBOX_INTEGRITY_H_

#include "mbox_cmd.h"

/**
 * @brief Initializes the mailbox integrity state.
 */
void mbox_integrity_init(void);

/**
 * @brief Stamps sequence counter and checksum onto an outbound message.
 *
 * @param[in]     receiver_id Destination core identifier.
 * @param[in,out] msg         Message to stamp in place.
 */
void mbox_integrity_stamp_outbound(MboxCoreId receiver_id, MboxPostMsg *msg);

/**
 * @brief Validates the sequence counter and checksum of an inbound message.
 *
 * @param[in] src_core_id Source core identifier.
 * @param[in] msg         Received message to validate.
 *
 * @return VPI_SUCCESS if the message is valid, an error code otherwise.
 */
int32_t mbox_integrity_validate_inbound(MboxCoreId src_core_id, MboxPostMsg *msg);

#endif
