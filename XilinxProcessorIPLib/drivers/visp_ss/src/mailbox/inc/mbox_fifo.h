/**
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (C) 2021 VeriSilicon Holdings Co., Ltd.
 *
 * @file mbox_fifo.h
 * @brief Header of FIFO used by mbox
 * @author shenzhunwen <zhunwen.shen@verisilicon.com>
 */

#ifndef _MBOX_FIFO_H_
#define _MBOX_FIFO_H_

#include <stdbool.h>
#include <sys/_stdint.h>

#define MBOX_MAX_PAYLOAD (16396U+20U)
#define MAX_ITEM (16396U)

#define payload_extra_size 24

/**
 * @brief Structure of fifo control
 */
typedef struct FifoControl {
	uint64_t *buffer; //
	uint32_t virt_addr_dummy_res[2]; //This is to reserve virt address used by APU Linux
	uint32_t item_size;
	uint32_t item_total;
	uint32_t buffer_size;
	uint32_t item_stored;
	uint32_t read_offset;
	uint32_t write_offset;
} __attribute((aligned(8))) FifoControl;

/**
 * @brief Structure of init fifo data
 */
typedef struct FifoInitData {
	uint32_t buffer_addr;
	uint32_t item_size;
	uint32_t item_total;
	uint32_t buffer_size;
} FifoInitData;

/**
 * @brief Structure of Mbox post message
 */
typedef struct MboxPostMsg {
	uint8_t group_id;
	uint8_t ack : 1;
	uint8_t flags : 7;
	uint16_t msg_id;
	uint32_t size;
	uint8_t payload[MAX_ITEM + payload_extra_size];
} __attribute((aligned(8))) MboxPostMsg;

/**
 * @brief Enum structure of Mailbox Interrupt id
 */
typedef enum MboxIntId {
	MAILBOX_INTERRUPT_0,
	MAILBOX_INTERRUPT_1,
	MAILBOX_INTERRUPT_2,
	MAILBOX_INTERRUPT_3,
	DUMMY_MAILBOX_INTERRUPT = 0xDEADFEED,
} MboxIntId;

/**
 * @brief Initialize a Fifo with given input
 * @param fifo FifoControl pointer
 * @param fifodata Fifo init data for construct the fifo
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_init(FifoControl *fifo, FifoInitData *fifodata/*,int fd*/);

/**
 * @brief Write a given meg to the fifo
 * @param msg write message
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_write(MboxPostMsg *msg, FifoControl *fifo/*,int fd*/);

/**
 * @brief Read a given meg from the fifo
 * @param msg read message
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_read(MboxPostMsg *msg, FifoControl *fifo/*,int fd*/);

/**
 * @brief Reset the FIFO
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int fifo_reset(FifoControl *fifo/*,int fd*/);

/**
 * @brief Check current FIFO stored number
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t fifo_get_stored(FifoControl *fifo/*,int fd*/);

/**
 * @brief Check current FIFO is full or not
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval true for full, false for failure
 */
bool fifo_is_full(FifoControl *fifo/*,int fd*/);

/**
 * @brief Check current FIFO is empty or not
 * @param fifo FifoControl info pointer
 * @return Return result
 * @retval ture for empty, false for failure
 */
bool fifo_is_empty(FifoControl *fifo/*,int fd*/);

/**
 * @brief osFree FifoControl buffer data, only used if the buffer is assigned by
 * @brief osMalloc() or similar function
 * @param fifo FifoControl info pointer
 */
int fifo_buffer_free(FifoControl *fifo);

#endif //_MBOX_FIFO_H_
