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


#include "mbox_fifo.h"
#include "sensor_cmd.h"

#include <stdlib.h>
#include <string.h>

#include "mbox_error_code.h"
#include "oslayer.h"
void fifo_info(FifoControl *fifo)
{
	xil_printf("\tbuffer (%x) %x\n", &fifo->buffer, fifo->buffer);
	xil_printf("\titem_size (%x) %lu\n", &fifo->item_size, fifo->item_size);
	xil_printf("\titem_total (%x) %lu\n", &fifo->item_total, fifo->item_total);
	xil_printf("\tbuffer_size (%x) %lu\n", &fifo->buffer_size, fifo->buffer_size);
	xil_printf("\titem_stored (%x) %lu\n", &fifo->item_stored, fifo->item_stored);
	xil_printf("\tread_offset (%x) %lu\n", &fifo->read_offset, fifo->read_offset);
	xil_printf("\twrite_offset (%x) %lu\n", &fifo->write_offset, fifo->write_offset);
}

int fifo_init(FifoControl *fifo, FifoInitData *init_fifo)
{
	if (fifo == NULL || init_fifo == NULL)
		return VPI_ERR_INVALID;

	fifo->buffer = (void *)(uintptr_t)(init_fifo->buffer_addr);
	fifo->item_size = init_fifo->item_size;
	fifo->item_total = init_fifo->item_total;
	fifo->buffer_size = init_fifo->buffer_size;
#if 1    //TODO: do not init these values if this devices boots second.... or syncronize the init's to avoid overriding
	fifo->item_stored = 0;
	fifo->read_offset = 0;
	fifo->write_offset = 0;
#endif
	//fifo_info(fifo);
	return VPI_SUCCESS;
}

int fifo_write(MboxPostMsg *msg, FifoControl *fifo)
{
	if (fifo == NULL || msg == NULL)
		return VPI_ERR_INVALID;
	if (fifo->item_stored >= fifo->item_total)
		return VPI_ERR_FULL;
	// fifo_info(fifo);
	// xil_printf("APU write msg->size:%d.\r\n", msg->size);
	memcpy((void *)((0xFFFFFFFF & (unsigned int)fifo->buffer) + fifo->write_offset), msg,
	       sizeof(MboxPostMsg) - sizeof(Payload_packet) + msg->size);

	fifo->write_offset += fifo->item_size;
	if (fifo->write_offset >= fifo->buffer_size)
		fifo->write_offset = 0;
	fifo->item_stored++;
	return VPI_SUCCESS;
}

int fifo_read(MboxPostMsg *msg, FifoControl *fifo)
{

	if (fifo == NULL)
		return VPI_ERR_INVALID;
	if (fifo->item_stored == 0)
		return VPI_ERR_EMPTY;

	memcpy(msg, (void *)((0xFFFFFFFF & (unsigned int)fifo->buffer) + fifo->read_offset),
	       fifo->item_size);
	fifo->read_offset += fifo->item_size;
	if (fifo->read_offset >= fifo->buffer_size)
		fifo->read_offset = 0;
	fifo->item_stored--;
	// fifo_info(fifo);
	return VPI_SUCCESS;
}

int fifo_reset(FifoControl *fifo)
{
	if (fifo == NULL)
		return VPI_ERR_INVALID;

	fifo->item_stored = 0;
	fifo->read_offset = 0;
	fifo->write_offset = 0;

	return VPI_SUCCESS;
}

uint32_t fifo_get_stored(FifoControl *fifo)
{
	if (fifo == NULL)
		return VPI_ERR_INVALID;
	return fifo->item_stored;
}

bool fifo_is_full(FifoControl *fifo)
{
	return fifo->item_stored >= fifo->item_total ? true : false;
}

bool fifo_is_empty(FifoControl *fifo)
{
	//	xil_printf("fifo ctrl addresses %x,%x,%x,%x,->item stroed %x,read offset-> %x \n",&fifo->buffer,&fifo->item_size,&fifo->item_total,
	//	&fifo->buffer_size,&fifo->item_stored,&fifo->read_offset);
	return fifo->item_stored == 0 ? true : false;
}

int fifo_buffer_free(FifoControl *fifo)
{
	if (fifo == NULL)
		return VPI_ERR_INVALID;
	osFree(fifo->buffer);
	return VPI_SUCCESS;
}
