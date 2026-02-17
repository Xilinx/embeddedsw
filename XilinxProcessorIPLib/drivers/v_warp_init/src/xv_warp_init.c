// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_init.c
 * @addtogroup v_warp_init Overview
 */

/***************************** Include Files *********************************/
#include "xv_warp_init.h"


/************************** Function Definitions *****************************/
#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initialize the warp init instance with configuration
 *
 * This function initializes the XV_warp_init driver instance using the
 * provided configuration structure. Sets up base addresses and initializes
 * the descriptor configuration.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  ConfigPtr Pointer to the configuration structure
 *
 * @return XST_SUCCESS on successful initialization
 *
 * @note This function is only available in non-Linux builds. Both pointers
 *       must not be NULL.
 *
 ******************************************************************************/
int XV_warp_init_CfgInitialize(XV_warp_init *InstancePtr, XV_warp_init_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->config = ConfigPtr;
    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    InstancePtr->RemapVectorDesc_BaseAddr = 0;
    InstancePtr->NumDescriptors = 0;

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Start the warp init IP core
 *
 * This function starts the warp init IP core by setting the AP_START bit
 * in the control register while preserving the auto-restart configuration.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note The instance must be ready before calling this function
 *
 ******************************************************************************/
void XV_warp_init_Start(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Check if the warp init IP core has finished processing
 *
 * This function checks the AP_DONE bit in the control register to determine
 * if the IP core has completed its current operation.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return 1 if operation is done, 0 otherwise
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_IsDone(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if the warp init IP core is idle
 *
 * This function checks the AP_IDLE bit in the control register to determine
 * if the IP core is currently idle.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return 1 if idle, 0 if busy
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_IsIdle(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if the warp init IP core is ready for next input
 *
 * This function checks if the IP core is ready to accept new input by
 * verifying that the AP_START bit is not set.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return 1 if ready for next input, 0 otherwise
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_IsReady(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enable auto-restart mode for the warp init IP core
 *
 * This function enables auto-restart mode by setting bit 7 of the control
 * register, allowing the IP to automatically restart after completion.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note None
 *
 ******************************************************************************/
void XV_warp_init_EnableAutoRestart(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disable auto-restart mode for the warp init IP core
 *
 * This function disables auto-restart mode by clearing the control register,
 * requiring manual restart after each operation completes.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note None
 *
 ******************************************************************************/
void XV_warp_init_DisableAutoRestart(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Get the IP status register value
 *
 * This function reads and returns the IP status register which contains
 * various status flags from the warp init IP core.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return IP status register value
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_Get_ip_status(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IP_STATUS_REG_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the AXI master read/write address
 *
 * This function sets the 64-bit AXI master read/write base address for
 * memory access operations.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  Data 64-bit address value to set
 *
 * @return None
 *
 * @note Address is written as two 32-bit registers
 *
 ******************************************************************************/
void XV_warp_init_Set_maxi_read_write(XV_warp_init *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA, (u32)(Data));
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get the AXI master read/write address
 *
 * This function reads and returns the current 64-bit AXI master read/write
 * base address.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return Current 64-bit address value
 *
 * @note Address is read from two 32-bit registers
 *
 ******************************************************************************/
u64 XV_warp_init_Get_maxi_read_write(XV_warp_init *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA);
    Data += (u64)XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_MAXI_READ_WRITE_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the descriptor address
 *
 * This function sets the 64-bit descriptor base address for the warp
 * initialization descriptor data structure.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  Data 64-bit descriptor address to set
 *
 * @return None
 *
 * @note Address is written as two 32-bit registers
 *
 ******************************************************************************/
void XV_warp_init_Set_desc_addr(XV_warp_init *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA, (u32)(Data));
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Get the descriptor address
 *
 * This function reads and returns the current 64-bit descriptor base
 * address.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return Current 64-bit descriptor address
 *
 * @note Address is read from two 32-bit registers
 *
 ******************************************************************************/
u64 XV_warp_init_Get_desc_addr(XV_warp_init *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA);
    Data += (u64)XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_DESC_ADDR_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enable global interrupts
 *
 * This function enables the global interrupt output by setting the Global
 * Interrupt Enable (GIE) register.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note None
 *
 ******************************************************************************/
void XV_warp_init_InterruptGlobalEnable(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disable global interrupts
 *
 * This function disables the global interrupt output by clearing the Global
 * Interrupt Enable (GIE) register.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note None
 *
 ******************************************************************************/
void XV_warp_init_InterruptGlobalDisable(XV_warp_init *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enable specific interrupts
 *
 * This function enables the interrupts specified by the mask parameter by
 * setting the corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  Mask Interrupt mask with bits set for interrupts to enable
 *
 * @return None
 *
 * @note Multiple interrupts can be enabled simultaneously using OR'd mask
 *
 ******************************************************************************/
void XV_warp_init_InterruptEnable(XV_warp_init *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disable specific interrupts
 *
 * This function disables the interrupts specified by the mask parameter by
 * clearing the corresponding bits in the Interrupt Enable Register (IER).
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  Mask Interrupt mask with bits set for interrupts to disable
 *
 * @return None
 *
 * @note Multiple interrupts can be disabled simultaneously using OR'd mask
 *
 ******************************************************************************/
void XV_warp_init_InterruptDisable(XV_warp_init *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clear interrupt status flags
 *
 * This function clears the interrupt status flags specified by the mask
 * parameter by writing to the Interrupt Status Register (ISR).
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 * @param  Mask Interrupt mask with bits set for interrupts to clear
 *
 * @return None
 *
 * @note Write 1 to clear the corresponding interrupt status bit
 *
 ******************************************************************************/
void XV_warp_init_InterruptClear(XV_warp_init *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Get enabled interrupts
 *
 * This function reads and returns the Interrupt Enable Register (IER) value
 * showing which interrupts are currently enabled.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return Contents of the Interrupt Enable Register
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_InterruptGetEnabled(XV_warp_init *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Get interrupt status
 *
 * This function reads and returns the Interrupt Status Register (ISR) value
 * showing which interrupts are currently pending.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return Contents of the Interrupt Status Register
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_InterruptGetStatus(XV_warp_init *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress, XV_WARP_INIT_CTRL_ADDR_ISR);
}

/*****************************************************************************/
/**
 * @brief Set the flush bit to initiate IP core flush
 *
 * This function sets the flush bit in the control register to initiate
 * a flush operation, clearing any pending data in the IP core pipeline.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return None
 *
 * @note Check flush completion using XV_warp_init_Get_FlushDone()
 *
 ******************************************************************************/
void XV_warp_init_SetFlushbit(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL);
    Data |= AP_CTRL_FLUSH_BIT;
    XV_warp_init_WriteReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL, Data);
}

/*****************************************************************************/
/**
 * @brief Get the flush done status
 *
 * This function checks if the flush operation has completed by reading
 * the flush done status bit from the control register.
 *
 * @param  InstancePtr Pointer to the XV_warp_init instance
 *
 * @return Flush done status (non-zero if flush is complete, 0 otherwise)
 *
 * @note None
 *
 ******************************************************************************/
u32 XV_warp_init_Get_FlushDone(XV_warp_init *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_init_ReadReg(InstancePtr->Ctrl_BaseAddress,
		XV_WARP_INIT_CTRL_ADDR_AP_CTRL) & AP_CTRL_FLUSH_STATUSBIT;
    return Data;
}
