// ==============================================================
// Copyright (c) 1986 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_warp_filter.c
 * @addtogroup v_warp_filter Overview
 */

/***************************** Include Files *********************************/
#include "xv_warp_filter.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Initializes the Warp Filter driver instance.
 *
 * This function initializes the driver instance with the provided configuration
 * structure. It sets up the base address, marks the instance as ready, and
 * initializes descriptor-related fields.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param ConfigPtr Pointer to the configuration structure.
 *
 * @return XST_SUCCESS indicating successful initialization.
 *
 * @note This function is only available when not compiling for Linux.
 * @note Both InstancePtr and ConfigPtr must not be NULL.
 *
 *******************************************************************************/
s32 XV_warp_filter_CfgInitialize(XV_warp_filter *InstancePtr, XV_warp_filter_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->config = ConfigPtr;
    InstancePtr->Control_BaseAddress = ConfigPtr->Control_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    InstancePtr->WarpFilterDesc_BaseAddr = 0;
    InstancePtr->NumDescriptors = 0;

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Starts the Warp Filter core.
 *
 * This function triggers the start of the Warp Filter processing by setting
 * the AP_START bit in the control register while preserving the auto-restart bit.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Start(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL) & 0x80;
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Checks if the Warp Filter core has finished processing.
 *
 * This function reads the AP_DONE bit from the control register to determine
 * if the core has completed its current operation.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 1 if processing is done, 0 otherwise.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_IsDone(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Warp Filter core is idle.
 *
 * This function reads the AP_IDLE bit from the control register to determine
 * if the core is currently idle.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 1 if the core is idle, 0 otherwise.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_IsIdle(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Checks if the Warp Filter core is ready for next input.
 *
 * This function checks the AP_START bit to determine if the core is ready
 * to accept new input data.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 1 if the core is ready for next input, 0 otherwise.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_IsReady(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enables auto-restart mode for the Warp Filter core.
 *
 * This function enables the auto-restart feature by setting the appropriate
 * bit in the control register, allowing the core to automatically restart
 * after completing each frame.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_EnableAutoRestart(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disables auto-restart mode for the Warp Filter core.
 *
 * This function disables the auto-restart feature by clearing the control
 * register, requiring manual start for each frame.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_DisableAutoRestart(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Sets the flush bit to flush the Warp Filter pipeline.
 *
 * This function sets the flush bit in the control register to initiate
 * flushing of the internal pipeline. This is typically used to clear
 * pending operations.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_SetFlushbit(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress,
		XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL);
    Data |= AP_CTRL_BITS_FLUSH_BIT;
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress,
		XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the flush done status.
 *
 * This function reads the flush done status bit from the control register
 * to determine if the flush operation has completed.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return Flush done status bit value (non-zero if flush is complete).
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_Get_FlushDone(XV_warp_filter *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_AP_CTRL)
			       & AP_CTRL_BITS_FLUSH_STATUSBIT;
    return Data;
}


/*****************************************************************************/
/**
 * @brief Sets the descriptor address for the Warp Filter.
 *
 * This function writes the 64-bit descriptor address to the control registers,
 * specifying the location of the descriptor structure in memory.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit descriptor address.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_desc_addr(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the descriptor address from the Warp Filter.
 *
 * This function reads the 64-bit descriptor address from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit descriptor address.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_desc_addr(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_DESC_ADDR_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the AXI master read interface base address.
 *
 * This function writes the 64-bit base address for the AXI master read
 * interface to the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit base address for AXI master read.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_read(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the AXI master read interface base address.
 *
 * This function reads the 64-bit base address for the AXI master read
 * interface from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit base address for AXI master read.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_read(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the AXI master read interface address for segment table.
 *
 * This function writes the 64-bit address for reading segment table data
 * via the AXI master interface.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit address for segment table read.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_reads(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the AXI master read interface address for segment table.
 *
 * This function reads the 64-bit address for reading segment table data
 * from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit address for segment table read.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_reads(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READS_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the AXI master write interface base address.
 *
 * This function writes the 64-bit base address for the AXI master write
 * interface to the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit base address for AXI master write.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_write(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the AXI master write interface base address.
 *
 * This function reads the 64-bit base address for the AXI master write
 * interface from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit base address for AXI master write.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_write(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the secondary AXI master read interface base address.
 *
 * This function writes the 64-bit base address for the secondary AXI master
 * read interface (used for multi-plane formats) to the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit base address for secondary AXI master read.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_read1(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the secondary AXI master read interface base address.
 *
 * This function reads the 64-bit base address for the secondary AXI master
 * read interface from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit base address for secondary AXI master read.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_read1(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the secondary segment table read address.
 *
 * This function writes the 64-bit address for reading secondary segment table
 * data via the AXI master interface.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit address for secondary segment table read.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_read1s(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the secondary segment table read address.
 *
 * This function reads the 64-bit address for reading secondary segment table
 * data from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit address for secondary segment table read.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_read1s(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_READ1S_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the secondary AXI master write interface base address.
 *
 * This function writes the 64-bit base address for the secondary AXI master
 * write interface (used for multi-plane formats) to the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Data 64-bit base address for secondary AXI master write.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_Set_maxi_write1(XV_warp_filter *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA, (u32)(Data));
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA + 4, (u32)(Data >> 32));
}

/*****************************************************************************/
/**
 * @brief Gets the secondary AXI master write interface base address.
 *
 * This function reads the 64-bit base address for the secondary AXI master
 * write interface from the control registers.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return 64-bit base address for secondary AXI master write.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u64 XV_warp_filter_Get_maxi_write1(XV_warp_filter *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA);
    Data += (u64)XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_MAXI_WRITE1_DATA + 4) << 32;
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enables global interrupts for the Warp Filter.
 *
 * This function enables the global interrupt enable (GIE) bit, allowing
 * the core to generate interrupts.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_InterruptGlobalEnable(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disables global interrupts for the Warp Filter.
 *
 * This function disables the global interrupt enable (GIE) bit, preventing
 * the core from generating interrupts.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_InterruptGlobalDisable(XV_warp_filter *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enables specific interrupts for the Warp Filter.
 *
 * This function enables the interrupt sources specified by the Mask parameter
 * in the interrupt enable register.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Mask Bit mask of interrupts to enable.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_InterruptEnable(XV_warp_filter *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disables specific interrupts for the Warp Filter.
 *
 * This function disables the interrupt sources specified by the Mask parameter
 * in the interrupt enable register.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Mask Bit mask of interrupts to disable.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_InterruptDisable(XV_warp_filter *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clears pending interrupts for the Warp Filter.
 *
 * This function clears the interrupt status bits specified by the Mask parameter
 * by writing to the interrupt status register.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 * @param Mask Bit mask of interrupts to clear.
 *
 * @return None
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
void XV_warp_filter_InterruptClear(XV_warp_filter *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_warp_filter_WriteReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Gets the enabled interrupt mask.
 *
 * This function reads the interrupt enable register to determine which
 * interrupt sources are currently enabled.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return Bit mask of enabled interrupts.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_InterruptGetEnabled(XV_warp_filter *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Gets the pending interrupt status.
 *
 * This function reads the interrupt status register to determine which
 * interrupts are currently pending.
 *
 * @param InstancePtr Pointer to the XV_warp_filter instance.
 *
 * @return Bit mask of pending interrupts.
 *
 * @note InstancePtr must not be NULL and must be in the ready state.
 *
 *******************************************************************************/
u32 XV_warp_filter_InterruptGetStatus(XV_warp_filter *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_warp_filter_ReadReg(InstancePtr->Control_BaseAddress, XV_WARP_FILTER_CONTROL_ADDR_ISR);
}
