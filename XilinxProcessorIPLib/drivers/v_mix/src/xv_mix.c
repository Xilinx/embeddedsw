// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_mix.c
* @addtogroup v_mix Overview
*/

/***************************** Include Files *********************************/
#include "xv_mix.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/**
 * Initialize the XV_mix instance.
 * @param InstancePtr Pointer to the XV_mix instance to initialize.
 * @param ConfigPtr Pointer to the configuration structure.
 * @param EffectiveAddr Base address of the hardware instance.
 *
 * This function initializes an XV_mix instance using the provided
 * configuration structure and base address. It sets up the instance
 * configuration, assigns the base address, and marks the instance as ready.
 *
 * @return XST_SUCCESS on success.
 */
int XV_mix_CfgInitialize(XV_mix *InstancePtr,
                         XV_mix_Config *ConfigPtr,
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

/**
 * Starts the XV_mix hardware core.
 *
 * This function initiates the operation of the XV_mix hardware by setting
 * the 'start' bit in the control register. It first asserts that the
 * provided instance pointer is valid and that the hardware is ready.
 * The function preserves the auto-restart bit (bit 7) in the control
 * register and sets the 'ap_start' bit (bit 0) to begin processing.
 *
 * @param InstancePtr Pointer to the XV_mix instance to be started.
 *
 * @return None.
 */
void XV_mix_Start(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * Checks if the XV_mix hardware core has completed its current operation.
 *
 * This function reads the control register of the XV_mix instance to determine
 * if the hardware operation is done. It asserts that the instance pointer is
 * valid and that the core is ready before accessing the register.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   1 if the operation is done, 0 otherwise.
 */
u32 XV_mix_IsDone(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the XV_mix hardware core is idle.
 *
 * This function reads the control register of the XV_mix instance to determine
 * if the hardware is currently idle. It asserts that the provided instance pointer
 * is not NULL and that the instance is ready before accessing the hardware register.
 *
 * @param  InstancePtr Pointer to the XV_mix instance.
 * @return 1 if the hardware is idle, 0 otherwise.
 */
u32 XV_mix_IsIdle(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * Checks if the XV_mix hardware core is ready to accept new input.
 *
 * This function reads the control register of the XV_mix instance to determine
 * if the core is ready for the next operation. It asserts that the instance
 * pointer is valid and that the instance is marked as ready.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   1 if the core is ready for new input, 0 otherwise.
 */
u32 XV_mix_IsReady(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables the auto-restart feature of the XV_mix hardware core.
 *
 * This function sets the appropriate control register bit to allow the hardware
 * core to automatically restart its operation after completing a task, without
 * requiring software intervention.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return   None.
 */
void XV_mix_EnableAutoRestart(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * Disable the auto-restart feature of the XV_mix hardware core.
 *
 * This function disables the auto-restart mechanism by writing 0 to the
 * AP_CTRL register of the hardware. When auto-restart is disabled, the core
 * will not automatically restart processing after completing a task.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return   None.
 */
void XV_mix_DisableAutoRestart(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the flush bit in the control register of the XV_mix hardware instance.
 *
 * This function reads the current value of the control register, sets the flush bit,
 * and writes the updated value back to the register. The flush bit is typically used
 * to trigger a hardware flush operation, ensuring that any pending data is processed.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @note The function asserts that the InstancePtr is not NULL and that the hardware
 *       instance is ready before performing the operation.
 * @return   None.
 */
void XV_mix_SetFlushbit(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress,
		XV_MIX_CTRL_ADDR_AP_CTRL);
    Data |= XV_MIX_CTRL_BITS_FLUSH_BIT;
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress,
		XV_MIX_CTRL_ADDR_AP_CTRL, Data);
}

/**
 * Retrieves the flush done status from the XV_mix hardware instance.
 *
 * This function reads the FLUSH status bit from the AP_CTRL register of the
 * XV_mix hardware core to determine if the flush operation has completed.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the FLUSH status bit (non-zero if flush is done, zero otherwise).
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_mix_Get_FlushDone(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_AP_CTRL)
			       & XV_MIX_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}

/**
 * Sets the hardware register width for the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with the XV_mix instance. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_width(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_WIDTH hardware register for the XV_mix instance.
 *
 * This function reads the HWREG_WIDTH register from the hardware using the base address
 * specified in the instance's configuration. It asserts that the instance pointer is not
 * NULL and that the instance is ready before accessing the hardware register.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_WIDTH register.
 */
u32 XV_mix_Get_HwReg_width(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the 'height' parameter of the XV_mix instance.
 *
 * This function writes the specified 'Data' value to the HWREG_HEIGHT register
 * of the XV_mix hardware. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_HEIGHT register.
 */
void XV_mix_Set_HwReg_height(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_HEIGHT hardware register from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value of the HWREG_HEIGHT register as a 32-bit unsigned integer.
 */
u32 XV_mix_Get_HwReg_height(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format in the XV_mix instance.
 *
 * This function writes the specified video format data to the hardware register
 * associated with the video format configuration of the XV_mix core.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The video format data to be written to the hardware register.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_video_format(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

/**
 * Retrieves the current hardware register value for the video format from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_VIDEO_FORMAT register.
 *
 * Preconditions:
 *   - InstancePtr must be a valid pointer to an initialized XV_mix structure.
 *   - The hardware must be ready for register access.
 */
u32 XV_mix_Get_HwReg_video_format(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * Sets the background Y (luminance) or R (red) value in the hardware register.
 *
 * This function writes the specified value to the background Y/R register of the
 * XV_mix hardware instance. It first asserts that the instance pointer is valid
 * and that the hardware is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to be written to the background Y/R register.
 */
void XV_mix_Set_HwReg_background_Y_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_Y_R_DATA, Data);
}

/**
 * Retrieves the value of the background Y (luminance) register from the hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The current value of the background Y (R) hardware register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * hardware component is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_background_Y_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_Y_R_DATA);
    return Data;
}

/**
 * Sets the background U (Cb) or G (Green) component hardware register for the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register responsible for
 * the background U (Cb) or G (Green) color component in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to be written to the background U/G hardware register.
 *
 * @note The function asserts that the instance pointer is not NULL and that the instance is ready.
 */
void XV_mix_Set_HwReg_background_U_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_U_G_DATA, Data);
}

/**
 * Retrieves the value of the HWReg background U/G register from the XV_mix hardware instance.
 *
 * This function reads the current value of the background U (chroma) or G (green) register
 * from the hardware register space of the XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The 32-bit value read from the HWReg background U/G register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the
 *           hardware is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_background_U_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_U_G_DATA);
    return Data;
}

/**
 * Sets the background vertical B (blue) hardware register for the XV_mix instance.
 *
 * This function writes the specified data value to the HWREG_BACKGROUND_V_B register
 * of the XV_mix hardware. It first asserts that the instance pointer is not NULL and
 * that the hardware component is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the background V_B register.
 */
void XV_mix_Set_HwReg_background_V_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_V_B_DATA, Data);
}

/**
 * Retrieves the value of the HWReg background V_B register from the XV_mix hardware instance.
 *
 * This function reads the current value of the background V_B hardware register
 * for the specified XV_mix instance. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the register read.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value read from the HWReg background V_B register.
 */
u32 XV_mix_Get_HwReg_background_V_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_BACKGROUND_V_B_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer enable in the XV_mix instance.
 *
 * This function writes the specified data value to the HWREG_LAYERENABLE register
 * of the XV_mix hardware, enabling or disabling layers as specified by the data.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the HWREG_LAYERENABLE register.
 *
 * @note The function asserts that the instance pointer is not NULL and that the
 *       instance is ready before performing the register write.
 */
void XV_mix_Set_HwReg_layerEnable(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERENABLE_DATA, Data);
}

/**
 * Retrieves the value of the hardware register that controls layer enablement
 * for the XV_mix instance.
 *
 * This function reads the HWREG_LAYERENABLE register from the hardware to
 * determine which layers are currently enabled in the video mixer.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERENABLE register, indicating the enabled
 *           layers as a bitmask.
 *
 * @note     The function asserts that the instance pointer is not NULL and that
 *           the instance is ready before accessing the hardware register.
 */
u32 XV_mix_Get_HwReg_layerEnable(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERENABLE_DATA);
    return Data;
}

/**
 * Sets the alpha value for layer 0 in the XV_mix hardware register.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the transparency (alpha) of layer 0 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to set for layer 0.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layerAlpha_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA, Data);
}

/**
 * Retrieves the hardware register value for the alpha blending parameter of layer 0.
 *
 * This function reads the value of the HWREG_LAYERALPHA_0 register from the hardware,
 * which controls the alpha (transparency) setting for layer 0 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return
 *   The current value of the HWREG_LAYERALPHA_0 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_0_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the start X position of layer 0 in the XV_mix instance.
 *
 * This function writes the specified value to the HWREG_LAYERSTARTX_0 register,
 * which controls the horizontal start position of layer 0 in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LAYERSTARTX_0 register.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_layerStartX_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartX_0 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value read from the HWReg_layerStartX_0 register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
u32 XV_mix_Get_HwReg_layerStartX_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_0_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for layer 0 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y coordinate value to set for layer 0.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * hardware is ready before writing the specified Y coordinate value to the
 * appropriate hardware register for layer 0.
 */
void XV_mix_Set_HwReg_layerStartY_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_0 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWReg_layerStartY_0 register, which typically represents
 * the starting Y coordinate for layer 0 in the video mixer hardware.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value read from the HWReg_layerStartY_0 register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the hardware is ready.
 */
u32 XV_mix_Get_HwReg_layerStartY_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_0_DATA);
    return Data;
}

/**
 * Sets the width of layer 0 in the hardware register for the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 0 of the video mixer. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 0.
 */
void XV_mix_Set_HwReg_layerWidth_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 0 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERWIDTH_0 register.
 *
 * Preconditions:
 *   - InstancePtr must be a valid pointer to an initialized XV_mix structure.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 *
 * This function reads the width configuration of layer 0 from the hardware register.
 */
u32 XV_mix_Get_HwReg_layerWidth_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_0_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 0 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * responsible for configuring the stride of layer 0. It first asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer stride 0 register.
 */
void XV_mix_Set_HwReg_layerStride_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_0 hardware register for the specified XV_mix instance.
 *
 * This function reads the value of the layer stride register (layer 0) from the hardware,
 * which is used to determine the memory stride for the first layer in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The current value of the HWREG_LAYERSTRIDE_0 register.
 */
u32 XV_mix_Get_HwReg_layerStride_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_0_DATA);
    return Data;
}

/**
 * Sets the height of layer 0 in the XV_mix hardware register.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 0 of the XV_mix instance. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to set for layer 0.
 */
void XV_mix_Set_HwReg_layerHeight_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer height 0.
 *
 * This function reads the value of the HWREG_LAYERHEIGHT_0 register from the
 * hardware associated with the given XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           - Must not be NULL.
 *           - Instance must be initialized and ready.
 *
 * @return   The value of the HWREG_LAYERHEIGHT_0 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_0_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 0 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * corresponding to layer 0. It first asserts that the provided instance pointer is not NULL
 * and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 0 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERSCALEFACTOR_0 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then reads and returns the value from the corresponding hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_0_DATA);
    return Data;
}

/**
 * Sets the video format for layer 0 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The video format value to be written to the hardware register.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before writing the specified video format value to the
 * corresponding hardware register for layer 0.
 */
void XV_mix_Set_HwReg_layerVideoFormat_0(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERVIDEOFORMAT_0 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 32-bit value read from the HWREG_LAYERVIDEOFORMAT_0 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_0(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_0_DATA);
    return Data;
}

