// Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#include <string.h>

#include "mbox_phy.h"

#include "mbox_api.h"
#include "mbox_error_code.h"
#include "oslayer.h"
#include "vlog.h"
#include "xil_mmu.h"
#include "xil_printf.h"
#include "xipipsu.h"
#include "xparameters.h"
#include "xscugic.h"

/** Log tag used by the mailbox transport layer. */
#define LOGTAG "MPHY"

/** IPI channel bitmask used to notify the RPU of a posted message. */
#define IPI5_IPI_NOBUF3_BITMASK 0x00001000U
#if ARMR52
/** Base address of the APU-to-RPU IPI block on Cortex-R52 builds. */
#define IPI1_BASEADDR 0xeb340000
#else
/** Base address of the APU-to-RPU IPI block. */
#define IPI5_BASEADDR 0xeb380000
#endif
#ifdef SDT
#include "xinterrupt_wrap.h"
#endif

/** Size of the reserved mailbox shared-memory section, in bytes. */
#define MBOX_SECTION_SIZE 0x200000
/** Size of a single 2 MB memory block, in bytes. */
#define BLOCK_SIZE_2MB 0x200000U

extern XScuGic Intc;
extern uint32_t __mbox_start;

/** @cond INTERNAL */
extern void mbox_interface_rx_dispatch(uint32_t ipi_src_mask);
/** @endcond */

XIpiPsu IpiInst_apu_rpu; /**< IPI driver instance for APU-to-RPU messaging. */
XScuGic GicInst;         /**< Interrupt controller instance. */
uint32_t flag = 0;       /**< General-purpose state flag. */

static MboxFifoCtrl *apu_fifo_ctrl; /**< APU mailbox FIFO control block. */
static MboxPostMsg *wmsg;           /**< Reusable outbound message buffer. */

static MboxPostMsg *rmsg_rpu0; /**< Receive buffer for RPU core 0. */
static MboxPostMsg *rmsg_rpu1; /**< Receive buffer for RPU core 1. */
static MboxPostMsg *rmsg_rpu2; /**< Receive buffer for RPU core 2. */
static MboxPostMsg *rmsg_rpu3; /**< Receive buffer for RPU core 3. */

/**
 * @brief Triggers an IPI to a target RPU core and waits for acknowledgment.
 *
 * @param[in] instance_ptr Pointer to the IPI driver instance.
 * @param[in] dst_ipi_id   Destination core identifier.
 *
 * @return XST_SUCCESS on success, XST_FAILURE otherwise.
 */
static XStatus trigger_ipi(XIpiPsu *instance_ptr, uint32_t dst_ipi_id)
{
	u32 status = 0;
	u32 dst_ipi_id_offset = 0;

	if (dst_ipi_id == MBOX_CORE_RPU0) {
		dst_ipi_id_offset = RPU0_TARGET;
	} else if (dst_ipi_id == MBOX_CORE_RPU1) {
		dst_ipi_id_offset = RPU1_TARGET;
	} else if (dst_ipi_id == MBOX_CORE_RPU2) {
		dst_ipi_id_offset = RPU2_TARGET;
	} else if (dst_ipi_id == MBOX_CORE_RPU3) {
		dst_ipi_id_offset = RPU3_TARGET;
	}

	instance_ptr->Config.TargetList[dst_ipi_id_offset].Mask = IPI5_IPI_NOBUF3_BITMASK;
	XIpiPsu_TriggerIpi(instance_ptr, instance_ptr->Config.TargetList[dst_ipi_id_offset].Mask);

	status = XIpiPsu_PollForAck(instance_ptr,
				     instance_ptr->Config.TargetList[dst_ipi_id_offset].Mask,
				     TIMEOUT_COUNT);
	return (status == XST_SUCCESS) ? XST_SUCCESS : XST_FAILURE;
}

/**
 * @brief Triggers the IPI that notifies the receiver of a posted message.
 */
XStatus apu_postmsg(MboxCoreId receiver_id)
{
	u32 status = XST_FAILURE;

	if (MBOX_CORE_RPU0 == receiver_id)
		status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU0);
	if (MBOX_CORE_RPU1 == receiver_id)
		status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU1);
	if (MBOX_CORE_RPU2 == receiver_id)
		status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU2);
	if (MBOX_CORE_RPU3 == receiver_id)
		status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU3);

	return status;
}

