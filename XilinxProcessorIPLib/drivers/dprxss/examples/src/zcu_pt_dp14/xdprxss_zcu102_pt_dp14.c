/*******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file dptxss_zcu102_pt_dp14.c
*
* This file contains a design example using the XDpRxSs driver in single stream
* (SST) transport mode.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 vk 10/04/17 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "main.h"

#include "tx.h"
#include "rx.h"

void operationMenu();
void resetIp();
void DpTxSs_Main();
void DpRxSs_Main();
void DpPt_Main();


//extern XVidC_VideoMode resolution_table[];
// adding new resolution definition example
// XVIDC_VM_3840x2160_30_P_SB, XVIDC_B_TIMING3_60_P_RB
// and XVIDC_VM_3840x2160_60_P_RB has added
typedef enum {
    XVIDC_VM_1920x1080_60_P_RB = (XVIDC_VM_CUSTOM + 1),
	XVIDC_B_TIMING3_60_P_RB ,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
	XVIDC_VM_7680x4320_25_P,
	XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,

	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR,
    XVIDC_CM_NUM_SUPPORTED
} XVIDC_CUSTOM_MODES;

// CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
					(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{
	{ XVIDC_VM_1920x1080_60_P_RB, "1920x1080@60Hz (RB)", XVIDC_FR_60HZ,
		{1920, 48, 32, 80, 2080, 1,
		1080, 3, 5, 23, 1111, 0, 0, 0, 0, 0}},
	{ XVIDC_B_TIMING3_60_P_RB, "2560x1440@60Hz (RB)", XVIDC_FR_60HZ,
		 {2560, 48, 32, 80, 2720, 1,
		1440, 3, 5, 33, 1481, 0, 0, 0, 0, 0}},
	{ XVIDC_VM_3840x2160_120_P_RB, "3840x2160@120Hz (RB)", XVIDC_FR_120HZ,
		{3840, 8, 32, 40, 3920, 1,
		2160, 113, 8, 6, 2287, 0, 0, 0, 0, 1} },

	{ XVIDC_VM_7680x4320_24_P, "7680x4320@24Hz", XVIDC_FR_24HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_25_P, "7680x4320@25Hz", XVIDC_FR_25HZ,
		{7680, 352, 176, 592, 8800, 1,
		4320, 16, 20, 144, 4500, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_P, "7680x4320@30Hz", XVIDC_FR_30HZ,
		{7680, 8, 32, 40, 7760, 0,
		4320, 47, 8, 6, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_100_P_RB2, "3840x2160@100Hz (RB2)", XVIDC_FR_100HZ,
		{3840, 8, 32, 40, 3920, 0,
		2160, 91, 8, 6, 2265, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
		{7680, 48, 32, 80, 7840, 0,
		4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_P_RB2, "5120x2880@60Hz (RB2)", XVIDC_FR_60HZ,
		{5120, 8, 32, 40, 5200, 0,
		2880, 68, 8, 6, 2962, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_7680x4320_30_MSTR, "7680x4320_MSTR@30Hz", XVIDC_FR_30HZ,
		{7680, 25, 97, 239, 8041, 0,
		4320, 48, 9, 5, 4382, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_5120x2880_60_MSTR, "5120x2880@60Hz_MSTR", XVIDC_FR_60HZ,
		{5120, 25, 97, 239, 5481, 0,
		2880, 48, 9, 5, 2942, 0, 0, 0, 0, 1}},
	{ XVIDC_VM_3840x2160_120_MSTR, "3840x2160@120Hz_MSTR", XVIDC_FR_120HZ,
		{3840, 48, 34, 79, 4001, 1,
		2160, 4, 6, 53, 2223, 0, 0, 0, 0, 1}},
};

XVidC_VideoMode resolution_table[] =
{
	XVIDC_VM_640x480_60_P,
	XVIDC_VM_480_60_P,
	XVIDC_VM_800x600_60_P,
	XVIDC_VM_1024x768_60_P,
	XVIDC_VM_720_60_P,
	XVIDC_VM_1600x1200_60_P,
	XVIDC_VM_1366x768_60_P,
	XVIDC_VM_1080_60_P,
	XVIDC_VM_UHD_30_P,
	XVIDC_VM_UHD_60_P,
	XVIDC_VM_2560x1600_60_P,
	XVIDC_VM_1280x1024_60_P,
	XVIDC_VM_1792x1344_60_P,
	XVIDC_VM_848x480_60_P,
	XVIDC_VM_1280x960_60_P,
	XVIDC_VM_1920x1440_60_P,
	XVIDC_VM_USE_EDID_PREFERRED,

	XVIDC_VM_1920x1080_60_P_RB,
	XVIDC_VM_3840x2160_60_P_RB,
	XVIDC_VM_3840x2160_120_P_RB,
	XVIDC_VM_7680x4320_24_P,
	XVIDC_VM_7680x4320_30_P,
	XVIDC_VM_3840x2160_100_P_RB2,
	XVIDC_VM_7680x4320_30_DELL,
	XVIDC_VM_5120x2880_60_P_RB2,
	XVIDC_VM_7680x4320_30_MSTR,
	XVIDC_VM_5120x2880_60_MSTR,
	XVIDC_VM_3840x2160_120_MSTR

};

/*****************************************************************************/
/**
*
* This function initializes VFMC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
int VideoFMC_Init(void)
{
	int Status;
	u8 Buffer[2];
	int ByteCount;

	xil_printf("VFMC: Setting IO Expanders...\n\r");
	
	XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
	/* Initialize the IIC driver so that it is ready to use. */
	ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr_IIC == NULL) {
	        return XST_FAILURE;
	}
	
	Status = XIic_CfgInitialize(&IicInstance, ConfigPtr_IIC,
			ConfigPtr_IIC->BaseAddress);
	if (Status != XST_SUCCESS) {
	        return XST_FAILURE;
	}
	
	XIic_Reset(&IicInstance);

	/* Set the I2C Mux to select the HPC FMC */
	Buffer[0] = 0x07;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}

