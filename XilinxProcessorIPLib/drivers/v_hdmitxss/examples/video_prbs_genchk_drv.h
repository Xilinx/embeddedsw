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
 * @file video_prbs_genchk_drv.h
 *
 * This file contains the definitions for the Video PRBS generator and checker
 * used in the HDMI example design.
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

#ifndef VIDEO_PRBS_GENCHK_DRV_H_
#define VIDEO_PRBS_GENCHK_DRV_H_

#include "xvidc.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

typedef struct {
  u32 PRBSGenChkBase;
} XVideoPRBSGenChk_t;

int XVideoPRBSGenChk_Init(XVideoPRBSGenChk_t *InstancePtr, u32 PRBSGenChkBase);
int XVideoPRBSGenChk_Enable(XVideoPRBSGenChk_t *InstancePtr, u8 setclr);
int XVideoPRBSGenChk_SetPixCfg(XVideoPRBSGenChk_t *InstancePtr, XVidC_PixelsPerClock PixPerClk);
int XVideoPRBSGenChk_SetBpcCfg(XVideoPRBSGenChk_t *InstancePtr, XVidC_ColorDepth BitsPerColor);
u32 XVideoPRBSGenChk_GetErrCnt(XVideoPRBSGenChk_t *InstancePtr);
int XVideoPRBSGenChk_ClrErrCnt(XVideoPRBSGenChk_t *InstancePtr);

#define XVideo_PRBS_GenChk_In32   Xil_In32  /**< Input Operations */
#define XVideo_PRBS_GenChk_Out32  Xil_Out32 /**< Output Operations */

/*****************************************************************************/
/**
 *
 * This function macro reads the given register.
 *
 * @param  BaseAddress is the base address of the Video PRBS Generator/Checker core.
 * @param  RegOffset is the register offset of the register
 *
 * @return The 32-bit value of the register.
 *
 * @note   C-style signature:
 *   u32 XVideo_PRBS_GenChk_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 ******************************************************************************/
#define XVideo_PRBS_GenChk_ReadReg(BaseAddress, RegOffset) \
    XVideo_PRBS_GenChk_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
 *
 * Write the given register.
 *
 * @param  BaseAddress is the base address of the Video PRBS Generator/Checker core.
 * @param  RegOffset is the register offset of the register
 * @param  Data is the 32-bit value to write to the register.
 *
 * @return None.
 *
 * @note   C-style signature:
 *   void XVideo_PRBS_GenChk_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 ******************************************************************************/
#define XVideo_PRBS_GenChk_WriteReg(BaseAddress, RegOffset, Data)   \
    XVideo_PRBS_GenChk_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

#define PRBS_VER     0x000 // Video PRBS Generator/Checker Version Register#define PRBS_CTRL    0x004 // Video PRBS Generator/Checker Control Register#define PRBS_ERR_CNT 0x008 // Video PRBS Checker Error Count Register

// Video PRBS Generator/Checker Control Register
#define PRBS_CTRL_REG_ENABLE_MASK     0x1
#define PRBS_CTRL_REG_ENABLE_SHIFT    0
#define PRBS_CTRL_REG_PIXCFG_MASK     0x3
#define PRBS_CTRL_REG_PIXCFG_SHIFT    1
#define PRBS_CTRL_REG_BPCCFG_MASK     0x3
#define PRBS_CTRL_REG_BPCCFG_SHIFT    3

#define PRBS_ERR_CNT_REG_MASK     0xFFFFFFFF
#define PRBS_ERR_CNT_REG_SHIFT    0


#endif /* VIDEO_PRBS_GENCHK_DRV_H_ */
