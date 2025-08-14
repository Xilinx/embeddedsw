// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_letterbox.c
 * @addtogroup xv_letterbox Overview
 */

/***************************** Include Files *********************************/
#include "xv_letterbox.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/**
 * XV_letterbox_CfgInitialize - Initializes a specific XV_letterbox instance.
 *
 * This function sets up the XV_letterbox instance pointed to by InstancePtr
 * using the configuration provided in ConfigPtr and the specified base address
 * EffectiveAddr. It asserts that all input pointers are valid, copies the
 * configuration, updates the base address, and marks the instance as ready.
 *
 * @param InstancePtr    Pointer to the XV_letterbox instance to be initialized.
 * @param ConfigPtr      Pointer to the configuration structure for the instance.
 * @param EffectiveAddr  Base address to be used for the hardware instance.
 *
 * @return XST_SUCCESS if initialization is successful.
 */
int XV_letterbox_CfgInitialize(XV_letterbox *InstancePtr,
                               XV_letterbox_Config *ConfigPtr,
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
 * XV_letterbox_Start - Starts the XV_letterbox hardware core.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * hardware component is ready. It then reads the current control register value,
 * preserves the auto-restart bit, and sets the start bit to initiate the
 * hardware operation.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance to be started.
 *
 * @return This function does not return a value.
 */
void XV_letterbox_Start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * XV_letterbox_IsDone - Checks if the XV_letterbox hardware core has finished
 * processing.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * hardware component is ready. It reads the control register and checks the
 * 'done' bit to determine if the hardware operation is complete.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance to be checked.
 *
 * @return 1 if the operation is done, 0 otherwise.
 */
u32 XV_letterbox_IsDone(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
                               XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}


/**
 * Checks if the XV_letterbox hardware core is idle.
 *
 * This function reads the control register of the XV_letterbox instance to determine
 * if the hardware is currently idle. It asserts that the instance pointer is not NULL
 * and that the instance is ready before accessing the hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *
 * @return   1 if the hardware is idle, 0 otherwise.
 */

u32 XV_letterbox_IsIdle(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}


/**
 * Checks if the XV_letterbox hardware core is ready to accept new input.
 *
 * This function reads the control register of the XV_letterbox instance to determine
 * if the hardware core is ready for the next operation. It asserts that the instance
 * pointer is not NULL and that the instance is marked as ready. The function returns
 * a non-zero value if the core is ready, and zero otherwise.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *
 * @return   1 if the hardware core is ready for new input, 0 otherwise.
 */
u32 XV_letterbox_IsReady(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}


/**
 * XV_letterbox_EnableAutoRestart - Enables auto-restart for the XV_letterbox
 * hardware core.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * hardware component is ready. It writes to the control register to enable the
 * auto-restart feature, allowing the hardware to automatically restart
 * processing after completion without software intervention.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance.
 *
 * @return This function does not return a value.
 */
void XV_letterbox_EnableAutoRestart(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
                          XV_LETTERBOX_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * XV_letterbox_DisableAutoRestart - Disables auto-restart for the XV_letterbox
 * hardware core.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * hardware component is ready. It writes to the control register to disable the
 * auto-restart feature, so the hardware will not automatically restart after
 * completion.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance.
 *
 * @return This function does not return a value.
 */
void XV_letterbox_DisableAutoRestart(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * Sets the hardware register value for the width parameter of the XV_letterbox instance.
 *
 * This function writes the specified width value to the hardware register associated
 * with the XV_letterbox core. It first asserts that the instance pointer is not NULL
 * and that the instance is ready before performing the register write.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The width value to be written to the hardware register.
 * @return None.
 */


void XV_letterbox_Set_HwReg_width(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_WIDTH hardware register from the XV_letterbox instance.
 *
 * This function reads the current width configuration from the hardware register
 * associated with the XV_letterbox peripheral.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *           It must be a valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_WIDTH register as a 32-bit unsigned integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */

u32 XV_letterbox_Get_HwReg_width(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the height parameter of the XV_letterbox
 * instance.
 *
 * This function writes the specified height value to the hardware register
 * associated with the XV_letterbox core. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The height value to be written to the hardware register.
 * @return None.
 */
void XV_letterbox_Set_HwReg_height(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_HEIGHT hardware register from the
 * XV_letterbox instance.
 *
 * This function reads the current height configuration from the hardware
 * register associated with the XV_letterbox peripheral.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *           It must be a valid pointer to an initialized XV_letterbox
 *           structure.
 *
 * @return   The value of the HWREG_HEIGHT register as a 32-bit unsigned
 *           integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_letterbox_Get_HwReg_height(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}


/**
 * Sets the hardware register for the video format in the XV_letterbox instance.
 *
 * This function writes the specified video format data to the hardware register
 * associated with the XV_letterbox instance. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the write
 * operation.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The video format data to be written to the hardware register.
 * @return None.
 */

void XV_letterbox_Set_HwReg_video_format(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA, Data);
}


/**
 * Retrieves the current hardware register value for the video format from the XV_letterbox instance.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *
 * @return   The value of the HWREG_VIDEO_FORMAT register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that the hardware is ready.
 */

u32 XV_letterbox_Get_HwReg_video_format(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_VIDEO_FORMAT_DATA);
    return Data;
}

/**
 * Sets the hardware register value for the column start parameter of the
 * XV_letterbox instance.
 *
 * This function writes the specified column start value to the hardware
 * register associated with the XV_letterbox core. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the register write.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The column start value to be written to the hardware
 *                    register.
 * @return None.
 */
void XV_letterbox_Set_HwReg_col_start(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_COL_START_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_COL_START hardware register from the
 * XV_letterbox instance.
 *
 * This function reads the current column start configuration from the hardware
 * register associated with the XV_letterbox peripheral.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance. It must be a
 *           valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_COL_START register as a 32-bit unsigned
 *           integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_letterbox_Get_HwReg_col_start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_COL_START_DATA);
    return Data;
}


/**
 * XV_letterbox_Set_HwReg_col_end - Sets the hardware register value for the column
 * end parameter in the XV_letterbox instance. This function writes the specified
 * data value to the HWREG_COL_END register of the letterbox hardware, using the
 * base address from the instance configuration. It first asserts that the
 * instance pointer is not NULL and that the instance is ready before performing
 * the register write operation.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance.
 * @param Data:        Value to be written to the HWREG_COL_END register.
 * @return None
 */

void XV_letterbox_Set_HwReg_col_end(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_END_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_COL_END hardware register for the specified
 * XV_letterbox instance. This function asserts that the provided instance
 * pointer is not NULL and that the instance is ready before reading the
 * register value from the hardware. The value read from the HWREG_COL_END
 * register is then returned to the caller.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance for which
 *          the HWREG_COL_END register value is to be retrieved.
 *
 * @return   The value of the HWREG_COL_END hardware register.
 */
u32 XV_letterbox_Get_HwReg_col_end(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_COL_END_DATA);
    return Data;
}


/**
 * Sets the hardware register value for the row start parameter of the
 * XV_letterbox instance.
 *
 * This function writes the specified row start value to the hardware register
 * associated with the XV_letterbox core. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The row start value to be written to the hardware
 *                    register.
 * @return None.
 */
void XV_letterbox_Set_HwReg_row_start(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_START_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_ROW_START hardware register from the
 * XV_letterbox instance.
 *
 * This function reads the current row start configuration from the hardware
 * register associated with the XV_letterbox peripheral.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance. It must be a
 *           valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_ROW_START register as a 32-bit unsigned
 *           integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_letterbox_Get_HwReg_row_start(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_START_DATA);
    return Data;
}


/**
 * Sets the hardware register value for the row end parameter of the
 * XV_letterbox instance.
 *
 * This function writes the specified row end value to the hardware register
 * associated with the XV_letterbox core. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Data        The row end value to be written to the hardware register.
 * @return None.
 */
void XV_letterbox_Set_HwReg_row_end(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_END_DATA, Data);
}

/**
 * Retrieves the value of the HWREG_ROW_END hardware register from the
 * XV_letterbox instance.
 *
 * This function reads the current row end configuration from the hardware
 * register associated with the XV_letterbox peripheral.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance. It must be a
 *           valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_ROW_END register as a 32-bit unsigned
 *           integer.
 *
 * @note     The function asserts that InstancePtr is not NULL and that the
 *           instance is ready before accessing the hardware register.
 */
u32 XV_letterbox_Get_HwReg_row_end(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_ROW_END_DATA);
    return Data;
}


/**
 * XV_letterbox_Set_HwReg_Y_R_value - Sets the Y register value for the hardware
 * register in the XV_letterbox instance. This function asserts that the
 * provided instance pointer is not NULL and that the instance is ready before
 * writing the specified data value to the HWREG_Y_R register using the
 * XV_letterbox_WriteReg function.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance. Must not be NULL
 * and must be initialized.
 * @param Data: The 32-bit value to be written to the HWREG_Y_R register.
 */
void XV_letterbox_Set_HwReg_Y_R_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_Y_R_VALUE_DATA, Data);
}