//	I2C_Scan(XPAR_IIC_0_BASEADDR);

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_0_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	/* Configure VFMC IO Expander 1:
	 * Enable LMK03318 -> In a power-down state the I2C bus becomes unusable.
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(0)
	 * Select IDT8T49N241 clock as source for FMC_GT_CLKp(1)
	 * Enable IDT8T49N241 */
	Buffer[0] = 0x1E;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_1_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}

	xil_printf(" done!\n\r");

	Status = IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}

//	Status = TI_LMK03318_Init(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	Status = TI_LMK03318_PowerDown(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs interrupt example. If the
* DpRxSs_Main function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @param    None.
*
* @return
*        - XST_FAILURE if the interrupt example was unsuccessful.
*
* @note        Unless setup failed, main will never return since
*        DpRxSs_Main is blocking (it is waiting on interrupts).
*
******************************************************************************/
int main()
{
	u32 Status;
	
//	xil_printf("------------------------------------------\n\r");
//	xil_printf("DisplayPort1.4 Example\n\r");
//	xil_printf("(c) 2018 by Xilinx\n\r");
//	xil_printf("-------------------------------------------\n\r\n\r");
	
	xil_printf("\n******************************************************"
				"**********\n\r");
	xil_printf("            DisplayPort Pass Through Demonstratio"
			"n                \n\r");
	xil_printf("                   (c) by Xilinx   ");
	xil_printf("%s %s\n\r\r\n", __DATE__  ,__TIME__ );
	xil_printf("                   System Configuration:\r\n");
	xil_printf("                      DP SS : %d byte\r\n",
					2 * SET_TX_TO_2BYTE);
	xil_printf("\n********************************************************"
				"********\n\r");
	
	
	Status = DpSs_Main();
	if (Status != XST_SUCCESS) {
	xil_printf("DisplayPort Subsystem design example failed.");
	return XST_FAILURE;
	}
	
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the main entry point for the design example using the
* XDpRxSs driver. This function will setup the system with interrupts handlers.
*
* @param    DeviceId is the unique device ID of the DisplayPort RX
*        Subsystem core.
*
* @return
*        - XST_FAILURE if the system setup failed.
*        - XST_SUCCESS should never return since this function, if setup
*          was successful, is blocking.
*
* @note        If system setup was successful, this function is blocking in
*        order to illustrate interrupt handling taking place for
*        different types interrupts.
*        Refer xdprxss.h file for more info.
*
******************************************************************************/
u32 DpSs_Main(void)
{
	u32 Status;
	u8 UserInput;
	
	XDpRxSs_Config *ConfigPtr_rx;
	XDpTxSs_Config *ConfigPtr_tx;
//	u32 ReadVal=0;
//	u16 DrpVal;

	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");
	
	/* Setup Video Phy, left to the user for implementation */
	DpRxSs_VideoPhyInit(XVPHY_DEVICE_ID);

	/* Obtain the device configuration
	 * for the DisplayPort RX Subsystem */
	ConfigPtr_rx = XDpRxSs_LookupConfig(XDPRXSS_DEVICE_ID);
	if (!ConfigPtr_rx) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into
	 * the DpRxSsInst's Config structure. */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr_rx,
					ConfigPtr_rx->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpRxSsInst.UsrOpt.MstSupport) {
		xil_printf("INFO:DPRXSS is MST enabled. DPRXSS can be "
			"switched to SST/MST\n\r");
	} else {
		xil_printf("INFO:DPRXSS is SST enabled. DPRXSS works "
			"only in SST mode.\n\r");
	}


/* Obtain the device configuration for the DisplayPort TX Subsystem */
	ConfigPtr_tx = XDpTxSs_LookupConfig(XPAR_DPTXSS_0_DEVICE_ID);
	if (!ConfigPtr_tx) {
		return XST_FAILURE;
	}
	/* Copy the device configuration into
	 * the DpTxSsInst's Config structure. */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr_tx,
			ConfigPtr_tx->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\r\n");
		return XST_FAILURE;
	}

	/* Check for SST/MST support */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("INFO:DPTXSS is MST enabled. DPTXSS can be "
			"switched to SST/MST\r\n");
	} else {
		xil_printf("INFO:DPTXSS is SST enabled. DPTXSS works "
			"only in SST mode.\r\n");
	}

	/* Set DP141 Tx driver here. */
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, 0x78);
	i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, 0x78);

	/*Megachips Retimer Initialization*/
	XDpRxSs_McDp6000_init(&DpRxSsInst, XPAR_IIC_0_BASEADDR);


	/* issue HPD at here to inform DP source */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	// Disabling TX interrupts
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,
						XDP_TX_INTERRUPT_MASK, 0xFFF);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);



	/* FrameBuffer initialization. */
	Status = XVFrmbufRd_Initialize(&frmbufrd, FRMBUF_RD_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Read "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}

	Status = XVFrmbufWr_Initialize(&frmbufwr, FRMBUF_WR_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Write "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}

	resetIp();
//	XVFrmbufWr_SetCallback(&frmbufwr, &bufferWr_callback,&frmbufwr);

	DpSs_SetupIntrSystem();


	// Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}


    operationMenu();
	while (1) {
		UserInput = XUartPs_RecvByte_NonBlocking();

		if (UserInput!=0) {
			xil_printf("UserInput: %c\r\n",UserInput);
			switch (UserInput) {
			case 't':
				DpTxSs_Main();
				break;
			case 'r':
				DpRxSs_Main();
				break;
			case 'p':
				DpPt_Main();
				break;
			case 'l':
				Dplb_Main();
				break;
			}
		}
	}

	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function sets GT in 16-bits (2-Byte) or 32-bits (4-Byte) mode.
*
* @param    InstancePtr is a pointer to the Video PHY instance.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void PHY_Two_byte_set (XVphy *InstancePtr, u8 Rx_to_two_byte,
			u8 Tx_to_two_byte)
{
	u16 DrpVal;
	u16 WriteVal;
	
	if (Rx_to_two_byte == 1) {
		XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_RX_DATA_WIDTH,&DrpVal);
		DrpVal &= ~0x1E0;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x60;
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				XVPHY_DRP_RX_DATA_WIDTH, WriteVal);
		
		XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_RX_INT_DATA_WIDTH,&DrpVal);
		DrpVal &= ~0x3;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
				XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
				XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
		XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
				XVPHY_DRP_RX_INT_DATA_WIDTH, WriteVal);
		xil_printf ("RX Channel configured for 2byte mode\r\n");
	}

	if (Tx_to_two_byte == 1) {
		u32 Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
				TX_DATA_WIDTH_REG, &DrpVal);

		if (Status != XST_SUCCESS) {
			xil_printf("DRP access failed\r\n");
			return;
		}

		DrpVal &= ~0xF;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x3;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
					TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
					TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
					TX_DATA_WIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
					TX_DATA_WIDTH_REG, WriteVal);
		if(Status != XST_SUCCESS){
			xil_printf("DRP access failed\r\n");
			return;
		}

		Status = XVphy_DrpRd(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
					TX_INT_DATAWIDTH_REG, &DrpVal);
		if (Status != XST_SUCCESS) {
			xil_printf("DRP access failed\r\n");
			return;
		}

		DrpVal &= ~0xC00;
		WriteVal = 0x0;
		WriteVal = DrpVal | 0x0;
		Status  =XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH1,
					TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH2,
					TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH3,
					TX_INT_DATAWIDTH_REG, WriteVal);
		Status +=XVphy_DrpWr(InstancePtr, 0, XVPHY_CHANNEL_ID_CH4,
					TX_INT_DATAWIDTH_REG, WriteVal);
		if (Status != XST_SUCCESS) {
			xil_printf("DRP access failed\r\n");
			return;
		}
		xil_printf ("TX Channel configured for 2byte mode\r\n");
	}
}

