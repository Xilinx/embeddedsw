#include <string.h>
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


#include "mbox_cmd.h"
#include <stdio.h>
#include "xil_printf.h"
#include <xipipsu.h>
#include "xscugic.h"
#include <xil_cache.h>
#include "oslayer.h"
#include "mbox_api.h"
#include "mbox_error_code.h"
#include "sensor_cmd.h"
#include <xil_mmu.h>
#include <xipipsu.h>
#include "vlog.h"
#if ENABLE_VMIX_MACRO
	#include "xv_mix_l2.h"
	#include "xvidc.h"
#endif

#define  XPAR_PS_0_PSPMC_0_PSV_IPI_1_BIT_MASK  0x00001000U
#define  XPAR_PS_0_PSPMC_0_PSV_IPI_2_BIT_MASK  0x00000010U


#ifdef SDT
	#include "xinterrupt_wrap.h"
#endif
#ifdef XPAR_I2C0_BASEADDR
#include "control.h"
#endif

#define LOGTAG "MCMD"
extern XScuGic Intc;
extern long int apu_parsecontrolenter;

static MboxFifoCtrl *rpu0_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu1_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu2_fifo_ctrl = NULL;
static MboxFifoCtrl *rpu3_fifo_ctrl = NULL;
static MboxFifoCtrl *apu_fifo_ctrl = NULL;
static MboxPostMsg *rmsg_rpu0 = NULL;
static MboxPostMsg *rmsg_rpu1 = NULL;
static MboxPostMsg *rmsg_rpu2 = NULL;
static MboxPostMsg *rmsg_rpu3 = NULL;
static MboxPostMsg *rmsg_apu = NULL;
static MboxPostMsg *rmsg_response_to_apu = NULL;
static MboxPostMsg *rmsg_command_to_apu = NULL;
static MboxPostMsg *wmsg = NULL;  // for write message to RPU
XIpiPsu IpiInst_apu_rpu;
XScuGic GicInst;
uint32_t flag = 0;
//#define DEBUG_ENABLE_LOG

/*-------------------for set param to RPU-------------*/
volatile static bool ACK = false;
volatile static bool mb_data_flag = 0;
volatile static uint32_t payload_ret_cookie;
volatile static bool ACK_PROC = false; // for processing command from RPU
volatile static bool ACK_PROC_FUSA = false; // for processing command from RPU

extern uint32_t __mbox_start;

/**
 * Interrupt Handler :
 * -Polls for each of the valid sources
 * -Checks if there is a message
 * -Reads the message
 * -Inverts the bits
 * -Sends back the inverted message as response
 *
 */
void IpiIntrHandler(void *XIpiPsuPtr)
{

	u32 IpiSrcMask; /**< Holds the IPI status register value */
	u32 SrcIndex;
	XIpiPsu *InstancePtr = (XIpiPsu *) XIpiPsuPtr;
	Xil_AssertVoid(InstancePtr != NULL);
	IpiSrcMask = XIpiPsu_GetInterruptStatus(InstancePtr);
#ifdef DEBUG_ENABLE_LOG
	xil_printf("!!APU ---->IRQ no %d triggered\r\n", InstancePtr->Config.IntId);
#endif
	/* Poll for each source and send Response (Response = ~Msg) */
	for (SrcIndex = 0U; SrcIndex < InstancePtr->Config.TargetCount;
	     SrcIndex++) {

		if (IpiSrcMask & InstancePtr->Config.TargetList[SrcIndex].Mask) {
			/* Clear the Interrupt Status - This clears the OBS bit on the SRC CPU registers */
			XIpiPsu_ClearInterruptStatus(InstancePtr,
						     InstancePtr->Config.TargetList[SrcIndex].Mask);
		}
	}
	apu_mailbox_read(IpiSrcMask);  //Hook the function which is to be called by the interrupt handler
}

