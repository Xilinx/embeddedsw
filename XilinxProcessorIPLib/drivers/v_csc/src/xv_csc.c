// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_csc.c
 * @addtogroup v_csc Overview
*/


/***************************** Include Files *********************************/
/*
 * Include the header file for the XV_csc driver.
 * This header provides the necessary type definitions, macros,
 * and function prototypes for interacting with the Color Space Converter (CSC) hardware.
 */
#include "xv_csc.h"

/************************** Function Implementation *************************/
#ifndef __linux__

/**
 * XV_csc_CfgInitialize - Initializes a specific XV_csc instance.
 *
 * This function initializes an XV_csc instance using the provided configuration
 * structure and effective base address. It copies the configuration, sets the
 * base address, and marks the instance as ready for use.
 *
 * @param InstancePtr: Pointer to the XV_csc instance to be initialized.
 * @param ConfigPtr: Pointer to the configuration structure for the instance.
 * @param EffectiveAddr: Physical base address for the hardware instance.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - ConfigPtr must not be NULL.
 *   - EffectiveAddr must not be NULL.
 */

int XV_csc_CfgInitialize(XV_csc *InstancePtr,
                         XV_csc_Config *ConfigPtr,
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
 * Starts the Color Space Converter (CSC) hardware core.
 *
 * This function initiates the operation of the CSC hardware by setting the
 * appropriate control register bit. It first asserts that the provided
 * instance pointer is valid and that the core is ready. Then, it reads the
 * current value of the control register, preserves the auto-restart bit,
 * and sets the start bit to begin processing.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return None
 *
 * @note This function does not wait for the operation to complete.
 */

void XV_csc_Start(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * Checks if the XV_csc hardware core has completed its current operation.
 *
 * This function reads the control register of the XV_csc instance to determine
 * if the hardware operation is done. It asserts that the instance pointer is
 * not NULL and that the instance is ready before accessing the hardware.
 *
 * @param    InstancePtr is a pointer to the XV_csc instance.
 *
 * @return   1 if the operation is done, 0 otherwise.
 */

u32 XV_csc_IsDone(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * Checks if the XV_csc hardware core is idle.
 *
 * This function reads the control register of the XV_csc instance and determines
 * whether the core is currently idle. It asserts that the instance pointer is not
 * NULL and that the core is ready before performing the check.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return
 *   - 1 if the core is idle.
 *   - 0 if the core is not idle.
 */

u32 XV_csc_IsIdle(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * Checks if the XV_csc hardware core is ready to accept new input.
 *
 * This function reads the control register of the XV_csc instance to determine
 * if the core is ready for the next operation. It asserts that the instance pointer
 * is valid and that the core has been initialized. The readiness is determined by
 * checking the 'ap_start' bit in the control register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return
 *   - 1 if the core is ready for new input.
 *   - 0 if the core is busy processing previous input.
 */

u32 XV_csc_IsReady(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * Enables the auto-restart feature for the Color Space Converter (CSC) hardware.
 *
 * This function sets the appropriate control register bit to enable automatic
 * restarting of the CSC hardware after each operation, allowing continuous processing
 * without manual intervention.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @pre
 *   - InstancePtr must not be NULL.
 *   - InstancePtr->IsReady must be equal to XIL_COMPONENT_IS_READY.
 *
 * @note
 *   - This function does not check if auto-restart is already enabled.
 *   - The function is intended for use with Xilinx CSC hardware.
 */

void XV_csc_EnableAutoRestart(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * Disable the auto-restart feature of the XV_csc hardware core.
 *
 * This function disables the auto-restart mechanism by writing 0 to the
 * AP_CTRL register of the core. Auto-restart allows the core to automatically
 * start processing after completing the previous operation. Disabling it
 * requires manual intervention to start subsequent operations.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @pre
 *  - InstancePtr must not be NULL.
 *  - InstancePtr->IsReady must be set to XIL_COMPONENT_IS_READY.
 *
 * @return None.
 */

void XV_csc_DisableAutoRestart(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the input video format hardware register for the XV_csc instance.
 *
 * This function writes the specified data value to the hardware register
 * responsible for configuring the input video format of the Color Space Converter (CSC).
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the input video format register.
 *
 * @return None.
 * @note The instance pointer must not be NULL and the instance must be ready.
 */

void XV_csc_Set_HwReg_InVideoFormat(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_INVIDEOFORMAT_DATA, Data);
}

/**
 * Retrieves the hardware register value for the input video format.
 *
 * This function reads the value from the hardware register that specifies
 * the input video format for the Color Space Converter (CSC) instance.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *        Must not be NULL and must be initialized.
 *
 * @return The value of the input video format hardware register.
 */
u32 XV_csc_Get_HwReg_InVideoFormat(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_INVIDEOFORMAT_DATA);
    return Data;
}

/**
 * Sets the output video format hardware register for the CSC (Color Space Converter) instance.
 *
 * This function writes the specified data value to the hardware register responsible for
 * configuring the output video format of the CSC core.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the output video format register.
 *
 * @note The function asserts that the instance pointer is not NULL and that the instance is ready.
 * @return None
 */

void XV_csc_Set_HwReg_OutVideoFormat(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_OUTVIDEOFORMAT_DATA, Data);
}

/**
 * Retrieves the output video format hardware register value from the CSC (Color Space Converter) instance.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The value of the HWREG_OUTVIDEOFORMAT register.
 *
 * @note This function asserts that the instance pointer is valid and the component is ready.
 */

u32 XV_csc_Get_HwReg_OutVideoFormat(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_OUTVIDEOFORMAT_DATA);
    return Data;
}

/**
 * Sets the hardware register for the width parameter of the XV_csc instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with the XV_csc instance. It first checks that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Width value to be written to the hardware register.
 * @return None.
 */

void XV_csc_Set_HwReg_width(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}
/**
 * Retrieves the hardware register value for the width parameter of the CSC (Color Space Converter) instance.
 *
 * This function reads the width value from the hardware register associated with the given CSC instance.
 * It asserts that the instance pointer is not NULL and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The width value read from the hardware register.
 */

u32 XV_csc_Get_HwReg_width(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * Sets the hardware register for the height parameter in the XV_csc instance.
 *
 * This function writes the specified height value to the hardware register
 * responsible for controlling the height in the Color Space Converter (CSC) core.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        The height value to be written to the hardware register.
 *
 * @return None
 * @note The instance pointer must not be NULL and the instance must be ready.
 */

void XV_csc_Set_HwReg_height(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}
/**
 * Retrieves the value of the hardware register that stores the height parameter
 * for the Color Space Converter (CSC) instance.
 *
 * This function reads the height value from the hardware register associated
 * with the CSC peripheral. It asserts that the provided instance pointer is
 * not NULL and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return The value of the height register (u32).
 */

u32 XV_csc_Get_HwReg_height(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}
/**
 * Sets the hardware register for column start value in the Color Space Converter (CSC) core.
 *
 * This function writes the specified column start value to the hardware register
 * responsible for controlling the column start position in the CSC core.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        The column start value to be written to the hardware register.
 *
 * @return None
 *
 * @note The function asserts that the instance pointer is not NULL and that the
 *       instance is ready before performing the register write.
 */

void XV_csc_Set_HwReg_ColStart(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLSTART_DATA, Data);
}
/**
 * Retrieves the value of the hardware register for column start from the CSC (Color Space Converter) instance.
 *
 * This function reads the HWREG_COLSTART register from the hardware using the base address
 * provided in the instance configuration. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the read operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return The value of the HWREG_COLSTART hardware register.
 */

u32 XV_csc_Get_HwReg_ColStart(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLSTART_DATA);
    return Data;
}

/**
 * Sets the hardware register for the column end value in the Color Space Converter (CSC) core.
 *
 * This function writes the specified column end value to the corresponding hardware register.
 * It first asserts that the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        The column end value to be written to the hardware register.
 * @return None
 */

void XV_csc_Set_HwReg_ColEnd(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLEND_DATA, Data);
}
/**
 * Retrieves the value of the HWREG_COLEND hardware register from the Color Space Converter (CSC) instance.
 *
 * This function reads the column end value from the CSC hardware register, which is used to determine
 * the end column for color space conversion operations.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *        - Must not be NULL.
 *        - Instance must be initialized and ready.
 *
 * @return The value of the HWREG_COLEND register.
 *
 * @note The function asserts that the instance pointer is valid and the component is ready.
 */

u32 XV_csc_Get_HwReg_ColEnd(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_COLEND_DATA);
    return Data;
}
/**
 * Sets the hardware register for the row start value in the Color Space Converter (CSC) core.
 *
 * This function writes the specified row start value to the corresponding hardware register.
 * It asserts that the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_ROWSTART register.
 * @return None
 */

