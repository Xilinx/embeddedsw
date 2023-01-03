/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_control.h
* @addtogroup ospipsv Overview
* @{
*
* This is the header file for the low-level functions of OSPIPSV driver.
* These functions will be used internally by the user API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.2   sk  02/20/20 First release
* 1.6   sk  02/07/22 Replaced driver version in addtogroup with Overview.
* 1.8   sk   11/29/22 Added support for Indirect Non-Dma write.
* 1.8   akm  01/03/23 Use Xil_WaitForEvent() API for register bit polling.
*
* </pre>
*
******************************************************************************/
#ifndef XOSPIPSV_LOWLEVEL_H_		/**< prevent circular inclusions */
#define XOSPIPSV_LOWLEVEL_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xospipsv.h"
#include "xil_util.h"
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief
* Disable the OSPIPSV device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
******************************************************************************/
static inline void XOspiPsv_Disable(const XOspiPsv *InstancePtr)
{
	u32 cfg_reg;

	cfg_reg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG);

	cfg_reg &= ~(u32)(XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK);
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress, XOSPIPSV_CONFIG_REG,
		cfg_reg);
}

/*****************************************************************************/
/**
* @brief
* Enable the OSPIPSV device.
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
******************************************************************************/
static inline void XOspiPsv_Enable(const XOspiPsv *InstancePtr)
{
	u32 ConfigReg = XOspiPsv_ReadReg(InstancePtr->Config.BaseAddress,
			XOSPIPSV_CONFIG_REG);

	ConfigReg |= XOSPIPSV_CONFIG_REG_ENB_SPI_FLD_MASK;
	XOspiPsv_WriteReg(InstancePtr->Config.BaseAddress,
		XOSPIPSV_CONFIG_REG, ConfigReg);
}

/*****************************************************************************/
/**
* @brief
* Configures the OSPI MUX to Linear mode
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
******************************************************************************/
static inline void XOspiPsv_ConfigureMux_Linear(const XOspiPsv *InstancePtr)
{
	XOspiPsv_Disable(InstancePtr);

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/*
	 * Execution is happening in non secure world, configure MUX
	 * settings through SMC calls
	 */

	/* Request for OSPI node */
	Xil_Smc(PM_REQUEST_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
	/* Change MUX settings to select LINEAR mode */
	Xil_Smc(PM_IOCTL_SMC_FID, (((u64)PM_IOCTL_OSPI_MUX_SELECT << 32) | OSPI_NODE_ID) , PM_OSPI_MUX_SEL_LINEAR, 0,0,0,0,0);
	/* Release OSPI node */
	Xil_Smc(PM_RELEASE_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0,0,0,0,0);
#else
	XOspiPsv_WriteReg(XPMC_IOU_SLCR_BASEADDR, XPMC_IOU_SLCR_OSPI_MUX_SEL,
		XOspiPsv_ReadReg(XPMC_IOU_SLCR_BASEADDR, XPMC_IOU_SLCR_OSPI_MUX_SEL) |
			(u32)XPMC_IOU_SLCR_OSPI_MUX_SEL_DAC_MASK);
#endif

	XOspiPsv_Enable(InstancePtr);
}

/*****************************************************************************/
/**
* @brief
* Configures the OSPI MUX to DMA mode
*
* @param	InstancePtr is a pointer to the XOspiPsv instance.
*
* @return	None.
*
******************************************************************************/
static inline void XOspiPsv_ConfigureMux_Dma(const XOspiPsv *InstancePtr)
{
	XOspiPsv_Disable(InstancePtr);

#if defined (__aarch64__) && (EL1_NONSECURE == 1)
	/* Request for OSPI node */
	Xil_Smc(PM_REQUEST_DEVICE_SMC_FID,OSPI_NODE_ID,0, 0, 0, 0, 0, 0);
	/* Change MUX settings to select DMA mode */
	Xil_Smc(PM_IOCTL_SMC_FID, (((u64)PM_IOCTL_OSPI_MUX_SELECT << 32) | OSPI_NODE_ID), PM_OSPI_MUX_SEL_DMA, 0, 0, 0, 0, 0);
	/* Release OSPI node */
	Xil_Smc(PM_RELEASE_DEVICE_SMC_FID,OSPI_NODE_ID, 0, 0, 0, 0, 0, 0);
#else
	XOspiPsv_WriteReg(XPMC_IOU_SLCR_BASEADDR, XPMC_IOU_SLCR_OSPI_MUX_SEL,
			XOspiPsv_ReadReg(XPMC_IOU_SLCR_BASEADDR, XPMC_IOU_SLCR_OSPI_MUX_SEL) &
					~(u32)XPMC_IOU_SLCR_OSPI_MUX_SEL_DAC_MASK);
#endif

	XOspiPsv_Enable(InstancePtr);
}

/************************** Function Prototypes ******************************/

u32 XOspiPsv_Stig_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_Stig_Write(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_Dac_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_Dac_Write(XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg);
u32 XOspiPsv_Dma_Read(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
void XOspiPsv_FifoRead(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
void XOspiPsv_FifoWrite(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
void XOspiPsv_Setup_Stig_Ctrl(const XOspiPsv *InstancePtr, u32 Cmd_op,
	u32 Rd_data_en, u32 Num_rd_data_bytes, u32 Cmd_addr_en, u32 Mode_bit_en,
	u32 Num_addr_bytes, u32 Wr_data_en, u32 Num_wr_data_bytes, u32 Dummy,
	u32 Membank_en);
void XOspiPsv_Setup_Dev_Write_Instr_Reg(const XOspiPsv *InstancePtr,
	const XOspiPsv_Msg *Msg);
void XOspiPsv_Setup_Dev_Read_Instr_Reg(const XOspiPsv *InstancePtr,
	const XOspiPsv_Msg *Msg);
void XOspiPsv_Setup_Devsize(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
void XOspiPsv_Start_Indr_RdTransfer(const XOspiPsv *InstancePtr);
void XOspiPsv_Config_IndirectAhb(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
void XOspiPsv_Config_Dma(const XOspiPsv *InstancePtr,
		const XOspiPsv_Msg *Msg);
u32 XOspiPsv_Exec_Dma(const XOspiPsv *InstancePtr);
u32 XOspiPsv_Exec_Flash_Cmd(const XOspiPsv *InstancePtr);
u32 XOspiPsv_CheckOspiIdle(const XOspiPsv *InstancePtr);
u32 XOspiPsv_WaitForLock(const XOspiPsv *InstancePtr, u32 Mask);
u32 XOspiPsv_ExecuteRxTuning(XOspiPsv *InstancePtr, XOspiPsv_Msg *FlashMsg,
								u32 TXTap);
u32 XOspiPsv_CalculateRxTap(XOspiPsv *InstancePtr, XOspiPsv_Msg *FlashMsg,
		u8 *AvgRXTap, u8 *MaxWindowSize, u8 DummyIncr, u32 TXTap);
u32 XOspiPsv_ConfigureTaps(const XOspiPsv *InstancePtr, u32 RxTap, u32 TxTap);
u32 XOspiPsv_IDac_Write(const XOspiPsv *InstancePtr, const XOspiPsv_Msg *Msg);

#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_LOWLEVEL_H_ */
/** @} */
