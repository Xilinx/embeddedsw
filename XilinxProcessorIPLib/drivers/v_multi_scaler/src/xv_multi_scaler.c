/*************************************************************************
 * Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xv_multi_scaler.c
 * @addtogroup v_multi_scaler Overview
 */

/***************************** Include Files *********************************/
#include "xv_multi_scaler.h"


/************************** Function Implementation **************************/

#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initialize the Multi Scaler core instance
 *
 * This function initializes the Multi Scaler driver/device instance with
 * the configuration parameters from the provided config structure. It copies
 * configuration parameters and sets the instance to ready state.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance to be worked on
 * @param ConfigPtr is a pointer to the configuration structure containing
 *        hardware build-time parameters
 *
 * @return XST_SUCCESS if initialization was successful
 *
 * @note The calling application must memset the instance structure to zero
 *       before passing it to this function
 ******************************************************************************/
int XV_multi_scaler_CfgInitialize(XV_multi_scaler *InstancePtr,
		XV_multi_scaler_Config *ConfigPtr)
{
	u16 i;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	InstancePtr->MaxCols = ConfigPtr->MaxCols;
	InstancePtr->MaxRows = ConfigPtr->MaxRows;
	InstancePtr->SamplesPerClock = ConfigPtr->SamplesPerClock;
	InstancePtr->MaxDataWidth = ConfigPtr->MaxDataWidth;
	InstancePtr->PhaseShift = ConfigPtr->PhaseShift;
	InstancePtr->ScaleMode = ConfigPtr->ScaleMode;
	InstancePtr->NumTaps = ConfigPtr->NumTaps;
	InstancePtr->MaxOuts = ConfigPtr->MaxOuts;

	InstancePtr->Config.Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
	InstancePtr->Config.MaxCols = ConfigPtr->MaxCols;
	InstancePtr->Config.MaxRows = ConfigPtr->MaxRows;
	InstancePtr->Config.SamplesPerClock = ConfigPtr->SamplesPerClock;
	InstancePtr->Config.MaxDataWidth = ConfigPtr->MaxDataWidth;
	InstancePtr->Config.PhaseShift = ConfigPtr->PhaseShift;
	InstancePtr->Config.ScaleMode = ConfigPtr->ScaleMode;
	InstancePtr->Config.NumTaps = ConfigPtr->NumTaps;
	InstancePtr->Config.MaxOuts = ConfigPtr->MaxOuts;
#ifdef SDT
	InstancePtr->Config.IntrId = ConfigPtr->IntrId;
	InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Start the Multi Scaler core
 *
 * This function starts the Multi Scaler hardware by setting the AP_START bit
 * in the control register. Auto-restart mode is preserved if previously set.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_Start(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL) &
		XV_MULTI_SCALER_AP_AUTO_RESTART_BIT_MASK;
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL, Data |
		XV_MULTI_SCALER_AP_START_BIT_MASK);
}

/*****************************************************************************/
/**
 * @brief Check if Multi Scaler processing is done
 *
 * This function checks the AP_DONE bit in the control register to determine
 * if the Multi Scaler has completed the current frame processing.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Non-zero if the core has finished processing the current frame,
 *         zero otherwise
 ******************************************************************************/
u32 XV_multi_scaler_IsDone(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	return Data & XV_MULTI_SCALER_AP_DONE_BIT_MASK;
}

/*****************************************************************************/
/**
 * @brief Check if Multi Scaler is idle
 *
 * This function checks the AP_IDLE bit in the control register to determine
 * if the Multi Scaler hardware is  currently idle (not processing).
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Non-zero if the core is idle, zero if it is actively processing
 ******************************************************************************/
u32 XV_multi_scaler_IsIdle(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	return Data & XV_MULTI_SCALER_AP_IDLE_BIT_MASK;
}

/*****************************************************************************/
/**
 * @brief Check if Multi Scaler is ready for next operation
 *
 * This function checks if the Multi Scaler is ready to accept new input by
 * verifying that the AP_START bit is not set in the control register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Non-zero if the core is ready for next input, zero otherwise
 ******************************************************************************/
