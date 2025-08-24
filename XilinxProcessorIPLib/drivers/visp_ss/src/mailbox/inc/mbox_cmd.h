/**
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (C) 2021 VeriSilicon Holdings Co., Ltd.
 *
 * @file mbox_cmd.h
 * @brief Header of  the interface of Mailbox cmd
 * @author shenzhunwen <zhunwen.shen@verisilicon.com>
 */
#ifndef _MBOX_CMD_H_
#define _MBOX_CMD_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <xil_types.h>
//#include "vsi_command.h"
#include "mbox_api.h"
#include "mbox_error_code.h"
#include "mbox_fifo.h"
#include "mbox_hardware.h"
#include "sensor_cmd.h"


#define MAX_MSGS_PER_BOX     (12U)
#define MBOX_FIFO_BLOCK_SIZE (sizeof(FifoControl) + sizeof(MboxPostMsg)*MAX_MSGS_PER_BOX)

/************************* Test Configuration ********************************/

/* Interrupt Controller device ID */
#define INTC_DEVICE_ID	XPAR_SCUGIC_0_DEVICE_ID
/* Time out parameter while polling for response */
#define TIMEOUT_COUNT 500000

/*****************************************************************************/

int32_t mailbox_init(uint32_t cpu);
int init_mailbox_ipi();
int32_t write_mboxcmd(uint32_t, void *, uint32_t, MboxCoreId, MboxCoreId);
void apu_mailbox_read(/*void *CallbackRef*/ uint32_t);
uint32_t ParseCommand(MBCmdId_E, void *, uint32_t,	MboxCoreId, MboxCoreId);
uint32_t ApuProcessCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			   MboxCoreId src_cpu);
uint32_t ApuProcessFusaCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			       MboxCoreId src_cpu);
void mailbox_close();
void Apu_Mbox_Check_Command(void);
void Apu_Mbox_Check_FusaCommand(void);
#endif
