// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_hscaler.c
 * @addtogroup v_hscaler Overview
 */

/***************************** Include Files *********************************/
#include "xv_hscaler.h"

/************************** Function Implementation *************************/
#ifndef __linux__

/*****************************************************************************/
/**
*
* XV_hscaler_CfgInitialize - Initializes an XV_hscaler instance.
*
* @param    InstancePtr is a pointer to the XV_hscaler instance to be worked on.
* @param    ConfigPtr points to the configuration structure.
* @param    EffectiveAddr is the base address of the device.
*
* @return
*           - XST_SUCCESS if initialization was successful.
*
* @note     None.
*
******************************************************************************/

/**
 * XV_hscaler_CfgInitialize - Initialize an XV_hscaler instance.
 *
 * This function initializes an XV_hscaler instance using the provided
 * configuration structure and effective base address. It copies the configuration,
 * sets the base address, and marks the instance as ready for use.
 *
 * @param InstancePtr   Pointer to the XV_hscaler instance to be initialized.
 * @param ConfigPtr     Pointer to the configuration structure for the device.
 * @param EffectiveAddr Physical base address of the device.
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *
 * Preconditions:
 *   - InstancePtr must not be NULL.
 *   - ConfigPtr must not be NULL.
 *   - EffectiveAddr must not be NULL.
 */

int XV_hscaler_CfgInitialize(XV_hscaler *InstancePtr,
                             XV_hscaler_Config *ConfigPtr,
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
 * XV_hscaler_Start - Start the HScaler hardware.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * @return None
 * This function sets the ap_start bit in the control register to
 * start the hardware. It preserves the auto-restart bit.
 *
 */
void XV_hscaler_Start(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read control register and keep auto-restart bit (bit 7)
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_AP_CTRL) & 0x80;
    // Set ap_start (bit 0) to start the hardware
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/**
 * XV_hscaler_IsDone - Check if the HScaler operation is done.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the control register and checks the ap_done
 * bit (bit 1) to determine if the hardware has finished processing.
 *
 * @return   1 if done, 0 otherwise.
 */
u32 XV_hscaler_IsDone(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read control register and check ap_done (bit 1)
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}


/**
 * XV_hscaler_IsIdle - Check if the HScaler hardware is idle.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the control register and checks the ap_idle
 * bit (bit 2) to determine if the hardware is idle.
 *
 * @return   1 if idle, 0 otherwise.
 */
u32 XV_hscaler_IsIdle(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read control register and check ap_idle (bit 2)
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}


/**
 * XV_hscaler_IsReady - Check if HScaler is ready for next input.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the control register and checks the ap_start
 * bit (bit 0) to determine if the hardware is ready for the next
 * input. Returns 1 if ready, 0 otherwise.
 *
 * @return   1 if ready, 0 otherwise.
 */
u32 XV_hscaler_IsReady(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read control register
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_AP_CTRL);
    // Check ap_start (bit 0); return 1 if not set (ready)
    return !(Data & 0x1);
}

/**
 * XV_hscaler_EnableAutoRestart - Enable auto-restart mode.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * @return None
 *
 * This function sets the auto-restart bit (bit 7) in the control
 * register, enabling the hardware to automatically restart after
 * completing an operation.
 */
void XV_hscaler_EnableAutoRestart(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Set auto-restart bit (bit 7) in control register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_AP_CTRL, 0x80);
}

/**
 * XV_hscaler_DisableAutoRestart - Disable auto-restart mode.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * @return None
 *
 * This function clears the auto-restart bit (bit 7) in the control
 * register, disabling the hardware from automatically restarting
 * after completing an operation.
 */
void XV_hscaler_DisableAutoRestart(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Clear auto-restart bit (bit 7) in control register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_AP_CTRL, 0);
}


/**
 * XV_hscaler_Set_HwReg_Height - Set the HWREG_HEIGHT register.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 * @param    Data is the value to write to HWREG_HEIGHT register.
 *
 * @return None
 * This function writes the specified value to the HWREG_HEIGHT
 * register of the HScaler hardware.
 */
