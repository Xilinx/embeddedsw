// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_hcresampler.c
 * @addtogroup v_hcresampler Overview
 */

/***************************** Include Files *********************************/
#include "xv_hcresampler.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/**
 * XV_hcresampler_CfgInitialize - Initialize the XV_hcresampler instance.
 *
 * This function initializes an XV_hcresampler instance using the provided
 * configuration structure and effective base address. It sets up the instance's
 * configuration, assigns the base address, and marks the instance as ready.
 *
 * @param InstancePtr    Pointer to the XV_hcresampler instance to be initialized.
 * @param ConfigPtr      Pointer to the configuration structure containing
 *                       hardware-specific settings.
 * @param EffectiveAddr  Physical base address of the device.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - ConfigPtr must not be NULL.
 *   - EffectiveAddr must not be NULL.
 */
int XV_hcresampler_CfgInitialize(XV_hcresampler *InstancePtr,
                                 XV_hcresampler_Config *ConfigPtr,
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
 * This Function Starts the HC Resampler hardware core.
 *
 * This function initiates the operation of the HC Resampler by writing to the
 * control register. It first asserts that the provided instance pointer is valid
 * and that the core is ready. Then, it reads the current value of the control
 * register, preserves the auto-restart bit (bit 7), and sets the start bit (bit 0)
 * to begin processing.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @note The function assumes that the hardware core has been properly initialized
 *       and is ready to start.
 */

void XV_hcresampler_Start(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * Checks if the HC Resampler hardware core has completed its operation.
 *
 * This function reads the control register of the HC Resampler hardware core
 * to determine if the current operation is done. It asserts that the provided
 * instance pointer is not NULL and that the core is ready before accessing
 * the hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_hcresampler instance.
 *
 * @return   1 if the operation is done, 0 otherwise.
 */


u32 XV_hcresampler_IsDone(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}


/**
 * Checks if the HC Resampler hardware core is idle.
 *
 * This function reads the control register of the HC Resampler hardware core
 * to determine if it is idle. It asserts that the provided instance pointer
 * is not NULL and that the core is ready before accessing the hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_hcresampler instance.
 *
 * @return   1 if the core is idle, 0 otherwise.
 */
u32 XV_hcresampler_IsIdle(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}


/**
 * Checks if the HC Resampler hardware core is ready for the next input.
 *
 * This function reads the control register of the HC Resampler hardware core
 * to determine if it is ready for the next input. It asserts that the provided
 * instance pointer is not NULL and that the core is ready before accessing
 * the hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_hcresampler instance.
 *
 * @return   1 if the core is ready, 0 otherwise.
 */
u32 XV_hcresampler_IsReady(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables the auto-restart feature of the HC Resampler hardware.
 *
 * This function sets the auto-restart bit in the control register of the
 * HC Resampler, allowing the hardware to automatically restart its operation
 * after completing a task, without requiring software intervention.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @return None
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - The instance must be ready (IsReady == XIL_COMPONENT_IS_READY).
 */

void XV_hcresampler_EnableAutoRestart(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, 0x80);
}


/**
 * Disable the auto-restart feature of the HC Resampler hardware.
 *
 * This function disables the auto-restart capability of the HC Resampler
 * by writing '0' to the AP_CTRL register. After calling this function,
 * the hardware will not automatically restart processing after completing
 * a task and will require manual intervention to start again.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return None
 * @pre
 *  - InstancePtr must not be NULL.
 *  - InstancePtr->IsReady must be equal to XIL_COMPONENT_IS_READY.
 */

void XV_hcresampler_DisableAutoRestart(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_AP_CTRL, 0);
}


/**
 * Sets the hardware register width for the HC resampler instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with the HC resampler. It first asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Data        The width value to be written to the hardware register.
 *
 * @return None
 */

void XV_hcresampler_Set_HwReg_width(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_WIDTH hardware register for the specified
 * XV_hcresampler instance.
 *
 * This function reads the HWREG_WIDTH register from the hardware and returns
 * its value. It asserts that the provided instance pointer is not NULL and
 * that the instance is ready before accessing the register.
 *
 * @param  InstancePtr Pointer to the XV_hcresampler instance.
 * @return The value of the HWREG_WIDTH register.
 */

u32 XV_hcresampler_Get_HwReg_width(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}
/**
 * Sets the hardware register value for the 'height' parameter of the HC resampler instance.
 *
 * This function writes the specified height value to the corresponding hardware register
 * of the given XV_hcresampler instance. It first asserts that the instance pointer is not
 * NULL and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Data        The height value to be written to the hardware register.
 * @return None
 */

void XV_hcresampler_Set_HwReg_height(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/**
 * Retrieves the value of the HWReg_height hardware register from the HC Resampler instance.
 *
 * This function reads the current value of the HWReg_height register from the hardware
 * using the base address specified in the instance configuration.
 *
 * @param    InstancePtr is a pointer to the XV_hcresampler instance.
 *           It must be initialized and ready before calling this function.
 *
 * @return   The value of the HWReg_height register as a 32-bit unsigned integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the instance
 *           is ready before accessing the hardware register.
 */


u32 XV_hcresampler_Get_HwReg_height(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}


/**
 * Sets the input video format hardware register for the HC resampler instance.
 *
 * This function writes the specified input video format data to the hardware register
 * associated with the given HC resampler instance.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Data        Value to be written to the input video format hardware register.
 *
 * @return None.
 */

void XV_hcresampler_Set_HwReg_input_video_format(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA, Data);
}

/**
 * Retrieves the current input video format hardware register value.
 *
 * This function reads the value of the input video format register from the
 * hardware and returns it to the caller.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return The value of the input video format hardware register.
 */


u32 XV_hcresampler_Get_HwReg_input_video_format(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * Sets the output video format hardware register for the HC resampler instance.
 *
 * This function writes the specified output video format data to the hardware register
 * associated with the given HC resampler instance.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Data        Value to be written to the output video format hardware register.
 *
 * @return None.
 */
void XV_hcresampler_Set_HwReg_output_video_format(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA, Data);
}

/**
 * Retrieves the current output video format hardware register value.
 *
 * This function reads the value of the output video format register from the
 * hardware and returns it to the caller.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return The value of the output video format hardware register.
 */
u32 XV_hcresampler_Get_HwReg_output_video_format(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA);
    return Data;
}


/**
 * Sets the hardware register coefficients for the HC resampler instance.
 *
 * This function writes the specified coefficient data to the hardware register
 * associated with the given HC resampler instance. It is used to configure the
 * resampling filter coefficients.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Data        Value to be written to the coefficient hardware register.
 *
 * @return None.
 */
void XV_hcresampler_Set_HwReg_coefs_0_0(XV_hcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA, Data);
}