u32 XV_multi_scaler_IsReady(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL);
	/* check ap_start to see if the pcore is ready for next input */
	return !(Data & XV_MULTI_SCALER_AP_START_BIT_MASK);
}

/*****************************************************************************/
/**
 * @brief Enable auto-restart mode for Multi Scaler
 *
 * This function enables the auto-restart feature, allowing the Multi Scaler
 * to automatically restart processing after completing the current frame.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_EnableAutoRestart(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL,
		XV_MULTI_SCALER_AP_AUTO_RESTART_BIT_MASK);
}

/*****************************************************************************/
/**
 * @brief Disable auto-restart mode for Multi Scaler
 *
 * This function disables the auto-restart feature, requiring manual restart
 * after each frame processing operation completes.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_DisableAutoRestart(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Set the number of output channels
 *
 * This function writes the number of active output channels to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the number of output channels to configure
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_num_outs(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_NUM_OUTS_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the number of output channels
 *
 * This function reads the number of active output channels from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The number of output channels currently configured
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_num_outs(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_NUM_OUTS_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 0
 *
 * This function writes the input image width for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image width in pixels
 *
 * @return None
 *
 * @note Similar Set/Get functions exist for all 8 channels (0-7) and follow
 *       the same pattern for Width, Height, Stride, Rate, Format, and Buffer
 *       address parameters
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 0
 *
 * This function reads the input image width for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image width in pixels
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 0
 *
 * This function writes the output image width for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 0
 *
 * This function reads the output image width for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 0
 *
 * This function writes the input image height for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 0
 *
 * This function reads the input image height for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 0
 *
 * This function writes the output image height for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 0
 *
 * This function reads the output image height for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 0
 *
 * This function writes the line processing rate for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 0
 *
 * This function reads the line processing rate for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 0
 *
 * This function writes the pixel processing rate for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 0
 *
 * This function reads the pixel processing rate for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 0
 *
 * This function writes the input pixel format for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 0
 *
 * This function reads the input pixel format for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 0
 *
 * This function writes the output pixel format for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 0
 *
 * This function reads the output pixel format for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 0
 *
 * This function writes the input buffer stride for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 0
 *
 * This function reads the input buffer stride for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 0
 *
 * This function writes the output buffer stride for channel 0 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_0(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 0
 *
 * This function reads the output buffer stride for channel 0 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_0(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_0_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 0
 *
 * This function writes the 64-bit address of source image buffer 0 for channel 0
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 0
 *
 * This function reads the 64-bit address of source image buffer 0 for channel 0
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_0_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 0
 *
 * This function writes the 64-bit address of source image buffer 1 for channel 0
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 0
 *
 * This function reads the 64-bit address of source image buffer 1 for channel 0
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_0_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 0
 *
 * This function writes the 64-bit address of destination image buffer 0 for
 * channel 0 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 0
 *
 * This function reads the 64-bit address of destination image buffer 0 for
 * channel 0 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_0_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 0
 *
 * This function writes the 64-bit address of destination image buffer 1 for
 * channel 0 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 0
 *
 * This function reads the 64-bit address of destination image buffer 1 for
 * channel 0 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_0_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_0_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 1
 *
 * This function writes the input image width for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 1
 *
 * This function reads the input image width for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 1
 *
 * This function writes the output image width for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 1
 *
 * This function reads the output image width for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 1
 *
 * This function writes the input image height for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 1
 *
 * This function reads the input image height for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 1
 *
 * This function writes the output image height for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 1
 *
 * This function reads the output image height for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 1
 *
 * This function writes the line processing rate for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 1
 *
 * This function reads the line processing rate for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 1
 *
 * This function writes the pixel processing rate for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 1
 *
 * This function reads the pixel processing rate for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 1
 *
 * This function writes the input pixel format for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 1
 *
 * This function reads the input pixel format for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 1
 *
 * This function writes the output pixel format for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 1
 *
 * This function reads the output pixel format for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 1
 *
 * This function writes the input buffer stride for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 1
 *
 * This function reads the input buffer stride for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 1
 *
 * This function writes the output buffer stride for channel 1 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_1(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 1
 *
 * This function reads the output buffer stride for channel 1 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_1(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_1_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 1
 *
 * This function writes the 64-bit address of source image buffer 0 for channel 1
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 1
 *
 * This function reads the 64-bit address of source image buffer 0 for channel 1
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_1_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 1
 *
 * This function writes the 64-bit address of source image buffer 1 for channel 1
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 1
 *
 * This function reads the 64-bit address of source image buffer 1 for channel 1
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_1_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 1
 *
 * This function writes the 64-bit address of destination image buffer 0 for
 * channel 1 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 1
 *
 * This function reads the 64-bit address of destination image buffer 0 for
 * channel 1 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_1_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 1
 *
 * This function writes the 64-bit address of destination image buffer 1 for
 * channel 1 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 1
 *
 * This function reads the 64-bit address of destination image buffer 1 for
 * channel 1 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_1_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_1_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 2
 *
 * This function writes the input image width for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 2
 *
 * This function reads the input image width for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 2
 *
 * This function writes the output image width for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 2
 *
 * This function reads the output image width for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 2
 *
 * This function writes the input image height for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 2
 *
 * This function reads the input image height for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 2
 *
 * This function writes the output image height for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 2
 *
 * This function reads the output image height for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 2
 *
 * This function writes the line processing rate for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 2
 *
 * This function reads the line processing rate for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 2
 *
 * This function writes the pixel processing rate for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 2
 *
 * This function reads the pixel processing rate for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 2
 *
 * This function writes the input pixel format for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 2
 *
 * This function reads the input pixel format for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 2
 *
 * This function writes the output pixel format for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 2
 *
 * This function reads the output pixel format for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 2
 *
 * This function writes the input buffer stride for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 2
 *
 * This function reads the input buffer stride for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 2
 *
 * This function writes the output buffer stride for channel 2 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_2(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 2
 *
 * This function reads the output buffer stride for channel 2 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_2(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_2_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 2
 *
 * This function writes the 64-bit address of source image buffer 0 for channel 2
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 2
 *
 * This function reads the 64-bit address of source image buffer 0 for channel 2
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_2_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 2
 *
 * This function writes the 64-bit address of source image buffer 1 for channel 2
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 2
 *
 * This function reads the 64-bit address of source image buffer 1 for channel 2
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_2_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 2
 *
 * This function writes the 64-bit address of destination image buffer 0 for
 * channel 2 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 2
 *
 * This function reads the 64-bit address of destination image buffer 0 for
 * channel 2 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_2_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 2
 *
 * This function writes the 64-bit address of destination image buffer 1 for
 * channel 2 to the hardware registers.
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 2
 *
 * This function reads the 64-bit address of destination image buffer 1 for
 * channel 2 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_2_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_2_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 3
 *
 * This function writes the input image width for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 3
 *
 * This function reads the input image width for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 3
 *
 * This function writes the output image width for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image width in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 3
 *
 * This function reads the output image width for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image width in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 3
 *
 * This function writes the input image height for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 3
 *
 * This function reads the input image height for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 3
 *
 * This function writes the output image height for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output image height in pixels
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 3
 *
 * This function reads the output image height for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output image height in pixels
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 3
 *
 * This function writes the line processing rate for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 3
 *
 * This function reads the line processing rate for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 3
 *
 * This function writes the pixel processing rate for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 3
 *
 * This function reads the pixel processing rate for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 3
 *
 * This function writes the input pixel format for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 3
 *
 * This function reads the input pixel format for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 3
 *
 * This function writes the output pixel format for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 3
 *
 * This function reads the output pixel format for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 3
 *
 * This function writes the input buffer stride for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 3
 *
 * This function reads the input buffer stride for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 3
 *
 * This function writes the output buffer stride for channel 3 to the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value in bytes
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_3(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 3
 *
 * This function reads the output buffer stride for channel 3 from the hardware
 * register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_3(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_3_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 3
 *
 * This function writes the 64-bit address of source image buffer 0 for channel 3
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 3
 *
 * This function reads the 64-bit address of source image buffer 0 for channel 3
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_3_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 3
 *
 * This function writes the 64-bit address of source image buffer 1 for channel 3
 * to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 3
 *
 * This function reads the 64-bit address of source image buffer 1 for channel 3
 * from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_3_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 3
 *
 * This function writes the 64-bit address of destination image buffer 0 for
 * channel 3 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 3
 *
 * This function reads the 64-bit address of destination image buffer 0 for
 * channel 3 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_3_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 3
 *
 * This function writes the 64-bit address of destination image buffer 1 for
 * channel 3 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the 64-bit buffer address
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 3
 *
 * This function reads the 64-bit address of destination image buffer 1 for
 * channel 3 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The 64-bit buffer address
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_3_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_3_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 4
 *
 * This function writes the input width for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 4
 *
 * This function reads the input width for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 4
 *
 * This function writes the output width for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 4
 *
 * This function reads the output width for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 4
 *
 * This function writes the input height for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 4
 *
 * This function reads the input height for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 4
 *
 * This function writes the output height for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 4
 *
 * This function reads the output height for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 4
 *
 * This function writes the line rate for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 4
 *
 * This function reads the line rate for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 4
 *
 * This function writes the pixel rate for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 4
 *
 * This function reads the pixel rate for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 4
 *
 * This function writes the input pixel format for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 4
 *
 * This function reads the input pixel format for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 4
 *
 * This function writes the output pixel format for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 4
 *
 * This function reads the output pixel format for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 4
 *
 * This function writes the input stride for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 4
 *
 * This function reads the input stride for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 4
 *
 * This function writes the output stride for channel 4 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_4(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 4
 *
 * This function reads the output stride for channel 4 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_4(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_4_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 4
 *
 * This function writes the source image buffer 0 address for channel 4 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 4
 *
 * This function reads the source image buffer 0 address for channel 4 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_4_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 4
 *
 * This function writes the source image buffer 1 address for channel 4 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 4
 *
 * This function reads the source image buffer 1 address for channel 4 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_4_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 4
 *
 * This function writes the destination image buffer 0 address for channel 4 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 4
 *
 * This function reads the destination image buffer 0 address for channel 4 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_4_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 4
 *
 * This function writes the destination image buffer 1 address for channel 4 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 4
 *
 * This function reads the destination image buffer 1 address for channel 4 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_4_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_4_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 5
 *
 * This function writes the input width for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 5
 *
 * This function reads the input width for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 5
 *
 * This function writes the output width for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 5
 *
 * This function reads the output width for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 5
 *
 * This function writes the input height for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 5
 *
 * This function reads the input height for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 5
 *
 * This function writes the output height for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 5
 *
 * This function reads the output height for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 5
 *
 * This function writes the line rate for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 5
 *
 * This function reads the line rate for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 5
 *
 * This function writes the pixel rate for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 5
 *
 * This function reads the pixel rate for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 5
 *
 * This function writes the input pixel format for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 5
 *
 * This function reads the input pixel format for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 5
 *
 * This function writes the output pixel format for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 5
 *
 * This function reads the output pixel format for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 5
 *
 * This function writes the input stride for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 5
 *
 * This function reads the input stride for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 5
 *
 * This function writes the output stride for channel 5 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_5(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 5
 *
 * This function reads the output stride for channel 5 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_5(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_5_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 5
 *
 * This function writes the source image buffer 0 address for channel 5 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 5
 *
 * This function reads the source image buffer 0 address for channel 5 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_5_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 5
 *
 * This function writes the source image buffer 1 address for channel 5 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 5
 *
 * This function reads the source image buffer 1 address for channel 5 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_5_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 5
 *
 * This function writes the destination image buffer 0 address for channel 5 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 5
 *
 * This function reads the destination image buffer 0 address for channel 5 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_5_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 5
 *
 * This function writes the destination image buffer 1 address for channel 5 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 5
 *
 * This function reads the destination image buffer 1 address for channel 5 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_5_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_5_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 6
 *
 * This function writes the input width for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 6
 *
 * This function reads the input width for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 6
 *
 * This function writes the output width for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 6
 *
 * This function reads the output width for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 6
 *
 * This function writes the input height for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 6
 *
 * This function reads the input height for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 6
 *
 * This function writes the output height for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 6
 *
 * This function reads the output height for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 6
 *
 * This function writes the line rate for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 6
 *
 * This function reads the line rate for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 6
 *
 * This function writes the pixel rate for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 6
 *
 * This function reads the pixel rate for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 6
 *
 * This function writes the input pixel format for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 6
 *
 * This function reads the input pixel format for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 6
 *
 * This function writes the output pixel format for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 6
 *
 * This function reads the output pixel format for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 6
 *
 * This function writes the input stride for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 6
 *
 * This function reads the input stride for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 6
 *
 * This function writes the output stride for channel 6 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_6(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 6
 *
 * This function reads the output stride for channel 6 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_6(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_6_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 6
 *
 * This function writes the source image buffer 0 address for channel 6 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 6
 *
 * This function reads the source image buffer 0 address for channel 6 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_6_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 6
 *
 * This function writes the source image buffer 1 address for channel 6 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 6
 *
 * This function reads the source image buffer 1 address for channel 6 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_6_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 6
 *
 * This function writes the destination image buffer 0 address for channel 6 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 6
 *
 * This function reads the destination image buffer 0 address for channel 6 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_6_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 6
 *
 * This function writes the destination image buffer 1 address for channel 6 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 6
 *
 * This function reads the destination image buffer 1 address for channel 6 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_6_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_6_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input width for channel 7
 *
 * This function writes the input width for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input width for channel 7
 *
 * This function reads the input width for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthIn_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHIN_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output width for channel 7
 *
 * This function writes the output width for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output width value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output width for channel 7
 *
 * This function reads the output width for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output width value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_WidthOut_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_WIDTHOUT_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input height for channel 7
 *
 * This function writes the input height for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input height for channel 7
 *
 * This function reads the input height for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightIn_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTIN_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output height for channel 7
 *
 * This function writes the output height for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output height value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output height for channel 7
 *
 * This function reads the output height for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output height value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_HeightOut_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_HEIGHTOUT_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set line rate for channel 7
 *
 * This function writes the line rate for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the line rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_LineRate_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get line rate for channel 7
 *
 * This function reads the line rate for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The line rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_LineRate_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_LINERATE_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set pixel rate for channel 7
 *
 * This function writes the pixel rate for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the pixel rate value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get pixel rate for channel 7
 *
 * This function reads the pixel rate for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The pixel rate value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_PixelRate_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_PIXELRATE_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input pixel format for channel 7
 *
 * This function writes the input pixel format for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input pixel format for channel 7
 *
 * This function reads the input pixel format for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InPixelFmt_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INPIXELFMT_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output pixel format for channel 7
 *
 * This function writes the output pixel format for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output pixel format value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output pixel format for channel 7
 *
 * This function reads the output pixel format for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output pixel format value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutPixelFmt_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTPIXELFMT_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set input stride for channel 7
 *
 * This function writes the input stride for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the input stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_InStride_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input stride for channel 7
 *
 * This function reads the input stride for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The input stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_InStride_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_INSTRIDE_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set output stride for channel 7
 *
 * This function writes the output stride for channel 7 to the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the output stride value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_OutStride_7(XV_multi_scaler *InstancePtr,
	u32 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get output stride for channel 7
 *
 * This function reads the output stride for channel 7 from the hardware register.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The output stride value
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_OutStride_7(XV_multi_scaler *InstancePtr)
{
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_OUTSTRIDE_7_DATA);
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 0 address for channel 7
 *
 * This function writes the source image buffer 0 address for channel 7 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 0 address for channel 7
 *
 * This function reads the source image buffer 0 address for channel 7 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf0_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF0_7_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set source image buffer 1 address for channel 7
 *
 * This function writes the source image buffer 1 address for channel 7 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the source image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get source image buffer 1 address for channel 7
 *
 * This function reads the source image buffer 1 address for channel 7 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The source image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_srcImgBuf1_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_SRCIMGBUF1_7_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 0 address for channel 7
 *
 * This function writes the destination image buffer 0 address for channel 7 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 0 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 0 address for channel 7
 *
 * This function reads the destination image buffer 0 address for channel 7 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 0 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf0_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF0_7_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Set destination image buffer 1 address for channel 7
 *
 * This function writes the destination image buffer 1 address for channel 7 to the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Data is the destination image buffer 1 address value
 *
 * @return None
 *
 * @note None
 ******************************************************************************/
