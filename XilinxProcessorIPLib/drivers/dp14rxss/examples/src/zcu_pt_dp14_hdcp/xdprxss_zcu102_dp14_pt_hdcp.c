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
* @file xdprxss_zcu102_pt_dp14.c
*
* This file contains a design example using the XDpSs driver in single stream
* (SST) transport mode to demonstrate Pass-through design.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 KI 04/01/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "main.h"

#include "si5328drv.h"
//
#define PS_IIC_CLK 400000
//
XIicPs Ps_Iic0, Ps_Iic1;
//

#if ENABLE_HDCP_IN_DESIGN
unsigned int gKeyMGMTBaseAddress[2] = {
		XPAR_DP_RX_HIER_0_HDCP_KEYMNGMT_BLK_0_BASEADDR,
		XPAR_DP_TX_HIER_0_HDCP_KEYMNGMT_BLK_0_BASEADDR};
#else
unsigned int gKeyMGMTBaseAddress[2] = {0, 0};
#endif

int gIsKeyWrittenInEeeprom = FALSE;

#ifdef TxOnly
#include "tx.h"
#endif
#ifdef RxOnly
#include "rx.h"
#endif

void operationMenu();
//void resetIp();
void resetIp_wr();
void resetIp_rd();
void DpTxSs_Main();
void DpRxSs_Main();
void DpPt_Main();
int I2cMux_Ps(u8 mux);
u32 DpSs_VideoPhyInit(u16 DeviceId);
u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr);

u32 frame_array[4] = {0x10000000, 0x20000000, 0x30000000, 0x40000000};
u8 frame_pointer = 1;
u8 frame_pointer_rd = 3;
u8 not_to_read = 1;
u8 not_to_write = 3;
u8 fb_rd_start = 0;
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


typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16 FormatBits;
} VideoFormats;

#define NUM_TEST_FORMATS 15
VideoFormats ColorFormats[NUM_TEST_FORMATS] =
{
	//memory format            stream format        bits per component
	{XVIDC_CSF_MEM_RGBX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUVX8,      XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_YUYV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_RGBX10,     XVIDC_CSF_RGB,       10},
	{XVIDC_CSF_MEM_YUVX10,     XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_Y_UV8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_Y_UV8_420,  XVIDC_CSF_YCRCB_420, 8},
	{XVIDC_CSF_MEM_RGB8,       XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_YUV8,       XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y_UV10,     XVIDC_CSF_YCRCB_422, 10},
	{XVIDC_CSF_MEM_Y_UV10_420, XVIDC_CSF_YCRCB_420, 10},
	{XVIDC_CSF_MEM_Y8,         XVIDC_CSF_YCRCB_444, 8},
	{XVIDC_CSF_MEM_Y10,        XVIDC_CSF_YCRCB_444, 10},
	{XVIDC_CSF_MEM_BGRX8,      XVIDC_CSF_RGB,       8},
	{XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8}
};


extern u32 StreamOffset[4];

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
	Buffer[0] = 0x01;
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

//	xil_printf(" done!\n\r");

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


	xil_printf("\n******************************************************"
				"**********\n\r");
	xil_printf("            DisplayPort Pass Through Demonstration"
			"                \n\r");
	xil_printf("                   (c) by Xilinx   ");
	xil_printf("%s %s\n\r\r\n", __DATE__  ,__TIME__ );
	xil_printf("                   System Configuration:\r\n");
	xil_printf("                      DP SS : %d byte\r\n",
					2 * SET_TX_TO_2BYTE);
	xil_printf("                      Use I2S and ACR : %d \r\n",
					1 * I2S_AUDIO);
#if ENABLE_HDCP_IN_DESIGN
	xil_printf("                      HDCP  : %d \r\n",ENABLE_HDCP_IN_DESIGN);
#endif
	xil_printf("\n********************************************************"
				"********\n\r");

#if PHY_COMP
	xil_printf ("*****   Application is in Compliance Mode  *****\r\n");
	xil_printf ("***** Press 't' to start the TX video path *****\r\n");
#endif

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

