/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcfupmc.h
*
* This is the file which contains code for CFU block.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   12/21/2017 Initial release
* 2.00  bsv  03/01/2019 Added error handling APIs
* 2.01  bsv  11/06/2019 XCfupmc_ClearCfuIsr API added
* 3.00  bsv  06/27/2020 Code clean up
* 4.00  ma   06/17/2021 Added defines for CFU_STREAM_2 and CFU_FDRO_2
*                       base addresses
*       bsv  07/15/2021 Fix doxygen warnings
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XCFUPMC_H
#define XCFUPMC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#include "cfu_apb.h"
#include "xcfupmc_hw.h"

/************************** Constant Definitions *****************************/
/**@cond cfupmc_internal
 * @{
 */

/* CFU key hole register address */
/* Address updates after RTL HW40 */
#define CFU_STREAM_ADDR	(0xF12C0000U)
#define CFU_FDRO_ADDR	(0xF12C2000U)
#define CFU_STREAM_2_ADDR	(0xF1F80000U)
#define CFU_FDRO_2_ADDR		(0xF1FC0000U)
#define CFUPMC_GLB_SIG_EN	(0x1U)
#define CFUPMC_PROT_DISABLE	(0x0U)
#define CFUPMC_PROT_ENABLE	(0x1U)
#define CFU_APB_CFU_FGCR_GRESTORE_CLEAR	(0x0U)
#define CFU_APB_CFU_FGCR_EN_GLOBS_B_ASSERT_VAL	(0x0U)
#define CFU_APB_CFU_CTL_CFRAME_DISABLE_CLR_VAL	(0x0U)
#define CFU_APB_CFU_CTL_CFI_LOCAL_RESET_CLR_VAL	(0x0U)
#define CFU_APB_CFU_CTL_SEU_GO_CLR_VAL	(0x0U)
#define CFU_APB_CFU_CTL_DECOMPRESS_CLR_VAL	(0x0U)
#define CFU_APB_CFU_CTL_IGNORE_CFI_ERROR_CLR_VAL	(0x0U)
#define CFU_APB_CFU_CTL_DECOMPRESS_CLR_VAL	(0x0U)

/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/**
* This typedef contains configuration information for a CFU core.
* Each CFU core should have a configuration structure associated.
*/
typedef struct {
	u16 DeviceId;		/**< DeviceId is the unique ID of the
					*  device */
	u32 BaseAddress;	/**< BaseAddress is the physical base address
					*  of the device's registers */
} XCfupmc_Config;

/******************************************************************************/
/**
*
* The XCfupmc driver instance data structure. A pointer to an instance data
* structure is passed around by functions to refer to a specific driver
* instance.
*/
typedef struct {
	XCfupmc_Config Config;		/**< Hardware configuration */
	u32 IsReady;			/**< Device and the driver instance
						*  are initialized */
	u8 DeCompress;			/**< Indicates whether compression is
						* is enabled or not */
	u8 Crc32Check;			/**< Indicates if CRC32 is enabled
						* or not */
	u32 Crc32Val;			/**< Checksum value */
	u8 Crc8Dis;			/**< Indicates if CRC8 is enabled
						* or not */
}XCfupmc;

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This function clears CFU ISR
*
* @param	InstancePtr is a pointer to the XCfupmc instance.
* @param	IsrMask specifies the bits to be cleared in ISR
*
* @return	None
*
******************************************************************************/
static inline void XCfupmc_ClearIsr(const XCfupmc *InstancePtr, u32 IsrMask)
{
	(void)InstancePtr;
	XCfupmc_WriteReg(CFU_APB_CFU_ISR, IsrMask);
}

/*****************************************************************************/
/**
* This function reads CFU ISR
*
* @param	InstancePtr is a pointer to the XCfupmc instance._
*
* @return	32-bit register value
*
******************************************************************************/
static inline u32 XCfupmc_ReadIsr(const XCfupmc *InstancePtr)
{
	(void)InstancePtr;

	return XCfupmc_ReadReg(CFU_APB_CFU_ISR);
}

/*****************************************************************************/
/**
* This function reads CFU Status
*
* @param	InstancePtr is a pointer to the XCfupmc instance.
*
* @return	32-bit register value
*
******************************************************************************/
static inline u32 XCfupmc_ReadStatus(const XCfupmc *InstancePtr)
{
	(void)InstancePtr;

	return XCfupmc_ReadReg(CFU_APB_CFU_STATUS);
}

/**@cond cfu_apb_internal
 * @{
 */

#ifdef XCFUPMC_DEBUG
#define XCfupmc_Printf(...)	xil_printf(__VA_ARGS__)
#else
#define XCfupmc_Printf(...)
#endif

/**
 * @}
 * @endcond
 */

/************************** Function Prototypes ******************************/
XCfupmc_Config *XCfupmc_LookupConfig(u16 DeviceId);
s32 XCfupmc_CfgInitialize(XCfupmc *InstancePtr, const XCfupmc_Config *CfgPtr,
	u32 EffectiveAddr);
s32 XCfupmc_SelfTest(const XCfupmc *InstancePtr);
void XCfupmc_MaskRegWrite(const XCfupmc *InstancePtr, u32 Addr, u32 Mask,
	u32 Val);
void XCfupmc_SetGlblSigEn(const XCfupmc *InstancePtr, u8 Enable);
void XCfupmc_GlblSeqInit(const XCfupmc *InstancePtr);
void XCfupmc_CfuErrHandler(const XCfupmc *InstancePtr);
void XCfupmc_CfiErrHandler(const XCfupmc *InstancePtr);
void XCfupmc_ExtErrorHandler(const XCfupmc *InstancePtr);
void XCfupmc_ClearIgnoreCfiErr(const XCfupmc *InstancePtr);
void XCfupmc_ClearCfuIsr(const XCfupmc *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif  /* XCFUPMC_H */
