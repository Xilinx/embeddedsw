// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_vscaler.c
 * @addtogroup xv_vscaler Overview
 */
/***************************** Include Files *********************************/
#include "xv_vscaler.h"

/************************** Function Implementation *************************/
#ifndef __linux__

/**
 * XV_vscaler_CfgInitialize - Initialize the XV_vscaler instance.
 *
 * This function initializes an XV_vscaler instance using the provided
 * configuration structure and effective base address. It sets up the instance
 * configuration, assigns the base address, and marks the driver as ready.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance to initialize.
 * @param ConfigPtr: Pointer to the configuration structure.
 * @param EffectiveAddr: Physical base address of the device.
 * @return XST_SUCCESS if successful.
 */

int XV_vscaler_CfgInitialize(XV_vscaler *InstancePtr,
                             XV_vscaler_Config *ConfigPtr,
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
 * XV_vscaler_Start - Start the v_vscaler hardware.
 *
 * This function sets the ap_start bit in the control register to start the
 * hardware. It preserves the auto-restart bit (bit 7) if it was previously set.
 * The function asserts that the InstancePtr is not NULL and that the driver is
 * ready before accessing the hardware registers.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return None.
 */
void XV_vscaler_Start(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * XV_vscaler_IsDone - Check if the v_vscaler hardware has finished processing.
 *
 * This function reads the control register and checks the ap_done bit (bit 1)
 * to determine if the hardware has completed its operation. It asserts that
 * the InstancePtr is not NULL and that the driver is ready before accessing
 * the hardware registers.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return 1 if the hardware is done, 0 otherwise.
 */
u32 XV_vscaler_IsDone(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/**
 * XV_vscaler_IsIdle - Check if the v_vscaler hardware is idle.
 *
 * This function reads the control register and checks the ap_idle bit (bit 2)
 * to determine if the hardware is currently idle. It asserts that the
 * InstancePtr is not NULL and that the driver is ready before accessing the
 * hardware registers.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return 1 if the hardware is idle, 0 otherwise.
 */
u32 XV_vscaler_IsIdle(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/**
 * XV_vscaler_IsReady - Check if the v_vscaler hardware is ready for new input.
 *
 * This function reads the control register and checks the ap_start bit (bit 0)
 * to determine if the hardware is ready to accept new input. It asserts that
 * the InstancePtr is not NULL and that the driver is ready before accessing
 * the hardware registers.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return 1 if the hardware is ready for new input, 0 otherwise.
 */
u32 XV_vscaler_IsReady(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/**
 * XV_vscaler_EnableAutoRestart - Enable auto-restart for the v_vscaler hardware.
 *
 * This function sets the auto-restart bit (bit 7) in the control register,
 * enabling the hardware to automatically restart after completing its current
 * operation. It asserts that the InstancePtr is not NULL and that the driver is
 * ready before accessing the hardware registers.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return None.
 */
void XV_vscaler_EnableAutoRestart(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * XV_vscaler_DisableAutoRestart - Disables the auto-restart feature of the V-Scaler
 * hardware instance. This function asserts that the provided instance pointer is not
 * NULL and that the instance is ready before writing to the control register to
 * disable auto-restart. After calling this function, the V-Scaler hardware will not
 * automatically restart its operation after completing a task, and must be manually
 * started if further processing is required.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance to operate on. Must not be
 * NULL and must point to an initialized and ready XV_vscaler structure.
 * @return None.
 */

void XV_vscaler_DisableAutoRestart(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_AP_CTRL, 0);
}

/**
 * XV_vscaler_Set_HwReg_HeightIn - Sets the input height hardware register for the
 * XV_vscaler instance.
 *
 * This function writes the specified input height value to the hardware register
 * associated with the XV_vscaler instance. It first asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @param Data:        The input height value to be written to the hardware
 *                     register.
 * @return None.
 */
void XV_vscaler_Set_HwReg_HeightIn(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA, Data);
}

/**
 * XV_vscaler_Get_HwReg_HeightIn - Retrieves the input height value from the
 * hardware register for the XV_vscaler instance.
 *
 * This function reads the value of the input height hardware register
 * associated with the XV_vscaler instance. It asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register read operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return The value of the input height hardware register.
 */
u32 XV_vscaler_Get_HwReg_HeightIn(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTIN_DATA);
    return Data;
}


/**
 * XV_vscaler_Set_HwReg_Width - Sets the hardware register width for the
 * XV_vscaler instance.
 *
 * This function writes the specified width value to the hardware register
 * associated with the XV_vscaler instance. It asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register write operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and
 *                     must be initialized.
 * @param Data:        The width value to be written to the hardware register.
 * @return None.
 */
void XV_vscaler_Set_HwReg_Width(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}


/**
 * XV_vscaler_Get_HwReg_Width - Retrieves the width value from the hardware
 * register for the XV_vscaler instance.
 *
 * This function reads the value of the width hardware register associated with
 * the XV_vscaler instance. It asserts that the instance pointer is not NULL and
 * that the instance is ready before performing the register read operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return The value of the width hardware register.
 */
u32 XV_vscaler_Get_HwReg_Width(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/**
 * XV_vscaler_Set_HwReg_HeightOut - Sets the output height hardware register for
 * the XV_vscaler instance.
 *
 * This function writes the specified output height value to the hardware
 * register associated with the XV_vscaler instance. It asserts that the
 * instance pointer is not NULL and that the instance is ready before
 * performing the register write operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @param Data:        The output height value to be written to the hardware
 *                     register.
 * @return None.
 */
void XV_vscaler_Set_HwReg_HeightOut(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA, Data);
}

/**
 * XV_vscaler_Get_HwReg_HeightOut - Retrieves the output height value from the
 * hardware register for the XV_vscaler instance.
 *
 * This function reads the value of the output height hardware register
 * associated with the XV_vscaler instance. It asserts that the instance
 * pointer is not NULL and that the instance is ready before performing the
 * register read operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return The value of the output height hardware register.
 */
u32 XV_vscaler_Get_HwReg_HeightOut(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_HWREG_HEIGHTOUT_DATA);
    return Data;
}

/**
 * XV_vscaler_Set_HwReg_LineRate - Sets the line rate hardware register for the
 * XV_vscaler instance.
 *
 * This function writes the specified line rate value to the hardware register
 * associated with the XV_vscaler instance. It asserts that the instance pointer
 * is not NULL and that the instance is ready before performing the register
 * write operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @param Data:        The line rate value to be written to the hardware
 *                     register.
 * @return None.
 */
void XV_vscaler_Set_HwReg_LineRate(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA, Data);
}

/**
 * XV_vscaler_Get_HwReg_LineRate - Retrieves the line rate value from the hardware
 * register for the XV_vscaler instance.
 *
 * This function reads the value of the line rate hardware register associated
 * with the XV_vscaler instance. It asserts that the instance pointer is not
 * NULL and that the instance is ready before performing the register read
 * operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance.
 * @return The value of the line rate hardware register.
 */
u32 XV_vscaler_Get_HwReg_LineRate(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_VSCALER_CTRL_ADDR_HWREG_LINERATE_DATA);
    return Data;
}