/**
 * Sets the alpha value for layer 1 in the XV_mix hardware register.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the transparency (alpha) of layer 1 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to set for layer 1.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layerAlpha_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_1_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer alpha 1.
 *
 * This function reads the current value of the HWREG_LAYERALPHA_1 register
 * from the XV_mix hardware instance specified by InstancePtr.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           It must be initialized and ready.
 *
 * @return   The 32-bit value of the HWREG_LAYERALPHA_1 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that
 *           the instance is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_1_DATA);
    return Data;
}

/**
 * Sets the start X coordinate for layer 1 in the XV_mix hardware register.
 *
 * This function writes the specified value to the hardware register responsible
 * for the starting X position of layer 1 in the video mixer instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to set for layer 1.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layerStartX_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_1_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartX_1 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The current value of the HWReg_layerStartX_1 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_1_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for layer 1 in the hardware register.
 *
 * This function writes the specified Y coordinate value to the hardware register
 * associated with layer 1's start Y position. It first asserts that the provided
 * XV_mix instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y coordinate value to set for layer 1.
 */
void XV_mix_Set_HwReg_layerStartY_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_1_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_1 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value read from the HWReg_layerStartY_1 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_1_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 1 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 1. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 1.
 */
void XV_mix_Set_HwReg_layerWidth_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_1_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer width 1.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The value of the HWREG_LAYERWIDTH_1 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_1_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the layer stride of layer 1 in the XV_mix instance.
 *
 * This function writes the specified stride value to the hardware register associated
 * with layer 1. It first asserts that the provided instance pointer is not NULL and
 * that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The stride value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerStride_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_1_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_1 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The current value of the HWREG_LAYERSTRIDE_1 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then reads and returns the value from the HWREG_LAYERSTRIDE_1 register.
 */
u32 XV_mix_Get_HwReg_layerStride_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_1_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 1 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 1. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 1.
 */
void XV_mix_Set_HwReg_layerHeight_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_1_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the height of layer 1.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value read from the HWREG_LAYERHEIGHT_1 hardware register.
 */
u32 XV_mix_Get_HwReg_layerHeight_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_1_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 1 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 1 scaling in the XV_mix hardware. It first asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_1_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 1 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_1 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_1_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 1 in the XV_mix instance.
 *
 * This function writes the specified data to the hardware register that controls
 * the video format for layer 1 of the video mixer. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer video format register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_1(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 1.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERVIDEOFORMAT_1 register.
 *
 * This function asserts that the instance pointer is not NULL and that
 * the instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_1(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_1_DATA);
    return Data;
}

/**
 * Sets the value of the Layer 1 Buffer 1 vertical hardware register.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * Layer 1 Buffer 1's vertical parameter. The value is split into two 32-bit
 * parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer1_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the Layer 1 Buffer 1 vertical register from the hardware.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the Layer 1 Buffer 1 vertical register.
 */
u64 XV_mix_Get_HwReg_layer1_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for Layer 1 Buffer 2 Vertical parameter.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * Layer 1 Buffer 2 Vertical (HWREG_LAYER1_BUF2_V) in the XV_mix instance. The
 * value is split into two 32-bit writes to accommodate the register width.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * @note The function asserts that the instance pointer is not NULL and that the
 *       instance is ready before performing the register writes.
 */
void XV_mix_Set_HwReg_layer1_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER1_BUF2_V hardware register from the XV_mix instance.
 *
 * This function reads two consecutive 32-bit registers and combines them to form a 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The 64-bit value read from the HWREG_LAYER1_BUF2_V register.
 *
 * @note The function asserts that the instance pointer is not NULL and that the instance is ready.
 */
u64 XV_mix_Get_HwReg_layer1_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the hardware register for Layer 1 Buffer 3 virtual address.
 *
 * This function writes a 64-bit address value to the hardware register
 * associated with Layer 1 Buffer 3. The address is split into two 32-bit
 * values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit address value to be written to the register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer1_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the hardware register for layer1 buffer 3 (V) from the XV_mix instance.
 *
 * This function reads two 32-bit registers from the hardware: the lower 32 bits from
 * XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA and the upper 32 bits from
 * XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4. The two values are combined to form a 64-bit result.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The 64-bit value read from the hardware register.
 *
 * @note The function asserts that the InstancePtr is not NULL and that the instance is ready.
 */
u64 XV_mix_Get_HwReg_layer1_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER1_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 2 in the XV_mix hardware register.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the transparency (alpha) of layer 2 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to set for layer 2.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layerAlpha_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_2_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer alpha 2 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERALPHA_2 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_2_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the start X position of layer 2 in the XV_mix instance.
 *
 * This function writes the specified value to the HWREG_LAYERSTARTX_2 register,
 * which controls the horizontal start position for layer 2 in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer start X register for layer 2.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_layerStartX_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_2_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for the start X position of layer 2.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTARTX_2 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_2_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for layer 2 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y coordinate value to be set for layer 2.
 *
 * This function writes the specified Y coordinate value to the hardware register
 * associated with the start position of layer 2. It asserts that the instance
 * pointer is not NULL and that the hardware is ready before performing the write.
 */
void XV_mix_Set_HwReg_layerStartY_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_2_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_2 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value read from the HWReg_layerStartY_2 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_2_DATA);
    return Data;
}

/**
 * Sets the width of layer 2 in the hardware register for the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 2 of the video mixer. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 2.
 */
void XV_mix_Set_HwReg_layerWidth_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_2_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the width of layer 2.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERWIDTH_2 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_2_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 2 in the XV_mix instance.
 *
 * This function writes the specified data value to the HWREG_LAYERSTRIDE_2 register
 * of the XV_mix hardware. It first asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LAYERSTRIDE_2 register.
 */
void XV_mix_Set_HwReg_layerStride_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_2_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_2 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 32-bit value read from the HWREG_LAYERSTRIDE_2 register.
 */
u32 XV_mix_Get_HwReg_layerStride_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_2_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 2 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 2. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 2.
 */
void XV_mix_Set_HwReg_layerHeight_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_2_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the height
 * of layer 2 in the XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERHEIGHT_2 register.
 *
 * @note     The function asserts that the instance pointer is not NULL and
 *           that the instance is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_layerHeight_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_2_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 2 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 2 scaling in the XV_mix hardware. It first asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_2_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the scale factor
 * for layer 2 in the XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERSCALEFACTOR_2 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_2_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 2 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the HWREG_LAYERVIDEOFORMAT_2 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified data to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_2(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_2_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 2.
 *
 * This function reads the value of the HWREG_LAYERVIDEOFORMAT_2 register
 * from the hardware associated with the specified XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           It must be a valid pointer and the instance must be ready.
 *
 * @return   The 32-bit value of the HWREG_LAYERVIDEOFORMAT_2 register.
 *
 * @note     None.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_2(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_2_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 2 (buffer 1) in the hardware register.
 *
 * This function writes a 64-bit address (Data) to the hardware register
 * associated with layer 2 buffer 1. The address is split into two 32-bit
 * values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set in the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer2_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER2_BUF1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The 64-bit value read from the HWREG_LAYER2_BUF1_V register.
 */
u64 XV_mix_Get_HwReg_layer2_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 2 Buffer 2 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 2 Buffer 2 V.
 * The value is split into two 32-bit parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer2_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer2_buf2_V hardware register from the XV_mix instance.
 *
 * This function reads two 32-bit registers from the hardware (lower and upper parts)
 * and combines them into a single 64-bit value. It asserts that the provided instance
 * pointer is not NULL and that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer2_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer2_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 2 Buffer 3 Vertical (V) hardware register.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * Layer 2 Buffer 3 Vertical (V) in the XV_mix instance. The value is split into
 * two 32-bit writes to accommodate hardware register width.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer2_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER2_BUF3_V hardware register for the specified XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper parts) and combines them to form a 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 64-bit value read from the HWREG_LAYER2_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer2_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER2_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 3 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to be set for layer 3.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the alpha blending for layer 3. It asserts that the instance
 * pointer is not NULL and that the hardware is ready before performing the write.
 */
void XV_mix_Set_HwReg_layerAlpha_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_3_DATA, Data);
}

/**
 * Retrieves the hardware register value for the alpha setting of layer 3 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The value of the HWREG_LAYERALPHA_3 register.
 *
 * @note This function asserts that the instance pointer is valid and the component is ready.
 */
u32 XV_mix_Get_HwReg_layerAlpha_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_3_DATA);
    return Data;
}

/**
 * Sets the starting X coordinate for layer 3 in the hardware register.
 *
 * This function writes the specified value to the hardware register that controls
 * the starting X position of layer 3 in the video mixer instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to set for layer 3.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layerStartX_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_3_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for the start X position of layer 3.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTARTX_3 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_3_DATA);
    return Data;
}

/**
 * Sets the start Y position for layer 3 in the hardware register.
 *
 * This function writes the specified Y-coordinate value to the hardware register
 * associated with the start position of layer 3. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to set for layer 3.
 */
void XV_mix_Set_HwReg_layerStartY_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_3_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_3 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWReg_layerStartY_3 register, which typically represents
 * the starting Y coordinate for layer 3 in the video mixer hardware.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value read from the HWReg_layerStartY_3 register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the hardware is ready.
 */
u32 XV_mix_Get_HwReg_layerStartY_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_3_DATA);
    return Data;
}

/**
 * Sets the width of layer 3 in the hardware register for the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 3 of the video mixer. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to set for layer 3.
 */
void XV_mix_Set_HwReg_layerWidth_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_3_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the width of layer 3.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERWIDTH_3 register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerWidth_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_3_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 3 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 3. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer stride 3 register.
 */
void XV_mix_Set_HwReg_layerStride_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_3_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer stride 3 from the XV_mix instance.
 *
 * This function reads the value of the HWREG_LAYERSTRIDE_3 register from the hardware
 * associated with the given XV_mix instance pointer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTRIDE_3 hardware register.
 */
u32 XV_mix_Get_HwReg_layerStride_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_3_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 3 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 3.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified height value to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerHeight_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_3_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the height of layer 3.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERHEIGHT_3 register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * hardware component is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerHeight_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_3_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 3 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 3 scaling in the XV_mix hardware. It first asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_3_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 3 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_3 hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_3_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format of layer 3 in the XV_mix instance.
 *
 * This function writes the specified video format data to the hardware register
 * corresponding to layer 3. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The video format data to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_3(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_3_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 3.
 *
 * This function reads the value of the HWREG_LAYERVIDEOFORMAT_3 register from the
 * hardware, which specifies the video format configuration for layer 3 in the mixer.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           It must point to a valid, initialized XV_mix structure.
 *
 * @return   The 32-bit value of the HWREG_LAYERVIDEOFORMAT_3 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_3(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_3_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 3, buffer 1, in the hardware register.
 *
 * This function writes a 64-bit address (Data) to the hardware registers
 * associated with layer 3, buffer 1. The address is split into two 32-bit
 * values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set in the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer3_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer3_buf1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a
 * single 64-bit value, and returns the result. It asserts that the provided
 * XV_mix instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer3_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer3_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the buffer address for layer 3, buffer 2, in the hardware register.
 *
 * This function writes a 64-bit address (Data) to the hardware register
 * associated with layer 3, buffer 2, by splitting the address into two
 * 32-bit values and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set in the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer3_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER3_BUF2_V hardware register for the specified XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper) and combines them to form a 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWREG_LAYER3_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer3_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 3 Buffer 3 Vertical (V) hardware register.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * Layer 3 Buffer 3 Vertical (V) in the XV_mix instance. The value is split into
 * two 32-bit writes to accommodate hardware register width.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer3_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER3_BUF3_V hardware register for the specified XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper parts) and combines them to form a 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 64-bit value read from the HWREG_LAYER3_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer3_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER3_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 4 in the XV_mix hardware register.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the alpha blending for layer 4 of the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to set for layer 4.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr must be initialized and ready.
 */
void XV_mix_Set_HwReg_layerAlpha_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_4_DATA, Data);
}

/**
 * Retrieves the hardware register value for the alpha blending parameter of layer 4.
 *
 * This function reads the value of the HWREG_LAYERALPHA_4 register from the hardware,
 * which controls the alpha (transparency) setting for layer 4 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The current value of the HWREG_LAYERALPHA_4 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_4_DATA);
    return Data;
}

/**
 * Sets the horizontal start position (X coordinate) for layer 4 in the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The horizontal start position value to be written to the hardware register.
 *
 * This function asserts that the instance pointer is not NULL and that the hardware is ready,
 * then writes the specified value to the HWREG_LAYERSTARTX_4 register.
 */
