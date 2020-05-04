/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.	All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_scenechange_l2.c
 * @addtogroup v_scenechange_v1_1
 * @{
 *
 * The SceneChange Layer-2 Driver.
 * The functions in this file provides an abstraction from the register
 * peek/poke methodology by implementing most common use-case provided by
 * the core.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  pv   10/10/18   Initial Release.
 *			 Added flushing feature support for the driver.
 *			 it supports only for memory based scenechange IP.
 *			 flush bit should be set and held (until reset) by
 *			 software to flush pending transactions.IP is expecting
 *			 a hard reset, when flushing is done.(There is a flush
 *			 status bit and is asserted when the flush is done).
 * <pre>
 *
 * ****************************************************************************/
#include "xv_scenechange.h"
#include "xv_scenechange_hw.h"

static void XV_scenechange_layer_height(XV_scenechange *InstancePtr, u32 Data,
		u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA),
			Data);
}

static void XV_scenechange_layer_set_width(XV_scenechange *InstancePtr,
					   u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA),
			Data);
}

static void XV_scenechange_layer_set_stride(XV_scenechange *InstancePtr,
					    u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA),
			Data);
}

static void XV_scenechange_layer_set_vfmt(XV_scenechange *InstancePtr,
					  u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA),
			Data);
}

static void XV_scenechange_layer_set_subsample(XV_scenechange *InstancePtr,
					       u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA),
			Data);
}

static void XV_scenechange_layer_set_bufaddr(XV_scenechange *InstancePtr,
					     u64 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA),
			(u32) Data);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			(((streamid * XV_SCD_LAYER_OFFSET) +
			XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA) + 4),
			(u32) (Data >> 32));
}

int XV_scenechange_Layer_config(XV_scenechange *InstancePtr, u8 layerid)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->LayerConfig[layerid].LayerId >
			InstancePtr->ScdConfig->NumStreams)
		return XST_FAILURE;

	if (InstancePtr->LayerConfig[layerid].Width >
			InstancePtr->ScdConfig->Cols)
		return XST_FAILURE;

	if (InstancePtr->LayerConfig[layerid].Height >
			InstancePtr->ScdConfig->Rows)
		return XST_FAILURE;

	if (!((InstancePtr->ScdConfig->EnableY8 &&
			(InstancePtr->LayerConfig[layerid].
			VFormat == XV_SCD_HAS_Y8)) ||
			(InstancePtr->ScdConfig->EnableY10 &&
			(InstancePtr->LayerConfig[layerid].
			VFormat == XV_SCD_HAS_Y10))))
		return XST_FAILURE;

	XV_scenechange_layer_set_width(InstancePtr,
			InstancePtr->LayerConfig[layerid].Width, layerid);
	XV_scenechange_layer_height(InstancePtr,
			InstancePtr->LayerConfig[layerid].Height, layerid);
	XV_scenechange_layer_set_stride(InstancePtr,
			InstancePtr->LayerConfig[layerid].Stride, layerid);
	XV_scenechange_layer_set_vfmt(InstancePtr,
			InstancePtr->LayerConfig[layerid].VFormat, layerid);
	XV_scenechange_layer_set_subsample(InstancePtr,
			InstancePtr->LayerConfig[layerid].SubSample, layerid);
	XV_scenechange_layer_set_bufaddr(InstancePtr,
			InstancePtr->LayerConfig[layerid].BufferAddr, layerid);

	return XST_SUCCESS;
}

void XV_scenechange_Layer_stream_enable(XV_scenechange *InstancePtr,
		u32 streams)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_Set_HwReg_stream_enable(InstancePtr, streams);
}

static void XV_scenechange_SetFlushbit(XV_scenechange *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress,
			XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
	Data |= XV_SCD_CTRL_FLUSH_DONE_BIT_MASK;

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, Data);
}

static u32 XV_scenechange_Get_FlushDone(XV_scenechange *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress,
			XV_SCENECHANGE_CTRL_ADDR_AP_CTRL) &
		XV_SCD_CTRL_FLUSH_DONE_BIT_MASK;
	return Data;
}

/*****************************************************************************/
/**
* This function stops the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return XST_SUCCESS if the core in stop state
*         XST_FAILURE if the core is not in stop state
*
******************************************************************************/
u32 XV_scenechange_Stop(XV_scenechange *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 cnt = 0;
	u32 Data = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_InterruptGlobalDisable(InstancePtr);
	XV_scenechange_InterruptDisable(InstancePtr,
			XV_SCENECHANGE_CTRL_ADDR_ISR_AP_DONE);
	XV_scenechange_DisableAutoRestart(InstancePtr);
	XV_scenechange_SetFlushbit(InstancePtr);

	if (InstancePtr->ScdConfig->MemoryBased) {
		do {
			Data = XV_scenechange_Get_FlushDone(InstancePtr);
			usleep(XV_SCD_WAIT_FOR_FLUSH_DELAY);
			cnt++;
		} while ((Data == 0) && (cnt < XV_SCD_WAIT_FOR_FLUSH_DONE));

		if (Data == 0)
			Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
* This function Waits for the core to reach idle state
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return XST_SUCCESS if the core is in idle state
*         XST_FAILURE if the core is not in idle state
*
******************************************************************************/
u32 XV_scenechange_WaitForIdle(XV_scenechange *InstancePtr)
{
	int Status = XST_FAILURE;
	u32 isIdle = 0;
	u32 cnt = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Wait for idle */
	do {
		isIdle = XV_scenechange_IsIdle(InstancePtr);
		cnt++;
	} while ((isIdle != 1) && (cnt < XV_SCD_IDLE_TIMEOUT));

	if (isIdle == 1)
		Status = XST_SUCCESS;

	return Status;
}

void XV_scenechange_EnableInterrupts(void *InstancePtr)
{
	XV_scenechange_InterruptEnable(InstancePtr,
			XV_SCENECHANGE_CTRL_ADDR_ISR_AP_DONE);
}