int I2cMux_Ps(u8 mux)
{
  u8 Buffer;
  int Status;

  if (mux == 0) {
	  /* Close other mux */
	  Buffer = 0x0;
	  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1,
			  I2C_MUX_ADDR);

	  /* open the othe rmux for Si5328 */
	  Buffer = 0x10;
	  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1,
			  I2C_MUX_ADDR_SI);
  } else {

	  Buffer = 0x0;
	  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1,
			  I2C_MUX_ADDR_SI);

	  /* open the other Mux */
	  Buffer = 0x1;
	  Status = XIicPs_MasterSendPolled(&Ps_Iic1, (u8 *)&Buffer, 1,
			  I2C_MUX_ADDR);

  }

  return Status;
}


int I2cClk_Ps(u32 InFreq, u32 OutFreq)
{
  int Status;

  I2cMux_Ps(0);

  /* Free running mode */
  if (InFreq == 0) {
     Status = Si5328_SetClock_Ps(&Ps_Iic1, (I2C_ADDR_SI5328),
                            (SI5328_CLKSRC_XTAL), (SI5328_XTAL_FREQ), OutFreq);

     if (Status != (SI5328_SUCCESS)) {
          print("Error programming SI5328\r\n");
          return 0;
     }
  }
  /* Locked mode */
  else {
     Status = Si5328_SetClock_Ps(&Ps_Iic1, (I2C_ADDR_SI5328),
                                (SI5328_CLKSRC_CLK1), InFreq, OutFreq);

     if (Status != (SI5328_SUCCESS)) {
        print("Error programming SI5328\r\n");
        return 0;
     }
  }

  I2cMux_Ps(1);

  usleep(10000);
  usleep(10000);
  usleep(10000);
  usleep(10000);
#if ENABLE_AUDIO
  //resetting the MMCM after Si5328 programming
  XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x1);
  XGpio_WriteReg (aud_gpio_ConfigPtr->BaseAddress, 0x0, 0x0);
  //MRst is still in reset
#endif
  usleep(10000);
  usleep(10000);
  usleep(10000);

  return 1;
}



u32 DpSs_Main(void)
{
	u32 Status;
	u8 UserInput;
#ifdef RxOnly
	XDpRxSs_Config *ConfigPtr_rx;
#endif
#ifdef TxOnly
	XDpTxSs_Config *ConfigPtr_tx;
#endif
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

	DpSs_VideoPhyInit(XVPHY_DEVICE_ID);
	set_vphy(0x14);

#ifdef RxOnly
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

	/*Megachips Retimer Initialization*/
	XDpRxSs_McDp6000_init(&DpRxSsInst, DpRxSsInst.IicPtr->BaseAddress);

	/* issue HPD at here to inform DP source */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);

#endif

#ifdef TxOnly
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
    //Keeping 0db gain on RX
    //Adding 6db gain on TX
	//Adding some Eq gain
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, 0x3C);

#endif

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

	resetIp_rd();
	resetIp_wr();


	// programming Si5328 for 512*Fs for Audio clock
#if ENABLE_AUDIO
	I2cMux_Ps(0);
    Si5328_Init_Ps(&Ps_Iic1, I2C_ADDR_SI5328);
    I2cClk_Ps(0, 36864000);
#endif

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

#if ENABLE_HDCP_IN_DESIGN
	XHdcp1xExample_Poll();
#endif
		UserInput = XUartPs_RecvByte_NonBlocking();

		if (UserInput!=0) {
			xil_printf("UserInput: %c\r\n",UserInput);
			switch (UserInput) {
#ifdef PT
			case 'p':
				DpPt_Main();
				break;
#endif
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
#ifdef RxOnly
void CalculateCRC(void)
{

    u32 RegVal;
 /* Reset CRC Test Counter in DP DPCD Space. */
    /* Read Config Register */
    RegVal = XVidFrameCrc_ReadReg(VidFrameCRC_rx.Base_Addr,
                            VIDEO_FRAME_CRC_CONFIG);

    /* Toggle CRC Clear Bit */
    XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                    VIDEO_FRAME_CRC_CONFIG,
                    (RegVal | VIDEO_FRAME_CRC_CLEAR));
    XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                    VIDEO_FRAME_CRC_CONFIG,
                    (RegVal & ~VIDEO_FRAME_CRC_CLEAR));

    VidFrameCRC_rx.TEST_CRC_CNT = 0;

    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                 XDP_RX_CRC_CONFIG,
                 (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
                  VidFrameCRC_rx.TEST_CRC_CNT));

    /*Set pixel mode as per lane count - it is default behavior
      User has to adjust this accordingly if there is change in pixel
      width programming
     */

    VidFrameCRC_rx.Mode_422 =
                    (XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                                          XDP_RX_MSA_MISC0) >> 1) & 0x3;

    if (VidFrameCRC_rx.Mode_422 != 0x1) {
	XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                          VIDEO_FRAME_CRC_CONFIG,
                            DpRxSsInst.UsrOpt.LaneCount);
    } else { // 422
        XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                              VIDEO_FRAME_CRC_CONFIG,
                                (DpRxSsInst.UsrOpt.LaneCount | 0x80000000));

    }

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
//    VidFrameCRC_rx.Mode_422 =
//                    (XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//                                          XDP_RX_MSA_MISC0) >> 1) & 0x3;

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
#endif
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
#ifdef RxOnly
	VidFrameCRC_rx.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx);
