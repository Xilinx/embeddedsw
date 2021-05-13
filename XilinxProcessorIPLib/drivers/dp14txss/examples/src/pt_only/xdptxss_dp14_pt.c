/*******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file dptxss_pt_dp14.c
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
* 1.01 KU 04/01/19 2019.1 - Updates to RX SetLinkRate, audio enable.
*                         - Setting Tx link to 1.62 for improved linkup
*                         - Audio starts after infoframe
* 1.02 KU 06/01/19 2019.2 - Using API to send AudioInfoframe on TX.
*                         - CRC is reported on each component
*                         - checking for 0x84 type frame in Infoframe
* 1.03 KU 12/23/19 2020.1 - Minor updates
* 1.04 ND 02/14/20 2020.1 - mcdp related function call now need dprxss instance address
*                           instead of base address  as first parameter
* 1.05 ND 03/06/20 2020.1 - Added support for VCK190.
* 						    Application file are common for VCK190 and
* 						    ZCU102 PT design.
* 1.06 ND 07/28/20 2020.2 - Minor updates and removal of redundant code related to PSIIC
* 							initialization.
* 1.07 KU 09/02/20 2020.2 - Added support for Adaptive Sync, HDR InfoFrames and
* 							Colorimetry Information over VSC packets
* 							New Interrupt added for Vsync On TX
* 							This application does not support Y420
* 							Added support for Fabric 8b10b for Versal
* 1.08 KU 11/19/20 2021.1 - Updated the CRC IP register programming
* 							for VSC based resolution.
* 1.09 ND 02/22/21 2021.1 - Updated changes to reset Test_crc counter in DP dpcd
* 							space to avoid Race condition. Updated changes
* 							related to CRC being written to DPCD for 422 format.
* 1.10 ND 04/01/21 2021.1 - Moved all global variables declaration from .h to .c
* 							files due to gcc compiler compilation error.
* 1.11 KU 04/12/21 2021.1 -	Updates to Versal GT programming to get /20 clock
* 1.12 ND 04/15/21 2021.1 - Change in Interrupt Event name of Adaptive sync
* 							interrupt event. Added stream parameter XDP_TX_STREAM_ID1
* 							in XDpRxSs_GetVblank().
* 							CTS related updates
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "main.h"

#ifndef versal
#include "si5328drv.h"
#endif
//
#define PS_IIC_CLK 400000
//
XIicPs Ps_Iic1;
//
#define ENABLE_AUDIO XPAR_DP_RX_HIER_0_V_DP_RXSS1_0_AUDIO_ENABLE
#define 			XINTC XScuGic

#if ENABLE_AUDIO
XGpio_Config  *aud_gpio_ConfigPtr;
XGpio   aud_gpio;
XAxis_Switch axis_switch_rx;
XAxis_Switch axis_switch_tx;
XI2srx_Config *Config_rx;
XI2stx_Config *Config;
XI2s_Rx I2s_rx;
XI2s_Tx I2s_tx;
#endif

#ifdef versal
XClk_Wiz_Config *CfgPtr_Dynamic;
XClk_Wiz ClkWiz_Dynamic;
#if (VERSAL_FABRIC_8B10B == 1)
XClk_Wiz_Config *CfgPtr_Dynamic_rx;
XClk_Wiz ClkWiz_Dynamic_rx;
#endif
#endif

#ifdef Tx
#include "tx.h"
#endif
#ifdef Rx
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
int I2cClk_Ps(u32 InFreq, u32 OutFreq);
u32 DpSs_PhyInit(u16 DeviceId);
u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr);

u32 frame_array[3] = {0x10000000, 0x20000000, 0x30000000}; //, 0x40000000};
u32 frame_array_y[3] = {0x40000000, 0x50000000, 0x60000000};
u8 frame_pointer = 0;
u8 frame_pointer_rd = 2;
u8 not_to_read = 1;
u8 not_to_write = 3;
u8 fb_rd_start = 0;
u32 vblank_init = 0;
u8 vblank_captured = 0;
u16 fb_wr_count = 0;
u16 fb_rd_count = 0;
u64 XVFRMBUFRD_BUFFER_BASEADDR_Y;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR_Y;
XINTC IntcInst; 	/* The interrupt controller instance. */
XIicPs_Config *XIic1Ps_ConfigPtr;
Video_CRC_Config VidFrameCRC_rx; /* Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx;
u8 use_vsc;
XTmrCtr TmrCtr; 		/* Timer instance.*/
XIic IicInstance; 	/* I2C bus for MC6000 and IDT */

#ifndef versal
XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/
#else
void* VPhyInst;
#endif

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
#ifndef versal
XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
#endif

XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
XV_FrmbufRd_l2     frmbufrd;
#endif
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
XV_FrmbufWr_l2     frmbufwr;
#endif
extern u32 StreamOffset[4];
extern u8 supports_adaptive;
extern XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
XV_axi4s_remap_Config   *rx_remap_Config;
XV_axi4s_remap          rx_remap;
XV_axi4s_remap_Config   *tx_remap_Config;
XV_axi4s_remap          tx_remap;
#endif
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

