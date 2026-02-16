// ==============================================================
// Copyright (c) 2015 - 2022 Xilinx Inc. All rights reserved.
// Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/**
 * @file xv_tpg.c
 * @addtogroup v_tpg Overview
 */

/***************************** Include Files *********************************/

#include "xv_tpg.h"

/************************** Function Implementation *************************/
#ifndef __linux__
/*****************************************************************************/
/**
 * @brief Configure and initialize the TPG core instance
 *
 * This function initializes the TPG instance with the provided configuration
 * and effective base address. It copies the configuration data and sets the
 * ready flag to indicate successful initialization.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance to be initialized
 * @param  ConfigPtr Pointer to the configuration structure
 * @param  EffectiveAddr Effective base address of the TPG hardware
 *
 * @return XST_SUCCESS on successful initialization
 *
 * @note This function is only available for non-Linux systems
 *
 *******************************************************************************/
int XV_tpg_CfgInitialize(XV_tpg *InstancePtr,
                         XV_tpg_Config *ConfigPtr,
                         UINTPTR EffectiveAddr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);
    Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0x0);

    /* Setup the instance */
    InstancePtr->Config = *ConfigPtr;
    InstancePtr->Config.BaseAddress = EffectiveAddr;
#ifdef SDT
    InstancePtr->Config.IntrId = ConfigPtr->IntrId;
    InstancePtr->Config.IntrParent = ConfigPtr->IntrParent;
#endif

    /* Set the flag to indicate the driver is ready */
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * @brief Start the TPG core
 *
 * This function starts the TPG core by setting the AP_START bit in the control
 * register while preserving the auto-restart bit.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return None
 *
 * @note Instance must be initialized and ready before calling this function
 *
 *******************************************************************************/