#endif
#ifdef TxOnly
	VidFrameCRC_tx.Base_Addr = VIDEO_FRAME_CRC_TX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx);
#endif

	/* Initialize Timer */
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
	XTmrCtr_SetResetValue(&TmrCtr, XTIMER0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTIMER0_DEVICE_ID);

	VideoFMC_Init();

	IDT_8T49N24x_Configure(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR);

	usleep(300000);

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

    XIic0Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
    if (NULL == XIic0Ps_ConfigPtr) {
            return XST_FAILURE;
    }

    Status = XIicPs_CfgInitialize(&Ps_Iic0, XIic0Ps_ConfigPtr,
								XIic0Ps_ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }

    XIicPs_Reset(&Ps_Iic0);
    /*
     * Set the IIC serial clock rate.
     */
    XIicPs_SetSClk(&Ps_Iic0, PS_IIC_CLK);

    /* Initialize PS IIC1 */
    XIic1Ps_ConfigPtr = XIicPs_LookupConfig(XPAR_XIICPS_1_DEVICE_ID);
    if (NULL == XIic1Ps_ConfigPtr) {
            return XST_FAILURE;
    }

    Status = XIicPs_CfgInitialize(&Ps_Iic1, XIic1Ps_ConfigPtr,
								XIic1Ps_ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
            return XST_FAILURE;
    }

    XIicPs_Reset(&Ps_Iic1);
    /*
     * Set the IIC serial clock rate.
     */
    XIicPs_SetSClk(&Ps_Iic1, PS_IIC_CLK);


#if ENABLE_AUDIO

    XAxis_Switch_Config *ConfigPtr_AXIS_SWITCH_RX =
		XAxisScr_LookupConfig(XPAR_DP_RX_HIER_0_AXIS_SWITCH_0_DEVICE_ID);
     if (ConfigPtr_AXIS_SWITCH_RX == NULL) {
             return XST_FAILURE;
     }

     Status = XAxisScr_CfgInitialize(&axis_switch_rx,
		 ConfigPtr_AXIS_SWITCH_RX, ConfigPtr_AXIS_SWITCH_RX->BaseAddress);
     if (Status != XST_SUCCESS) {
             return XST_FAILURE;
     }

     XAxis_Switch_Config *ConfigPtr_AXIS_SWITCH_TX =
		 XAxisScr_LookupConfig(XPAR_DP_TX_HIER_0_AXIS_SWITCH_0_DEVICE_ID);
      if (ConfigPtr_AXIS_SWITCH_TX == NULL) {
              return XST_FAILURE;
      }

      Status = XAxisScr_CfgInitialize(&axis_switch_tx,
		  ConfigPtr_AXIS_SWITCH_TX, ConfigPtr_AXIS_SWITCH_TX->BaseAddress);
      if (Status != XST_SUCCESS) {
              return XST_FAILURE;
      }


    Config = XI2s_Tx_LookupConfig(
			XPAR_DP_RX_HIER_0_I2S_TRANSMITTER_0_DEVICE_ID);
    if (Config == NULL) {
         return XST_FAILURE;
    }

    Status = XI2s_Tx_CfgInitialize(&I2s_tx, Config, Config->BaseAddress);
    if (Status != XST_SUCCESS) {
         return XST_FAILURE;
    }

    Config_rx = XI2s_Rx_LookupConfig(
			XPAR_DP_TX_HIER_0_I2S_RECEIVER_0_DEVICE_ID);
    if (Config == NULL) {
          return XST_FAILURE;
    }

    Status = XI2s_Rx_CfgInitialize(&I2s_rx, Config_rx, Config_rx->BaseAddress);
    if (Status != XST_SUCCESS) {
          return XST_FAILURE;
    }

    aud_gpio_ConfigPtr =
            XGpio_LookupConfig(XPAR_DP_RX_HIER_0_AXI_GPIO_0_DEVICE_ID);

    if(aud_gpio_ConfigPtr == NULL) {
	aud_gpio.IsReady = 0;
            return (XST_DEVICE_NOT_FOUND);
    }

    Status = XGpio_CfgInitialize(&aud_gpio,
		aud_gpio_ConfigPtr,
			aud_gpio_ConfigPtr->BaseAddress);
    if(Status != XST_SUCCESS) {
            xil_printf("ERR:: GPIO for TPG Reset ");
            xil_printf("Initialization failed %d\r\n", Status);
            return(XST_FAILURE);
    }

