/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* Board specific code for ZCU106 is similar to that of ZCU102, except that
* GT mux configuration and PCIe reset are not applicable for ZCU106.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ssc  01/20/16 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*                     Added ZCU106 support
*
* </pre>
*
* @note
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xfsbl_board.h"
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XFsbl_BoardConfig(void);
static void XFsbl_UsbPhyReset(void);
#if defined(XPS_BOARD_ZCU102)
static void XFsbl_PcieReset(void);
#endif
/************************** Variable Definitions *****************************/

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
static u32 XFsbl_BoardConfig(void)
{
	XIicPs I2c0InstancePtr;
	u8 WriteBuffer[BUF_LEN] = {0};
	XIicPs_Config *I2c0CfgPtr;
	s32 Status;
	u32 UStatus;
#if defined(XPS_BOARD_ZCU102)
	u32 ICMCfgLane[NUM_GT_LANES];
#endif

	/* Initialize the IIC0 driver so that it is ready to use */
	I2c0CfgPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (I2c0CfgPtr == NULL) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	Status = XIicPs_CfgInitialize(&I2c0InstancePtr, I2c0CfgPtr,
			I2c0CfgPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_INIT;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_INIT\r\n");
		goto END;
	}

	/* Set the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_IOEXP);
    if(Status != XST_SUCCESS) {
	UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
	XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_SET_SCLK\r\n");
	goto END;
    }
	/* Configure I/O pins as Output */
	WriteBuffer[0] = CMD_CFG_0_REG;
	WriteBuffer[1] = DATA_OUTPUT;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle to start another transfer */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}

	/*
	 * Deasserting I2C_MUX_RESETB
	 * And GEM3 Resetb
	 * Selecting lanes based on configuration
	 */
	WriteBuffer[0] = CMD_OUTPUT_0_REG;
	WriteBuffer[1] = DATA_COMMON_CFG;

#if defined(XPS_BOARD_ZCU102)
	ICMCfgLane[0] = XFsbl_In32(SERDES_ICM_CFG0) & SERDES_ICM_CFG0_L0_ICM_CFG_MASK;
	ICMCfgLane[1] = (XFsbl_In32(SERDES_ICM_CFG0) &
		SERDES_ICM_CFG0_L1_ICM_CFG_MASK) >> SERDES_ICM_CFG0_L1_ICM_CFG_SHIFT;
	ICMCfgLane[2] = XFsbl_In32(SERDES_ICM_CFG1) & (SERDES_ICM_CFG1_L2_ICM_CFG_MASK);
	ICMCfgLane[3] = (XFsbl_In32(SERDES_ICM_CFG1) &
		SERDES_ICM_CFG1_L3_ICM_CFG_MASK) >> SERDES_ICM_CFG1_L3_ICM_CFG_SHIFT;

	/* For ZCU102 board, check if GT combination is valid against the lane# */
	if (((ICMCfgLane[0] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[0] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[0] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[1] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[1] != ICM_CFG_VAL_DP)
			&& (ICMCfgLane[1] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[2] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[2] != ICM_CFG_VAL_USB)
			&& (ICMCfgLane[2] != ICM_CFG_VAL_PWRDN)) ||
		((ICMCfgLane[3] != ICM_CFG_VAL_PCIE)
			&& (ICMCfgLane[3] != ICM_CFG_VAL_SATA)
			&& (ICMCfgLane[3] != ICM_CFG_VAL_PWRDN))) {

		UStatus = XFSBL_ERROR_GT_LANE_SELECTION;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_GT_LANE_SELECTION\r\n");
		goto END;
	}

	/**
	 * If any of the lanes are of PCIe or PowerDown, that particular lane
	 * shall be configured as PCIe, else shall be configured
	 * as DP/USB/SATA, as applicable to that lane.
	 *
	 * Lane# 	WriteBuffer[1] bit#		bit value '0'	bit value '1'
	 * --------------------------------------------------------------
	 * Lane0			0					PCIe		DP
	 * Lane1			1					PCIe		DP
	 * Lane2			2					PCIe		USB
	 * Lane3			3					PCIe		SATA
	 */

	if (ICMCfgLane[0] == ICM_CFG_VAL_DP) {
		WriteBuffer[1] |= DATA_GT_L0_DP_CFG;
	}

	if (ICMCfgLane[1] == ICM_CFG_VAL_DP) {
		WriteBuffer[1] |= DATA_GT_L1_DP_CFG;
	}

	if (ICMCfgLane[2] == ICM_CFG_VAL_USB) {
		WriteBuffer[1] |= DATA_GT_L2_USB_CFG;
	}

	if (ICMCfgLane[3] == ICM_CFG_VAL_SATA) {
		WriteBuffer[1] |= DATA_GT_L3_SATA_CFG;
	}
#endif

	/* Send the Data */
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, IOEXPANDER1_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}

	/* Change the IIC serial clock rate */
	Status = XIicPs_SetSClk(&I2c0InstancePtr, IIC_SCLK_RATE_I2CMUX);

	if(Status != XST_SUCCESS) {
		UStatus  = XFSBL_ERROR_I2C_SET_SCLK;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_SET_SCLK\r\n");
		goto END;
	}
	/* Set I2C Mux for channel-2 (MAXIM_PMBUS) */
	WriteBuffer[0] = CMD_CH_2_REG;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 1, PCA9544A_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}
	/**
	 * The below piece of code is needed for PL DDR to work
	 * (to take PL DDR out of reset). Hence including this code only when
	 * PL DDR is in design.
	 */
#ifdef XPAR_MIG_0_BASEADDR
	/* Enable Regulator (FMC ADJ) */
	WriteBuffer[0] = CMD_ON_OFF_CFG;
	WriteBuffer[1] = ON_OFF_CFG_VAL;
	Status = XIicPs_MasterSendPolled(&I2c0InstancePtr,
			WriteBuffer, 2, MAX15301_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_ERROR_I2C_WRITE;
		XFsbl_Printf(DEBUG_GENERAL,"XFSBL_ERROR_I2C_WRITE\r\n");
		goto END;
	}

	/* Wait until bus is idle */
	while (XIicPs_BusIsBusy(&I2c0InstancePtr)>0) {
		/*For MISRA C compliance*/
	}
#endif

	XFsbl_Printf(DEBUG_INFO,"Board Configuration successful \n\r");
	UStatus = XFSBL_SUCCESS;

END:

	return UStatus;

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
static void XFsbl_UsbPhyReset(void)
{

	/* USB PHY Reset */
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);
	(void)usleep(DELAY_1_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_LO);
	(void)usleep(DELAY_5_US);
	XFsbl_Out32(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOTMODE_1_HI);

}

#if defined(XPS_BOARD_ZCU102)
/*****************************************************************************/
/**
 * This function is used to provide PCIe reset on ZCU102 board.
 *
 * @param none
 *
 * @return none
 *
 *****************************************************************************/
static void XFsbl_PcieReset(void)
{

	u32 RegVal;
	u32 ICMCfg0L0;

	ICMCfg0L0 = XFsbl_In32(SERDES_ICM_CFG0) & SERDES_ICM_CFG0_L0_ICM_CFG_MASK;

	/* Give reset only if we have PCIe in design */
	if (ICMCfg0L0 == ICM_CFG_VAL_PCIE)
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
	u32 Status;
#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106)
	/* Program I2C to configure GT lanes */
	Status = XFsbl_BoardConfig();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	XFsbl_UsbPhyReset();
#if defined(XPS_BOARD_ZCU102)
	XFsbl_PcieReset();
#endif
#else
	Status = XFSBL_SUCCESS;
	goto END;
#endif

END:
	return Status;
}