void XV_multi_scaler_Set_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr,
	u64 Data)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA,
		(u32) Data);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA + 4,
		(u32) (Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get destination image buffer 1 address for channel 7
 *
 * This function reads the destination image buffer 1 address for channel 7 from the hardware registers.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return The destination image buffer 1 address value
 *
 * @note None
 ******************************************************************************/
u64 XV_multi_scaler_Get_HwReg_dstImgBuf1_7_V(XV_multi_scaler *InstancePtr)
{
	u64 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA);
	Data += XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_DSTIMGBUF1_7_V_DATA + 4) << 32;
	return Data;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 0
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 0 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_0 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 0
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 0 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_0 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 0
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_0 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 0
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_0;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 0
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_0;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 0
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE +
		(offset + i)*4)	= *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 0
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 0
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 0
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_0_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 0
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 0 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_0 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 0
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 0 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_0 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 0
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_0 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 0
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_0;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 0
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 0.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_0_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_0;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 0
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 0
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 0
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 0
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 0 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_0_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_0_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 1
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 1 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_1 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 1
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 1 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_1 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 1
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_1 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 1
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_1;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 1
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_1;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 1
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 1
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 1
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 1
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_1_BASE +
		offset + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 1
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 1 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_1 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 1
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 1 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_1 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 1
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_1 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 1
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_1;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 1
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 1.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_1_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_1;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 1
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 1
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 1
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 1
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 1 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_1_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_1_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 2
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 2 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_2 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 2
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 2 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_2 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 2
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_2 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 2
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_2;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 2
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_2;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 2
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 2
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 2
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 2
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_2_BASE + offset
		+ i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 2
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 2 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_2 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 2
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 2 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_2 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 2
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_2 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 2
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_2;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 2
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 2.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_2_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_2;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 2
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 2
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 2
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		offset + i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 2
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 2 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_2_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_2_BASE +
		offset + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 3
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 3 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_3 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 3
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 3 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_3 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 3
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_3 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 3
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_3;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 3
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_3;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 3
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 3
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 3
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + offset
		+ i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 3
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_3_BASE + offset
		+ i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 3
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 3 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_3 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 3
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 3 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_3 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 3
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_3 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 3
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_3;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 3
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 3.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_3_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_3;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 3
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 3
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 3
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 3
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 3 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_3_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_3_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 4
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 4 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 4
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 4 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 4
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_4 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 4
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_4;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 4
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_4;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 4
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 4 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 4
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 4 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 4
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 4 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 4
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 4 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_4_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 4
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 4 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 4
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 4 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 4
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_4 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 4
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of the hfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_4;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 4
 *
 * This function returns the number of coefficient entries in the horizontal
 * filter coefficient memory bank 4.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth of the hfltCoeff_4 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_4_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_4;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 4
 *
 * This function writes an array of integer words to the horizontal filter
 * coefficient memory bank 4 at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset to start writing
 * @param data is a pointer to the integer data array
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if write failed
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 4
 *
 * This function reads an array of integer words from the horizontal filter
 * coefficient memory bank 4 at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset to start reading
 * @param data is a pointer to the integer data array to store read values
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if read failed
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 4
 *
 * This function writes an array of bytes to the horizontal filter
 * coefficient memory bank 4 at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset to start writing
 * @param data is a pointer to the byte data array
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if write failed
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 4
 *
 * This function reads an array of bytes from the horizontal filter
 * coefficient memory bank 4 at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset to start reading
 * @param data is a pointer to the byte data array to store read values
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if read failed
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_4_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_4_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 5
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 5 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_5 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 5
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 5 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_5 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 5
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_5 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 5
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_5;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 5
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_5;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 5
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 5
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 5
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 5
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_5_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 5
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 5 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_5 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 5
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 5 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_5 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 5
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_5 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 5
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_5;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 5
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 5.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_5_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_5;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 5
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 5
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 5
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 5
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 5 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_5_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_5_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 6
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 6 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_6 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 6
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 6 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_6 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 6
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_6 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 6
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_6;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 6
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_6;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 6
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 6
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 6
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 6
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_6_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 6
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 6 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_6 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 6
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 6 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_6 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 6
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_6 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 6
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_6;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 6
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 6.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_6_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_6;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 6
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 6
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 6
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 6
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 6 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_6_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_6_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of vertical filter coefficient memory bank 7
 *
 * This function returns the base address of the vertical filter coefficient
 * memory bank 7 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the vfltCoeff_7 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of vertical filter coefficient memory bank 7
 *
 * This function returns the high address of the vertical filter coefficient
 * memory bank 7 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the vfltCoeff_7 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of vertical filter coefficient memory bank 7
 *
 * This function returns the total size in bytes of the vertical filter
 * coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the vfltCoeff_7 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of vertical filter coefficient memory bank 7
 *
 * This function returns the bit width of each coefficient entry in the
 * vertical filter coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_VFLTCOEFF_7;
}

