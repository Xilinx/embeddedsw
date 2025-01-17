/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcfupmc.c
*
* This file which contains the code related to CFU block.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
* 2.00  bsv	 03/01/2019	Added error handling APIs
* 2.01  bsv  11/06/2019 XCfupmc_ClearCfuIsr API added
* 3.00  bsv  27/06/2020 Code clean up
*       pre  01/16/2025 Fixed warning
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xcfupmc.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void XCfupmc_Reset(const XCfupmc *InstancePtr);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
*
* This function initializes an CFU core. This function must be called
* prior to using an CFU driver.
*
* @param	InstancePtr is a pointer to the XCfupmc instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XCfupmc instance.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address mapping from EffectiveAddr to the device physical
*		base address unchanged once this function is invoked.
*		Unexpected errors may occur if the address mapping changes
*		after this function is called. If address translation is not
*		used, pass in the physical address instead.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
******************************************************************************/
s32 XCfupmc_CfgInitialize(XCfupmc *InstancePtr, const XCfupmc_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0U);

	/* Setup the instance */
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
		sizeof(XCfupmc_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function does CFI register write using MASK register
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 * @param	Addr Address of the register to be written
 * @param	Mask Mask of the bit field to be written
 * @param	Val Value of bit field
 *
 * @return  None
 *
 ******************************************************************************/
void XCfupmc_MaskRegWrite(const XCfupmc *InstancePtr, u32 Addr, u32 Mask, u32 Val)
{
	(void)InstancePtr;

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_WriteReg(CFU_APB_CFU_MASK, Mask);
	XCfupmc_WriteReg(Addr,
		((XCfupmc_ReadReg(Addr)&(~Mask)) | Val));

	return;
}

/*****************************************************************************/
/**
 * This function enables or disables the global signals
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_SetGlblSigEn(const XCfupmc *InstancePtr, u8 Enable)
{

	Xil_AssertVoid(InstancePtr != NULL);

	/* Remove CFU Protection */
	XCfupmc_WriteReg(CFU_APB_CFU_PROTECT, CFUPMC_PROT_DISABLE);

	if (Enable == CFUPMC_GLB_SIG_EN) {
		/* Assert EN_GLOB */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		     CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK,
			 CFU_APB_CFU_FGCR_EN_GLOBS_B_ASSERT_VAL);
	}
	else {
		/* Deassert EN_GLOB */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK,
			CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK);
	}

	/* Enable CFU Protection */
	XCfupmc_WriteReg(CFU_APB_CFU_PROTECT, CFUPMC_PROT_ENABLE);
}

/*****************************************************************************/
/**
 * This function inits the global events to needs to be done before
 * loading the CFI data
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_GlblSeqInit(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf( "PL Global Sequence: Init \n\r");

	/* Remove CFU Protection */
	XCfupmc_WriteReg(CFU_APB_CFU_PROTECT, CFUPMC_PROT_DISABLE);

	/* Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GRESTORE_MASK, CFU_APB_CFU_FGCR_GRESTORE_MASK);

	usleep(10U);
	/* De Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GRESTORE_MASK, CFU_APB_CFU_FGCR_GRESTORE_CLEAR);
	/* Assert InitComplete */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK,
			CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(CFU_APB_CFU_PROTECT, CFUPMC_PROT_ENABLE);

	return;
}

/*****************************************************************************/
/**
 * This function resets the CFU block
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
static void XCfupmc_Reset(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf("CFU Reset\n\r");

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK);
	usleep(10U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			     CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			     CFU_APB_CFU_CTL_CFRAME_DISABLE_CLR_VAL);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK);
	usleep(10U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_CLR_VAL);

	return;
}

/*****************************************************************************/
/**
 * This function checks and handles CFU errors
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_CfuErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegVal;

	RegVal = XCfupmc_ReadReg(CFU_APB_CFU_ISR);
	RegVal = RegVal & (CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
			CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
			CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
			CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
			CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
			CFU_APB_CFU_ISR_CRC8_ERROR_MASK);
	if (RegVal != 0U) {
		XCfupmc_Reset(InstancePtr);
		XCfupmc_ClearCfuIsr(InstancePtr);
	}

	return;
}

/*****************************************************************************/
/**
 * This function handles CFI errors
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_CfiErrHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Reset(InstancePtr);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_SEU_GO_MASK,
			CFU_APB_CFU_CTL_SEU_GO_CLR_VAL);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_DECOMPRESS_MASK,
			CFU_APB_CFU_CTL_DECOMPRESS_CLR_VAL);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK,
			CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK);

	return;
}

/*****************************************************************************/
/**
 * This function clears CFU ISR
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_ClearCfuIsr(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_ClearIsr(InstancePtr, (CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
			CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
			CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
			CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
			CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
			CFU_APB_CFU_ISR_CRC8_ERROR_MASK));

	return;
}

/*****************************************************************************/
/**
 * This function clears Ignore CFI ERROR mask in CFU_APB_CFU_CTL
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_ClearIgnoreCfiErr(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK,
			CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_CLR_VAL);

	return;
}

/****************************************?*************************************/
/**
* This function is called when bitstream transfer is aborted by user
*
* @param   InstancePtr is a pointer to the XCfupmc instance.
*
* @return  None
*
*****************************************?*************************************/
void XCfupmc_ExtErrorHandler(const XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK);
	usleep(10U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_CLR_VAL);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_SEU_GO_MASK,
			CFU_APB_CFU_CTL_SEU_GO_CLR_VAL);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_DECOMPRESS_MASK,
			CFU_APB_CFU_CTL_DECOMPRESS_CLR_VAL);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK);
	usleep(10U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_CLR_VAL);

	return;
}
