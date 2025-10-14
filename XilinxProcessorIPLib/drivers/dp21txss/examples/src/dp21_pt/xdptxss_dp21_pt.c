/*******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file dptxss_pt_dp21.c
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
* 1.00 ND 09/23/24 Initial release.
* 1.01 ND 04/10/25 Added Parreto fmc support.
* 1.02 ND 04/30/25 Added logic to handle 13.5g failure on TX Qpll1, detection of
*                  rx cable unplug and backward compatibility with dp14 sinks for
*					8b/10b.
* 1.03 KU 09/23/25 Optimization of function and clean up. Increased the gain of
*                  TDP2004
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "main.h"
#include "ti_lmk03318.h"
#include "si5344drv.h"
#include "xintc.h"

#define PS_IIC_CLK 100000
#ifdef SDT
#define INTRNAME_DPTX   0
#define INTRNAME_DPRX   0
#endif
#define 			XINTC XIntc

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
#ifndef SDT
u32 DpSs_PhyInit(u16 DeviceId);
#else
u32 DpSs_PhyInit(UINTPTR BaseAddress);
#endif
u32 CalcStride(XVidC_ColorFormat Cfmt,
					  u16 AXIMMDataWidth,
					  XVidC_VideoStream *StreamPtr);
extern XDp_MainStreamAttributes* Msa_test;
u32 frame_array[3] = {0x90000000, 0xA0000000, 0xB0000000};
u32 frame_array_y[3] = {0xC0000000, 0xD0000000, 0xE0000000};
u8 frame_pointer = 0;
u8 frame_pointer_rd = 2;
u8 not_to_read = 1;
u8 not_to_write = 3;
u8 fb_rd_start = 0;
u32 vblank_init = 0;
u8 vblank_captured = 0;
u8 Clip_4k=0;
u32 Clip_4k_Hactive=0;
u32 Clip_4k_Vactive=0;
u16 fb_wr_count = 0;
u16 fb_rd_count = 0;
u64 XVFRMBUFRD_BUFFER_BASEADDR_Y;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR_Y;
XINTC IntcInst; 	/* The interrupt controller instance. */
Video_CRC_Config VidFrameCRC_rx; /* Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx;
XTmrCtr TmrCtr; 		/* Timer instance.*/
XIic IicInstance; 	/* I2C bus for MC6000 and IDT */
XVphy VPhyInst; 	/* The DPRX Subsystem instance.*/

typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16 FormatBits;
} VideoFormats;

#define NUM_TEST_FORMATS 21//15
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
	{XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8},
	{XVIDC_CSF_MEM_RGBX12,     XVIDC_CSF_RGB,       12},
	{XVIDC_CSF_MEM_RGB16,     XVIDC_CSF_RGB,       16},
	{XVIDC_CSF_MEM_YUVX12,     XVIDC_CSF_YCRCB_444,       12},
	{XVIDC_CSF_MEM_Y_UV12,     XVIDC_CSF_YCRCB_422,       12},
	{XVIDC_CSF_MEM_YUV16,     XVIDC_CSF_YCRCB_444,       16},
	{XVIDC_CSF_MEM_Y_UV16,     XVIDC_CSF_YCRCB_422,       16},
};

// CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
					(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{	{ XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
		{7680, 48, 32, 80, 7840, 0,
		4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}},
};

//Set the TX PLL and Channel based on the VPHY config
#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif


//Set the RX PLL and Channel based on the VPHY config
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif


//These are the REFCLK sources for VCU118 and ZCU102
#ifdef PLATFORM_MB //VCU118 (270Mhz on REFCLK0, 400Mhz on REFCLK1)
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK1;
#else //ZCU102 (270Mhz on REFCLK0, 400Mhz on NORTHREFCLK0)
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTNORTHREFCLK0;
#endif



XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */

XV_tpg TpgInst;

XDpRxSs DpRxSsInst;    /* The DPRX Subsystem instance.*/
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
XV_FrmbufRd_l2     frmbufrd;
#endif
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
XV_FrmbufWr_l2     frmbufwr;
#endif
extern u32 StreamOffset[4];
extern XDpTxSs DpTxSsInst; 		/* The DPTX Subsystem instance.*/

void enable_caches()
{
#ifdef __PPC__
    Xil_ICacheEnableRegion(CACHEABLE_REGION_MASK);
    Xil_DCacheEnableRegion(CACHEABLE_REGION_MASK);
#elif __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#endif
}

void disable_caches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheDisable();
#endif
#endif
}

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
	int Status=XST_SUCCESS;
	u8 Buffer[2];
	int ByteCount;

	xil_printf("VFMC: Setting IO Expanders...\n\r");

	XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
	/* Initialize the IIC driver so that it is ready to use. */
    #ifndef SDT
	ConfigPtr_IIC = XIic_LookupConfig(IIC_DEVICE_ID);
    #else
    ConfigPtr_IIC = XIic_LookupConfig(XPAR_XIIC_0_BASEADDR);
    #endif
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
#ifndef PLATFORM_MB
	Buffer[0] = 0x01;
#else
	Buffer[0] = 0x02;
#endif
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_MUX_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C Mux.\n\r");
	    return XST_FAILURE;
	}
#ifndef PARRETO_FMC
	/* Configure VFMC IO Expander 0:
	 * Disable Si5344
	 * Set primary clock source for LMK03318 to IOCLKp(0)
	 * Set secondary clock source for LMK03318 to IOCLKp(1)
	 * Disable LMK61E2*/
	Buffer[0] = 0x01;
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_0_ADDR,
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
	Buffer[0] = 0x16;
	ByteCount = XIic_Send(IIC_BASE_ADDR, I2C_VFMCEXP_1_ADDR,
			(u8*)Buffer, 1, XIIC_STOP);
	if (ByteCount != 1) {
		xil_printf("Failed to set the I2C IO Expander.\n\r");
		return XST_FAILURE;
	}
	xil_printf(" done!\n\r");

	Status = IDT_8T49N24x_Init(IIC_BASE_ADDR, I2C_IDT8N49_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize IDT 8T49N241.\n\r");
		return XST_FAILURE;
	}

	Status = TI_LMK03318_PowerDown(IIC_BASE_ADDR, I2C_LMK03318_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to initialize TI LMK03318.\n\r");
		return XST_FAILURE;
	}

#ifdef PLATFORM_MB
    Status = SI5344_Init (&IicInstance, I2C_SI5344_ADDR);
#else
    Status = SI5344_Init (&Ps_Iic1, I2C_SI5344_ADDR);
#endif
    if (Status != XST_SUCCESS) {
	xil_printf("Failed to Si5344\n\r");
        return XST_FAILURE;
    }
#else
	u32 freq=0;
	while(freq!=1){
		freq = i2c_read_freq (IIC_BASE_ADDR, 0x4D, 0x0);
	}
	xil_printf ("Freq lock = %x\r\n", freq);
