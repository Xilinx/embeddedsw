/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
*
* @file xplmi_glitchdetector.h
*
* This is the header file which contains definitions for the glitch detector
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- ---------------------------------------------------------------------------
* 1.00  pre  06/09/2024 Initial release
*
* </pre>
*
* @note
*
**************************************************************************************************/

#ifndef XPLMI_GLITCHDETECTOR_H
#define XPLMI_GLITCHDETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *****************************************************/
#include "xil_types.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *************************************************/
#define XPLMI_GLITCHDETECTOR_SECURE_MODE /* Comment this to access in server mode */
#define PMC_ANALOG_BASE_ADDR     0xF1160000 /**< PMC analog base address */
#define GD_CTRL_OFFSET              0x00000 /**< GD_CTRL register offset */
#define GD_STATUS_OFFSET            0x00004 /**< GD_STATUS register offset */
#define GD_FUSE_CTRL0_OFFSET        0x00020 /**< GD_FUSE_CTRL0 register offset */
#define GD_FUSE_CTRL1_OFFSET        0x00024 /**< GD_FUSE_CTRL1 register offset */
#define GICP4_IRQ_STATUS_ADDR    0xF1140050 /**< GICP4_IRQ_STATUS register address */
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
#define XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE  (3U) /**< Payload size of command in
                                                          secure mode*/
#define XGLITCHDETECTOR_SECURE_DEFAULT_RESPONSE_SIZE (1U) /**< Response size in secure mode*/
#define XGLITCHDETECTOR_SECURE_WRITE_DEFAULT         0xFFFFFFFF /**< Default write mask*/
#define XGLITCHDETECTOR_SECURE_READ_DEFAULT          0x00 /**< Default read mask */
#endif

#define DEVICE_INT_ID 191 /* when using for PL,change interrupt ID here */

/**************************** Type Definitions ***************************************************/
typedef enum
{
    ENABLE = 0, /**< Status enable */
    DISABLE /**< Status disable */
}eStatus;

/***************** Macros (Inline Functions) Definitions *****************************************/

/************************** Function Prototypes **************************************************/
int XPlmi_ConfigureGlitchDetector(u8 Depth, u8 Width, u8 RefVoltage, u8 UserRegVal,
                                  u8 GlitchDetectorNum);
int XPlmi_ChangeGlitchDetectorState(u8 GlitchDetectorNum,eStatus EnableStatus);
u8 XPlmi_GlitchDetectorStatus(u8 GlitchDetectorNum);
int XPlmi_WriteReg32(u32 Offset, u32 Data);
int XPlmi_UpdateReg32(u32 Offset, u32 Mask, u32 Data);
void XPlmi_ReadReg32(u32 Offset, u32 *Data);

/************************** Variable Definitions *************************************************/

/*************************************************************************************************/

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_GLITCHDETECTOR_H */