void XV_mix_Set_HwReg_layerStartX_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_4_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTARTX_4 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWREG_LAYERSTARTX_4 register, which typically represents
 * the starting X coordinate for layer 4 in the video mixer hardware.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value read from the HWREG_LAYERSTARTX_4 register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the hardware is ready.
 */
u32 XV_mix_Get_HwReg_layerStartX_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_4_DATA);
    return Data;
}

/**
 * Sets the Y-coordinate start position for layer 4 in the hardware register.
 *
 * This function writes the specified Y-coordinate value to the hardware register
 * associated with the start position of layer 4. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to set for layer 4.
 */
void XV_mix_Set_HwReg_layerStartY_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_4_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_4 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value read from the HWReg_layerStartY_4 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_4_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 4 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 4 of the video mixer. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be written to the hardware register for layer 4.
 */
void XV_mix_Set_HwReg_layerWidth_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_4_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 4 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERWIDTH_4 register.
 *
 * This function asserts that the provided instance pointer is not NULL and that
 * the instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerWidth_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_4_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 4 in the XV_mix instance.
 *
 * This function writes the specified data value to the HWREG_LAYERSTRIDE_4 register
 * of the XV_mix hardware. It first asserts that the provided instance pointer is not
 * NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LAYERSTRIDE_4 register.
 */
void XV_mix_Set_HwReg_layerStride_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_4_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer stride 4.
 *
 * This function reads the value from the hardware register associated with
 * layer stride 4 of the XV_mix instance. It asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the read.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERSTRIDE_4 register.
 */
u32 XV_mix_Get_HwReg_layerStride_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_4_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 4 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 4. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be set for layer 4.
 */
void XV_mix_Set_HwReg_layerHeight_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_4_DATA, Data);
}

/**
 * Retrieves the hardware register value for the height of layer 4 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERHEIGHT_4 register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
u32 XV_mix_Get_HwReg_layerHeight_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_4_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 4 in the XV_mix instance.
 *
 * This function writes the specified scale factor data to the hardware register
 * associated with layer 4 scaling in the XV_mix hardware. It first asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_4_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 4 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERSCALEFACTOR_4 register.
 *
 * This function asserts that the provided instance pointer is not NULL and that the instance is ready.
 * It then reads and returns the value from the corresponding hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_4_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format of layer 4 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register that controls
 * the video format for layer 4 of the video mixer hardware. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer 4 video format hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_4(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_4_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 4.
 *
 * This function reads the value of the HWREG_LAYERVIDEOFORMAT_4 register from the hardware,
 * which specifies the video format configuration for layer 4 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERVIDEOFORMAT_4 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_4(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_4_DATA);
    return Data;
}

/**
 * Sets the value of the Layer 4 Buffer 1 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 4 Buffer 1 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer4_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer4_buf1_V hardware register from the XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper) and combines them to form a 64-bit value.
 * It asserts that the provided instance pointer is not NULL and that the instance is ready before accessing the registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer4_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer4_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for Layer 4 Buffer 2 Vertical (V) parameter.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 4 Buffer 2 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer4_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER4_BUF2_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER4_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer4_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 4 Buffer 3 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 4 Buffer 3 V.
 * The value is split into two 32-bit parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer4_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER4_BUF3_V hardware register.
 *
 * This function reads two consecutive 32-bit registers from the hardware to
 * construct a 64-bit value representing the contents of the HWREG_LAYER4_BUF3_V
 * register for layer 4, buffer 3. It first asserts that the provided instance
 * pointer is valid and that the hardware component is ready.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   64-bit value read from the HWREG_LAYER4_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer4_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER4_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the hardware register value for the alpha blending of layer 5 in the XV_mix instance.
 *
 * This function writes the specified alpha value to the hardware register controlling
 * the transparency (alpha) of layer 5 in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to be set for layer 5.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layerAlpha_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_5_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer alpha 5.
 *
 * This function reads the current value of the HWREG_LAYERALPHA_5 register
 * for the specified XV_mix instance. It asserts that the instance pointer
 * is not NULL and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The 32-bit value of the HWREG_LAYERALPHA_5 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_5_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the start X position of layer 5 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LAYERSTARTX_5 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified data to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerStartX_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_5_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTARTX_5 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWREG_LAYERSTARTX_5 register, which typically represents
 * the starting X coordinate for layer 5 in the video mixer hardware.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value read from the HWREG_LAYERSTARTX_5 register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the hardware is ready.
 */
u32 XV_mix_Get_HwReg_layerStartX_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_5_DATA);
    return Data;
}

/**
 * Sets the Y-coordinate start position for layer 5 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to set for layer 5.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * hardware is ready before writing the specified value to the corresponding
 * hardware register for layer 5's start Y position.
 */
void XV_mix_Set_HwReg_layerStartY_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_5_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_5 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the hardware register that controls the starting Y position
 * for layer 5 in the video mixer hardware. It asserts that the provided instance pointer is valid
 * and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The value of the HWReg_layerStartY_5 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_5_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 5 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 5 of the video mixer. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 5.
 */
void XV_mix_Set_HwReg_layerWidth_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_5_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 5 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERWIDTH_5 register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
u32 XV_mix_Get_HwReg_layerWidth_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_5_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 5 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 5. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the HWREG_LAYERSTRIDE_5 register.
 */
void XV_mix_Set_HwReg_layerStride_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_5_DATA, Data);
}

/**
 * Retrieves the hardware register value for the layer stride of layer 5.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERSTRIDE_5 register.
 *
 * This function asserts that the instance pointer is valid and the instance is ready,
 * then reads and returns the value from the corresponding hardware register.
 */
u32 XV_mix_Get_HwReg_layerStride_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_5_DATA);
    return Data;
}

/**
 * Sets the height value for layer 5 in the XV_mix hardware register.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 5 of the XV_mix instance. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Height value to be set for layer 5.
 */
void XV_mix_Set_HwReg_layerHeight_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_5_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the height of layer 5.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERHEIGHT_5 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_5_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 5 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * corresponding to layer 5. It first asserts that the provided instance pointer is not NULL
 * and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_5_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 5 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_5 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_5_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 5 in the XV_mix instance.
 *
 * This function writes the specified data to the hardware register that controls
 * the video format for layer 5 of the video mixer. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer 5 video format register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_5(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_5_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 5.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERVIDEOFORMAT_5 register.
 *
 * This function asserts that the instance pointer is not NULL and that
 * the instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_5(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_5_DATA);
    return Data;
}

/**
 * Sets the value of the hardware register for Layer 5 Buffer 1 Vertical (V) address.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 5 Buffer 1 V address.
 * The value is split into two 32-bit parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer5_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER5_BUF1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER5_BUF1_V register.
 */
u64 XV_mix_Get_HwReg_layer5_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 5 Buffer 2 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 5 Buffer 2 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer5_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer5_buf2_V hardware register from the XV_mix instance.
 *
 * This function reads two 32-bit registers from the hardware (lower and upper parts)
 * and combines them into a single 64-bit value. It asserts that the provided instance
 * pointer is not NULL and that the hardware component is ready before accessing the registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer5_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer5_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 5 Buffer 3 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 5 Buffer 3 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer5_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER5_BUF3_V hardware register from the XV_mix instance.
 *
 * This function reads two consecutive 32-bit registers and combines them to form a 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWREG_LAYER5_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer5_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER5_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 6 in the hardware register of the XV_mix instance.
 *
 * This function writes the specified alpha value to the hardware register
 * corresponding to layer 6, allowing control over the transparency of that layer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to be set for layer 6.
 *
 * @note The function asserts that InstancePtr is not NULL and that the instance is ready.
 */
void XV_mix_Set_HwReg_layerAlpha_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_6_DATA, Data);
}

/**
 * Retrieves the hardware register value for the alpha setting of layer 6 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERALPHA_6 register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
u32 XV_mix_Get_HwReg_layerAlpha_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_6_DATA);
    return Data;
}

/**
 * Sets the horizontal start position (X coordinate) for layer 6 in the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to set for the start of layer 6.
 *
 * This function asserts that the instance pointer is valid and that the hardware is ready,
 * then writes the specified value to the corresponding hardware register for layer 6's start X position.
 */
void XV_mix_Set_HwReg_layerStartX_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_6_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for the start X position of layer 6.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERSTARTX_6 register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerStartX_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_6_DATA);
    return Data;
}

/**
 * Sets the Y-coordinate start position for layer 6 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to set for layer 6.
 *
 * This function writes the specified Y-coordinate value to the hardware register
 * controlling the start position of layer 6. It asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 */
void XV_mix_Set_HwReg_layerStartY_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_6_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_6 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value read from the HWReg_layerStartY_6 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_6_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 6 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 6. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 6.
 */
void XV_mix_Set_HwReg_layerWidth_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_6_DATA, Data);
}

/**
 * Retrieves the width of layer 6 from the hardware register.
 *
 * This function reads the value of the HWREG_LAYERWIDTH_6 register for the specified
 * XV_mix instance and returns the width of layer 6 as a 32-bit unsigned integer.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The width of layer 6 as read from the hardware register.
 *
 * @note     The InstancePtr must be initialized and ready before calling this function.
 */
u32 XV_mix_Get_HwReg_layerWidth_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_6_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 6 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 6. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the layer stride 6 hardware register.
 */
void XV_mix_Set_HwReg_layerStride_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_6_DATA, Data);
}

/**
 * Retrieves the hardware register value for the stride of layer 6 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTRIDE_6 register.
 */
u32 XV_mix_Get_HwReg_layerStride_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_6_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 6 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 6.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified height value to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerHeight_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_6_DATA, Data);
}

/**
 * Retrieves the height value for layer 6 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The value of the layer 6 height register.
 */
u32 XV_mix_Get_HwReg_layerHeight_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_6_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 6 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 6 of the video mixer. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_6_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 6 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_6 hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_6_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format of layer 6 in the XV_mix instance.
 *
 * This function writes the specified video format data to the hardware register
 * corresponding to layer 6 of the video mixer. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The video format data to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_6(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_6_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 6.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERVIDEOFORMAT_6 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_6(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_6_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 6, buffer 1 (vertical) in the XV_mix hardware register.
 *
 * This function writes a 64-bit address value to the hardware register associated with
 * layer 6's buffer 1 (vertical) of the XV_mix instance. The address is split into two
 * 32-bit values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit address value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer6_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER6_BUF1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER6_BUF1_V register.
 */
u64 XV_mix_Get_HwReg_layer6_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 6 Buffer 2 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 6 Buffer 2 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer6_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer6_buf2_V hardware register from the XV_mix instance.
 *
 * This function reads two consecutive 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer6_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer6_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 6 Buffer 3 Vertical (V) hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 6 Buffer 3 V
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer6_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer6_buf3_V hardware register for the specified XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper parts) and combines them into a single 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 64-bit value read from the HWReg_layer6_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer6_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER6_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 7 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to be set for layer 7.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before writing the specified alpha value to the
 * corresponding hardware register for layer 7.
 */
void XV_mix_Set_HwReg_layerAlpha_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_7_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer alpha 7.
 *
 * This function reads the current value of the HWREG_LAYERALPHA_7 register
 * from the XV_mix hardware instance specified by InstancePtr.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           It must be initialized and ready.
 *
 * @return   The 32-bit value of the HWREG_LAYERALPHA_7 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that
 *           the instance is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_7_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the start X position of layer 7 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to be written to the HWREG_LAYERSTARTX_7 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified data to the corresponding hardware register for layer 7's start X position.
 */
void XV_mix_Set_HwReg_layerStartX_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_7_DATA, Data);
}