void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate) {

	switch (link_rate) {
	case 0x6:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_162GBPS);
		break;
	case 0x14:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_540GBPS);
		break;
	case 0x1E:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
					ONBOARD_REF_CLK,
					XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_810GBPS);
		break;
	default:
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
		    		    ONBOARD_REF_CLK,
		    		    XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
//				    DP159_FORWARDED_CLK,
//				    XVPHY_DP_REF_CLK_FREQ_HZ_135);
		    		    ONBOARD_REF_CLK,
		    		    XVPHY_DP_REF_CLK_FREQ_HZ_270);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CHA,
				  XVPHY_DP_LINK_RATE_HZ_270GBPS);
		XVphy_CfgLineRate(InstancePtr, 0, XVPHY_CHANNEL_ID_CMN1,
				  XVPHY_DP_LINK_RATE_HZ_270GBPS);
		break;
	}
}

/*****************************************************************************/
/**
*
* This function is a non-blocking UART return byte
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
char XUartPs_RecvByte_NonBlocking()
{
	u32 RecievedByte;
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
	
	/* Return the byte received */
	return (u8)RecievedByte;
}

/*****************************************************************************/
/**
*
* This function is called when DisplayPort Subsystem core requires delay
* or sleep. It provides timer with predefined amount of loop iterations.
*
* @param    InstancePtr is a pointer to the XDp instance.
*
* @return    None.
*
*
******************************************************************************/
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds *
			(DpInstance->Config.SAxiClkHz / 1000000));
	
	XTmrCtr_Reset(DpInstance->UserTimerPtr, 0);
	XTmrCtr_Start(DpInstance->UserTimerPtr, 0);
	
	/* Wait specified number of useconds. */
	do {
	    TimerVal = XTmrCtr_GetValue(DpInstance->UserTimerPtr, 0);
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
*
* This function Calculates CRC values of Video components
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void CalculateCRC(void)
{
     /* Reset CRC Test Counter in DP DPCD Space. */
	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_CONFIG,
		     (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC_rx.TEST_CRC_CNT));

	/*Set pixel mode as per lane count - it is default behavior
	  User has to adjust this accordingly if there is change in pixel
	  width programming
	 */
	XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
			      VIDEO_FRAME_CRC_CONFIG,
			      DpRxSsInst.UsrOpt.LaneCount);
	
	/* Set pixel mode as per lane count - it
	 * is default behavior Reset DTG. */
	XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr,
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
								XDP_RX_USER_PIXEL_WIDTH));

	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);
	
	/* Add delay (~40 ms) for Frame CRC
	 * to compute on couple of frames. */
	CustomWaitUs(DpRxSsInst.DpPtr, 400000);
	
	/* Read computed values from Frame
	 * CRC module and MISC0 for colorimetry */
	VidFrameCRC_rx.Pixel_r  = XVidFrameCrc_ReadReg(VidFrameCRC_rx.Base_Addr,
					VIDEO_FRAME_CRC_VALUE_G_R) & 0xFFFF;
	VidFrameCRC_rx.Pixel_g  = XVidFrameCrc_ReadReg(VidFrameCRC_rx.Base_Addr,
					VIDEO_FRAME_CRC_VALUE_G_R) >> 16;
	VidFrameCRC_rx.Pixel_b  = XVidFrameCrc_ReadReg(VidFrameCRC_rx.Base_Addr,
					VIDEO_FRAME_CRC_VALUE_B) & 0xFFFF;
	VidFrameCRC_rx.Mode_422 =
			(XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					      XDP_RX_MSA_MISC0) >> 1) & 0x3;
	
	/* Write CRC values to DPCD TEST CRC space. */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_COMP0,
		     (VidFrameCRC_rx.Mode_422 == 0x1) ? 0 : VidFrameCRC_rx.Pixel_r);
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_COMP1,
		     (VidFrameCRC_rx.Mode_422==0x1) ? VidFrameCRC_rx.Pixel_b :
						      VidFrameCRC_rx.Pixel_g);
	/* Check for 422 format and move CR/CB
	 * calculated CRC to G component.
	 * Place as tester needs this way
	 * */
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
		     XDP_RX_CRC_COMP2,
		     (VidFrameCRC_rx.Mode_422 == 0x1) ? VidFrameCRC_rx.Pixel_r :
							VidFrameCRC_rx.Pixel_b);
	
	VidFrameCRC_rx.TEST_CRC_CNT = 1;
	XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
		     (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
		      VidFrameCRC_rx.TEST_CRC_CNT));
	
	xil_printf("[Video CRC] R/Cr: 0x%x, G/Y: 0x%x, B/Cb: 0x%x\r\n\n",
		   VidFrameCRC_rx.Pixel_r, VidFrameCRC_rx.Pixel_g,
		   VidFrameCRC_rx.Pixel_b);
}