#ifndef SDT
static XStatus SetupInterruptSystem(XScuGic *IntcInstancePtr,
				    XIpiPsu *IpiInstancePtr, u32 IpiIntrId)
{
	uint32_t Status = 0, ret = 0;
	XScuGic_Config *IntcConfig; /* Config for interrupt controller */

	/* Initialize the interrupt controller driver */
	IntcConfig = XScuGic_LookupConfig(0xe2000000);
	if (NULL == IntcConfig)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(&Intc, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler) XScuGic_InterruptHandler, IntcInstancePtr);

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */
	LOGI("Interrupt ID: %d\r\n", IpiIntrId);
	Status = XScuGic_Connect(IntcInstancePtr, IpiIntrId,
				 (Xil_InterruptHandler) IpiIntrHandler, (void *) IpiInstancePtr);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IpiIntrId);

	/* Enable interrupts */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif


int ipi_init(int src_ipi_id, XIpiPsu *IpiInst)
{
	uint32_t ret = 0, baseaddr = 0;
	XIpiPsu_Config *CfgPtr;

	switch (src_ipi_id) {
		case MBOX_CORE_APU:
			baseaddr = 0xeb380000; //IPI-5
			break;
		default:
			xil_printf("Not a Valid IPI core \n");
			return -1;
			break;
	}
	/* Look Up the config data */

#ifndef SDT
	CfgPtr = XIpiPsu_LookupConfig(src_ipi_id);
#else
	CfgPtr = XIpiPsu_LookupConfig(baseaddr);
#endif
	/* Init with the Cfg Data */
	XIpiPsu_CfgInitialize(IpiInst, CfgPtr, CfgPtr->BaseAddress);

	// xil_printf("APU ipi instance src config:\r\n");
	// xil_printf("InstancePtr->Config.BaseAddress:%x.\r\n", IpiInst->Config.BaseAddress);
	// xil_printf("IpiInst->Config.IntId:%x.\r\n", IpiInst->Config.IntId);
	// xil_printf("InstancePtr->Config.BitMask:%x.\r\n", IpiInst->Config.BitMask);
	// xil_printf("InstancePtr->Config.BufferIndex:%x.\r\n", IpiInst->Config.BufferIndex);
	// xil_printf("InstancePtr->Config.TargetList[0].Mask:%x.\r\n", IpiInst->Config.TargetList[0].Mask);
#ifndef SDT
	/* Setup the GIC */
	SetupInterruptSystem(&Intc, IpiInst, (IpiInst->Config.IntId));
#else
	XSetupInterruptSystem(IpiInst, &IpiIntrHandler,
			      IpiInst->Config.IntId,
			      IpiInst->Config.IntrParent,
			      //XINTERRUPT_DEFAULT_PRIORITY);
			      0xA0);
#endif
	/* Enable reception of IPIs from all CPUs */
	XIpiPsu_InterruptEnable(IpiInst, XIPIPSU_ALL_MASK);

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInst, XIPIPSU_ALL_MASK);
	return ret;

}


/**
 * @brief	Tests the IPI by sending a message and checking the response
 */

static XStatus trigger_ipi(XIpiPsu *InstancePtr, uint32_t dst_ipi_id)
{

	u32 Status = 0, dst_ipi_id_offset = 0;

	if (dst_ipi_id == MBOX_CORE_RPU0) {
		dst_ipi_id_offset = RPU0_TARGET; /*3rd IPI target is the APU,1-rpu[0],2-rpu[1]*/
	} else if (dst_ipi_id == MBOX_CORE_RPU1) {
		dst_ipi_id_offset = RPU1_TARGET; /*3rd IPI target is the APU,1-rpu[0],2-rpu[1]*/
	} else if (dst_ipi_id == MBOX_CORE_RPU2) {
		dst_ipi_id_offset = RPU2_TARGET; /*3rd IPI target is the APU,1-rpu[0],2-rpu[1]*/
	} else if (dst_ipi_id == MBOX_CORE_RPU3) {
		dst_ipi_id_offset = RPU3_TARGET; /*3rd IPI target is the APU,1-rpu[0],2-rpu[1]*/
	}

	/* Self trigger
	dst_ipi_id_offset=APU_TARGET; */
	InstancePtr->Config.TargetList[dst_ipi_id_offset].Mask = 0x1000;
	XIpiPsu_TriggerIpi(InstancePtr, InstancePtr->Config.TargetList[dst_ipi_id_offset].Mask);

	Status = XIpiPsu_PollForAck(InstancePtr, InstancePtr->Config.TargetList[dst_ipi_id_offset].Mask,
				    TIMEOUT_COUNT);

	if (XST_SUCCESS == Status) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}
	return Status;
}