/**
 * Retrieves the hardware register value for the start X position of layer 7.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTARTX_7 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_7_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for layer 7 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to be written to the HWREG_LAYERSTARTY_7 register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready. It then writes the specified value to the hardware
 * register controlling the start Y position for layer 7.
 */
void XV_mix_Set_HwReg_layerStartY_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_7_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_7 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value read from the HWREG_LAYERSTARTY_7 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_7_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 7 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with layer 7. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 7.
 */
void XV_mix_Set_HwReg_layerWidth_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_7_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 7 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERWIDTH_7 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_7_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 7 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 7. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the layer stride 7 hardware register.
 */
void XV_mix_Set_HwReg_layerStride_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_7_DATA, Data);
}

/**
 * Retrieves the hardware register value for the layer stride of layer 7.
 *
 * This function reads the value of the HWREG_LAYERSTRIDE_7 register from the hardware,
 * which specifies the stride (in bytes) for layer 7 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTRIDE_7 register.
 */
u32 XV_mix_Get_HwReg_layerStride_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_7_DATA);
    return Data;
}

/**
 * Sets the height of layer 7 in the XV_mix hardware register.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 7 of the XV_mix instance. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to set for layer 7.
 */
void XV_mix_Set_HwReg_layerHeight_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_7_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the height
 * of layer 7 in the XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERHEIGHT_7 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_layerHeight_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_7_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 7 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 7 of the video mixer. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_7_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 7.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_7 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_7_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 7 in the XV_mix instance.
 *
 * This function writes the specified data to the hardware register that controls
 * the video format for layer 7 of the video mixer. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the layer 7 video format hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_7(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_7_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 7.
 *
 * This function reads the value of the HWREG_LAYERVIDEOFORMAT_7 register from the
 * hardware instance pointed to by InstancePtr. It asserts that the instance pointer
 * is not NULL and that the instance is ready before accessing the register.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERVIDEOFORMAT_7 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_7(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_7_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 7 (buffer 1) in the XV_mix hardware register.
 *
 * This function writes a 64-bit address value to the hardware register associated
 * with layer 7's buffer 1. The address is split into two 32-bit values and written
 * to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit address value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer7_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA + 4, (u32)(Data >> 32));
}


/**
 * Retrieves the 64-bit value of the HWReg_layer7_buf1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer7_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer7_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for Layer 7 Buffer 2 (vertical) in the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 7 Buffer 2 (vertical)
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer7_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER7_BUF2_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware to construct
 * the full 64-bit value for the Layer 7 Buffer 2 Vertical register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWREG_LAYER7_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer7_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for Layer 7 Buffer 3 Vertical (V) address.
 *
 * This function writes a 64-bit value to the hardware register associated with Layer 7's
 * third buffer vertical address in the XV_mix instance. The value is split into two 32-bit
 * parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer7_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer7_buf3_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a
 * single 64-bit value, and returns the result. It asserts that the provided
 * XV_mix instance pointer is not NULL and that the instance is ready.
 *
 * @param    InstancePtr Pointer to the XV_mix instance.
 *
 * @return   64-bit value read from the HWReg_layer7_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer7_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER7_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the alpha value for layer 8 in the hardware register.
 *
 * This function writes the specified alpha value to the hardware register
 * controlling the alpha blending for layer 8 of the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to set for layer 8.
 */
void XV_mix_Set_HwReg_layerAlpha_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_8_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer alpha 8.
 *
 * This function reads the value of the HWREG_LAYERALPHA_8 register from the
 * hardware associated with the given XV_mix instance. It asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the read operation.
 *
 * @param  InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERALPHA_8 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_8_DATA);
    return Data;
}

/**
 * Sets the start X coordinate for layer 8 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to be set for layer 8.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before writing the specified value to the corresponding
 * hardware register for layer 8's start X position.
 */
void XV_mix_Set_HwReg_layerStartX_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_8_DATA, Data);
}

/**
 * Retrieves the start X coordinate for layer 8 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERSTARTX_8 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_8_DATA);
    return Data;
}

/**
 * Sets the Y start position for layer 8 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Y start position value to write.
 */
void XV_mix_Set_HwReg_layerStartY_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_8_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTARTY_8 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTY_8 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_8_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer width 8 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer width 8 register.
 */
void XV_mix_Set_HwReg_layerWidth_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_8_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer width 8 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERWIDTH_8 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_8_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer stride 8 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the HWREG_LAYERSTRIDE_8 register.
 */
void XV_mix_Set_HwReg_layerStride_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_8_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer stride 8.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTRIDE_8 register.
 */
u32 XV_mix_Get_HwReg_layerStride_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_8_DATA);
    return Data;
}

/**
 * Sets the height of layer 8 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Height value to set for layer 8.
 */
void XV_mix_Set_HwReg_layerHeight_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_8_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERHEIGHT_8 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERHEIGHT_8 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_8_DATA);
    return Data;
}

/**
 * Sets the scale factor for layer 8 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Scale factor value to write.
 */
void XV_mix_Set_HwReg_layerScaleFactor_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_8_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 8.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_8 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_8_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer 8 video format in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer video format register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_8(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_8_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 8 video format.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERVIDEOFORMAT_8 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_8(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_8_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER8_BUF1_V hardware register for layer 8 buffer 1.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer8_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER8_BUF1_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER8_BUF1_V register.
 */
u64 XV_mix_Get_HwReg_layer8_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER8_BUF2_V hardware register for layer 8 buffer 2.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer8_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer8_buf2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer8_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer8_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 8 Buffer 3 Vertical register in the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer8_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER8_BUF3_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER8_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer8_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER8_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the hardware register value for layer alpha 9.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERALPHA_9 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_9_DATA);
    return Data;
}

/**
 * Sets the start X coordinate for layer 9 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the layer's start X coordinate.
 */
void XV_mix_Set_HwReg_layerStartX_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_9_DATA, Data);
}

/**
 * Retrieves the start X coordinate for layer 9 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTX_9 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_9_DATA);
    return Data;
}

/**
 * Sets the start Y position for layer 9 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the layer start Y position.
 */
void XV_mix_Set_HwReg_layerStartY_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_9_DATA, Data);
}

/**
 * Retrieves the start Y coordinate for layer 9 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTY_9 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_9_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer width 9 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer width 9 register.
 */
void XV_mix_Set_HwReg_layerWidth_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_9_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer width 9.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERWIDTH_9 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_9_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 9.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer stride 9 register.
 */
void XV_mix_Set_HwReg_layerStride_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_9_DATA, Data);
}


/**
 * Retrieves the hardware register value for layer stride 9.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTRIDE_9 register.
 */
u32 XV_mix_Get_HwReg_layerStride_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_9_DATA);
    return Data;
}

/**
 * Sets the height value for layer 9 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Height value to set for layer 9.
 */
void XV_mix_Set_HwReg_layerHeight_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_9_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 9 height from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the layer 9 height hardware register.
 */
u32 XV_mix_Get_HwReg_layerHeight_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_9_DATA);
    return Data;
}

/**
 * Sets the scale factor for layer 9 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Scale factor value to write.
 */
void XV_mix_Set_HwReg_layerScaleFactor_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_9_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 9.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_9 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_9_DATA);
    return Data;
}

/**
 * Sets the video format for layer 9 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Video format value to set.
 */
void XV_mix_Set_HwReg_layerVideoFormat_9(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_9_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 9 video format.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERVIDEOFORMAT_9 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_9(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_9_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 9 (buffer 1) in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data 64-bit buffer address to be set.
 */
void XV_mix_Set_HwReg_layer9_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg layer 9 buffer 1 vertical parameter from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg layer 9 buffer 1 vertical register.
 */
u64 XV_mix_Get_HwReg_layer9_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER9_BUF2_V hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer9_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer9_buf2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer9_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer9_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the buffer 3 vertical address for layer 9 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the hardware register.
 */
void XV_mix_Set_HwReg_layer9_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg layer 9 buffer 3 vertical parameter from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg layer 9 buffer 3 vertical register.
 */
u64 XV_mix_Get_HwReg_layer9_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER9_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the value of the HWREG_LAYERALPHA_10 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERALPHA_10 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_10_DATA);
    return Data;
}

/**
 * Sets the start X coordinate for layer 10 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the layer's start X coordinate.
 */
void XV_mix_Set_HwReg_layerStartX_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_10_DATA, Data);
}

/**
 * Retrieves the start X position for layer 10 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTX_10 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_10_DATA);
    return Data;
}

/**
 * Sets the Y start position for layer 10 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Y start position value to write.
 */
void XV_mix_Set_HwReg_layerStartY_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_10_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_10 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStartY_10 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_10_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer width 10 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer width 10 register.
 */
void XV_mix_Set_HwReg_layerWidth_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_10_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer width 10.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERWIDTH_10 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_10_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer stride 10 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer stride 10 register.
 */
void XV_mix_Set_HwReg_layerStride_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_10_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStride_10 register for the given XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStride_10 register.
 */
u32 XV_mix_Get_HwReg_layerStride_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_10_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer height 10 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer height 10 register.
 */
void XV_mix_Set_HwReg_layerHeight_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_10_DATA, Data);
}

/**
 * Retrieves the hardware register value for layerHeight_10 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERHEIGHT_10 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_10_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer scale factor 10.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer scale factor 10 register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_10_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 10.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_10 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_10_DATA);
    return Data;
}

/**
 * Sets the video format for layer 10 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Video format value to write.
 */
void XV_mix_Set_HwReg_layerVideoFormat_10(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_10_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 10 video format.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERVIDEOFORMAT_10 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_10(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_10_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 10 (buf1) in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set.
 */
void XV_mix_Set_HwReg_layer10_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer10_buf1_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer10_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer10_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER10_BUF2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data 64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer10_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER10_BUF2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER10_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer10_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the buffer 3 vertical register value for layer 10 in the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the hardware register.
 */
void XV_mix_Set_HwReg_layer10_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER10_BUF3_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the hardware register.
 */
u64 XV_mix_Get_HwReg_layer10_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER10_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the hardware register value for layer alpha 11.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERALPHA_11 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_11_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layerStartX_11 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layerStartX_11 register.
 */
void XV_mix_Set_HwReg_layerStartX_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_11_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartX_11 register from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStartX_11 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_11_DATA);
    return Data;
}

/**
 * Sets the Y start position for layer 11 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Y start position value to write.
 */
void XV_mix_Set_HwReg_layerStartY_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_11_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_11 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStartY_11 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_11_DATA);
    return Data;
}

/**
 * Sets the width for layer 11 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Width value to set for layer 11.
 */
void XV_mix_Set_HwReg_layerWidth_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_11_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer width 11 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERWIDTH_11 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_11_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer stride 11 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer stride 11 register.
 */
void XV_mix_Set_HwReg_layerStride_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_11_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer stride 11 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTRIDE_11 register.
 */
u32 XV_mix_Get_HwReg_layerStride_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_11_DATA);
    return Data;
}

/**
 * Sets the height value for layer 11 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Height value to be set for layer 11.
 */
void XV_mix_Set_HwReg_layerHeight_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_11_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERHEIGHT_11 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERHEIGHT_11 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_11_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer scale factor 11 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer scale factor 11 register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_11_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 11.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_11 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_11_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer 11 video format in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer video format register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_11(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_11_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 11 video format.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERVIDEOFORMAT_11 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_11(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_11_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 11 (buffer 1) in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set.
 */
void XV_mix_Set_HwReg_layer11_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg layer 11 buffer 1 register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg layer 11 buffer 1 register.
 */
u64 XV_mix_Get_HwReg_layer11_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the buffer 2 address (64-bit) for layer 11 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit address to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layer11_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer11_buf2_V register from the hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer11_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer11_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the Layer 11 Buffer 3 vertical register in the XV_mix hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer11_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER11_BUF3_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER11_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer11_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER11_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the value of the HWREG_LAYERALPHA_12 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERALPHA_12 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_12_DATA);
    return Data;
}

/**
 * Sets the start X position for layer 12 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the layer start X position.
 */
