/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* @file xfsbl_board.c
*
* This file contains board specific code of FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ssc  01/20/16 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_board.h"
#ifdef XPS_BOARD_ZCU102
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static s32 XFsbl_BoardConfig(void);
static void XFsbl_UsbPhyReset(void);
static void XFsbl_PcieReset(void);
/************************** Variable Definitions *****************************/
XIicPs I2c0InstancePtr;

/*****************************************************************************/
/**
 * This function is used to perform GT configuration for ZCU102 board.
 * It also provides reset to GEM, enables FMC ADJ
 *
 * @param none
 *
 * @return
 * 		- XFSBL_SUCCESS for successful configuration
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
static s32 XFsbl_BoardConfig(void)
{

	u8 WriteBuffer[BUF_LEN] = {0};
	XIicPs_Config *I2c0CfgPtr;
	s32 Status = XFSBL_SUCCESS;
	u32 ICMCfg0;
	u32 ICMCfg1;

	/* Initialize the IIC0 driver so that it is ready to use */
	I2c0CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (I2c0CfgPtr == NULL) {
		Status = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	Status = XIicPs_CfgInitialize(&I2c0InstancePtr, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	/* Set the IIC serial clock rate */
	XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_IOEXP);

	/* Configure I/O pins as Output */
	WriteBuffer[0] = CMD_CFG_0_REG;
	WriteBuffer[1] = DATA_OUTPUT;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle to start another transfer */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr));

	/*
	 * Deasserting I2C_MUX_RESETB
	 * And GEM3 Resetb
	 * Selecting lanes based on configuration
	 */
	WriteBuffer[0] = CMD_OUTPUT_0_REG;

	/* Populate WriteBuffer[1] based on ICM_CFG configuration */
	ICMCfg0 = XFsbl_In32(SERDES_ICM_CFG0) &
			(SERDES_ICM_CFG0_L0_ICM_CFG_MASK | SERDES_ICM_CFG0_L1_ICM_CFG_MASK);

	ICMCfg1 = XFsbl_In32(SERDES_ICM_CFG1) &
			(SERDES_ICM_CFG1_L2_ICM_CFG_MASK | SERDES_ICM_CFG1_L3_ICM_CFG_MASK);

	if ((ICMCfg0 == ICM_CFG0_PCIE_DP) && (ICMCfg1 == ICM_CFG1_USB_SATA))
	{
		/* gt1110 */
		WriteBuffer[1] = DATA_COMMON_CFG | DATA_GT_1110_CFG;
	}
	else
	if ((ICMCfg0 == ICM_CFG0_DP_DP) && (ICMCfg1 == ICM_CFG1_USB_SATA))
	{
		/* gt1111 */
		WriteBuffer[1] = DATA_COMMON_CFG | DATA_GT_1111_CFG;
	}
	else
	if ((ICMCfg0 == ICM_CFG0_PCIE_PCIE) && (ICMCfg1 == ICM_CFG1_USB_SATA))
	{
		/* gt1100 */
		WriteBuffer[1] = DATA_COMMON_CFG | DATA_GT_1100_CFG;
	}
	else
	{
		/* gt0000 or no GT configuration */
		WriteBuffer[1] = DATA_COMMON_CFG | DATA_GT_0000_CFG;
	}

	/* Send the Data */
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr));

	/* Change the IIC serial clock rate */
	XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_I2CMUX);

	/* Set I2C Mux for channel-2 (MAXIM_PMBUS) */
	WriteBuffer[0] = CMD_CH_2_REG;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 1, PCA9544A_ADDR);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr));

	/* Enable Regulator (FMC ADJ) */
	WriteBuffer[0] = CMD_ON_OFF_CFG;
	WriteBuffer[1] = ON_OFF_CFG_VAL;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, MAX15301_ADDR);
	if (Status != XST_SUCCESS) {
		Status = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr));

	XFsbl_Printf(DEBUG_INFO,"Board Configuration successful \n\r");

END:

	return Status;

}

/*****************************************************************************/
/**
 * This function is used to provide Reset to USB Phy on ZCU102 board.
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XFsbl_UsbPhyReset(void)
{

	/* USB PHY Reset */
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);
	(void)usleep(DELAY_1_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_LO);
	(void)usleep(DELAY_5_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);

}

/*****************************************************************************/
/**
 * This function is used to provide PCIe reset on ZCU102 board.
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
void XFsbl_PcieReset(void)
{

	u32 RegVal = 0;
	u32 ICMCfg0;
	u32 ICMCfg1;

	ICMCfg0 = XFsbl_In32(SERDES_ICM_CFG0) &
			(SERDES_ICM_CFG0_L0_ICM_CFG_MASK | SERDES_ICM_CFG0_L1_ICM_CFG_MASK);

	ICMCfg1 = XFsbl_In32(SERDES_ICM_CFG1) &
			(SERDES_ICM_CFG1_L2_ICM_CFG_MASK | SERDES_ICM_CFG1_L3_ICM_CFG_MASK);

	/* Give reset only if we have PCIe in design */
	if ((ICMCfg0 == ICM_CFG0_PCIE_PCIE) || (ICMCfg1 == ICM_CFG1_PCIE_PCIE))
	{

		/* Set MIO31 direction as output */
		XFsbl_Out32(GPIO_DIRM_1, GPIO_MIO31_MASK);

		/* Set MIO31 output enable */
		XFsbl_Out32(GPIO_OEN_1, GPIO_MIO31_MASK);

		/* Set MIO31 to HIGH */
		RegVal = XFsbl_In32(GPIO_DATA_1) | GPIO_MIO31_MASK;
		XFsbl_Out32(GPIO_DATA_1, RegVal);

		(void)usleep(DELAY_1_US);

		/* Set MIO31 to LOW */
		RegVal = XFsbl_In32(GPIO_DATA_1) & ~(GPIO_MIO31_MASK);
		XFsbl_Out32(GPIO_DATA_1, RegVal);

		(void)usleep(DELAY_5_US);

		/* Set MIO31 to HIGH */
		RegVal = XFsbl_In32(GPIO_DATA_1) | GPIO_MIO31_MASK;
		XFsbl_Out32(GPIO_DATA_1, RegVal);
	}

}
#endif

/*****************************************************************************/
/**
 * This function does board specific initialization.
 * Currently this is done for ZCU102 board.
 * If there isn't any board specific initialization required, it just returns.
 *
 * @param none
 *
 * @return
 * 		- XFSBL_SUCCESS for successful configuration
 * 		- errors as mentioned in xfsbl_error.h
 *
 *****************************************************************************/
u32 XFsbl_BoardInit(void)
{
	u32 Status = XFSBL_SUCCESS;
#ifdef XPS_BOARD_ZCU102
	/* Program I2C to configure GT lanes */
	Status = XFsbl_BoardConfig();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	XFsbl_UsbPhyReset();
	XFsbl_PcieReset();

	END:
#endif
	return Status;
}