void XV_csc_Set_HwReg_RowStart(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWSTART_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_ROWSTART hardware register from the CSC core.
 *
 * This function reads the HWREG_ROWSTART register using the base address from the
 * configuration of the provided XV_csc instance. It asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the read operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return The value of the HWREG_ROWSTART register.
 */

u32 XV_csc_Get_HwReg_RowStart(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWSTART_DATA);
    return Data;
}
/**
 * Sets the hardware register for the row end value in the Color Space Converter (CSC) core.
 *
 * This function writes the specified data to the HWREG_ROWEND register of the CSC hardware.
 * It asserts that the provided instance pointer is not NULL and that the instance is ready
 * before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_ROWEND register.
 * @return None
 */

void XV_csc_Set_HwReg_RowEnd(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWEND_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_ROWEND hardware register from the Color Space Converter (CSC) instance.
 *
 * This function reads the HWREG_ROWEND register, which typically indicates the end row address or value
 * used in hardware processing for the CSC core. It asserts that the provided instance pointer is valid
 * and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value read from the HWREG_ROWEND register.
 */

u32 XV_csc_Get_HwReg_RowEnd(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROWEND_DATA);
    return Data;
}
/**
 * Sets the value of the HWREG_K11 hardware register for the CSC (Color Space Converter) instance.
 *
 * This function writes the specified data to the HWREG_K11 register of the CSC hardware.
 * It first asserts that the provided instance pointer is not NULL and that the instance is ready.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_K11 register.
 * @return None
 */