void XV_mix_Set_HwReg_layerStartX_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_12_DATA, Data);
}

/**
 * Retrieves the start X coordinate for layer 12 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTX_12 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_12_DATA);
    return Data;
}

/**
 * Sets the start Y position for hardware layer 12 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Y position value to set for layer 12.
 */
void XV_mix_Set_HwReg_layerStartY_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_12_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_12 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStartY_12 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_12_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer width 12 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer width 12 register.
 */
void XV_mix_Set_HwReg_layerWidth_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_12_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERWIDTH_12 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERWIDTH_12 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_12_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer stride 12 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer stride 12 register.
 */
void XV_mix_Set_HwReg_layerStride_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_12_DATA, Data);
}

/**
 * Retrieves the hardware register value for the layer stride of layer 12.
 *
 * This function reads the value of the HWREG_LAYERSTRIDE_12 register from the
 * hardware associated with the given XV_mix instance. It asserts that the
 * instance pointer is not NULL and that the instance is ready before accessing
 * the register.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERSTRIDE_12 register.
 */
u32 XV_mix_Get_HwReg_layerStride_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_12_DATA);
    return Data;
}

/**
 * Sets the height for layer 12 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data Height value to set for layer 12.
 */
void XV_mix_Set_HwReg_layerHeight_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_12_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerHeight_12 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerHeight_12 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_12_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer scale factor 12.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_12_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer scale factor 12.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSCALEFACTOR_12 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_12_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer 12 video format in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_12(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_12_DATA, Data);
}

/**
 * Retrieves the hardware register value for layer 12 video format.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERVIDEOFORMAT_12 register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_12(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_12_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 12 (buffer 1) in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layer12_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER12_BUF1_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the register.
 */
u64 XV_mix_Get_HwReg_layer12_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER12_BUF2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer12_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER12_BUF2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER12_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer12_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER12_BUF3_V hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to write to the register.
 */
void XV_mix_Set_HwReg_layer12_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER12_BUF3_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER12_BUF3_V register.
 */
u64 XV_mix_Get_HwReg_layer12_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER12_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the hardware register value for layer alpha 13 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERALPHA_13 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_13_DATA);
    return Data;
}

/**
 * Sets the start X position for layer 13 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the start X position.
 */
void XV_mix_Set_HwReg_layerStartX_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_13_DATA, Data);
}

/**
 * Retrieves the start X coordinate for layer 13 from the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWREG_LAYERSTARTX_13 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_13_DATA);
    return Data;
}

/**
 * Sets the start Y position for layer 13 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to set for the start Y position.
 */
void XV_mix_Set_HwReg_layerStartY_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_13_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_13 hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return Value of the HWReg_layerStartY_13 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_13_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer width 13 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer width 13 register.
 */
void XV_mix_Set_HwReg_layerWidth_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_13_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to layer width 13.
 *
 * This function reads the value from the HWREG_LAYERWIDTH_13 register of the XV_mix
 * hardware instance specified by InstancePtr. It asserts that the instance pointer is
 * not NULL and that the hardware component is ready before performing the read operation.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERWIDTH_13 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_13_DATA);
    return Data;
}

/**
 * Sets the hardware register for layer stride 13.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to write to the layer stride 13 register.
 */
void XV_mix_Set_HwReg_layerStride_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_13_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer stride 13.
 *
 * This function reads the value from the HWREG_LAYERSTRIDE_13 register
 * for the specified XV_mix instance.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The current value of the HWREG_LAYERSTRIDE_13 register.
 *
 */
u32 XV_mix_Get_HwReg_layerStride_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_13_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 13 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 13. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 13.
 */
void XV_mix_Set_HwReg_layerHeight_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_13_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to layer height 13.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value read from the HWREG_LAYERHEIGHT_13 register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerHeight_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_13_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 13 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        Value to be written to the HWREG_LAYERSCALEFACTOR_13 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified data to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_13_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 13 in the XV_mix instance.
 *
 * This function reads the value of the HWREG_LAYERSCALEFACTOR_13 register from the hardware,
 * which specifies the scale factor configuration for layer 13 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_13 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_13_DATA);
    return Data;
}

/**
 * Sets the video format for layer 13 in the XV_mix hardware register.
 *
 * This function writes the specified video format data to the hardware register
 * associated with layer 13 of the XV_mix instance. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The video format data to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_13(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_13_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 13.
 *
 * This function reads the value of the HWREG_LAYERVIDEOFORMAT_13 register
 * from the hardware associated with the given XV_mix instance.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LAYERVIDEOFORMAT_13 register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_13(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_13_DATA);
    return Data;
}

/**
 * Sets the buffer address for layer 13 (buffer 1) in the XV_mix hardware register.
 *
 * This function writes a 64-bit address (Data) to the hardware register associated
 * with layer 13's buffer 1. The address is split into two 32-bit values and written
 * to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit buffer address to be set for layer 13 buffer 1.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer13_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer13_buf1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a
 * single 64-bit value, and returns the result. It asserts that the provided
 * XV_mix instance pointer is not NULL and that the instance is ready before
 * accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return 64-bit value read from the HWReg_layer13_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer13_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER13_BUF2_V hardware register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * layer 13 buffer 2 vertical parameter. The value is split into two 32-bit
 * parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * @note The function asserts that InstancePtr is not NULL and that the instance is ready.
 */
void XV_mix_Set_HwReg_layer13_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer13_buf2_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer13_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer13_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the buffer 3 vertical register value for layer 13 in the XV_mix hardware.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * layer 13's buffer 3 vertical parameter. The value is split into two 32-bit
 * writes to accommodate hardware register width.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer13_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer13_buf3_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It first asserts that the provided XV_mix instance pointer is not NULL
 * and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer13_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer13_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER13_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the hardware register value for the alpha blending parameter of layer 14.
 *
 * This function reads the value of the HWREG_LAYERALPHA_14 register from the hardware,
 * which controls the alpha (transparency) setting for layer 14 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The current value of the HWREG_LAYERALPHA_14 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_14_DATA);
    return Data;
}

/**
 * Sets the start X coordinate for layer 14 in the XV_mix hardware register.
 *
 * This function writes the specified value to the HWREG_LAYERSTARTX_14 register
 * of the XV_mix hardware instance. It first asserts that the provided instance
 * pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to set for layer 14.
 */
void XV_mix_Set_HwReg_layerStartX_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_14_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for the start X position of layer 14.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTARTX_14 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_14_DATA);
    return Data;
}

/**
 * Sets the Y-coordinate start position for layer 14 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to be written to the hardware register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * hardware is ready before writing the specified value to the corresponding
 * hardware register for layer 14's start Y position.
 */
void XV_mix_Set_HwReg_layerStartY_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_14_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_14 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value read from the HWReg_layerStartY_14 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_14_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer width 14 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * corresponding to layer width 14. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer width 14 register.
 */
void XV_mix_Set_HwReg_layerWidth_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_14_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 14 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERWIDTH_14 hardware register.
 *
 * @note This function asserts that the instance pointer is valid and that the instance is ready.
 */
u32 XV_mix_Get_HwReg_layerWidth_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_14_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 14 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 14. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer stride 14 register.
 */
void XV_mix_Set_HwReg_layerStride_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_14_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_14 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The current value of the HWREG_LAYERSTRIDE_14 register.
 */
u32 XV_mix_Get_HwReg_layerStride_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_14_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 14 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 14. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 14.
 */
void XV_mix_Set_HwReg_layerHeight_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_14_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for layer height 14 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERHEIGHT_14 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_14_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the scale factor of layer 14 in the XV_mix instance.
 *
 * This function writes the specified scale factor value to the hardware register
 * associated with layer 14. It first asserts that the provided instance pointer is not NULL
 * and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_14_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 14 in the XV_mix instance.
 *
 * This function reads the value of the HWREG_LAYERSCALEFACTOR_14 register from the hardware,
 * which specifies the scale factor configuration for layer 14 in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERSCALEFACTOR_14 register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_14_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format of layer 14 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register that controls
 * the video format for layer 14 of the video mixer. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the layer video format register for layer 14.
 */
void XV_mix_Set_HwReg_layerVideoFormat_14(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_14_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 14.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERVIDEOFORMAT_14 register.
 *
 * This function asserts that the instance pointer is not NULL and that
 * the instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_14(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_14_DATA);
    return Data;
}


/**
 * Sets the value of the HWREG_LAYER14_BUF1_V hardware register for the specified XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with layer 14 buffer 1.
 * The value is split into two 32-bit writes to accommodate hardware register width.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - The XV_mix instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer14_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER14_BUF1_V hardware register from the XV_mix instance.
 *
 * This function reads two consecutive 32-bit registers from the hardware,
 * combines them into a single 64-bit value, and returns the result.
 * It asserts that the provided instance pointer is not NULL and that the
 * instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER14_BUF1_V register.
 */
u64 XV_mix_Get_HwReg_layer14_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWReg Layer 14 Buffer 2 V register for the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * Layer 14 Buffer 2 V. The value is split into two 32-bit parts and written to
 * consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer14_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer14_buf2_V hardware register.
 *
 * This function reads two consecutive 32-bit registers from the hardware,
 * combines them into a single 64-bit value, and returns the result.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWReg_layer14_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer14_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for layer 14 buffer 3 vertical parameter.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * layer 14 buffer 3's vertical parameter in the XV_mix instance. The value is
 * split into two 32-bit writes to accommodate the hardware register layout.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer14_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer14_buf3_V hardware register from the XV_mix instance.
 *
 * This function reads two 32-bit registers from the hardware (low and high parts)
 * and combines them into a single 64-bit value. It asserts that the provided
 * instance pointer is not NULL and that the instance is ready before accessing the hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer14_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer14_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER14_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the value of the hardware register for layer alpha 15.
 *
 * This function reads the value of the HWREG_LAYERALPHA_15 register from the
 * XV_mix hardware instance specified by InstancePtr. It asserts that the
 * instance pointer is not NULL and that the hardware is ready before
 * performing the read operation.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The 32-bit value read from the HWREG_LAYERALPHA_15 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_15_DATA);
    return Data;
}

/**
 * Sets the start X position for layer 15 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X start position value to be written to the hardware register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready before writing the specified value to the corresponding
 * hardware register for layer 15's start X position.
 */
void XV_mix_Set_HwReg_layerStartX_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_15_DATA, Data);
}

/**
 * Retrieves the value of the hardware register for the start X position of layer 15.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LAYERSTARTX_15 register.
 */
u32 XV_mix_Get_HwReg_layerStartX_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_15_DATA);
    return Data;
}

/**
 * Sets the Y-coordinate start position for layer 15 in the XV_mix hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y-coordinate value to be written to the hardware register.
 *
 * This function asserts that the instance pointer is valid and that the hardware
 * component is ready before writing the specified value to the corresponding
 * hardware register for layer 15's start Y position.
 */
void XV_mix_Set_HwReg_layerStartY_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_15_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_15 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWReg_layerStartY_15 register, which typically represents
 * the Y-coordinate start position for layer 15 in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 32-bit value read from the HWReg_layerStartY_15 register.
 */
u32 XV_mix_Get_HwReg_layerStartY_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_15_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the width of layer 15 in the XV_mix instance.
 *
 * This function writes the specified width value to the hardware register
 * corresponding to layer 15. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The width value to be set for layer 15.
 */
void XV_mix_Set_HwReg_layerWidth_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_15_DATA, Data);
}

/**
 * Retrieves the hardware register value for the width of layer 15 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERWIDTH_15 register.
 *
 * This function asserts that the provided instance pointer is not NULL and that the
 * instance is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerWidth_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_15_DATA);
    return Data;
}

/**
 * Sets the hardware register value for layer stride 15 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with layer stride 15. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the HWREG_LAYERSTRIDE_15 register.
 */