/*****************************************************************************/
/**
*
* This function scans VFMC- IIC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
void I2C_Scan(u32 BaseAddress)
{
	u8 Buffer[2];
	int BytesRecvd;
	int i;

	print("\n\r");
	print("---------------------\n\r");
	print("- I2C Scan: \n\r");
	print("---------------------\n\r");

	for (i = 0; i < 128; i++) {
		BytesRecvd = XIic_Recv(BaseAddress, i, (u8*)Buffer, 1, XIIC_STOP);
		if (BytesRecvd == 0) {
			continue;
		}
		xil_printf("Found device: 0x%02x\n\r",i);
	}
	print("\n\r");
}

/*****************************************************************************/
/**
*
* This function reads DP141 VFMC- IIC.
*
* @param    None.
*
* @return    None.
*
* @note        None.
*
******************************************************************************/
u8 i2c_read_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;


	Exit = FALSE;
	Data = 0;

	do {
		/* Set Address */
//		Buffer[0] = (RegisterAddress >> 8);
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/* Maximum retries. */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/* Read data. */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
					      (u8*)Buffer, 1, XIIC_STOP);
//			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
//			if (ByteCount != 1) {
//				Exit = FALSE;
//				Exit = TRUE;
//			else {
				Data = Buffer[0];
				Exit = TRUE;
