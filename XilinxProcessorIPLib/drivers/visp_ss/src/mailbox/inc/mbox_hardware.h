/**
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (C) 2021 VeriSilicon Holdings Co., Ltd.
 *
 * @file mailbox.h
 * @brief Header of the enum &struct about mailbox ioctl interface
 * @author shenzhunwen <zhunwen.shen@verisilicon.com>
 */

#ifndef _MBOX_HARDWARE_H_
#define _MBOX_HARDWARE_H_

#include "mbox_fifo.h"

typedef enum MboxIOCFlag {
	MAILBOXIOC_WRITE_REG = 0x100,
	MAILBOXIOC_READ_REG,
	MAILBOXIOC_TRI_INT,
	MAILBOXIOC_WRITE_MSG_REG,
	MAILBOXIOC_READ_MSG_REG,
	DUMMY_MboxIOCFlag = 0xDEADFEED,
} MboxIOCFlag;

typedef enum MboxFifoFlag {
	MAILBOXFIFO_BUFFER,
	MAILBOXFIFO_ITEM_SIZE,
	MAILBOXFIFO_ITEM_TOTAL,
	MAILBOXFIFO_BUFFER_SIZE,
	MAILBOXFIFO_ITEM_STORED,
	MAILBOXFIFO_READ_OFFSET,
	MAILBOXFIFO_WRITE_OFFSET,
	DUMMY_MAILBOXFIFO = 0xDEADFEED,
} MboxFifoFlag;

typedef enum MboxMsgFlag {
	MAILBOXMSG_GROUP_ID = 0x10,
	MAILBOXMSG_MSG_ID,
	MAILBOXMSG_ACK,
	MAILBOXMSG_FLAGS,
	MAILBOXMSG_SIZE,
	MAILBOXMSG_PAYLOAD, //MAILBOXMSG_PAYLOAD must be the last one
	DUMMY_MAILBOXMSG = 0xDEADFEED,
} MboxMsgFlag;

typedef struct mailbox_reg_s {
	unsigned long val;
	FifoControl *fifo;
	MboxFifoFlag fifoflag;
} mailbox_reg_t;

typedef struct mailbox_reg_msg {
	unsigned long val;
	FifoControl *fifo;
	MboxMsgFlag msgflag;
} mailbox_reg_msg_t;


int mailbox_write_msg(MboxPostMsg *msg, FifoControl *fifo, int fd);
int mailbox_resd_msg(MboxPostMsg *msg, FifoControl *fifo, int fd);

#endif