void XV_mix_Set_HwReg_layerStride_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_15_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_15 hardware register for the specified XV_mix instance.
 *
 * This function reads the value from the HWREG_LAYERSTRIDE_15 register, which typically holds
 * the stride information for layer 15 in the video mixer hardware. It performs necessary assertions
 * to ensure the instance pointer is valid and the component is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The current value of the HWREG_LAYERSTRIDE_15 register.
 */
u32 XV_mix_Get_HwReg_layerStride_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_15_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height of layer 15 in the XV_mix instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with layer 15. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The height value to be written to the hardware register for layer 15.
 */
void XV_mix_Set_HwReg_layerHeight_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_15_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to layer height 15.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERHEIGHT_15 register.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerHeight_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_15_DATA);
    return Data;
}

/**
 * Sets the scale factor for layer 15 in the XV_mix hardware register.
 *
 * This function writes the specified scale factor value to the hardware register
 * corresponding to layer 15 of the XV_mix instance. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before performing
 * the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The scale factor value to be set for layer 15.
 */
void XV_mix_Set_HwReg_layerScaleFactor_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_15_DATA, Data);
}

/**
 * Retrieves the hardware register value for the scale factor of layer 15 from the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value of the HWREG_LAYERSCALEFACTOR_15 register.
 *
 * This function asserts that the provided instance pointer is not NULL and that the instance is ready.
 * It then reads and returns the value from the corresponding hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_15_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 15 in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer video format register for layer 15.
 *
 * This function asserts that the instance pointer is not NULL and that the instance is ready.
 * It then writes the specified data to the corresponding hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_15(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_15_DATA, Data);
}

/**
 * Retrieves the hardware register value for the video format of layer 15.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The value of the HWREG_LAYERVIDEOFORMAT_15 register.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before reading the register value.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_15(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_15_DATA);
    return Data;
}

/**
 * Sets the value of the hardware register for layer 15 buffer 1 (vertical) in the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with layer 15's buffer 1
 * (vertical) by splitting the value into two 32-bit parts and writing them to consecutive addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer15_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER15_BUF1_V hardware register for the specified XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper parts) and combines them into a single 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER15_BUF1_V register.
 *
 * @note The function asserts that InstancePtr is not NULL and that the instance is ready.
 */
u64 XV_mix_Get_HwReg_layer15_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER15_BUF2_V hardware register for layer 15 buffer 2.
 *
 * This function writes a 64-bit value to the corresponding hardware register by splitting
 * the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer15_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_LAYER15_BUF2_V hardware register.
 *
 * This function reads two consecutive 32-bit registers from the hardware to
 * construct a 64-bit value representing the contents of the HWREG_LAYER15_BUF2_V
 * register for layer 15, buffer 2. It first asserts that the provided instance
 * pointer is valid and that the hardware component is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWREG_LAYER15_BUF2_V register.
 */
u64 XV_mix_Get_HwReg_layer15_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the hardware register for layer 15 buffer 3 (vertical) in the XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with layer 15 buffer 3 (vertical)
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer15_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer15_buf3_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer15_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer15_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER15_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the value of the HWREG_LAYERALPHA_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The 32-bit value read from the HWREG_LAYERALPHA_16 register.
 */
u32 XV_mix_Get_HwReg_layerAlpha_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERALPHA_16_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the start X position of layer 16 in the XV_mix instance.
 *
 * This function writes the specified value to the HWREG_LAYERSTARTX_16 register,
 * which controls the horizontal start position of layer 16 in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LAYERSTARTX_16 register.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_layerStartX_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_16_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartX_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The current value of the HWReg_layerStartX_16 register.
 *
 * @note This function asserts if the instance pointer is NULL or the instance is not ready.
 */
u32 XV_mix_Get_HwReg_layerStartX_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTX_16_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for layer 16 in the hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y coordinate value to be set for layer 16.
 *
 * This function asserts that the instance pointer is valid and that the
 * hardware is ready before writing the specified Y coordinate value to the
 * corresponding hardware register for layer 16.
 */
void XV_mix_Set_HwReg_layerStartY_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_16_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_layerStartY_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The value read from the HWReg_layerStartY_16 register.
 *
 * @note This function asserts if the instance pointer is NULL or the instance is not ready.
 */
u32 XV_mix_Get_HwReg_layerStartY_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTARTY_16_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the layer width (16-bit) in the XV_mix instance.
 *
 * This function writes the specified 32-bit data value to the hardware register
 * responsible for configuring the width of a specific layer in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        32-bit value to be written to the layer width register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layerWidth_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_16_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERWIDTH_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The current value of the HWREG_LAYERWIDTH_16 register.
 */
u32 XV_mix_Get_HwReg_layerWidth_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERWIDTH_16_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the layer stride (16-bit) in the XV_mix instance.
 *
 * This function writes the specified 32-bit data value to the hardware register
 * responsible for controlling the layer stride (16-bit) parameter of the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        32-bit value to be written to the layer stride register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layerStride_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_16_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSTRIDE_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The current value of the HWREG_LAYERSTRIDE_16 register.
 */
u32 XV_mix_Get_HwReg_layerStride_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSTRIDE_16_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_LAYERHEIGHT_16 hardware register for the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the HWREG_LAYERHEIGHT_16 register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance
 * is ready before writing the specified data to the hardware register.
 */
void XV_mix_Set_HwReg_layerHeight_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_16_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERHEIGHT_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value read from the HWREG_LAYERHEIGHT_16 register.
 */
u32 XV_mix_Get_HwReg_layerHeight_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERHEIGHT_16_DATA);
    return Data;
}

/**
 * Sets the hardware register for the layer scale factor 16 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register
 * associated with the layer scale factor 16. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to write to the layer scale factor 16 register.
 */
void XV_mix_Set_HwReg_layerScaleFactor_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_16_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERSCALEFACTOR_16 hardware register for the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The value read from the HWREG_LAYERSCALEFACTOR_16 register.
 *
 * This function asserts that the provided instance pointer is not NULL and that the instance is ready.
 * It then reads and returns the value from the corresponding hardware register.
 */
u32 XV_mix_Get_HwReg_layerScaleFactor_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERSCALEFACTOR_16_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the video format of layer 16 in the XV_mix instance.
 *
 * This function writes the specified data value to the hardware register that controls
 * the video format for layer 16 of the video mixer. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the layer 16 video format hardware register.
 */
void XV_mix_Set_HwReg_layerVideoFormat_16(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_16_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LAYERVIDEOFORMAT_16 hardware register for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 32-bit value read from the HWREG_LAYERVIDEOFORMAT_16 register.
 *
 * @note This function asserts if the instance pointer is NULL or the instance is not ready.
 */
u32 XV_mix_Get_HwReg_layerVideoFormat_16(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYERVIDEOFORMAT_16_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER16_BUF1_V hardware register for layer 16 buffer 1.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * layer 16 buffer 1 vertical parameter. The value is split into two 32-bit parts
 * and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer16_buf1_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer16_buf1_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before accessing the hardware registers.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer16_buf1_V register.
 */
u64 XV_mix_Get_HwReg_layer16_buf1_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF1_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER16_BUF2_V hardware register for the specified XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with layer 16 buffer 2.
 * The value is split into two 32-bit parts and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_mix_Set_HwReg_layer16_buf2_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer16_buf2_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided XV_mix instance pointer is not NULL and
 * that the instance is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return 64-bit value read from the HWReg_layer16_buf2_V register.
 */
u64 XV_mix_Get_HwReg_layer16_buf2_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_LAYER16_BUF3_V hardware register for the specified XV_mix instance.
 *
 * This function writes a 64-bit value to the hardware register associated with layer 16 buffer 3 (vertical)
 * by splitting the value into two 32-bit parts and writing them to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 */
void XV_mix_Set_HwReg_layer16_buf3_V(XV_mix *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA, (u32)(Data));
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWReg_layer16_buf3_V hardware register from the XV_mix instance.
 *
 * This function reads two 32-bit registers (lower and upper parts) and combines them into a single 64-bit value.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWReg_layer16_buf3_V register.
 */
u64 XV_mix_Get_HwReg_layer16_buf3_V(XV_mix *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA);
    Data += (u64)XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LAYER16_BUF3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_RESERVE hardware register for the XV_mix instance.
 *
 * This function writes the specified 32-bit value to the HWREG_RESERVE register
 * of the XV_mix hardware. It first asserts that the provided instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_RESERVE register.
 */
void XV_mix_Set_HwReg_reserve(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_RESERVE_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_RESERVE register from the XV_mix hardware instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 32-bit value read from the HWREG_RESERVE register.
 *
 * @note This function asserts if the instance pointer is NULL or the instance is not ready.
 */
u32 XV_mix_Get_HwReg_reserve(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_RESERVE_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the logo's starting X coordinate.
 *
 * This function writes the specified value to the HWREG_LOGOSTARTX register,
 * which determines the horizontal start position of the logo in the video mixer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The X coordinate value to set for the logo start position.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_logoStartX(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTX_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_logoStartX hardware register from the XV_mix instance.
 *
 * This function reads the current value of the HWReg_logoStartX register, which typically
 * represents the starting X coordinate for a logo overlay in the video mixer hardware.
 *
 * @param    InstancePtr  Pointer to the XV_mix instance.
 *
 * @return   The value of the HWReg_logoStartX register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_mix_Get_HwReg_logoStartX(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTX_DATA);
    return Data;
}

/**
 * Sets the starting Y coordinate for the logo in the hardware register.
 *
 * This function writes the specified Y coordinate value to the HWREG_LOGOSTARTY
 * register of the XV_mix hardware instance. It first asserts that the instance
 * pointer is not NULL and that the hardware is ready before performing the write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The Y coordinate value to set for the logo start position.
 */
void XV_mix_Set_HwReg_logoStartY(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTY_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_logoStartY hardware register.
 *
 * This function reads the value of the HWReg_logoStartY register from the
 * hardware instance pointed to by InstancePtr. It asserts that the instance
 * pointer is not NULL and that the instance is ready before accessing the
 * register.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value read from the HWReg_logoStartY register.
 */
u32 XV_mix_Get_HwReg_logoStartY(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSTARTY_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the logo width in the XV_mix instance.
 *
 * This function writes the specified logo width value to the corresponding
 * hardware register of the XV_mix core. It first asserts that the instance
 * pointer is not NULL and that the core is ready before performing the write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The logo width value to be written to the hardware register.
 */
void XV_mix_Set_HwReg_logoWidth(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOWIDTH_DATA, Data);
}

/**
 * Retrieves the current value of the logo width hardware register.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 *                      Must not be NULL and must be initialized.
 *
 * @return The value of the HWREG_LOGOWIDTH register.
 *
 * @note   This function asserts that the instance pointer is valid and
 *         that the component is ready before accessing the register.
 */
u32 XV_mix_Get_HwReg_logoWidth(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOWIDTH_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the logo height in the XV_mix instance.
 *
 * This function writes the specified logo height value to the hardware register
 * associated with the logo height parameter of the XV_mix core.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the logo height hardware register.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_logoHeight(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOHEIGHT_DATA, Data);
}

/**
 * Retrieves the value of the hardware register corresponding to the logo height.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 *
 * @return The value of the HWREG_LOGOHEIGHT register.
 */
u32 XV_mix_Get_HwReg_logoHeight(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOHEIGHT_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the logo scale factor in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to set for the logo scale factor hardware register.
 *
 * This function asserts that the instance pointer is not NULL and that the instance
 * is ready before writing the specified value to the logo scale factor register.
 */
void XV_mix_Set_HwReg_logoScaleFactor(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSCALEFACTOR_DATA, Data);
}

/**
 * Retrieves the current value of the logo scale factor hardware register.
 *
 * This function reads the value of the HWREG_LOGOSCALEFACTOR register from the
 * hardware instance pointed to by InstancePtr. It asserts that the instance pointer
 * is not NULL and that the instance is ready before accessing the register.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the logo scale factor hardware register.
 */
u32 XV_mix_Get_HwReg_logoScaleFactor(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOSCALEFACTOR_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the logo alpha blending in the XV_mix instance.
 *
 * This function writes the specified alpha value to the hardware register responsible
 * for controlling the logo's transparency in the video mixer. The function asserts
 * that the provided instance pointer is not NULL and that the instance is ready before
 * performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The alpha value to be written to the logo alpha hardware register.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr must be initialized and ready.
 */
void XV_mix_Set_HwReg_logoAlpha(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOALPHA_DATA, Data);
}

/**
 * Retrieves the current value of the logo alpha hardware register.
 *
 * This function reads the value of the HWREG_LOGOALPHA register from the
 * hardware associated with the given XV_mix instance. The logo alpha value
 * typically controls the transparency level of a logo overlay in the video
 * mixer hardware.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The current value of the logo alpha hardware register.
 *
 * @note     The InstancePtr must be initialized and ready before calling this function.
 */
u32 XV_mix_Get_HwReg_logoAlpha(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOALPHA_DATA);
    return Data;
}

/**
 * Sets the minimum red value for the logo color key in the hardware register.
 *
 * This function writes the specified red component value to the hardware register
 * responsible for the minimum color key value used for logo blending or transparency.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The minimum red value to be set for the logo color key.
 *
 * Preconditions:
 * - InstancePtr must not be NULL.
 * - InstancePtr must be initialized and ready.
 */
void XV_mix_Set_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_R_DATA, Data);
}

/**
 * Retrieves the current value of the HWREG_LOGOCLRKEYMIN_R hardware register.
 *
 * This function reads the value of the HWREG_LOGOCLRKEYMIN_R register from the
 * hardware associated with the given XV_mix instance. It asserts that the
 * instance pointer is not NULL and that the instance is ready before accessing
 * the register.
 *
 * @param    InstancePtr Pointer to the XV_mix instance.
 *
 * @return   The value read from the HWREG_LOGOCLRKEYMIN_R register.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_R_DATA);
    return Data;
}

/**
 * Sets the minimum green value for the logo color key in the hardware register.
 *
 * This function writes the specified value to the HWREG_LOGOCLRKEYMIN_G register,
 * which is used for color keying operations in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The minimum green value to set for the logo color key.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_G_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LOGOCLRKEYMIN_G hardware register.
 *
 * This function reads the current value of the logo color key minimum (G component)
 * register from the hardware, using the base address specified in the instance configuration.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Instance must be initialized and ready.
 *
 * @return The value read from the HWREG_LOGOCLRKEYMIN_G register.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_G_DATA);
    return Data;
}

/**
 * Sets the minimum blue value for the logo color key in the hardware register.
 *
 * This function writes the specified blue component value to the hardware register
 * responsible for the logo color key minimum threshold. It ensures that the instance
 * pointer is valid and that the hardware is ready before performing the write operation.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The blue component value to set as the minimum for the logo color key.
 *
 * @note The instance pointer must be valid and the hardware must be ready before calling this function.
 */
void XV_mix_Set_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_B_DATA, Data);
}

