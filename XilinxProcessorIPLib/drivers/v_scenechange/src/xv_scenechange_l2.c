/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xv_scenechange_l2.c
 * @addtogroup v_scenechange Overview
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
 * </pre>
 *
 * ****************************************************************************/

/***************************** Include Files *********************************/
#include "xv_scenechange.h"
#include "xv_scenechange_hw.h"



/************************** Function Prototypes ******************************/
static void XV_scenechange_layer_height(XV_scenechange *InstancePtr, u32 Data,
		u8 streamid);
static void XV_scenechange_layer_set_width(XV_scenechange *InstancePtr,
					   u32 Data, u8 streamid);
static void XV_scenechange_layer_set_stride(XV_scenechange *InstancePtr,
					    u32 Data, u8 streamid);
static void XV_scenechange_layer_set_vfmt(XV_scenechange *InstancePtr,
					  u32 Data, u8 streamid);
static void XV_scenechange_layer_set_subsample(XV_scenechange *InstancePtr,
					       u32 Data, u8 streamid);
static void XV_scenechange_layer_set_bufaddr(XV_scenechange *InstancePtr,
					     u64 Data, u8 streamid);
static void XV_scenechange_SetFlushbit(XV_scenechange *InstancePtr);
static u32 XV_scenechange_Get_FlushDone(XV_scenechange *InstancePtr);


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Set layer height for a specific stream
 *
 * This function writes the height value to the hardware register for the
 * specified stream layer.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
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
/*****************************************************************************/
/**
 * @brief Set layer width for a specific stream
 *
 * This function writes the width value to the hardware register for the
 * specified stream layer.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/static void XV_scenechange_layer_set_width(XV_scenechange *InstancePtr,
					   u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA),
			Data);
}
/*****************************************************************************/
/**
 * @brief Set layer stride for a specific stream
 *
 * This function writes the stride value to the hardware register for the
 * specified stream layer.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/static void XV_scenechange_layer_set_stride(XV_scenechange *InstancePtr,
					    u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA),
			Data);
}
/*****************************************************************************/
/**
 * @brief Set video format for a specific stream
 *
 * This function writes the video format value to the hardware register for
 * the specified stream layer.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/static void XV_scenechange_layer_set_vfmt(XV_scenechange *InstancePtr,
					  u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA),
			Data);
}
/*****************************************************************************/
/**
 * @brief Set subsample value for a specific stream
 *
 * This function writes the subsample value to the hardware register for the
 * specified stream layer.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/static void XV_scenechange_layer_set_subsample(XV_scenechange *InstancePtr,
					       u32 Data, u8 streamid)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress,
			((streamid * XV_SCD_LAYER_OFFSET) +
			 XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA),
			Data);
}
/*****************************************************************************/
/**
 * @brief Set frame buffer address for a specific stream
 *
 * This function writes the 64-bit frame buffer address to the hardware
 * registers for the specified stream layer. The address is written as two
 * 32-bit values.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the 64-bit buffer address to be set
 * @param  streamid is the stream identifier (0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/static void XV_scenechange_layer_set_bufaddr(XV_scenechange *InstancePtr,
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

/*****************************************************************************/
/**
 * @brief Configure layer parameters for a specific stream
 *
 * This function validates and configures all layer parameters including width,
 * height, stride, video format, subsample, and buffer address for the
 * specified layer. It performs validation checks against IP capabilities.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  layerid is the layer identifier to configure (0-7)
 *
 * @return XST_SUCCESS if layer configuration is valid and completed
 *         XST_FAILURE if configuration parameters are invalid
 *
 * @note Validates layer ID, width, height, and video format compatibility
 *
 *******************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Enable specific streams for scene change detection
 *
 * This function enables one or more streams for scene change detection by
 * writing the stream enable mask to the hardware register.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  streams is the bit mask of streams to enable (bit 0-7)
 *
 * @return None
 *
 * @note Each bit in the streams parameter corresponds to one stream
 *
 *******************************************************************************/
void XV_scenechange_Layer_stream_enable(XV_scenechange *InstancePtr,
		u32 streams)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_scenechange_Set_HwReg_stream_enable(InstancePtr, streams);
}

/*****************************************************************************/
/**
 * @brief Set the flush bit in the control register
 *
 * This function sets the flush bit to initiate flushing of pending
 * transactions in memory-based scene change IP.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note Flush bit should be held until reset and requires hard reset after flush
 *
 *******************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Get flush done status
 *
 * This function reads and returns the flush done status bit from the control
 * register, indicating whether the flush operation has completed.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Flush done status bit value (non-zero when flush is complete)
 *
 * @note None
 *
 *******************************************************************************/
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
 * @brief Stop the scene change core instance
 *
 * This function stops the core by disabling interrupts, auto-restart, and
 * initiating a flush operation. For memory-based IP, it waits for the flush
 * to complete before returning.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return XST_SUCCESS if the core is stopped and flush completed
 *         XST_FAILURE if flush operation timed out
 *
 * @note For memory-based IP, this function waits up to the flush timeout period
 *
 *******************************************************************************/
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
 * @brief Wait for the core to reach idle state
 *
 * This function polls the core idle status until the core becomes idle or
 * the timeout period is reached.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return XST_SUCCESS if the core reached idle state
 *         XST_FAILURE if the core did not reach idle within timeout period
 *
 * @note Timeout is defined by XV_SCD_IDLE_TIMEOUT constant
 *
 *******************************************************************************/
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

/*****************************************************************************/
/**
 * @brief Enable scene change interrupts
 *
 * This function enables the AP_DONE interrupt for the scene change core,
 * allowing the interrupt handler to be triggered when processing completes.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_EnableInterrupts(void *InstancePtr)
{
	XV_scenechange_InterruptEnable(InstancePtr,
			XV_SCENECHANGE_CTRL_ADDR_ISR_AP_DONE);
}