#endif
	return XST_SUCCESS;
}
#ifdef PARRETO_FMC
int i2c_write_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u32 Value)
{
    u32 Status;
        u32 ByteCount = 0;
        u8 Buffer[4];
        u8 Retry = 0;
        // Write data
        Buffer[0] = RegisterAddress;
        Buffer[1] = (Value & 0x000000FF);
        Buffer[2] = (Value & 0x0000FF00) >> 8;
        Buffer[3] = (Value & 0x00FF0000) >> 16;
        Buffer[4] = (Value & 0xFF000000) >> 24;

        xil_printf ("%x, %x, %x, %x\r\n",Buffer[1], Buffer[2], Buffer[3], Buffer[4]);
        while (1) {
#ifndef versal
                ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 5, XIIC_STOP);
                if (ByteCount == 5) {
                        Status=XST_SUCCESS;
                }
                else{
                        Status=XST_FAILURE;
                }
#else
            Status = XIicPs_MasterSendPolled(&Ps_Iic0,
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

u8 i2c_read_freq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
        u32 ByteCount = 0;
        u8 Buffer[1];
        u8 Data;
        u8 Retry = 0;
        u8 Exit;


        Exit = FALSE;
        Data = 0;

        do {
                // Set Address
                Buffer[0] = RegisterAddress & 0xff;
                ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

                if (ByteCount != 1) {
                        Retry++;

                        // Maximum retries
                        if (Retry == 255) {
                                Exit = TRUE;
                        }
                }

                // Read data
                else {
                        //Read data
                        ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

                        Data = Buffer[0];
                        Exit = TRUE;
                }
        } while (!Exit);

        return Data;
}

int i2c_write_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u8 Value)
{
    u32 Status;
        u32 ByteCount = 0;
        u8 Buffer[1];
        u8 Retry = 0;
        // Write data
        Buffer[0] = RegisterAddress;
        Buffer[1] = Value;

        while (1) {
#ifndef versal
                ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
                if (ByteCount == 2) {
                        Status=XST_SUCCESS;
                }
                else{
                        Status=XST_FAILURE;
                }
#else
            Status = XIicPs_MasterSendPolled(&Ps_Iic0,
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

u8 i2c_read_tdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
        u32 ByteCount = 0;
        u8 Buffer[1];
        u8 Data;
        u8 Retry = 0;
        u8 Exit;


        Exit = FALSE;
        Data = 0;

        do {
                // Set Address
                Buffer[0] = RegisterAddress & 0xff;
                ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

                if (ByteCount != 1) {
                        Retry++;

                        // Maximum retries
                        if (Retry == 255) {
                                Exit = TRUE;
                        }
                }

                // Read data
                else {
                        //Read data
                        ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

                        Data = Buffer[0];
                        Exit = TRUE;
                }
        } while (!Exit);

        return Data;
}
#endif

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
	Xil_ICacheInvalidate ();
	Xil_ICacheEnable ();
	/* Initialize DCache */
	Xil_DCacheInvalidate ();
	Xil_DCacheEnable ();

	xil_printf("\n******************************************************"
				"**********\n\r");
	xil_printf("            DisplayPort Pass Through Demonstration"
			"                \n\r");
	xil_printf("                   (c) by AMD   ");
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
	disable_caches();

	return XST_SUCCESS;
}

u32 DpSs_Main(void)
{
	u32 Status;
	char CommandKey;

#ifdef Rx
	XDpRxSs_Config *ConfigPtr_rx;
#endif

#ifdef Tx
	XDpTxSs_Config *ConfigPtr_tx;
	XV_tpg_Config* ConfigPtr_tpg;
#endif



	/* Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = DpSs_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

    #ifndef SDT
	ConfigPtr_tpg = XV_tpg_LookupConfig(XPAR_DP_TX_HIER_0_V_TPG_0_DEVICE_ID);
    #else
    ConfigPtr_tpg = XV_tpg_LookupConfig(XPAR_DP_TX_HIER_0_V_TPG_0_BASEADDR);
    #endif
	if (!ConfigPtr_tpg) {
		return XST_FAILURE;
	}

	Status = XV_tpg_CfgInitialize(&TpgInst, ConfigPtr_tpg, ConfigPtr_tpg->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("TPG config initialization failed\n\r");
		return XST_FAILURE;
	}

#ifdef Rx
	/* Obtain the device configuration
	 * for the DisplayPort RX Subsystem */
    #ifndef SDT
	ConfigPtr_rx = XDpRxSs_LookupConfig(XDPRXSS_DEVICE_ID);
    #else
    ConfigPtr_rx = XDpRxSs_LookupConfig(XPAR_DPRXSS_0_BASEADDR);
    #endif
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
    #ifndef SDT
		ConfigPtr_tx = XDpTxSs_LookupConfig(XPAR_DPTXSS_0_DEVICE_ID);
    #else
        ConfigPtr_tx = XDpTxSs_LookupConfig(XPAR_DPTXSS_0_BASEADDR);
    #endif
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

#ifdef Tx
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES		/* FrameBuffer Rd initialization. */
    #ifndef SDT
	Status = XVFrmbufRd_Initialize(&frmbufrd, FRMBUF_RD_DEVICE_ID);
    #else
    Status = XVFrmbufRd_Initialize(&frmbufrd, XPAR_DP_TX_HIER_0_V_FRMBUF_RD_0_BASEADDR);
    #endif
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Read "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}
#endif
#endif

#ifdef Rx
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES		/* FrameBuffer Wr initialization. */
    #ifndef SDT
	Status = XVFrmbufWr_Initialize(&frmbufwr, FRMBUF_WR_DEVICE_ID);
    #else
    Status = XVFrmbufWr_Initialize(&frmbufwr, XPAR_DP_RX_HIER_0_V_FRMBUF_WR_0_BASEADDR);
    #endif
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Frame Buffer Write "
			   "initialization failed\r\n");
		return (XST_FAILURE);
	}
