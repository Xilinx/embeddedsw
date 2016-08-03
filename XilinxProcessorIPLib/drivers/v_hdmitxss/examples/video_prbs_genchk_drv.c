/*
 * Copyright (c) 2016 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*****************************************************************************/
/**
 *
 * @file video_prbs_genchk_drv.c
 *
 * This file contains the driver implementation for the Video PRBS generator
 * and checker used in the HDMI example design.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- --- ----------   -----------------------------------------------
 * X.XX  XX  YYYY/MM/DD   ...
 * 1.00  RHe 2015/05/08   First release
 * </pre>
 *
 ******************************************************************************/

#include "video_prbs_genchk_drv.h"

/*****************************************************************************/
/**
*
* This function initializes the Video PRBS Generator/Checker.
* This function must be called prior to using the Generator/Checker.
*
* @param  InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
* @param  PRBSGenChkBase is the base address for the core.
*
* @return
*  - XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XVideoPRBSGenChk_Init (XVideoPRBSGenChk_t *InstancePtr, u32 PRBSGenChkBase)
{
  InstancePtr->PRBSGenChkBase = PRBSGenChkBase;

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the Generator/Checker.
*
* @param InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
* @param setclr specifies TRUE/FALSE value to either assert or
*        release Generator/Checker reset.
*
* @return
*  - XST_SUCCESS if action was successful.
*
* @note		None.
*
******************************************************************************/
int XVideoPRBSGenChk_Enable (XVideoPRBSGenChk_t *InstancePtr, u8 setclr)
{
  u32 data;

  data = XVideo_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);

  if (setclr)
    data |= (1 << PRBS_CTRL_REG_ENABLE_SHIFT);
  else
    data &= ~(PRBS_CTRL_REG_ENABLE_MASK << PRBS_CTRL_REG_ENABLE_SHIFT);

  XVideo_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the pixel configuration.
*
* @param InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
* @param PixPerClk specifies the number of pixels per clock.
*
* @return
*  - XST_SUCCESS if action was successful.
*  - XST_FAILURE if invalid pixel configuration is requested.
*
* @note		None.
*
******************************************************************************/
int XVideoPRBSGenChk_SetPixCfg (XVideoPRBSGenChk_t *InstancePtr, XVidC_PixelsPerClock PixPerClk)
{
  u32 data;
  u8 value;

  data = XVideo_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);
  data &= ~(PRBS_CTRL_REG_PIXCFG_MASK << PRBS_CTRL_REG_PIXCFG_SHIFT);

  switch (PixPerClk)
  {
    case (XVIDC_PPC_1) :
       value = 0;
       break;

    case (XVIDC_PPC_2) :
       value = 1;
       break;

    case (XVIDC_PPC_4) :
       value = 2;
       break;

    default :
      xil_printf("VideoPRBSGenChk: Invalid pixels per clock specified\n\r");
      return XST_FAILURE;
  }

  data |= ((value & PRBS_CTRL_REG_PIXCFG_MASK) << PRBS_CTRL_REG_PIXCFG_SHIFT);

  XVideo_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the bits per color configuration.
*
* @param InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
* @param BitsPerColor specifies the number of bits per color component per pixel.
*
* @return
*  - XST_SUCCESS if action was successful.
*  - XST_FAILURE if invalid bits per color configuration is requested.
*
* @note		None.
*
******************************************************************************/
int XVideoPRBSGenChk_SetBpcCfg (XVideoPRBSGenChk_t *InstancePtr, XVidC_ColorDepth BitsPerColor)
{
  u32 data;
  u8 value;

  data = XVideo_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);
  data &= ~(PRBS_CTRL_REG_BPCCFG_MASK << PRBS_CTRL_REG_BPCCFG_SHIFT);

  switch (BitsPerColor)
  {
    case (XVIDC_BPC_8) :
      value = 0;
      break;

    case (XVIDC_BPC_10) :
      value = 1;
      break;

    case (XVIDC_BPC_12) :
      value = 2;
      break;

    case (XVIDC_BPC_16) :
      value = 3;
      break;

    default :
      xil_printf("VideoPRBSGenChk: Invalid bits per color specified\n\r");
      return XST_FAILURE;
  }

  data |= ((value & PRBS_CTRL_REG_BPCCFG_MASK) << PRBS_CTRL_REG_BPCCFG_SHIFT);

  XVideo_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the pixel error count from the checker.
*
* @param InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
*
* @return The number of pixel errors found
*
* @note The count value does not wrap after reaching 0xFFFFFFFF.
*
******************************************************************************/
u32 XVideoPRBSGenChk_GetErrCnt (XVideoPRBSGenChk_t *InstancePtr)
{
  return XVideo_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_ERR_CNT);
}

/*****************************************************************************/
/**
*
* This function clears the pixel error count for the checker.
*
* @param InstancePtr is a pointer to the XVideoPRBSGenChk_t core instance.
*
* @return
*  - XST_SUCCESS if action was successful.
*
* @note
*
******************************************************************************/
int XVideoPRBSGenChk_ClrErrCnt (XVideoPRBSGenChk_t *InstancePtr)
{
  // Any write to the error count register clears it.
  XVideo_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_ERR_CNT, 0);

  return XST_SUCCESS;
}
