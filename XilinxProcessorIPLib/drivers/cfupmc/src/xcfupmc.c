/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
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
* @note		None.
*
******************************************************************************/
s32 XCfupmc_CfgInitialize(XCfupmc *InstancePtr, XCfupmc_Config *CfgPtr,
			u32 EffectiveAddr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != ((u32)0));

	/* Setup the instance */
	(void)memcpy((void *)&(InstancePtr->Config), (const void *)CfgPtr,
						sizeof(XCfupmc_Config));
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function does CFI register write using MASK register
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 * @param	Addr Address of the regiser to be written
 * @param	Mask Mask of the bit field to be written
 * @param	Val Value of bit field
 *
 * @return  None
 *
 * @note	None.
 ******************************************************************************/
void XCfupmc_MaskRegWrite(XCfupmc *InstancePtr, u32 Addr, u32 Mask, u32 Val)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			CFU_APB_CFU_MASK, Mask);
	/* XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,Addr, Val); */

	/* TODO remove the hack once Mask register is implemented correctly */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, Addr,
		((XCfupmc_ReadReg(InstancePtr->Config.BaseAddress,
					Addr))&(~Mask)) | Val);
}

/*****************************************************************************/
/**
 * This function sets the CFU parameters as specified in the Instance Ptr
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return  None
 *
 * @note	None.
 ******************************************************************************/
void XCfupmc_SetParam(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf("CFU Initialization before CFI data load\n\r");

	/* Remove CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 0U);

	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			CFU_APB_CFU_ITR, CFU_APB_CFU_ITR_SEU_ENDOFCALIB_MASK);

	/* Enable compression if required */
	if (InstancePtr->DeCompress == 1U)
	{
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
				CFU_APB_CFU_CTL_DECOMPRESS_MASK,
					CFU_APB_CFU_CTL_DECOMPRESS_MASK);
	}

	/* Disable packet header CRC8 if required */
	if (InstancePtr->Crc8Dis == 1U)
	{
		/* crc32 check enable bit. Present from ITR6 */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
				CFU_APB_CFU_CTL_CRC8_DISABLE_MASK,
					CFU_APB_CFU_CTL_CRC8_DISABLE_MASK);
	}

	/* Enable CRC32 if required */
	if (InstancePtr->Crc32Check == 1U)
	{
		/* crc32 check enable bit. Present from ITR6 */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
				CFU_APB_CFU_CTL_CRC32_CHECK_MASK,
					CFU_APB_CFU_CTL_CRC32_CHECK_MASK);
	}

	/* Always reset CFU_FGCR[3]=0 for GLUTMASK=0 */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_GLUTMASK_MASK, 0);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 1U);
}

/*****************************************************************************/
/**
 * This function waits for CFU stream busy status
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return
 *		- XST_SUCCESS if there is no CRC failure
 *
 ******************************************************************************/
s32 XCfupmc_WaitForStreamBusy(XCfupmc *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	XCfupmc_Printf("Wait for CFU stream busy\n\r");

	/* Check for CFU stream busy status */
	while ((XCfupmc_ReadReg(
			InstancePtr->Config.BaseAddress, CFU_APB_CFU_STATUS) &
				CFU_APB_CFU_STATUS_CFU_STREAM_BUSY_MASK) ==
					CFU_APB_CFU_STATUS_CFU_STREAM_BUSY_MASK);

	/* TODO should return error after timeout */
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function checks CFI data CRC after loading to PL
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return
 *		- XST_SUCCESS if there is no CRC failure
 *
 ******************************************************************************/
s32 XCfupmc_CheckParam(XCfupmc *InstancePtr)
{
	s32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);

	XCfupmc_Printf("Settings After CFI Data loading\n\r");

	/* Check the expected CRC Value */
	if (InstancePtr->Crc32Check == 1U)
	{
		XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			CFU_APB_CFU_CRC_EXPECT, InstancePtr->Crc32Val);
		if ((XCfupmc_ReadReg(InstancePtr->Config.BaseAddress,
			CFU_APB_CFU_ISR) & CFU_APB_CFU_ISR_CRC32_ERROR_MASK)
				== CFU_APB_CFU_ISR_CRC32_ERROR_MASK)
		{
			XCfupmc_Printf("CRC32 failed \n\r");
			Status = (s32 )XST_FAILURE;
			goto END;
		}
	}

	/* Reset the decompression bit */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_DECOMPRESS_MASK, 0U);

	/* Enable SEU GO upon selection by user */

	/* Enable the CFU Protection  */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress,
			CFU_APB_CFU_PROTECT,
			CFU_APB_CFU_PROTECT_ACTIVE_MASK);

	XCfupmc_Printf("Bitstream Loading Success \n\r");
	Status = (s32 )XST_SUCCESS;