void XV_hscaler_Set_HwReg_Height(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Write Data to HWREG_HEIGHT register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}


/**
 * XV_hscaler_Get_HwReg_Height - Get HWREG_HEIGHT register value.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the HWREG_HEIGHT register from the HScaler
 * hardware and returns its value.
 *
 * @return   Value of HWREG_HEIGHT register.
 */
u32 XV_hscaler_Get_HwReg_Height(XV_hscaler *InstancePtr) {
    u32 Data;

    // Check that the instance pointer is not NULL
    Xil_AssertNonvoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read HWREG_HEIGHT register value
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}


/**
 * XV_hscaler_Set_HwReg_WidthIn - Set HWREG_WIDTHIN register.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 * @param    Data is the value to write to HWREG_WIDTHIN register.
 *
 * @return None
 * This function writes the specified value to the HWREG_WIDTHIN
 * register of the HScaler hardware.
 */
void XV_hscaler_Set_HwReg_WidthIn(XV_hscaler *InstancePtr, u32 Data) {
    // Check that the instance pointer is not NULL
    Xil_AssertVoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Write Data to HWREG_WIDTHIN register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA, Data);
}

/**
 * XV_hscaler_Get_HwReg_WidthIn - Get HWREG_WIDTHIN register value.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the HWREG_WIDTHIN register from the HScaler
 * hardware and returns its value.
 *
 * @return   Value of HWREG_WIDTHIN register.
 */
u32 XV_hscaler_Get_HwReg_WidthIn(XV_hscaler *InstancePtr) {
    u32 Data;

    // Check that the instance pointer is not NULL
    Xil_AssertNonvoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read HWREG_WIDTHIN register value
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_HWREG_WIDTHIN_DATA);
    return Data;
}


/**
 * XV_hscaler_Set_HwReg_WidthOut - Set HWREG_WIDTHOUT register.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 * @param    Data is the value to write to HWREG_WIDTHOUT register.
 *
 * @return   None
 * This function writes the specified value to the HWREG_WIDTHOUT
 * register of the HScaler hardware.
 */
void XV_hscaler_Set_HwReg_WidthOut(XV_hscaler *InstancePtr, u32 Data) {
    // Check that the instance pointer is not NULL
    Xil_AssertVoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Write Data to HWREG_WIDTHOUT register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA, Data);
}


/**
 * XV_hscaler_Get_HwReg_WidthOut - Get HWREG_WIDTHOUT register value.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the HWREG_WIDTHOUT register from the HScaler
 * hardware and returns its value.
 *
 * @return   Value of HWREG_WIDTHOUT register.
 */
u32 XV_hscaler_Get_HwReg_WidthOut(XV_hscaler *InstancePtr) {
    u32 Data;

    // Check that the instance pointer is not NULL
    Xil_AssertNonvoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read HWREG_WIDTHOUT register value
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_HWREG_WIDTHOUT_DATA);
    return Data;
}


/**
 * XV_hscaler_Set_HwReg_ColorMode - Set HWREG_COLORMODE register.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 * @param    Data is the value to write to HWREG_COLORMODE register.
 *
 * @return   None
 * This function writes the specified value to the HWREG_COLORMODE
 * register of the HScaler hardware.
 */
void XV_hscaler_Set_HwReg_ColorMode(XV_hscaler *InstancePtr, u32 Data) {
    // Check that the instance pointer is not NULL
    Xil_AssertVoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Write Data to HWREG_COLORMODE register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA, Data);
}


/**
 * XV_hscaler_Get_HwReg_ColorMode - Get HWREG_COLORMODE register value.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the HWREG_COLORMODE register from the HScaler
 * hardware and returns its value.
 *
 * @return   Value of HWREG_COLORMODE register.
 */
u32 XV_hscaler_Get_HwReg_ColorMode(XV_hscaler *InstancePtr) {
    u32 Data;

    // Check that the instance pointer is not NULL
    Xil_AssertNonvoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read HWREG_COLORMODE register value
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_HWREG_COLORMODE_DATA);
    return Data;
}