#endif
#endif

DpSs_SetupIntrSystem();

	/* Setup Video Phy, left to the user for implementation */
    #ifndef SDT
	DpSs_PhyInit(XVPHY_DEVICE_ID);
    #else
    DpSs_PhyInit(XPAR_VID_PHY_CONTROLLER_1_BASEADDR);
    #endif

#ifdef Tx
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	// This configures the vid_phy for line rate to start with
	//Even though CPLL can be used in limited case,
	//using QPLL is recommended for more coverage.
#endif

#ifdef Tx
	Status = config_phy(&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_TX_PLL_TYPE, VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
	if (Status != XST_SUCCESS) {
		xil_printf ("Configuration of PHY for TX failed\r\n");
	}
#endif

#ifdef Rx
    Status = config_phy(&VPhyInst, XDP_TX_LINK_BW_SET_UHBR135, VPHY_RX_PLL_TYPE, VPHY_RX_CHANNEL_TYPE, XVPHY_DIR_RX);
		if (Status != XST_SUCCESS) {
		xil_printf ("Configuration of PHY for RX failed\r\n");
	}

	/* issue HPD at here to inform DP source */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);
#endif

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
	resetIp_rd();
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	resetIp_wr();
#endif

#ifdef Rx
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES		/* FrameBuffer Wr callback. */
	XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE,
								&bufferWr_callback, &frmbufwr);
#endif
#endif

#ifdef Tx
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES		/* FrameBuffer Rd callback. */
	XVFrmbufRd_SetCallback(&frmbufrd, XVFRMBUFRD_HANDLER_DONE,
								&bufferRd_callback, &frmbufrd);
#endif
#endif

//	 Adding custom resolutions at here.
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
							(XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}


    operationMenu();
	while (1) {
		CommandKey = 0;
		CommandKey = xil_getc(0xff);
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);
			switch (CommandKey) {
			case 'p':
				DpPt_Main();
				break;

			default:
				xil_printf("Please select correct option\r\n");
				break;
			}
		}
	}

	return XST_SUCCESS;
}

void PLLRefClkSel (XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Channel) {

	XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										VPHY_REFCLK_SEL_270, 270000000);
	XVphy_CfgQuadRefClkFreq(InstancePtr, 0,
										VPHY_REFCLK_SEL_400, 400000000);

	switch (link_rate) {

	case XDP_TX_LINK_BW_SET_162GBPS:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
									XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
	case XDP_TX_LINK_BW_SET_540GBPS:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
												XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
	case XDP_TX_LINK_BW_SET_810GBPS:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
															XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_810GBPS);

			break;
	case XDP_TX_LINK_BW_SET_UHBR10:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
															XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			break;

	case XDP_TX_LINK_BW_SET_UHBR20:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
															XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			break;

	case XDP_TX_LINK_BW_SET_UHBR135:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
															XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			break;

	default:
			// XVphy_CfgLineRate(InstancePtr, 0,
			// 		XVPHY_CHANNEL_ID_CMN0, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
						Channel, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
															XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_270GBPS);
			break;
	}
}


