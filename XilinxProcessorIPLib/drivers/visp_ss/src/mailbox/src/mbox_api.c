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

#ifndef LOGTAG
	#define LOGTAG "DEV"
#endif
#include "mbox_api.h"

#include <stdlib.h>
#include <stdio.h>
#include "mbox_error_code.h"
#include "vlog.h"
#include "oslayer.h"

/*
 * Adjacent-pair memory layout matching NEW_RPU:
 * Index 0: RPU0->APU (RPU0 send)
 * Index 1: APU->RPU0 (RPU0 receive)
 * Index 2: RPU1->APU (RPU1 send)
 * Index 3: APU->RPU1 (RPU1 receive)
 * Index 4: RPU2->APU (RPU2 send)
 * Index 5: APU->RPU2 (RPU2 receive)
 * Index 6: RPU3->APU (RPU3 send)
 * Index 7: APU->RPU3 (RPU3 receive)
 */
const MboxPair default_pairs[] = {
	{MBOX_CORE_RPU0, MBOX_CORE_APU},  /* Index 0: RPU0 send */
	{MBOX_CORE_APU, MBOX_CORE_RPU0},  /* Index 1: RPU0 receive */
	{MBOX_CORE_RPU1, MBOX_CORE_APU},  /* Index 2: RPU1 send */
	{MBOX_CORE_APU, MBOX_CORE_RPU1},  /* Index 3: RPU1 receive */
	{MBOX_CORE_RPU2, MBOX_CORE_APU},  /* Index 4: RPU2 send */
	{MBOX_CORE_APU, MBOX_CORE_RPU2},  /* Index 5: RPU2 receive */
	{MBOX_CORE_RPU3, MBOX_CORE_APU},  /* Index 6: RPU3 send */
	{MBOX_CORE_APU, MBOX_CORE_RPU3},  /* Index 7: RPU3 receive */
};

#define NUM_CHANNELS (sizeof(default_pairs) / sizeof(default_pairs[0]))

/* 2D lookup: mbox_fifo_control[sender][receiver] -> MboxFifoCtrl* */
static MboxFifoCtrl *mbox_fifo_control[MBOX_CORE_MAX][MBOX_CORE_MAX] = {{NULL}};

/**
 * @brief Calculate memory address for a specific channel based on adjacent layout
 */
static uint32_t get_channel_base_addr(uint32_t base_addr, uint32_t channel_index,
				      uint32_t item_size, uint32_t item_total, uint32_t alignment)
{
	uint32_t fifo_ctrl_size = (sizeof(FifoControl) + alignment - 1) & ~(alignment - 1);
	uint32_t buffer_size = (item_size + alignment - 1) & ~(alignment - 1);
	uint32_t channel_size = fifo_ctrl_size + (buffer_size * item_total);
	return base_addr + (channel_index * channel_size);
}

/**
 * @brief Setup shared memory using adjacent-pair layout matching NEW_RPU
 */
static int32_t mbox_mem_map(MboxCoreId core_id, uint32_t shm_start_addr,
			    uint32_t item_size, uint32_t item_total)
{
	uint32_t i, j;
	const uint32_t alignment = 8;

	for (i = 0; i < NUM_CHANNELS; i++) {
		MboxCoreId sender_id = default_pairs[i].sender;
		MboxCoreId receiver_id = default_pairs[i].receiver;

		/* APU sets up all channels since it communicates with all RPUs */
		if (sender_id != core_id && receiver_id != core_id)
			continue;

		uint32_t channel_base = get_channel_base_addr(shm_start_addr, i,
							      item_size, item_total, alignment);
		uint32_t fifo_addr = channel_base;
		uint32_t aligned_fifo_ctrl_size = (sizeof(FifoControl) + alignment - 1) & ~(alignment - 1);
		uint32_t buffer_start = channel_base + aligned_fifo_ctrl_size;
		uint32_t aligned_buffer_size = (item_size + alignment - 1) & ~(alignment - 1);

		mbox_fifo_control[sender_id][receiver_id] = (MboxFifoCtrl *)osMalloc(sizeof(MboxFifoCtrl));
		if (mbox_fifo_control[sender_id][receiver_id] == NULL)
			return VPI_ERR_NOMEM;

		MboxFifoCtrl *ctrl = mbox_fifo_control[sender_id][receiver_id];
		ctrl->core_id = sender_id;
		ctrl->sender_id = sender_id;
		ctrl->receiver_id = receiver_id;
		ctrl->fifo = (FifoControl *)(uintptr_t)fifo_addr;

		/* Calculate per-slot buffer addresses */
		for (j = 0; j < item_total && j < MAX_MSGS_PER_BOX; j++) {
			ctrl->buffer_addresses[j] =
				(uint8_t *)(uintptr_t)(buffer_start + (j * aligned_buffer_size));
		}

		/* Initialize FifoControl in shared memory */
		FifoControl *shared_fifo = ctrl->fifo;
		shared_fifo->buffer = (void *)(uintptr_t)buffer_start;
		shared_fifo->item_size = item_size;
		shared_fifo->item_total = item_total;
		shared_fifo->buffer_size = item_total * item_size;
		shared_fifo->item_stored = 0;

		if (sender_id == core_id) {
			/* APU is sender: init write_offset only */
			shared_fifo->write_offset = 0;
		} else {
			/* APU is receiver: init read_offset only */
			shared_fifo->read_offset = 0;
		}

		fifo_info(ctrl);
	}

	return VPI_SUCCESS;
}

