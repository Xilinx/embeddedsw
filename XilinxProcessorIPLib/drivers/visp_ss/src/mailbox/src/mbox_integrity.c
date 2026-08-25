// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#include "mbox_integrity.h"

#include "mbox_checksum.h"
#include "mbox_error_code.h"
#include "mbox_seq.h"

/**
 * @brief Initializes the mailbox integrity state.
 */
void mbox_integrity_init(void)
{
#if defined(ENABLE_MBOX_INTEGRITY)
	mbox_seq_init_all();
#endif
}

/**
 * @brief Stamps sequence counter and checksum onto an outbound message.
 */
void mbox_integrity_stamp_outbound(MboxCoreId receiver_id, MboxPostMsg *msg)
{
#if defined(ENABLE_MBOX_INTEGRITY)
	mbox_seq_assign_outbound(receiver_id, &msg->payload);
	msg->checksum = mbox_checksum_calculate(msg);
#else
	(void)receiver_id;
	(void)msg;
#endif
}

/**
 * @brief Validates the sequence counter and checksum of an inbound message.
 */
int32_t mbox_integrity_validate_inbound(MboxCoreId src_core_id, MboxPostMsg *msg)
{
	if (msg->payload.payload_size > MAX_ITEM)
		return VPI_ERR_INVALID;

#if defined(ENABLE_MBOX_INTEGRITY)
	if (msg->checksum != mbox_checksum_calculate(msg))
		return VPI_ERR_CHECKSUM;

	if (msg->msg_id == MB_CMD_RES_SUCCESS &&
	    msg->payload.resp_field.processed_cmdid == APU_2_RPU_MB_CMB_INIT_FIRMWARE)
		mbox_seq_mark_resync(src_core_id);

	return mbox_seq_validate_inbound(src_core_id, msg);
#else
	(void)src_core_id;
	return VPI_SUCCESS;
#endif
}