/**
 * XV_hscaler_Set_HwReg_ColorModeOut - Set HWREG_COLORMODEOUT reg.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 * @param    Data is the value to write to HWREG_COLORMODEOUT reg.
 *
 * @return   None
 * This function writes the specified value to the HWREG_COLORMODEOUT
 * register of the HScaler hardware.
 */
void XV_hscaler_Set_HwReg_ColorModeOut(XV_hscaler *InstancePtr, u32 Data) {
    // Check that the instance pointer is not NULL
    Xil_AssertVoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Write Data to HWREG_COLORMODEOUT register
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress,
                        XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA, Data);
}

/**
 * XV_hscaler_Get_HwReg_ColorModeOut - Get HWREG_COLORMODEOUT reg value.
 *
 * @param    InstancePtr is a pointer to the XV_hscaler instance.
 *
 * This function reads the HWREG_COLORMODEOUT register from the
 * HScaler hardware and returns its value.
 *
 * @return   Value of HWREG_COLORMODEOUT register.
 */
u32 XV_hscaler_Get_HwReg_ColorModeOut(XV_hscaler *InstancePtr) {
    u32 Data;

    // Check that the instance pointer is not NULL
    Xil_AssertNonvoid(InstancePtr != NULL);
    // Check that the instance is ready
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    // Read HWREG_COLORMODEOUT register value
    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress,
                              XV_HSCALER_CTRL_ADDR_HWREG_COLORMODEOUT_DATA);
    return Data;
}

/**
 * This function sets the hardware register pixel rate.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param Data The value to set in the hardware register pixel rate.
 * @return None
 */
void XV_hscaler_Set_HwReg_PixelRate(XV_hscaler *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA, Data);
}

/**
 * This function gets the hardware register pixel rate.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The value of the hardware register pixel rate.
 */
u32 XV_hscaler_Get_HwReg_PixelRate(XV_hscaler *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_HWREG_PIXELRATE_DATA);
    return Data;
}

/**
 * This function gets the base address of the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The base address of the hardware register hfltCoeff.
 */
UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_BaseAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE);
}

/**
 * This function gets the high address of the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The high address of the hardware register hfltCoeff.
 */
UINTPTR XV_hscaler_Get_HwReg_hfltCoeff_HighAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH);
}

/**
 * This function gets the total bytes of the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The total bytes of the hardware register hfltCoeff.
 */
u32 XV_hscaler_Get_HwReg_hfltCoeff_TotalBytes(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1);
}

/**
 * This function gets the bit width of the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The bit width of the hardware register hfltCoeff.
 */
u32 XV_hscaler_Get_HwReg_hfltCoeff_BitWidth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_WIDTH_HWREG_HFLTCOEFF;
}

/**
 * This function gets the depth of the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The depth of the hardware register hfltCoeff.
 */
u32 XV_hscaler_Get_HwReg_hfltCoeff_Depth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_DEPTH_HWREG_HFLTCOEFF;
}

/**
 * This function writes words to the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start writing.
 * @param data Pointer to the data to write.
 * @param length The number of words to write.
 * @return The number of words written.
 */
u32 XV_hscaler_Write_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * This function reads words from the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start reading.
 * @param data Pointer to the buffer to store the read data.
 * @param length The number of words to read.
 * @return The number of words read.
 */
u32 XV_hscaler_Read_HwReg_hfltCoeff_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * This function writes bytes to the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start writing.
 * @param data Pointer to the data to write.
 * @param length The number of bytes to write.
 * @return The number of bytes written.
 */
u32 XV_hscaler_Write_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * This function reads bytes from the hardware register hfltCoeff.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start reading.
 * @param data Pointer to the buffer to store the read data.
 * @param length The number of bytes to read.
 * @return The number of bytes read.
 */
u32 XV_hscaler_Read_HwReg_hfltCoeff_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_HFLTCOEFF_BASE + offset + i);
    }
    return length;
}

/** This function gets the base address of the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The base address of the hardware register phasesH_V.
 */