/**
 * Retrieves the minimum blue value for the logo color key from the hardware register.
 *
 * This function reads the value of the HWREG_LOGOCLRKEYMIN_B register, which specifies
 * the minimum blue component for the logo color key used in the video mixer hardware.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the HWREG_LOGOCLRKEYMIN_B register.
 *
 * @note     The InstancePtr must be initialized and ready before calling this function.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMIN_B_DATA);
    return Data;
}

/**
 * Sets the maximum red value for the logo color key hardware register.
 *
 * This function writes the specified value to the HWREG_LOGOCLRKEYMAX_R register,
 * which is used for color keying operations in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LOGOCLRKEYMAX_R register.
 *
 * @note The instance pointer must be valid and the hardware must be ready before calling this function.
 */
void XV_mix_Set_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_R_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LOGOCLRKEYMAX_R hardware register.
 *
 * This function reads the value from the HWREG_LOGOCLRKEYMAX_R register of the
 * XV_mix instance specified by InstancePtr. It asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the read operation.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The 32-bit value read from the HWREG_LOGOCLRKEYMAX_R register.
 *
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before performing the calculation.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_R_DATA);
    return Data;
}

/**
 * Sets the maximum value for the logo color key in the hardware register.
 *
 * This function writes the specified value to the HWREG_LOGOCLRKEYMAX_G register
 * of the XV_mix instance. It first asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the logo color key max register.
 */
void XV_mix_Set_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_G_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_LOGOCLRKEYMAX_G hardware register for the given XV_mix instance.
 *
 * This function reads the value from the HWREG_LOGOCLRKEYMAX_G register, which is used for logo color keying
 * operations in the XV_mix hardware. It asserts that the provided instance pointer is not NULL and that the
 * instance is ready before performing the register read.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The value read from the HWREG_LOGOCLRKEYMAX_G register.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_G_DATA);
    return Data;
}

/**
 * Sets the maximum blue value for the logo color key hardware register.
 *
 * This function writes the specified value to the HWREG_LOGOCLRKEYMAX_B register,
 * which is used for color keying operations in the video mixer hardware.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Data        The value to be written to the HWREG_LOGOCLRKEYMAX_B register.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_mix_Set_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_B_DATA, Data);
}

/**
 * Retrieves the maximum blue component value for the logo color key hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The value of the HWREG_LOGOCLRKEYMAX_B register.
 */
u32 XV_mix_Get_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_HWREG_LOGOCLRKEYMAX_B_DATA);
    return Data;
}

/**
 * Retrieves the base address of the logo R (red) video hardware register for the given XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that the instance is ready.
 * It then returns the calculated base address for the logo R video hardware register by adding the
 * configured base address to the register offset.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The base address of the logo R video hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
u32 XV_mix_Get_HwReg_logoR_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE);
}

/**
 * Retrieves the high address of the HWREG_LOGOR_V register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that
 * the instance is ready. It then calculates and returns the high address of the
 * HWREG_LOGOR_V register by adding the register's offset to the base address
 * of the hardware configuration.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The high address (as a 32-bit unsigned integer) of the HWREG_LOGOR_V register.
 */
u32 XV_mix_Get_HwReg_logoR_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH);
}

/**
 * Retrieves the total number of bytes for the logoR_V hardware register.
 *
 * This function calculates the total byte size of the logoR_V register by
 * subtracting the base address from the high address and adding one.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The total number of bytes occupied by the logoR_V hardware register.
 */
u32 XV_mix_Get_HwReg_logoR_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1);
}

/**
 * Retrieves the bit width of the HWREG_LOGOR_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The bit width of the HWREG_LOGOR_V register.
 */
u32 XV_mix_Get_HwReg_logoR_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOR_V;
}

/**
 * Retrieves the depth (number of elements) of the HWREG_LOGOR_V hardware register.
 *
 * This function returns the constant value representing the depth of the
 * HWREG_LOGOR_V register region for the specified XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The depth of the HWREG_LOGOR_V register region.
 */
u32 XV_mix_Get_HwReg_logoR_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOR_V;
}

/**
 * Writes an array of integer values to the logoR_V hardware register of the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWREG_LOGOR_V register.
 * @param data        Pointer to the array of integer data to write.
 * @param length      Number of words to write from the data array.
 *
 * @return The number of words written on success, or 0 if the write operation exceeds the register's address range.
 *
 * @note This function assumes that the hardware register is memory-mapped and accessible via the base address.
 *       The function checks for valid instance and ensures the write does not exceed the register's bounds.
 */
u32 XV_mix_Write_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * Reads a sequence of words from the HWREG_LOGOR_V hardware register of the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWREG_LOGOR_V register.
 * @param data        Pointer to the buffer where the read data will be stored.
 * @param length      Number of words to read.
 *
 * @return The number of words successfully read, or 0 if the requested range is out of bounds.
 *
 * @note This function assumes that the hardware register is memory-mapped and accessible via the
 *       base address in InstancePtr->Config.BaseAddress. The function also checks that the
 *       requested read does not exceed the register's address range.
 */
u32 XV_mix_Read_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * @brief Writes a sequence of bytes to the HWREG_LOGOR_V hardware register of the XV_mix instance.
 *
 * This function writes 'length' bytes from the buffer pointed to by 'data' into the hardware register
 * starting at the specified 'offset'. It ensures that the write does not exceed the register's address range.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset in bytes from the base address of the HWREG_LOGOR_V register.
 * @param data        Pointer to the data buffer to be written.
 * @param length      Number of bytes to write.
 *
 * @return Number of bytes written on success, or 0 if the write would exceed the register's address range.
 */
u32 XV_mix_Write_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a specified number of bytes from the HWREG_LOGOR_V hardware register.
 *
 * This function reads 'length' bytes from the hardware register starting at the given 'offset'
 * and stores them into the buffer pointed to by 'data'. It ensures that the read does not exceed
 * the bounds of the HWREG_LOGOR_V register space.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset from the base address of HWREG_LOGOR_V to start reading.
 * @param data        Pointer to the buffer where the read bytes will be stored.
 * @param length      Number of bytes to read.
 *
 * @return Number of bytes read on success, or 0 if the read would exceed register bounds.
 */
u32 XV_mix_Read_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOR_V_BASE + offset + i);
    }
    return length;
}

/**
 * Retrieves the base address of the logoG_V hardware register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that
 * the instance is ready. It then calculates and returns the base address of the
 * logoG_V hardware register by adding the register offset to the base address
 * from the instance configuration.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The base address of the logoG_V hardware register.
 */
u32 XV_mix_Get_HwReg_logoG_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE);
}

/**
 * Retrieves the high address of the HWREG_LOGOG_V register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that
 * the instance is ready. It then calculates and returns the high address of the
 * HWREG_LOGOG_V register by adding the base address from the configuration to
 * the register's offset.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The high address of the HWREG_LOGOG_V register.
 */
u32 XV_mix_Get_HwReg_logoG_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH);
}

/**
 * Retrieves the total number of bytes for the HWReg_logoG_V register.
 *
 * This function calculates the total byte size of the HWReg_logoG_V register
 * by subtracting the base address from the high address and adding one.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *           It must be a valid pointer and the instance must be ready.
 *
 * @return   The total number of bytes occupied by the HWReg_logoG_V register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before performing the calculation.
 */
u32 XV_mix_Get_HwReg_logoG_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1);
}

/**
 * Retrieves the bit width of the HWREG_LOGOG_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The bit width of the HWREG_LOGOG_V register as an unsigned 32-bit integer.
 */
u32 XV_mix_Get_HwReg_logoG_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOG_V;
}

/**
 * Retrieves the hardware register value for the logo G vertical depth parameter.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The value of the hardware register for logo G vertical depth.
 */
u32 XV_mix_Get_HwReg_logoG_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOG_V;
}

/**
 * @brief Writes an array of integer values to the logoG_V hardware register of the XV_mix instance.
 *
 * This function writes a specified number of 32-bit words from the provided data array
 * into the hardware register region starting at the given offset. The function ensures
 * that the write operation does not exceed the bounds of the hardware register address space.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWREG_LOGOG_V register.
 * @param data        Pointer to the array of 32-bit integers to be written.
 * @param length      Number of 32-bit words to write.
 *
 * @return Number of words written on success, or 0 if the operation would exceed register bounds.
 */
u32 XV_mix_Write_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * Reads a sequence of words from the HWREG_LOGOG_V hardware register of the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWREG_LOGOG_V register.
 * @param data        Pointer to the buffer where the read data will be stored.
 * @param length      Number of words to read.
 *
 * @return The number of words successfully read, or 0 if the requested range is out of bounds.
 *
 * @note This function assumes that the hardware register is memory-mapped and accessible via the
 *       base address in the instance configuration. It also checks that the instance pointer is valid
 *       and that the instance is ready before proceeding.
 */
u32 XV_mix_Read_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * @brief Writes a sequence of bytes to the HWREG_LOGOG_V hardware register of the XV_mix instance.
 *
 * This function copies a specified number of bytes from the provided data buffer to the hardware register
 * starting at a given offset. It ensures that the write operation does not exceed the register's address range.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset in bytes from the base address of the HWREG_LOGOG_V register where writing starts.
 * @param data        Pointer to the source data buffer to be written.
 * @param length      Number of bytes to write from the data buffer.
 *
 * @return Number of bytes written on success, or 0 if the write operation would exceed the register's bounds.
 */
