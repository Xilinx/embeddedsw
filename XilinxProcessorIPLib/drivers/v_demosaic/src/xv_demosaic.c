// ==============================================================
// Copyright (c) 1986 - 2021 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_demosaic.c
 * @addtogroup v_demosaic Overview
 */

/***************************** Include Files *********************************/
#include "xv_demosaic.h"

/************************** Function Definitions *****************************/

#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initializes the Demosaic IP instance.
 *
 * This function initializes the Demosaic instance by copying the configuration
 * data and setting the effective base address for register access. It marks
 * the instance as ready after successful initialization.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance to initialize.
 * @param ConfigPtr Pointer to the configuration structure containing IP parameters.
 * @param EffectiveAddr The effective base address for the IP registers.
 *
 * @return XST_SUCCESS on successful initialization.
 *
 * @note This function is used in bare-metal and RTOS environments (not Linux).
 *
 *******************************************************************************/
int XV_demosaic_CfgInitialize(XV_demosaic *InstancePtr,
                               XV_demosaic_Config *ConfigPtr,
                               UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    InstancePtr->Config = *ConfigPtr;
    InstancePtr->Config.BaseAddress = EffectiveAddr;

    /* Set the flag to indicate the driver is ready */
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Starts the Demosaic IP core.
 *
 * This function triggers the Demosaic IP to start processing by setting the
 * AP_START bit in the control register while preserving the auto-restart bit.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return None
 *
 * @note The instance must be initialized and ready before calling this function.
 *
 *******************************************************************************/
void XV_demosaic_Start(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Checks if the Demosaic IP has completed processing.
 *
 * This function reads the control register and checks the AP_DONE bit to
 * determine if the IP core has finished processing the current frame.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return 1 if processing is done, 0 otherwise.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_IsDone(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Demosaic IP is idle.
 *
 * This function reads the control register and checks the AP_IDLE bit to
 * determine if the IP core is in idle state (not processing).
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return 1 if the IP is idle, 0 otherwise.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_IsIdle(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Demosaic IP is ready for new input.
 *
 * This function checks the AP_START bit to determine if the IP core is
 * ready to accept a new frame for processing. The IP is ready when the
 * AP_START bit is not set.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return 1 if the IP is ready for next input, 0 otherwise.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_IsReady(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enables auto-restart mode for the Demosaic IP.
 *
 * This function enables the auto-restart feature, which causes the IP to
 * automatically restart processing after completing each frame without
 * requiring explicit start commands.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return None
 *
 * @note Auto-restart is useful for continuous video stream processing.
 *
 *******************************************************************************/
void XV_demosaic_EnableAutoRestart(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disables auto-restart mode for the Demosaic IP.
 *
 * This function disables the auto-restart feature. The IP will stop after
 * completing each frame and require an explicit start command for the next
 * frame.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_demosaic_DisableAutoRestart(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Sets the frame width for the Demosaic IP.
 *
 * This function configures the active image width in pixels that the
 * Demosaic IP will process.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Data The frame width in pixels.
 *
 * @return None
 *
 * @note The width must not exceed the MAX_COLS configured in hardware.
 *
 *******************************************************************************/
void XV_demosaic_Set_HwReg_width(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_WIDTH_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the configured frame width from the Demosaic IP.
 *
 * This function reads the current frame width configuration in pixels
 * from the Demosaic IP hardware register.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return The configured frame width in pixels.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_Get_HwReg_width(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_WIDTH_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the frame height for the Demosaic IP.
 *
 * This function configures the active image height in lines that the
 * Demosaic IP will process.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Data The frame height in lines.
 *
 * @return None
 *
 * @note The height must not exceed the MAX_ROWS configured in hardware.
 *
 *******************************************************************************/
void XV_demosaic_Set_HwReg_height(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_HEIGHT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the configured frame height from the Demosaic IP.
 *
 * This function reads the current frame height configuration in lines
 * from the Demosaic IP hardware register.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return The configured frame height in lines.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_Get_HwReg_height(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_HEIGHT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the Bayer pattern phase for the Demosaic IP.
 *
 * This function configures the starting phase of the Bayer color filter
 * array pattern. The phase determines which color (R, G, or B) is at the
 * top-left pixel position.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Data The Bayer phase value (0-3 representing different RGGB patterns).
 *
 * @return None
 *
 * @note Valid Bayer phases are typically: 0=RGGB, 1=GRBG, 2=GBRG, 3=BGGR.
 *
 *******************************************************************************/
void XV_demosaic_Set_HwReg_bayer_phase(XV_demosaic *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_BAYER_PHASE_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the configured Bayer pattern phase from the Demosaic IP.
 *
 * This function reads the current Bayer phase configuration from the
 * Demosaic IP hardware register.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return The configured Bayer phase value (0-3).
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_Get_HwReg_bayer_phase(XV_demosaic *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_HWREG_BAYER_PHASE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enables global interrupts for the Demosaic IP.
 *
 * This function sets the Global Interrupt Enable (GIE) bit, allowing
 * the IP core to generate interrupt signals to the processor.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return None
 *
 * @note Individual interrupt sources must also be enabled using
 *       XV_demosaic_InterruptEnable().
 *
 *******************************************************************************/
void XV_demosaic_InterruptGlobalEnable(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disables global interrupts for the Demosaic IP.
 *
 * This function clears the Global Interrupt Enable (GIE) bit, preventing
 * the IP core from generating interrupt signals to the processor.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return None
 *
 * @note This disables all interrupt generation regardless of individual
 *       interrupt enable settings.
 *
 *******************************************************************************/
void XV_demosaic_InterruptGlobalDisable(XV_demosaic *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enables specific interrupt sources for the Demosaic IP.
 *
 * This function sets bits in the Interrupt Enable Register (IER) to enable
 * specific interrupt sources. Multiple interrupts can be enabled by using
 * a bitwise OR of interrupt masks.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Mask Bit mask of interrupts to enable.
 *
 * @return None
 *
 * @note Global interrupts must also be enabled using
 *       XV_demosaic_InterruptGlobalEnable().
 *
 *******************************************************************************/
void XV_demosaic_InterruptEnable(XV_demosaic *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disables specific interrupt sources for the Demosaic IP.
 *
 * This function clears bits in the Interrupt Enable Register (IER) to disable
 * specific interrupt sources. Multiple interrupts can be disabled by using
 * a bitwise OR of interrupt masks.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Mask Bit mask of interrupts to disable.
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_demosaic_InterruptDisable(XV_demosaic *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clears pending interrupt flags for the Demosaic IP.
 *
 * This function clears the specified interrupt status bits by writing to
 * the Interrupt Status Register (ISR). This acknowledges the interrupt
 * and allows new interrupts of the same type to be generated.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 * @param Mask Bit mask of interrupt flags to clear.
 *
 * @return None
 *
 * @note This should be called in the interrupt handler after servicing
 *       the interrupt.
 *
 *******************************************************************************/
void XV_demosaic_InterruptClear(XV_demosaic *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_demosaic_WriteReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Gets the enabled interrupt sources for the Demosaic IP.
 *
 * This function reads the Interrupt Enable Register (IER) to determine
 * which interrupt sources are currently enabled.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return Bit mask of currently enabled interrupts.
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_demosaic_InterruptGetEnabled(XV_demosaic *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Gets the current interrupt status for the Demosaic IP.
 *
 * This function reads the Interrupt Status Register (ISR) to determine
 * which interrupts are currently pending/active.
 *
 * @param InstancePtr Pointer to the XV_demosaic instance.
 *
 * @return Bit mask of currently pending/active interrupts.
 *
 * @note The returned status should be checked and interrupts cleared using
 *       XV_demosaic_InterruptClear() in the interrupt handler.
 *
 *******************************************************************************/
u32 XV_demosaic_InterruptGetStatus(XV_demosaic *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_demosaic_ReadReg(InstancePtr->Config.BaseAddress, XV_DEMOSAIC_CTRL_ADDR_ISR);
}