#endif

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
#ifdef TxOnly
	DpTxSs_SetupIntrSystem();
#endif
	// Rx side
#ifdef RxOnly
	DpRxSs_SetupIntrSystem();
#endif

	XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE,
								&bufferWr_callback,&frmbufwr);

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
#ifdef RxOnly
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
				(Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt for the DP device */
	XScuGic_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

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

#endif
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
#ifdef TxOnly
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
				 XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID,
				 (Xil_InterruptHandler)XVFrmbufRd_InterruptHandler,
				 &frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/* Disable the interrupt vector at the interrupt controller */
	XScuGic_Disable(IntcInstPtr, XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID);


#endif
#if ENABLE_HDCP_IN_DESIGN
	/* Hook up Rx interrupt service routine */
	Status = XScuGic_Connect(IntcInstPtr, XPAR_FABRIC_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID,
			(XInterruptHandler)XDpRxSs_TmrCtrIntrHandler,&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Timer interrupt connect failed!\n\r");
		return XST_FAILURE;
	}

	XScuGic_Enable(IntcInstPtr, XPAR_FABRIC_DPRXSS_0_DPRXSS_TIMER_IRQ_VEC_ID);
#endif

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



/*****************************************************************************/
/**
*
* This function configures Video Phy.
*
* @param    None.
*
* @return
*        - XST_SUCCESS if Video Phy configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note        None.
*
******************************************************************************/
u32 DpSs_VideoPhyInit(u16 DeviceId)
{

	XVphy_Config *ConfigPtr;

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
	ConfigPtr = XVphy_LookupConfig(DeviceId);
	if (!ConfigPtr) {
		return XST_FAILURE;
	}


	PLLRefClkSel (&VPhyInst, 0x14);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			ONBOARD_REF_CLK,
			ONBOARD_REF_CLK,
			XVPHY_PLL_TYPE_QPLL1,
			XVPHY_PLL_TYPE_CPLL,
			0x14);
	//set the default vswing and pe for v0po

    //setting vswing
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);
    XVphy_SetTxVoltageSwing(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_DIFF_SWING_DP_V0P0);

    //setting postcursor
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3,
		XVPHY_GTHE4_PREEMP_DP_L0);
    XVphy_SetTxPostCursor(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4,
		XVPHY_GTHE4_PREEMP_DP_L0);


#if TX_BUFFER_BYPASS
    XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH1, 0x3E, DIVIDER_540);
    XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH2, 0x3E, DIVIDER_540);
    XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH3, 0x3E, DIVIDER_540);
    XVphy_DrpWr(&VPhyInst, 0, XVPHY_CHANNEL_ID_CH4, 0x3E, DIVIDER_540);
#endif

	PHY_Two_byte_set (&VPhyInst, SET_RX_TO_2BYTE, SET_TX_TO_2BYTE);

     XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(TRUE));
     XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(TRUE));

     XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_TX,(FALSE));
     XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));


     XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(TRUE));
     XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(TRUE));

     XVphy_ResetGtPll(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA, XVPHY_DIR_RX,(FALSE));
     XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_RX,(FALSE));

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function configures Frame BufferWr for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_wr(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr){
	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufWr_Stop(&frmbufwr);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufWr\r\n");
//	}
//	frame_pointer = 1;
	XVFRMBUFWR_BUFFER_BASEADDR = frame_array[frame_pointer];

	Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write "
			"buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
	XVFrmbufWr_InterruptEnable(&frmbufwr,
			XVFRMBUFWR_HANDLER_READY | XVFRMBUFWR_HANDLER_DONE);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	/* Start Frame Buffers */
	XVFrmbufWr_Start(&frmbufwr);

	//xil_printf("INFO: FRMBUFwr configured\r\n");
	return(Status);
}