//	I2C_Scan(XPAR_IIC_0_BASEADDR);

	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x52;
#ifdef versal
	  Status = XIicPs_MasterSendPolled(&Ps_Iic1,(u8 *)&Buffer,
									  1,I2C_VFMCEXP_0_ADDR);
	    if(Status == XST_SUCCESS)
	    {
		ByteCount=1;
	    }
	    else
	    {
		ByteCount=0;
	    }
#else
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_0_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
#endif
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
#ifdef versal
	Status = XIicPs_MasterSendPolled(&Ps_Iic1,
	           (u8 *)&Buffer,1,I2C_VFMCEXP_1_ADDR);
    if(Status == XST_SUCCESS)
    {
	ByteCount=1;
    }
    else
    {
	ByteCount=0;
    }
#else
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_VFMCEXP_1_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
#endif
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}
//	xil_printf(" done!\n\r");
	Status = IDT_8T49N24x_Init(IIC_BASE_ADDR, I2C_IDT8N49_ADDR);

	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}
//	Status = TI_LMK03318_Init(XPAR_IIC_0_BASEADDR, I2C_LMK03318_ADDR);
	Status = TI_LMK03318_PowerDown(IIC_BASE_ADDR, I2C_LMK03318_ADDR);
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

	/* Initialize ICache */
	Xil_ICacheInvalidate();
	Xil_ICacheDisable();

	/* Initialize DCache */
	Xil_DCacheInvalidate();
	Xil_DCacheDisable();

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
#if ADAPTIVE
	xil_printf("                      Adaptive Sync Mode : %d \r\n",
					1 * ADAPTIVE_TYPE);
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

#ifndef versal
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

#else
  Buffer = mux;
  Status = XIicPs_MasterSendPolled(&Ps_Iic1,
                                   (u8 *)&Buffer,
                                   1,
									  I2C_MUX_ADDR_SI);

  if (Status != XST_SUCCESS) {
          xil_printf ("mux failed\r\n");
  }
#endif

  return Status;
}


int I2cClk_Ps(u32 InFreq, u32 OutFreq)
{
  int Status;

  I2cMux_Ps(0);

  /* Free running mode */
  if (InFreq == 0) {
#ifndef versal
     Status = Si5328_SetClock_Ps(&Ps_Iic1, (I2C_ADDR_SI5328),
                            (SI5328_CLKSRC_XTAL), (SI5328_XTAL_FREQ), OutFreq);

     if (Status != (SI5328_SUCCESS)) {
          print("Error programming SI5328. FM\r\n");
//          return 0;
     }
#endif
  }
  /* Locked mode */
  else {
#ifndef versal
     Status = Si5328_SetClock_Ps(&Ps_Iic1, (I2C_ADDR_SI5328),
                                (SI5328_CLKSRC_CLK1), InFreq, OutFreq);

     if (Status != (SI5328_SUCCESS)) {
        print("Error programming SI5328. LM\r\n");
//        return 0;
     }
#endif
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
    u32 dprx_sts = 0;
    u32 dptx_sts = 0;
    u32 retval = 0;
#ifdef Rx
	XDpRxSs_Config *ConfigPtr_rx;
#endif
#ifdef Tx
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

#ifdef Rx
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
#endif

#ifdef Tx
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
#endif
	/* Setup Video Phy, left to the user for implementation */

	DpSs_PhyInit(XVPHY_DEVICE_ID);
    u32 loop = 0;
    u32 good;

#ifdef Tx

#ifdef versal
    //de-asserting TX reset to GT
    GtCtrl (GT_RST_MASK, 0x00000000, 1);
    good = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
    good &= ALL_LANE;
    while ((good != ALL_LANE) && loop < 10000) {
        good = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
        good &= ALL_LANE;
        loop++;
    }
    if (loop == 10000) {
	xil_printf("+\r\n");
        // Issue one more reset. Sometimes the first reset may not complete the
        // POR
        GtCtrl (GT_RST_MASK, 0x00000001, 1);
        GtCtrl (GT_RST_MASK, 0x00000000, 1);
    }
    loop = 0;
    good = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
    good &= ALL_LANE;
    while ((good != ALL_LANE) && loop < 10000) {
        good = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x280);
        good &= ALL_LANE;
        loop++;
    }
    if (loop == 10000) {
	xil_printf("++\r\n");
    }
    loop = 0;
#endif

	/* Set DP141 Tx driver here. */
    //Keeping 0db gain on RX
    //Adding 6db gain on TX
	//Adding some Eq gain
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x02, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x05, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x08, 0x3C);
    i2c_write_dp141(XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, 0x0B, 0x3C);

#endif

#ifdef versal
#if (VERSAL_FABRIC_8B10B == 1)
        /* Unlocking the NPI space so that GT CH1 divider value
         * can be programmed. This will generate a /20 clk
         */
	XDp_WriteReg(GT_QUAD_BASE,0xC,0xF9E8D7C6);
	retval = XDp_ReadReg(GT_QUAD_BASE, CH1CLKDIV_REG);
	retval &= ~DIV_MASK;
	retval |= DIV;
	XDp_WriteReg(GT_QUAD_BASE, CH1CLKDIV_REG, retval);
