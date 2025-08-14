// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_deinterlacer.c
 * @addtogroup v_deinterlacer Overview
 * **/

/***************************** Include Files *********************************/
#include "xv_deinterlacer.h"

/************************** Function Implementation *************************/
#ifndef __linux__


/**
 * Initializes a specific XV_deinterlacer instance with the provided configuration.
 *
 * This function sets up the deinterlacer instance pointed to by InstancePtr
 * using the configuration data in ConfigPtr and the specified base address.
 * It performs basic parameter validation using assertions.
 *
 * @param InstancePtr    Pointer to the XV_deinterlacer instance to initialize.
 * @param ConfigPtr      Pointer to the configuration structure for the device.
 * @param EffectiveAddr  Base address for the device instance.
 *
 * @return Status of the initialization operation.
 *         Typically returns XST_SUCCESS if successful, or an error code otherwise.
 */
int XV_deinterlacer_CfgInitialize(XV_deinterlacer *InstancePtr,
                                  XV_deinterlacer_Config *ConfigPtr,
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
 * This function Starts the XV_deinterlacer hardware core.
 *
 * This function initiates the operation of the XV_deinterlacer hardware by setting
 * the start bit in the control register. It first asserts that the provided instance
 * pointer is not NULL and that the hardware is ready. The function preserves the
 * auto-restart bit (bit 7) in the control register and sets the start bit (bit 0)
 * to begin processing.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return None.
 */
void XV_deinterlacer_Start(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}



/**
 * Checks if the deinterlacer hardware core has completed its current operation.
 *
 * This function reads the status register of the deinterlacer hardware and returns
 * whether the operation is done. It asserts that the provided instance pointer is
 * not NULL and that the instance is ready before accessing the hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_deinterlacer instance.
 *
 * @return   1 if the operation is done, 0 otherwise.
 *
 * @note     None.
 */
u32 XV_deinterlacer_IsDone(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the XV_deinterlacer hardware core is idle.
 *
 * This function reads the control register of the deinterlacer hardware core
 * to determine if it is currently idle. It asserts that the provided instance
 * pointer is not NULL and that the instance is ready before accessing the hardware.
 *
 * @param    InstancePtr is a pointer to the XV_deinterlacer instance.
 *
 * @return   1 if the core is idle, 0 otherwise.
 */
u32 XV_deinterlacer_IsIdle(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * Checks if the XV_deinterlacer hardware core is ready to accept new input.
 *
 * This function reads the control register of the deinterlacer hardware to determine
 * if it is ready for the next operation. It asserts that the provided instance pointer
 * is not NULL and that the instance is marked as ready. The function returns a non-zero
 * value if the core is ready, and zero otherwise.
 *
 * @param    InstancePtr is a pointer to the XV_deinterlacer instance.
 *
 * @return   1 if the core is ready for the next input, 0 otherwise.
 */
u32 XV_deinterlacer_IsReady(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables the auto-restart feature of the XV_deinterlacer hardware.
 *
 * This function sets the appropriate control register bit to allow the
 * deinterlacer hardware to automatically restart its operation after
 * completing a frame, without requiring software intervention.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @pre
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be equal to XIL_COMPONENT_IS_READY.
 *
 * @note
 *   This function does not check if the hardware is currently running.
 */
void XV_deinterlacer_EnableAutoRestart(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * Disable the auto-restart feature of the deinterlacer hardware.
 *
 * This function disables the auto-restart capability of the XV_deinterlacer
 * instance specified by InstancePtr. When auto-restart is disabled, the
 * hardware will not automatically restart processing after completing a frame.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @note The InstancePtr must be valid and the hardware must be ready before
 *       calling this function.
 */
void XV_deinterlacer_DisableAutoRestart(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the width parameter for the XV_deinterlacer instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with the deinterlacer core. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        The width value to be set in the hardware register.
 */
void XV_deinterlacer_Set_width(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WIDTH_DATA, Data);
}

/**
 * Retrieves the current width configuration from the XV_deinterlacer hardware instance.
 *
 * This function reads the width value from the hardware register associated with the
 * deinterlacer instance. It asserts that the provided instance pointer is not NULL and
 * that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return The width value currently configured in the hardware.
 */
u32 XV_deinterlacer_Get_width(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

/**
 * Sets the height parameter for the XV_deinterlacer instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with the deinterlacer core. It first checks that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        The height value to be set in the hardware register.
 */
void XV_deinterlacer_Set_height(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_HEIGHT_DATA, Data);
}

/**
 * Retrieves the current height configuration value from the XV_deinterlacer hardware instance.
 *
 * This function reads the HEIGHT register from the deinterlacer hardware and returns its value.
 * It asserts that the provided instance pointer is not NULL and that the hardware is ready.
 *
 * @param    InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return   The current height value configured in the deinterlacer hardware.
 */
u32 XV_deinterlacer_Get_height(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}


/**
 * Sets the read frame buffer address for the XV_deinterlacer instance.
 *
 * This function writes a 64-bit address to the hardware registers associated
 * with the read frame buffer. The address is split into two 32-bit values and
 * written to consecutive register addresses.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        64-bit address of the read frame buffer.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 */
void XV_deinterlacer_Set_read_fb(XV_deinterlacer *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA, (u32)(Data));
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the current value of the read frame buffer address from the deinterlacer hardware.
 *
 * This function reads a 64-bit address from two consecutive 32-bit hardware registers,
 * combines them, and returns the resulting address. It asserts that the provided
 * instance pointer is not NULL and that the instance is ready before accessing the hardware.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @return 64-bit value representing the read frame buffer address.
 */
u64 XV_deinterlacer_Get_read_fb(XV_deinterlacer *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA);
    Data |= (u64)(XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_READ_FB_V_DATA + 4)) << 32;
    return Data;
}

/**
 * Sets the read frame buffer address for the XV_deinterlacer instance.
 *
 * This function writes a 64-bit address to the hardware registers associated
 * with the read frame buffer. The address is split into two 32-bit values and
 * written to consecutive registers.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        64-bit address of the read frame buffer.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 * @return None
 */
void XV_deinterlacer_Set_write_fb(XV_deinterlacer *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA, (u32)Data);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA + 4, (u32)(Data >> 32));
}

/**
 * Retrieves the current value of the write frame buffer address from the deinterlacer hardware.
 *
 * This function reads a 64-bit address from two consecutive 32-bit hardware registers
 * associated with the write frame buffer. It first asserts that the provided instance
 * pointer is not NULL and that the instance is ready for use.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @return The 64-bit write frame buffer address.
 */
u64 XV_deinterlacer_Get_write_fb(XV_deinterlacer *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA);
    Data |= (u64)(XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_WRITE_FB_V_DATA + 4)) << 32;
    return Data;
}