/*****************************************************************************/
/**
*
* This function sets VPHY based on the linerate
*
* @param	user_config_struct.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/
u32 config_phy (XVphy *InstancePtr, int LineRate, XVphy_PllType Pll, XVphy_ChannelId Channel, XVphy_DirectionType Dir){
	u32 Status=XST_SUCCESS;

	XVphy_PllRefClkSelType Refclk;

	if ((LineRate == XDP_TX_LINK_BW_SET_810GBPS) ||
		(LineRate == XDP_TX_LINK_BW_SET_540GBPS) ||
		(LineRate == XDP_TX_LINK_BW_SET_270GBPS) ||
		(LineRate == XDP_TX_LINK_BW_SET_162GBPS) ) {
			Refclk = VPHY_REFCLK_SEL_270;
	} else {
			Refclk = VPHY_REFCLK_SEL_400;
	}

	//QPLL1 doesnt support 13.5g
	 if((LineRate == XDP_TX_LINK_BW_SET_UHBR135) && (Pll == XVPHY_PLL_TYPE_QPLL1)){
		xil_printf("QPLL1 doesnt support 13.5G..skipping\r\n");
		return Status;
	 }

	PLLRefClkSel (InstancePtr, LineRate, Channel);

	XVphy_SetupDP21Phy (InstancePtr, 0, Channel, Dir, LineRate, Refclk, Pll);

    if (Dir == XVPHY_DIR_TX) {
		Status = XVphy_DP21PhyReset (InstancePtr, 0, Channel,	Dir);
	} else {
		//For RX, this is accomplished in the rx pll reset handler
	}
	if (Status == XST_FAILURE) {
			xil_printf ("Issue encountered in PHY config and reset\r\n");
	}

	return Status;
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

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);

	if(color_mode == 2){
		VidFrameCRC_rx.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx.Mode_422 = 0x0;
	}


    if (VidFrameCRC_rx.Mode_422 != 0x1) {
	XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                          VIDEO_FRAME_CRC_CONFIG,
                            5);
    } else { // 422
        XVidFrameCrc_WriteReg(VidFrameCRC_rx.Base_Addr,
                              VIDEO_FRAME_CRC_CONFIG,
                                (5 | 0x80000000));
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
				Data = Buffer[0];
				Exit = TRUE;
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

	// Write data
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount == 2) {
			Status=XST_SUCCESS;
		}
		else{
			Status=XST_FAILURE;
		}
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
	u32 Status = XST_SUCCESS;

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
    #ifndef SDT
	Status = XTmrCtr_Initialize(&TmrCtr, XTIMER0_DEVICE_ID);
    #else
    Status = XTmrCtr_Initialize(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR);
    #endif
	if (Status != XST_SUCCESS){
		xil_printf("ERR:Timer failed to initialize. \r\n");
		return XST_FAILURE;
	}
    #ifndef SDT
	XTmrCtr_SetResetValue(&TmrCtr, XTIMER0_DEVICE_ID, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XTIMER0_DEVICE_ID);
    #else
	XTmrCtr_SetResetValue(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, XPAR_XTMRCTR_0_BASEADDR);
    #endif

	VideoFMC_Init();
#ifdef PARRETO_FMC
	u8 dat;
	//dummy read
	dat = i2c_read_tdp2004(IIC_BASE_ADDR, 0x18, 0xF0);
	dat = i2c_read_tdp2004(IIC_BASE_ADDR, 0x18, 0xF1);
	//Setting the gain to 2.6db
	i2c_write_tdp2004(IIC_BASE_ADDR, 0x18, 0x83, 0x7);
	//putting the driver in run mode
	i2c_write_tdp2004(IIC_BASE_ADDR, 0x18, 0x84, 0x4);
#else
	IDT_8T49N24x_SetClock(IIC_BASE_ADDR, I2C_IDT8N49_ADDR, 0,
            270000000, TRUE);
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
#ifndef SDT
u32 DpSs_SetupIntrSystem(void)
{
	u32 Status;
	XIntc *IntcInstPtr = &IntcInst;

	// Tx side
#ifdef Tx
	DpTxSs_SetupIntrSystem();
#endif
	// Rx side
#ifdef Rx
	DpRxSs_SetupIntrSystem();
#endif

	/* Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\n\r");
		return XST_FAILURE;
	}

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined
	 * above performs the specific interrupt processing for the device.
	 * */
