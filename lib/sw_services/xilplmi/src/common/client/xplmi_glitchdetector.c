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
 *       pre  07/15/26 Added input parameter validation at required places
 *       pre  10/16/26 Added FMEA checks for register range, sensitivity bounds,
 *                     masked GD_FUSE_CTRL readback, and enable-state validation.
 *                     XPlmi_ReadReg32 and XPlmi_WriteReg32 now accept only
 *                     glitch detector registers. Pack PM IOCTLs with the
 *                     xilpm module id.
 * </pre>
 *
 * @note	XPlmi_ReadReg32 and XPlmi_WriteReg32 are exported. They now return
 *		XST_FAILURE for every PMC analog offset except the glitch detector
 *		registers. Read accepts GD_CTRL, GD_STATUS, GD_FUSE_CTRL0, and
 *		GD_FUSE_CTRL1. Write accepts GD_CTRL, GD_FUSE_CTRL0, and
 *		GD_FUSE_CTRL1. GD_STATUS is read only. No in-tree caller is affected.
 *		An out-of-tree caller that used these APIs for another PMC_ANALOG
 *		register now fails. In secure mode the PLM already enforces register
 *		access through the node access table.
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
#define XPLMI_GLITCHDETECTOR1		 (1U) /**< Glitch detector1 number */
#define XPLMI_WIDTH_MAX             (0x0FU) /**< Width is 4 bits; larger values overlap reference
                                            voltage */
#define XPLMI_USERREG_MAX           (0x01U) /**< User-register select is a single bit */
#define XPLMI_DEPTH_MAX             (0x1FU) /**< Depth field is 5 bits */
#define XPLMI_REFVOL_MAX            (0x0FU) /**< Reference voltage field is 4 bits */
/** Bits programmed in GD_FUSE_CTRL. Reserved gaps are left unchanged. */
#define XPLMI_GD_FUSE_CTRL_MASK     (XPLMI_USERREG_MAX | \
				(((u32)XPLMI_DEPTH_2MSBITS_MASK) << XPLMI_DEPTH_2MSBITS_SHIFT) | \
				(((u32)XPLMI_WIDTH_MAX) << XPLMI_WIDTH_BITS_SHIFT) | \
				(((u32)XPLMI_REFVOL_3LSBITS_MASK) << XPLMI_REFVOL_3LSBITS_SHIFT) | \
				(((u32)XPLMI_REFVOL_MSBIT_MASK) << XPLMI_REFVOL_MSBIT_SHIFT) | \
				(((u32)XPLMI_DEPTH_3LSBITS_MASK) << XPLMI_DEPTH_3LSBITS_SHIFT))
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

/*
 * XPLMI_PACK_PAYLOAD* stamps XILPLMI module id 1. Register IOCTLs must be
 * addressed to xilpm (module id 2), otherwise API 0x22 is scatter-write2.
 */
#define XPLMI_PACK_PM_PAYLOAD5(Payload, ApiId, Arg1, Arg2, Arg3, Arg4, Arg5) \
	XPLMI_PACK_PAYLOAD((Payload), PACK_XILPM_HEADER(5UL, (ApiId)), \
		(Arg1), (Arg2), (Arg3), (Arg4), (Arg5), 0U)

/************************** Function Prototypes **************************************************/

/************************** Variable Definitions *************************************************/
static XMailbox MailboxInstance;
static XPlmi_ClientInstance PlmiClientInstance;
#endif

/************************** Function Prototypes **************************************************/
static u32 XPlmi_IsGlitchDetectorOffset(u32 Offset);
static u32 XPlmi_IsGlitchDetectorWriteOffset(u32 Offset);

/*************************************************************************************************/
/**
 * @brief	Checks whether the offset is one of the readable glitch detector registers
 *
 * @param	Offset is the register offset from the PMC analog base. Valid values are
 *		GD_CTRL_OFFSET, GD_STATUS_OFFSET, GD_FUSE_CTRL0_OFFSET, and
 *		GD_FUSE_CTRL1_OFFSET.
 *
 * @return	1U when Offset is a glitch detector register, 0U otherwise
**************************************************************************************************/
static u32 XPlmi_IsGlitchDetectorOffset(u32 Offset)
{
	u32 IsValid = 0U;

	if ((Offset == GD_CTRL_OFFSET) ||
	    (Offset == GD_STATUS_OFFSET) ||
	    (Offset == GD_FUSE_CTRL0_OFFSET) ||
	    (Offset == GD_FUSE_CTRL1_OFFSET)) {
		IsValid = 1U;
	}

	return IsValid;
}

