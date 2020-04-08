/*******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xvidframe_crc.c
*
* This is the main header file for the Xilinx HDCP abstraction layer.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00  GM   17/07/17 First Release
*</pre>
*
*****************************************************************************/

#include <string.h>
#include "xparameters.h"
#include "xstatus.h"
#include "xvidframe_crc.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* This function is used to initialize the Video Frame CRC instance.
*
* @param    None
*
* @return   - XST_SUCCESS or XST_FAILURE
*
* @note	    None.
*
******************************************************************************/
int XVidFrameCrc_Initialize(Video_CRC_Config *VidFrameCRC)
{

	/* Initialize CRC & Set default Pixel Mode */
	VidFrameCRC->TEST_CRC_SUPPORTED = 1;

	XVidFrameCrc_WriteReg(VidFrameCRC->Base_Addr,
			VIDEO_FRAME_CRC_CONFIG,
			(VIDEO_FRAME_CRC_CLEAR |
			 XPAR_VPHY_0_INPUT_PIXELS_PER_CLOCK));


  return (XST_SUCCESS);
}

/**
*
* This function is used to initialize the Video Frame CRC instance.
*
* @param    None
*
* @return   - XST_SUCCESS or XST_FAILURE
*
* @note	    None.
*
******************************************************************************/
void XVidFrameCrc_Reset(Video_CRC_Config *VidFrameCRC)
{
	u32 RegVal;

	/* Read Config Register */
	RegVal = XVidFrameCrc_ReadReg(VidFrameCRC->Base_Addr,
				VIDEO_FRAME_CRC_CONFIG);

	/* Toggle CRC Clear Bit */
	XVidFrameCrc_WriteReg(VidFrameCRC->Base_Addr,
			VIDEO_FRAME_CRC_CONFIG,
			(RegVal | VIDEO_FRAME_CRC_CLEAR));
	XVidFrameCrc_WriteReg(VidFrameCRC->Base_Addr,
			VIDEO_FRAME_CRC_CONFIG,
			(RegVal & ~VIDEO_FRAME_CRC_CLEAR));

}

/*****************************************************************************/
/**
*
* This function reports CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XVidFrameCrc_Report(Video_CRC_Config *VidFrameCRC)
 {
	xil_printf("------------\r\n");
	xil_printf("Video Frame CRC\n\r");
	xil_printf("------------\r\n\r\n");
	xil_printf("CRC PPC     =  %d\r\n",
		   XVidFrameCrc_ReadReg(VidFrameCRC->Base_Addr,
					VIDEO_FRAME_CRC_CONFIG)
		   & VIDEO_FRAME_CRC_PXLMODE_MASK);
	xil_printf("CRC - R/Cr   =  0x%x\r\n",
		   XVidFrameCrc_ReadReg(VidFrameCRC->Base_Addr,
					VIDEO_FRAME_CRC_VALUE_G_R)
		   & VIDEO_FRAME_CRC_R_Y_COMP_MASK);
	xil_printf("CRC - G/Y  =  0x%x\r\n",
		   (XVidFrameCrc_ReadReg(VidFrameCRC->Base_Addr,
					 VIDEO_FRAME_CRC_VALUE_G_R)
		    & VIDEO_FRAME_CRC_G_CR_COMP_MASK) >>
		   VIDEO_FRAME_CRC_G_CR_COMP_SHIFT);
	xil_printf("CRC - B/Cb  =  0x%x\r\n\r\n",
		   XVidFrameCrc_ReadReg(VidFrameCRC->Base_Addr,
					VIDEO_FRAME_CRC_VALUE_B)
		   & VIDEO_FRAME_CRC_B_CB_COMP_MASK);
 }