/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_rd(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr)
	{

	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufRd_Stop(&frmbufrd);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufRd\r\n");
//	}

	XVFRMBUFRD_BUFFER_BASEADDR = frame_array[frame_pointer_rd];

	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
//	XVFrmbufRd_InterruptEnable(&frmbufrd,
//			XVFRMBUFRD_HANDLER_READY | XVFRMBUFRD_HANDLER_DONE);


	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	//xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
}


/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
u32 offset_rd = 0;

int ConfigFrmbuf_rd_trunc(u32 offset){

	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufRd_Stop(&frmbufrd);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufRd\r\n");
//	}

	XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000)) + offset;

	offset_rd = offset;
	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd,
				XV_frmbufrd_Get_HwReg_stride(&frmbufrd.FrmbufRd),
				XV_frmbufrd_Get_HwReg_video_format(&frmbufrd.FrmbufRd),
				XVFrmbufRd_GetVideoStream(&frmbufrd)
			);

	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
//	XVFrmbufRd_InterruptEnable(&frmbufrd, 0);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
}


void frameBuffer_stop(XDpTxSs_MainStreamAttributes Msa[4]) {
//    xil_printf ("FB stop start..\r\n");
	fb_rd_start = 0;
	XVFrmbufRd_Stop(&frmbufrd);
	XVFrmbufWr_Stop(&frmbufwr);

	resetIp_wr(Msa);
	resetIp_rd(Msa);
//	xil_printf ("FB stop end..\r\n");
}


void frameBuffer_stop_rd(XDpTxSs_MainStreamAttributes Msa[4]) {
//    xil_printf ("FB stop start..\r\n");
	fb_rd_start = 0;
	XVFrmbufRd_Stop(&frmbufrd);
	resetIp_rd(Msa);
}


void frameBuffer_stop_wr(XDpTxSs_MainStreamAttributes Msa[4]) {
	XVFrmbufWr_Stop(&frmbufwr);
	resetIp_wr(Msa);
}


void frameBuffer_start_wr(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K) {

	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

	resetIp_wr(Msa);

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		Cfmt = ColorFormats[7].MemFormat;
		VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_8;
	}else if(Msa[0].BitsPerColor == 10){
		Cfmt = ColorFormats[3].MemFormat;
		VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_10;
	}else if(Msa[0].BitsPerColor == 6){ // FrameBuufer can't use 6bpc.
		Cfmt = ColorFormats[7].MemFormat; // instead, use 8bpc setting
		VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_8;
	}

	VidStream.PixPerClk  = (int)DpRxSsInst.UsrOpt.LaneCount;
//	VidStream.VmId = VmId;
//	TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;

	remap_start_wr(Msa, downshift4K);

	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
					256,
					&VidStream);
	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);
}


void frameBuffer_start_rd(XVidC_VideoMode VmId,
		XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K) {

	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

	resetIp_rd(Msa);

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		Cfmt = ColorFormats[7].MemFormat;
		VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_8;
	}else if(Msa[0].BitsPerColor == 10){
		Cfmt = ColorFormats[3].MemFormat;
		VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_10;
	}else if(Msa[0].BitsPerColor == 6){ // FrameBuufer can't use 6bpc.
		Cfmt = ColorFormats[7].MemFormat; // instead, use 8bpc setting
		VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		VidStream.ColorDepth = XVIDC_BPC_8;
	}

	VidStream.PixPerClk  = Msa[0].UserPixelWidth;
//	VidStream.VmId = VmId;
//	TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;


	remap_start_rd(Msa, downshift4K);

	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
					256,
					&VidStream);

	// Tx side may change due to sink monitor capability
	if(downshift4K == 1){ // if sink is 4K monitor,
		VidStream.VmId = VmId; // This will be set as 4K60
		TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
		VidStream.Timing = *TimingPtr;
		VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);
	}

	ConfigFrmbuf_rd(stride, Cfmt, &VidStream);

	fb_rd_start = 1;

}


void resetIp_rd()
{

	//xil_printf("\r\nReset HLS IP \r\n");
//	power_down_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x1);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
//	power_down_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x1);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x3);
	usleep(10000);          //hold reset line

}


