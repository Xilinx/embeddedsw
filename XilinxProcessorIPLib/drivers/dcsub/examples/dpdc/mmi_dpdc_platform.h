/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mmi_dpdc_platform.h
*
* This header file contains platform initialization function declarations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who     Date      Changes
* ---- ---     --------  --------------------------------------------------
* 1.00 arm     04/02/26  Initial version
*
* </pre>
*
******************************************************************************/

#ifndef MMI_DPDC_PLATFORM_H_
#define MMI_DPDC_PLATFORM_H_	/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xdcsub.h"
#include "mmi_dc_nonlive_test.h"

/************************** Constant Definitions *****************************/

#define DC_BASEADDR            	0xEDD00000	/**< DC base address */
#define DCDMA_BASEADDR         	0xEDD10000	/**< DC DMA base address */
#define DP_BASEADDR   		    0xEDE00000	/**< DP base address */

/** @name MMI CRX registers
 *  @{
 */

/** MMI CRX base address */
#define XMMI_CRX_BASEADDR			0xEDC00000

/** WPROTS write protection register */
#define XMMI_CRX_WPROTS				0x00000030
/** WPROTP write protection register */
#define XMMI_CRX_WPROTP				0x00000034
/** WPROTS/WPROTP value that disables write protection */
#define XMMI_CRX_WPROT_DISABLE			0x00000000

/** DC reset control register */
#define XMMI_CRX_RST_DC				0x00000318

/** RST_DC: DC block reset */
#define XMMI_CRX_RST_DC_BLK_MASK		0x00000001
/** RST_DC: DC subsystem configuration POR */
#define XMMI_CRX_RST_DC_SS_CFG_POR_MASK		0x00000002
/** RST_DC: all DC reset bits */
#define XMMI_CRX_RST_DC_MASK			(XMMI_CRX_RST_DC_BLK_MASK | \
						 XMMI_CRX_RST_DC_SS_CFG_POR_MASK)
/** RST_DC: all DC resets released */
#define XMMI_CRX_RST_DC_RELEASE			0x00000000
/* @} */

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
 * This macro reads the specified MMI CRX register.
 *
 * @param	BaseAddress is the base address of MMI CRX.
 * @param	RegOffset is the register offset to be read from.
 *
 * @return	The 32-bit value of the specified register.
 *
 * @note	C-style signature:
 *		u32 XMmiCrx_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 *****************************************************************************/
#define XMmiCrx_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
 * This macro writes the specified MMI CRX register.
 *
 * @param	BaseAddress is the base address of MMI CRX.
 * @param	RegOffset is the register offset to write to.
 * @param	Data is the 32-bit data to write to the specified register.
 *
 * @return	None.
 *
 * @note	C-style signature:
 *		void XMmiCrx_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 *****************************************************************************/
#define XMmiCrx_WriteReg(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (Data))

/************************** Variable Definitions *****************************/

/* External descriptor declarations */
extern XDcDma_Descriptor *AudDesc0;

/************************** Function Prototypes ******************************/

u32 XClk_WaitForLock(XClk_Wiz_Config *CfgPtr);
u32 XDpDc_AssertDcReset(void);
u32 XDpDc_ReleaseDcReset(void);
u32 XDpDc_InitClkWiz(RunConfig *RunCfgPtr);
u32 XDpDc_InitDcSubsystem(RunConfig *RunCfgPtr);
void XDpDc_SetupInterrupts(RunConfig *RunCfgPtr);
u32 XDpDc_InitPlatform(RunConfig *RunCfgPtr);

#ifdef __cplusplus
}
#endif

#endif /* MMI_DPDC_PLATFORM_H_ */