#endif
#endif

	config_phy(0x14);

#ifdef Rx
#ifdef versal

      //release reset to RX GT
    GtCtrl (GT_RST_MASK, 0x00000000, 0);
    loop = 0;
    good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
    good &= ALL_LANE;
    while ((good != ALL_LANE) && loop < 10000) {
        good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
        good &= ALL_LANE;
        loop++;
    }
    if (loop == 10000) {
	xil_printf("-\r\n");
        // Issue one more reset. Sometimes the first reset may not complete the
        // POR
        GtCtrl (GT_RST_MASK, 0x00000001, 0);
        GtCtrl (GT_RST_MASK, 0x00000000, 0);
    }
    loop = 0;
    good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
    good &= ALL_LANE;
    while ((good != ALL_LANE) && loop < 10000) {
        good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
        good &= ALL_LANE;
        loop++;
    }

    if (loop == 10000) {
	xil_printf("--\r\n");
    }

    GtCtrl (GT_RATE_MASK, (VERSAL_810G << 1), 0); //bridge << 1); //rate
    loop = 0;
    good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
    good &= ALL_LANE;
    while ((good != ALL_LANE) && loop < 10000) {
        good = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x208);
        good &= ALL_LANE;
        loop++;
    }

#if (VERSAL_FABRIC_8B10B == 1)
	loop = 0;
	//issue reset
	XClk_Wiz_WriteReg(CfgPtr_Dynamic_rx->BaseAddr, 0x0, 0xA);
	while(!(XClk_Wiz_ReadReg(CfgPtr_Dynamic_rx->BaseAddr, XCLK_WIZ_STATUS_OFFSET) & 1)) {
		if(loop == 10000) {
				break;
		}
		usleep(100);
		loop++;
	}
	if (loop == 10000) {
		xil_printf ("Rx Clk_wizard failed to lock\r\n");
	}
#endif
#endif

	/*Megachips Retimer Initialization*/
#ifdef versal
	DpRxSsInst.IicPsPtr = &Ps_Iic1;
#endif
	XDpRxSs_McDp6000_init(&DpRxSsInst);

	/* issue HPD at here to inform DP source */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);

#endif

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	/* FrameBuffer initialization. */
	Status = XVFrmbufRd_Initialize(&frmbufrd, FRMBUF_RD_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Read "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Status = XVFrmbufWr_Initialize(&frmbufwr, FRMBUF_WR_DEVICE_ID);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Write "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}
#endif

	resetIp_rd();
	resetIp_wr();

#if ENABLE_AUDIO
#ifdef versal
	//programming IDT_8T49N24x for Audio clock
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR_1, 0, 36864000, TRUE);
#else
	// programming Si5328 for 512*Fs for Audio clock
	I2cMux_Ps(0);
    Si5328_Init_Ps(&Ps_Iic1, I2C_ADDR_SI5328);
    I2cClk_Ps(0, 36864000);
#endif
#endif

	DpSs_SetupIntrSystem();

    operationMenu();
	while (1) {
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

#ifndef versal
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
#endif
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
#ifdef versal
    RecievedByte = XUartPsv_ReadReg(STDIN_BASEADDRESS, XUARTPSV_UARTDR_OFFSET);
#else
	RecievedByte = XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
#endif
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
#ifdef Rx
void CalculateCRC(void)
{

    u32 RegVal;
    u8 color_mode = 0;
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

//    VidFrameCRC_rx.Mode_422 =
//                    (XVidFrameCrc_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
//                                          XDP_RX_MSA_MISC0) >> 1) & 0x3;

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);

	if(color_mode == 2){
		VidFrameCRC_rx.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx.Mode_422 = 0x0;
	}


    if (VidFrameCRC_rx.Mode_422 != 0x1) {
	XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                          VIDEO_FRAME_CRC_CONFIG,
                            4/*DpRxSsInst.UsrOpt.LaneCount*/);
    } else { // 422
        XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                              VIDEO_FRAME_CRC_CONFIG,
                                (/*DpRxSsInst.UsrOpt.LaneCount*/4 | 0x80000000));
    }
#if PHY_COMP
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
				 VidFrameCRC_rx.Pixel_r);
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                 XDP_RX_CRC_COMP1,
                                                  VidFrameCRC_rx.Pixel_g);
    /* Check for 422 format and move CR/CB
     * calculated CRC to G component.
     * Place as tester needs this way
     * */
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr,
                 XDP_RX_CRC_COMP2,
                                                    VidFrameCRC_rx.Pixel_b);
    if(DpRxSsInst.no_video_trigger == 0){
    VidFrameCRC_rx.TEST_CRC_CNT = 1;
    XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, XDP_RX_CRC_CONFIG,
                 (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
                  VidFrameCRC_rx.TEST_CRC_CNT));
    }
#if !PHY_COMP
    xil_printf("[Video CRC] R/Cr: 0x%x, G/Y: 0x%x, B/Cb: 0x%x\r\n\n",
               VidFrameCRC_rx.Pixel_r, VidFrameCRC_rx.Pixel_g,
               VidFrameCRC_rx.Pixel_b);