/**
 * Retrieves the value of the HWREG_Y_R hardware register from the XV_letterbox
 * instance.
 *
 * This function reads the current Y (or R) value from the hardware register
 * associated with the XV_letterbox peripheral. It asserts that the instance
 * pointer is not NULL and that the instance is ready before accessing the
 * hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance. It must be a
 *           valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_Y_R register as a 32-bit unsigned integer.
 */
u32 XV_letterbox_Get_HwReg_Y_R_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_Y_R_VALUE_DATA);
    return Data;
}


/**
 * XV_letterbox_Set_HwReg_Cb_G_value - Sets the Cb/G register value for the
 * hardware register in the XV_letterbox instance.
 *
 * This function writes the specified data value to the HWREG_CB_G register
 * of the XV_letterbox hardware. It asserts that the provided instance pointer
 * is not NULL and that the instance is ready before performing the register
 * write operation.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance. Must not be NULL
 *                    and must be initialized.
 * @param Data        The 32-bit value to be written to the HWREG_CB_G register.
 * @return None
 */
void XV_letterbox_Set_HwReg_Cb_G_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_CB_G_VALUE_DATA, Data);
}


/**
 * Retrieves the value of the HWREG_CB_G register from the XV_letterbox hardware
 * instance. This function reads the register value from the hardware using the
 * base address specified in the configuration structure of the provided
 * XV_letterbox instance pointer.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 *           It must be initialized and ready before calling this function.
 *
 * @return   The 32-bit value read from the HWREG_CB_G register.
 */

