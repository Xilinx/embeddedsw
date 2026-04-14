/**
 * Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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

#define payload_extra_size 24
#define MAX_ITEM (16396U+4U)

#define MAX_MSGS_PER_BOX 30

/* Forward declaration for MboxFifoCtrl */
struct MboxFifoCtrl;

/**
 * @brief Payload type enum (CMD or RESP)
 */
typedef enum payload_type {
	CMD,
	RESP,
	dummy_pt = 0xDEADFEED,
} Payload_type;

/**
 * @brief Response field structure
 */
typedef struct response_field {
	uint32_t processed_cmdid;
	uint32_t error_subcode_t;
} response_field_t;

/**
 * @brief Payload packet structure - must match NEW_RPU binary layout
 */
typedef struct payload_template {
	Payload_type type;
	uint32_t cookie;
	uint32_t payload_size;
	uint32_t reserved[1]; /* padding for 8-byte alignment - matches NEW_RPU */
	response_field_t resp_field;
	uint8_t payload_data[MAX_ITEM];
} Payload_packet;

/**
 * @brief Structure of fifo control (shared memory - must match RPU layout)
 */
typedef struct FifoControl {
	uint64_t *buffer; //
	uint32_t virt_addr_dummy_res[2]; //This is to reserve virt address used by APU Linux
	uint32_t item_size;
	uint32_t item_total;
	uint32_t buffer_size;
	uint32_t item_stored;
	volatile uint32_t read_offset;
	volatile uint32_t write_offset;
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
 * @brief Structure of Mbox post message - must match NEW_RPU binary layout
 */
typedef struct MboxPostMsg {
	uint32_t media_server_flags;
	uint32_t reserved[3]; /* padding for 8-byte alignment */
	uint32_t msg_id;
	uint32_t size;
	Payload_packet payload;
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
 * @param mbox_fifo MboxFifoCtrl pointer
 * @param init_fifo Fifo init data for construct the fifo
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t fifo_init(struct MboxFifoCtrl *mbox_fifo, FifoInitData *init_fifo);

/**
 * @brief Print FIFO information
 * @param mbox_fifo MboxFifoCtrl info pointer
 */
void fifo_info(struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Write a given meg to the fifo
 * @param msg write message
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t fifo_write(MboxPostMsg *msg, struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Read a given meg from the fifo
 * @param msg read message
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t fifo_read(MboxPostMsg *msg, struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Reset the FIFO
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t fifo_reset(struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Check current FIFO stored number
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t fifo_get_stored(struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Check current FIFO is full or not
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval true for full, false for failure
 */
bool fifo_is_full(struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief Check current FIFO is empty or not
 * @param mbox_fifo MboxFifoCtrl info pointer
 * @return Return result
 * @retval true for empty, false for failure
 */
bool fifo_is_empty(struct MboxFifoCtrl *mbox_fifo);

/**
 * @brief osFree FifoControl buffer data, only used if the buffer is assigned by
 * @brief osMalloc() or similar function
 * @param mbox_fifo MboxFifoCtrl info pointer
 */
int32_t fifo_buffer_free(struct MboxFifoCtrl *mbox_fifo);

#endif //_MBOX_FIFO_H_
