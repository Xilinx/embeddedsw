/**
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * @file mbox_seq.c
 * @brief Mailbox per-core sequence counter integrity checks
 */

#include "mbox_seq.h"
#include "mbox_error_code.h"
#include "vlog.h"

#include <stdint.h>
#include <string.h>

#define LOGTAG "MSEQ"

#define MBOX_SEQ_MAX INT32_MAX

/*
 * Bounds for the "unified receiver rule" shared with visp_mbox
 * a small forward gap is normal message loss and is accepted/resynced
 * immediately, while a backward/stale value or an implausibly large gap is
 * only trusted after it repeats, so a forced resync only happens after a run
 * of consecutive irrecoverable mismatches.
 */
#define MBOX_SEQ_FORWARD_GAP_MAX 1024U
#define MBOX_SEQ_MISMATCH_ESCALATE 8U

typedef struct mbox_seq_state {
	uint32_t inbound_seq;
	uint32_t outbound_seq;
	uint32_t seq_mismatch_count;
	bool     seq_resync_pending;
} mbox_seq_state_t;

static mbox_seq_state_t seq_state[MBOX_CORE_MAX];

static uint32_t mbox_seq_increment(uint32_t seq)
{
	return (seq >= MBOX_SEQ_MAX) ? 0U : (seq + 1U);
}

void mbox_seq_init_all(void)
{
	memset(seq_state, 0, sizeof(seq_state));
}

void mbox_seq_assign_outbound(MboxCoreId remote_core_id, Payload_packet *payload)
{
	if (remote_core_id >= MBOX_CORE_MAX || !payload)
		return;

	payload->seq_counter = seq_state[remote_core_id].outbound_seq;
	seq_state[remote_core_id].outbound_seq =
		mbox_seq_increment(seq_state[remote_core_id].outbound_seq);
}

int32_t mbox_seq_validate_inbound(MboxCoreId remote_core_id, const MboxPostMsg *msg)
{
	uint32_t expected_seq;
	uint32_t received_seq;
	uint32_t gap;

	if (remote_core_id >= MBOX_CORE_MAX || !msg)
		return VPI_ERR_INVALID;

	received_seq = msg->payload.seq_counter;

	if (seq_state[remote_core_id].seq_resync_pending) {
		seq_state[remote_core_id].seq_resync_pending = false;
		seq_state[remote_core_id].seq_mismatch_count = 0;
		seq_state[remote_core_id].inbound_seq = mbox_seq_increment(received_seq);
		return VPI_SUCCESS;
	}

	expected_seq = seq_state[remote_core_id].inbound_seq;
	if (received_seq == expected_seq) {
		seq_state[remote_core_id].seq_mismatch_count = 0;
		seq_state[remote_core_id].inbound_seq = mbox_seq_increment(received_seq);
		return VPI_SUCCESS;
	}

	/* Circular forward distance from expected to received (handles rollover). */
	gap = (received_seq - expected_seq) & (uint32_t)MBOX_SEQ_MAX;
	if (gap > 0 && gap <= MBOX_SEQ_FORWARD_GAP_MAX) {
		LOGW("Mbox sequence gap from core %d: expected %u got %u (dropped %u), resyncing\r\n",
		     remote_core_id, expected_seq, received_seq, gap);
		seq_state[remote_core_id].seq_mismatch_count = 0;
		seq_state[remote_core_id].inbound_seq = mbox_seq_increment(received_seq);
		return VPI_SUCCESS;
	}

	/* Backward/stale/huge-forward gap: escalate to a forced resync only
	 * after repeated mismatches so a single corrupted value can't wedge
	 * the link, but a real desync still self-heals.
	 */
	seq_state[remote_core_id].seq_mismatch_count++;
	if (seq_state[remote_core_id].seq_mismatch_count >= MBOX_SEQ_MISMATCH_ESCALATE) {
		LOGE("Mbox %u consecutive sequence mismatches from core %d, forcing resync at %u\r\n",
		     seq_state[remote_core_id].seq_mismatch_count, remote_core_id, received_seq);
		seq_state[remote_core_id].seq_mismatch_count = 0;
		seq_state[remote_core_id].inbound_seq = mbox_seq_increment(received_seq);
		return VPI_SUCCESS;
	}

	LOGE("Mbox sequence mismatch from core %d: expected %u got %u msg_id=%u (count=%u)\r\n",
	     remote_core_id, expected_seq, received_seq, msg->msg_id,
	     seq_state[remote_core_id].seq_mismatch_count);
	return VPI_ERR_SEQ_MISMATCH;
}

void mbox_seq_mark_resync(MboxCoreId remote_core_id)
{
	if (remote_core_id >= MBOX_CORE_MAX)
		return;

	seq_state[remote_core_id].seq_resync_pending = true;
}
