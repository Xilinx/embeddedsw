/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define INVALID_PATH 0xFFFF
static uint16_t g_mem_list[MBOX_CORE_MAX][MBOX_CORE_MAX] = {{INVALID_PATH, 0, 1},
	{2, INVALID_PATH, INVALID_PATH},
	{3, INVALID_PATH, INVALID_PATH}
};

static int32_t mbox_mem_map(MboxFifoCtrl *mbox_fifo, MboxCoreId core_id,
			    uint32_t shm_start_addr, uint32_t shm_block_size)
{
	size_t sender_id;
	size_t receiver_id;
	size_t mlist = 0U;
	uint32_t buffer_addr = shm_start_addr; // Initialize buffer_addr
	FifoInitData *init_fifo = NULL;
	int ret = VPI_SUCCESS; // Define ret variable for error handling

	init_fifo = (FifoInitData *)osMalloc(sizeof(FifoInitData));
	if (NULL == init_fifo)
		return VPI_ERR_NOMEM;

	init_fifo->item_size = sizeof(MboxPostMsg);
	init_fifo->buffer_size = shm_block_size - ((sizeof(FifoControl) + 7U) / 8U * 8U);
	init_fifo->item_total = init_fifo->buffer_size / init_fifo->item_size;

	for (sender_id = MBOX_CORE_RPU0; sender_id < MBOX_CORE_MAX; sender_id++) {
		for (receiver_id = MBOX_CORE_RPU0; receiver_id < MBOX_CORE_MAX; receiver_id++) {
			// Check for invalid communication paths and ignore such combinations

			if (sender_id == receiver_id ||

			    (sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU1) ||
			    (sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU2) ||
			    (sender_id == MBOX_CORE_RPU0 && receiver_id == MBOX_CORE_RPU3) ||

			    (sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU0) ||
			    (sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU2) ||
			    (sender_id == MBOX_CORE_RPU1 && receiver_id == MBOX_CORE_RPU3) ||

			    (sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU0) ||
			    (sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU1) ||
			    (sender_id == MBOX_CORE_RPU2 && receiver_id == MBOX_CORE_RPU3) ||

			    (sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU0) ||
			    (sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU1) ||
			    (sender_id == MBOX_CORE_RPU3 && receiver_id == MBOX_CORE_RPU2)

			   )
				continue;

			// Assign the mbox_fifo[] with the address of memory if core_id is either sender or receiver
			if (sender_id == core_id || receiver_id == core_id) {
				g_mem_list[sender_id][receiver_id] = mlist;
				(mbox_fifo + mlist)->core_id = core_id;
				(mbox_fifo + mlist)->buffer_address =
					buffer_addr + ((sizeof(FifoControl) + 7U) / 8U * 8U);
				(mbox_fifo + mlist)->sender_id = sender_id;
				(mbox_fifo + mlist)->receiver_id = receiver_id;
				(mbox_fifo + mlist)->fifo =
					(FifoControl *)(uintptr_t)(buffer_addr); //buffer_addr == 0 for first time
				init_fifo->buffer_addr = (mbox_fifo + mlist)->buffer_address;

				/*TODO: If core_id is sender, only then init the fifo, if receiver ignore the fifo_init()*/
				ret = fifo_init((mbox_fifo + mlist)->fifo, init_fifo);
				if (ret != VPI_SUCCESS)
					return ret;

				fifo_info((mbox_fifo + mlist)->fifo);

				/*Increment mlist only if core_id is sender or receiver
				 * */
				mlist++;
			}

			/*If the communication path is valid, increment the buffer_addr,
			 * even if core_id is not sender or receiver.
			 */

			buffer_addr += shm_block_size;
		}
	}

	osFree(init_fifo);

	return VPI_SUCCESS;
}