void XV_csc_Set_HwReg_K11(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_DATA, Data);
}
/**
 * Retrieves the value of the HWREG_K11 hardware register from the CSC (Color Space Converter) instance.
 *
 * This function reads the value of the K11 register from the hardware using the base address
 * specified in the instance configuration. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the read operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value read from the HWREG_K11 register.
 */

u32 XV_csc_Get_HwReg_K11(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_DATA);
    return Data;
}
/**
 * Set the value of the HWREG_K12 hardware register for the XV_csc instance.
 *
 * This function writes the specified data to the HWREG_K12 register of the
 * Color Space Converter (CSC) hardware. It first checks that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        32-bit value to write to the HWREG_K12 register.
 * @return None
 */

void XV_csc_Set_HwReg_K12(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_DATA, Data);
}
/**
 * Retrieves the value of the HWREG_K12 hardware register from the CSC (Color Space Converter) instance.
 *
 * This function reads the value of the HWREG_K12 register using the base address from the instance's configuration.
 * It asserts that the instance pointer is not NULL and that the instance is ready before accessing the register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The 32-bit value read from the HWREG_K12 register.
 */

u32 XV_csc_Get_HwReg_K12(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_DATA);
    return Data;
}
/**
 * Set the value of the HWREG_K13 hardware register for the XV_csc instance.
 *
 * This function writes the specified data to the HWREG_K13 register of the
 * Color Space Converter (CSC) hardware. It first checks that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_K13 register.
 * @return None
 */

void XV_csc_Set_HwReg_K13(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_K13 hardware register from the CSC (Color Space Converter) instance.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *        - Must not be NULL.
 *        - Must be initialized and ready.
 *
 * @return The 32-bit value read from the HWREG_K13 register.
 *
 * @note This function asserts that the instance pointer is valid and the instance is ready.
 */

u32 XV_csc_Get_HwReg_K13(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_K21 hardware register for the XV_csc instance.
 *
 * This function writes the specified data to the HWREG_K21 register of the
 * Color Space Converter (CSC) hardware. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        32-bit value to be written to the HWREG_K21 register.
 * @return None
 */

void XV_csc_Set_HwReg_K21(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_K21 hardware register from the CSC (Color Space Converter) instance.
 *
 * This function reads the value of the HWREG_K21 register using the base address
 * specified in the instance configuration. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the read operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The 32-bit value read from the HWREG_K21 register.
 */

u32 XV_csc_Get_HwReg_K21(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_K22 hardware register for the XV_csc instance.
 *
 * This function writes the specified data to the HWREG_K22 register of the
 * Color Space Converter (CSC) hardware. It first checks that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_K22 register.
 * @return None
 */

void XV_csc_Set_HwReg_K22(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_K22 hardware register from the CSC (Color Space Converter) instance.
 *
 * This function reads the value of the K22 register from the hardware using the base address
 * specified in the instance configuration. It asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the read operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value read from the HWREG_K22 register.
 */

u32 XV_csc_Get_HwReg_K22(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_DATA);
    return Data;
}

/**
 * Sets the value of the HWREG_K23 hardware register for the XV_csc instance.
 *
 * This function writes the specified data to the HWREG_K23 register of the
 * Color Space Converter (CSC) hardware. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write operation.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data        Value to be written to the HWREG_K23 register.
 * @return None
 */
void XV_csc_Set_HwReg_K23(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_DATA, Data);
}

/**
 * This function gets the K23 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K23 register.
 */
u32 XV_csc_Get_HwReg_K23(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_DATA);
    return Data;
}

/**
 * This function sets the K31 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K31 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K31(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_DATA, Data);
}

/**
 * This function gets the K31 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K31 register.
 */
u32 XV_csc_Get_HwReg_K31(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_DATA);
    return Data;
}

/**
 * This function sets the K32 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K32 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K32(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_DATA, Data);
}

/**
 * This function gets the K32 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K32 register.
 */
u32 XV_csc_Get_HwReg_K32(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_DATA);
    return Data;
}