u32 XV_letterbox_Get_HwReg_Cb_G_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CB_G_VALUE_DATA);
    return Data;
}



/**
 * XV_letterbox_Set_HwReg_Cr_B_value - Set the value of the HWREG_CR_B register
 * for the XV_letterbox instance.
 *
 * This function writes the specified data value to the HWREG_CR_B register of
 * the XV_letterbox hardware. It first asserts that the provided instance
 * pointer is not NULL and that the instance is ready for use. The function
 * then writes the data to the hardware register using the base address from
 * the configuration structure.
 *
 * @param InstancePtr: Pointer to the XV_letterbox instance.
 * @param Data:        Value to be written to the HWREG_CR_B register.
 * @return None.
 */
void XV_letterbox_Set_HwReg_Cr_B_value(XV_letterbox *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_HWREG_CR_B_VALUE_DATA, Data);
}



/**
 * Retrieves the value of the HWREG_CR_B register from the XV_letterbox instance.
 *
 * This function reads the current value of the HWREG_CR_B hardware register
 * associated with the XV_letterbox peripheral. It asserts that the instance
 * pointer is not NULL and that the instance is ready before accessing the
 * hardware register.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance. It must be a
 *           valid pointer to an initialized XV_letterbox structure.
 *
 * @return   The value of the HWREG_CR_B register as a 32-bit unsigned integer.
 */
