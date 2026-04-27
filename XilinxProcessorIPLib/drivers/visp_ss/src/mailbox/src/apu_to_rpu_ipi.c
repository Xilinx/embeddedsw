/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xipipsu_self_test_example.c
*
* This file consists of a self test example which uses the XIpiPsu driver to
* send an IPI message to self and get a response
* Each IPI channel can trigger an interrupt to itself and can exchange messages
* through the message buffer. This feature is used here to exercise the driver
* APIs.
* Example control flow:
* - Init the IPI and GIC drivers
* - Setup Interrupt System with IPI handler which inverts the received message
*   and sends back as response
* - Write a Message and Trigger IPI to Self.
* - Keep polling for response till timeout
* - Interrupt handler receives IPI and sends back response
* - Read the received response and do a sanity check
* - Print PASS or FAIL based on sanity check of response message
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 2.2  ms  01/23/17 Modified xil_printf statement in main function to
*                   ensure that "Successfully ran" and "Failed" strings
*                   are available in all examples. This is a fix for
*                   CR-965028.
* </pre>
*
******************************************************************************/
/*****************************************************************************/
#include "stdlib.h"
#include "xil_types.h"
#include <string.h>

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xscugic.h"
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xenv_standalone.h"
#include <xil_mmu.h>

#include <mbox_cmd.h>
#include <mbox_api.h>
#include <sensor_cmd.h>

#include "vlog.h"
#include "align.h"
#define LOGTAG "APUTORPU"
#include "xvisp_ss.h"

//#define MULTICORE_ON_APU

#define MBOX_SECTION_SIZE 0x400000
#define BLOCK_SIZE_2MB 0x200000U

extern uint32_t __mbox_start;
extern uint32_t __cam_load_calib_start;


int mailbox_wrapper()
{
	u32 ret = XST_FAILURE, i = 0;
	uint64_t mbox_start = (uint32_t)&__mbox_start;
	uint64_t load_calib_start = (uint32_t)&__cam_load_calib_start;
	uint64_t HAL_RESERVED_MEM_START_RPU0 = 0, HAL_RESERVED_MEM_START_RPU1 = 0,
						  HAL_RESERVED_MEM_SIZE = 0x20000000;

	LOGI("Unified Mailbox version 1.0 \r\n");

	src_cpu_id = MBOX_CORE_APU;

	for (i = 0; i < (MBOX_SECTION_SIZE / BLOCK_SIZE_2MB); i++) {
#if defined __aarch64__

		Xil_SetTlbAttributes(mbox_start + i * BLOCK_SIZE_2MB, NORM_NONCACHE);
#endif
	}

#if defined __aarch64__
	Xil_SetTlbAttributes(load_calib_start, NORM_NONCACHE);
#endif

	HAL_RESERVED_MEM_START_RPU0 = 0x4FE00000;
	HAL_RESERVED_MEM_START_RPU1 = 0x6FE00000;
	HAL_RESERVED_MEM_SIZE = 0x20000000;

	int num_region = HAL_RESERVED_MEM_SIZE / BLOCK_SIZE_2MB;

	for (int i = 0; i < (num_region); i++) {
#if defined __aarch64__
		Xil_SetTlbAttributes(HAL_RESERVED_MEM_START_RPU0 + i * BLOCK_SIZE_2MB, STRONG_ORDERED);
#ifdef MULTICORE_ON_APU
		Xil_SetTlbAttributes(HAL_RESERVED_MEM_START_RPU1 + i * BLOCK_SIZE_2MB, STRONG_ORDERED);
#endif
#endif
	}


	init_mailbox_ipi() ;

#define RPU_ONLINE_MAX_RETRIES 100U

	ret = XST_FAILURE;
	{
		u32 retries = RPU_ONLINE_MAX_RETRIES;
		while (ret != XST_SUCCESS) {
			ret = apu_postmsg(MBOX_CORE_RPU0);
			LOGI("Waiting for RPU %d to come online \r\n", MBOX_CORE_RPU0);
			if (--retries == 0) {
				LOGE("RPU %d failed to come online\r\n", MBOX_CORE_RPU0);
				return XST_FAILURE;
			}
			sleep(3);
		}
	}


#ifdef MULTICORE_ON_APU
	ret = XST_FAILURE;
	//Xil_SetTlbAttributes(load_calib_start, NORM_NONCACHE );
	{
		u32 retries = RPU_ONLINE_MAX_RETRIES;
		while (ret != XST_SUCCESS) {
			ret = apu_postmsg(MBOX_CORE_RPU1);
			LOGI("Waiting for RPU %d to come online \r\n", MBOX_CORE_RPU1);
			if (--retries == 0) {
				LOGE("RPU %d failed to come online\r\n", MBOX_CORE_RPU1);
				return XST_FAILURE;
			}
			sleep(3);
		}
	}
	LOGI("\n\n\n\n Both RPU ready!!! \r\n");
#else
	LOGI("\n\n\n\n RPU-0  ready!!! \r\n");
#endif

	/* Send APU_2_RPU_MB_CMB_INIT_FIRMWARE to unblock RPU firmware */
	{
		Payload_packet init_packet;
		memset(&init_packet, 0, sizeof(Payload_packet));
		init_packet.type = CMD;
		init_packet.cookie = 0;
		init_packet.payload_size = sizeof(uint32_t);
		/* instance_id in payload_data[0] */
		init_packet.payload_data[0] = 0;

		LOGI("Sending APU_2_RPU_MB_CMB_INIT_FIRMWARE to RPU-0\r\n");
		ret = Send_Command(APU_2_RPU_MB_CMB_INIT_FIRMWARE, &init_packet,
				   init_packet.payload_size + payload_extra_size,
				   dest_cpu_id, src_cpu_id);
		if (ret != 0) {
			LOGE("Failed to send INIT_FIRMWARE command, ret=%d\r\n", ret);
			return XST_FAILURE;
		}

		uint8_t ack_ret = apu_wait_for_ACK(init_packet.cookie,
						    init_packet.payload_data);
		if (ack_ret != RET_SUCCESS) {
			LOGE("INIT_FIRMWARE ACK failed, ret=%u\r\n", ack_ret);
			return XST_FAILURE;
		}
		LOGI("RPU firmware initialization confirmed\r\n");
	}

#ifdef MULTICORE_ON_APU
	/* Send APU_2_RPU_MB_CMB_INIT_FIRMWARE to RPU-1 as well */
	{
		Payload_packet init_packet;
		memset(&init_packet, 0, sizeof(Payload_packet));
		init_packet.type = CMD;
		init_packet.cookie = 0;
		init_packet.payload_size = sizeof(uint32_t);
		init_packet.payload_data[0] = 0;

		LOGI("Sending APU_2_RPU_MB_CMB_INIT_FIRMWARE to RPU-1\r\n");
		ret = Send_Command(APU_2_RPU_MB_CMB_INIT_FIRMWARE, &init_packet,
				   init_packet.payload_size + payload_extra_size,
				   MBOX_CORE_RPU1, src_cpu_id);
		if (ret != 0) {
			LOGE("Failed to send INIT_FIRMWARE to RPU-1, ret=%d\r\n", ret);
			return XST_FAILURE;
		}

		uint8_t ack_ret = apu_wait_for_ACK(init_packet.cookie,
						    init_packet.payload_data);
		if (ack_ret != RET_SUCCESS) {
			LOGE("INIT_FIRMWARE ACK from RPU-1 failed, ret=%u\r\n", ack_ret);
			return XST_FAILURE;
		}
		LOGI("RPU-1 firmware initialization confirmed\r\n");
	}
#endif

	return 0;

}