#endif

#endif

}
#endif

#ifdef versal
void GtCtrl(u32 mask, u32 data, u8 is_tx)
{
	u32 readval;
	if (is_tx == 1) {
#ifdef Tx
		readval = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x04C);
#endif
	} else {
#ifdef Rx
		readval = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x04C);
#endif
	}

	readval &= (~mask);
	readval |= data;

	if (is_tx == 1) {
#ifdef Tx
		XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, 0x04C, readval);
#endif
	} else {
#ifdef Rx
		XDp_WriteReg(DpRxSsInst.DpPtr->Config.BaseAddr, 0x04C, readval);
#endif
	}
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
    u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	/* Write data */
//	Buffer[0] = (RegisterAddress >> 8);
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
#ifndef versal
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
#else
	    Status = XIicPs_MasterSendPolled(&Ps_Iic1,
	                                             (u8 *)&Buffer,
	                                             2,
												 I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;

			// Maximum retries
			if (Retry == 255) {
				return XST_FAILURE;
			}
		}

		else {
			return XST_SUCCESS;
		}
	}
}

void read_DP141()
{
	u8 Data;
	int i =0;
//	u8 Buffer[1];

#ifndef versal
	for (i = 0 ; i < 0xD ; i++) {
		Data = i2c_read_dp141(XPAR_IIC_0_BASEADDR,
				      I2C_TI_DP141_ADDR, i);
		xil_printf("%x : %02x \r\n",i, Data);
	}
#else
    u8 Buffer;
    u32 Status;
    Buffer = 0x02 & 0xff;
    Status = XIicPs_MasterSendPolled(&Ps_Iic1,
                                             (u8 *)&Buffer,
                                             1,
											 I2C_TI_DP141_ADDR);


//        for(i=0; i<0xD; i++){
//                Data = i2c_read_dp141( XPAR_IIC_0_BASEADDR, I2C_TI_DP141_ADDR, i);
                XIicPs_MasterRecvPolled(&Ps_Iic1, &Data, 1, I2C_TI_DP141_ADDR);
                xil_printf("%x : %02x \r\n",i, Data);
//        }
#endif
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
	u8 Buffer[2];
	int ByteCount;

	XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
	int result=XST_SUCCESS;
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

	/*Initializing vck190 onboard IDT8T49N24X for audio*/
#ifdef versal

	result=IDT_8T49N24x_Init(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR_1);
	if(result!=XST_SUCCESS)
	{
		xil_printf("IDT_8T49N24x_Init failed\r\n");
	}

   /*
	  * Get the CLK_WIZ Dynamic reconfiguration driver instance
	  */
	 CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XPAR_CLK_WIZ_0_DEVICE_ID);
	 if (!CfgPtr_Dynamic) {
			 return XST_FAILURE;
	 }

	 /*
	  * Initialize the CLK_WIZ Dynamic reconfiguration driver
	  */
	 Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
			  CfgPtr_Dynamic->BaseAddr);
	 if (Status != XST_SUCCESS) {
			 return XST_FAILURE;
	 }

#if (VERSAL_FABRIC_8B10B == 1)

	 CfgPtr_Dynamic_rx = XClk_Wiz_LookupConfig(XPAR_CLK_WIZ_1_DEVICE_ID);
	 if (!CfgPtr_Dynamic_rx) {
			 return XST_FAILURE;
	 }

	 /*
	  * Initialize the CLK_WIZ Dynamic reconfiguration driver
	  */
	 Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic_rx, CfgPtr_Dynamic_rx,
			  CfgPtr_Dynamic_rx->BaseAddr);
	 if (Status != XST_SUCCESS) {
			 return XST_FAILURE;
	 }


#endif
#endif

	/* Initialize CRC & Set default Pixel Mode to 1. */
#ifdef Rx
	VidFrameCRC_rx.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx);
#endif
#ifdef Tx
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

//   /* Initialize PS IIC1 */
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

	/* Set the I2C Mux to select the HPC FMC */
#ifdef versal
	I2cMux_Ps(0x04);
#else
	Buffer[0] = 0x01;
	ByteCount = XIic_Send(XPAR_IIC_0_BASEADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}
#endif

	VideoFMC_Init();

#ifndef versal
	IDT_8T49N24x_SetClock(XPAR_IIC_0_BASEADDR, I2C_IDT8N49_ADDR, 0,
            270000000, TRUE);
#else
	IDT_8T49N24x_SetClock(0x0, I2C_IDT8N49_ADDR, 0,
            270000000, TRUE);
#endif

	usleep(300000);


#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES

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
#endif

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
#ifdef Tx
	DpTxSs_SetupIntrSystem();
#endif
	// Rx side
#ifdef Rx
	DpRxSs_SetupIntrSystem();
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE,
								&bufferWr_callback, &frmbufwr);
#endif

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	XVFrmbufRd_SetCallback(&frmbufrd, XVFRMBUFRD_HANDLER_DONE,
								&bufferRd_callback, &frmbufrd);