/**
 * This function sets the K33 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K33 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K33(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_DATA, Data);
}

/**
 * This function gets the K33 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K33 register.
 */
u32 XV_csc_Get_HwReg_K33(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_DATA);
    return Data;
}


/**
 * This function sets the ROffset_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the ROffset_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_ROffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_V_DATA, Data);
}

/**
 * This function gets the ROffset_V hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the ROffset_V register.
 */
u32 XV_csc_Get_HwReg_ROffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_V_DATA);
    return Data;
}

/**
 * This function sets the GOffset_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the GOffset_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_GOffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_V_DATA, Data);
}

/**
 * This function gets the GOffset_V hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the GOffset_V register.
 */
u32 XV_csc_Get_HwReg_GOffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_V_DATA);
    return Data;
}

/**
 * This function sets the BOffset_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the BOffset_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_BOffset_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_V_DATA, Data);
}

/**
 * This function gets the BOffset_V hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the BOffset_V register.
 */
u32 XV_csc_Get_HwReg_BOffset_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_V_DATA);
    return Data;
}

/**
 * This function sets the ClampMin_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the ClampMin_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_ClampMin_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_V_DATA, Data);
}

/**
 * This function gets the ClampMin_V hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the ClampMin_V register.
 */
u32 XV_csc_Get_HwReg_ClampMin_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_V_DATA);
    return Data;
}

/**
 * This function sets the ClipMax_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the ClipMax_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_ClipMax_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_V_DATA, Data);
}

/**
 * This function gets the ClipMax_V hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the ClipMax_V register.
 */
u32 XV_csc_Get_HwReg_ClipMax_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_V_DATA);
    return Data;
}

/**
 * This function sets the K11_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K11_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K11_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_2_DATA, Data);
}

/**
 * This function gets the K11_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K11_2 register.
 */
u32 XV_csc_Get_HwReg_K11_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K11_2_DATA);
    return Data;
}

/**
 * This function sets the K12_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K12_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K12_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_2_DATA, Data);
}

/**
 * This function gets the K12_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K12_2 register.
 */
u32 XV_csc_Get_HwReg_K12_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K12_2_DATA);
    return Data;
}

/**
 * This function sets the K13_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K13_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K13_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_2_DATA, Data);
}

/**
 * This function gets the K13_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K13_2 register.
 */
u32 XV_csc_Get_HwReg_K13_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K13_2_DATA);
    return Data;
}

/**
 * This function sets the K21_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K21_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K21_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_2_DATA, Data);
}

/**
 * This function gets the K21_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K21_2 register.
 */
u32 XV_csc_Get_HwReg_K21_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K21_2_DATA);
    return Data;
}

/**
 * This function sets the K22_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K22_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K22_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_2_DATA, Data);
}

/**
 * This function gets the K22_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K22_2 register.
 */
u32 XV_csc_Get_HwReg_K22_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K22_2_DATA);
    return Data;
}

/**
 * This function sets the K23_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K23_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K23_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_2_DATA, Data);
}

/**
 * This function gets the K23_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K23_2 register.
 */
u32 XV_csc_Get_HwReg_K23_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K23_2_DATA);
    return Data;
}

/**
 * This function sets the K31_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K31_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K31_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_2_DATA, Data);
}

/**
 * This function gets the K31_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K31_2 register.
 */
u32 XV_csc_Get_HwReg_K31_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K31_2_DATA);
    return Data;
}

/**
 * This function sets the K32_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K32_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K32_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_2_DATA, Data);
}

/**
 * This function gets the K32_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K32_2 register.
 */
u32 XV_csc_Get_HwReg_K32_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K32_2_DATA);
    return Data;
}

/**
 * This function sets the K33_2 hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the K33_2 register.
 * @return None.
 */
void XV_csc_Set_HwReg_K33_2(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_2_DATA, Data);
}

/**
 * This function gets the K33_2 hardware register value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the K33_2 register.
 */
