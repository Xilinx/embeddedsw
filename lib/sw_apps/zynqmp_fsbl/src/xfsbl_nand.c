/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xfsbl_nand.c
*
* This is the file which contains nand related code for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   04/21/14 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

#ifdef XFSBL_NAND

#include "xnandpsu.h"
#include "xnandpsu_bbm.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define NAND_DEVICE_ID          0
#define XFSBL_IMAGE_SEARCH_OFFSET 	0x8000

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/
XNandPsu_Config *Config;
XNandPsu NandInstance;                  /* XNand Instance */
XNandPsu *NandInstPtr = &NandInstance;


/*****************************************************************************/
/**
 * This function is used to initialize the qspi controller and driver
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_NandInit(void )
{
	u32 Status = XFSBL_SUCCESS;

	Config = XNandPsu_LookupConfig(NAND_DEVICE_ID);
        if (Config == NULL) {
                Status = XFSBL_ERROR_NAND_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_NAND_INIT\r\n");
                goto END;
        }
        /**
         * Initialize the NAND flash driver.
         */
        Status = (u32)XNandPsu_CfgInitialize(NandInstPtr, Config,
                        Config->BaseAddress);
        if (Status != XST_SUCCESS) {
                Status = XFSBL_ERROR_NAND_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_NAND_INIT\r\n");
                goto END;
        }

	XFsbl_Printf(DEBUG_INFO,"Nand Init Success\r\n");
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to copy the data from NAND to destination
 * address
 *
 * @param SrcAddress is the address of the NAND flash where copy should
 * start from
 *
 * @param DestAddress is the address of the destination where it
 * should copy to
 *
 * @param Length Length of the bytes to be copied
 *
 * @return
 * 		- XFSBL_SUCCESS for successful copy
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_NandCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length)
{
	u32 Status = XFSBL_SUCCESS;
	u32 MultiBootOffset=0U;
	u32 FlashImageOffsetAddress=0U;
	u32 CurrentBlock;
	u32 RealSrcAddress;
	u32 NoofBlocks;

	/**
	 * Read the Multiboot Register
	 */
	MultiBootOffset = XFsbl_In32(CSU_CSU_MULTI_BOOT);
	FlashImageOffsetAddress = MultiBootOffset * XFSBL_IMAGE_SEARCH_OFFSET;
	CurrentBlock = (u32)(FlashImageOffsetAddress/NandInstPtr->Geometry.BlockSize);
	NoofBlocks= ((SrcAddress - FlashImageOffsetAddress) / NandInstPtr->Geometry.BlockSize) + 1;
	RealSrcAddress=SrcAddress;

	while(NoofBlocks > 0)
	{
		if (XNandPsu_IsBlockBad(NandInstPtr, CurrentBlock) == XST_SUCCESS)
		{
			XFsbl_Printf(DEBUG_DETAILED, "Identified block (%d) as bad\r\n",
											CurrentBlock);
			RealSrcAddress= RealSrcAddress + NandInstPtr->Geometry.BlockSize ;
			XFsbl_Printf(DEBUG_DETAILED,
						"Src Address: %x, Calculated real Address:%x\r\n",
						SrcAddress, RealSrcAddress);
		}
		CurrentBlock+=1;
		NoofBlocks-=1;
	}

	Status = (u32)XNandPsu_Read(NandInstPtr, (u64)(RealSrcAddress),(u64)Length,
                                               (u8 *) DestAddress);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_NAND_READ;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_NAND_READ\r\n");
                goto END;
        }
END:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to release the nand settings
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_NandRelease(void )
{

	return XFSBL_SUCCESS;
}


#endif /*  end of #ifdef XFSBL_NAND */