/*****************************************************************************/
/**
 * @brief Get depth of vertical filter coefficient memory bank 7
 *
 * This function returns the depth (number of entries) of the vertical filter
 * coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_vfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_VFLTCOEFF_7;
}

/*****************************************************************************/
/**
 * @brief Write words to vertical filter coefficient memory bank 7
 *
 * This function writes integer words to the vertical filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from vertical filter coefficient memory bank 7
 *
 * This function reads integer words from the vertical filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to vertical filter coefficient memory bank 7
 *
 * This function writes individual bytes to the vertical filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from vertical filter coefficient memory bank 7
 *
 * This function reads individual bytes from the vertical filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_vfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_VFLTCOEFF_7_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Get base address of horizontal filter coefficient memory bank 7
 *
 * This function returns the base address of the horizontal filter coefficient
 * memory bank 7 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Base address of the hfltCoeff_7 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BaseAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE);
}

/*****************************************************************************/
/**
 * @brief Get high address of horizontal filter coefficient memory bank 7
 *
 * This function returns the high address of the horizontal filter coefficient
 * memory bank 7 in the hardware address space.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return High address of the hfltCoeff_7 memory region
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_HighAddress(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return (InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH);
}

/*****************************************************************************/
/**
 * @brief Get total size of horizontal filter coefficient memory bank 7
 *
 * This function returns the total size in bytes of the horizontal filter
 * coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Total size of the hfltCoeff_7 memory region in bytes
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_TotalBytes(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Get bit width of horizontal filter coefficient memory bank 7
 *
 * This function returns the bit width of each coefficient entry in the
 * horizontal filter coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bit width of coefficient entries
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_BitWidth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_WIDTH_HWREG_MM_HFLTCOEFF_7;
}

/*****************************************************************************/
/**
 * @brief Get depth of horizontal filter coefficient memory bank 7
 *
 * This function returns the depth (number of entries) of the horizontal filter
 * coefficient memory bank 7.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Depth (number of coefficient entries) in the memory bank
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Get_HwReg_mm_hfltCoeff_7_Depth(XV_multi_scaler
	*InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_MULTI_SCALER_CTRL_DEPTH_HWREG_MM_HFLTCOEFF_7;
}

/*****************************************************************************/
/**
 * @brief Write words to horizontal filter coefficient memory bank 7
 *
 * This function writes integer words to the horizontal filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the data array to write
 * @param length is the number of words to write
 *
 * @return Number of words written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE +
		(offset + i)*4) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read words from horizontal filter coefficient memory bank 7
 *
 * This function reads integer words from the horizontal filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the word offset from the base address
 * @param data is a pointer to the buffer to store the read data
 * @param length is the number of words to read
 *
 * @return Number of words read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Words(XV_multi_scaler
	*InstancePtr, int offset, int *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length)*4 >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(int *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE +
		(offset + i)*4);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Write bytes to horizontal filter coefficient memory bank 7
 *
 * This function writes individual bytes to the horizontal filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the byte array to write
 * @param length is the number of bytes to write
 *
 * @return Number of bytes written, or 0 if the write would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Write_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + offset +
		i) = *(data + i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Read bytes from horizontal filter coefficient memory bank 7
 *
 * This function reads individual bytes from the horizontal filter coefficient
 * memory bank 7 starting at the specified offset.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param offset is the byte offset from the base address
 * @param data is a pointer to the buffer to store the read bytes
 * @param length is the number of bytes to read
 *
 * @return Number of bytes read, or 0 if the read would exceed memory bounds
 *
 * @note None
 ******************************************************************************/