END:
	return Status;
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
void XCfupmc_SetGlblSigEn(XCfupmc *InstancePtr, u8 Enable)
{

	Xil_AssertVoid(InstancePtr != NULL);

	/* Remove CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 0U);

	if (Enable == (u8)TRUE)
	{
		/* Assert EN_GLOB */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		     CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK, 0x0U);
	} else {
		/* Deassert EN_GLOB */
		XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		     CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK,
		     CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK);
	}

	/* Enable CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 1U);
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
void XCfupmc_GlblSeqInit(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf( "PL Global Sequence: Init \n\r");

	/* Remove CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 0U);

	/* Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GRESTORE_MASK, CFU_APB_CFU_FGCR_GRESTORE_MASK);

	usleep(10);
	/* De Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_GRESTORE_MASK, 0x0U);

	/* Assert InitComplete */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK,
			CFU_APB_CFU_FGCR_INIT_COMPLETE_MASK);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 1U);
}

/*****************************************************************************/
/**
 * This function starts the sequence to enable Global signals
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_StartGlblSeq(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf( "PL Global Sequence: start up \n\r");

	/* Remove CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 0U);

	/* Assert EN_GLOB */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_EN_GLOBS_B_MASK, 0x0U);

	usleep(10);
	/* Assert GMC_B  */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GMC_B_MASK, CFU_APB_CFU_FGCR_GMC_B_MASK);

	usleep(10);
	/* Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GRESTORE_MASK, CFU_APB_CFU_FGCR_GRESTORE_MASK);

	usleep(10);
	/* De Assert GRESTORE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_GRESTORE_MASK, 0x0U);

	usleep(10);
	/* De Assert GHIGH_B */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GHIGH_B_MASK, CFU_APB_CFU_FGCR_GHIGH_B_MASK);

	/* Allow GHIGH_B to propogate */
	usleep(100);

	/* De Assert GTS_CFG_B */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
		CFU_APB_CFU_FGCR_GTS_CFG_B_MASK, CFU_APB_CFU_FGCR_GTS_CFG_B_MASK);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 1U);
}

/*****************************************************************************/
/**
 * This function does end of Global sequence
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_EndGlblSeq(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf("PL Global Sequence: EOS \n\r");

	/* Remove CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 0U);

	/* Assert GWE */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_GWE_MASK, CFU_APB_CFU_FGCR_GWE_MASK);
	usleep(10);

	/* Assert EOS */
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_FGCR,
			CFU_APB_CFU_FGCR_EOS_MASK, CFU_APB_CFU_FGCR_EOS_MASK);
	usleep(10);

	/* Enable CFU Protection */
	XCfupmc_WriteReg(InstancePtr->Config.BaseAddress, CFU_APB_CFU_PROTECT, 1U);
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
void XCfupmc_Reset(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf("CFU Reset \n\r");

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			     CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			     CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK);
	usleep(10);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			     CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			     0U);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			     CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			     CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK);
	usleep(10);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			     CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			     0U);
}

/*****************************************************************************/
/**
 * This function waits till Fabric is busy
 *
 * @param	InstancePtr is a pointer to the XCfupmc instance.
 *
 * @return	None
 *
 ******************************************************************************/
void XCfupmc_WaitForStreamDone(XCfupmc *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Printf("Wait for Stream Done \n\r");

	while ((XCfupmc_ReadStatus() &
	       CFU_APB_CFU_STATUS_CFU_STREAM_BUSY_MASK) ==
	       CFU_APB_CFU_STATUS_CFU_STREAM_BUSY_MASK);
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
void XCfupmc_CfuErrHandler(XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32	RegVal = XCfupmc_ReadReg(InstancePtr->Config.BaseAddress,
				CFU_APB_CFU_ISR);
    RegVal = RegVal & (CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
			 CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
			 CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
			 CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
			 CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
			 CFU_APB_CFU_ISR_CRC8_ERROR_MASK);

	if(RegVal)
	{
		XCfupmc_Reset(InstancePtr);
		XCfupmc_ClearCfuIsr(InstancePtr);
	}
	else
	{
		/** MISRA-C compliance */
	}
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
void XCfupmc_CfiErrHandler(XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_Reset(InstancePtr);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
						CFU_APB_CFU_CTL_SEU_GO_MASK, 0U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
						CFU_APB_CFU_CTL_DECOMPRESS_MASK, 0U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
						CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK,
						CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK);

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
void XCfupmc_ClearCfuIsr(XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_ClearIsr(InstancePtr, (CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
							CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
							CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
							CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
							CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
							CFU_APB_CFU_ISR_CRC8_ERROR_MASK));
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
void XCfupmc_ClearIgnoreCfiErr(XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
							CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_MASK,0U);
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
void XCfupmc_ExtErrorHandler(XCfupmc *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK);
	usleep(10);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFRAME_DISABLE_MASK, 0U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_SEU_GO_MASK, 0U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_DECOMPRESS_MASK, 0U);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK);
	usleep(10);
	XCfupmc_MaskRegWrite(InstancePtr, CFU_APB_CFU_CTL,
			CFU_APB_CFU_CTL_CFI_LOCAL_RESET_MASK, 0U);
}
