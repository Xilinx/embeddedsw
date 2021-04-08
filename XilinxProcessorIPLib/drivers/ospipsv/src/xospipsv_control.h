/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv_control.h
* @addtogroup ospipsv_v1_4
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

#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_LOWLEVEL_H_ */
/** @} */
