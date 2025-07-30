// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
*
* @file xv_frmbufrd.c
* @addtogroup v_frmbuf_rd Overview
*/
/***************************** Include Files *********************************/
#include "xv_frmbufrd.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/**
 * XV_frmbufrd_CfgInitialize - Initializes a XV_frmbufrd instance.
 *
 * @param InstancePtr   Pointer to the XV_frmbufrd instance to initialize.
 * @param ConfigPtr     Pointer to the configuration structure.
 * @param EffectiveAddr Base address of the hardware instance.
 *
 * This function sets up the driver instance with the provided configuration
 * and base address. It also marks the instance as ready for use.
 *
 * @return XST_SUCCESS on successful initialization.
 */
int XV_frmbufrd_CfgInitialize(XV_frmbufrd *InstancePtr,
                               XV_frmbufrd_Config *ConfigPtr,
                               UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

    /* Setup the instance */
    InstancePtr->Config = *ConfigPtr;
    InstancePtr->Config.BaseAddress = EffectiveAddr;
#ifdef SDT
    InstancePtr->Config.IntrId = ConfigPtr->IntrId;
    InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif

    /* Set the flag to indicate the driver is ready */
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

/**
 * Starts the Frame Buffer Read core.
 *
 * This function initiates the operation of the Frame Buffer Read (frmbufrd) hardware core.
 * It asserts that the provided instance pointer is valid and that the core is ready.
 * The function reads the current control register value, preserves the auto-restart bit,
 * and sets the start bit to begin processing.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return None.
 */
void XV_frmbufrd_Start(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * Checks if the Frame Buffer Read core has completed its operation.
 *
 * This function reads the control register to determine if the hardware core
 * has finished processing. It asserts that the instance pointer is valid and
 * the core is ready, then checks the 'done' bit in the control register.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return 1 if the operation is done, 0 otherwise.
 */
u32 XV_frmbufrd_IsDone(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the XV_frmbufrd hardware core is currently idle.
 *
 * This function reads the control register of the frame buffer read core
 * and extracts the idle status bit. It asserts that the provided instance
 * pointer is not NULL and that the instance is ready before accessing the
 * hardware.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufrd instance.
 *
 * @return   1 if the core is idle, 0 otherwise.
 */
u32 XV_frmbufrd_IsIdle(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * Checks if the Frame Buffer Read core is ready to accept new input.
 *
 * This function reads the control register and checks the 'ap_start' bit to determine
 * if the hardware core is ready for the next operation. If 'ap_start' is not set,
 * the core is ready for new input.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return 1 if the core is ready for new input, 0 otherwise.
 */
u32 XV_frmbufrd_IsReady(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    // Check ap_start to see if the core is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables auto-restart mode for the Frame Buffer Read core.
 *
 * This function sets the auto-restart bit in the control register,
 * allowing the hardware core to automatically restart its operation
 * after completing the current task. It asserts that the provided
 * instance pointer is valid and that the core is ready.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return None.
 */
void XV_frmbufrd_EnableAutoRestart(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * Disable the auto-restart feature of the Frame Buffer Read core.
 *
 * This function disables the auto-restart capability by writing 0 to the
 * AP_CTRL register of the Frame Buffer Read hardware. When auto-restart is
 * disabled, the core will not automatically restart its operation after
 * completing a frame; instead, it will require explicit re-enablement.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @note The function asserts that the instance pointer is not NULL and that
 *       the core is ready before performing the operation.
 */
void XV_frmbufrd_DisableAutoRestart(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the flush bit in the Frame Buffer Read (frmbufrd) control register.
 *
 * This function reads the current value of the control register, sets the
 * flush bit, and writes the updated value back to the register. Setting the
 * flush bit typically initiates a flush operation in the hardware, ensuring
 * that any buffered data is processed or cleared as required.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @note The function asserts that the InstancePtr is not NULL and that the
 *       instance is ready before performing the register operations.
 */
void XV_frmbufrd_SetFlushbit(XV_frmbufrd* InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress,
			       XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);
    Data |= XV_FRMBUFRD_CTRL_BITS_FLUSH_BIT;
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress,
			 XV_FRMBUFRD_CTRL_ADDR_AP_CTRL, Data);
}

/**
 * Retrieves the flush done status from the Frame Buffer Read core.
 *
 * This function reads the control register of the Frame Buffer Read (frmbufrd) instance
 * and extracts the flush status bit, indicating whether the flush operation has completed.
 *
 * @param	InstancePtr is a pointer to the XV_frmbufrd instance.
 *
 * @return	The value of the flush done status bit (non-zero if flush is done, zero otherwise).
 *
 * @note	None.
 */
u32 XV_frmbufrd_Get_FlushDone(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_AP_CTRL)
			       & XV_FRMBUFRD_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}

/**
 * Sets the hardware register value for the frame buffer reader width.
 *
 * This function writes the specified width value to the hardware register
 * associated with the frame buffer reader instance.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        The width value to be written to the hardware register.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_frmbufrd_Set_HwReg_width(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/**
 * Retrieves the hardware register value for the frame buffer reader width.
 *
 * This function reads the width value from the hardware register associated
 * with the frame buffer reader instance specified by InstancePtr.
 *
 * @param   InstancePtr is a pointer to the XV_frmbufrd instance.
 *
 * @return  The width value read from the hardware register.
 *
 * @note    The InstancePtr must be initialized and ready before calling this function.
 */
u32 XV_frmbufrd_Get_HwReg_width(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the frame buffer reader's height parameter.
 *
 * This function writes the specified height value to the HWREG_HEIGHT register
 * of the frame buffer reader hardware. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        The height value to be written to the hardware register.
 */
void XV_frmbufrd_Set_HwReg_height(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/**
 * Retrieves the value of the hardware register that holds the frame buffer height.
 *
 * This function reads the height value from the hardware register associated with
 * the frame buffer read IP core. It asserts that the provided instance pointer is
 * not NULL and that the instance is ready before accessing the register.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufrd instance.
 *
 * @return   The height value read from the hardware register.
 */
u32 XV_frmbufrd_Get_HwReg_height(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/**
 * Sets the hardware register stride value for the frame buffer reader instance.
 *
 * This function writes the specified stride value to the hardware register
 * associated with the frame buffer reader. The stride typically represents
 * the number of bytes between the start of one line of pixels and the next
 * in memory.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        The stride value to be set in the hardware register.
 *
 * @note The instance pointer must be valid and the instance must be ready
 *       before calling this function.
 */
void XV_frmbufrd_Set_HwReg_stride(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_STRIDE_DATA, Data);
}

/**
 * Retrieves the current value of the hardware register that specifies the stride
 * (number of bytes per line) for the frame buffer read instance.
 *
 * @param  InstancePtr Pointer to the XV_frmbufrd instance.
 *         - Must not be NULL.
 *         - Instance must be initialized and ready.
 *
 * @return The value of the HWREG_STRIDE register.
 *
 * @note   This function asserts if the instance pointer is NULL or the instance is not ready.
 */
u32 XV_frmbufrd_Get_HwReg_stride(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_STRIDE_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format in the Frame Buffer Read core.
 *
 * This function writes the specified video format data to the hardware register
 * responsible for configuring the video format of the Frame Buffer Read (frmbufrd) core.
 * It first asserts that the provided instance pointer is not NULL and that the
 * instance is ready before performing the register write.
 *
 * @param  InstancePtr Pointer to the XV_frmbufrd instance.
 * @param  Data        The video format data to be written to the hardware register.
 *
 * @return none
 */
void XV_frmbufrd_Set_HwReg_video_format(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

/**
 * Retrieves the current hardware register value for the video format.
 *
 * This function reads the HWREG_VIDEO_FORMAT register from the frame buffer reader
 * hardware and returns its value. It asserts that the provided instance pointer is
 * not NULL and that the instance is ready before accessing the hardware register.
 *
 * @param    InstancePtr    Pointer to the XV_frmbufrd instance.
 *
 * @return   The value of the HWREG_VIDEO_FORMAT hardware register.
 */
u32 XV_frmbufrd_Get_HwReg_video_format(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * Sets the hardware register for the frame buffer address in the XV_frmbufrd instance.
 *
 * This function writes a 64-bit frame buffer address (Data) to the hardware registers
 * associated with the frame buffer reader. The address is split into two 32-bit values
 * and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        64-bit frame buffer address to be set in the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 *
 * @return None.
 */
void XV_frmbufrd_Set_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_FRM_BUFFER_V hardware register.
 *
 * This function reads two 32-bit values from the hardware register at the specified
 * base address and register offset, combines them into a single 64-bit value, and returns it.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return The 64-bit value read from the HWREG_FRM_BUFFER_V register.
 */
u64 XV_frmbufrd_Get_HwReg_frm_buffer_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the hardware register for frame buffer 2 virtual address.
 *
 * This function writes a 64-bit address (Data) to the hardware register
 * associated with frame buffer 2. The address is split into two 32-bit
 * values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        64-bit virtual address to be set for frame buffer 2.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 *
 * @return None.
 */
void XV_frmbufrd_Set_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value from the HWREG_FRM_BUFFER2_V hardware register.
 *
 * This function reads two consecutive 32-bit registers from the hardware to
 * construct a 64-bit value representing the HWREG_FRM_BUFFER2_V register.
 * It first asserts that the provided instance pointer is valid and that the
 * hardware component is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.

 * @return The 64-bit value read from the HWREG_FRM_BUFFER2_V register.
 */
u64 XV_frmbufrd_Get_HwReg_frm_buffer2_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the hardware register for frame buffer 3 virtual address.
 *
 * This function writes a 64-bit address (Data) to the hardware register
 * associated with frame buffer 3 in the frame buffer read IP core.
 * The address is split into two 32-bit values and written to consecutive
 * register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        64-bit virtual address to be set for frame buffer 3.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_frmbufrd_Set_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA, (u32)(Data));
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value from the HWREG_FRM_BUFFER3_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided instance pointer is not NULL and that the
 * instance is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @return The 64-bit value read from the HWREG_FRM_BUFFER3_V register.
 */
u64 XV_frmbufrd_Get_HwReg_frm_buffer3_V(XV_frmbufrd *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA);
    Data += (u64)XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the HWREG_FIELD_ID register for the frame buffer reader instance.
 *
 * This function writes the specified data value to the hardware register
 * responsible for the field ID in the frame buffer reader. It asserts that
 * the instance pointer is not NULL, the instance is ready, and the
 * configuration is set to interlaced mode before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        Value to be written to the HWREG_FIELD_ID register.
 */
void XV_frmbufrd_Set_HwReg_field_id(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertVoid(InstancePtr->Config.Interlaced);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_FIELD_ID register from the frame buffer reader instance.
 *
 * This function asserts that the provided instance pointer is valid, the instance is ready,
 * and that the configuration is set to interlaced mode. It then reads and returns the value
 * of the HWREG_FIELD_ID register from the hardware.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return The value of the HWREG_FIELD_ID register.
 */
u32 XV_frmbufrd_Get_HwReg_field_id(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(InstancePtr->Config.Interlaced);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_ID_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_FIDOUTMODE hardware register for the frame buffer reader.
 *
 * This function writes the specified data value to the HWREG_FIDOUTMODE register
 * of the frame buffer reader hardware instance pointed to by InstancePtr.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Data        The value to be written to the HWREG_FIDOUTMODE register.
 *
 * @return None.
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_frmbufrd_Set_HwReg_fidOutMode(XV_frmbufrd *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIDOUTMODE_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_FIDOUTMODE hardware register for the specified
 * Frame Buffer Read (frmbufrd) instance.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *        - Must not be NULL.
 *        - The instance must be initialized and ready.
 *
 * @return
 *   The current value of the HWREG_FIDOUTMODE register.
 *
 * @note
 *   This function asserts if the input pointer is NULL or the instance is not ready.
 */
u32 XV_frmbufrd_Get_HwReg_fidOutMode(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIDOUTMODE_DATA);
    return Data;
}

/**
 * Retrieves the value of the HWREG_FID_ERROR register from the frame buffer reader instance.
 *
 * This function reads the hardware register that indicates a frame ID error from the
 * specified XV_frmbufrd instance. It asserts that the instance pointer is not NULL and
 * that the instance is ready before accessing the register.
 *
 * @param    InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return   The value of the HWREG_FID_ERROR register.
 */
u32 XV_frmbufrd_Get_HwReg_fid_error(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FID_ERROR_DATA);
    return Data;
}

/**
 * Retrieves the value of the HWREG_FIELD_OUT hardware register for the given XV_frmbufrd instance.
 *
 * This function reads the value from the HWREG_FIELD_OUT register using the base address
 * specified in the instance's configuration. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the read operation.
 *
 * @param    InstancePtr  Pointer to the XV_frmbufrd instance.
 *
 * @return   The value read from the HWREG_FIELD_OUT register.
 */
u32 XV_frmbufrd_Get_HwReg_field_out(XV_frmbufrd *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_HWREG_FIELD_OUT_DATA);
    return Data;
}

/**
 * Enables the global interrupt for the Frame Buffer Read core.
 *
 * This function sets the global interrupt enable (GIE) bit in the hardware,
 * allowing the core to generate interrupts. It asserts that the provided
 * instance pointer is valid and that the core is ready before enabling the interrupt.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return None.
 */
void XV_frmbufrd_InterruptGlobalEnable(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_GIE, 1);
}

/**
 * @brief Disables the global interrupt for the Frame Buffer Read core.
 *
 * This function disables all interrupts generated by the Frame Buffer Read
 * hardware by clearing the Global Interrupt Enable (GIE) register.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @note The instance pointer must be valid and the core must be ready before
 *       calling this function.
 * @return None.
 */
void XV_frmbufrd_InterruptGlobalDisable(XV_frmbufrd *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_GIE, 0);
}

/**
 * Enables specific interrupts for the Frame Buffer Read core.
 *
 * This function sets the bits specified by the Mask parameter in the
 * Interrupt Enable Register (IER) of the Frame Buffer Read hardware.
 * Only the interrupts corresponding to the set bits in Mask will be enabled.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Mask        Bitmask of interrupts to enable.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 * @return None.
 */
void XV_frmbufrd_InterruptEnable(XV_frmbufrd *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER, Register | Mask);
}

/**
 * Disable specific interrupts for the Frame Buffer Read (frmbufrd) instance.
 *
 * This function disables the interrupts specified by the Mask parameter for the
 * given XV_frmbufrd instance. It reads the current interrupt enable register,
 * clears the bits specified by Mask, and writes the updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Mask        Bitmask of interrupts to disable.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 * @return None.
 */
void XV_frmbufrd_InterruptDisable(XV_frmbufrd *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER, Register & (~Mask));
}

/**
 * Clears the specified interrupt(s) for the Frame Buffer Read (frmbufrd) instance.
 *
 * This function writes the provided interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s) for the given XV_frmbufrd instance.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 * @param Mask        Bitmask indicating which interrupt(s) to clear.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 * @return None.
 */
void XV_frmbufrd_InterruptClear(XV_frmbufrd *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufrd_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_ISR, Mask);
}

/**
 * Retrieves the interrupt enable register value for the Frame Buffer Read (frmbufrd) instance.
 *
 * This function checks that the provided instance pointer is valid and that the instance
 * is ready. It then reads and returns the value of the Interrupt Enable Register (IER)
 * from the hardware.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return
 *   The current value of the Interrupt Enable Register (IER).
 */
u32 XV_frmbufrd_InterruptGetEnabled(XV_frmbufrd *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_IER);
}

/**
 * Retrieves the interrupt status register value for the Frame Buffer Read (frmbufrd) instance.
 *
 * This function asserts that the provided instance pointer is valid and that the instance
 * is ready. It then reads and returns the value of the Interrupt Status Register (ISR)
 * from the hardware, indicating which interrupts are currently active.
 *
 * @param InstancePtr Pointer to the XV_frmbufrd instance.
 *
 * @return
 *   The current value of the Interrupt Status Register (ISR).
 */
u32 XV_frmbufrd_InterruptGetStatus(XV_frmbufrd *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufrd_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFRD_CTRL_ADDR_ISR);
}