MboxFifoCtrl *vpi_mbox_init(MboxCoreId core_id, uint32_t shm_addr, uint32_t shm_block_size)
{
	MboxFifoCtrl *mbox_fifo = NULL;
	if (core_id < 0 || core_id >= MBOX_CORE_MAX) {
		// Invalid core_id
		return NULL;
	}

	mbox_fifo = (MboxFifoCtrl *)osMalloc(sizeof(MboxFifoCtrl) *
					     ((MBOX_CORE_MAX - 1) * 2));

	if (NULL == mbox_fifo) {
		// Memory allocation failure
		return NULL;
	}

	int ret = mbox_mem_map(mbox_fifo, core_id, shm_addr, shm_block_size);
	if (ret != VPI_SUCCESS) {
		osFree(mbox_fifo);
		return NULL;
	}

	return mbox_fifo;
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
	if (sender_id != mbox_fifo->core_id && receiver_id != mbox_fifo->core_id)
		return VPI_ERR_INVALID;
	return VPI_SUCCESS;
}

int32_t vpi_mbox_post(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId receiver_id,
		      MboxDriverCb mbox_driver_cb)
{
	uint16_t ret;

	if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
		return VPI_ERR_UNINITED;

	if (NULL == msg)
		return VPI_ERR_INVALID;

	if (VPI_ERR_INVALID == mbox_core_id_check(mbox_fifo, mbox_fifo->core_id, receiver_id))
		return VPI_ERR_INVALID;

	ret = fifo_write(msg, (mbox_fifo + g_mem_list[mbox_fifo->core_id][receiver_id])->fifo);

	if (ret != VPI_SUCCESS)
		return ret;

	if (mbox_driver_cb == NULL) // Check if mbox_driver_cb is NULL before calling it
		return VPI_ERR_INVALID;

	mbox_driver_cb(receiver_id);
	return VPI_SUCCESS;
}


int32_t vpi_mbox_broadcast(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg,
			   MboxDriverCb mbox_driver_cb)
{
	MboxCoreId recv_core;
	int32_t ret;

	if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
		return VPI_ERR_UNINITED;
	if (msg == NULL)
		return VPI_ERR_INVALID;

	for (recv_core = 0; recv_core < MBOX_CORE_MAX; recv_core++)
		if (mbox_fifo->core_id != recv_core) {
			ret = vpi_mbox_post(mbox_fifo, msg, recv_core, mbox_driver_cb);
			if (ret != VPI_SUCCESS)
				return ret;
		}
	return VPI_SUCCESS;
}

int32_t vpi_mbox_read(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId sender_id)
{
	if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
		return VPI_ERR_UNINITED;
	if (msg == NULL)
		return VPI_ERR_INVALID;
	if (VPI_ERR_INVALID ==
	    mbox_core_id_check(mbox_fifo, sender_id, mbox_fifo->core_id))
		return VPI_ERR_INVALID;

	return fifo_read(msg,
			 (mbox_fifo + g_mem_list[sender_id][mbox_fifo->core_id])
			 ->fifo);
}

int32_t vpi_mbox_reset(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id)
{
	if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
		return VPI_ERR_UNINITED;
	if (VPI_ERR_INVALID ==
	    mbox_core_id_check(mbox_fifo, sender_id, receiver_id))
		return VPI_ERR_INVALID;

	return fifo_reset((mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}

uint32_t vpi_mbox_get_stored(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
			     MboxCoreId receiver_id)
{
	if (VPI_ERR_UNINITED == mbox_fifoctrl_init_check(mbox_fifo))
		return VPI_ERR_UNINITED;
	if (VPI_ERR_INVALID ==
	    mbox_core_id_check(mbox_fifo, sender_id, receiver_id))
		return VPI_ERR_INVALID;

	return fifo_get_stored(
		       (mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}

bool vpi_mbox_is_full(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		      MboxCoreId receiver_id)
{
	return fifo_is_full((mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}

bool vpi_mbox_is_empty(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id)
{
	return fifo_is_empty(
		       (mbox_fifo + g_mem_list[sender_id][receiver_id])->fifo);
}

void vpi_mbox_destory(MboxFifoCtrl *mbox_fifo)
{
	uint16_t i;
	for (i = 0; i < (MBOX_CORE_MAX - 1) * 2; i++)
		osFree((mbox_fifo + i)->fifo);
}