//			}
		}
	} while (!Exit);

	return Data;
}

int i2c_write_dp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		u16 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	/* Write data */
//	Buffer[0] = (RegisterAddress >> 8);
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount != 2) {
			Retry++;

			/* Maximum retries */
			if (Retry == 15) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

void read_DP141()
{
	u8 Data;
	int i =0;
//	u8 Buffer[1];

	for (i = 0 ; i < 0xD ; i++) {
		Data = i2c_read_dp141(XPAR_IIC_0_BASEADDR,
				      I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if required peripherals are initialized and
*        configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpSs_PlatformInit(void)
{
	u32 Status;
	
	/* Initialize CRC & Set default Pixel Mode to 1. */
	VidFrameCRC_rx.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx);
	VidFrameCRC_tx.Base_Addr = VIDEO_FRAME_CRC_TX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx);
	
	/* Initialize Timer */
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetResetValue(&TmrCtr, XTIMER0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTIMER0_DEVICE_ID);
	
	VideoFMC_Init();
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR,
			      0, 270000000, TRUE);

//	rx_remap_Config->BaseAddress = REMAP_RX_BASEADDR;
//	rx_remap.Config.BaseAddress = REMAP_RX_BASEADDR;
//	tx_remap_Config->BaseAddress = REMAP_TX_BASEADDR;
//	tx_remap.Config.BaseAddress = REMAP_TX_BASEADDR;

	rx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_RX_DEVICE_ID);
	Status = XV_axi4s_remap_CfgInitialize(&rx_remap, rx_remap_Config,
					      rx_remap_Config->BaseAddress);
	rx_remap.IsReady = XIL_COMPONENT_IS_READY;
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: AXI4S_REMAP Initialization "
			   "failed %d\r\n", Status);
		return (XST_FAILURE);
	}
	
	tx_remap_Config = XV_axi4s_remap_LookupConfig(REMAP_TX_DEVICE_ID);
	Status = XV_axi4s_remap_CfgInitialize(&tx_remap, tx_remap_Config,
					      tx_remap_Config->BaseAddress);
	tx_remap.IsReady = XIL_COMPONENT_IS_READY;
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: AXI4S_REMAP Initialization "
			   "failed %d\r\n", Status);
		return(XST_FAILURE);
	}

	XV_axi4s_remap_Set_width(&rx_remap, 7680);
	XV_axi4s_remap_Set_height(&rx_remap, 4320);
	XV_axi4s_remap_Set_ColorFormat(&rx_remap, 0);
	XV_axi4s_remap_Set_inPixClk(&rx_remap, 4);
	XV_axi4s_remap_Set_outPixClk(&rx_remap, 4);
	
	XV_axi4s_remap_Set_width(&tx_remap, 7680);
	XV_axi4s_remap_Set_height(&tx_remap, 4320);
	XV_axi4s_remap_Set_ColorFormat(&tx_remap, 0);
	XV_axi4s_remap_Set_inPixClk(&tx_remap, 4);
	XV_axi4s_remap_Set_outPixClk(&tx_remap, 4);
	
	return Status;
}