MboxFifoCtrl *vpi_mbox_init(MboxCoreId core_id, uint32_t shm_addr, uint32_t shm_block_size)
{
	if (core_id < 0 || core_id >= MBOX_CORE_MAX)
		return NULL;

	int ret = mbox_mem_map(core_id, shm_addr, sizeof(MboxPostMsg), MAX_MSGS_PER_BOX);
	if (ret != VPI_SUCCESS)
		return NULL;

	/* Return a non-NULL sentinel so callers know init succeeded.
	 * Actual FIFO controls are accessed via the 2D array internally. */
	return mbox_fifo_control[core_id][0] ? mbox_fifo_control[core_id][0] :
	       mbox_fifo_control[0][core_id];
}


int32_t mbox_fifoctrl_init_check(MboxFifoCtrl *mbox_fifo)
{
	return (mbox_fifo == NULL) ? VPI_ERR_UNINITED : VPI_SUCCESS;
}

int32_t mbox_core_id_check(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
			   MboxCoreId receiver_id)
{
	if (sender_id == receiver_id)
		return VPI_ERR_INVALID;
	if (sender_id < 0 || sender_id >= MBOX_CORE_MAX || receiver_id < 0 ||
	    receiver_id >= MBOX_CORE_MAX)
		return VPI_ERR_INVALID;
	return VPI_SUCCESS;
}

int32_t vpi_mbox_post(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId receiver_id,
		      MboxDriverCb mbox_driver_cb)
{
	int32_t ret;
	MboxCoreId sender_id = MBOX_CORE_APU;
	MboxFifoCtrl *ctrl;

	if (NULL == msg)
		return VPI_ERR_INVALID;

	ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return VPI_ERR_UNINITED;

	ret = fifo_write(msg, ctrl);
	if (ret != VPI_SUCCESS)
		return ret;

	if (mbox_driver_cb == NULL)
		return VPI_ERR_INVALID;

	mbox_driver_cb(receiver_id);
	return VPI_SUCCESS;
}


int32_t vpi_mbox_broadcast(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg,
			   MboxDriverCb mbox_driver_cb)
{
	MboxCoreId recv_core;
	int32_t ret;

	if (msg == NULL)
		return VPI_ERR_INVALID;

	for (recv_core = 0; recv_core < MBOX_CORE_MAX; recv_core++) {
		if (MBOX_CORE_APU != recv_core) {
			ret = vpi_mbox_post(mbox_fifo, msg, recv_core, mbox_driver_cb);
			if (ret != VPI_SUCCESS)
				return ret;
		}
	}
	return VPI_SUCCESS;
}

int32_t vpi_mbox_read(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId sender_id)
{
	MboxCoreId receiver_id = MBOX_CORE_APU;
	MboxFifoCtrl *ctrl;

	if (msg == NULL)
		return VPI_ERR_INVALID;

	ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return VPI_ERR_UNINITED;

	return fifo_read(msg, ctrl);
}

int32_t vpi_mbox_reset(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id)
{
	MboxFifoCtrl *ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return VPI_ERR_UNINITED;

	return fifo_reset(ctrl);
}

uint32_t vpi_mbox_get_stored(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
			     MboxCoreId receiver_id)
{
	MboxFifoCtrl *ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return VPI_ERR_UNINITED;

	return fifo_get_stored(ctrl);
}

bool vpi_mbox_is_full(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		      MboxCoreId receiver_id)
{
	MboxFifoCtrl *ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return true;
	return fifo_is_full(ctrl);
}

bool vpi_mbox_is_empty(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id)
{
	MboxFifoCtrl *ctrl = mbox_fifo_control[sender_id][receiver_id];
	if (ctrl == NULL)
		return true;
	return fifo_is_empty(ctrl);
}

void vpi_mbox_destory(MboxFifoCtrl *mbox_fifo)
{
	uint32_t s, r;
	for (s = 0; s < MBOX_CORE_MAX; s++) {
		for (r = 0; r < MBOX_CORE_MAX; r++) {
			if (mbox_fifo_control[s][r] != NULL) {
				osFree(mbox_fifo_control[s][r]);
				mbox_fifo_control[s][r] = NULL;
			}
		}
	}
}