/**
 * XV_vscaler_Set_HwReg_ColorMode - Sets the hardware register for color mode in the
 * XV_vscaler instance. This function writes the specified color mode data to the
 * hardware register associated with the XV_vscaler instance. It first asserts that
 * the instance pointer is not NULL and that the instance is ready before performing
 * the register write operation.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and must
 *                     be initialized and ready.
 * @param Data:        The color mode data to be written to the hardware register.
 *
 * @return None.
 */
void XV_vscaler_Set_HwReg_ColorMode(XV_vscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA, Data);
}
/**
 * Retrieves the current color mode hardware register value from the V-Scaler
 * instance. This function reads the value of the color mode register from the
 * hardware using the base address specified in the instance configuration.
 *
 * @param    InstancePtr is a pointer to the XV_vscaler instance.
 *           It must be initialized and ready before calling this function.
 *
 * @return   The value of the color mode hardware register.
 *
 * @note     The function asserts that the instance pointer is not NULL and that
 *           the instance is ready before accessing the hardware register.
 */

u32 XV_vscaler_Get_HwReg_ColorMode(XV_vscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA);
    return Data;
}
/**
 * XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress - Get the base address of the
 * vertical filter coefficient hardware register for the XV_vscaler instance.
 *
 * This function returns the base address of the vertical filter coefficient
 * hardware register by adding the base address from the instance configuration
 * to the defined offset for the vertical filter coefficient register.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and
 *                     must be initialized and ready.
 * @return The base address of the vertical filter coefficient hardware register.
 */
UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_BaseAddress(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE);
}

/**
 * XV_vscaler_Get_HwReg_vfltCoeff_HighAddress - Get the high address of the
 * vertical filter coefficient hardware register for the XV_vscaler instance.
 *
 * This function returns the high address of the vertical filter coefficient
 * hardware register by adding the base address from the instance configuration
 * to the defined high offset for the vertical filter coefficient register.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and
 *                     must be initialized and ready.
 * @return The high address of the vertical filter coefficient hardware register.
 */
UINTPTR XV_vscaler_Get_HwReg_vfltCoeff_HighAddress(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH);
}
/**
 * Retrieves the total number of bytes for the vertical filter coefficient
 * hardware register in the XV_vscaler instance. This function calculates the
 * total byte size by subtracting the base address of the HWREG_VFLTCOEFF
 * register from its high address and adding one, which gives the total
 * addressable bytes for the coefficient register block.
 *
 * @param    InstancePtr is a pointer to the XV_vscaler instance.
 *           It must be a valid pointer and the instance must be ready.
 *
 * @return   The total number of bytes allocated for the HWREG_VFLTCOEFF
 *           register block.
 */

u32 XV_vscaler_Get_HwReg_vfltCoeff_TotalBytes(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1);
}
/**
 * XV_vscaler_Get_HwReg_vfltCoeff_BitWidth - Retrieves the bit width of the
 * vertical filter coefficient hardware register for the XV_vscaler instance.
 *
 * This function returns the bit width of the HWREG_VFLTCOEFF register as
 * defined by the XV_VSCALER_CTRL_WIDTH_HWREG_VFLTCOEFF macro. It asserts
 * that the instance pointer is not NULL and that the instance is ready
 * before returning the bit width.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL
 *                     and must be initialized and ready.
 * @return The bit width of the vertical filter coefficient hardware register.
 */
u32 XV_vscaler_Get_HwReg_vfltCoeff_BitWidth(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_VSCALER_CTRL_WIDTH_HWREG_VFLTCOEFF;
}
/**
 * XV_vscaler_Get_HwReg_vfltCoeff_Depth - Retrieves the depth of the vertical
 * filter coefficient hardware register for the XV_vscaler instance.
 *
 * This function returns the depth of the HWREG_VFLTCOEFF register as defined
 * by the XV_VSCALER_CTRL_DEPTH_HWREG_VFLTCOEFF macro. It asserts that the
 * instance pointer is not NULL and that the instance is ready before returning
 * the depth value.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and
 *                     must be initialized and ready.
 * @return The depth of the vertical filter coefficient hardware register.
 */
u32 XV_vscaler_Get_HwReg_vfltCoeff_Depth(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_VSCALER_CTRL_DEPTH_HWREG_VFLTCOEFF;
}
/**
 * XV_vscaler_Write_HwReg_vfltCoeff_Words - Writes an array of 32-bit words to the
 * vertical filter coefficient hardware register of the XV_vscaler instance.
 *
 * This function writes the specified number of 32-bit words from the provided data
 * array to the HWREG_VFLTCOEFF register block, starting at the given offset. It
 * first checks that the instance pointer is valid and that the instance is ready.
 * The function also ensures that the write operation does not exceed the bounds of
 * the register block. If the operation is valid, it writes each word sequentially.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and must
 *                     be initialized and ready.
 * @param offset:      Offset (in words) from the base of the HWREG_VFLTCOEFF
 *                     register block where writing begins.
 * @param data:        Pointer to the array of 32-bit words to write.
 * @param length:      Number of 32-bit words to write.
 * @return             The number of words written if successful, or 0 if the
 *                     operation would exceed the register block bounds.
 */