#ifdef Rx
	/* Hook up Rx interrupt service routine */
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID,
				(XInterruptHandler)XDpRxSs_DpIntrHandler,
				&DpRxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\n\r");
		return XST_FAILURE;
	}



#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Status = XIntc_Connect(IntcInstPtr,
			XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID,
				 (XInterruptHandler)XVFrmbufWr_InterruptHandler,
				 &frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

#endif

#endif
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
#ifdef Tx
	Status = XIntc_Connect(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID,
				(XInterruptHandler)XDpTxSs_DpIntrHandler,
				&DpTxSsInst);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	Status = XIntc_Connect(IntcInstPtr,
			XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID,
				 (XInterruptHandler)XVFrmbufRd_InterruptHandler,
				 &frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR:: FRMBUF RD interrupt connect failed!\r\n");
		return XST_FAILURE;
	}
#endif
#endif


	/* Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */

	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef Rx
	/* Enable the interrupt for the DP device */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID);
#endif

#ifdef Tx
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID);
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
#else
u32 DpSs_SetupIntrSystem(void)
{
	u32 Status;
	XIntc *IntcInstPtr = &IntcInst;

	// Tx side
#ifdef Tx
	DpTxSs_SetupIntrSystem();
#endif
	// Rx side
#ifdef Rx
	DpRxSs_SetupIntrSystem();
#endif

	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined
	 * above performs the specific interrupt processing for the device.
	 * */
#ifdef Rx
	/* Hook up Rx interrupt service routine */
	Status = XSetupInterruptSystem(&DpRxSsInst, XDpRxSs_DpIntrHandler,
				       DpRxSsInst.Config.IntrId[INTRNAME_DPRX],
				       DpRxSsInst.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP RX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Status = XSetupInterruptSystem(&frmbufwr, XVFrmbufWr_InterruptHandler,
				       frmbufwr.FrmbufWr.Config.IntrId,
				       frmbufwr.FrmbufWr.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP FrameBuffer interrupt connect failed!\r\n");
		return XST_FAILURE;
	}
#endif

#endif
	/* Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device
	 */
#ifdef Tx
	Status = XSetupInterruptSystem(&DpTxSsInst,XDpTxSs_DpIntrHandler,
				       DpTxSsInst.Config.IntrId[INTRNAME_DPTX],
				       DpTxSsInst.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: DP TX SS DP interrupt connect failed!\r\n");
		return XST_FAILURE;
	}

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	Status = XSetupInterruptSystem(&frmbufrd, XVFrmbufRd_InterruptHandler,
				       frmbufrd.FrmbufRd.Config.IntrId,
				       frmbufrd.FrmbufRd.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Frame Buffer Read interrupt connect failed!\r\n");
		return XST_FAILURE;
	}
#endif
#endif
	return (XST_SUCCESS);
}
#endif
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
#ifndef SDT
u32 DpSs_PhyInit(u16 DeviceId)
#else
u32 DpSs_PhyInit(UINTPTR BaseAddress)
#endif
{
	XVphy_Config *ConfigPtr;
	// extern XVphy_User_Config PHY_User_Config_Table[];

	/* Obtain the device configuration for the DisplayPort RX Subsystem */
    #ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
    #else
    ConfigPtr = XVphy_LookupConfig(BaseAddress);
    #endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}


	PLLRefClkSel (&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_TX_CHANNEL_TYPE);
	PLLRefClkSel (&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_RX_CHANNEL_TYPE);

	// XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
	// 		   PHY_User_Config_Table[5].CPLLRefClkSrc,
	// 		   PHY_User_Config_Table[5].QPLLRefClkSrc,
	// 		   PHY_User_Config_Table[5].TxPLL,
	// 		   PHY_User_Config_Table[5].RxPLL,
	// 		   PHY_User_Config_Table[5].LineRate);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
            VPHY_REFCLK_SEL_400,
            VPHY_REFCLK_SEL_400,
            VPHY_TX_PLL_TYPE,
            VPHY_RX_PLL_TYPE,
            XDP_TX_LINK_BW_SET_UHBR20);

	//set the default vswing and pe for v0po

#ifdef Rx
#ifndef PARRETO_FMC
	xil_printf ("Setting polarity (RX) for new DP2.1 FMC\r\n");
	XVphy_SetPolarity(&VPhyInst, 0, XVPHY_CHANNEL_ID_CHA,
			XVPHY_DIR_RX, 1);
#endif
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


	Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	Status |= XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR_Y);
	if(Status != XST_SUCCESS) {
		xil_printf("ERROR:: Unable to configure Frame Buffer "
				"Read buffer address\r\n");
		return(XST_FAILURE);
	}


	/* Enable Interrupt */
	XVFrmbufRd_InterruptEnable(&frmbufrd,
			XVFRMBUFRD_HANDLER_DONE);

	/* When Adaptive mode is 0 or Monitor does not support Adaptive Sync
	 * the FB read is configured in AutoEnableRestart mode
	 */
	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);

	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

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
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufRd\r\n");
	}
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
	XVFrmbufRd_InterruptEnable(&frmbufrd, 0);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	/* Start Frame Buffers */
	XVFrmbufRd_Start(&frmbufrd);

	return(Status);
