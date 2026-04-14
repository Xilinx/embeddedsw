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


#include "mbox_fifo.h"
#include "mbox_api.h"
#include "sensor_cmd.h"

#include <stdlib.h>
#include <string.h>

#include "mbox_error_code.h"
#include "oslayer.h"

/**
 * @brief Calculate the actual size of a mailbox message
 * @param msg Pointer to the mailbox message
 * @return Actual size in bytes (header + variable payload size)
 */
static uint32_t get_mbox_message_size(MboxPostMsg *msg) {
	return sizeof(MboxPostMsg) - MAX_ITEM + msg->payload.payload_size;
}

void fifo_info(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo = mbox_fifo->fifo;
	xil_printf("\tfifo_control location (%lx)\n", (unsigned long)fifo);
	xil_printf("\tbuffer (%lx) %lx\n", (unsigned long)&fifo->buffer, (unsigned long)fifo->buffer);
	xil_printf("\titem_size (%lx) %lu\n", (unsigned long)&fifo->item_size, fifo->item_size);
	xil_printf("\titem_total (%lx) %lu\n", (unsigned long)&fifo->item_total, fifo->item_total);
	xil_printf("\tbuffer_size (%lx) %lu\n", (unsigned long)&fifo->buffer_size, fifo->buffer_size);
	xil_printf("\twrite_index (%lx) %lu\n", (unsigned long)&fifo->write_offset, fifo->write_offset);
	xil_printf("\tread_index (%lx) %lu\n", (unsigned long)&fifo->read_offset, fifo->read_offset);
	uint32_t stored = (fifo->write_offset - fifo->read_offset + fifo->item_total) % fifo->item_total;
	xil_printf("\tstored_items: %lu\n", stored);
}

int32_t fifo_init(MboxFifoCtrl *mbox_fifo, FifoInitData *init_fifo)
{
	FifoControl *fifo;
	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL || init_fifo == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;
	fifo->buffer = (void *)(uintptr_t)(init_fifo->buffer_addr);
	fifo->item_size = init_fifo->item_size;
	fifo->item_total = init_fifo->item_total;
	fifo->buffer_size = init_fifo->buffer_size;
	fifo->item_stored = 0;
	/* Note: only the owner should init its offset.
	 * Sender inits write_offset, receiver inits read_offset.
	 * Both are set here for legacy compat; caller may override. */
	fifo->write_offset = 0;
	fifo->read_offset = 0;

	return VPI_SUCCESS;
}

int32_t fifo_write(MboxPostMsg *msg, MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	uint32_t current_write_index, current_read_index, next_write_index;
	uint8_t *write_pos;
	uint32_t actual_size;

	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL || msg == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;

	/* DMB: Ensure we read latest index values from shared memory */
	dmb_memory_barrier();

	current_write_index = fifo->write_offset;
	current_read_index = fifo->read_offset;
	next_write_index = (current_write_index + 1) % fifo->item_total;

	/* Check if FIFO is full (circular buffer: full when next write == read) */
	if (next_write_index == current_read_index)
		return VPI_ERR_FULL;

	write_pos = mbox_fifo->buffer_addresses[current_write_index];
	actual_size = get_mbox_message_size(msg);
	memcpy(write_pos, msg, actual_size);

	/* DMB: Ensure data write completes before updating index */
	dmb_memory_barrier();

	fifo->write_offset = next_write_index;

	/* DMB: Ensure index update is visible to remote core */
	dmb_memory_barrier();

	return VPI_SUCCESS;
}

int32_t fifo_read(MboxPostMsg *msg, MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	uint32_t current_write_index, current_read_index, next_read_index;
	uint8_t *read_pos;
	uint32_t actual_size;

	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;

	/* DMB: Ensure we read latest index values from shared memory */
	dmb_memory_barrier();

	current_write_index = fifo->write_offset;
	current_read_index = fifo->read_offset;

	/* Check if FIFO is empty (circular buffer: empty when write == read) */
	if (current_write_index == current_read_index)
		return VPI_ERR_EMPTY;

	read_pos = mbox_fifo->buffer_addresses[current_read_index];
	actual_size = get_mbox_message_size((MboxPostMsg *)read_pos);
	memcpy(msg, read_pos, actual_size);

	/* DMB: Ensure data read completes before updating index */
	dmb_memory_barrier();

	next_read_index = (current_read_index + 1) % fifo->item_total;
	fifo->read_offset = next_read_index;

	/* DMB: Ensure index update is visible to remote core */
	dmb_memory_barrier();

	return VPI_SUCCESS;
}

int32_t fifo_reset(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;
	/* Reset write_index to 0; read_index is owned by remote core */
	fifo->write_offset = 0;

	return VPI_SUCCESS;
}

uint32_t fifo_get_stored(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	uint32_t write_index, read_index;

	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;
	dmb_memory_barrier();
	write_index = fifo->write_offset;
	read_index = fifo->read_offset;
	return (write_index - read_index + fifo->item_total) % fifo->item_total;
}

bool fifo_is_full(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	uint32_t write_index, read_index;

	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return true;

	fifo = mbox_fifo->fifo;
	dmb_memory_barrier();
	write_index = fifo->write_offset;
	read_index = fifo->read_offset;
	return ((write_index + 1) % fifo->item_total) == read_index;
}

bool fifo_is_empty(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;

	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return true;

	fifo = mbox_fifo->fifo;
	dmb_memory_barrier();
	return fifo->write_offset == fifo->read_offset;
}

int32_t fifo_buffer_free(MboxFifoCtrl *mbox_fifo)
{
	FifoControl *fifo;
	if (mbox_fifo == NULL || mbox_fifo->fifo == NULL)
		return VPI_ERR_INVALID;

	fifo = mbox_fifo->fifo;
	osFree(fifo->buffer);
	return VPI_SUCCESS;
}