u32 XV_mix_Write_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a specified number of bytes from the HWREG_LOGOG_V hardware register.
 *
 * This function reads 'length' bytes from the hardware register starting at the given 'offset'
 * and stores them into the buffer pointed to by 'data'. It ensures that the read does not exceed
 * the bounds of the register space.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset from the base address of HWREG_LOGOG_V to start reading.
 * @param data        Pointer to the buffer where the read bytes will be stored.
 * @param length      Number of bytes to read.
 *
 * @return Number of bytes read on success, or 0 if the read would exceed register bounds.
 */
u32 XV_mix_Read_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOG_V_BASE + offset + i);
    }
    return length;
}

/**
 * Retrieves the base address of the LogoB V hardware register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that the
 * instance is ready. It then returns the calculated base address for the LogoB V
 * hardware register by adding the configured base address to the register offset.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 * @return The base address of the LogoB V hardware register.
 */
u32 XV_mix_Get_HwReg_logoB_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE);
}

/**
 * Retrieves the high address of the HWREG_LOGOB_V register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that
 * the instance is ready. It then calculates and returns the high address of the
 * HWREG_LOGOB_V register by adding the register's offset to the base address of
 * the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The high address of the HWREG_LOGOB_V register.
 */
u32 XV_mix_Get_HwReg_logoB_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH);
}

/**
 * Retrieves the total number of bytes for the logoB vertical hardware register.
 *
 * This function calculates the total byte size of the logoB vertical hardware register
 * by subtracting the base address from the high address and adding one.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The total number of bytes for the logoB vertical hardware register.
 */
u32 XV_mix_Get_HwReg_logoB_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1);
}

/**
 * Retrieves the bit width of the HWREG_LOGOB_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The bit width of the HWREG_LOGOB_V register as defined by
 *         XV_MIX_CTRL_WIDTH_HWREG_LOGOB_V.
 */
u32 XV_mix_Get_HwReg_logoB_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOB_V;
}

/**
 * Retrieves the hardware register value for the vertical depth of Logo B in the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The hardware register value representing the vertical depth of Logo B.
 */
u32 XV_mix_Get_HwReg_logoB_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOB_V;
}

/**
 * @brief Writes an array of integer values to the logoB_V hardware register of the XV_mix instance.
 *
 * This function writes a specified number of 32-bit words from the provided data array
 * to the hardware register region starting at the given offset. The write operation is
 * bounded by the size of the hardware register region to prevent out-of-bounds access.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWREG_LOGOB_V register.
 * @param data        Pointer to the array of 32-bit integer data to write.
 * @param length      Number of 32-bit words to write from the data array.
 *
 * @return Number of words written on success, or 0 if the write would exceed the register region.
 */
u32 XV_mix_Write_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a sequence of words from the logoB_V hardware register of the XV_mix instance.
 *
 * This function reads 'length' 32-bit words from the hardware register region starting at
 * 'offset' (word offset) from the base address of the HWREG_LOGOB_V register block, and stores
 * them in the provided 'data' buffer.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Word offset from the base of the HWREG_LOGOB_V register block.
 * @param data        Pointer to the buffer where the read data will be stored.
 * @param length      Number of 32-bit words to read.
 *
 * @return Number of words read on success, or 0 if the requested range is out of bounds.
 *
 * @note The function asserts that InstancePtr is not NULL and that the instance is ready.
 */
u32 XV_mix_Read_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * @brief Writes a sequence of bytes to the HWREG_LOGOB_V hardware register.
 *
 * This function writes a specified number of bytes from the provided data buffer
 * to the hardware register region starting at HWREG_LOGOB_V, at the given offset.
 * It ensures that the write does not exceed the bounds of the register region.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset in bytes from the base address of HWREG_LOGOB_V where writing starts.
 * @param data        Pointer to the data buffer containing bytes to write.
 * @param length      Number of bytes to write from the data buffer.
 *
 * @return Number of bytes written on success, or 0 if the write would exceed register bounds.
 */
u32 XV_mix_Write_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a specified number of bytes from the HWREG_LOGOB_V hardware register into a buffer.
 *
 * This function reads 'length' bytes starting from 'offset' within the HWREG_LOGOB_V register
 * and stores them into the provided 'data' buffer. It ensures that the read does not exceed
 * the register's bounds.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset in bytes from the base of HWREG_LOGOB_V register to start reading.
 * @param data        Pointer to the buffer where the read bytes will be stored.
 * @param length      Number of bytes to read.
 *
 * @return Number of bytes read on success, or 0 if the read would exceed register bounds.
 */
u32 XV_mix_Read_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOB_V_BASE + offset + i);
    }
    return length;
}

/**
 * Retrieves the base address of the Logo A V hardware register for the XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that the
 * instance is ready. It then returns the calculated base address for the Logo A V
 * hardware register by adding the configured base address to the register offset.
 *
 * @param  InstancePtr  Pointer to the XV_mix instance.
 * @return The base address of the Logo A V hardware register.
 */
u32 XV_mix_Get_HwReg_logoA_V_BaseAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE);
}

/**
 * Retrieves the high address of the HWREG_LOGOA_V register for the given XV_mix instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that the instance
 * is ready for use. It then calculates and returns the high address of the HWREG_LOGOA_V
 * register by adding the base address from the instance configuration to the register offset.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The high address of the HWREG_LOGOA_V register.
 */
u32 XV_mix_Get_HwReg_logoA_V_HighAddress(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH);
}

/**
 * Retrieves the total number of bytes for the logoA vertical hardware register.
 *
 * This function calculates the total byte size of the logoA vertical hardware register
 * by subtracting the base address from the high address and adding one.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The total number of bytes for the logoA vertical hardware register.
 */
u32 XV_mix_Get_HwReg_logoA_V_TotalBytes(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1);
}

/**
 * Retrieves the bit width of the hardware register for the LogoA vertical position (V).
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @return The bit width of the HWREG_LOGOA_V register.
 *
 * This function asserts that the instance pointer is not NULL and that the
 * instance is ready before returning the bit width constant for the LogoA_V hardware register.
 */
u32 XV_mix_Get_HwReg_logoA_V_BitWidth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_WIDTH_HWREG_LOGOA_V;
}

/**
 * Retrieves the hardware register value for the vertical depth of Logo A.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The hardware register value representing the vertical depth of Logo A.
 */
u32 XV_mix_Get_HwReg_logoA_V_Depth(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_MIX_CTRL_DEPTH_HWREG_LOGOA_V;
}

/**
 * Writes a sequence of words to the HWReg_logoA_V hardware register of the XV_mix instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of the HWReg_logoA_V register.
 * @param data        Pointer to the array of data words to be written.
 * @param length      Number of words to write.
 *
 * @return The number of words written on success, or 0 if the write operation exceeds the register's address range.
 *
 * @note This function assumes that the hardware instance is ready and that the data pointer is valid.
 */
u32 XV_mix_Write_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a specified number of 32-bit words from the HWReg_logoA_V hardware register.
 *
 * This function reads 'length' 32-bit words starting from the given 'offset'
 * within the HWReg_logoA_V register space and stores them into the provided
 * 'data' buffer. It ensures that the read operation does not exceed the
 * register's address range.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset (in words) from the base address of HWReg_logoA_V to start reading.
 * @param data        Pointer to the buffer where the read words will be stored.
 * @param length      Number of 32-bit words to read.
 *
 * @return Number of words read on success, or 0 if the requested range is out of bounds.
 */
u32 XV_mix_Read_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * @brief Writes a sequence of bytes to the HWReg_logoA_V register of the XV_mix hardware.
 *
 * This function writes a specified number of bytes from the provided data buffer
 * to the HWReg_logoA_V register, starting at the given offset. It ensures that the
 * write operation does not exceed the register's address range.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset in bytes from the base address of HWReg_logoA_V where writing starts.
 * @param data        Pointer to the buffer containing the data to write.
 * @param length      Number of bytes to write.
 *
 * @return Number of bytes written on success, or 0 if the operation would exceed the register's range.
 */
u32 XV_mix_Write_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * @brief Reads a specified number of bytes from the HWREG_LOGOA_V hardware register.
 *
 * This function reads 'length' bytes from the hardware register region starting at 'offset'
 * and stores them into the buffer pointed to by 'data'. It ensures that the read operation
 * does not exceed the bounds of the hardware register region.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param offset      Offset from the base address of the HWREG_LOGOA_V register region.
 * @param data        Pointer to the buffer where the read bytes will be stored.
 * @param length      Number of bytes to read.
 *
 * @return The number of bytes read on success, or 0 if the requested range is out of bounds.
 */
u32 XV_mix_Read_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_HIGH - XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_MIX_CTRL_ADDR_HWREG_LOGOA_V_BASE + offset + i);
    }
    return length;
}

/**
 * Enables the global interrupt for the XV_mix instance.
 *
 * This function sets the Global Interrupt Enable (GIE) bit in the control register
 * of the XV_mix hardware core, allowing interrupts to be generated.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @note     The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_InterruptGlobalEnable(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_GIE, 1);
}

/**
 * Disables the global interrupt for the XV_mix hardware instance.
 *
 * This function clears the global interrupt enable bit in the Global Interrupt Enable (GIE)
 * register, effectively disabling all interrupts for the XV_mix hardware. The instance pointer
 * must be valid and the hardware must be ready before calling this function.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_InterruptGlobalDisable(XV_mix *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_GIE, 0);
}

/**
 * Enables specific interrupts for the XV_mix hardware module.
 *
 * This function sets the bits specified by the Mask parameter in the
 * Interrupt Enable Register (IER) of the XV_mix instance. It first reads
 * the current value of the IER, then sets the bits corresponding to Mask,
 * and writes the updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Mask        Bitmask of interrupts to enable.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_InterruptEnable(XV_mix *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER, Register | Mask);
}

/**
 * Disable specific interrupts for the XV_mix instance.
 *
 * This function disables the interrupts specified by the Mask parameter
 * for the given XV_mix instance. It reads the current interrupt enable
 * register, clears the bits corresponding to the Mask, and writes the
 * updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Mask        Bitmask of interrupts to disable.
 *
 * @note The XV_mix instance must be initialized and ready before calling
 *       this function.
 */
void XV_mix_InterruptDisable(XV_mix *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER, Register & (~Mask));
}

/**
 * @brief Clears the specified interrupt(s) for the XV_mix instance.
 *
 * This function writes the given interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s) for the XV_mix hardware instance.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 * @param Mask Bitmask specifying which interrupt(s) to clear.
 *
 * @note The XV_mix instance must be initialized and ready before calling this function.
 */
void XV_mix_InterruptClear(XV_mix *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_mix_WriteReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_ISR, Mask);
}

/**
 * Retrieves the interrupt enable register value for the XV_mix instance.
 *
 * This function reads and returns the current value of the Interrupt Enable Register (IER)
 * for the specified XV_mix hardware instance. It asserts that the instance pointer is not
 * NULL and that the instance is ready before accessing the register.
 *
 * @param    InstancePtr is a pointer to the XV_mix instance.
 *
 * @return   The value of the Interrupt Enable Register (IER).
 */
u32 XV_mix_InterruptGetEnabled(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_IER);
}

/**
 * Retrieves the interrupt status register value for the XV_mix instance.
 *
 * This function reads and returns the current value of the interrupt status
 * register (ISR) for the specified XV_mix hardware instance. It asserts that
 * the instance pointer is not NULL and that the instance is ready before
 * accessing the register.
 *
 * @param InstancePtr Pointer to the XV_mix instance.
 *
 * @return The value of the interrupt status register.
 */
u32 XV_mix_InterruptGetStatus(XV_mix *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_mix_ReadReg(InstancePtr->Config.BaseAddress, XV_MIX_CTRL_ADDR_ISR);
}