void resetIp_wr()
{
	//xil_printf("\r\nReset HLS IP \r\n");
//	power_down_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x2);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
//	power_down_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x2);
	usleep(10000);          //hold reset line
//	power_up_HLSIPs();
	Xil_Out32(XPAR_PROCESSOR_HIER_0_HLS_RST_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
}


void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format){
	XV_axi4s_remap_Set_width(remap, width);
	XV_axi4s_remap_Set_height(remap, height);
	XV_axi4s_remap_Set_ColorFormat(remap, color_format);
	XV_axi4s_remap_Set_inPixClk(remap, in_ppc);
	XV_axi4s_remap_Set_outPixClk(remap, out_ppc);
}

void remap_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{

	//Remap on RX side only converts to 4 PPC

	remap_set(&rx_remap, (int)DpRxSsInst.UsrOpt.LaneCount, 4,
				Msa[0].Vtm.Timing.HActive,  Msa[0].Vtm.Timing.VActive
				, 0);

	XV_axi4s_remap_EnableAutoRestart(&rx_remap);
	XV_axi4s_remap_Start(&rx_remap);
}


void remap_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{

	u8 tx_ppc = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_USER_PIXEL_WIDTH);

	if(downshift4K == 1 && (Msa[0].Vtm.Timing.HActive >= 7680 &&
			Msa[0].Vtm.Timing.VActive >= 4320)){
		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
			3840,
			2160
			, 0);
	}
	// 4K120 will be changed to 4K60
	else if(downshift4K == 1 &&
			(Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive
			* Msa[0].Vtm.Timing.VActive > 4096*2160*60)){

		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
			3840,
			2160
			, 0);

	}else{
		remap_set(&tx_remap, 4, tx_ppc, //Msa[0].UserPixelWidth,
				Msa[0].Vtm.Timing.HActive,
				Msa[0].Vtm.Timing.VActive,
			0);

	}

	XV_axi4s_remap_EnableAutoRestart(&tx_remap);

	XV_axi4s_remap_Start(&tx_remap);
}

u8 write_stop = 0;


void bufferWr_callback(void *InstancePtr){
	u32 Status;

	if(XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x10000000) + (0x10000000 * 3))){
		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000) + (0x10000000 * 2) +
										offset_rd);
		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x10000000);
	}else{
		XVFRMBUFRD_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + offset_rd;
		XVFRMBUFWR_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + 0x10000000;
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Write buffer address\r\n");
	}

	if (fb_rd_start) {
	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
	}
	}
}


void bufferRd_callback(void *InstancePtr){

}


/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive;
	u16 MMWidthBytes = AXIMMDataWidth/8;

	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
	  || (Cfmt == XVIDC_CSF_MEM_Y10)) {
	// 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
	stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}
	else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
		   || (Cfmt == XVIDC_CSF_MEM_Y8)) {
	// 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
	stride = ((width+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}
	else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)) {
	// 3 bytes per pixel (RGB8, YUV8)
	stride = (((width*3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}
	else {
	// 4 bytes per pixel
	stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}

	return(stride);
}

#ifdef RxOnly

u32 rx_maud_dup = 0;
u32 rx_naud_dup = 0;
u8 start_i2s_clk = 0;
u8 lock = 0;
u32 appx_fs_dup = 0;
extern u32 maud_dup;
extern u32 naud_dup;



void Dppt_DetectAudio (void) {

#if ENABLE_AUDIO
	u32 rx_maud = 0;
	u32 rx_naud = 0;
	u32 appx_fs = 0;
	u8 i2s_invalid = 0;

	rx_maud = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_AUDIO_MAUD);
	rx_naud = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_AUDIO_NAUD);
	appx_fs = DpRxSsInst.UsrOpt.LinkRate;
	appx_fs = (appx_fs*270);

	appx_fs = appx_fs * rx_maud;
	appx_fs = (appx_fs / rx_naud) * 100;
	appx_fs = (appx_fs * 1000) / 512;
	appx_fs = appx_fs / 1000;

    if (appx_fs >= 31 && appx_fs <= 33) {
	appx_fs = 32000;
	lock = 0;

	} else if (appx_fs >= 43 && appx_fs <= 45) {
		appx_fs = 44100;
		lock = 0;

	} else if (appx_fs >= 47 && appx_fs <= 49) {
		appx_fs = 48000;
		lock = 0;

	} else {
		//invalid
		i2s_invalid = 1;
		if (lock == 0) {
			xil_printf ("This Audio Sampling Fs is not supported by "
					"this design\r\n");
			lock = 1;
		}
	}

    if ((rx_maud != maud_dup)) {
	XACR_WriteReg (RX_ACR_ADDR, RXACR_MAUD, rx_maud); // divider
	maud_dup = rx_maud;

    }
    if ((rx_naud != naud_dup)) {
	XACR_WriteReg (RX_ACR_ADDR, RXACR_NAUD, rx_naud); // divider
	naud_dup = rx_naud;
    }

	if ((appx_fs_dup != appx_fs) && (i2s_invalid == 0)) {
		XACR_WriteReg (RX_ACR_ADDR, RXACR_ENABLE, 0x1);
	} else {

	}

	if ((appx_fs_dup != appx_fs) && (i2s_invalid == 0)) {
		XACR_WriteReg (RX_ACR_ADDR, RXACR_MODE, 0x1);
		start_i2s_clk = 1;
		appx_fs_dup = appx_fs;
	}
#endif

}