XStatus apu_postmsg(MboxCoreId receiver_id)
{
	u32 Status = XST_FAILURE;

	if (MBOX_CORE_RPU0 == receiver_id)
		Status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU0);

	if (MBOX_CORE_RPU1 == receiver_id)

		Status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU1);

	if (MBOX_CORE_RPU2 == receiver_id)
		Status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU2);

	if (MBOX_CORE_RPU3 == receiver_id)

		Status = trigger_ipi(&IpiInst_apu_rpu, MBOX_CORE_RPU3);

	return Status;
}

int32_t write_mboxcmd(uint32_t cmdId, void *struct_msg, uint32_t size, MboxCoreId receiver_id,
		      MboxCoreId core_id)
{
	uint32_t ret;
	int status;

	if (size == 0)
		wmsg->msg_id = cmdId;

	else {
		wmsg->msg_id = cmdId;
		wmsg->size = sizeof(Payload_packet) - MAX_ITEM + ((Payload_packet *)
			     struct_msg)->payload_size; // record valid  Payload_packet size
		memcpy(wmsg->payload, struct_msg, wmsg->size);
	}

	if (MBOX_CORE_APU == core_id)
		ret = vpi_mbox_post(apu_fifo_ctrl, wmsg, receiver_id, apu_postmsg);

	if (ret) {
		if (ret == VPI_ERR_FULL)
			LOGE("VPI_ERR_FULL error in rpu_fifo_ctrl[1]\n");
		return VPI_ERR_GENERIC;
	}

	return VPI_SUCCESS;
}

void Apu_Mbox_Check_Command(void)
{
	uint16_t msg_id_;
	uint32_t size_;
	Payload_packet proc_cmd_packet;

	if (ACK_PROC == true) {
		LOGI("\r\n processing command from RPU \r\n");
		ACK_PROC = false;
		msg_id_ = rmsg_command_to_apu->msg_id;
		size_ = rmsg_command_to_apu->size;
		memcpy(&proc_cmd_packet, rmsg_command_to_apu->payload, size_);
		ApuProcessCommand(msg_id_, &proc_cmd_packet, size_, src_cpu_id, dest_cpu_id);
	}
}


void Apu_Mbox_Check_FusaCommand(void)
{
	uint16_t msg_id_;
	uint32_t size_;
	Payload_packet proc_cmd_packet;

	if (ACK_PROC_FUSA == true) {
		ACK_PROC_FUSA = false;
		msg_id_ = rmsg_command_to_apu->msg_id;
		size_ = rmsg_command_to_apu->size;
		memcpy(&proc_cmd_packet, rmsg_command_to_apu->payload, size_);
		ApuProcessFusaCommand(msg_id_, &proc_cmd_packet, size_, src_cpu_id, dest_cpu_id);
	}
}


uint8_t apu_wait_for_ACK(uint32_t cookie, void *data)
{

	volatile Payload_packet *packet = (Payload_packet *)rmsg_response_to_apu->payload;

	while (ACK == false || cookie != payload_ret_cookie) {

		//	if(ACK_PROC == true){
		////			LOGI("\r\n processing command from RPU \r\n");
		//	ACK_PROC = false;
		//	msg_id_ = rmsg_apu->msg_id;
		//	size_ = rmsg_apu->size;
		//	memcpy(&proc_cmd_packet, rmsg_apu->payload, size_);
		//	ApuProcessCommand(msg_id_, &proc_cmd_packet, size_, src_cpu_id,dest_cpu_id);
		//	}
		Apu_Mbox_Check_Command();
	}
	if (mb_data_flag)
		memcpy(data, packet->payload_data, packet->payload_size);
	ACK = false;
	mb_data_flag = false;
	return packet->resp_field.error_subcode_t;
}


