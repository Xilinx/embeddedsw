// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * *
* @file xv_frmbufwr.c
* @addtogroup v_frmbuf_wr Overview
*
**/
/***************************** Include Files *********************************/
#include "xv_frmbufwr.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/**
 * Initialize the XV_frmbufwr instance.
 *
 * @param InstancePtr   Pointer to the XV_frmbufwr instance to be initialized.
 * @param ConfigPtr     Pointer to the configuration structure.
 * @param EffectiveAddr Physical base address of the device.
 *
 * This function initializes an XV_frmbufwr instance using the provided
 * configuration structure and base address. It sets up the instance
 * configuration, updates the base address, and marks the instance as ready.
 *
 * @return: XST_SUCCESS on successful initialization.
 */
int XV_frmbufwr_CfgInitialize(XV_frmbufwr *InstancePtr,
                               XV_frmbufwr_Config *ConfigPtr,
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
 * Starts the Frame Buffer Write (frmbufwr) hardware core.
 *
 * This function asserts the start signal of the frmbufwr core by writing to the
 * control register. It preserves the auto-restart bit if it is set.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   None.
 *
 * @note     The hardware core must be properly initialized and ready before
 *           calling this function.
 */
void XV_frmbufwr_Start(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * Checks if the Frame Buffer Write (frmbufwr) hardware core has completed its current operation.
 *
 * This function reads the status register of the frmbufwr core and returns the "done" status bit.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   1 if the operation is done, 0 otherwise.
 *
 * @note     The function asserts that the InstancePtr is not NULL and that the core is ready.
 */
u32 XV_frmbufwr_IsDone(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the Frame Buffer Write (frmbufwr) core is idle.
 *
 * This function reads the control register of the Frame Buffer Write core
 * and returns the status of the idle bit. The core is considered idle if
 * the bit is set.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   1 if the core is idle, 0 otherwise.
 *
 * @note     None.
 */
u32 XV_frmbufwr_IsIdle(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * Checks if the XV_frmbufwr instance is ready to accept new input.
 *
 * This function reads the control register of the frame buffer write core to determine
 * if it is ready for the next operation. It asserts that the provided instance pointer
 * is not NULL and that the instance is marked as ready. The function returns a non-zero
 * value if the core is ready for the next input, and zero otherwise.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   1 if the core is ready for the next input, 0 otherwise.
 */
u32 XV_frmbufwr_IsReady(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables auto-restart mode for the Frame Buffer Write (frmbufwr) hardware core.
 *
 * This function sets the auto-restart bit in the control register, allowing
 * the core to automatically restart its operation after completion without
 * requiring software intervention.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   None.
 *
 * @note     The hardware core must be properly initialized and ready before
 *           calling this function.
 */
void XV_frmbufwr_EnableAutoRestart(XV_frmbufwr *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * Disables the auto-restart feature of the frame buffer write core.
 *
 * @param InstancePtr: Pointer to the XV_frmbufwr instance.
 *
 * This function disables the auto-restart functionality by writing 0 to the
 * AP_CTRL register of the frame buffer write hardware. It asserts that the
 * instance pointer is not NULL and that the core is ready before performing
 * the operation.
 *
 * @return   None.
 */
void XV_frmbufwr_DisableAutoRestart(XV_frmbufwr *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the flush bit in the Frame Buffer Write (frmbufwr) control register.
 *
 * This function sets the flush bit in the AP_CTRL register to trigger a flush
 * operation in the hardware. It reads the current value of the control register,
 * sets the flush bit, and writes the updated value back.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   None.
 *
 * @note     The hardware core must be properly initialized and ready before
 *           calling this function.
 */
void XV_frmbufwr_SetFlushbit(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress,
                   XV_FRMBUFWR_CTRL_ADDR_AP_CTRL);
    Data |= XV_FRMBUFWR_CTRL_BITS_FLUSH_BIT;
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress,
             XV_FRMBUFWR_CTRL_ADDR_AP_CTRL, Data);
}

/**
 * Retrieves the flush done status from the Frame Buffer Write core.
 *
 * This function reads the status register of the Frame Buffer Write hardware
 * instance to determine if the flush operation has completed. It asserts that
 * the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The value of the flush done status bit (non-zero if flush is done, zero otherwise).
 */
u32 XV_frmbufwr_Get_FlushDone(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_AP_CTRL)
			       & XV_FRMBUFWR_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}

/**
 * Sets the hardware register value for the frame buffer writer's width parameter.
 *
 * This function writes the specified width value to the hardware register
 * associated with the frame buffer writer instance. It first asserts that the
 * provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        The width value to be written to the hardware register.
 *
 * @return none.
 */
void XV_frmbufwr_Set_HwReg_width(XV_frmbufwr *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/**
 * Retrieves the value of the hardware register that specifies the width
 * configuration for the frame buffer write instance.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The width value read from the hardware register.
 *
 * @note     The function asserts that the instance pointer is not NULL and
 *           that the instance is ready before accessing the hardware register.
 */
u32 XV_frmbufwr_Get_HwReg_width(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * Sets the height hardware register for the frame buffer write instance.
 *
 * This function writes the specified height value to the hardware register
 * responsible for configuring the frame buffer's height. It first asserts
 * that the provided instance pointer is not NULL and that the instance is
 * ready for operation.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        The height value to be written to the hardware register.
 *
 * @return none.
 */
void XV_frmbufwr_Set_HwReg_height(XV_frmbufwr *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_HEIGHT hardware register for the specified
 * Frame Buffer Write (frmbufwr) instance.
 *
 * @param  InstancePtr  Pointer to the XV_frmbufwr instance.
 * @return The current value of the HWREG_HEIGHT register.
 *
 * @note   The InstancePtr must be initialized and ready before calling this function.
 */
u32 XV_frmbufwr_Get_HwReg_height(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/**
 * Sets the hardware register stride value for the frame buffer write instance.
 *
 * This function writes the specified stride value to the hardware register
 * associated with the frame buffer writer. The stride typically represents
 * the number of bytes between the start of one line of pixels and the next
 * in memory.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        The stride value to be written to the hardware register.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_frmbufwr_Set_HwReg_stride(XV_frmbufwr *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_STRIDE_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_STRIDE register from the frame buffer write hardware.
 *
 * This function reads the stride value (in bytes) from the hardware register,
 * which typically represents the number of bytes between the start of one line
 * of pixels and the start of the next line in memory.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *           It must be a valid pointer to an initialized XV_frmbufwr structure.
 *
 * @return   The current value of the HWREG_STRIDE register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_frmbufwr_Get_HwReg_stride(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_STRIDE_DATA);
    return Data;
}

/**
 * Sets the hardware register for the video format in the frame buffer write instance.
 *
 * This function writes the specified video format data to the hardware register
 * associated with the frame buffer writer. It first asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 *
 * @param  InstancePtr Pointer to the XV_frmbufwr instance.
 * @param  Data        The video format data to be written to the hardware register.
 *
 * @return none
 */
void XV_frmbufwr_Set_HwReg_video_format(XV_frmbufwr *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}

/**
 * Retrieves the current hardware register value for the video format.
 *
 * This function reads the value of the HWREG_VIDEO_FORMAT register from the
 * frame buffer write hardware instance specified by InstancePtr.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The value of the HWREG_VIDEO_FORMAT register.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_frmbufwr_Get_HwReg_video_format(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * Sets the hardware register for the frame buffer virtual address.
 *
 * This function writes a 64-bit address (Data) to the hardware register
 * associated with the frame buffer. The address is split into two 32-bit
 * values and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        64-bit virtual address to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_frmbufwr_Set_HwReg_frm_buffer_V(XV_frmbufwr *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA, (u32)(Data));
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the value of the HWREG_FRM_BUFFER_V hardware register.
 *
 * This function reads a 64-bit value from the hardware register associated with
 * frame buffer V. It reads two 32-bit values from consecutive register addresses,
 * combines them into a single 64-bit value, and returns the result.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The 64-bit value read from the HWREG_FRM_BUFFER_V register.
 */
u64 XV_frmbufwr_Get_HwReg_frm_buffer_V(XV_frmbufwr *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA);
    Data += (u64)XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the value of the HWREG_FRM_BUFFER2_V hardware register for the frame buffer writer.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * frame buffer 2. The value is split into two 32-bit parts and written to consecutive
 * register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 *
 * @return  none.
 */
void XV_frmbufwr_Set_HwReg_frm_buffer2_V(XV_frmbufwr *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA, (u32)(Data));
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_FRM_BUFFER2_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware, combines them into a 64-bit value,
 * and returns the result. It asserts that the provided instance pointer is not NULL and that the
 * instance is ready before performing the read operations.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @return The 64-bit value read from the HWREG_FRM_BUFFER2_V register.
 */
u64 XV_frmbufwr_Get_HwReg_frm_buffer2_V(XV_frmbufwr *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA);
    Data += (u64)XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER2_V_DATA + 4) << 32;
    return Data;
}

/**
 * Sets the hardware register for frame buffer 3 (vertical) in the XV_frmbufwr instance.
 *
 * This function writes a 64-bit value to the hardware register associated with
 * frame buffer 3's vertical parameter. The value is split into two 32-bit parts
 * and written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Data        64-bit value to be written to the hardware register.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 * @return  none.
 */
void XV_frmbufwr_Set_HwReg_frm_buffer3_V(XV_frmbufwr *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA, (u32)(Data));
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the 64-bit value of the HWREG_FRM_BUFFER3_V hardware register.
 *
 * This function reads two 32-bit registers from the hardware to construct
 * a 64-bit value representing the contents of the HWREG_FRM_BUFFER3_V register.
 * It asserts that the provided instance pointer is not NULL and that the
 * instance is ready before performing the read operations.
 *
 * @param  InstancePtr Pointer to the XV_frmbufwr instance.
 * @return 64-bit value read from the HWREG_FRM_BUFFER3_V register.
 */
u64 XV_frmbufwr_Get_HwReg_frm_buffer3_V(XV_frmbufwr *InstancePtr) {
    u64 Data;
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA);
    Data += (u64)XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FRM_BUFFER3_V_DATA + 4) << 32;
    return Data;
}

/**
 * Retrieves the value of the HWREG_FIELD_ID register from the frame buffer write hardware.
 *
 * This function reads the HWREG_FIELD_ID register from the hardware instance specified by
 * InstancePtr. It asserts that the instance pointer is valid, the instance is ready, and
 * the configuration is set to interlaced mode before performing the register read.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The value read from the HWREG_FIELD_ID register.
 *
 * @note     The function asserts that the instance is configured for interlaced mode.
 */
u32 XV_frmbufwr_Get_HwReg_field_id(XV_frmbufwr *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
    Xil_AssertNonvoid(InstancePtr->Config.Interlaced);

    Data = XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_HWREG_FIELD_ID_DATA);
    return Data;
}

/**
 * Enables the global interrupt for the Frame Buffer Write (frmbufwr) core.
 *
 * This function sets the Global Interrupt Enable (GIE) bit in the control register,
 * allowing the core to generate interrupts. It asserts that the provided instance
 * pointer is not NULL and that the core is ready before enabling the interrupt.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @return none.
 */
void XV_frmbufwr_InterruptGlobalEnable(XV_frmbufwr *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_GIE, 1);
}

/**
 * XV_frmbufwr_InterruptGlobalDisable - Disables the global interrupt for the frame buffer write core.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 *
 * This function disables the global interrupt enable (GIE) bit in the control register
 * of the frame buffer write hardware. It asserts that the provided instance pointer is
 * not NULL and that the instance is ready before performing the operation.
 *
 * @return none.
 */
void XV_frmbufwr_InterruptGlobalDisable(XV_frmbufwr *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_GIE, 0);
}

/**
 * Enables specific interrupts for the Frame Buffer Write core.
 *
 * This function sets the bits specified by the Mask parameter in the Interrupt Enable Register (IER)
 * of the Frame Buffer Write hardware instance. Only the interrupts corresponding to the set bits in
 * the Mask will be enabled.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Mask        Bitmask of interrupts to enable. Each bit corresponds to a specific interrupt.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 * @return none.
 */
void XV_frmbufwr_InterruptEnable(XV_frmbufwr *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_IER);
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_IER, Register | Mask);
}