// This process takes in all the MSA values and find out resolution, BPC,
// refresh rate. Further this sets the pixel_width based on the pixel_clock and
// lane set. This is to ensure that it matches the values in TX driver. Else
// video cannot be passthrough. Approximation is implemented for refresh rates.
// Sometimes a refresh rate of 60 is detected as 59
// and vice-versa. Approximation is done for single digit.

/*
 * This function is a call back to write the MSA values to Tx as they are
 * read from the Rx, instead of reading them from the Video common library
 */

u8 tx_ppc_set = 0;

int Dppt_DetectResolution(void *InstancePtr,
							XDpTxSs_MainStreamAttributes Msa[4]){

	u32 DpHres = 0;
	u32 DpVres = 0;
	int i = 0;
	XVidC_VideoMode VmId_1;

	frameBuffer_stop_wr(Msa);

	while ((DpHres == 0 || i < 300) && DpRxSsInst.link_up_trigger == 1) {
		DpHres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HRES);
		i++;
	}
	while ((DpVres == 0 || i < 300) && DpRxSsInst.link_up_trigger == 1) {
		DpVres = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VHEIGHT);
		i++;
	}

	// Assuming other MSAs would be stable by this time
	u32 DpHres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_HTOTAL);
	u32 DpVres_total = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_VTOTAL);
	u32 rxMsamisc0 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC0);
	u32 rxMsamisc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC1);
	u32 rxMsaMVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MVID);
	u32 rxMsaNVid = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_NVID);


	Msa[0].Misc0 = rxMsamisc0;
	Msa[0].Misc1 = rxMsamisc1;
	rxMsamisc0 = ((rxMsamisc0 >> 5) & 0x00000007);