void XV_tpg_Start(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

/*****************************************************************************/
/**
 * @brief Check if TPG core processing is done
 *
 * This function checks the AP_DONE bit in the control register to determine
 * if the current frame processing is complete.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return 1 if processing is done, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_IsDone(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if TPG core is idle
 *
 * This function checks the AP_IDLE bit in the control register to determine
 * if the TPG core is in idle state.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return 1 if core is idle, 0 otherwise
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_IsIdle(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

/*****************************************************************************/
/**
 * @brief Check if TPG core is ready for next input
 *
 * This function checks the AP_START bit to determine if the core is ready
 * to accept new input data.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return 1 if ready for next input, 0 if busy
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_IsReady(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

/*****************************************************************************/
/**
 * @brief Enable auto-restart mode
 *
 * This function enables auto-restart mode for the TPG core, allowing it to
 * automatically restart after each frame completion.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_EnableAutoRestart(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x80);
}

/*****************************************************************************/
/**
 * @brief Disable auto-restart mode
 *
 * This function disables auto-restart mode for the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_DisableAutoRestart(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * @brief Set the height of the output frame
 *
 * This function sets the height of the frame to be generated by the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Height value in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_height(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_HEIGHT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current height setting
 *
 * This function retrieves the current height setting of the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current height value in pixels
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_height(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the width of the output frame
 *
 * This function sets the width of the frame to be generated by the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Width value in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_width(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_WIDTH_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current width setting
 *
 * This function retrieves the current width setting of the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current width value in pixels
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_width(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the background pattern ID
 *
 * This function sets the background test pattern ID for the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Background pattern ID value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_bckgndId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BCKGNDID_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current background pattern ID
 *
 * This function retrieves the current background pattern ID setting.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current background pattern ID value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_bckgndId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BCKGNDID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enable motion for color bar pattern
 *
 * This function enables motion animation for the color bar background pattern.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Motion enable value (1 = enable, 0 = disable)
 *
 * @return None
 *
 * @note Motion is only applicable when background pattern is set to color bars
 *
 *******************************************************************************/
void XV_tpg_Set_motionEn(XV_tpg *InstancePtr, u32 Data) {
    u32 Pattern;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Pattern = XV_tpg_Get_bckgndId(InstancePtr);
    if (Pattern == XTPG_BKGND_COLOR_BARS)
	    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress,
			    XV_TPG_CTRL_ADDR_BCK_MOTION_EN_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get motion enable status for color bar pattern
 *
 * This function retrieves the current motion enable status for color bars.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Motion enable status (1 = enabled, 0 = disabled)
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_motionEnStatus(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress,
			  XV_TPG_CTRL_ADDR_BCK_MOTION_EN_DATA) &
			  (XV_TPG_CTRL_ADDR_MOTION_EN_MASK);
    return Data;
}
/*****************************************************************************/
/**
 * @brief Set the overlay pattern ID
 *
 * This function sets the overlay test pattern ID for the TPG core.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Overlay pattern ID value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_ovrlayId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_OVRLAYID_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current overlay pattern ID
 *
 * This function retrieves the current overlay pattern ID setting.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current overlay pattern ID value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_ovrlayId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_OVRLAYID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the color mask ID
 *
 * This function sets the color mask ID to control which color components
 * are enabled in the output.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Color mask ID value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_maskId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MASKID_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current color mask ID
 *
 * This function retrieves the current color mask ID setting.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current color mask ID value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_maskId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MASKID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the motion speed
 *
 * This function sets the variance of the test pattern between consecutive
 * frames when motion is enabled.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Motion speed value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_motionSpeed(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MOTIONSPEED_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current motion speed setting
 *
 * This function retrieves the current motion speed value.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current motion speed value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_motionSpeed(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MOTIONSPEED_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the color format
 *
 * This function sets the color format for the output video stream.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Color format value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_colorFormat(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current color format setting
 *
 * This function retrieves the current color format configuration.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current color format value
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_colorFormat(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_COLORFORMAT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set horizontal crosshair location
 *
 * This function sets the horizontal (X) coordinate for the crosshair overlay.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Horizontal crosshair position in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_crossHairX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRX_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get horizontal crosshair location
 *
 * This function retrieves the current horizontal crosshair position.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current horizontal crosshair position in pixels
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_crossHairX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set vertical crosshair location
 *
 * This function sets the vertical (Y) coordinate for the crosshair overlay.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Vertical crosshair position in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_crossHairY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRY_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get vertical crosshair location
 *
 * This function retrieves the current vertical crosshair position.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current vertical crosshair position in pixels
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_crossHairY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set zone plate horizontal control start value
 *
 * This function sets the horizontal component starting point based on
 * sinusoidal values for the zone plate test pattern.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Horizontal control start value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_ZplateHorContStart(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTSTART_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get zone plate horizontal control start value
 *
 * This function retrieves the current horizontal control start value.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current horizontal control start value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_ZplateHorContStart(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTSTART_DATA);
    return Data;
}


/*****************************************************************************/
/**
 * @brief Set zone plate horizontal control delta value
 *
 * This function sets the variance (delta) between horizontal components
 * for the zone plate pattern.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Horizontal control delta value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_ZplateHorContDelta(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTDELTA_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get zone plate horizontal control delta value
 *
 * This function retrieves the current horizontal control delta value.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current horizontal control delta value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_ZplateHorContDelta(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTDELTA_DATA);
    return Data;
}


/*****************************************************************************/
/**
 * @brief Set zone plate vertical control start value
 *
 * This function sets the vertical component starting point based on
 * sinusoidal values for the zone plate test pattern.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Vertical control start value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_ZplateVerContStart(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTSTART_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get zone plate vertical control start value
 *
 * This function retrieves the current vertical control start value.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current vertical control start value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_ZplateVerContStart(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTSTART_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set zone plate vertical control delta value
 *
 * This function sets the variance (delta) between vertical components
 * for the zone plate pattern.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Vertical control delta value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_ZplateVerContDelta(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTDELTA_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get zone plate vertical control delta value
 *
 * This function retrieves the current vertical control delta value.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current vertical control delta value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_ZplateVerContDelta(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTDELTA_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set the box size
 *
 * This function sets the size of the foreground box overlay.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Box size value in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_boxSize(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXSIZE_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get the current box size
 *
 * This function retrieves the current box size setting.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current box size value in pixels
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_boxSize(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXSIZE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set box red color component
 *
 * This function sets the Y or R (red/luma) component value for the box overlay.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Red/luma component value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_boxColorR(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORR_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get box red color component
 *
 * This function retrieves the Y or R component value of the box.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current red/luma component value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_boxColorR(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORR_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set box green color component
 *
 * This function sets the U or G (green/chroma U) component value for the box.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Green/chroma U component value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_boxColorG(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORG_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get box green color component
 *
 * This function retrieves the U or G component value of the box.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current green/chroma U component value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_boxColorG(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORG_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set box blue color component
 *
 * This function sets the V or B (blue/chroma V) component value for the box.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Blue/chroma V component value
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_boxColorB(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORB_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get box blue color component
 *
 * This function retrieves the V or B component value of the box.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current blue/chroma V component value
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_boxColorB(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORB_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enable input video stream passthrough
 *
 * This function enables the input video stream passthrough mode.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Enable value (1 = enable, 0 = disable)
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_enableInput(XV_tpg *InstancePtr, u32 Data) {
    u32 Reg;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Reg = XV_tpg_Get_fieldId(InstancePtr);
    Reg &= ~(XV_TPG_CTRL_ADDR_FIELDID_PASSTHR_MASK);

    if (Data) {
        Reg |= (1) << XV_TPG_CTRL_ADDR_FIELDID_PASSTHR_SHIFT;
    }

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress,
			XV_TPG_CTRL_ADDR_FIELDID_DATA, Reg);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ENABLEINPUT_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get input enable status
 *
 * This function retrieves the current input video stream enable status.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current enable status (1 = enabled, 0 = disabled)
 *
 * @note None
 *
 *******************************************************************************/
u32 XV_tpg_Get_enableInput(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ENABLEINPUT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set passthrough window left boundary
 *
 * This function sets the left (starting X) boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Left boundary X coordinate in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/
void XV_tpg_Set_passthruStartX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTX_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get passthrough window left boundary
 *
 * This function retrieves the current left boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current left boundary X coordinate
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_passthruStartX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set passthrough window upper boundary
 *
 * This function sets the upper (starting Y) boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Upper boundary Y coordinate in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_passthruStartY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTY_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get passthrough window upper boundary
 *
 * This function retrieves the current upper boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current upper boundary Y coordinate
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_passthruStartY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set passthrough window right boundary
 *
 * This function sets the right (ending X) boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Right boundary X coordinate in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_passthruEndX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDX_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get passthrough window right boundary
 *
 * This function retrieves the current right boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current right boundary X coordinate
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_passthruEndX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Set passthrough window lower boundary
 *
 * This function sets the lower (ending Y) boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 * @param  Data Lower boundary Y coordinate in pixels
 *
 * @return None
 *
 * @note None
 *
 *******************************************************************************/

void XV_tpg_Set_passthruEndY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDY_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Get passthrough window lower boundary
 *
 * This function retrieves the current lower boundary of the passthrough window.
 *
 * @param  InstancePtr Pointer to the XV_tpg instance
 *
 * @return Current lower boundary Y coordinate
 *
 * @note None
 *
 *******************************************************************************/

u32 XV_tpg_Get_passthruEndY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the dynamic range of DisplayPort color square in RGB.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Data is the dynamic range value to be set.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_Set_dpDynamicRange(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPDYNAMICRANGE_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the dynamic range of DisplayPort color square in RGB.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return The dynamic range value of DisplayPort color square in RGB.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

u32 XV_tpg_Get_dpDynamicRange(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPDYNAMICRANGE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Sets the coefficients of DisplayPort color square in YUV.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Data is the YUV coefficient value to be set.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_Set_dpYUVCoef(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPYUVCOEF_DATA, Data);
}

/*****************************************************************************/
/**
 * @brief Gets the coefficients of DisplayPort color square in YUV.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return The YUV coefficient value of DisplayPort color square.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

u32 XV_tpg_Get_dpYUVCoef(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPYUVCOEF_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enables or disables the interlaced video pattern.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Data is a boolean value to enable (TRUE) or disable (FALSE) interlaced mode.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_Set_Interlaced(XV_tpg *InstancePtr, _Bool Data) {
    u32 Reg;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Reg = XV_tpg_Get_fieldId(InstancePtr);
    Reg &= ~(XV_TPG_CTRL_ADDR_FIELDID_INTERLACED_MASK);
    Reg |= (Data) << XV_TPG_CTRL_ADDR_FIELDID_INTERLACED_SHIFT;

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_FIELDID_DATA, Reg);
}

/*****************************************************************************/
/**
 * @brief Sets the field polarity for interlaced video.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Data is a boolean value to set the field polarity.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/
void XV_tpg_Set_Polarity(XV_tpg *InstancePtr, _Bool Data) {
    u32 Reg;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Reg = XV_tpg_Get_fieldId(InstancePtr);
    Reg &= ~(XV_TPG_CTRL_ADDR_FIELDID_POLARITY_MASK);
    Reg |= (Data) << XV_TPG_CTRL_ADDR_FIELDID_POLARITY_SHIFT;

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress,
			XV_TPG_CTRL_ADDR_FIELDID_DATA, Reg);
}

/*****************************************************************************/
/**
 * @brief Gets the field ID for the current frame.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return The field ID value.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/
u32 XV_tpg_Get_fieldId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress,
				XV_TPG_CTRL_ADDR_FIELDID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * @brief Enables the global interrupts.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_InterruptGlobalEnable(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * @brief Disables the global interrupts.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_InterruptGlobalDisable(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * @brief Enables the interrupts specified by the mask.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Mask is the interrupt mask value to enable specific interrupts.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_InterruptEnable(XV_tpg *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * @brief Disables the interrupts specified by the mask.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Mask is the interrupt mask value to disable specific interrupts.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_InterruptDisable(XV_tpg *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * @brief Clears the interrupts specified by the mask.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 * @param  Mask is the interrupt mask value to clear specific interrupts.
 *
 * @return None.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

void XV_tpg_InterruptClear(XV_tpg *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * @brief Reports the currently enabled interrupts.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return The current interrupt enable status mask.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

u32 XV_tpg_InterruptGetEnabled(XV_tpg *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * @brief Reports the current interrupt status.
 *
 * @param  InstancePtr is a pointer to the XV_tpg instance.
 *
 * @return The current interrupt status mask.
 *
 * @note   Asserts if the pointer is NULL or instance is not ready.
 *
 *****************************************************************************/

u32 XV_tpg_InterruptGetStatus(XV_tpg *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ISR);
}
