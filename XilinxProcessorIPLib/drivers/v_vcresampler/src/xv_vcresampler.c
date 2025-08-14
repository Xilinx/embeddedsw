// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_vcresampler.c
 * @addtogroup v_vcresampler Overview
 */

/***************************** Include Files *********************************/
#include "xv_vcresampler.h"

/************************** Function Implementation *************************/
#ifndef __linux__


/**
 * Initializes a specific XV_vcresampler instance with the provided configuration and base address.
 *
 * @param InstancePtr    Pointer to the XV_vcresampler instance to be initialized.
 * @param ConfigPtr      Pointer to the configuration structure for the XV_vcresampler.
 * @param EffectiveAddr  Base address for the hardware instance.
 *
 * @return XST_SUCCESS if initialization is successful.
 */
int XV_vcresampler_CfgInitialize(XV_vcresampler *InstancePtr,
                                 XV_vcresampler_Config *ConfigPtr,
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
 * Starts the VC resampler hardware.
 *
 * This function asserts the validity of the instance pointer and its readiness,
 * then writes to the control register to start the hardware operation.
 *
 * @param    InstancePtr is a pointer to the XV_vcresampler instance.
 *
 * @return   None.
 */

void XV_vcresampler_Start(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}


/**
 * Checks if the VC Resampler core has completed its operation.
 *
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 *
 * @return
 *   - 1 if the operation is done.
 *   - 0 if the operation is not done.
 */

u32 XV_vcresampler_IsDone(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the XV_vcresampler core is idle.
 *
 * @param    InstancePtr is a pointer to the XV_vcresampler instance.
 *
 * @return   Returns 1 if the core is idle, 0 otherwise.
 */


u32 XV_vcresampler_IsIdle(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}


/**
 * Checks if the XV_vcresampler instance is ready for the next input.
 *
 * @param    InstancePtr is a pointer to the XV_vcresampler instance.
 *
 * @return   1 if the core is ready for the next input, 0 otherwise.
 */

u32 XV_vcresampler_IsReady(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * This function enables auto-restart for the XV_vcresampler instance.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return None.
 */
void XV_vcresampler_EnableAutoRestart(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * This function disables auto-restart for the XV_vcresampler instance.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return None.
 */
void XV_vcresampler_DisableAutoRestart(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * This function sets the hardware register width.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Data Width data to set.
 * @return None.
 */
void XV_vcresampler_Set_HwReg_width(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/**
 * This function gets the hardware register width.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return Width data.
 */
u32 XV_vcresampler_Get_HwReg_width(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VCRESAMPLER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * This function sets the hardware register height.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Data Height data to set.
 * @return None.
 */
void XV_vcresampler_Set_HwReg_height(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/**
 * This function gets the hardware register height.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return Height data.
 */
u32 XV_vcresampler_Get_HwReg_height(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VCRESAMPLER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/**
 * This function sets the input video format.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Data Input video format data to set.
 * @return None.
 */
void XV_vcresampler_Set_HwReg_input_video_format(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA, Data);
}

/**
 * This function gets the input video format.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return Input video format data.
 */
u32 XV_vcresampler_Get_HwReg_input_video_format(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VCRESAMPLER_CTRL_ADDR_HWREG_INPUT_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * This function sets the output video format.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Data Output video format data to set.
 * @return None.
 */
void XV_vcresampler_Set_HwReg_output_video_format(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA, Data);
}

/**
 * This function gets the output video format.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return Output video format data.
 */
u32 XV_vcresampler_Get_HwReg_output_video_format(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VCRESAMPLER_CTRL_ADDR_HWREG_OUTPUT_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * This function sets the coefficients for the resampler.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Data Coefficients data to set.
 * @return None.
 */
void XV_vcresampler_Set_HwReg_coefs_0_0(XV_vcresampler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA, Data);
}

/**
 * This function gets the coefficients for the resampler.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return Coefficients data.
 */
u32 XV_vcresampler_Get_HwReg_coefs_0_0(XV_vcresampler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VCRESAMPLER_CTRL_ADDR_HWREG_COEFS_0_0_DATA);
    return Data;
}

/**
 * This function enables global interrupts.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return None.
 */
void XV_vcresampler_InterruptGlobalEnable(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_GIE, 1);
}

/**
 * This function disables global interrupts.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @return None.
 */
void XV_vcresampler_InterruptGlobalDisable(XV_vcresampler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_GIE, 0);
}

/**
 * This function enables specific interrupts.
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Mask Interrupt mask to enable.
 * @return None.
 */
void XV_vcresampler_InterruptEnable(XV_vcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register = XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress,
                                      XV_VCRESAMPLER_CTRL_ADDR_IER);
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress,
                            XV_VCRESAMPLER_CTRL_ADDR_IER, Register | Mask);
}



/**
 * Disable specific interrupts for the VC Resampler core.
 *
 * This function disables the interrupts specified by the Mask parameter
 * for the given VC Resampler instance. It reads the current interrupt enable
 * register, clears the bits specified by Mask, and writes the updated value
 * back to the register.
 *
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Mask        Bitmask of interrupts to disable.
 *
 * @return None.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */


void XV_vcresampler_InterruptDisable(XV_vcresampler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER);
    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER, Register & (~Mask));
}

/**
 * Clears specific interrupt(s) for the VC resampler hardware.
 *
 * This function writes the specified interrupt mask to the Interrupt Status Register (ISR)
 * to clear the corresponding interrupt(s) for the VC resampler instance.
 *
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 * @param Mask        Bitmask indicating which interrupt(s) to clear.
 *
 * @note The InstancePtr must be initialized and ready before calling this function.
 */

void XV_vcresampler_InterruptClear(XV_vcresampler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vcresampler_WriteReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_ISR, Mask);
}

/**
 * Retrieves the interrupt enable register value for the VC resampler instance.
 *
 * This function checks that the provided instance pointer is valid and that the
 * instance is ready. It then reads and returns the value of the interrupt enable
 * register (IER) from the hardware.
 *
 * @param InstancePtr Pointer to the XV_vcresampler instance.
 *
 * @return
 *   The current value of the interrupt enable register.
 */

u32 XV_vcresampler_InterruptGetEnabled(XV_vcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_IER);
}

/**
 * XV_vcresampler_InterruptGetStatus - Retrieves the interrupt status of the VC resampler instance.
 *
 * @param InstancePtr: Pointer to the XV_vcresampler instance.
 *                     Must not be NULL and must be initialized.
 *
 * @return
 *   The current value of the Interrupt Status Register (ISR) for the VC resampler.
 *
 * This function asserts the validity of the instance pointer and its readiness,
 * then reads and returns the interrupt status register value.
 */

u32 XV_vcresampler_InterruptGetStatus(XV_vcresampler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vcresampler_ReadReg(InstancePtr->Config.BaseAddress, XV_VCRESAMPLER_CTRL_ADDR_ISR);
}