/**
 * Retrieves the current coefficient hardware register value for the HC resampler instance.
 *
 * This function reads the value of the coefficient register from the hardware and
 * returns it to the caller. It is used to obtain the current filter coefficients
 * configured in the HC resampler.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return The value of the coefficient hardware register.
 */
u32 XV_hcresampler_Get_HwReg_coefs_0_0(XV_hcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA);
    return Data;
}

/**
 * Enables global interrupts for the HC Resampler hardware core.
 *
 * This function sets the global interrupt enable bit in the control register
 * of the HC Resampler hardware core, allowing it to generate interrupts.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return None
 */
void XV_hcresampler_InterruptGlobalEnable(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_GIE, 1);
}

/**
 * Disables global interrupts for the HC Resampler hardware core.
 *
 * This function clears the global interrupt enable bit in the control register
 * of the HC Resampler hardware core, preventing it from generating interrupts.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return None
 */
void XV_hcresampler_InterruptGlobalDisable(XV_hcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_GIE, 0);
}

/**
 * Enables specific interrupts for the HC Resampler hardware core.
 *
 * This function sets the specified interrupt mask in the interrupt enable
 * register of the HC Resampler hardware core, allowing those interrupts to be
 * generated.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Mask        The mask of interrupts to enable.
 *
 * @return None
 */
void XV_hcresampler_InterruptEnable(XV_hcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER, Register | Mask);
}

/**
 * Disable specific interrupts for the HC Resampler core.
 *
 * This function disables the interrupts specified by the Mask parameter
 * for the given HC Resampler instance. It reads the current interrupt
 * enable register, clears the bits specified by Mask, and writes the
 * updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Mask Bitmask of interrupts to disable.
 *
 * @return None.
 */

void XV_hcresampler_InterruptDisable(XV_hcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER, Register & (~Mask));
}


/**
 * @brief Clears the specified interrupt(s) for the HC Resampler instance.
 *
 * This function writes the given interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s) for the specified HC Resampler instance.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 * @param Mask Bitmask specifying which interrupt(s) to clear.
 *
 * @return None.
 */

void XV_hcresampler_InterruptClear(XV_hcresampler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_ISR, Mask);
}
/**
 * Retrieves the interrupt enable register value for the HC Resampler instance.
 *
 * This function reads the Interrupt Enable Register (IER) of the specified
 * XV_hcresampler instance to determine which interrupts are currently enabled.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return The value of the interrupt enable register (IER).
 */

u32 XV_hcresampler_InterruptGetEnabled(XV_hcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_IER);
}


/**
 * Retrieves the current interrupt status for the HC Resampler instance.
 *
 * This function reads the Interrupt Status Register (ISR) of the specified
 * XV_hcresampler instance to determine which interrupts are currently active.
 *
 * @param InstancePtr Pointer to the XV_hcresampler instance.
 *
 * @return The value of the interrupt status register (ISR).
 */
u32 XV_hcresampler_InterruptGetStatus(XV_hcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_HCRESAMPLER_CTRL_ADDR_ISR);
}