void apu_mailbox_read(uint32_t IpiSrcMask)
{
	if (vpi_mbox_is_empty(apu_fifo_ctrl, dest_cpu_id,
			      src_cpu_id)) //rpu0 check  the msg from MBOX_CORE_APU
		xil_printf("APU there is no msg in Share memory!\r\n");
	else {
		while (!vpi_mbox_is_empty(apu_fifo_ctrl, dest_cpu_id, src_cpu_id)) {
			vpi_mbox_read(apu_fifo_ctrl, rmsg_apu, dest_cpu_id); //rpu0 rece MBOX_CORE_APU's msg

			//need make sure APU initiative call function to RPU after last function had receive response
			//to make it is compatitable for apu_wait_for_mb_data and apu_wait_for_ACK function
			if ((rmsg_apu->msg_id == MB_CMD_RES_SUCCESS) || (rmsg_apu->msg_id == MB_CMD_GET_SUCCESS))
				memcpy(rmsg_response_to_apu, rmsg_apu,
				       sizeof(MboxPostMsg) - sizeof(Payload_packet) + rmsg_apu->size);

			else
				memcpy(rmsg_command_to_apu, rmsg_apu,
				       sizeof(MboxPostMsg) - sizeof(Payload_packet) + rmsg_apu->size);
			ParseCommand(rmsg_apu->msg_id, rmsg_apu->payload, rmsg_apu->size, src_cpu_id, dest_cpu_id);

		}
	}

}


int32_t mailbox_init(uint32_t cpu)
{

	uint32_t mbox_start = (uint32_t)&__mbox_start;
	rmsg_rpu0 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu0 == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}

	rmsg_rpu1 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu1 == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	rmsg_rpu2 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu2 == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	rmsg_rpu3 = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_rpu3 == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	rmsg_apu = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_apu == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	rmsg_response_to_apu = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_response_to_apu == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	rmsg_command_to_apu = (MboxPostMsg *)osMalloc(sizeof(MboxPostMsg));
	if (rmsg_command_to_apu == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}
	wmsg = (MboxPostMsg *) osMalloc(sizeof(MboxPostMsg));
	if (wmsg == NULL) {
		xil_printf("APU-%d Failed to allocate memory\r\n", XPAR_CPU_ID);
		return VPI_ERR_NOMEM;
	}


#define MBOX_SECTION_SIZE 0x200000
#define BLOCK_SIZE_2MB 0x200000U
#define HAL_RESERVED_MEM_SIZE=0x20000000;

	for (int i = 0; i < (MBOX_SECTION_SIZE / BLOCK_SIZE_2MB); i++) {
		xil_printf("MBOX_FIFO_BLOCK_SIZE/BLOCK_SIZE_2MB %d, i-%d\r\n",
			   MBOX_FIFO_BLOCK_SIZE / BLOCK_SIZE_2MB, i);
#if defined __aarch64__
		Xil_SetTlbAttributes(mbox_start + i * BLOCK_SIZE_2MB, NORM_NONCACHE);
#endif
	}


	if (MBOX_CORE_APU == cpu)
		apu_fifo_ctrl = vpi_mbox_init(MBOX_CORE_APU, mbox_start, MBOX_FIFO_BLOCK_SIZE);

	else if (MBOX_CORE_RPU0 == cpu)
		rpu0_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU0, mbox_start, MBOX_FIFO_BLOCK_SIZE);

	else if (MBOX_CORE_RPU1 == cpu)
		rpu1_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU1, mbox_start, MBOX_FIFO_BLOCK_SIZE);

	else if (MBOX_CORE_RPU2 == cpu)
		rpu2_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU2, mbox_start, MBOX_FIFO_BLOCK_SIZE);

	else if (MBOX_CORE_RPU3 == cpu)
		rpu3_fifo_ctrl = vpi_mbox_init(MBOX_CORE_RPU3, mbox_start, MBOX_FIFO_BLOCK_SIZE);
}

void mailbox_close()
{
	osFree(apu_fifo_ctrl);
	osFree(rpu0_fifo_ctrl);
	osFree(rpu1_fifo_ctrl);
	osFree(rpu2_fifo_ctrl);
	osFree(rpu3_fifo_ctrl);
	osFree(rmsg_rpu0);
	osFree(rmsg_rpu1);
	osFree(rmsg_rpu2);
	osFree(rmsg_rpu3);
	osFree(rmsg_apu);
	osFree(rmsg_response_to_apu);
	osFree(rmsg_command_to_apu);
	osFree(wmsg);
	wmsg = NULL;
	rmsg_apu = NULL;
	rmsg_response_to_apu = NULL;
	rmsg_rpu0 = NULL;
	rmsg_rpu1 = NULL;
	rmsg_rpu2 = NULL;
	rmsg_rpu3 = NULL;

}


