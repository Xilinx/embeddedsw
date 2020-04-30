// ==============================================================
// Copyright (c) 2015 - 2020 Xilinx Inc. All rights reserved.
// SPDX-License-Identifier: MIT
// ==============================================================

/***************************** Include Files *********************************/
#include "xv_tpg.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XV_tpg_CfgInitialize(XV_tpg *InstancePtr,
                         XV_tpg_Config *ConfigPtr,
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

void XV_tpg_Start(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL) & 0x80;
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XV_tpg_IsDone(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XV_tpg_IsIdle(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XV_tpg_IsReady(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XV_tpg_EnableAutoRestart(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x80);
}

void XV_tpg_DisableAutoRestart(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0);
}

/*****************************************************************************/
/**
 * * This function sets the height of the frame
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the height of the frame
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_height(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_HEIGHT_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the height of the frame
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  height of the frame
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_height(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the width of the frame
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the width of the frame
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_width(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_WIDTH_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the width of the frame
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  width of the frame
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_width(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the back ground id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the background id
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_bckgndId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BCKGNDID_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the background id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  value of background id
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_bckgndId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BCKGNDID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets motion enable for color bar pattern
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the motion enable
 * *
 * * @return  None
 * *
 * ******************************************************************************/
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
 * * This function gets status of motion enable for color bar pattern
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  status of the motion enable for color bar pattern
 * *
 * ******************************************************************************/
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
 * * This function sets the overlay id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the overlay id
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_ovrlayId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_OVRLAYID_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the overlay id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  value of overlay id
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_ovrlayId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_OVRLAYID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the color mask id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the color mask
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_maskId(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MASKID_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the color mask id
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  value of color mask
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_maskId(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MASKID_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the variance of test pattern between frames
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the variance of test pattern between frames
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_motionSpeed(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MOTIONSPEED_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the variance of test pattern between frames
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  variance of test pattern between frames
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_motionSpeed(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_MOTIONSPEED_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the color format
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set the color format
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_colorFormat(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_COLORFORMAT_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the color format
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return color forat used.
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_colorFormat(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_COLORFORMAT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets horizontal cross hair location
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set horizontal cross hair location
 * *
 * * @return  None
 * *
 * ******************************************************************************/
void XV_tpg_Set_crossHairX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRX_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets horizontal cross hair location
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  horizontal cross hair location
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_crossHairX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets vertical cross hair location
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set vertical cross hair location
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_crossHairY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRY_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets vertical cross hair location
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  vertical cross hair location
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_crossHairY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_CROSSHAIRY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets horizontal component starting point based sinusoidal values
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set horizontal component starting point based sinusoidal values
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_ZplateHorContStart(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTSTART_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets horizontal component starting point based sinusoidal values
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return sinusoidal values for horizontal component
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_ZplateHorContStart(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTSTART_DATA);
    return Data;
}


/*****************************************************************************/
/**
 * * This function sets the variance between horizontal components
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set variance between horizontal components
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_ZplateHorContDelta(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTDELTA_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the variance between horizontal components
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  variance between sinusoidal values of vertical component
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_ZplateHorContDelta(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEHORCONTDELTA_DATA);
    return Data;
}


/*****************************************************************************/
/**
 * * This function sets vertical component starting point based sinusoidal values
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set vertical component starting point based sinusoidal values
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_ZplateVerContStart(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTSTART_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets vertical component starting point based sinusoidal values
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  Vertical component starting point based sinusoidal values
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_ZplateVerContStart(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTSTART_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the variance between vertical components
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set variance between vertical components
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_ZplateVerContDelta(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTDELTA_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the variance between vertical components
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  variance between sinusoidal values of vertical component
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_ZplateVerContDelta(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ZPLATEVERCONTDELTA_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the size of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified size of the box
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_boxSize(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXSIZE_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the size of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return size of the box
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_boxSize(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXSIZE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the Y or R component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified Y component value
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_boxColorR(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORR_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the Y or R component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  Y or R component value of the box
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_boxColorR(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORR_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the U or G component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified U component value
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_boxColorG(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORG_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the U or G component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  U or G component value of the box
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_boxColorG(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORG_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the V or B component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified V component value
 * *
 * * @return  None
 * *
 * ******************************************************************************/

void XV_tpg_Set_boxColorB(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORB_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the V or B component value of the box
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return  V or B component value of the box
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_boxColorB(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_BOXCOLORB_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function enables input video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to enable the video stream
 * *
 * * @return None
 * *
 * ******************************************************************************/
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
 * * This function gets the tpg enable status
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return enable status of video stream
 * *
 * ******************************************************************************/
u32 XV_tpg_Get_enableInput(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ENABLEINPUT_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the left boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified boundary
 * *
 * * @return None
 * *
 * ******************************************************************************/
void XV_tpg_Set_passthruStartX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTX_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the left boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return left boundary of pass through window of video stream
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_passthruStartX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the right boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified boundary
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_Set_passthruStartY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTY_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the right boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return right boundary of pass through window of video stream
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_passthruStartY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUSTARTY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the upper boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified boundary
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_Set_passthruEndX(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDX_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets the upper boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return upper boundary of pass through window of video stream
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_passthruEndX(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDX_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the lower boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified boundary
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_Set_passthruEndY(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDY_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets lower boundary of pass through window of video stream
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return lower boundary of pass through window of video stream
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_passthruEndY(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_PASSTHRUENDY_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets Dynamic range of DisplayPort color square in RGB
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to set user specified Dynamic range
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_Set_dpDynamicRange(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPDYNAMICRANGE_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets Dynamic range of DisplayPort color square in RGB
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return Dynamic range of DisplayPort color square in RGB
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_dpDynamicRange(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPDYNAMICRANGE_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the Co-efficients of DisplayPort color square in YUV
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_Set_dpYUVCoef(XV_tpg *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPYUVCOEF_DATA, Data);
}

/*****************************************************************************/
/**
 * * This function gets Co-efficients of DisplayPort color square in YUV
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return Co-efficients of DisplayPort color square in YUV
 * *
 * ******************************************************************************/

u32 XV_tpg_Get_dpYUVCoef(XV_tpg *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_DPYUVCOEF_DATA);
    return Data;
}

/*****************************************************************************/
/**
 * * This function sets the interlaced video pattern
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to enable interlaced video pattern
 * *
 * * @return None
 * *
 * ******************************************************************************/

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
 * * This function sets the polarity
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Data is a variable to enable/disable polarity
 * *
 * * @return None
 * *
 * ******************************************************************************/
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
 * * This function gets field id for the frame
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return value of field id
 * *
 * ******************************************************************************/
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
 * * This function enables the global interrupts
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_InterruptGlobalEnable(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_GIE, 1);
}

/*****************************************************************************/
/**
 * * This function disables the global interrupts
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_InterruptGlobalDisable(XV_tpg *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_GIE, 0);
}

/*****************************************************************************/
/**
 * * This function enables the interrupts by using Mask value specified by user
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Mask is a variable to enable the specific interrupts
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_InterruptEnable(XV_tpg *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER, Register | Mask);
}

/*****************************************************************************/
/**
 * * This function disables the interrupts by using Mask value specified by user
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Mask is a variable to disable the specific interrupts
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_InterruptDisable(XV_tpg *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER, Register & (~Mask));
}

/*****************************************************************************/
/**
 * * This function clears the interrupts by using Mask value specified by user.
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * * @param  Mask is a variable to clear the triggered/specific interrupts
 * *
 * * @return None
 * *
 * ******************************************************************************/

void XV_tpg_InterruptClear(XV_tpg *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XV_tpg_WriteReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ISR, Mask);
}

/*****************************************************************************/
/**
 * * This function reports the current interrupts enable Status
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return Current interrupts enable status
 * *
 * ******************************************************************************/

u32 XV_tpg_InterruptGetEnabled(XV_tpg *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_IER);
}

/*****************************************************************************/
/**
 * * This function reports the interrupt Status
 * *
 * * @param  InstancePtr is a pointer to core instance to be worked upon
 * *
 * * @return current interrupt status
 * *
 * ******************************************************************************/

u32 XV_tpg_InterruptGetStatus(XV_tpg *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XV_tpg_ReadReg(InstancePtr->Config.BaseAddress, XV_TPG_CTRL_ADDR_ISR);
}
