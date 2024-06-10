/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/


/*************************************************************************************************/
/**
 *
 * @file xplmi_glitchdetector.c
 *
 * This is the file which contains glitch detector code.
 * This contains APIs for configuring glitch detector, enable/disable the glitch detector
 * and to read the status of glitch detector.
 * These APIs support both client and server mode for glitch detector0 and glitch detector1.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- --------------------------------------------------------------------------
 * 1.00  pre  06/09/2024 Initial release
 *
 * </pre>
 *
 * @note
 *
 *************************************************************************************************/

/***************************** Include Files *****************************************************/
#include "xplmi_glitchdetector.h"
#include "xil_io.h"
#include "xscugic.h"
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
#include "pm_api_sys.h"
#endif

/************************** Constant Definitions *************************************************/
#define XPLMI_REFVOL_3LSBITS_MASK   0x07 /**< Mask to extract 3 least significant bits from
                                        reference voltage*/
#define XPLMI_DEPTH_3LSBITS_MASK    0x07 /**< Mask to extract 3 least significant bits from
                                         depth */
#define XPLMI_REFVOL_MSBIT_MASK     0x08 /**< Mask to extract most significant bit from
                                         reference voltage */
#define XPLMI_DEPTH_2MSBITS_MASK    0x18 /**< Mask to extract 2 most significant bits from
                                         depth */
#define XPLMI_REFVOL_MSBIT_SHIFT    (16U) /**< Shift value to place most significant bit of
                                          reference voltage */
#define XPLMI_REFVOL_3LSBITS_SHIFT  (12U) /**< Shift value to place 3 least significant bits of
                                          reference voltage */
#define XPLMI_DEPTH_2MSBITS_SHIFT    (1U) /**< Shift value to place 2 most significant bits of
                                          depth */
#define XPLMI_DEPTH_3LSBITS_SHIFT   (16U) /**< Shift value to place 3 least significant bits of
                                          depth */
#define XPLMI_WIDTH_BITS_SHIFT      (8U) /**< Shift value to place bits of width */
#define XPLMI_WORD_LEN              (4U) /**< Word size in bytes */
#define XPLMI_GD1_STATUS_POS        (16U) /**< Bit position of glitch detector1 status */

/**************************** Type Definitions ***************************************************/

/***************** Macros (Inline Functions) Definitions *****************************************/

/************************** Function Prototypes **************************************************/

/************************** Variable Definitions *************************************************/
XScuGic InterruptController; /* Instance of the Interrupt Controller */
XScuGic_Config *GicConfig; /* The configuration parameters of the
                           controller */

/*************************************************************************************************/
/**
 * @brief   Writes value to register
 *
 * @param	Offset Offset of the register
 * @param   Data Value to be written in register
 *
 * @return  XST_SUCCESS on success
 *          error code on failure
**************************************************************************************************/
int XPlmi_WriteReg32(u32 Offset, u32 Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE];
	u32 Response;

	Payload[0] = Offset;
	Payload[1] = XGLITCHDETECTOR_SECURE_WRITE_DEFAULT;
	Payload[2] = Data;
	Status = XPm_DevIoctl2(PM_DEV_PMC_PROC, IOCTL_MASK_WRITE_REG, Payload,
			XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE, &Response,
			XGLITCHDETECTOR_SECURE_DEFAULT_RESPONSE_SIZE);
#else
	Xil_Out32(PMC_ANALOG_BASE_ADDR + Offset, Data);
	Status = XST_SUCCESS;
#endif
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   Updates register value
 *
 * @param	Offset Offset address of the register
 * @param	Mask Bits to be masked
 * @param	Data value to be written
 *
 * @return  XST_SUCCESS on success
 *          error code on failure
**************************************************************************************************/
int XPlmi_UpdateReg32(u32 Offset, u32 Mask, u32 Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE];
	u32 Response;

	Payload[0] = Offset;
	Payload[1] = Mask;
	Payload[2] = Data;
	Status = XPm_DevIoctl2(PM_DEV_PMC_PROC, IOCTL_MASK_WRITE_REG, Payload,
			XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE, &Response,
			XGLITCHDETECTOR_SECURE_DEFAULT_RESPONSE_SIZE);
