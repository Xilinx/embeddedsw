/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF PLRCHANTABILITY,
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEPLNT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_fabric.c
*
* This file which contains the code related to PL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xpmcfw_fabric.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XCframe CframeIns={0}; /* CFRAME Driver Instance */
XCfupmc CfupmcIns={0}; /* CFU Driver Instance */
/*****************************************************************************/

/*****************************************************************************/
/**
 * This function initializes the CFU driver
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_CfuInit()
{
	XStatus Status;
	XCfupmc_Config *Config;

	if(CfupmcIns.IsReady)
	{
		Status = XPMCFW_SUCCESS;
		goto END;
	}
	/*
	 * Initialize the CFU driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCfupmc_LookupConfig((u16)XPAR_XCFUPMC_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XPMCFW_ERR_CFU_LOOKUP;
		goto END;
	}

	Status = XCfupmc_CfgInitialize(&CfupmcIns, Config, Config->BaseAddress);
	if (Status != XPMCFW_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_CFU_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCfupmc_SelfTest(&CfupmcIns);
	if (Status != XPMCFW_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_CFU_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the Cframe driver
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_CframeInit()
{
	XStatus Status;
	XCframe_Config *Config;

	if(CframeIns.IsReady)
	{
		Status = XPMCFW_SUCCESS;
		goto END;
	}
	/*
	 * Initialize the Cframe driver so that it's ready to use
	 * look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCframe_LookupConfig((u16)XPAR_XCFRAME_0_DEVICE_ID);
	if (NULL == Config) {
		Status = XPMCFW_ERR_CFRAME_LOOKUP;
		goto END;
	}

	Status = XCframe_CfgInitialize(&CframeIns, Config, Config->BaseAddress);
	if (Status != XPMCFW_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_CFRAME_CFG, Status);
		goto END;
	}

	/*
	 * Performs the self-test to check hardware build.
	 */
	Status = XCframe_SelfTest(&CframeIns);
	if (Status != XPMCFW_SUCCESS) {
		Status = XPMCFW_UPDATE_ERR(XPMCFW_ERR_CFRAME_SELFTEST, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function initializes the CFU and Cframe driver
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_FabricInit()
{
	XStatus Status;

	/** Initialize the CFU Driver */
	Status = XPmcFw_CfuInit();
	if (Status != XPMCFW_SUCCESS) {
		goto END;
	}

	/** Initialize the CFI Driver */
	Status = XPmcFw_CframeInit();
	if (Status != XPMCFW_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * * This function initializes the global sequencing
 * *
 * * @return      None
 *******************************************************************************/

void XPmcFw_FabricGlblSeqInit()
{
	XCfupmc_GlblSeqInit(&CfupmcIns);
}

/*****************************************************************************/
/**
 * This function initializes the CFU to load the CFI data
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_FabricPrepare()
{
	XStatus Status;

	/* Set CFU settings */
	CfupmcIns.Crc8Dis=1U;
	//CfupmcIns.DeCompress=1U;
	XCfupmc_SetParam(&CfupmcIns);
	Status = XPMCFW_SUCCESS;
	return Status;
}

void XPmcFw_FabricEnable()
{
	/* Enable the global signals */
        XCfupmc_SetGlblSigEn(&CfupmcIns, (u8 )TRUE);
}

/*****************************************************************************/
/**
 * This function does the CFU settings after bitstream load
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_FabricStartSeq()
{
	XStatus Status;

	/*wait for stream status */
	Status = XCfupmc_WaitForStreamBusy(&CfupmcIns);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}

	/* Set CFU settings */
	Status = XCfupmc_CheckParam(&CfupmcIns);
	if (XPMCFW_SUCCESS != Status)
	{
		goto END;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function does the fabric end global sequence
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_FabricEndSeq()
{
	/* Start the PL global sequence */
	XCfupmc_EndGlblSeq(&CfupmcIns);

	/* Disable the global signals */
	XCfupmc_SetGlblSigEn(&CfupmcIns, (u8 )FALSE);

	return XPMCFW_SUCCESS;
}

/*****************************************************************************/
/**
 * This function writes the TRIM values to VGG, CRAM, BRAM, URAM
 *
 * @param   TrimType
 *
 * @return	None
 ******************************************************************************/
void XPmcFw_ApplyTrim(u32 TrimType)
{
	u32 TrimVal;
	Xuint128 VggTrim={0};

	/* Read the corresponding efuse registers for TRIM values */
	switch (TrimType)
	{
		/* Read VGG trim efuse registers */
		case XPMCFW_FABRIC_TRIM_VGG:
		{
			VggTrim.Word0 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_0);
			VggTrim.Word1 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_1);
			VggTrim.Word2 = Xil_In32(EFUSE_CACHE_TRIM_CFRM_VGG_2);
			XCframe_VggTrim(&CframeIns, &VggTrim);
		}
		break;
		/* Read CRAM trim efuse registers */
		case XPMCFW_FABRIC_TRIM_CRAM:
		{
			TrimVal = Xil_In32(EFUSE_CACHE_TRIM_CRAM);
			XCframe_CramTrim(&CframeIns, TrimVal);

		}
		break;
		/* Read BRAM trim efuse registers */
		case XPMCFW_FABRIC_TRIM_BRAM:
		{
			TrimVal = Xil_In32(EFUSE_CACHE_TRIM_BRAM);
			XCframe_BramTrim(&CframeIns, TrimVal);
		}
		break;
		/* Read URAM trim efuse registers */
		case XPMCFW_FABRIC_TRIM_URAM:
		{
			TrimVal = Xil_In32(EFUSE_CACHE_TRIM_URAM);
			XCframe_UramTrim(&CframeIns, TrimVal);
		}
		break;
		default:
		{
			break;
		}
	}
}

/*****************************************************************************/
/**
 * This function repairs the BRAM and URAM
 *
 * @param   None
 *
 * @return	None
 *
 ******************************************************************************/
void XPmcFw_BramUramRepair()
{

}

/*****************************************************************************/
/**
 * This function initiates house cleaning of the PL
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_FabricClean()
{
	XStatus Status;

	XPmcFw_Printf(DEBUG_INFO, "Fabric Cleaning Started...");

	/* Enable ROWON */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
			XCFRAME_CMD_REG_ROWON);

	/* HCLEANR Type 3,4,5,6 */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
			XCFRAME_CMD_REG_HCLEANR);
	/* BRAM TRIM */
	XPmcFw_ApplyTrim(XPMCFW_FABRIC_TRIM_BRAM);

	/* URAM TRIM */
	XPmcFw_ApplyTrim(XPMCFW_FABRIC_TRIM_URAM);

	/* BRAM/URAM REPAIR */
	XPmcFw_BramUramRepair();

	/* Start HCLEAN Type 0,1,2 */
	XCframe_WriteCmd(&CframeIns, XCFRAME_FRAME_BCAST,
				XCFRAME_CMD_REG_HCLEAN);

	/* Poll for house clean completion */
	while ((Xil_In32(CFU_APB_CFU_STATUS) &
			CFU_APB_CFU_STATUS_HC_COMPLETE_MASK) !=
				CFU_APB_CFU_STATUS_HC_COMPLETE_MASK);
	while ((Xil_In32(CFU_APB_CFU_STATUS) &
			CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK) ==
				CFU_APB_CFU_STATUS_CFI_CFRAME_BUSY_MASK);
	XPmcFw_Printf(DEBUG_INFO, "Done\n\r");

	/* VGG TRIM */
	XPmcFw_ApplyTrim(XPMCFW_FABRIC_TRIM_VGG);

	/* CRAM TRIM */
	XPmcFw_ApplyTrim(XPMCFW_FABRIC_TRIM_CRAM);

	Status = XPMCFW_SUCCESS;
	return Status;
}

