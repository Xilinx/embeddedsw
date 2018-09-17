/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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

#if 0

#include <string.h>
#include "xparameters.h"
#include "xstatus.h"
#include "xvidframe_crc.h"

//#ifdef VIDEO_FRAME_CRC_EN

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

	XVidFrameCrc_WriteReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
			      VIDEO_FRAME_CRC_CONFIG,
			      (VIDEO_FRAME_CRC_CLEAR |
			       XPAR_VPHY_0_INPUT_PIXELS_PER_CLOCK));


	return (XST_SUCCESS);
}

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
void XVidFrameCrc_Reset(void)
{
	u32 RegVal;

	/* Read Config Register */
	RegVal = XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
				VIDEO_FRAME_CRC_CONFIG);

	/* Toggle CRC Clear Bit */
	XVidFrameCrc_WriteReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
			VIDEO_FRAME_CRC_CONFIG,
			(RegVal | VIDEO_FRAME_CRC_CLEAR));
	XVidFrameCrc_WriteReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
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
void XVidFrameCrc_Report(void)
 {
	xil_printf("------------\r\n");
	xil_printf("Video Frame CRC\n\r");
	xil_printf("------------\r\n\r\n");
	xil_printf("CRC PPC     =  %d\r\n",
		   XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
					VIDEO_FRAME_CRC_CONFIG)
		   & VIDEO_FRAME_CRC_PXLMODE_MASK);
	xil_printf("CRC - R/Cr   =  0x%x\r\n",
		   XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
					VIDEO_FRAME_CRC_VALUE_G_R)
		   & VIDEO_FRAME_CRC_R_Y_COMP_MASK);
	xil_printf("CRC - G/Y  =  0x%x\r\n",
		   (XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
					 VIDEO_FRAME_CRC_VALUE_G_R)
		    & VIDEO_FRAME_CRC_G_CR_COMP_MASK) >>
		   VIDEO_FRAME_CRC_G_CR_COMP_SHIFT);
	xil_printf("CRC - B/Cb  =  0x%x\r\n\r\n",
		   XVidFrameCrc_ReadReg(XPAR_VIDEO_FRAME_CRC_BASEADDR,
					VIDEO_FRAME_CRC_VALUE_B)
		   & VIDEO_FRAME_CRC_B_CB_COMP_MASK);
 }

#endif