/**
 * @brief Handles the IPI interrupt and dispatches received messages.
 *
 * @param[in] XIpiPsuPtr Pointer to the IPI driver instance.
 */
void IpiIntrHandler(void *XIpiPsuPtr)
{
	u32 ipi_src_mask;
	u32 src_index;
	XIpiPsu *instance_ptr = (XIpiPsu *)XIpiPsuPtr;

	Xil_AssertVoid(instance_ptr != NULL);
	ipi_src_mask = XIpiPsu_GetInterruptStatus(instance_ptr);

	for (src_index = 0U; src_index < instance_ptr->Config.TargetCount; src_index++) {
		if (ipi_src_mask & instance_ptr->Config.TargetList[src_index].Mask) {
			XIpiPsu_ClearInterruptStatus(instance_ptr,
						     instance_ptr->Config.TargetList[src_index].Mask);
		}
	}

	mbox_interface_rx_dispatch(ipi_src_mask);
}

#ifndef SDT
/**
 * @brief Connects and enables the IPI interrupt in the GIC.
 *
 * @param[in] intc_instance_ptr Pointer to the interrupt controller instance.
 * @param[in] ipi_instance_ptr  Pointer to the IPI driver instance.
 * @param[in] ipi_intr_id       Interrupt identifier of the IPI.
 *
 * @return XST_SUCCESS on success, XST_FAILURE otherwise.
 */
static XStatus SetupInterruptSystem(XScuGic *intc_instance_ptr,
				    XIpiPsu *ipi_instance_ptr,
				    u32 ipi_intr_id)
{
	XScuGic_Config *intc_config;
	uint32_t status;

	intc_config = XScuGic_LookupConfig(0xe2000000);
	if (intc_config == NULL)
		return XST_FAILURE;

	status = XScuGic_CfgInitialize(&Intc, intc_config, intc_config->CpuBaseAddress);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     intc_instance_ptr);

	status = XScuGic_Connect(intc_instance_ptr, ipi_intr_id,
				 (Xil_InterruptHandler)IpiIntrHandler,
				 (void *)ipi_instance_ptr);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	XScuGic_Enable(intc_instance_ptr, ipi_intr_id);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/**
 * @brief Initializes and enables the IPI for the given source core.
 *
 * @param[in] src_ipi_id Source core identifier.
 * @param[in] ipi_inst   Pointer to the IPI driver instance to initialize.
 *
 * @return 0 on success, -1 on failure.
 */
static int ipi_init(int src_ipi_id, XIpiPsu *ipi_inst)
{
	uint32_t baseaddr = 0;
	XIpiPsu_Config *cfg_ptr;

	switch (src_ipi_id) {
	case MBOX_CORE_APU:
#if ARMR52
		baseaddr = IPI1_BASEADDR;
#else
		baseaddr = IPI5_BASEADDR;
#endif
		break;
	default:
		xil_printf("Not a Valid IPI core \n");
		return -1;
	}

#ifndef SDT
	cfg_ptr = XIpiPsu_LookupConfig(src_ipi_id);
#else
	cfg_ptr = XIpiPsu_LookupConfig(baseaddr);
#endif
	XIpiPsu_CfgInitialize(ipi_inst, cfg_ptr, cfg_ptr->BaseAddress);

#ifndef SDT
	SetupInterruptSystem(&Intc, ipi_inst, ipi_inst->Config.IntId);
#else
	XSetupInterruptSystem(ipi_inst, &IpiIntrHandler,
			     ipi_inst->Config.IntId,
			     ipi_inst->Config.IntrParent,
			     0xA0);
#endif

	XIpiPsu_InterruptEnable(ipi_inst, XIPIPSU_ALL_MASK);
	XIpiPsu_ClearInterruptStatus(ipi_inst, XIPIPSU_ALL_MASK);
	return 0;
}

/**
 * @brief Allocates the transmit and receive buffers used by the transport.
 *
 * @return VPI_SUCCESS on success, VPI_ERR_NOMEM otherwise.
 */
static int32_t mailbox_phy_buffers_alloc(void)
{
	rmsg_rpu0 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu0 == NULL)
		goto err_nomem;

	rmsg_rpu1 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu1 == NULL)
		goto err_free_rpu0;

	rmsg_rpu2 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu2 == NULL)
		goto err_free_rpu1;

	rmsg_rpu3 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu3 == NULL)
		goto err_free_rpu2;

	wmsg = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (wmsg == NULL)
		goto err_free_rpu3;

	memset(wmsg, 0, sizeof(MboxPostMsg));
	return VPI_SUCCESS;