/*****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* DisplayPort TX Subsystem core. The function is application-specific since
* the actual system may or may not have an interrupt controller. The DPTX
* Subsystem core could be directly connected to a processor without an
* interrupt controller. The user should modify this function to fit the
* application.
*
* @param	None
*
* @return
*		- XST_SUCCESS if interrupt setup was successful.
*		- A specific error code defined in "xstatus.h" if an error
*		occurs.
*
* @note		None.
*
******************************************************************************/
u32 DpSs_SetupIntrSystem(void)
{
	u32 Status;
	XINTC *IntcInstPtr = &IntcInst;

	// Tx side
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_EVENT),
				&DpPt_HpdEventHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_HPD_PULSE),
				&DpPt_HpdPulseHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_LINK_RATE_CHG),
				&DpPt_LinkrateChgHandler, &DpTxSsInst);
	XDpTxSs_SetCallBack(&DpTxSsInst, (XDPTXSS_HANDLER_DP_PE_VS_ADJUST),
				&DpPt_pe_vs_adjustHandler, &DpTxSsInst);

	// Rx side
/* Set callbacks for all the interrupts */
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_PWR_CHG_EVENT,
			DpRxSs_PowerChangeHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_NO_VID_EVENT,
			DpRxSs_NoVideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VBLANK_EVENT,
			DpRxSs_VerticalBlankHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TLOST_EVENT,
			DpRxSs_TrainingLostHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_VID_EVENT,
			DpRxSs_VideoHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_TDONE_EVENT,
			DpRxSs_TrainingDoneHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_UNPLUG_EVENT,
			DpRxSs_UnplugHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_LINKBW_EVENT,
			DpRxSs_LinkBandwidthHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_PLL_RESET_EVENT,
			DpRxSs_PllResetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_BW_CHG_EVENT,
			DpRxSs_BWChangeHandler, &DpRxSsInst);
