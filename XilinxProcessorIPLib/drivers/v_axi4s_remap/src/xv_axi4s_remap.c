/******************************************************************************
 * Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
 * Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file xv_axi4s_remap.c
 * @addtogroup v_axi4s_remap Overview
 */

/***************************** Include Files *********************************/
#include "xv_axi4s_remap.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initialize AXI4-Stream Remap driver with configuration
 *
 * This function initializes the AXI4-Stream Remap driver instance with the
 * provided configuration structure and effective base address. It copies the
 * configuration, sets the base address, and marks the driver as ready.
 *
 * @param  InstancePtr Pointer to the driver instance structure to initialize
 * @param  ConfigPtr Pointer to the configuration structure
 * @param  EffectiveAddr Effective base address for the device registers
 *
 * @return XST_SUCCESS if initialization completed successfully
 *
 * @note This function is only available in bare-metal builds (not Linux)
 *
 *******************************************************************************/
int XV_axi4s_remap_CfgInitialize(XV_axi4s_remap *InstancePtr,
		XV_axi4s_remap_Config *ConfigPtr,
		UINTPTR EffectiveAddr) {
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

	/* Setup the instance */
	InstancePtr->Config = *ConfigPtr;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Start the AXI4-Stream Remap core operation
 *
 * This function starts the AXI4-Stream Remap core by setting the AP_START bit
 * in the control register while preserving the auto-restart setting.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Start(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL) & 0x80;
	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Check if the core operation is done
 *
 * This function checks the AP_DONE bit in the control register to determine
 * if the current operation has completed.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if operation is done, 0 otherwise
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_IsDone(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
	return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if the core is idle
 *
 * This function checks the AP_IDLE bit in the control register to determine
 * if the core is currently idle and not processing data.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if core is idle, 0 if core is busy
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_IsIdle(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
	return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if the core is ready for new input
 *
 * This function checks if the core is ready to accept new input by verifying
 * that the AP_START bit is not set.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if core is ready for new input, 0 if core is busy
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_IsReady(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL);
	// check ap_start to see if the pcore is ready for next input
	return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enable automatic restart of the core
 *
 * This function enables the auto-restart feature which causes the core to
 * automatically restart processing after completing an operation.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_EnableAutoRestart(XV_axi4s_remap *InstancePtr) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disable automatic restart of the core
 *
 * This function disables the auto-restart feature, requiring manual restart
 * via XV_axi4s_remap_Start() after each operation completes.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_DisableAutoRestart(XV_axi4s_remap *InstancePtr) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Set the video frame height
 *
 * This function configures the height (number of rows) of the video frame
 * to be processed by the AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data Frame height in pixels
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_height(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_HEIGHT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the configured video frame height
 *
 * This function reads the currently configured height (number of rows) of
 * the video frame from the AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Configured frame height in pixels
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_height(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_HEIGHT_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set the video frame width
 *
 * This function configures the width (number of columns) of the video frame
 * to be processed by the AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data Frame width in pixels
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_width(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_WIDTH_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the configured video frame width
 *
 * This function reads the currently configured width (number of columns) of
 * the video frame from the AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Configured frame width in pixels
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_width(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_WIDTH_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set the color format configuration
 *
 * This function configures the color format to be used for video stream
 * processing by the AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data Color format value
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_ColorFormat(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the configured color format
 *
 * This function reads the currently configured color format from the
 * AXI4-Stream Remap core.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Configured color format value
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_ColorFormat(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_COLORFORMAT_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set the input pixels per clock
 *
 * This function configures the number of pixels per clock cycle for the
 * input video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data Number of input pixels per clock (1, 2, or 4)
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_inPixClk(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXCLK_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the configured input pixels per clock
 *
 * This function reads the currently configured number of pixels per clock
 * cycle for the input video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Configured number of input pixels per clock
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_inPixClk(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXCLK_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set the output pixels per clock
 *
 * This function configures the number of pixels per clock cycle for the
 * output video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data Number of output pixels per clock (1, 2, or 4)
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_outPixClk(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXCLK_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the configured output pixels per clock
 *
 * This function reads the currently configured number of pixels per clock
 * cycle for the output video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Configured number of output pixels per clock
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_outPixClk(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXCLK_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input HDMI 4:2:0 mode enable
 *
 * This function enables or disables HDMI 4:2:0 format processing for the
 * input video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data 1 to enable HDMI 4:2:0 mode, 0 to disable
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_inHDMI420(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INHDMI420_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input HDMI 4:2:0 mode configuration
 *
 * This function reads whether HDMI 4:2:0 format processing is enabled for
 * the input video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if HDMI 4:2:0 mode is enabled, 0 if disabled
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_inHDMI420(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INHDMI420_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output HDMI 4:2:0 mode enable
 *
 * This function enables or disables HDMI 4:2:0 format processing for the
 * output video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data 1 to enable HDMI 4:2:0 mode, 0 to disable
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_outHDMI420(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTHDMI420_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output HDMI 4:2:0 mode configuration
 *
 * This function reads whether HDMI 4:2:0 format processing is enabled for
 * the output video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if HDMI 4:2:0 mode is enabled, 0 if disabled
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_outHDMI420(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTHDMI420_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel drop enable
 *
 * This function enables or disables pixel dropping on the input video stream
 * to adjust the effective pixel rate.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data 1 to enable pixel drop, 0 to disable
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_inPixDrop(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXDROP_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel drop configuration
 *
 * This function reads whether pixel dropping is enabled on the input video
 * stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if pixel drop is enabled, 0 if disabled
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_inPixDrop(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_INPIXDROP_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel repeat enable
 *
 * This function enables or disables pixel repetition on the output video
 * stream to adjust the effective pixel rate.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Data 1 to enable pixel repeat, 0 to disable
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_Set_outPixRepeat(XV_axi4s_remap *InstancePtr, u32 Data) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXREPEAT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel repeat configuration
 *
 * This function reads whether pixel repetition is enabled on the output
 * video stream.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return 1 if pixel repeat is enabled, 0 if disabled
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_Get_outPixRepeat(XV_axi4s_remap *InstancePtr) {
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_OUTPIXREPEAT_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Enable global interrupts
 *
 * This function enables the global interrupt output from the AXI4-Stream
 * Remap core by setting the Global Interrupt Enable (GIE) bit.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 * @note Individual interrupts must also be enabled via InterruptEnable()
 *
 *******************************************************************************/
void XV_axi4s_remap_InterruptGlobalEnable(XV_axi4s_remap *InstancePtr) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disable global interrupts
 *
 * This function disables the global interrupt output from the AXI4-Stream
 * Remap core by clearing the Global Interrupt Enable (GIE) bit.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_InterruptGlobalDisable(XV_axi4s_remap *InstancePtr) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enable specific interrupts
 *
 * This function enables individual interrupt sources by setting the
 * corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Mask Bitmask of interrupts to enable (bit 0: ap_done, bit 1: ap_ready)
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 * @note Global interrupts must also be enabled via InterruptGlobalEnable()
 *
 *******************************************************************************/
void XV_axi4s_remap_InterruptEnable(XV_axi4s_remap *InstancePtr, u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register =  XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disable specific interrupts
 *
 * This function disables individual interrupt sources by clearing the
 * corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Mask Bitmask of interrupts to disable (bit 0: ap_done, bit 1: ap_ready)
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
void XV_axi4s_remap_InterruptDisable(XV_axi4s_remap *InstancePtr, u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register =  XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clear interrupt status flags
 *
 * This function clears pending interrupt status bits by writing to the
 * Interrupt Status Register (ISR). This is a toggle-on-write register.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 * @param  Mask Bitmask of interrupt flags to clear
 *
 * @return None
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 * @note Writing 1 to a bit toggles/clears that interrupt status
 *
 *******************************************************************************/
void XV_axi4s_remap_InterruptClear(XV_axi4s_remap *InstancePtr, u32 Mask) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_axi4s_remap_WriteReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Get enabled interrupt mask
 *
 * This function reads the Interrupt Enable Register (IER) to determine which
 * interrupt sources are currently enabled.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Bitmask of enabled interrupts (bit 0: ap_done, bit 1: ap_ready)
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 *
 *******************************************************************************/
u32 XV_axi4s_remap_InterruptGetEnabled(XV_axi4s_remap *InstancePtr) {
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Get interrupt status flags
 *
 * This function reads the Interrupt Status Register (ISR) to determine which
 * interrupt conditions are currently pending.
 *
 * @param  InstancePtr Pointer to the initialized driver instance
 *
 * @return Bitmask of pending interrupt flags (bit 0: ap_done, bit 1: ap_ready)
 *
 * @note The driver instance must be ready (IsReady == XIL_COMPONENT_IS_READY)
 * @note Use InterruptClear() to clear pending interrupt flags
 *
 *******************************************************************************/
u32 XV_axi4s_remap_InterruptGetStatus(XV_axi4s_remap *InstancePtr) {
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_axi4s_remap_ReadReg(InstancePtr->Config.BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_ISR);
}