err_free_rpu3:
	osFree(rmsg_rpu3);
	rmsg_rpu3 = NULL;
err_free_rpu2:
	osFree(rmsg_rpu2);
	rmsg_rpu2 = NULL;
err_free_rpu1:
	osFree(rmsg_rpu1);
	rmsg_rpu1 = NULL;
err_free_rpu0:
	osFree(rmsg_rpu0);
	rmsg_rpu0 = NULL;
err_nomem:
	return VPI_ERR_NOMEM;
}

/**
 * @brief Initializes the mailbox transport layer for the local core.
 */
int32_t mbox_phy_init(MboxCoreId current_cpu_id)
{
	uint32_t mbox_start = (uint32_t)&__mbox_start;
	int32_t status;

	status = mailbox_phy_buffers_alloc();
	if (status != VPI_SUCCESS)
		return status;

	for (int i = 0; i < (MBOX_SECTION_SIZE / BLOCK_SIZE_2MB); i++) {
#if defined __aarch64__
		Xil_SetTlbAttributes(mbox_start + i * BLOCK_SIZE_2MB, NORM_NONCACHE);
#endif
	}

	if (MBOX_CORE_APU == current_cpu_id)
		apu_fifo_ctrl = vpi_mbox_init(MBOX_CORE_APU, mbox_start, MBOX_FIFO_BLOCK_SIZE);

	if (apu_fifo_ctrl == NULL)
		return VPI_ERR_UNINITED;

	if (ipi_init(MBOX_CORE_APU, &IpiInst_apu_rpu) != 0)
		return VPI_ERR_GENERIC;

	xil_printf("APU mailbox init done, sizeof(MboxPostMsg)=%u, sizeof(Payload_packet)=%u\r\n",
		   sizeof(MboxPostMsg), sizeof(Payload_packet));
	return VPI_SUCCESS;
}

/**
 * @brief Closes the mailbox transport layer and frees its buffers.
 */
void mbox_phy_close(void)
{
	if (apu_fifo_ctrl)
		vpi_mbox_destory(apu_fifo_ctrl);
	apu_fifo_ctrl = NULL;

	osFree(rmsg_rpu0);
	osFree(rmsg_rpu1);
	osFree(rmsg_rpu2);
	osFree(rmsg_rpu3);
	osFree(wmsg);

	rmsg_rpu0 = NULL;
	rmsg_rpu1 = NULL;
	rmsg_rpu2 = NULL;
	rmsg_rpu3 = NULL;
	wmsg = NULL;
}

/**
 * @brief Posts a fully stamped message over the mailbox transport.
 */
int32_t mbox_phy_send(MboxPostMsg *msg, MboxCoreId receiver_id, MboxCoreId core_id)
{
	uint32_t ret = VPI_SUCCESS;

	if (wmsg == NULL || msg == NULL)
		return VPI_ERR_UNINITED;

	/* Post the fully-stamped message as-is so the checksum stays intact. */
	memcpy(wmsg, msg, sizeof(MboxPostMsg) - MAX_ITEM + msg->payload.payload_size);

	if (MBOX_CORE_APU == core_id)
		ret = vpi_mbox_post(apu_fifo_ctrl, wmsg, receiver_id, apu_postmsg);

	if (ret != VPI_SUCCESS)
		return (ret == VPI_ERR_FULL) ? VPI_ERR_FULL : VPI_ERR_GENERIC;

	return VPI_SUCCESS;
}

/**
 * @brief Pops the next received message from the mailbox FIFO.
 */
int32_t mbox_phy_pop_rx(MboxPostMsg *out_msg)
{
	if (out_msg == NULL)
		return VPI_ERR_INVALID;

	if (apu_fifo_ctrl == NULL)
		return VPI_ERR_UNINITED;

	return vpi_mbox_read(apu_fifo_ctrl, out_msg, dest_cpu_id);
}

/**
 * @brief Returns whether the mailbox FIFO has a pending message.
 */
bool mbox_phy_has_rx(void)
{
	if (apu_fifo_ctrl == NULL)
		return false;

	return !vpi_mbox_is_empty(apu_fifo_ctrl, dest_cpu_id, src_cpu_id);
}
