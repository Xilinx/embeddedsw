/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2stx_selftest.c
 * @addtogroup i2stx Overview
 * @{
 *
 * Contains an basic sef-test API that displays the version of the
 * I2S Transmitter device. Additionally, it also verifies and displays
 * the configuration of the drivers for i2s data width, if the transmitter
 * core IsMaster, and the maximum channels supported by the I2S device.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * 2.0   kar    10/01/18 included xil_types and xil_assert header files.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2stx.h"
#include "xparameters.h"
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XI2S_TX_CORE_VER (0x00010000)
/*****************************************************************************/
/**
 *
 * This macro reads the maximum number of I2S channels available.
 *
 * @param  InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return Maximum number of I2S Channels.
 *
 * @note C-style signature:
 *   u32 XI2s_Tx_GetMaxChannels(XI2s_Tx *InstancePtr)
 *
 *****************************************************************************/
#define XI2s_Tx_GetMaxChannels(InstancePtr) \
((XI2s_Tx_ReadReg((InstancePtr)->Config.BaseAddress, (XI2S_TX_CORE_CFG_OFFSET))\
	  & XI2S_TX_REG_CFG_NUM_CH_MASK) >> XI2S_TX_REG_CFG_NUM_CH_SHIFT)

/*****************************************************************************/
/**
 *
 * This macro returns the I2S operating mode.
 *
 * @param  InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return
 *   - TRUE  : is I2S Master
 *   - FALSE : is I2S Slave
 *
 * @note C-style signature:
 *   u8 XI2s_Tx_IsI2sMaster(XI2s_Tx *InstancePtr)
 *
 *****************************************************************************/
#define XI2s_Tx_IsI2sMaster(InstancePtr) \
	((XI2s_Tx_ReadReg((InstancePtr)->Config.BaseAddress, \
			  (XI2S_TX_CORE_CFG_OFFSET)) \
	  & XI2S_TX_REG_CFG_MSTR_MASK) ? TRUE : FALSE)
/*****************************************************************************/
/**
 *
 * This macro returns the I2S data width.
 *
 * @param  InstancePtr is a pointer to the XI2s_Tx core instance.
 *
 * @return The I2s data width in number of bits
 *
 * @note C-style signature:
 *   u8 XI2s_Tx_GetI2sDataWidth(XI2s_Tx *InstancePtr)
 *
 *****************************************************************************/
static inline u32 XI2s_Tx_GetXI2sDataWidth(XI2s_Tx *InstancePtr) {
	u32 I2stx_Dw;
	I2stx_Dw = XI2s_Tx_ReadReg((InstancePtr)->Config.BaseAddress,
			(XI2S_TX_CORE_CFG_OFFSET));
	I2stx_Dw = I2stx_Dw & XI2S_TX_REG_CFG_DWDTH_MASK;
	if (I2stx_Dw == 0)
		I2stx_Dw = 16;
	else I2stx_Dw = 24;
	return I2stx_Dw;
}
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * Runs a self-test on the driver/device. The self-test is reads the version
 * register, configuration registers and verifies the values
 *
 * @param	InstancePtr is a pointer to the XI2s_Tx instance.
 *
 * @return
 *		- XST_SUCCESS if successful i.e. if the self test passes.
 *		- XST_FAILURE if unsuccessful i.e. if the self test fails
 *
 * @note	None.
 *
 *****************************************************************************/
int XI2s_Tx_SelfTest(XI2s_Tx *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 I2s_Tx_version, I2s_Tx_IsMaster;
	u32 I2s_Tx_DWidth, I2s_Tx_MaxNumChannels;

	/* Read the core version register */
	I2s_Tx_version = XI2s_Tx_GetVersion(InstancePtr);
	if (I2s_Tx_version != XI2S_TX_CORE_VER) {
		xil_printf("Self-test for I2S Transmitter Version %x.\r\n",
				I2s_Tx_version);
		return XST_FAILURE;
	}
	/*
	 * Read the core configuration registers for maximum channels,
	 * I2S data width and if the I2S Transmitter is master or not.
	 */
	I2s_Tx_IsMaster = XI2s_Tx_IsI2sMaster(InstancePtr);
	if (I2s_Tx_IsMaster != XPAR_XI2STX_0_IS_MASTER)
		return XST_FAILURE;

	I2s_Tx_MaxNumChannels = XI2s_Tx_GetMaxChannels(InstancePtr);
	if (I2s_Tx_MaxNumChannels != (2*XPAR_XI2STX_0_NUM_CHANNELS)) {
		xil_printf("Num of channels configured "
				"in the core in the core_cfg (%d) "
				"doesn't match the configuration of the "
				"core in the design (%d).\r\n",
				I2s_Tx_MaxNumChannels,
				(2*XPAR_XI2STX_0_NUM_CHANNELS));
		return XST_FAILURE;
	}

	I2s_Tx_DWidth = XI2s_Tx_GetXI2sDataWidth(InstancePtr);
	if (I2s_Tx_DWidth != XPAR_XI2STX_0_DWIDTH) {
		xil_printf("Data Width configured "
				"in the core in the core_cfg (%d) "
				"doesn't match the configuration of the "
				"core in the design (%d).\r\n", I2s_Tx_DWidth,
				XPAR_XI2STX_0_DWIDTH);
		return XST_FAILURE;
	}
	return Status;
}
/** @} */