UINTPTR XV_hscaler_Get_HwReg_phasesH_V_BaseAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE);
}

/**
 * function gets the high address of the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The high address of the hardware register phasesH_V.
 */
UINTPTR XV_hscaler_Get_HwReg_phasesH_V_HighAddress(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH);
}

/**
 * This function gets the total bytes of the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The total bytes of the hardware register phasesH_V.
 */
u32 XV_hscaler_Get_HwReg_phasesH_V_TotalBytes(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1);
}

/**
 * This function gets the bit width of the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The bit width of the hardware register phasesH_V.
 */
u32 XV_hscaler_Get_HwReg_phasesH_V_BitWidth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_WIDTH_HWREG_PHASESH_V;
}

/**
 * This function gets the depth of the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The depth of the hardware register phasesH_V.
 */
u32 XV_hscaler_Get_HwReg_phasesH_V_Depth(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_HSCALER_CTRL_DEPTH_HWREG_PHASESH_V;
}

/**
 * This function writes words to the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start writing.
 * @param data Pointer to the data to write.
 * @param length The number of words to write.
 * @return The number of words written.
 */
u32 XV_hscaler_Write_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + (offset + i)*4) = *(data + i);
    }
    return length;
}

/**
 * This function reads words from the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start reading.
 * @param data Pointer to the buffer to store the read data.
 * @param length The number of words to read.
 * @return The number of words read.
 */
u32 XV_hscaler_Read_HwReg_phasesH_V_Words(XV_hscaler *InstancePtr, int offset, int *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length)*4 > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(int *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + (offset + i)*4);
    }
    return length;
}

/**
 * This function writes bytes to the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start writing.
 * @param data Pointer to the data to write.
 * @param length The number of bytes to write.
 * @return The number of bytes written.
 */
u32 XV_hscaler_Write_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + offset + i) = *(data + i);
    }
    return length;
}

/**
 * This function reads bytes from the hardware register phasesH_V.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param offset The offset to start reading.
 * @param data Pointer to the buffer to store the read data.
 * @param length The number of bytes to read.
 * @return The number of bytes read.
 */
u32 XV_hscaler_Read_HwReg_phasesH_V_Bytes(XV_hscaler *InstancePtr, int offset, char *data, int length) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    int i;

    if ((offset + length) > (XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_HIGH - XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + 1))
        return 0;

    for (i = 0; i < length; i++) {
        *(data + i) = *(char *)(InstancePtr->Config.BaseAddress + XV_HSCALER_CTRL_ADDR_HWREG_PHASESH_V_BASE + offset + i);
    }
    return length;
}

/**
 * This function enables global interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return None
 */
void XV_hscaler_InterruptGlobalEnable(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_GIE, 1);
}

/**
 * This function disables global interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return None
 */
void XV_hscaler_InterruptGlobalDisable(XV_hscaler *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_GIE, 0);
}

/**
 * This function enables specific interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param Mask The mask of interrupts to enable.
 * @return None
 */
void XV_hscaler_InterruptEnable(XV_hscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER, Register | Mask);
}

/**
 * This function disables specific interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param Mask The mask of interrupts to disable.
 * @return None
 */
void XV_hscaler_InterruptDisable(XV_hscaler *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register = XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER, Register & (~Mask));
}

/**
 * This function clears specific interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @param Mask The mask of interrupts to clear.
 * @return None
 */
void XV_hscaler_InterruptClear(XV_hscaler *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_hscaler_WriteReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_ISR, Mask);
}

/**
 * This function gets the enabled interrupts.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The mask of enabled interrupts.
 */
u32 XV_hscaler_InterruptGetEnabled(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_IER);
}

/**
 * This function gets the interrupt status.
 *
 * @param InstancePtr Pointer to the XV_hscaler instance.
 * @return The mask of interrupt status.
 */
u32 XV_hscaler_InterruptGetStatus(XV_hscaler *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_hscaler_ReadReg(InstancePtr->Config.BaseAddress, XV_HSCALER_CTRL_ADDR_ISR);
}