//	u8 comp = ((rxMsamisc0 >> 1) & 0x00000003);

	u8 Bpc[] = {6, 8, 10, 12, 16};


	Msa[0].Vtm.Timing.HActive = DpHres;
	Msa[0].Vtm.Timing.VActive = DpVres;
	Msa[0].Vtm.Timing.HTotal = DpHres_total;
	Msa[0].Vtm.Timing.F0PVTotal = DpVres_total;
	Msa[0].MVid = rxMsaMVid;
	Msa[0].NVid = rxMsaNVid;
	Msa[0].HStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSTART);
	Msa[0].VStart =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSTART);

	Msa[0].Vtm.Timing.HSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSWIDTH);
	Msa[0].Vtm.Timing.F0PVSyncWidth =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSWIDTH);

	Msa[0].Vtm.Timing.HSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL);
	Msa[0].Vtm.Timing.VSyncPolarity =
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VSPOL);


	Msa[0].SynchronousClockMode = rxMsamisc0 & 1;
	u8 bpc = Bpc[rxMsamisc0];
	Msa[0].BitsPerColor = bpc;

	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( (Msa[0].Misc0 & 0x6 ) == 0x2  ) {
	//YUV422
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if( (Msa[0].Misc0 & 0x6 ) == 0x4  ) {
	//RGB or YUV444
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
		Msa[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;


	u32 recv_clk_freq =
		(((int)DpRxSsInst.UsrOpt.LinkRate*27)*rxMsaMVid)/rxMsaNVid;
//	xil_printf ("Rec clock is %d\r\n",recv_clk_freq);

	float recv_frame_clk =
		(int)( (recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) < 0.0 ?
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total) :
				(recv_clk_freq*1000000.0)/(DpHres_total * DpVres_total)+0.9
				);

	XVidC_FrameRate recv_frame_clk_int = recv_frame_clk;
	//Doing Approximation here
	if (recv_frame_clk_int == 49 || recv_frame_clk_int == 51) {
		recv_frame_clk_int = 50;
	} else if (recv_frame_clk_int == 59 || recv_frame_clk_int == 61) {
		recv_frame_clk_int = 60;
	} else if (recv_frame_clk_int == 29 || recv_frame_clk_int == 31) {
		recv_frame_clk_int = 30;
	} else if (recv_frame_clk_int == 76 || recv_frame_clk_int == 74) {
		recv_frame_clk_int = 75;
	} else if (recv_frame_clk_int == 121 || recv_frame_clk_int == 119) {
		recv_frame_clk_int = 120;
	}


	Msa[0].Vtm.FrameRate = recv_frame_clk_int;


	Msa[0].PixelClockHz = DpHres_total * DpVres_total * recv_frame_clk_int;
	Msa[0].DynamicRange = XDP_DR_CEA;
	Msa[0].YCbCrColorimetry = XDP_TX_MAIN_STREAMX_MISC0_YCBCR_COLORIMETRY_BT601;

	if((recv_clk_freq*1000000)>540000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount==4){
		tx_ppc_set = 0x4;
		Msa[0].UserPixelWidth = 0x4; //(int)DpRxSsInst.UsrOpt.LaneCount;

	}
	else if((recv_clk_freq*1000000)>270000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount!=1){
		tx_ppc_set = 0x2;
		Msa[0].UserPixelWidth = 0x2; //(int)DpRxSsInst.UsrOpt.LaneCount;

	}
	else{
		tx_ppc_set = 0x1;
		Msa[0].UserPixelWidth = 0x1; //(int)DpRxSsInst.UsrOpt.LaneCount;

	}

	XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, (int)DpRxSsInst.UsrOpt.LaneCount);
	Msa[0].OverrideUserPixelWidth = 1;

	XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxDtgEn(DpRxSsInst.DpPtr);

	xil_printf(
		"*** Detected resolution: "
			"%lu x %lu @ %luHz, BPC = %lu, PPC = %d***\n\r",
		DpHres, DpVres,recv_frame_clk_int,bpc,(int)DpRxSsInst.UsrOpt.LaneCount
	);

	VmId_1 = XVidC_GetVideoModeId(
			Msa[0].Vtm.Timing.HActive,
			Msa[0].Vtm.Timing.VActive,
			Msa[0].Vtm.FrameRate,0);

	frameBuffer_start_wr(VmId_1, Msa, 0);

#if PHY_COMP
		CalculateCRC();
#endif

		return 1;

}
#endif


#if defined(PT) || defined(LB)
void DpPt_TxSetMsaValuesImmediate(void *InstancePtr){

	/* Set the main stream attributes to the associated DisplayPort TX core
	 * registers. */
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HTOTAL +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_HTOTAL));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VTOTAL +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
						XDP_RX_MSA_VTOTAL));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr,XDP_TX_MAIN_STREAM_POLARITY+
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HSPOL)|
			(XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_VSPOL) <<
			XDP_TX_MAIN_STREAMX_POLARITY_VSYNC_POL_SHIFT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSWIDTH+
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSWIDTH +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSWIDTH));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HRES +
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_HRES));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VRES +
			StreamOffset[0],
			XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_VHEIGHT));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_HSTART +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_HSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_VSTART +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_VSTART));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0 +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC0));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC1 +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC1));
	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH +
		StreamOffset[0], tx_ppc_set);
//		XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_USER_PIXEL_WIDTH)
//			);



	/* Check for YUV422, BPP has to be set using component value to 2 */
	if( ( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_MSA_MISC0))
			 & 0x6 ) == 0x2  ) {
	//YUV422
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	}
	else if(( (XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,XDP_RX_MSA_MISC0))
			 & 0x6 ) == 0x4){
	// YUV444
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}else
	// RGB
		DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].ComponentFormat =
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
}
#endif