#endif

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
#ifdef Rx
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
				(Xil_InterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}
	/* Enable the interrupt for the DP device */
	XScuGic_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
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

#endif
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
#ifdef Tx
	Status = XScuGic_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
				(Xil_InterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/* Enable the interrupt */
	XScuGic_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	Status = XScuGic_Connect(IntcInstPtr,
				 XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID,
				 (Xil_InterruptHandler)XVFrmbufRd_InterruptHandler,
				 &frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

	/* Disable the interrupt vector at the interrupt controller */
	XScuGic_Enable(IntcInstPtr, XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID);
#endif

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
	u32 Status;

	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;
#ifdef versal
	Status = XIicPs_MasterSendPolled(&Ps_Iic1,
		                                             (u8 *)&Buffer,
		                                             2,
													 I2CSlaveAddress);
	if(Status == XST_SUCCESS)
	{
		ByteCount = 2;
	}
#else
	ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
			      (u8*)Buffer, 2, XIIC_STOP);
#endif

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
u32 DpSs_PhyInit(u16 DeviceId)
{
#ifndef versal
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
#else

     //Setting Vswing on TX
     GtCtrl (0x1F00,(5 << 8), 1);

#endif

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
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	int Status;

	/* Stop Frame Buffers */

	XVFRMBUFWR_BUFFER_BASEADDR = frame_array[frame_pointer];
	XVFRMBUFWR_BUFFER_BASEADDR_Y = frame_array_y[frame_pointer];

	Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
		return(XST_FAILURE);
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	Status |= XVFrmbufWr_SetChromaBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR_Y);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Write "
			"buffer address\r\n");
		return(XST_FAILURE);
	}

	/* Enable Interrupt */
	XVFrmbufWr_InterruptEnable(&frmbufwr,
			XVFRMBUFWR_HANDLER_DONE);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	/* Start Frame Buffers */
	XVFrmbufWr_Start(&frmbufwr);

	//xil_printf("INFO: FRMBUFwr configured\r\n");
	return(Status);
#endif
}

u8 stopped = 1;
u8 start_rdfb = 0;

/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 * The FrramBuffer is put in Autorestart mode for non-Adaptive Sync mode
 * When Adaptive Sync mode is enabled the FrameBuffer is put in manual mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int ConfigFrmbuf_rd(u32 StrideInBytes,
						XVidC_ColorFormat Cfmt,
						XVidC_VideoStream *StreamPtr)
	{
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	int Status;

	XVFRMBUFRD_BUFFER_BASEADDR = frame_array[frame_pointer];
	XVFRMBUFRD_BUFFER_BASEADDR_Y = frame_array_y[frame_pointer];

	/* Configure  Frame Buffers */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes, Cfmt, StreamPtr);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
		return(XST_FAILURE);
	}

	if (!supports_adaptive || !ADAPTIVE_TYPE) {
		Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
		Status |= XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR_Y);
		if(Status != XST_SUCCESS) {
			xil_printf("ERROR:: Unable to configure Frame Buffer "
					"Read buffer address\r\n");
			return(XST_FAILURE);
		}
	}

	/* Enable Interrupt */
	XVFrmbufRd_InterruptEnable(&frmbufrd,
			XVFRMBUFRD_HANDLER_DONE);

	/* When Adaptive mode is 0 or Monitor does not support Adaptive Sync
	 * the FB read is configured in AutoEnableRestart mode
	 */
	if (!supports_adaptive || !ADAPTIVE_TYPE) {
		XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
		/* Start Frame Buffers */
		XVFrmbufRd_Start(&frmbufrd);
	}
	//xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
#endif
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

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	int Status;

	/* Stop Frame Buffers */
//	Status = XVFrmbufRd_Stop(&frmbufrd);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Failed to stop XVFrmbufRd\r\n");
//	}
//
//	resetIp_rd();
	XVFRMBUFRD_BUFFER_BASEADDR = frame_array[frame_pointer_rd] + offset;
	XVFRMBUFRD_BUFFER_BASEADDR_Y = frame_array_y[frame_pointer_rd] + offset;

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
	Status |= XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR_Y);
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

//	xil_printf("INFO: FRMBUFrd configured\r\n");
	return(Status);
#endif
}


void frameBuffer_stop() {

	fb_rd_start = 0;
	frameBuffer_stop_rd();
	frameBuffer_stop_wr();

}


void frameBuffer_stop_rd() {
//    xil_printf ("FB stop start..\r\n");
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	u32 Status;
	fb_rd_start = 0;
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf ("Failed to stop Frame Buffer Write\r\n");
	}
	resetIp_rd();
	Status = XVFrmbufRd_WaitForIdle(&frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
	usleep(1000);
#endif
}