//	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LANE_SET_EVENT,
//			DpRxSs_AccessLaneSetHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_LINK_QUAL_EVENT,
			DpRxSs_AccessLinkQualHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_ACCESS_ERROR_COUNTER_EVENT,
			DpRxSs_AccessErrorCounterHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_CRC_TEST_EVENT,
			DpRxSs_CRCTestEventHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_INFO_PKT_EVENT,
			&DpRxSs_InfoPacketHandler, &DpRxSsInst);
	XDpRxSs_SetCallBack(&DpRxSsInst, XDPRXSS_HANDLER_DP_EXT_PKT_EVENT,
			&DpRxSs_ExtPacketHandler, &DpRxSsInst);
	/* Set custom timer wait */
	XDpRxSs_SetUserTimerHandler(&DpRxSsInst, &CustomWaitUs, &TmrCtr);

	XVFrmbufWr_SetCallback(&frmbufwr, &bufferWr_callback,&frmbufwr);

	/* The configuration parameters of the interrupt controller */
	XScuGic_Config *IntcConfig;
	
	/* Initialize the interrupt controller 
	 * driver so that it is ready to use. */
	IntcConfig = XScuGic_LookupConfig(XINTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}
	
	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcConfig,
	IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
	return XST_FAILURE;
	}
	
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined 
	 * above performs the specific interrupt processing for the device.
	 * */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
				(Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt for the DP device */
	XScuGic_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
				(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/* Enable the interrupt */
	XScuGic_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);

	Status = XScuGic_Connect(IntcInstPtr,
				 XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID,
				 (Xil_InterruptHandler)XVFrmbufWr_InterruptHandler,
				 &frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
		return XST_FAILURE;
	}
	/* Enable the interrupt vector at the interrupt controller */
	XScuGic_Enable(IntcInstPtr, XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID);

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception
	 * table.*/
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XINTC_HANDLER,
				     IntcInstPtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function puts the TI LMK03318 into sleep
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int TI_LMK03318_PowerDown(u32 I2CBaseAddress, u8 I2CSlaveAddress)
{
	/* Register 29 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 29, 0x03);

	/* Register 30 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 30, 0x3f);

	/* Register 31 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 31, 0x00);

	/* Register 32 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 32, 0x00);

	/* Register 34 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 34, 0x00);

	/* Register 35 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 35, 0x00);

	/* Register 37 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 37, 0x00);

	/* Register 39 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 39, 0x00);

	/* Register 41 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 41, 0x00);

	/* Register 43 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 43, 0x00);

	/* Register 50 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 50, 0xf6);

	/* Register 56 */
	TI_LMK03318_SetRegister(I2CBaseAddress, I2CSlaveAddress, 56, 0x01);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function send a single byte to the TI LMK03318
*
* @param I2CBaseAddress is the baseaddress of the I2C core.
* @param I2CSlaveAddress is the 7-bit I2C slave address.
*
* @return
*    - XST_SUCCESS Initialization was successful.
*    - XST_FAILURE I2C write error.
*
* @note None.
*
******************************************************************************/
int TI_LMK03318_SetRegister(u32 I2CBaseAddress, u8 I2CSlaveAddress,
			u8 RegisterAddress, u8 Value)
{
	u32 ByteCount = 0;
	u8 Buffer[2];

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);

	if (ByteCount != 2) {
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}