u32 XV_letterbox_Get_HwReg_Cr_B_value(XV_letterbox *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_HWREG_CR_B_VALUE_DATA);
    return Data;
}

/**
 * XV_letterbox_InterruptGlobalEnable - Enables global interrupts for the
 * XV_letterbox hardware core.
 *
 * This function asserts that the provided InstancePtr is not NULL and that
 * the hardware component is ready. It writes to the global interrupt enable
 * register to allow interrupt generation by the hardware.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 *
 * @return None.
 */
void XV_letterbox_InterruptGlobalEnable(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
                          XV_LETTERBOX_CTRL_ADDR_GIE, 1);
}



/**
 * XV_letterbox_InterruptGlobalDisable - Disables global interrupts for the
 * XV_letterbox hardware core.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * hardware component is ready. It writes to the global interrupt enable register
 * to disable interrupt generation by the hardware.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 *
 * @return None.
 */
void XV_letterbox_InterruptGlobalDisable(XV_letterbox *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
                          XV_LETTERBOX_CTRL_ADDR_GIE, 0);
}



/**
 * Enables the specified interrupts for the XV_letterbox instance.
 *
 * This function reads the current interrupt enable register, applies the given
 * mask to enable the specified interrupts, and writes the updated value back to
 * the register. It asserts that the instance pointer is not NULL and that the
 * instance is ready before performing any operations.
 *
 * @param    InstancePtr is a pointer to the XV_letterbox instance.
 * @param    Mask is the bitmask of interrupts to enable. Each bit in the mask
 *           corresponds to a specific interrupt source.
 * @return None
 */
void XV_letterbox_InterruptEnable(XV_letterbox *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER);
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER, Register | Mask);
}

/**
 * XV_letterbox_InterruptDisable - Disables specific interrupts for the XV_letterbox
 * instance.
 *
 * This function disables the interrupts specified by the Mask parameter for the
 * given XV_letterbox instance. It reads the current interrupt enable register,
 * clears the bits corresponding to the Mask, and writes the updated value back
 * to the register. The function asserts that the InstancePtr is not NULL and
 * that the instance is ready before performing the operation.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Mask        Bitmask of interrupts to disable.
 * @return None
 */


void XV_letterbox_InterruptDisable(XV_letterbox *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER);
    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_IER, Register & (~Mask));
}


/**
 * XV_letterbox_InterruptClear - Clears specific interrupts for the XV_letterbox
 * instance.
 *
 * This function writes the specified Mask to the interrupt status register to
 * clear the corresponding interrupt sources for the given XV_letterbox instance.
 * It asserts that the InstancePtr is not NULL and that the instance is ready
 * before performing the operation.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 * @param Mask        Bitmask of interrupts to clear.
 * @return None.
 */
void XV_letterbox_InterruptClear(XV_letterbox *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_letterbox_WriteReg(InstancePtr->Config.BaseAddress,
        XV_LETTERBOX_CTRL_ADDR_ISR, Mask);
}

/**
 * XV_letterbox_InterruptGetEnabled - Retrieves the currently enabled interrupts
 * for the XV_letterbox instance.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * instance is ready. It reads the interrupt enable register and returns its
 * value, indicating which interrupts are currently enabled for the hardware
 * core.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 *
 * @return The value of the interrupt enable register as a 32-bit unsigned
 * integer.
 */
u32 XV_letterbox_InterruptGetEnabled(XV_letterbox *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress,
                               XV_LETTERBOX_CTRL_ADDR_IER);
}


/**
 * XV_letterbox_InterruptGetStatus - Retrieves the current interrupt status for
 * the XV_letterbox instance.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * instance is ready. It reads the interrupt status register and returns its
 * value, indicating which interrupts are currently active for the hardware core.
 *
 * @param InstancePtr Pointer to the XV_letterbox instance.
 *
 * @return The value of the interrupt status register as a 32-bit unsigned
 * integer.
 */
u32 XV_letterbox_InterruptGetStatus(XV_letterbox *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_letterbox_ReadReg(InstancePtr->Config.BaseAddress, XV_LETTERBOX_CTRL_ADDR_ISR);
}