void frameBuffer_stop_wr() {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	u32 Status;
	Status = XVFrmbufWr_Stop(&frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf ("Failed to stop Frame Buffer Write\r\n");
	}
	resetIp_wr();
	Status = XVFrmbufWr_WaitForIdle(&frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
	usleep (1000);
#endif
}

void frameBuffer_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K) {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES

	XVidC_ColorFormat Cfmt;
	XVidC_VideoStream VidStream;

	resetIp_wr();

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[0].ComponentFormat ==
						XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[0].ComponentFormat ==
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		} else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 10){
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[0].ComponentFormat ==
								XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	}

	VidStream.PixPerClk  = (int)DpRxSsInst.UsrOpt.LaneCount;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;
#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	remap_start_wr(Msa, downshift4K);
#endif
	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
					256,
					&VidStream);
	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);
	stopped = 1;
	fb_wr_count = 0;
#endif
}

void frameBuffer_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K) {
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

	resetIp_rd();

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[0].ComponentFormat ==
						XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[0].ComponentFormat ==
				XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		} else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 10){
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[0].ComponentFormat ==
						XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	}

	VidStream.PixPerClk  = Msa[0].UserPixelWidth;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;

#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES
	remap_start_rd(Msa, downshift4K);
#endif
	/* Configure Frame Buffer */
	// Rx side
	u32 stride = CalcStride(Cfmt,
					256,
					&VidStream);

	// Tx side may change due to sink monitor capability
	if(downshift4K == 1){ // if sink is 4K monitor,
		VidStream.VmId = XVIDC_VM_3840x2160_30_P; //VmId; // This will be set as 4K30
		TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
		VidStream.Timing = *TimingPtr;
		VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);
	}

	ConfigFrmbuf_rd(stride, Cfmt, &VidStream);
	fb_rd_start = 1;
	fb_rd_count = 0;
#endif
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


#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES

void remap_set(XV_axi4s_remap *remap, u8 in_ppc, u8 out_ppc, u16 width,
		u16 height, u8 color_format){
	XV_axi4s_remap_Set_width(remap, width);
	XV_axi4s_remap_Set_height(remap, height);
	XV_axi4s_remap_Set_ColorFormat(remap, color_format);
	XV_axi4s_remap_Set_inPixClk(remap, in_ppc);
	XV_axi4s_remap_Set_outPixClk(remap, out_ppc);
}
#endif


#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES

void remap_start_wr(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{
    u8 color_format = 0;

    if( Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
	color_format = 0x2;
    } else if (Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
	color_format = 0x1;
    }
	//Remap on RX side only converts to 4 PPC
	remap_set(&rx_remap, (int)DpRxSsInst.UsrOpt.LaneCount, 4,
				Msa[0].Vtm.Timing.HActive,  Msa[0].Vtm.Timing.VActive
				, color_format);

	XV_axi4s_remap_EnableAutoRestart(&rx_remap);
	XV_axi4s_remap_Start(&rx_remap);
}
#endif


#ifdef XPAR_XV_AXI4S_REMAP_NUM_INSTANCES

void remap_start_rd(XDpTxSs_MainStreamAttributes Msa[4], u8 downshift4K)
{

	u8 tx_ppc = XDp_ReadReg(DpTxSsInst.DpPtr->Config.BaseAddr,
			XDP_TX_USER_PIXEL_WIDTH);

    u8 color_format = 0;

    if( Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
	color_format = 0x2;
    } else if (Msa[0].ComponentFormat ==
			XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
	color_format = 0x1;
    }


	if(downshift4K == 1 && (Msa[0].Vtm.Timing.HActive >= 7680 &&
			Msa[0].Vtm.Timing.VActive >= 4320)){
		remap_set(&tx_remap, 4, tx_ppc,
			3840,
			2160
			, color_format);
	}
	// 4K120 will be changed to 4K60
	else if(downshift4K == 1 &&
			(Msa[0].Vtm.FrameRate * Msa[0].Vtm.Timing.HActive
			* Msa[0].Vtm.Timing.VActive > 4096*2160*60)){

		remap_set(&tx_remap, 4, tx_ppc,
			3840,
			2160
			, color_format);

	}else{
		remap_set(&tx_remap, 4, tx_ppc,
				Msa[0].Vtm.Timing.HActive,
				Msa[0].Vtm.Timing.VActive,
				color_format);

	}

	XV_axi4s_remap_EnableAutoRestart(&tx_remap);

	XV_axi4s_remap_Start(&tx_remap);
}

#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void bufferWr_callback(void *InstancePtr){
	u32 Status;

	/* If Adaptive Mode is 1, the FB Read is triggered on FB Wr Interrupt
	 * This ensures that the frame rate is maintained at TX (appx)
	 *
	 */
	if (fb_rd_start && supports_adaptive && ADAPTIVE_TYPE) {
		Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFWR_BUFFER_BASEADDR);
		Status |= XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFWR_BUFFER_BASEADDR_Y);
		if(Status != XST_SUCCESS) {
			xil_printf("ERROR:: Unable to configure Frame Buffer "
					"Read buffer address\r\n");
		}
		if (stopped) {
			XVFrmbufRd_Start(&frmbufrd);
			start_rdfb = 0;
			stopped = 0;
		} else {
			/* Ideally this should never be hit in Adaptive Mode 1
			 * However it can get hit due to system latencies or
			 * due to delay in servicing interrupts.
			 * Setting start_rdfb to 1. If this is 1 then FB RD will be
			 * started when RD interrupt is asserted
			 */
            start_rdfb = 1;
//	        xil_printf(ANSI_COLOR_RED"FrameBUffer RD is not idle !!"ANSI_COLOR_RESET"\r\n");
		}
	}


	if(XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x10000000) + (0x10000000 * 2))){

		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x10000000) + (0x10000000 * 1) +
										offset_rd);
		XVFRMBUFRD_BUFFER_BASEADDR_Y = (0 + (0x40000000) + (0x10000000 * 1) +
										offset_rd);

		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x10000000);
		XVFRMBUFWR_BUFFER_BASEADDR_Y = 0 + (0x40000000);
	}else{
		XVFRMBUFRD_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + offset_rd;
		XVFRMBUFRD_BUFFER_BASEADDR_Y = XVFRMBUFWR_BUFFER_BASEADDR_Y + offset_rd;

		XVFRMBUFWR_BUFFER_BASEADDR = XVFRMBUFWR_BUFFER_BASEADDR + 0x10000000;
		XVFRMBUFWR_BUFFER_BASEADDR_Y = XVFRMBUFWR_BUFFER_BASEADDR_Y + 0x10000000;
	}

	Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
	Status |= XVFrmbufWr_SetChromaBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR_Y);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Write buffer address\r\n");
	}

	/* In Non-Adaptive Scenario, the FB Read is in Autorestart mode
	 *
	 */

	if (fb_rd_start && (!supports_adaptive || !ADAPTIVE_TYPE)) {
		Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
		Status |= XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR_Y);
		if(Status != XST_SUCCESS) {
			xil_printf("ERROR:: Unable to configure Frame Buffer "
					"Read buffer address\r\n");
		}
	}
	fb_wr_count++;
}
#endif

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
void bufferRd_callback(void *InstancePtr){
	stopped = 1;
	if (start_rdfb) {
		start_rdfb = 0;
		XVFrmbufRd_Start(&frmbufrd);
		stopped = 0;
	}
	fb_rd_count++;
}
#endif


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

	} else if (Cfmt == XVIDC_CSF_MEM_YUYV8) {
		stride = (((width*2)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	} else {
	// 4 bytes per pixel
	stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;

	}

	return(stride);
}

#ifdef Rx

u32 rx_maud_dup = 0;
u32 rx_naud_dup = 0;
volatile u8 start_i2s_clk = 0;
u8 lock = 0;
volatile u32 appx_fs_dup = 0;
u32 maud_dup = 0;
u32 naud_dup = 0;
extern XilAudioInfoFrame_rx AudioinfoFrame;

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
//		xil_printf ("^^\r\n");

	} else if (appx_fs >= 43 && appx_fs <= 45) {
		appx_fs = 44100;
		lock = 0;
//		xil_printf ("^^\r\n");

	} else if (appx_fs >= 47 && appx_fs <= 49) {
		appx_fs = 48000;
		lock = 0;
//		xil_printf ("^^\r\n");

	} else {
		//invalid
		i2s_invalid = 1;
		if (lock == 0) {
			xil_printf ("**\r\n");
			appx_fs = 0;
			appx_fs_dup = 0;
			lock = 1;
		}
	}

#if 0
    if ((rx_maud != maud_dup)) {
	XACR_WriteReg (RX_ACR_ADDR, RXACR_MAUD, rx_maud); // divider
	maud_dup = rx_maud;

    }
    if ((rx_naud != naud_dup)) {
	XACR_WriteReg (RX_ACR_ADDR, RXACR_NAUD, rx_naud); // divider
	naud_dup = rx_naud;
    }