#endif
}


void frameBuffer_stop() {

	fb_rd_start = 0;
#ifdef Tx
	frameBuffer_stop_rd();
#endif
#ifdef Rx
	frameBuffer_stop_wr();
#endif

}

#ifdef Tx
void frameBuffer_stop_rd() {
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
#endif

#ifdef Rx
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
#endif

void frameBuffer_start_wr(XDpTxSs_MainStreamAttributes Msa[4]) {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES

	XVidC_ColorFormat Cfmt;
	XVidC_VideoStream VidStream;
	resetIp_wr();

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		}else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 10){
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 12){
		VidStream.ColorDepth = XVIDC_BPC_12;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[18].MemFormat;
			VidStream.ColorFormatId = ColorFormats[18].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[17].MemFormat;
			VidStream.ColorFormatId = ColorFormats[17].StreamFormat;
		} else {
			Cfmt = ColorFormats[15].MemFormat;
			VidStream.ColorFormatId = ColorFormats[15].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 16){
		VidStream.ColorDepth = XVIDC_BPC_16;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[20].MemFormat;
			VidStream.ColorFormatId = ColorFormats[20].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[19].MemFormat;
			VidStream.ColorFormatId = ColorFormats[19].StreamFormat;
		} else {
			Cfmt = ColorFormats[16].MemFormat;
			VidStream.ColorFormatId = ColorFormats[16].StreamFormat;
		}
	}

	VidStream.PixPerClk  = 8;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;

	u32 stride = CalcStride(Cfmt,
					512,
					&VidStream);
	ConfigFrmbuf_wr(stride, Cfmt, &VidStream);
	stopped = 1;
	fb_wr_count = 0;
#endif
}