u32 XV_vscaler_Write_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}
/**
 * XV_vscaler_Read_HwReg_vfltCoeff_Words - Reads an array of 32-bit words from
 * the vertical filter coefficient hardware register of the XV_vscaler instance.
 *
 * This function reads the specified number of 32-bit words from the
 * HWREG_VFLTCOEFF register block, starting at the given offset, and stores
 * them in the provided data array. It first checks that the instance pointer
 * is valid and that the instance is ready. The function also ensures that the
 * read operation does not exceed the bounds of the register block. If the
 * operation is valid, it reads each word sequentially.
 *
 * @param InstancePtr: Pointer to the XV_vscaler instance. Must not be NULL and
 *                     must be initialized and ready.
 * @param offset:      Offset (in words) from the base of the HWREG_VFLTCOEFF
 *                     register block where reading begins.
 * @param data:        Pointer to the array where the read 32-bit words will be
 *                     stored.
 * @param length:      Number of 32-bit words to read.
 * @return             The number of words read if successful, or 0 if the
 *                     operation would exceed the register block bounds.
 */
u32 XV_vscaler_Read_HwReg_vfltCoeff_Words(XV_vscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * This Function Writes a sequence of bytes to the HWREG_VFLTCOEFF register of the
 * XV_vscaler hardware instance.
 *
 * This function writes 'length' bytes from the buffer pointed to by 'data'
 * into the hardware register starting at the specified 'offset'. It ensures
 * that the write operation does not exceed the register's address range.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance.
 * @param offset      Offset from the base address of HWREG_VFLTCOEFF where
 *                    writing starts.
 * @param data        Pointer to the data buffer containing bytes to write.
 * @param length      Number of bytes to write from the data buffer.
 *
 * @return Number of bytes written on success, or 0 if the operation would
 *         exceed the register's address range.
 */

u32 XV_vscaler_Write_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + offset + i) = *(data + i);
    }
    return length;
}



/**
 * XV_vscaler_Read_HwReg_vfltCoeff_Bytes - Reads a sequence of bytes from the
 * HWREG_VFLTCOEFF register of the XV_vscaler hardware instance.
 *
 * This function reads 'length' bytes from the hardware register starting at the
 * specified 'offset' and stores them in the buffer pointed to by 'data'. It
 * ensures that the read operation does not exceed the register's address range.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance. Must not be NULL and
 *                    must be initialized and ready.
 * @param offset      Offset from the base address of HWREG_VFLTCOEFF where
 *                    reading starts.
 * @param data        Pointer to the data buffer where read bytes will be stored.
 * @param length      Number of bytes to read from the hardware register.
 *
 * @return Number of bytes read on success, or 0 if the operation would exceed
 *         the register's address range.
 */
u32 XV_vscaler_Read_HwReg_vfltCoeff_Bytes(XV_vscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr -> IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_HIGH - XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_VSCALER_CTRL_ADDR_HWREG_VFLTCOEFF_BASE + offset + i);
    }
    return length;
}


/**
 * Enables the global interrupt for the XV_vscaler instance.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * instance is ready. It then writes to the Global Interrupt Enable (GIE)
 * register to enable global interrupts for the scaler hardware.
 *
 * @param    InstancePtr is a pointer to the XV_vscaler instance for which the
 *          global interrupt should be enabled. The instance must be initialized
 *          and ready before calling this function.
 * @return  None.
 */