u32 XV_csc_Get_HwReg_K33_2(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_K33_2_DATA);
    return Data;
}

/**
 * This function sets the ROffset_2_V hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data Data to write to the ROffset_2_V register.
 * @return None.
 */
void XV_csc_Set_HwReg_ROffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_2_V_DATA, Data);
}

/**
 * This function gets the hardware register R offset 2 value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the hardware register R offset 2.
 */
u32 XV_csc_Get_HwReg_ROffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_ROFFSET_2_V_DATA);
    return Data;
}

/**
 * This function sets the hardware register G offset 2 value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data The value to set in the hardware register G offset 2.
 * @return None
 */
void XV_csc_Set_HwReg_GOffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_2_V_DATA, Data);
}

/**
 * This function gets the hardware register G offset 2 value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the hardware register G offset 2.
 */
u32 XV_csc_Get_HwReg_GOffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_GOFFSET_2_V_DATA);
    return Data;
}

/**
 * This function sets the hardware register B offset 2 value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data The value to set in the hardware register B offset 2.
 * @return None
 */
void XV_csc_Set_HwReg_BOffset_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_2_V_DATA, Data);
}

/**
 * This function gets the hardware register B offset 2 value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the hardware register B offset 2.
 */
u32 XV_csc_Get_HwReg_BOffset_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_BOFFSET_2_V_DATA);
    return Data;
}

/**
 * This function sets the hardware register clamp minimum value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data The value to set in the hardware register clamp minimum.
 * @return None
 */
void XV_csc_Set_HwReg_ClampMin_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_2_V_DATA, Data);
}

/**
 * This function gets the hardware register clamp minimum value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the hardware register clamp minimum.
 */
u32 XV_csc_Get_HwReg_ClampMin_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLAMPMIN_2_V_DATA);
    return Data;
}

/**
 * This function sets the hardware register clip maximum value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Data The value to set in the hardware register clip maximum.
 * @return None
 */
void XV_csc_Set_HwReg_ClipMax_2_V(XV_csc *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_2_V_DATA, Data);
}

/**
 * This function gets the hardware register clip maximum value.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the hardware register clip maximum.
 */
u32 XV_csc_Get_HwReg_ClipMax_2_V(XV_csc *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_HWREG_CLIPMAX_2_V_DATA);
    return Data;
}

/**
 * This function enables global interrupts.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return None
 */
void XV_csc_InterruptGlobalEnable(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_GIE, 1);
}

/**
 * This function disables global interrupts.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return None
 */
void XV_csc_InterruptGlobalDisable(XV_csc *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_GIE, 0);
}

/**
 * This function enables specific interrupts.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Mask The mask of interrupts to enable.
 * @return None
 */
void XV_csc_InterruptEnable(XV_csc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register = XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER, Register | Mask);
}




/**
 * Disable specific interrupts for the XV_csc instance.
 *
 * This function disables the interrupts specified by the Mask parameter
 * for the given XV_csc instance. It reads the current interrupt enable
 * register, clears the bits specified by Mask, and writes the updated
 * value back to the register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Mask Bitmask specifying which interrupts to disable.
 *
 * @note
 * - The InstancePtr must be valid and initialized.
 * - Only the interrupts specified by Mask will be disabled.
 */
void XV_csc_InterruptDisable(XV_csc *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER, Register & (~Mask));
}


/**
 * This function clears specific interrupts.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @param Mask The mask of interrupts to clear.
 * @return None
 */
void XV_csc_InterruptClear(XV_csc *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_csc_WriteReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_ISR, Mask);
}



/**
 * This Function Retrieves the interrupt enable register value for the XV_csc instance.
 *
 * This function checks that the provided instance pointer is valid and that the
 * instance is ready. It then reads and returns the value of the Interrupt Enable
 * Register (IER) from the hardware.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 * @return The value of the interrupt enable register.
 */
u32 XV_csc_InterruptGetEnabled(XV_csc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_IER);
}



/**
 * Retrieves the interrupt status of the Color Space Converter (CSC) core.
 *
 * This function reads the Interrupt Status Register (ISR) of the CSC hardware
 * to determine the current interrupt status. It asserts that the provided
 * instance pointer is not NULL and that the instance is ready before accessing
 * the hardware register.
 *
 * @param InstancePtr Pointer to the XV_csc instance.
 *
 * @return
 *   The value of the interrupt status register (ISR).
 */

u32 XV_csc_InterruptGetStatus(XV_csc *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_csc_ReadReg(InstancePtr->Config.BaseAddress, XV_CSC_CTRL_ADDR_ISR);
}
