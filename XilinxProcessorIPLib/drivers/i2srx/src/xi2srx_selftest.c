/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2srx_selftest.c
 * @addtogroup i2srx_v2_2
 * @{
 * Contains an basic self-test API that displays the version of the
 * I2S Receiver device. Additionally, it also verifies and displays
 * the configuration of the drivers for i2s data width, if the Receiver
 * core IsMaster, and the maximum channels supported by the I2s device.
 *
 * @note None
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date       Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0   kar    01/25/18   Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xi2srx_hw.h"
#include "xi2srx_debug.h"
#include "xstatus.h"
/************************** Constant Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XI2S_RX_CORE_VER (0x00010000)
/*****************************************************************************/
/**
 *
 * This macro reads the maximum number of XI2s channels available.
 *
 * @param  InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return Maximum number of XI2s Channels.
 *
 * @note C-style signature:
 *   u32 XI2s_Rx_GetMaxChannels(XI2s_Rx *InstancePtr)
 *
 *****************************************************************************/
#define XI2s_Rx_GetMaxChannels(InstancePtr) \
((XI2s_Rx_ReadReg((InstancePtr)->Config.BaseAddress, (XI2S_RX_CORE_CFG_OFFSET))\
	  & XI2S_RX_REG_CFG_NUM_CH_MASK) >> XI2S_RX_REG_CFG_NUM_CH_SHIFT)

/*****************************************************************************/
/**
 *
 * This macro returns the XI2s operating mode.
 *
 * @param  InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return
 *   - TRUE  : is XI2s Master
 *   - FALSE : is XI2s Slave
 *
 * @note C-style signature:
 *   u8 XI2s_Rx_IsXI2sMaster(XI2s_Rx *InstancePtr)
 *
 *****************************************************************************/
#define XI2s_Rx_IsXI2sMaster(InstancePtr) \
((XI2s_Rx_ReadReg((InstancePtr)->Config.BaseAddress, (XI2S_RX_CORE_CFG_OFFSET))\
	  & XI2S_RX_REG_CFG_MSTR_MASK) ? TRUE : FALSE)

/*****************************************************************************/
/**
 *
 * This inline function returns the XI2s data width.
 *
 * @param  InstancePtr is a pointer to the XI2s_Rx core instance.
 *
 * @return The XI2s data width in number of bits
 *
 *****************************************************************************/
static inline u32 XI2s_Rx_GetXI2sDataWidth(XI2s_Rx *InstancePtr) {
	u32 I2srx_Dw;
	I2srx_Dw = XI2s_Rx_ReadReg((InstancePtr)->Config.BaseAddress,
			(XI2S_RX_CORE_CFG_OFFSET));
	I2srx_Dw = I2srx_Dw & XI2S_RX_REG_CFG_DWDTH_MASK;
	if(I2srx_Dw == 0)
		I2srx_Dw = 16;
	else I2srx_Dw = 24;
	return I2srx_Dw;
}
/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * Runs a self-test on the driver/device.
 * The self-test  reads the version register,
 * data width , max channels, Master or slave configuration and
 * verifies the values
 *
 * @param	InstancePtr is a pointer to the XI2s_Rx instance.
 *
 * @return
 *		- XST_SUCCESS if successful i.e. if the self test passes.
 *		- XST_FAILURE if unsuccessful i.e. if the self test fails
 *
 * @note		None.
 *
 *****************************************************************************/
int XI2s_Rx_SelfTest(XI2s_Rx *InstancePtr) {
	int Status = XST_SUCCESS;
	u32 I2s_Rx_version, I2s_Rx_IsMaster;
	u32 I2s_Rx_DWidth, I2s_Rx_MaxNumChannels;

	/* Read the core version register */
	I2s_Rx_version = XI2s_Rx_GetVersion(InstancePtr);
	if (I2s_Rx_version != XI2S_RX_CORE_VER) {
		xil_printf("Self-test for I2S Receiver Version %x.\r\n",
			       	I2s_Rx_version);
		return XST_INVALID_VERSION;
	}

	/* Read the core configuration registers for maximum channels,
	 * I2S data width and if the I2S Receiver is master or not.
	 */
	I2s_Rx_IsMaster = XI2s_Rx_IsXI2sMaster(InstancePtr);
	if (I2s_Rx_IsMaster != XPAR_XI2SRX_0_IS_MASTER) {

		xil_printf("IsMaster configuration of core in core_cfg (%d) "
				"doesn't match configuration of the core in "
				"the design (%d).\r\n", I2s_Rx_IsMaster,
				XPAR_XI2SRX_0_IS_MASTER);
		return XST_FAILURE;
	}

	I2s_Rx_MaxNumChannels = XI2s_Rx_GetMaxChannels(InstancePtr);
	if (I2s_Rx_MaxNumChannels != (2*XPAR_XI2SRX_0_NUM_CHANNELS)) {
		xil_printf("Num of channels configured in the "
				"core in the core_cfg (%d) "
				"doesn't match configuration of the core in "
				"the design (%d).\r\n", I2s_Rx_MaxNumChannels,
				(2*XPAR_XI2SRX_0_NUM_CHANNELS));
		return XST_FAILURE;
	}

	I2s_Rx_DWidth = XI2s_Rx_GetXI2sDataWidth(InstancePtr);
	if (I2s_Rx_DWidth != XPAR_XI2SRX_0_DWIDTH) {
		xil_printf("Data Width configured in the core "
				"in the core_cfg (%d) "
				"doesn't match configuration of the core in "
				"the design (%d).\r\n", I2s_Rx_DWidth,
				XPAR_XI2SRX_0_DWIDTH);
		return XST_FAILURE;
	}
	return Status;
}
/** @} */