/**
 * Sets the color format for the XV_deinterlacer instance.
 *
 * This function writes the specified color format value to the hardware register
 * associated with the deinterlacer core. It first asserts that the provided
 * instance pointer is not NULL and that the instance is ready for operation.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        The color format value to be set.
 *
 * @return none.
 */
void XV_deinterlacer_Set_colorFormat(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

/**
 * Retrieves the current color format setting from the deinterlacer hardware.
 *
 * This function reads the color format register from the deinterlacer instance
 * and returns its value. It asserts that the provided instance pointer is not
 * NULL and that the instance is ready before accessing the hardware register.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @return The value of the color format register.
 */
u32 XV_deinterlacer_Get_colorFormat(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_COLORFORMAT_DATA);
    return Data;
}

/**
 * Sets the deinterlacing algorithm for the XV_deinterlacer instance.
 *
 * This function writes the specified algorithm data to the hardware register
 * controlling the deinterlacing algorithm. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the write.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        The algorithm selection data to be written to the hardware.
 * @return None.
 */
void XV_deinterlacer_Set_algo(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ALGO_DATA, Data);
}

/**
 * Retrieves the current deinterlacing algorithm setting from the hardware.
 *
 * This function reads the value of the algorithm register from the deinterlacer
 * hardware instance specified by InstancePtr. It asserts that the instance pointer
 * is not NULL and that the instance is ready before accessing the register.
 *
 * @param    InstancePtr is a pointer to the XV_deinterlacer instance.
 *
 * @return   The value of the algorithm register (u32).
 *
 * @note     None.
 */