void frameBuffer_start_rd(XDpTxSs_MainStreamAttributes Msa[4]) {
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;

	/* Get video format to test */
	if(Msa[0].BitsPerColor <= 8){
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		} else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 10){
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 12){
		VidStream.ColorDepth = XVIDC_BPC_12;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[18].MemFormat;
			VidStream.ColorFormatId = ColorFormats[18].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[17].MemFormat;
			VidStream.ColorFormatId = ColorFormats[17].StreamFormat;
		} else {
			Cfmt = ColorFormats[15].MemFormat;
			VidStream.ColorFormatId = ColorFormats[15].StreamFormat;
		}
	}else if(Msa[0].BitsPerColor == 16){
		VidStream.ColorDepth = XVIDC_BPC_16;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[20].MemFormat;
			VidStream.ColorFormatId = ColorFormats[20].StreamFormat;

		} else if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[19].MemFormat;
			VidStream.ColorFormatId = ColorFormats[19].StreamFormat;
		} else {
			Cfmt = ColorFormats[16].MemFormat;
			VidStream.ColorFormatId = ColorFormats[16].StreamFormat;
		}
	}

	Msa[0].UserPixelWidth = DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].UserPixelWidth;
	Msa[0].Vtm.Timing = DpTxSsInst.DpPtr->TxInstance.MsaConfig[0].Vtm.Timing;

	VidStream.PixPerClk  = Msa[0].UserPixelWidth;
	VidStream.Timing = Msa[0].Vtm.Timing;
	VidStream.FrameRate = Msa[0].Vtm.FrameRate;
	if(Clip_4k == 1){
		VidStream.Timing.HActive = Clip_4k_Hactive;
		VidStream.Timing.VActive = Clip_4k_Vactive;
	}

	u32 stride = CalcStride(Cfmt,
					512,
					&VidStream);
	if(Clip_4k == 1){
		XVidC_VideoTiming* TimingPtr;
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
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x1);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x1);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
#endif
}


void resetIp_wr()
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x2);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x2);
	usleep(10000);          //hold reset line
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          //hold reset line
#endif
}

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void bufferWr_callback(void *InstancePtr){
	u32 Status;

	if(XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x90000000) + (0x10000000 * 2))){

		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x90000000) + (0x10000000 * 1) +
										offset_rd);
		XVFRMBUFRD_BUFFER_BASEADDR_Y = (0 + (0xC0000000) + (0x10000000 * 1) +
										offset_rd);

		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x90000000);
		XVFRMBUFWR_BUFFER_BASEADDR_Y = 0 + (0xC0000000);
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

	if (fb_rd_start) {
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
u8 lock = 0;

u32 maud_dup = 0;
u32 naud_dup = 0;

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

int Dppt_DetectResolution(void *InstancePtr){//,
	char *color;
	Msa_test = XDPRxss_GetMsa(&DpRxSsInst);
	frameBuffer_stop_wr();
	XDp_RxSetLineReset(DpRxSsInst.DpPtr,XDP_TX_STREAM_ID1);
	XDp_RxDtgDis(DpRxSsInst.DpPtr);
	if(Msa_test[0].ComponentFormat == 0){
		color="RGB";
	}else if(Msa_test[0].ComponentFormat == 1){
		color="YUV422";
	}else if(Msa_test[0].ComponentFormat == 2){
		color="YUV444";
	}
	if (DpRxSsInst.link_up_trigger == 1) {
		xil_printf(
			"*** Resolution: "
				"%lu x %lu @ %luHz, BPC = %lu, Color = %s ***\r\n",
				Msa_test[0].Vtm.Timing.HActive, Msa_test[0].Vtm.Timing.VActive,Msa_test[0].Vtm.FrameRate,Msa_test[0].BitsPerColor,
				color);
	}
	if (DpRxSsInst.link_up_trigger == 1) {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
		frameBuffer_start_wr(Msa_test);
#endif
		XDp_RxDtgEn(DpRxSsInst.DpPtr);

	}
	CalculateCRC();
		return 1;
}

extern u8 tx_after_rx;

int Dppt_DetectColor(void *InstancePtr,
							XDpTxSs_MainStreamAttributes Msa_test[4]){
	int x,y = 0;
	u8 color_mode = 0;
	u8 component;

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);
	if(color_mode == 0){
		component =
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
	}
	else if(color_mode == 1){
		component =
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	}
	else if(color_mode == 2){
		component =
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	} else {
		//RAW, 420, Y unsupported
		xil_printf(ANSI_COLOR_RED"Unsupported Color Format !!"ANSI_COLOR_RESET"\r\n");
	}

	if (component != Msa_test[0].ComponentFormat) {
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
			Dppt_DetectResolution(InstancePtr);//, Msa_test);
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