void XV_vscaler_InterruptGlobalEnable(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_GIE, 1);
}
/**
 * XV_vscaler_InterruptGlobalDisable - Disables the global interrupt for the
 * XV_vscaler instance.
 *
 * This function asserts that the provided InstancePtr is not NULL and that the
 * instance is ready. It then writes to the Global Interrupt Enable (GIE)
 * register to disable global interrupts for the scaler hardware.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance for which the global
 *        interrupt should be disabled. The instance must be initialized and
 *        ready before calling this function.
 * @return None.
 */
void XV_vscaler_InterruptGlobalDisable(XV_vscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_GIE, 0);
}

/**
 * XV_vscaler_InterruptEnable - Enables specific interrupts for the XV_vscaler
 * instance.
 *
 * This function enables the interrupts specified by the Mask parameter for the
 * XV_vscaler hardware instance. It first asserts that the provided instance
 * pointer is not NULL and that the instance is ready. The function reads the
 * current interrupt enable register, sets the bits specified by Mask, and
 * writes the updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance. Must not be NULL and
 *        must be initialized and ready.
 * @param Mask        Bitmask specifying which interrupts to enable.
 * @return None.
 */
void XV_vscaler_InterruptEnable(XV_vscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register = XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                                  XV_VSCALER_CTRL_ADDR_IER);
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_VSCALER_CTRL_ADDR_IER, Register | Mask);
}

/**
 * XV_vscaler_InterruptDisable - Disables specific interrupts for the XV_vscaler
 * instance.
 *
 * This function disables the interrupts specified by the Mask parameter for the
 * XV_vscaler hardware instance. It first asserts that the provided instance
 * pointer is not NULL and that the instance is ready. The function reads the
 * current interrupt enable register, clears the bits specified by Mask, and
 * writes the updated value back to the register.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance. Must not be NULL and
 *        must be initialized and ready.
 * @param Mask        Bitmask specifying which interrupts to disable.
 * @return None.
 */
void XV_vscaler_InterruptDisable(XV_vscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress,
                                   XV_VSCALER_CTRL_ADDR_IER);
    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress,
                       XV_VSCALER_CTRL_ADDR_IER, Register & (~Mask));
}
/**
 * XV_vscaler_InterruptClear - Clears specific interrupts for the XV_vscaler
 * instance.
 *
 * This function clears the interrupts specified by the Mask parameter for the
 * XV_vscaler hardware instance. It asserts that the provided instance pointer
 * is not NULL and that the instance is ready before writing to the interrupt
 * status register to clear the specified interrupts.
 *
 * @param InstancePtr Pointer to the XV_vscaler instance. Must not be NULL and
 *        must be initialized and ready.
 * @param Mask        Bitmask specifying which interrupts to clear.
 * @return None.
 */
void XV_vscaler_InterruptClear(XV_vscaler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_vscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_ISR, Mask);
}

/**
 * XV_vscaler_InterruptGetEnabled - Retrieves the interrupt enable register value
 * for the XV_vscaler instance. This function checks that the provided instance
 * pointer is not NULL and that the instance is ready before reading the
 * interrupt enable register from the hardware. The value returned indicates
 * which interrupts are currently enabled for the scaler hardware.
 *
 * @param    InstancePtr is a pointer to the XV_vscaler instance.
 *
 * @return   The contents of the interrupt enable register, indicating the
 *           enabled interrupts.
 */

u32 XV_vscaler_InterruptGetEnabled(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_IER);
}
/**
 * XV_vscaler_InterruptGetStatus - Retrieves the interrupt status for the given
 * XV_vscaler instance. This function asserts that the provided instance pointer
 * is not NULL and that the instance is ready before reading the interrupt
 * status register from the hardware. The status is read from the ISR (Interrupt
 * Status Register) at the base address specified in the configuration.
 *
 * @param    InstancePtr is a pointer to the XV_vscaler instance.
 *
 * @return   The current value of the interrupt status register.
 */

u32 XV_vscaler_InterruptGetStatus(XV_vscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_vscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_VSCALER_CTRL_ADDR_ISR);
}
