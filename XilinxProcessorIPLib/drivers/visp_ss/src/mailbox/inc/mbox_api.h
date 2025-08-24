/**
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (C) 2021 VeriSilicon Holdings Co., Ltd.
 *
 * @file mbox.h
 * @brief Header of Message Box
 * @author shenzhunwen<zhunwen.shen@verisilicon.com>
 */
#ifndef _MBOX_API_H_
#define _MBOX_API_H_

#include <stdbool.h>
#include <stdint.h>

#include "mbox_fifo.h"
/*
        Each core should be assigned an id number to tell
        the mbox who it is and give proper sender and receiver
        number of the message to clarify the source and
        destination of the incoming or outcoming message
        id number is defined based on MBOX_CORE_MAX
        count from 0 ~ MBOX_CORE_MAX-1
*/
/*
        The whole MBOX FIFO structure needs register size
        of mbox_fifo_block_size * MBOX_CORE_MAX *
                        (MBOX_CORE_MAX -1)
*/

/**
 * @brief Enum structure of Mbox core id
 */
typedef enum MboxCoreId {
	MBOX_CORE_RPU0 = 0,
	MBOX_CORE_RPU1 = 1,
	MBOX_CORE_RPU2 = 2,
	MBOX_CORE_RPU3 = 3,
	MBOX_CORE_APU = 4,
	MBOX_CORE_MAX,
	DUMMY_MBOX_CORE = 0xDEADFEED,
} MboxCoreId;

/*Target ID should be the index of XIpiPsu_Target TargetList[XIPIPSU_MAX_TARGETS] in XIpiPsu_Config, xipipsu_g.c*/
enum ipi_target_code_id {
	RPU0_TARGET = 1, //trigger 63 IPI Irq
	RPU1_TARGET = 2, //trigger 64 IPI Irq
	RPU2_TARGET = 3, //trigger 64 IPI Irq
	RPU3_TARGET = 4, //trigger 64 IPI Irq
	APU_TARGET = 5  //trigger 65 IPI Irq
};

extern uint32_t dest_cpu_id, src_cpu_id;


/**
 * @brief Structure of Mbox Control
 */
typedef struct MboxFifoCtrl {
	MboxCoreId core_id;
	MboxCoreId sender_id;
	MboxCoreId receiver_id;
	uint32_t buffer_address;
	FifoControl *fifo;
} __attribute((aligned(8))) MboxFifoCtrl;

/**
 * @brief Structure of Mbox ack message
 */
typedef struct MboxAckMsg {
	uint16_t meg_id;
	uint16_t result;
} __attribute((__packed__)) MboxAckMsg;

/**
 * @brief write data callback
 * @param receiver_id id of receiver core
 */
typedef void (*MboxDriverCb)(MboxCoreId receiver_id/*,int fd*/);

/**
 * @brief Initialize a Mbox and register the FIFO
 * @param core_id ID of current core
 * @param shm_addr Customized shared memory physical address
 * @param shm_block_size Customized shared memory block size
 * @return Return result
 * @retval MboxFifoCtrl point for succeed, NULL for failure
 */
MboxFifoCtrl *vpi_mbox_init(MboxCoreId core_id, uint32_t shm_addr,
			    uint32_t shm_block_size/*,int fd*/);

/**
 * @brief Post meg to the designated receiver
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param msg message to post
 * @param receiver_id core_id of the receiver
 * @param mbox_driver_cb callback function of mbox driver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t vpi_mbox_post(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId receiver_id/*,int fd*/,
		      MboxDriverCb mbox_driver_cb);

/**
 * @brief Boardcast meg to all other known cores
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param msg message to post
 * @param mbox_driver_cb callback function of mbox driver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t vpi_mbox_broadcast(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, /*,int fd*/
			   MboxDriverCb mbox_driver_cb);

/**
 * @brief Read meg from the designated sender
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param msg read message
 * @param sender_id core_id of the receiver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t vpi_mbox_read(MboxFifoCtrl *mbox_fifo, MboxPostMsg *msg, MboxCoreId sender_id/*,int fd*/);

/**
 * @brief Reset the designated FIFO with id to spectify
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval MBOX_SUCCESS for succeed, others for failure
 */
int32_t vpi_mbox_reset(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id/*,int fd*/);

/**
 * @brief Check designated FIFO stored number
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval stored numbers, others for failure
 */
uint32_t vpi_mbox_get_stored(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
			     MboxCoreId receiver_id/*,int fd*/);

/**
 * @brief Check designated FIFO is full or not
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval true for full, false for failure
 */
bool vpi_mbox_is_full(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		      MboxCoreId receiver_id/*,int fd*/);

/**
 * @brief Check designated FIFO is empty or not
 * @param mbox_fifo the MboxFifoCtrl pointer
 * @param sender_id core_id of the sender
 * @param receiver_id core_id of the receiver
 * @return Return result
 * @retval ture for empty, false for failure
 */
bool vpi_mbox_is_empty(MboxFifoCtrl *mbox_fifo, MboxCoreId sender_id,
		       MboxCoreId receiver_id/*,int fd*/);

/**
 * @brief Free everything including FIFO
 * @param mbox_fifo the MboxFifoCtrl pointer
 */
void vpi_mbox_destory(MboxFifoCtrl *mbox_fifo);

#endif //_MBOX_API_H_
