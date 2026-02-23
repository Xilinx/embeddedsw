// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_scenechange.c
 * @addtogroup v_scenechange Overview
 */

/***************************** Include Files *********************************/
#include "xv_scenechange.h"



/************************** Function Definitions *****************************/

#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initialize the scene change core instance
 *
 * This function initializes the scene change hardware instance by setting the
 * base address and marking it as ready for use.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  ConfigPtr is a pointer to the configuration structure
 *
 * @return XST_SUCCESS indicating successful initialization
 *
 * @note This function is used in bare-metal (non-Linux) environments
 *
 *******************************************************************************/
int XV_scenechange_CfgInitialize(XV_scenechange *InstancePtr, XV_scenechange_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Start the scene change core
 *
 * This function starts the scene change hardware core by setting the
 * ap_start bit in the control register while preserving the auto-restart bit.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Start(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Check if core processing is done
 *
 * This function checks the ap_done bit in the control register to determine
 * if the scene change core has completed processing.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return 1 if processing is done, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_IsDone(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if core is idle
 *
 * This function checks the ap_idle bit in the control register to determine
 * if the scene change core is currently idle.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return 1 if core is idle, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_IsIdle(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if core is ready for next input
 *
 * This function checks the ap_start bit to determine if the core is ready
 * to accept new input data.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return 1 if core is ready for next input, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_IsReady(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enable auto-restart mode
 *
 * This function enables auto-restart mode by setting bit 7 in the control
 * register, causing the core to automatically restart after completion.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_EnableAutoRestart(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disable auto-restart mode
 *
 * This function disables auto-restart mode by clearing the control register,
 * requiring manual restart after each completion.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_DisableAutoRestart(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 0
 *
 * This function sets the width register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 0
 *
 * This function reads and returns the width register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 0
 *
 * This function sets the height register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 0
 *
 * This function reads and returns the height register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 0
 *
 * This function sets the stride register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 0
 *
 * This function reads and returns the stride register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 0
 *
 * This function sets the video format register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 0
 *
 * This function reads and returns the video format register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 0
 *
 * This function sets the subsample register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_0(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 0
 *
 * This function reads and returns the subsample register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 0
 *
 * This function reads and returns the SAD data register value for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad0(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD valid flag for Stream 0
 *
 * This function reads the SAD control register and returns the valid bit
 * indicating whether the SAD value is valid.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return 1 if SAD value is valid, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad0_vld(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD0_CTRL);
    return Data & 0x1;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 0
 *
 * This function sets the frame buffer address register for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 0
 *
 * This function reads and returns the frame buffer address register for stream 0.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer0_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER0_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 1
 *
 * This function sets the width register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 1
 *
 * This function reads and returns the width register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 1
 *
 * This function sets the height register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 1
 *
 * This function reads and returns the height register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 1
 *
 * This function sets the stride register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 1
 *
 * This function reads and returns the stride register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 1
 *
 * This function sets the video format register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 1
 *
 * This function reads and returns the video format register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 1
 *
 * This function sets the subsample register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_1(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_1_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 1
 *
 * This function reads and returns the subsample register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 1
 *
 * This function reads and returns the SAD data register value for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad1(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD1_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 1
 *
 * This function sets the frame buffer address register for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER1_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 1
 *
 * This function reads and returns the frame buffer address register for stream 1.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer1_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER1_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 2
 *
 * This function sets the width register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 2
 *
 * This function reads and returns the width register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 2
 *
 * This function sets the height register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 2
 *
 * This function reads and returns the height register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 2
 *
 * This function sets the stride register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 2
 *
 * This function reads and returns the stride register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 2
 *
 * This function sets the video format register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 2
 *
 * This function reads and returns the video format register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 2
 *
 * This function sets the subsample register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_2(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_2_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 2
 *
 * This function reads and returns the subsample register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 2
 *
 * This function reads and returns the SAD data register value for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad2(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD2_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 2
 *
 * This function sets the frame buffer address register for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 2
 *
 * This function reads and returns the frame buffer address register for stream 2.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer2_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 3
 *
 * This function sets the width register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 3
 *
 * This function reads and returns the width register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 3
 *
 * This function sets the height register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 3
 *
 * This function reads and returns the height register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 3
 *
 * This function sets the stride register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 3
 *
 * This function reads and returns the stride register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 3
 *
 * This function sets the video format register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 3
 *
 * This function reads and returns the video format register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 3
 *
 * This function sets the subsample register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_3(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_3_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 3
 *
 * This function reads and returns the subsample register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 3
 *
 * This function reads and returns the SAD register value for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad3(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD3_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 3
 *
 * This function sets the frame buffer base address for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 3
 *
 * This function reads and returns the frame buffer base address for stream 3.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 3
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer3_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 4
 *
 * This function sets the width register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 4
 *
 * This function reads and returns the width register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 4
 *
 * This function sets the height register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 4
 *
 * This function reads and returns the height register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 4
 *
 * This function sets the stride register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 4
 *
 * This function reads and returns the stride register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 4
 *
 * This function sets the video format register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 4
 *
 * This function reads and returns the video format register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 4
 *
 * This function sets the subsample register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_4(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_4_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 4
 *
 * This function reads and returns the subsample register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 4
 *
 * This function reads and returns the SAD register value for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad4(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD4_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 4
 *
 * This function sets the frame buffer base address for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER4_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 4
 *
 * This function reads and returns the frame buffer base address for stream 4.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 4
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer4_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER4_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 5
 *
 * This function sets the width register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 5
 *
 * This function reads and returns the width register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 5
 *
 * This function sets the height register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 5
 *
 * This function reads and returns the height register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 5
 *
 * This function sets the stride register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 5
 *
 * This function reads and returns the stride register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 5
 *
 * This function sets the video format register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 5
 *
 * This function reads and returns the video format register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 5
 *
 * This function sets the subsample register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_5(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_5_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 5
 *
 * This function reads and returns the subsample register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 5
 *
 * This function reads and returns the SAD register value for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad5(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD5_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 5
 *
 * This function sets the frame buffer base address for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER5_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 5
 *
 * This function reads and returns the frame buffer base address for stream 5.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 5
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer5_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER5_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 6
 *
 * This function sets the width register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 6
 *
 * This function reads and returns the width register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 6
 *
 * This function sets the height register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 6
 *
 * This function reads and returns the height register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 6
 *
 * This function sets the stride register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 6
 *
 * This function reads and returns the stride register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 6
 *
 * This function sets the video format register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 6
 *
 * This function reads and returns the video format register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 6
 *
 * This function sets the subsample register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_6(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_6_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 6
 *
 * This function reads and returns the subsample register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 6
 *
 * This function reads and returns the SAD register value for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad6(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD6_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 6
 *
 * This function sets the frame buffer base address for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER6_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 6
 *
 * This function reads and returns the frame buffer base address for stream 6.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 6
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer6_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER6_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set width value for Stream 7
 *
 * This function sets the width register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the width value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_width_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get width value for Stream 7
 *
 * This function reads and returns the width register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Width value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_width_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_WIDTH_7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set height value for Stream 7
 *
 * This function sets the height register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the height value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_height_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get height value for Stream 7
 *
 * This function reads and returns the height register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Height value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_height_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_HEIGHT_7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stride value for Stream 7
 *
 * This function sets the stride register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stride value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stride_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stride value for Stream 7
 *
 * This function reads and returns the stride register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stride value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stride_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STRIDE_7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set video format value for Stream 7
 *
 * This function sets the video format register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the video format value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_video_format_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get video format value for Stream 7
 *
 * This function reads and returns the video format register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Video format value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_video_format_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_VIDEO_FORMAT_7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set subsample value for Stream 7
 *
 * This function sets the subsample register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the subsample value to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_subsample_7(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_7_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get subsample value for Stream 7
 *
 * This function reads and returns the subsample register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Subsample value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_subsample_7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SUBSAMPLE_7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Get SAD (Sum of Absolute Differences) value for Stream 7
 *
 * This function reads and returns the SAD register value for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return SAD value for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_sad7(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_SAD7_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set frame buffer address for Stream 7
 *
 * This function sets the frame buffer base address for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the frame buffer address to set
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER7_V_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get frame buffer address for Stream 7
 *
 * This function reads and returns the frame buffer base address for stream 7.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Frame buffer address for stream 7
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_frm_buffer7_V(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_FRM_BUFFER7_V_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set stream enable mask
 *
 * This function sets the stream enable register to enable/disable specific
 * streams for scene change detection.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Data is the stream enable bitmask (bits 0-7 for streams 0-7)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_Set_HwReg_stream_enable(XV_scenechange *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STREAM_ENABLE_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get stream enable mask
 *
 * This function reads and returns the stream enable register indicating which
 * streams are currently enabled.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Stream enable bitmask (bits 0-7 for streams 0-7)
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_Get_HwReg_stream_enable(XV_scenechange *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_HWREG_STREAM_ENABLE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enable global interrupts
 *
 * This function enables the global interrupt output by setting the Global
 * Interrupt Enable (GIE) register.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_InterruptGlobalEnable(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disable global interrupts
 *
 * This function disables the global interrupt output by clearing the Global
 * Interrupt Enable (GIE) register.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_InterruptGlobalDisable(XV_scenechange *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enable specific interrupts
 *
 * This function enables the interrupts specified by the Mask parameter by
 * setting the corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Mask is the bitmask of interrupts to enable
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_InterruptEnable(XV_scenechange *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disable specific interrupts
 *
 * This function disables the interrupts specified by the Mask parameter by
 * clearing the corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Mask is the bitmask of interrupts to disable
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_InterruptDisable(XV_scenechange *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clear pending interrupts
 *
 * This function clears the pending interrupts specified by the Mask parameter
 * by writing to the Interrupt Status Register (ISR).
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 * @param  Mask is the bitmask of interrupts to clear
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_scenechange_InterruptClear(XV_scenechange *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_scenechange_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Get enabled interrupts
 *
 * This function reads and returns the Interrupt Enable Register (IER)
 * indicating which interrupts are currently enabled.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Bitmask of enabled interrupts
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_InterruptGetEnabled(XV_scenechange *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Get interrupt status
 *
 * This function reads and returns the Interrupt Status Register (ISR)
 * indicating which interrupts are currently pending.
 *
 * @param  InstancePtr is a pointer to the XV_scenechange instance
 *
 * @return Bitmask of pending interrupts
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_scenechange_InterruptGetStatus(XV_scenechange *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_scenechange_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_SCENECHANGE_CTRL_ADDR_ISR);
}