/**
 * Disable specific interrupts for the Frame Buffer Write core.
 *
 * This function disables the interrupts specified by the Mask parameter
 * in the Interrupt Enable Register (IER) of the Frame Buffer Write core.
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Mask        Bitmask of interrupts to disable. Each bit corresponds
 *                    to a specific interrupt source.
 *
 * @note The function asserts that the InstancePtr is not NULL and that the
 *       core is ready before proceeding.
 * @return none.
 */
void XV_frmbufwr_InterruptDisable(XV_frmbufwr *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_IER);
    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_IER, Register & (~Mask));
}

/**
 * Clears specific interrupt(s) for the Frame Buffer Write (frmbufwr) instance.
 *
 * This function writes the specified interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s).
 *
 * @param InstancePtr Pointer to the XV_frmbufwr instance.
 * @param Mask        Bitmask indicating which interrupt(s) to clear.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */
void XV_frmbufwr_InterruptClear(XV_frmbufwr *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_frmbufwr_WriteReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_ISR, Mask);
}

/**
 * Retrieves the interrupt enable register value for the XV_frmbufwr instance.
 *
 * This function checks that the provided instance pointer is valid and that the
 * instance is ready. It then reads and returns the value of the Interrupt Enable
 * Register (IER) for the frame buffer write hardware.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The current value of the Interrupt Enable Register (IER).
 */
u32 XV_frmbufwr_InterruptGetEnabled(XV_frmbufwr *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_IER);
}

/**
 * Retrieves the interrupt status register value for the XV_frmbufwr instance.
 *
 * This function checks that the provided instance pointer is valid and that the
 * instance is ready. It then reads and returns the value of the Interrupt Status
 * Register (ISR) for the frame buffer write hardware.
 *
 * @param    InstancePtr is a pointer to the XV_frmbufwr instance.
 *
 * @return   The current value of the Interrupt Status Register (ISR).
 */
u32 XV_frmbufwr_InterruptGetStatus(XV_frmbufwr *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_frmbufwr_ReadReg(InstancePtr->Config.BaseAddress, XV_FRMBUFWR_CTRL_ADDR_ISR);
}
