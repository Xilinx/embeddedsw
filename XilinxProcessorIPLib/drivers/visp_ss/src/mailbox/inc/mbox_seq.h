/**
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * @file mbox_seq.h
 * @brief Header of mailbox per-core sequence counter integrity checks
 */
#ifndef _MBOX_SEQ_H_
#define _MBOX_SEQ_H_

#include "mbox_fifo.h"
#include "mbox_api.h"

/**
 * @brief Initialize sequence counter state for all mbox cores
 */
void mbox_seq_init_all(void);

/**
 * @brief Assign the next outbound sequence number to a payload
 * @param remote_core_id Target remote core ID
 * @param payload Pointer to Payload_packet to stamp
 */
void mbox_seq_assign_outbound(MboxCoreId remote_core_id, Payload_packet *payload);

/**
 * @brief Validate the sequence number of an inbound message
 * @param remote_core_id Source remote core ID
 * @param msg Pointer to received MboxPostMsg
 * @return VPI_SUCCESS if valid/accepted, VPI_ERR_SEQ_MISMATCH otherwise
 */
int32_t mbox_seq_validate_inbound(MboxCoreId remote_core_id, const MboxPostMsg *msg);

/**
 * @brief Force the inbound counter for a remote core to resync on next receive
 * @param remote_core_id Remote core ID whose next message defines the new baseline
 */
void mbox_seq_mark_resync(MboxCoreId remote_core_id);

#endif // _MBOX_SEQ_H_