#endif

	if ((appx_fs_dup != appx_fs) && (i2s_invalid == 0)) {
		XACR_WriteReg (RX_ACR_ADDR, RXACR_ENABLE, 0x0);
		XACR_WriteReg (RX_ACR_ADDR, RXACR_MAUD, rx_maud); // divider
		XACR_WriteReg (RX_ACR_ADDR, RXACR_NAUD, rx_naud); // divider
		XACR_WriteReg (RX_ACR_ADDR, RXACR_MODE, 0x0); // use streaming from DP
		start_i2s_clk = 1;
		AudioinfoFrame.frame_count = 0;
		appx_fs_dup = appx_fs;
		XACR_WriteReg (RX_ACR_ADDR, RXACR_ENABLE, 0x1);
		xil_printf ("^^\r\n");
	} else {

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
	u8 bpc;
	char *color;
	u8 color_mode = 0;
	u8 Bpc[] = {6, 8, 10, 12, 16};

	int i = 0;

	frameBuffer_stop_wr();

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

	use_vsc = ((rxMsamisc1 >> 6) &0x1);

	Msa[0].Misc0 = rxMsamisc0;
	Msa[0].Misc1 = rxMsamisc1;
	rxMsamisc0 = ((rxMsamisc0 >> 5) & 0x00000007);
	Msa[0].Vtm.Timing.HActive = DpHres;
	Msa[0].Vtm.Timing.VActive = DpVres;
	Msa[0].Vtm.Timing.HTotal = DpHres_total;
	Msa[0].Vtm.Timing.F0PVTotal = DpVres_total;
#if ADAPTIVE
	vblank_captured = 1;
	vblank_init = DpVres_total - DpVres;
#endif
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
	bpc = XDpRxss_GetBpc(&DpRxSsInst, XDP_TX_STREAM_ID1);
	Msa[0].BitsPerColor = bpc;

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);

	if(color_mode == 0){
			Msa[0].ComponentFormat =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
			color = "RGB";
	}
	else if(color_mode == 1){
			Msa[0].ComponentFormat =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
			color = "YCbCr444";
	}
	else if(color_mode == 2){
			Msa[0].ComponentFormat =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
			color = "YCbCr422";
	} else {
		//RAW, 420, Y unsupported
		xil_printf ("Unsupported Color Format\r\n");
	}

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
	Msa[0].DynamicRange = XDpRxss_GetDynamicRange(&DpRxSsInst, XDP_TX_STREAM_ID1);
	Msa[0].YCbCrColorimetry = XDpRxss_GetColorimetry(&DpRxSsInst, XDP_TX_STREAM_ID1);

	if((recv_clk_freq*1000000)>540000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount==4){
		tx_ppc_set = 0x4;
		Msa[0].UserPixelWidth = 0x4;

	}
	else if((recv_clk_freq*1000000)>270000000
			&& (int)DpRxSsInst.UsrOpt.LaneCount!=1){
		tx_ppc_set = 0x2;
		Msa[0].UserPixelWidth = 0x2;

	}
	else{
		tx_ppc_set = 0x1;
		Msa[0].UserPixelWidth = 0x1;

	}

	Msa[0].OverrideUserPixelWidth = 1;
	XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	XDp_RxSetUserPixelWidth(DpRxSsInst.DpPtr, (int)DpRxSsInst.UsrOpt.LaneCount);

	if (DpRxSsInst.link_up_trigger == 1) {
#if !PHY_COMP
		xil_printf(
			"*** Resolution: "
				"%lu x %lu @ %luHz, BPC = %lu, PPC = %d, Color = %s (%d) ***\r\n",
			DpHres, DpVres,recv_frame_clk_int,bpc,(int)DpRxSsInst.UsrOpt.LaneCount,
				color, use_vsc
		);
#endif
	}

	if (DpRxSsInst.link_up_trigger == 1) {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
		frameBuffer_start_wr(Msa, 0);
		XDp_RxDtgEn(DpRxSsInst.DpPtr);
#endif
	}

	CalculateCRC();

		return 1;
}

extern u8 tx_after_rx;

int Dppt_DetectColor(void *InstancePtr,
							XDpTxSs_MainStreamAttributes Msa[4]){

	int x,y = 0;
	u8 color_mode = 0;
	u8 use_vsc_local = 0;
	u8 component;

	u32 rxMsamisc1 = XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
			XDP_RX_MSA_MISC1);
	use_vsc_local = ((rxMsamisc1 >> 6) &0x1);
	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);
	if(color_mode == 0){
		component =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
	}
	else if(color_mode == 1){
		component =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}
	else if(color_mode == 2){
		component =
					XDP_TX_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	} else {
		//RAW, 420, Y unsupported
		xil_printf(ANSI_COLOR_RED"Unsupported Color Format !!"ANSI_COLOR_RESET"\r\n");
	}

	if ((component != Msa[0].ComponentFormat) ||
			(use_vsc != use_vsc_local)) {

			for (x=0;x<500;x++){
				for (y=0;y<500;y++){
					if (!DpRxSsInst.link_up_trigger) {
						return 0;
						break;
					}
				}
				if (!DpRxSsInst.link_up_trigger) {
					return 0;
					break;
				}
			}

			for (x=0;x<500;x++){
					for (y=0;y<500;y++){
						if (!DpRxSsInst.link_up_trigger) {
							return 0;
							break;
						}
					}
					if (!DpRxSsInst.link_up_trigger) {
						return 0;
						break;
					}
				}

		if (DpRxSsInst.link_up_trigger == 1) {
			frameBuffer_stop();
			xil_printf ("Color Format change detected.. restarting video & TX\r\n");
			Dppt_DetectResolution(InstancePtr, Msa);
			if (DpRxSsInst.link_up_trigger == 1) {
				return 1;
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
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
	//Ensure to set TX in async mode. The TX is in ASYNC mode
	//Setting the MISC0[0] to 0
	//Setting bit[12] of Misc0 to '1' for VSC packet

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC0 +
			StreamOffset[0], (((XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC0)) | 0x00001000) & 0xFFFFFFFE));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_MAIN_STREAM_MISC1 +
			StreamOffset[0], XDp_ReadReg(DpRxSsInst.DpPtr->Config.BaseAddr,
					XDP_RX_MSA_MISC1));

	XDp_WriteReg(DpTxSsInst.DpPtr->Config.BaseAddr, XDP_TX_USER_PIXEL_WIDTH +
		StreamOffset[0], tx_ppc_set);
}
#endif