/*****************************************************************************/
/**
 * This function does BISR and Laguna repair
 *
 * @param	None
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_BisrLaguna()
{

	/* BISR Repair */

	/* Laguna Repair */

	/* Scan Clear MBIST PL */

	return XPMCFW_SUCCESS;
}

/*****************************************************************************/
/**
 * This function reads CFI data from CFU block
 *
 * @param	CfiReadPtr is pointer to store the read CFI data
 * @param	CfiLen is length of the CFI data to be read
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_ReadFabricData(u32 *CfiReadPtr, u32 CfiLen)
{
	XStatus Status;

	XCframe_SetReadParam(&CframeIns, XCFRAME_FRAME_0, CfiLen);

	XPmcFw_Printf(DEBUG_INFO, "Reading Bitstream of Len 0x%08x....\n\r", CfiLen);

	/* DMA Transfer the CRAM Data to PMC RAM */
	Status = XPmcFw_DmaXfr((u64 )CFU_FDRO_ADDR, (u64)(UINTPTR)CfiReadPtr,
			CfiLen, XPMCFW_PMCDMA_0 | XPMCFW_SRC_CH_AXI_FIXED);

	XPmcFw_Printf(DEBUG_INFO, "Bitstream Reading Success \n\r");

#if 1
	for (int Index=0;Index<CfiLen/4;Index++) {
		XPmcFw_Printf(DEBUG_INFO, "QWord%d - %08x%08x%08x%08x \n\r", Index,
			CfiReadPtr[Index*4], CfiReadPtr[(Index*4)+1],
			CfiReadPtr[(Index*4)+2], CfiReadPtr[(Index*4)+3]);
	}
#endif

	return Status;
}

/*****************************************************************************/
/**
 * This function checks for Fabric errors after loading
 *
 * @return	Codes as mentioned in xpmcfw_err.h
 ******************************************************************************/
XStatus XPmcFw_CheckFabricErr()
{
	XStatus Status;
	u32 ErrStatus;
	u32 ErrMask;

	/* Wait for stream busy */
	XCfupmc_WaitForStreamDone(&CfupmcIns);

	ErrMask = CFU_APB_CFU_ISR_DECOMP_ERROR_MASK |
		CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK |
		CFU_APB_CFU_ISR_AXI_ALIGN_ERROR_MASK |
		CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
		CFU_APB_CFU_ISR_CRC32_ERROR_MASK |
		CFU_APB_CFU_ISR_CRC8_ERROR_MASK;

	ErrStatus = XCfupmc_ReadIsr(&CfupmcIns) & ErrMask;
	Status = ErrStatus;
	if(Status == XST_SUCCESS)
	{
		goto END;
	}
	if ((ErrStatus & (CFU_APB_CFU_ISR_CRC8_ERROR_MASK |
					CFU_APB_CFU_ISR_CRC32_ERROR_MASK)) != 0U)
	{
		XPmcFw_Printf(DEBUG_INFO, "Bitstream loading failed ISR: 0x%08x\n\r",
							ErrStatus);
		XCfupmc_CfuErrHandler(&CfupmcIns);
	}
	else if((ErrStatus & (CFU_APB_CFU_ISR_CFI_ROW_ERROR_MASK |
						CFU_APB_CFU_ISR_BAD_CFI_PACKET_MASK)) != 0U)
	{
		XCfupmc_CfiErrHandler(&CfupmcIns);
	}
	else {
		/** do nothing */
	}
END:
	return Status;
}