int init_mailbox_ipi()
{

	mailbox_init(src_cpu_id);
	ipi_init(MBOX_CORE_APU,
		 &IpiInst_apu_rpu); //2 for APU-IPI is the deviceId in the XIpiPsu_ConfigTable[]

	return 0;

}


int Send_Command(MBCmdId_E cmd, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;
	((Payload_packet *)data)->resp_field.error_subcode_t = RET_SUCCESS;
	ret = write_mboxcmd(cmd, data, size, dest_cpu, src_cpu);
	return ret;
}

int Send_Response(MBCmdId_E res, void *data, uint32_t size, uint8_t dest_cpu, uint8_t src_cpu)
{
	int ret = 0;

	ret = write_mboxcmd(res, data, size, dest_cpu, src_cpu);

	return ret;
}
#if ENABLE_VMIX_MACRO
	VMix_input_buffer Vmix_buff;
	extern XV_Mix_l2  mix;
	extern XVidC_VideoStream  VidStream;
#endif
uint32_t ParseCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
		      MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	//Response_packet resp_pckt, *buf_resp = data; //response packet to be sent to APU; //Received response packet
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif
	switch (cmd) {
		/*These cases handle the commands sent by the RPU's*/
		case MB_CMD_RES_SUCCESS:
			payload_ret_cookie = packet->cookie;
			ACK = true;
			break;

		case MB_CMD_GET_SUCCESS:
			payload_ret_cookie = packet->cookie;
			ACK = true;
			mb_data_flag = true;

			break;
		case RPU_2_APU_MB_CMD_ISP_ERR_REPORT:
		case RPU_2_APU_MB_CMD_IsiCreateIss:
		case RPU_2_APU_MB_CMD_IsiReleaseIss:
		case RPU_2_APU_MB_CMD_IsiEnumModeIss:
		case RPU_2_APU_MB_CMD_IsiCheckConnectionIss:
		case RPU_2_APU_MB_CMD_IsiOpenIss:
		case RPU_2_APU_MB_CMD_IsiCloseIss:
		case RPU_2_APU_MB_CMD_IsiGetModeIss:
		case	RPU_2_APU_MB_CMD_IsiGetCapsIss:
		case	RPU_2_APU_MB_CMD_IsiGetRevisionIss:
		case	RPU_2_APU_MB_CMD_IsiSetStreamingIss:
		case	RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss:
		case	RPU_2_APU_MB_CMD_IsiGetAGainIss:
		case	RPU_2_APU_MB_CMD_IsiGetDGainIss:
		case	RPU_2_APU_MB_CMD_IsiSetAGainIss:
		case	RPU_2_APU_MB_CMD_IsiSetDGainIss:
		case	RPU_2_APU_MB_CMD_IsiGetIntTimeIss:
		case	RPU_2_APU_MB_CMD_IsiSetIntTimeIss:
		case	RPU_2_APU_MB_CMD_IsiGetFpsIss:
		case	RPU_2_APU_MB_CMD_IsiSetFpsIss:
		case	RPU_2_APU_MB_CMD_IsiGetIspStatusIss:
		case	RPU_2_APU_MB_CMD_IsiSetWBIss:
		case	RPU_2_APU_MB_CMD_IsiGetWBIss:
		case	RPU_2_APU_MB_CMD_IsiSetBlcIss:
		case	RPU_2_APU_MB_CMD_IsiGetBlcIss:
		case	RPU_2_APU_MB_CMD_IsiSetTpgIss:
		case	RPU_2_APU_MB_CMD_IsiGetTpgIss:
		case	RPU_2_APU_MB_CMD_IsiGetExpandCurveIss:
		case	RPU_2_APU_MB_CMD_IsiWriteRegIss:
		case	RPU_2_APU_MB_CMD_IsiReadRegIss:

			ACK_PROC = true;
			break;

		case RPU_2_APU_MB_CMD_FUSA_EVENT_CB:
			ACK_PROC_FUSA = true;
			break;

		case RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM:
			//LOGI("RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM coreid %d\r\n", core_id);
			CtrlFullBufferInform(data);
			break;


		case RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE:
			// LOGI("RPU %d requested RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE \n",src_cpu);
			break;

		case MB_CMD_END:
		case APU_2_RPU_MB_CMB_INIT_FIRMWARE:
			break;

		/*Following cases handle the responses sent by the RPU*/

		case MB_CMD_RES_ERR:
			//LOGI("APU MB_CMD_RES_ERR %d, cookie 0x%x,cmd id %d,src_cpu  %d \n", core_id,buf_resp->cookie,buf_resp->cmdid,src_cpu);
			LOGI("APU MB_CMD_RES_ERR %d, cookie 0x%x,src_cpu  %d \n", core_id, packet->cookie, src_cpu);
			break;
		case MB_CMD_RES_TIMEOUT:
			//LOGI("APU MB_CMD_RES_TIMEOUT %d, cookie 0x%x,cmd id %d,src_cpu %d \n", core_id,buf_resp->cookie,buf_resp->cmdid,src_cpu);
			LOGI("APU MB_CMD_RES_TIMEOUT %d, cookie 0x%x,src_cpu %d \n", core_id, packet->cookie, src_cpu);
			break;
		case MB_CMD_BUF_RET:
			//LOGI("FUN: %s\t buff response from RPU0 0x%x\r\n",__func__,buf_resp->resp_payload[0]);
			//Vmix_buff.baseAddress = buf_resp->resp_payload[0];
#if ENABLE_VMIX_MACRO
			Vmix_buff.baseAddress = packet->payload_data[0];
			Status = VMix_User_defined(&VidStream);  //check this!??
			if (Status == XST_FAILURE) {
				LOGE("VMIX User defined failed.\n\r");
				return XST_FAILURE;
			} else
				LOGI("\r\n\r\n VMIX User defined is Done \n");
#endif
			//LOGI("@RPU Response error: 0x%x cmd:%d cookie:%d load:%d\r\n",buf_resp->error_subcode, buf_resp->cmdid, buf_resp->cookie,buf_resp->payload_type) ;
			break;
	}
	//TODO:Send response tacket
	return ret;
}

