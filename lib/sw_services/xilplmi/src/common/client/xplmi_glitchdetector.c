/**************************************************************************************************
* Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       pre  07/17/2024 Used register node to access PMC analog registers and fixed
 *                       misrac warnings
 *       pre  10/19/2024 Added support for PL microblaze
 * 1.01   abh   07/22/2025 Added doxygen tag
 * 2.4   gnr  03/18/26 Updated the Payload assignments with XPLMI_PACK_PAYLOAD macros
 *       tbk  02/24/26 Added SMC support for client applications
 *       tbk  05/19/26 Unified response indexing; helper now handles SMC/mailbox shift
 * </pre>
 *
 * @note
 *
 *************************************************************************************************/
/**
 * @addtogroup xilplmi_client_apis XilPlmi Client APIs
 * @{
 */
/***************************** Include Files *****************************************************/
#include "xplmi_glitchdetector.h"
#include "xil_io.h"
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
#include "xil_cache.h"
#include "xplmi_client.h"
#include "xplmi_generic.h"
#include "xilmailbox_ipips_control.h"
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
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
#define XPLMI_PM_MODULE_ID			(2U) /**< Module ID for xilpm */
#define XPLMI_PM_MODULE_ID_MASK		((u32)XPLMI_PM_MODULE_ID << XPLMI_MODULE_ID_SHIFT) /**< Module
											ID mask */
#define XPLMI_IOCTL_MASK_WRITE_REG  (29U) /**<  Write command ID */
#define XPLMI_IOCTL_READ_REG        (28U) /**< Read command ID */
#define XPLMI_PM_IOCTL              (0x22U) /**< PM IOCTL command ID */
#endif

/**************************** Type Definitions ***************************************************/

/***************** Macros (Inline Functions) Definitions *****************************************/
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
static inline u32 PACK_XILPM_HEADER(u32 Len, u32 ApiId)
{
	return ((Len << XPLMI_PAYLOAD_LEN_SHIFT) | XPLMI_PM_MODULE_ID_MASK | (ApiId));
}

/************************** Function Prototypes **************************************************/

/************************** Variable Definitions *************************************************/
static XMailbox MailboxInstance;
static XPlmi_ClientInstance PlmiClientInstance;
#endif

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
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Fill Payload */
	XPLMI_PACK_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_MASK_WRITE_REG,
			Offset, XGLITCHDETECTOR_SECURE_WRITE_DEFAULT, Data);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(&PlmiClientInstance, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
#else
	/* Write value to register */
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
static int XPlmi_UpdateReg32(u32 Offset, u32 Mask, u32 Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_MASK_WRITE_REG,
			Offset, Mask, Data);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(&PlmiClientInstance, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

#else
    /* Read register value */
	u32 RegVal = Xil_In32(PMC_ANALOG_BASE_ADDR + Offset);
	/* Modify with mask */
	RegVal = (RegVal & (~Mask)) | (Mask & Data);
	/* Write modified value to register */
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
 * @return  XST_SUCCESS on success
 *          error code on failure
**************************************************************************************************/
int XPlmi_ReadReg32(u32 Offset, u32 *Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_READ_REG,
			Offset, XGLITCHDETECTOR_SECURE_READ_DEFAULT, XGLITCHDETECTOR_SECURE_READ_DEFAULT);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(&PlmiClientInstance, Payload,
			(u32)PAYLOAD_ARG_CNT, Response, (u32)RESPONSE_ARG_CNT);
	if (Status == XST_SUCCESS) {
		*Data = Response[0U];
	}

#else
	/* Read register value */
	*Data = Xil_In32(PMC_ANALOG_BASE_ADDR + Offset);
	Status = XST_SUCCESS;
#endif
	return Status;
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
	/* Caclculate offset based on glitch detector number */
	u32 Offset = (u32)(GD_FUSE_CTRL0_OFFSET + (XPLMI_WORD_LEN * GlitchDetectorNum));

	/* Frame data to be written */
	Data = (((RefVoltage & XPLMI_REFVOL_MSBIT_MASK) << XPLMI_REFVOL_MSBIT_SHIFT) |
	        ((RefVoltage & XPLMI_REFVOL_3LSBITS_MASK) << XPLMI_REFVOL_3LSBITS_SHIFT));
	Data |= (((Depth & XPLMI_DEPTH_2MSBITS_MASK) << XPLMI_DEPTH_2MSBITS_SHIFT)|
	        ((Depth & XPLMI_DEPTH_3LSBITS_MASK) << XPLMI_DEPTH_3LSBITS_SHIFT));
	Data |=  ((u32)Width << XPLMI_WIDTH_BITS_SHIFT);
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

	/* Frame data to be written */
	if (EnableStatus == DISABLE) {
		Data = 1U << (GlitchDetectorNum * XPLMI_GD1_STATUS_POS);
	}

	/* Update GD_CTRL register */
	Status = XPlmi_UpdateReg32(GD_CTRL_OFFSET, Mask, Data);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the status of desired glitch detector
 *
 * @param   GlitchDetectorNum is the number of glitch detector whose status
 *          is to be read
 * @param   GlitchDetStatus holds the status
 *
 * @return  Status of glitch detector
 *
  ************************************************************************************************/
int XPlmi_GlitchDetectorStatus(u8 GlitchDetectorNum, u8 *GlitchDetStatus)
{
	int Status = XST_FAILURE;
	u32 RegVal = 0;

	Status = XPlmi_ReadReg32(GD_CTRL_OFFSET, &RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	*GlitchDetStatus = (RegVal >> (GlitchDetectorNum * XPLMI_GD1_STATUS_POS)) & 0x01U;

END:
	return Status;
}

#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
/*************************************************************************************************/
/**
 * @brief	This function initializes the mailbox and sets the client instance
 *
 * @return  Status of initialization
 *
  ************************************************************************************************/
int XPlmi_MailboxInitialize(void)
{
	int Status = XST_FAILURE;

	#ifdef XPLMI_CACHE_DISABLE
		Xil_DCacheDisable();
	#endif

	#ifndef SDT
	Status = (int)XMailbox_Initialize(&MailboxInstance, 0U);
	#else
	Status = (int)XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
	#endif
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Status = XPlmi_ClientInit(&PlmiClientInstance, &MailboxInstance);
END:
	return Status;
}

/*****************************************************************************/
/**
 * Poll for an acknowledgement using Observation Register.
 *
 * @return      XST_SUCCESS in case of success
 *              XST_FAILURE in case of failure
 */
/****************************************************************************/
int XPlmi_PollforDone(void)
{
	return (int)XIpiPs_PollforDone(&MailboxInstance);
}
#endif

/** @} End of xilplmi_client_apis group */