/*************************************************************************************************/
/**
 * @brief	Checks whether the offset is one of the writable glitch detector registers
 *
 * @param	Offset is the register offset from the PMC analog base. Valid values are
 *		GD_CTRL_OFFSET, GD_FUSE_CTRL0_OFFSET, and GD_FUSE_CTRL1_OFFSET.
 *		GD_STATUS_OFFSET is readable only.
 *
 * @return	1U when Offset may be written, 0U otherwise
**************************************************************************************************/
static u32 XPlmi_IsGlitchDetectorWriteOffset(u32 Offset)
{
	u32 IsValid = 0U;

	if ((Offset == GD_CTRL_OFFSET) ||
	    (Offset == GD_FUSE_CTRL0_OFFSET) ||
	    (Offset == GD_FUSE_CTRL1_OFFSET)) {
		IsValid = 1U;
	}

	return IsValid;
}

/*************************************************************************************************/
/**
 * @brief   Writes value to a glitch detector register
 *
 * @param	Offset is the register offset from the PMC analog base. Valid values are
 *		GD_CTRL_OFFSET, GD_FUSE_CTRL0_OFFSET, and GD_FUSE_CTRL1_OFFSET.
 * @param   Data is the value to be written in the register
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE when Offset is not a writable glitch detector register,
 *		  or on transport failure.
**************************************************************************************************/
int XPlmi_WriteReg32(u32 Offset, u32 Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[PAYLOAD_ARG_CNT];
#endif

	/** Reject a register the glitch detector does not write */
	if (XPlmi_IsGlitchDetectorWriteOffset(Offset) == 0U) {
		goto END;
	}

#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	/** Fill Payload */
	XPLMI_PACK_PM_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_MASK_WRITE_REG,
			Offset, XGLITCHDETECTOR_SECURE_WRITE_DEFAULT, Data);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(&PlmiClientInstance, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
#else
	/* Write value to register */
	Xil_Out32(PMC_ANALOG_BASE_ADDR + Offset, Data);
	Status = XST_SUCCESS;
#endif
END:
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
#else
	u32 RegVal;
#endif

	/** Reject a register the glitch detector does not write */
	if (XPlmi_IsGlitchDetectorWriteOffset(Offset) == 0U) {
		goto END;
	}

#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	/** Fill IPI Payload */
	XPLMI_PACK_PM_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_MASK_WRITE_REG,
			Offset, Mask, Data);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(&PlmiClientInstance, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

#else
	/* Read register value */
	RegVal = Xil_In32(PMC_ANALOG_BASE_ADDR + Offset);
	/* Modify with mask */
	RegVal = (RegVal & (~Mask)) | (Mask & Data);
	/* Write modified value to register */
	Xil_Out32(PMC_ANALOG_BASE_ADDR + Offset, RegVal);
	Status = XST_SUCCESS;
#endif
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief   Reads a glitch detector register.
 *
 * @param	Offset is the register offset from the PMC analog base. Valid values are
 *		GD_CTRL_OFFSET, GD_STATUS_OFFSET, GD_FUSE_CTRL0_OFFSET, and
 *		GD_FUSE_CTRL1_OFFSET.
 * @param	Data is the destination for the value read.
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XST_FAILURE when Data is NULL, Offset is not a glitch detector
 *		  register, or on transport failure.
**************************************************************************************************/
int XPlmi_ReadReg32(u32 Offset, u32 *Data)
{
	int Status = XST_FAILURE;
#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0U};
#endif

	/** Reject a missing destination or a register the glitch detector does not own */
	if ((Data == NULL) || (XPlmi_IsGlitchDetectorOffset(Offset) == 0U)) {
		goto END;
	}

#ifdef XPLMI_GLITCHDETECTOR_SECURE_MODE
	/** Fill IPI Payload */
	XPLMI_PACK_PM_PAYLOAD5(Payload, (u32)XPLMI_PM_IOCTL, PM_REG_PMC_ANALOG_ID, XPLMI_IOCTL_READ_REG,
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
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function configures desired glitch detector
 *
 * @param	Depth is the minimum voltage glitch detection depth, 0 to 0x1F
 * @param	Width is the minimum glitch detector width, 0 to 0x0F
 * @param	RefVoltage is the reference voltage for the glitch detector comparators,
 *		0 to 0x0F
 * @param	UserRegVal selects register values instead of eFUSE values. Valid values
 *		are 0 and 1.
 * @param	GlitchDetectorNum is the glitch detector to configure, 0 or 1
 *
 * @return
 *		- XST_SUCCESS when the programmed GD_FUSE_CTRL field bits read back.
 *		- XST_FAILURE when an input is out of range or those bits do not read
 *		  back. A transport failure from the read is returned unchanged.
 *
 * @note	This used to write the whole GD_FUSE_CTRL register, which forced
 *		reserved bits [3:1], [7:6], [15], and [31:20] to 0. It now updates
 *		only the XPLMI_GD_FUSE_CTRL_MASK bits, so a stale reserved value
 *		persists. In direct mode the read-modify-write and the readback
 *		are not atomic against other masters, so a concurrent writer can
 *		cause a false failure.
 *
  ************************************************************************************************/
int XPlmi_ConfigureGlitchDetector(u8 Depth, u8 Width, u8 RefVoltage, u8 UserRegVal,
                                  u8 GlitchDetectorNum)
{
	int Status = XST_FAILURE;
	u32 Data = 0U;
	u32 ReadVal = 0U;
	u32 Offset;

	/** Reject a detector the device does not have, or a setting that does not fit its field */
	if ((GlitchDetectorNum > XPLMI_GLITCHDETECTOR1) ||
	    (Depth > XPLMI_DEPTH_MAX) ||
	    (Width > XPLMI_WIDTH_MAX) ||
	    (RefVoltage > XPLMI_REFVOL_MAX) ||
	    (UserRegVal > XPLMI_USERREG_MAX)) {
		goto END;
	}

	/* Calculate offset based on glitch detector number */
	Offset = (u32)GD_FUSE_CTRL0_OFFSET + ((u32)XPLMI_WORD_LEN * (u32)GlitchDetectorNum);

	/* Frame data to be written */
	Data = (((RefVoltage & XPLMI_REFVOL_MSBIT_MASK) << XPLMI_REFVOL_MSBIT_SHIFT) |
	        ((RefVoltage & XPLMI_REFVOL_3LSBITS_MASK) << XPLMI_REFVOL_3LSBITS_SHIFT));
	Data |= (((Depth & XPLMI_DEPTH_2MSBITS_MASK) << XPLMI_DEPTH_2MSBITS_SHIFT)|
	        ((Depth & XPLMI_DEPTH_3LSBITS_MASK) << XPLMI_DEPTH_3LSBITS_SHIFT));
	Data |= (((u32)Width & XPLMI_WIDTH_MAX) << XPLMI_WIDTH_BITS_SHIFT);
	Data |= UserRegVal;

	/** Update only the programmed fields so reserved bits stay as they are */
	Status = XPlmi_UpdateReg32(Offset, XPLMI_GD_FUSE_CTRL_MASK, Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Report failure when the programmed field bits did not read back */
	Status = XPlmi_ReadReg32(Offset, &ReadVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((ReadVal & XPLMI_GD_FUSE_CTRL_MASK) != (Data & XPLMI_GD_FUSE_CTRL_MASK)) {
		Status = XST_FAILURE;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function enables/disables the desired glitch detector
 *
 * @param	GlitchDetectorNum is the glitch detector to enable or disable, 0 or 1
 * @param	EnableStatus is ENABLE or DISABLE
 *
 * @return
 *		- XST_SUCCESS when the detector bit reads back as requested.
 *		- XST_FAILURE when the detector number or enable state is invalid, or
 *		  the bit does not read back. A transport failure from the read is
 *		  returned unchanged.
 *
  ************************************************************************************************/
int XPlmi_ChangeGlitchDetectorState(u8 GlitchDetectorNum,eStatus EnableStatus)
{
	int Status = XST_FAILURE;
	u32 Mask;
	u32 Data = 0U;
	u32 RegVal = 0U;

	/** Reject an unknown detector or an enable state other than ENABLE or DISABLE */
	if ((GlitchDetectorNum > XPLMI_GLITCHDETECTOR1) ||
	    ((EnableStatus != ENABLE) && (EnableStatus != DISABLE))) {
		goto END;
	}

	Mask = ((u32)1U) << ((u32)GlitchDetectorNum * (u32)XPLMI_GD1_STATUS_POS);

	/* Frame data to be written */
	if (EnableStatus == DISABLE) {
		Data = Mask;
	}

	/* Update GD_CTRL register */
	Status = XPlmi_UpdateReg32(GD_CTRL_OFFSET, Mask, Data);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Report failure when the detector bit does not hold the requested state */
	Status = XPlmi_ReadReg32(GD_CTRL_OFFSET, &RegVal);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if ((RegVal & Mask) != Data) {
		Status = XST_FAILURE;
	}

END:
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

	/** Validate glitch detector number */
	if ((GlitchDetectorNum > XPLMI_GLITCHDETECTOR1) || (GlitchDetStatus == NULL)) {
		goto END;
	}

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