uint32_t ApuProcessCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			   MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	//Response_packet resp_pckt, *buf_resp = data; //response packet to be sent to APU; //Received response packet
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif

	switch (cmd) {
		case RPU_2_APU_MB_CMD_ISP_ERR_REPORT:
			u32 isp_mis_read = packet->payload_data[0] |
					   (packet->payload_data[1] << 8) |
					   (packet->payload_data[2] << 16) |
					   (packet->payload_data[3] << 24);
			LOGI("APU RPU_2_APU_MB_CMD_ISP_ERR_REPORT, core_id %d, cookie 0x%x, src_xpu %d, isp_mis_read 0x%08x\n",
			     core_id, packet->cookie, src_cpu, isp_mis_read);

			// memset(packet, 0, sizeof(Payload_packet));
			// packet->cookie = 0;
			// packet->type = CMD;
			// packet->payload_size = 0;
			// packet->payload_size += sizeof(uint32_t);
			// Send_Command (APU_2_RPU_MB_CMD_SOFT_RESET, &packet,sizeof(Payload_packet),dest_cpu_id, src_cpu_id);
			break;

		/*---------------------------------------sensor function--------------------------------------*/
		case RPU_2_APU_MB_CMD_IsiCreateIss:
			LOGI("RPU_2_APU_MB_CMD_IsiCreateIss coreid %d\r\n", core_id);

			ret = ControlIsiSensorDrvHandleRegisterIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiReleaseIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiSensorDrvHandleUnRegisterIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;

		case RPU_2_APU_MB_CMD_IsiEnumModeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiEnumModeIss coreid %d\r\n", core_id);

			ret = ControlIsiEnumModeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiCheckConnectionIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiCheckConnectionIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiOpenIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiOpenIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiCloseIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReleaseIss coreid %d\r\n", core_id);

			ret = ControlIsiCloseIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiWriteRegIss:
			LOGI("RPU_2_APU_MB_CMD_IsiWriteRegIss coreid %d\r\n", core_id);

			ret = ControlIsiWriteRegIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiReadRegIss:
			LOGI("RPU_2_APU_MB_CMD_IsiReadRegIss coreid %d\r\n", core_id);

			ret = ControlIsiReadRegIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetModeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetModeIss coreid %d\r\n", core_id);

			ret = ControlIsiGetModeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetCapsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetCapsIss coreid %d\r\n", core_id);

			ret = ControlIsiGetCapsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetRevisionIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetRevisionIss coreid %d\r\n", core_id);

			ret = ControlIsiGetRevisionIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetStreamingIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetStreamingIss coreid %d\r\n", core_id);

			ret = ControlIsiSetStreamingIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetAeBaseInfoIss coreid %d\r\n", core_id);

			ret = ControlIsiGetAeBaseInfoIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetAGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetAGainIss coreid %d\r\n", core_id);

			ret = ControlIsiGetAGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetDGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetDGainIss coreid %d\r\n", core_id);

			ret = ControlIsiGetDGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetAGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetAGainIss coreid %d\r\n", core_id);

			ret = ControlIsiSetAGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetDGainIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetDGainIss coreid %d\r\n", core_id);

			ret = ControlIsiSetDGainIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetIntTimeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetIntTimeIss coreid %d\r\n", core_id);

			ret = ControlIsiGetIntTimeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetIntTimeIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetIntTimeIss coreid %d\r\n", core_id);

			ret = ControlIsiSetIntTimeIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetFpsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetFpsIss coreid %d\r\n", core_id);

			ret = ControlIsiGetFpsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetFpsIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetFpsIss coreid %d\r\n", core_id);

			ret = ControlIsiSetFpsIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetIspStatusIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetIspStatusIss coreid %d\r\n", core_id);

			ret = ControlIsiGetIspStatusIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetWBIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetWBIss coreid %d\r\n", core_id);

			ret = ControlIsiSetWBIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetWBIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetWBIss coreid %d\r\n", core_id);

			ret = ControlIsiGetWBIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiSetBlcIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetBlcIss coreid %d\r\n", core_id);

			ret = ControlIsiSetBlcIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;

		case RPU_2_APU_MB_CMD_FULL_BUFFER_INFORM:
			CtrlFullBufferInform(data);
			break;


		case RPU_2_APU_MB_CMD_IsiGetBlcIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetBlcIss coreid %d\r\n", core_id);

			ret = ControlIsiGetBlcIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);

			break;
		case RPU_2_APU_MB_CMD_IsiSetTpgIss:
			LOGI("RPU_2_APU_MB_CMD_IsiSetTpgIss coreid %d\r\n", core_id);

			ret = ControlIsiSetTpgIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetTpgIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetTpgIss coreid %d\r\n", core_id);

			ret = ControlIsiGetTpgIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;
		case RPU_2_APU_MB_CMD_IsiGetExpandCurveIss:
			LOGI("RPU_2_APU_MB_CMD_IsiGetExpandCurveIss coreid %d\r\n", core_id);

			ret = ControlIsiGetExpandCurveIss(data);
			packet->resp_field.error_subcode_t = ret;

			//send response result
			ret = ControlSendBackData(data, core_id);
			if (ret != RET_SUCCESS)
				LOGE("can not send response, the result is %d.\r\n", ret);
			break;


		case RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE:
			// LOGI("RPU %d requested RPU_2_APU_MB_CMD_REPORT_INTERNAL_FAILURE \n",src_cpu);
			break;
		default:
			break;
	}
	//TODO:Send response tacket
	return ret;
}


uint32_t ApuProcessFusaCommand(MBCmdId_E cmd, void *data, uint32_t size,	MboxCoreId core_id,
			       MboxCoreId src_cpu)
{
	int ret = 0, Status = 0;
	//Response_packet resp_pckt, *buf_resp = data; //response packet to be sent to APU; //Received response packet
	Payload_packet *packet = (Payload_packet *)data;
#if ENABLE_VMIX_MACRO
	XV_Mix_l2 *MixerPtr = &mix;
#endif

	switch (cmd) {
		case RPU_2_APU_MB_CMD_FUSA_EVENT_CB:
			xil_printf("RPU_2_APU_MB_CMD_FUSA_EVENT_CB coreid %d\r\n", core_id);
			CtrlCamDeviceFusaEventCb(data);

			//send response result
			ret = ControlSendResponse(data, core_id);
			if (ret != RET_SUCCESS)
				xil_printf("can not send response, the result is %d.\r\n", ret);
			break;

		default:
			break;
	}
	//TODO:Send response tacket
	return ret;
}