#else
	u32 RegVal = Xil_In32(PMC_ANALOG_BASE_ADDR + Offset);
	RegVal = (RegVal & (~Mask)) | (Mask & Data);
	Xil_Out32(PMC_ANALOG_BASE_ADDR + Offset, RegVal);
	Status = XST_SUCCESS;
#endif
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   Reads register value.
 *
 * @param	Offset Offset of the register.
 * @param   Data Value to be read.
 *
**************************************************************************************************/
void XPlmi_ReadReg32(u32 Offset, u32 *Data)
{
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE];

	Payload[0] = Offset;
	Payload[1] = XGLITCHDETECTOR_SECURE_READ_DEFAULT;
	Payload[2] = XGLITCHDETECTOR_SECURE_READ_DEFAULT;
	XPm_DevIoctl2(PM_DEV_PMC_PROC, IOCTL_READ_REG, Payload,
			XGLITCHDETECTOR_SECURE_DEFAULT_PAYLOAD_SIZE, Data,
			XGLITCHDETECTOR_SECURE_DEFAULT_RESPONSE_SIZE);
#else
	*Data = Xil_In32(PMC_ANALOG_BASE_ADDR + Offset);
#endif
}

/*************************************************************************************************/
/**
 * @brief	This function configures desired glitch detector
 *
 * @param	Depth is the minimum voltage glitch detection depth
 * @param	Width is the minimum glitch detector width
 * @param   RefVoltage is the reference voltage for the glitch detector comparators
 * @param   UserRegVal indicates to select the register values instead of the eFUSE
 *          values to configure the glitch detector
 * @param   GlitchDetectorNum is the number of glitch detector to be configured
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - error code on failure.
 *
  ************************************************************************************************/
int XPlmi_ConfigureGlitchDetector(u8 Depth, u8 Width, u8 RefVoltage, u8 UserRegVal,
                                  u8 GlitchDetectorNum)
{
	int Status = XST_FAILURE;
	u32 Data = 0;
	u32 Offset = (u32)(GD_FUSE_CTRL0_OFFSET + (XPLMI_WORD_LEN * GlitchDetectorNum));

	Data = (((RefVoltage & XPLMI_REFVOL_MSBIT_MASK) << XPLMI_REFVOL_MSBIT_SHIFT) |
	        ((RefVoltage & XPLMI_REFVOL_3LSBITS_MASK) << XPLMI_REFVOL_3LSBITS_SHIFT));
	Data |= (((Depth & XPLMI_DEPTH_2MSBITS_MASK) << XPLMI_DEPTH_2MSBITS_SHIFT)|
	        ((Depth & XPLMI_DEPTH_3LSBITS_MASK) << XPLMI_DEPTH_3LSBITS_SHIFT));
	Data |=  (Width << XPLMI_WIDTH_BITS_SHIFT);
	Data |= UserRegVal;

	Status = XPlmi_WriteReg32(Offset, Data);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function enables/disables the desired glitch detector
 *
 * @param   GlitchDetectorNum is the number of glitch detector to be enabled/disabled
 * @param   EnableStatus indicates action i.e., enable/disable
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - error code on failure.
 *
  ************************************************************************************************/
int XPlmi_ChangeGlitchDetectorState(u8 GlitchDetectorNum,eStatus EnableStatus)
{
	int Status = XST_FAILURE;
	u32 Mask = 1U << (GlitchDetectorNum * XPLMI_GD1_STATUS_POS);
	u32 Data = 0;

	if(EnableStatus == DISABLE) {
		Data = 1U << (GlitchDetectorNum * XPLMI_GD1_STATUS_POS);
	}

	Status = XPlmi_UpdateReg32(GD_CTRL_OFFSET, Mask, Data);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the status of desired glitch detector
 *
 * @param   GlitchDetectorNum is the number of glitch detector whose status
 *          is to be read
 *
 * @return  Status of glitch detector
 *
  ************************************************************************************************/
u8 XPlmi_GlitchDetectorStatus(u8 GlitchDetectorNum)
{
	u8 Status;
	u32 RegVal;

	XPlmi_ReadReg32(GD_CTRL_OFFSET, &RegVal);
	Status = (RegVal >> (GlitchDetectorNum * XPLMI_GD1_STATUS_POS)) & 0x01;

	return Status;
}
