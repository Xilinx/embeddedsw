// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_gamma_lut.c
 * @addtogroup v_gamma_lut Overview
 */

/***************************** Include Files *********************************/
#include "xv_gamma_lut.h"

/************************** Variable Definitions *****************************/

#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initializes the Gamma LUT core instance
 *
 * This function initializes the Gamma LUT core by setting up the instance
 * configuration structure and marking the instance as ready.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  ConfigPtr is a pointer to the configuration structure.
 * @param  EffectiveAddr is the base address of the device.
 *
 * @return XST_SUCCESS on successful initialization
 *
 * @note None
 *
 *******************************************************************************/
int XV_gamma_lut_CfgInitialize(XV_gamma_lut *InstancePtr,
                               XV_gamma_lut_Config *ConfigPtr,
                               UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

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
 * @brief Starts the Gamma LUT core
 *
 * This function starts the Gamma LUT core by setting the AP_START bit in
 * the control register while preserving the auto-restart setting.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_Start(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Checks if the Gamma LUT core has finished processing
 *
 * This function checks the AP_DONE bit in the control register to determine
 * if the core has finished processing the current frame.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return 1 if done, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_IsDone(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Gamma LUT core is idle
 *
 * This function checks the AP_IDLE bit in the control register to determine
 * if the core is idle and not processing any data.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return 1 if idle, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_IsIdle(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Gamma LUT core is ready for next input
 *
 * This function checks if the core is ready to accept new input by verifying
 * the AP_START bit is not set.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return 1 if ready, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_IsReady(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enables auto-restart mode
 *
 * This function enables the auto-restart feature, which allows the core to
 * automatically restart processing after completion.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_EnableAutoRestart(XV_gamma_lut *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disables auto-restart mode
 *
 * This function disables the auto-restart feature, requiring manual restart
 * after each frame processing completion.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_DisableAutoRestart(XV_gamma_lut *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Sets the width register value
 *
 * This function writes the width value to the hardware width register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Data is the width value to set.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_Set_HwReg_width(XV_gamma_lut *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the width register value
 *
 * This function reads and returns the width value from the hardware width register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The width value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_width(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the height register value
 *
 * This function writes the height value to the hardware height register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Data is the height value to set.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_Set_HwReg_height(XV_gamma_lut *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the height register value
 *
 * This function reads and returns the height value from the hardware height register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The height value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_height(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the video format register value
 *
 * This function writes the video format value to the hardware video format register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Data is the video format value to set.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_Set_HwReg_video_format(XV_gamma_lut *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the video format register value
 *
 * This function reads and returns the video format value from the hardware video format register.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The video format value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_video_format(XV_gamma_lut *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Gets the base address of gamma LUT 0
 *
 * This function returns the base address of the gamma LUT 0 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The base address of gamma LUT 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_BaseAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE);
}

/*****************************************************************************/
/**
 * @brief Gets the high address of gamma LUT 0
 *
 * This function returns the high address of the gamma LUT 0 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The high address of gamma LUT 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_HighAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH);
}

/*****************************************************************************/
/**
 * @brief Gets the total bytes of gamma LUT 0
 *
 * This function calculates and returns the total size in bytes of the gamma LUT 0 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The total bytes of gamma LUT 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_TotalBytes(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Gets the bit width of gamma LUT 0 entries
 *
 * This function returns the bit width of each entry in gamma LUT 0.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The bit width of gamma LUT 0 entries
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_BitWidth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_0;
}

/*****************************************************************************/
/**
 * @brief Gets the depth of gamma LUT 0
 *
 * This function returns the depth (number of entries) of gamma LUT 0.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The depth of gamma LUT 0
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_0_Depth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_0;
}

/*****************************************************************************/
/**
 * @brief Writes words to gamma LUT 0
 *
 * This function writes multiple 32-bit words to gamma LUT 0 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of words to write.
 *
 * @return The number of words written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_0_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads words from gamma LUT 0
 *
 * This function reads multiple 32-bit words from gamma LUT 0 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of words to read.
 *
 * @return The number of words read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_0_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + (offset + i)*4);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Writes bytes to gamma LUT 0
 *
 * This function writes multiple bytes to gamma LUT 0 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of bytes to write.
 *
 * @return The number of bytes written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_0_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + offset + i) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads bytes from gamma LUT 0
 *
 * This function reads multiple bytes from gamma LUT 0 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of bytes to read.
 *
 * @return The number of bytes read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_0_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_0_BASE + offset + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Gets the base address of gamma LUT 1
 *
 * This function returns the base address of the gamma LUT 1 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The base address of gamma LUT 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_BaseAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE);
}

/*****************************************************************************/
/**
 * @brief Gets the high address of gamma LUT 1
 *
 * This function returns the high address of the gamma LUT 1 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The high address of gamma LUT 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_HighAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH);
}

/*****************************************************************************/
/**
 * @brief Gets the total bytes of gamma LUT 1
 *
 * This function calculates and returns the total size in bytes of the gamma LUT 1 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The total bytes of gamma LUT 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_TotalBytes(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Gets the bit width of gamma LUT 1 entries
 *
 * This function returns the bit width of each entry in gamma LUT 1.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The bit width of gamma LUT 1 entries
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_BitWidth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_1;
}

/*****************************************************************************/
/**
 * @brief Gets the depth of gamma LUT 1
 *
 * This function returns the depth (number of entries) of gamma LUT 1.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The depth of gamma LUT 1
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_1_Depth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_1;
}

/*****************************************************************************/
/**
 * @brief Writes words to gamma LUT 1
 *
 * This function writes multiple 32-bit words to gamma LUT 1 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of words to write.
 *
 * @return The number of words written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_1_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads words from gamma LUT 1
 *
 * This function reads multiple 32-bit words from gamma LUT 1 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of words to read.
 *
 * @return The number of words read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_1_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + (offset + i)*4);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Writes bytes to gamma LUT 1
 *
 * This function writes multiple bytes to gamma LUT 1 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of bytes to write.
 *
 * @return The number of bytes written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_1_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + offset + i) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads bytes from gamma LUT 1
 *
 * This function reads multiple bytes from gamma LUT 1 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of bytes to read.
 *
 * @return The number of bytes read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_1_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_1_BASE + offset + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Gets the base address of gamma LUT 2
 *
 * This function returns the base address of the gamma LUT 2 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The base address of gamma LUT 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_BaseAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE);
}

/*****************************************************************************/
/**
 * @brief Gets the high address of gamma LUT 2
 *
 * This function returns the high address of the gamma LUT 2 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The high address of gamma LUT 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_HighAddress(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH);
}

/*****************************************************************************/
/**
 * @brief Gets the total bytes of gamma LUT 2
 *
 * This function calculates and returns the total size in bytes of the gamma LUT 2 memory region.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The total bytes of gamma LUT 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_TotalBytes(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + 1);
}

/*****************************************************************************/
/**
 * @brief Gets the bit width of gamma LUT 2 entries
 *
 * This function returns the bit width of each entry in gamma LUT 2.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The bit width of gamma LUT 2 entries
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_BitWidth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_WIDTH_HWREG_GAMMA_LUT_2;
}

/*****************************************************************************/
/**
 * @brief Gets the depth of gamma LUT 2
 *
 * This function returns the depth (number of entries) of gamma LUT 2.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The depth of gamma LUT 2
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Get_HwReg_gamma_lut_2_Depth(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_GAMMA_LUT_CTRL_DEPTH_HWREG_GAMMA_LUT_2;
}

/*****************************************************************************/
/**
 * @brief Writes words to gamma LUT 2
 *
 * This function writes multiple 32-bit words to gamma LUT 2 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of words to write.
 *
 * @return The number of words written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_2_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads words from gamma LUT 2
 *
 * This function reads multiple 32-bit words from gamma LUT 2 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the word offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of words to read.
 *
 * @return The number of words read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_2_Words(XV_gamma_lut *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + (offset + i)*4);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Writes bytes to gamma LUT 2
 *
 * This function writes multiple bytes to gamma LUT 2 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start writing.
 * @param  data is a pointer to the data array to write.
 * @param  length is the number of bytes to write.
 *
 * @return The number of bytes written, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Write_HwReg_gamma_lut_2_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + offset + i) = *(data + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Reads bytes from gamma LUT 2
 *
 * This function reads multiple bytes from gamma LUT 2 starting at the
 * specified offset.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  offset is the byte offset to start reading.
 * @param  data is a pointer to the data array to store read values.
 * @param  length is the number of bytes to read.
 *
 * @return The number of bytes read, or 0 if offset+length exceeds bounds
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_Read_HwReg_gamma_lut_2_Bytes(XV_gamma_lut *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_HIGH - XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_GAMMA_LUT_CTRL_ADDR_HWREG_GAMMA_LUT_2_BASE + offset + i);
    }
    return length;
}

/*****************************************************************************/
/**
 * @brief Enables global interrupts
 *
 * This function enables the global interrupt output by setting the GIE
 * (Global Interrupt Enable) bit.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_InterruptGlobalEnable(XV_gamma_lut *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disables global interrupts
 *
 * This function disables the global interrupt output by clearing the GIE
 * (Global Interrupt Enable) bit.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_InterruptGlobalDisable(XV_gamma_lut *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enables specific interrupts
 *
 * This function enables individual interrupts specified by the Mask parameter
 * in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Mask is the bit mask of interrupts to enable.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_InterruptEnable(XV_gamma_lut *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_IER);
    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disables specific interrupts
 *
 * This function disables individual interrupts specified by the Mask parameter
 * in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Mask is the bit mask of interrupts to disable.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_InterruptDisable(XV_gamma_lut *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_IER);
    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clears pending interrupts
 *
 * This function clears pending interrupts specified by the Mask parameter
 * in the Interrupt Status Register (ISR).
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 * @param  Mask is the bit mask of interrupts to clear.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_gamma_lut_InterruptClear(XV_gamma_lut *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_gamma_lut_WriteReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Gets the enabled interrupt mask
 *
 * This function reads and returns the Interrupt Enable Register (IER) value,
 * indicating which interrupts are currently enabled.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The enabled interrupt mask
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_InterruptGetEnabled(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Gets the interrupt status
 *
 * This function reads and returns the Interrupt Status Register (ISR) value,
 * indicating which interrupts are currently pending.
 *
 * @param  InstancePtr is a pointer to the XV_gamma_lut instance.
 *
 * @return The interrupt status mask
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_gamma_lut_InterruptGetStatus(XV_gamma_lut *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_gamma_lut_ReadReg(InstancePtr->Config.BaseAddress, XV_GAMMA_LUT_CTRL_ADDR_ISR);
}