u32 XV_multi_scaler_Read_HwReg_mm_hfltCoeff_7_Bytes(XV_multi_scaler
	*InstancePtr, int offset, char *data, int length)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	int i;

	if ((offset + length) >
		(XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_HIGH -
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + 1))
		return 0;

	for (i = 0; i < length; i++) {
		*(data + i) = *(char *)(InstancePtr->Ctrl_BaseAddress +
		XV_MULTI_SCALER_CTRL_ADDR_HWREG_MM_HFLTCOEFF_7_BASE + offset +
		i);
	}
	return length;
}

/*****************************************************************************/
/**
 * @brief Enable global interrupts for Multi Scaler
 *
 * This function enables the global interrupt enable bit, allowing interrupts
 * from the Multi Scaler to be propagated to the system.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_InterruptGlobalEnable(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disable global interrupts for Multi Scaler
 *
 * This function disables the global interrupt enable bit, preventing interrupts
 * from the Multi Scaler from reaching the system.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_InterruptGlobalDisable(XV_multi_scaler *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enable specific interrupt sources
 *
 * This function enables interrupt sources specified by the mask parameter.
 * Multiple sources can be enabled by ORing the appropriate bits.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Mask is the bitmask of interrupt sources to enable
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_InterruptEnable(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disable specific interrupt sources
 *
 * This function disables interrupt sources specified by the mask parameter.
 * Multiple sources can be disabled by ORing the appropriate bits.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Mask is the bitmask of interrupt sources to disable
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_InterruptDisable(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Register = XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clear pending interrupts
 *
 * This function clears the interrupt status bits specified by the mask
 * parameter. This should be called in the interrupt handler to acknowledge
 * processed interrupts.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 * @param Mask is the bitmask of interrupt sources to clear
 *
 * @return None
 ******************************************************************************/
void XV_multi_scaler_InterruptClear(XV_multi_scaler *InstancePtr,
	u32 Mask) {
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XV_multi_scaler_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Get enabled interrupt sources
 *
 * This function returns the current interrupt enable register value indicating
 * which interrupt sources are enabled.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bitmask of enabled interrupt sources
 ******************************************************************************/
u32 XV_multi_scaler_InterruptGetEnabled(XV_multi_scaler *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Get pending interrupt status
 *
 * This function returns the current interrupt status register value indicating
 * which interrupt sources are currently pending.
 *
 * @param InstancePtr is a pointer to the Multi Scaler instance
 *
 * @return Bitmask of pending interrupt sources
 ******************************************************************************/
u32 XV_multi_scaler_InterruptGetStatus(XV_multi_scaler *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	return XV_multi_scaler_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_MULTI_SCALER_CTRL_ADDR_ISR);
}