u32 XV_deinterlacer_Get_algo(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ALGO_DATA);
    return Data;
}



/**
 * Sets the invert field ID parameter for the XV_deinterlacer instance.
 *
 * This function writes the specified value to the INVERT_FIELD_ID register
 * of the deinterlacer hardware. It first asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Data        Value to be written to the INVERT_FIELD_ID register.
 *
 * @return None
 */
void XV_deinterlacer_Set_invert_field_id(XV_deinterlacer *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_INVERT_FIELD_ID_DATA, Data);
}

/**
 * Retrieves the value of the INVERT_FIELD_ID register from the deinterlacer hardware.
 *
 * This function reads the INVERT_FIELD_ID register from the hardware instance specified
 * by the given XV_deinterlacer pointer. It asserts that the instance pointer is not NULL
 * and that the hardware component is ready before performing the read operation.
 *
 * @param    InstancePtr   Pointer to the XV_deinterlacer instance.
 *
 * @return   The value of the INVERT_FIELD_ID register.
 */
u32 XV_deinterlacer_Get_invert_field_id(XV_deinterlacer *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_INVERT_FIELD_ID_DATA);
    return Data;
}


/**
 * Enables the global interrupt for the XV_deinterlacer hardware instance.
 *
 * This function sets the Global Interrupt Enable (GIE) bit in the control register
 * of the deinterlacer hardware, allowing interrupts to be generated.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The instance must be initialized and ready (IsReady == XIL_COMPONENT_IS_READY).
 * @return None.
 */
void XV_deinterlacer_InterruptGlobalEnable(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_GIE, 1);
}


/**
 * This Function Disables the global interrupt for the XV_deinterlacer core.
 *
 * This function disables all interrupts by clearing the Global Interrupt Enable (GIE)
 * register of the XV_deinterlacer hardware instance.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return None.
 */
void XV_deinterlacer_InterruptGlobalDisable(XV_deinterlacer *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_GIE, 0);
}

/**
 * Enables specific interrupts for the XV_deinterlacer hardware instance.
 *
 * This function sets the bits specified by the Mask parameter in the Interrupt Enable Register (IER)
 * of the deinterlacer hardware, allowing the corresponding interrupts to be generated.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Mask Bitmask specifying which interrupts to enable.
 *
 * @return None.
 */
void XV_deinterlacer_InterruptEnable(XV_deinterlacer *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER, Register | Mask);
}

/**
 * Disable specific interrupts for the XV_deinterlacer instance.
 *
 * This function disables the interrupts specified by the Mask parameter
 * for the given XV_deinterlacer hardware instance. It reads the current
 * interrupt enable register, clears the bits specified by Mask, and writes
 * the updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Mask Bitmask of interrupts to disable.
 *
 * @return None.
 */
void XV_deinterlacer_InterruptDisable(XV_deinterlacer *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER, Register & (~Mask));
}


/**
 * This Function Clears the specified interrupt(s) for the XV_deinterlacer instance.
 *
 * This function writes the provided interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s) for the deinterlacer hardware.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 * @param Mask Bitmask specifying which interrupt(s) to clear.
 *
 * @return None.
 */
void XV_deinterlacer_InterruptClear(XV_deinterlacer *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_deinterlacer_WriteReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ISR, Mask);
}

/**
 * Retrieves the interrupt enable register value for the specified deinterlacer instance.
 *
 * This function checks that the provided instance pointer is valid and that the
 * deinterlacer component is ready. It then reads and returns the value of the
 * interrupt enable register.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return The value of the interrupt enable register.
 */
u32 XV_deinterlacer_InterruptGetEnabled(XV_deinterlacer *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_IER);
}

/**
 * Retrieves the interrupt status register value for the XV_deinterlacer instance.
 *
 * This function reads the interrupt status register (ISR) of the deinterlacer hardware
 * and returns its current value. It asserts that the provided instance pointer is not NULL
 * and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_deinterlacer instance.
 *
 * @return The current value of the interrupt status register.
 */
u32 XV_deinterlacer_InterruptGetStatus(XV_deinterlacer *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_deinterlacer_ReadReg(InstancePtr->Config.BaseAddress, XV_DEINTERLACER_CTRL_ADDR_ISR);
}
