/*******************************************************************************
* Copyright (C) 2025 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file dptxss_pt_dp21.c
*
* This file contains a design example using both XDpTxss and XDpRxSs
* drivers in multi stream (MST) transport mode to demonstrate Pass-through
* design.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.6   GM    03/15/26  Initial release.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "main.h"
#include "xintc.h"
#include "xparameters.h"

#define XTIMER0_DEVICE_ID 	XPAR_TMRCTR_0_DEVICE_ID

#define PS_IIC_CLK 100000
#ifdef SDT
#define INTRNAME_DPTX   0
#define INTRNAME_DPRX   0
#endif
#define	XINTC	XIntc

#ifdef Tx
#include "tx.h"
#endif
#ifdef Rx
#include "rx.h"
#endif

void Dp21RxSs_PtMst_ResetWrFb();
void Dp21RxSs_PtMst_ResetRdFb();
void Dp21RxSs_PtMst_PtSetup();
int I2cMux_Ps(u8 mux);
int I2cClk_Ps(u32 InFreq, u32 OutFreq);

#ifndef SDT
u32 Dp21RxSs_PtMst_PhyInit(u16 DeviceId);
#else
u32 Dp21RxSs_PtMst_PhyInit(UINTPTR BaseAddress);
#endif

u32 Dp21RxSs_PtMst_CalcStride(XVidC_ColorFormat Cfmt,
	       u16 AXIMMDataWidth,
	       XVidC_VideoStream *StreamPtr);
extern volatile u8 Tx_only;
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

u16 fb_wr_count = 0;
u16 fb_rd_count = 0;
u64 XVFRMBUFRD_BUFFER_BASEADDR_Y;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR_Y;
XINTC IntcInst;				/**< The interrupt controller instance. */
Video_CRC_Config VidFrameCRC_rx;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_rx_1;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_rx_2;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_rx_3;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_rx_4;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx;

Video_CRC_Config VidFrameCRC_tx_1;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx_2;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx_3;	/**< Video Frame CRC instance */
Video_CRC_Config VidFrameCRC_tx_4;	/**< Video Frame CRC instance */

XAxis_Switch axis_switch;
XTmrCtr TmrCtr;				/**< Timer instance.*/
XIic IicInstance;			/**< I2C bus for MC6000 and IDT */
XVphy VPhyInst;				/**< The DPRX Subsystem instance.*/

typedef struct {
	XVidC_ColorFormat MemFormat;
	XVidC_ColorFormat StreamFormat;
	u16 FormatBits;
} VideoFormats;

#define NUM_TEST_FORMATS 21
VideoFormats ColorFormats[NUM_TEST_FORMATS] =
{
	/*
	 * memory format	    stream format	bits per component
	 */
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

/*
 * CUSTOM_TIMING: Here is the detailed timing for each custom resolutions.
 */
const XVidC_VideoTimingMode XVidC_MyVideoTimingMode[
						    (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1))] =
{{(XVidC_VideoMode)XVIDC_VM_7680x4320_30_DELL, "7680x4320_DELL@30Hz", XVIDC_FR_30HZ,
  {7680, 48, 32, 80, 7840, 0,
   4320, 3, 5, 53, 4381, 0, 0, 0, 0, 1}},
};

/*
 * Set the TX PLL and Channel based on the VPHY config
 */
#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_TX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_TX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_TX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif

/*
 * Set the RX PLL and Channel based on the VPHY config
 */
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x1)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL0;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN0;
#endif
#if (XPAR_XVPHY_0_RX_PLL_SELECTION == 0x2)
XVphy_PllType VPHY_RX_PLL_TYPE = XVPHY_PLL_TYPE_QPLL1;
XVphy_ChannelId VPHY_RX_CHANNEL_TYPE = XVPHY_CHANNEL_ID_CMN1;
#endif

/*
 * These are the REFCLK sources for VCU118 and ZCU102
 */
#ifdef PLATFORM_MB	/**< VCU118 (270Mhz on REFCLK0, 400Mhz on REFCLK1) */
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK1;
#else /**< ZCU102 (270Mhz on REFCLK0, 400Mhz on NORTHREFCLK0) */
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_270 = XVPHY_REF_CLK_SEL_XPLL_GTREFCLK0;
XVphy_PllRefClkSelType VPHY_REFCLK_SEL_400 = XVPHY_REF_CLK_SEL_XPLL_GTNORTHREFCLK0;
#endif

#define PHY_CLK_DP14 270000000
#define PHY_CLK_DP21 400000000

XIic_Config *ConfigPtr_IIC;	/**< Pointer to configuration data */

XDpRxSs DpRxSsInst;		/**< The DPRX Subsystem instance.*/

#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
XV_FrmbufRd_l2     frmbufrd;
#endif
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
XV_FrmbufWr_l2     frmbufwr;
#endif

extern u32 StreamOffset[4];
extern XDpTxSs DpTxSsInst;	/**< The DPTX Subsystem instance. */

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
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int Dp21RxSs_PtMst_VideoFMC_Init(void)
{
	int Status = XST_SUCCESS;
	u8 Buffer[2];
	int ByteCount;

	xil_printf("VFMC: Setting IO Expanders...\n\r");

	XIic_Config *ConfigPtr_IIC;     /* Pointer to configuration data */
	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
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

	/*
	 * Set the I2C Mux to select the HPC FMC
	 */
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

#if 0
	/*
	 * Required for previous version of FMC cards.
	 */
	u32 freq = 0;
	while(freq != 1) {
		freq = Dp21RxSs_PtMst_I2cReadFreq (IIC_BASE_ADDR, 0x4D, 0x0);
	}
	xil_printf ("Freq lock = %x\r\n", freq);
#endif

	return XST_SUCCESS;
}

int Dp21RxSs_PtMst_I2cWriteFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		   u8 RegisterAddress, u32 Value)
{
	u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[4];
	u8 Retry = 0;

	/*
	 * Write data
	 */
        Buffer[0] = RegisterAddress;
        Buffer[1] = (Value & 0x000000FF);
        Buffer[2] = (Value & 0x0000FF00) >> 8;
        Buffer[3] = (Value & 0x00FF0000) >> 16;
        Buffer[4] = (Value & 0xFF000000) >> 24;

        xil_printf("%x, %x, %x, %x\r\n", Buffer[1], Buffer[2], Buffer[3], Buffer[4]);

	while (1) {
#ifndef versal
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 5, XIIC_STOP);
		if (ByteCount == 5) {
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
                }
#else
		Status = XIicPs_MasterSendPolled(&Ps_Iic0, (u8 *)&Buffer,
						 2, I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;
                        /*
			 * Maximum retries
			 */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

u8 Dp21RxSs_PtMst_I2cReadFreq(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/*
		 * Set Address
		 */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

                        /*
			 * Maximum retries
			 */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/*
			 * Read data
			 */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

			Data = Buffer[0];
			Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}

int Dp21RxSs_PtMst_I2cWriteTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u8 RegisterAddress, u8 Value)
{
	u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Retry = 0;

	/*
	 * Write data
	 */
	Buffer[0] = RegisterAddress;
	Buffer[1] = Value;

	while (1) {
#ifndef versal
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 2, XIIC_STOP);
		if (ByteCount == 2) {
			Status = XST_SUCCESS;
		} else {
			Status = XST_FAILURE;
		}
#else
		Status = XIicPs_MasterSendPolled(&Ps_Iic0, (u8 *)&Buffer, 2, I2CSlaveAddress);
#endif
		if (Status != XST_SUCCESS) {
			Retry++;
			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

u8 Dp21RxSs_PtMst_I2cReadTdp2004(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/*
		 * Set Address
		 */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/*
			 * Read data
			 */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 1, XIIC_STOP);

			Data = Buffer[0];
			Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}

/*****************************************************************************/
/**
*
* This is the main function for XDpRxSs pass-through example. If the
* Dp21RxSs_PtMst_Main function which setup the system succeeds, this function
* will wait for the interrupts.
*
* @param	None.
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

	/*
	 * Initialize ICache
	 */
	Xil_ICacheInvalidate ();
	Xil_ICacheEnable ();
	/*
	 * Initialize DCache
	 */
	Xil_DCacheInvalidate ();
	Xil_DCacheEnable ();

	xil_printf("\n******************************************************"
				"************************\n\r");
	xil_printf("         DisplayPort2.x MST Pass Through Demonstration"
			"                \n\r");
	xil_printf("                   (c) by AMD   ");
	xil_printf("%s %s\n\r\r\n", __DATE__  ,__TIME__ );
	xil_printf("                   System Configuration:\r\n");
	xil_printf("                      DP SS : %d byte\r\n",
					8);
	xil_printf("\n********************************************************"
				"************************\n\r");

	Status = Dp21RxSs_PtMst_Main();
	if (Status != XST_SUCCESS) {
		xil_printf("DisplayPort MST Pass Through example failed.\n\r");
		return XST_FAILURE;
	}
	disable_caches();

	return XST_SUCCESS;
}

u32 Dp21RxSs_PtMst_Main(void)
{
	u32 Status;
	char CommandKey;

#ifdef Rx
	XDpRxSs_Config *ConfigPtr_rx;
#endif

#ifdef Tx
	XDpTxSs_Config *ConfigPtr_tx;
#endif

	/*
	 * Do platform initialization in this function. This is hardware
	 * system specific. It is up to the user to implement this function.
	 */
	xil_printf("PlatformInit\n\r");
	Status = Dp21RxSs_PtMst_PlatformInit();
	if (Status != XST_SUCCESS) {
		xil_printf("Platform init failed!\n\r");
	}
	xil_printf("Platform initialization done.\n\r");

#ifdef Rx
	/*
	 * Obtain the device configuration
	 * for the DisplayPort RX Subsystem
	 */
#ifndef SDT
	ConfigPtr_rx = XDpRxSs_LookupConfig(XDPRXSS_DEVICE_ID);
#else
	ConfigPtr_rx = XDpRxSs_LookupConfig(XPAR_DPRXSS_0_BASEADDR);
#endif
	if (!ConfigPtr_rx) {
		return XST_FAILURE;
	}
	/*
	 * Copy the device configuration into
	 * the DpRxSsInst's Config structure.
	 */
	Status = XDpRxSs_CfgInitialize(&DpRxSsInst, ConfigPtr_rx,
					ConfigPtr_rx->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPRXSS config initialization failed.\n\r");
		return XST_FAILURE;
	}

	/*
	 * Check for SST/MST support
	 */
	if (DpRxSsInst.UsrOpt.MstSupport) {
		xil_printf("INFO:DPRXSS is MST enabled. DPRXSS can be "
			"switched to SST/MST\n\r");
	} else {
		xil_printf("INFO:DPRXSS is SST enabled. DPRXSS works "
			   "only in SST mode.\n\r");
	}
#endif

#ifdef Tx
	/*
	 * Obtain the device configuration for the DisplayPort TX Subsystem
	 */
#ifndef SDT
	ConfigPtr_tx = XDpTxSs_LookupConfig(XPAR_DPTXSS_0_DEVICE_ID);
#else
	ConfigPtr_tx = XDpTxSs_LookupConfig(XPAR_DPTXSS_0_BASEADDR);
#endif
	if (!ConfigPtr_tx) {
		return XST_FAILURE;
	}

	/*
	 * Copy the device configuration into
	 * the DpTxSsInst's Config structure.
	 */
	Status = XDpTxSs_CfgInitialize(&DpTxSsInst, ConfigPtr_tx,
					ConfigPtr_tx->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("DPTXSS config initialization failed.\r\n");
		return XST_FAILURE;
	}

	/*
	 * Check for SST/MST support
	 */
	if (DpTxSsInst.UsrOpt.MstSupport) {
		xil_printf("INFO:DPTXSS is MST enabled. DPTXSS can be "
			   "switched to SST/MST\r\n");
	} else {
		xil_printf("INFO:DPTXSS is SST enabled. DPTXSS works "
			   "only in SST mode.\r\n");
	}
#endif

#ifdef Tx
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES		/**< FrameBuffer Rd initialization.*/
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
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES		/**< FrameBuffer Wr initialization. */
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

	Dp21RxSs_PtMst_SetupIntrSystem();

	/*
	 * Setup Video Phy, left to the user for implementation
	 */
#ifndef SDT
	Dp21RxSs_PtMst_PhyInit(XVPHY_DEVICE_ID);
#else
	Dp21RxSs_PtMst_PhyInit(XPAR_VID_PHY_CONTROLLER_1_BASEADDR);
#endif

#ifdef Tx
	XVphy_BufgGtReset(&VPhyInst, XVPHY_DIR_TX,(FALSE));
	Status = Dp21RxSs_PtMst_PhyConfig(&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_TX_PLL_TYPE,
			    VPHY_TX_CHANNEL_TYPE, XVPHY_DIR_TX);
	if (Status != XST_SUCCESS) {
		xil_printf ("Configuration of PHY for TX failed\r\n");
	}
#endif

#ifdef Rx
	Status = Dp21RxSs_PtMst_PhyConfig(&VPhyInst, XDP_TX_LINK_BW_SET_UHBR135, VPHY_RX_PLL_TYPE,
			    VPHY_RX_CHANNEL_TYPE, XVPHY_DIR_RX);
	if (Status != XST_SUCCESS) {
		xil_printf ("Configuration of PHY for RX failed\r\n");
	}

	/*
	 * Issue HPD at here to inform DP source
	 */
	XDp_RxInterruptDisable(DpRxSsInst.DpPtr, 0xFFF8FFFF);
	XDp_RxInterruptEnable(DpRxSsInst.DpPtr, 0x80000000);
	XDp_RxGenerateHpdInterrupt(DpRxSsInst.DpPtr, 50000);
#endif

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
	Dp21RxSs_PtMst_ResetRdFb();
#endif

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Dp21RxSs_PtMst_ResetWrFb();
#endif

#ifdef Rx
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES		/**< FrameBuffer Wr callback. */
	XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE,
			       &Dp21RxSs_PtMst_WrBufferCallback, &frmbufwr);
#endif
#endif

#ifdef Tx
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES		/**< FrameBuffer Rd callback. */
	XVFrmbufRd_SetCallback(&frmbufrd, XVFRMBUFRD_HANDLER_DONE,
			       &Dp21RxSs_PtMst_RdBufferCallback, &frmbufrd);
#endif
#endif

	/*
	 * Adding custom resolutions at here.
	 */
	xil_printf("INFO> Registering Custom Timing Table with %d entries \r\n",
		   (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	Status = XVidC_RegisterCustomTimingModes(XVidC_MyVideoTimingMode,
						 (XVIDC_CM_NUM_SUPPORTED - (XVIDC_VM_CUSTOM + 1)));
	if (Status != XST_SUCCESS) {
		xil_printf("ERR: Unable to register custom timing table\r\r\n\n");
	}

	Dp21RxSs_PtMst_OperationMenu();
	while (1) {
		CommandKey = 0;
		CommandKey = Dp21RxSs_PtMst_Getc(0xff);
		if (CommandKey != 0) {
			xil_printf("UserInput: %c\r\n", CommandKey);
			switch (CommandKey) {
				case 'p':
					DpTxSsInst.UsrOpt.MstSupport = 0;
					Dp21RxSs_PtMst_PtSetup();
					Tx_only = 0;
					break;
#ifdef Tx
				case 'i':
					DpTxSsInst.UsrOpt.MstSupport = 1;
					DpTxSsInst.DpPtr->TxInstance.TxSetMsaCallback = NULL;
					DpTxSsInst.DpPtr->TxInstance.TxMsaCallbackRef = NULL;
					Tx_only = 1;
					main_loop();
					break;
#endif
				default:
					xil_printf("Please select correct option\r\n");
					break;
			}
		}
	}

	return XST_SUCCESS;
}

void Dp21RxSs_PtMst_PLLRefClkSel(XVphy *InstancePtr, u8 link_rate, XVphy_ChannelId Channel)
{
	XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_REFCLK_SEL_270, PHY_CLK_DP14);
	XVphy_CfgQuadRefClkFreq(InstancePtr, 0, VPHY_REFCLK_SEL_400, PHY_CLK_DP21);

	switch(link_rate) {
		case XDP_TX_LINK_BW_SET_162GBPS:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_162GBPS);
			break;
		case XDP_TX_LINK_BW_SET_540GBPS:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_540GBPS);
			break;
		case XDP_TX_LINK_BW_SET_810GBPS:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_810GBPS);
			break;
		case XDP_TX_LINK_BW_SET_UHBR10:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1000GBPS);
			break;
		case XDP_TX_LINK_BW_SET_UHBR20:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_2000GBPS);
			break;
		case XDP_TX_LINK_BW_SET_UHBR135:
			XVphy_CfgLineRate(InstancePtr, 0,
					  Channel, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			XVphy_CfgLineRate(InstancePtr, 0,
					  XVPHY_CHANNEL_ID_CHA, XVPHY_DP_LINK_RATE_HZ_1350GBPS);
			break;
		default:
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
* @param	InstancePtr is a pointer to the XVphy instance.
* @param	LineRate is link rate value.
* @param	Pll is an instance to the XVphy_PllType.
* @param	Channel is an instance to the XVphy_ChannelId.
* @param	Dir is an instance to the XVphy_DirectionType.
*
* @return	Status.
*
* @note		None.
*
******************************************************************************/
u32 Dp21RxSs_PtMst_PhyConfig (XVphy *InstancePtr, int LineRate, XVphy_PllType Pll,
		XVphy_ChannelId Channel, XVphy_DirectionType Dir)
{
	u32 Status = XST_SUCCESS;

	XVphy_PllRefClkSelType Refclk;

	if ((LineRate == RATE_810) ||
	    (LineRate == RATE_540) ||
	    (LineRate == RATE_270) ||
	    (LineRate == RATE_162)) {
		Refclk = VPHY_REFCLK_SEL_270;
	} else {
		Refclk = VPHY_REFCLK_SEL_400;
	}

	/*
	 * QPLL1 doesnt support 13.5g
	 */
	if ((LineRate == RATE_1350) && (Pll == XVPHY_PLL_TYPE_QPLL1)) {
		xil_printf("QPLL1 doesnt support 13.5G..skipping\r\n");
		return Status;
	}

	Dp21RxSs_PtMst_PLLRefClkSel (InstancePtr, LineRate, Channel);

	XVphy_SetupDP21Phy (InstancePtr, 0, Channel, Dir, LineRate, Refclk, Pll);

	if (Dir == XVPHY_DIR_TX) {
		Status = XVphy_DP21PhyReset(InstancePtr, 0, Channel, Dir);
	} else {
		/*
		 * For RX, this is accomplished in the rx pll reset handler
		 */
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
******************************************************************************/
void CustomWaitUs(void *InstancePtr, u32 MicroSeconds)
{
	u32 TimerVal;
	XDp *DpInstance = (XDp *)InstancePtr;
	u32 NumTicks = (MicroSeconds *
			(DpInstance->Config.SAxiClkHz / 1000000));

	XTmrCtr_Reset(DpInstance->UserTimerPtr, 0);
	XTmrCtr_Start(DpInstance->UserTimerPtr, 0);

	/*
	 * Wait specified number of useconds.
	 */
	do {
		TimerVal = XTmrCtr_GetValue(DpInstance->UserTimerPtr, 0);
	} while (TimerVal < NumTicks);
}

/*****************************************************************************/
/**
*
* This function Calculates CRC values of Video components
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
#ifdef Rx
void Dp21RxSs_PtMst_CalculateCRC(u8 Stream)
{
	u32 RegVal;
	u8 color_mode = 0;
	(void)RegVal;

	/*
	 * Reset CRC Test Counter in DP DPCD Space.
	 */
	XVidFrameCrc_Reset (&VidFrameCRC_rx);

	/*
	 * Reset CRC Test Counter in DP DPCD Space.
	 */
	XVidFrameCrc_Reset (&VidFrameCRC_rx_1);

	/*
	 * Reset CRC Test Counter in DP DPCD Space.
	 */
	XVidFrameCrc_Reset (&VidFrameCRC_rx_2);

	/*
	 * Reset CRC Test Counter in DP DPCD Space.
	 */
	XVidFrameCrc_Reset (&VidFrameCRC_rx_3);

	/*
	 * Reset CRC Test Counter in DP DPCD Space.
	 */
	XVidFrameCrc_Reset (&VidFrameCRC_rx_4);

	VidFrameCRC_rx.TEST_CRC_CNT = 0;
	VidFrameCRC_rx_1.TEST_CRC_CNT = 0;
	VidFrameCRC_rx_2.TEST_CRC_CNT = 0;
	VidFrameCRC_rx_3.TEST_CRC_CNT = 0;
	VidFrameCRC_rx_4.TEST_CRC_CNT = 0;

	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
		      XDP_RX_CRC_CONFIG,
		      (VidFrameCRC_rx.TEST_CRC_SUPPORTED << 5 |
		       VidFrameCRC_rx.TEST_CRC_CNT));

	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
		      XDP_RX_CRC_CONFIG,
		      (VidFrameCRC_rx_1.TEST_CRC_SUPPORTED << 5 |
		       VidFrameCRC_rx_1.TEST_CRC_CNT));

	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
		      XDP_RX_CRC_CONFIG,
		      (VidFrameCRC_rx_2.TEST_CRC_SUPPORTED << 5 |
		       VidFrameCRC_rx_2.TEST_CRC_CNT));

	Dp21RxSs_PtMst_Write(DpRxSsInst.DpPtr->Config.BaseAddr,
		      XDP_RX_CRC_CONFIG,
		      (VidFrameCRC_rx_3.TEST_CRC_SUPPORTED << 5 |
		       VidFrameCRC_rx_3.TEST_CRC_CNT));

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);
	if (color_mode == 2) {
		VidFrameCRC_rx.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx.Mode_422 = 0x0;
	}

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID2);
	if (color_mode == 2) {
		VidFrameCRC_rx_1.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx_1.Mode_422 = 0x0;
	}

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID3);
	if (color_mode == 2) {
		VidFrameCRC_rx_2.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx_2.Mode_422 = 0x0;
	}

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID4);
	if (color_mode == 2) {
		VidFrameCRC_rx_3.Mode_422 = 0x1;
	} else {
		VidFrameCRC_rx_3.Mode_422 = 0x0;
	}

	if (Stream == 0) {
		VidFrameCRC_rx_4.Mode_422 = VidFrameCRC_rx.Mode_422;
	} else if (Stream == 1) {
		VidFrameCRC_rx_4.Mode_422 = VidFrameCRC_rx_1.Mode_422;
	} else if (Stream == 1) {
		VidFrameCRC_rx_4.Mode_422 = VidFrameCRC_rx_2.Mode_422;
	} else if (Stream == 3) {
		VidFrameCRC_rx_4.Mode_422 = VidFrameCRC_rx_3.Mode_422;
	}

	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_rx, VidFrameCRC_rx.Mode_422);
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_rx_1, VidFrameCRC_rx_1.Mode_422);
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_rx_2, VidFrameCRC_rx_2.Mode_422);
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_rx_3, VidFrameCRC_rx_3.Mode_422);
	XVidFrameCrc_SetVideoFormat(&VidFrameCRC_rx_4, VidFrameCRC_rx_4.Mode_422);
}
#endif

/*****************************************************************************/
/**
*
* This function scans VFMC- IIC.
*
* @param	BaseAddress is base address of the I2C.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Dp21RxSs_PtMst_I2CScan(u32 BaseAddress)
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
* @param	I2CBaseAddress is base address of the I2C.
* @param	I2CSlaveAddress is address of the I2C slave device.
* @param	RegisterAddress is register address of the I2C slave device.
*
* @return	Data from I2C device.
*
* @note		None.
*
******************************************************************************/
u8 Dp21RxSs_PtMst_I2cReadDp141(u32 I2CBaseAddress, u8 I2CSlaveAddress, u16 RegisterAddress)
{
	u32 ByteCount = 0;
	u8 Buffer[1];
	u8 Data;
	u8 Retry = 0;
	u8 Exit;

	Exit = FALSE;
	Data = 0;

	do {
		/*
		 * Set Address
		 */
		Buffer[0] = RegisterAddress & 0xff;
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress,
				      (u8*)Buffer, 1, XIIC_REPEATED_START);

		if (ByteCount != 1) {
			Retry++;

			/*
			 * Maximum retries.
			 */
			if (Retry == 255) {
				Exit = TRUE;
			}
		} else {
			/*
			 * Read data.
			 */
			ByteCount = XIic_Recv(I2CBaseAddress, I2CSlaveAddress,
					      (u8*)Buffer, 1, XIIC_STOP);
				Data = Buffer[0];
				Exit = TRUE;
		}
	} while (!Exit);

	return Data;
}

int Dp21RxSs_PtMst_I2cWriteDp141(u32 I2CBaseAddress, u8 I2CSlaveAddress,
		    u16 RegisterAddress, u8 Value)
{
	u32 Status;
	u32 ByteCount = 0;
	u8 Buffer[2];
	u8 Retry = 0;

	/*
	 * Write data
	 */
	Buffer[0] = RegisterAddress & 0xff;
	Buffer[1] = Value;

	while (1) {
		ByteCount = XIic_Send(I2CBaseAddress, I2CSlaveAddress, (u8*)Buffer, 3, XIIC_STOP);

		if (ByteCount == 2) {
			Status=XST_SUCCESS;
		} else {
			Status=XST_FAILURE;
		}
		if (Status != XST_SUCCESS) {
			Retry++;

			/*
			 * Maximum retries
			 */
			if (Retry == 255) {
				return XST_FAILURE;
			}
		} else {
			return XST_SUCCESS;
		}
	}
}

/*****************************************************************************/
/**
*
* This function initialize required platform specific peripherals.
*
* @param	None.
*
* @return
*        - XST_SUCCESS if required peripherals are initialized and
*        configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
u32 Dp21RxSs_PtMst_PlatformInit(void)
{
	u32 Status = XST_SUCCESS;
	u8 TimerChannel = 0;

	/*
	 * Initialize CRC & Set default Pixel Mode to 1.
	 */
#ifdef Rx
	VidFrameCRC_rx.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx);

	VidFrameCRC_rx_1.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR_1;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx_1);

	VidFrameCRC_rx_2.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR_2;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx_2);

	VidFrameCRC_rx_3.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR_3;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx_3);

	VidFrameCRC_rx_4.Base_Addr = VIDEO_FRAME_CRC_RX_BASEADDR_4;
	XVidFrameCrc_Initialize(&VidFrameCRC_rx_4);
#endif

#ifdef Tx
	VidFrameCRC_tx.Base_Addr = VIDEO_FRAME_CRC_TX_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx);

	VidFrameCRC_tx_1.Base_Addr = VIDEO_FRAME_CRC_TX1_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx_1);

	VidFrameCRC_tx_2.Base_Addr = VIDEO_FRAME_CRC_TX2_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx_3);

	VidFrameCRC_tx_3.Base_Addr = VIDEO_FRAME_CRC_TX3_BASEADDR;
	XVidFrameCrc_Initialize(&VidFrameCRC_tx_3);

#endif
#ifndef SDT
	XAxis_Switch_Config *ConfigPtr_AXIS_SWITCH = XAxisScr_LookupConfig(XPAR_DP_RX_HIER_0_AXIS_SWITCH_0_DEVICE_ID);
#else
	XAxis_Switch_Config *ConfigPtr_AXIS_SWITCH = XAxisScr_LookupConfig(XPAR_XAXIS_SWITCH_0_BASEADDR);
#endif
	if (ConfigPtr_AXIS_SWITCH == NULL) {
		return XST_FAILURE;
	}

	Status = XAxisScr_CfgInitialize(&axis_switch, ConfigPtr_AXIS_SWITCH, ConfigPtr_AXIS_SWITCH->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Initialize Timer
	 */
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
	XTmrCtr_SetResetValue(&TmrCtr, TimerChannel, TIMER_RESET_VALUE);
	XTmrCtr_Start(&TmrCtr, TimerChannel);
#endif

	Dp21RxSs_PtMst_VideoFMC_Init();
	u8 dat;
	dat = Dp21RxSs_PtMst_I2cReadTdp2004(IIC_BASE_ADDR, 0x18, 0xF0);
	dat = Dp21RxSs_PtMst_I2cReadTdp2004(IIC_BASE_ADDR, 0x18, 0xF1);
	(void)dat;

	/*
	 * Setting the gain to 2.6db
	 */
	Dp21RxSs_PtMst_I2cWriteTdp2004(IIC_BASE_ADDR, 0x18, 0x83, 0x7);

	/*
	 * Putting the driver in run mode
	 */
	Dp21RxSs_PtMst_I2cWriteTdp2004(IIC_BASE_ADDR, 0x18, 0x84, 0x4);

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
u32 Dp21RxSs_PtMst_SetupIntrSystem(void)
{
	u32 Status;
	XIntc *IntcInstPtr = &IntcInst;
	(void)IntcInstPtr;
	(void)IntcInstPtr;

	/*
	 * Tx side
	 */
#ifdef Tx
	DpTxSs_SetupIntrSystem();
#endif
	/*
	 * Rx side
	 */
#ifdef Rx
	DpRxSs_SetupIntrSystem();
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstPtr, XINTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\n\r");
		return XST_FAILURE;
	}

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined
	 * above performs the specific interrupt processing for the device.
	 */
#ifdef Rx
	/*
	 * Hook up Rx interrupt service routine
	 */
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
	/*
	 * Connect the device driver handler that will be called when an
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

	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Status = XIntc_Start(IntcInstPtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef Rx
	/*
	 * Enable the interrupt for the DP device
	 */
	XIntc_Enable(IntcInstPtr, XINTC_DPRXSS_DP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID);
#endif

#ifdef Tx
	XIntc_Enable(IntcInstPtr, XINTC_DPTXSS_DP_INTERRUPT_ID);
	XIntc_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID);
#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception
	 * table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XINTC_HANDLER,
				     IntcInstPtr);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return (XST_SUCCESS);
}
#else
u32 Dp21RxSs_PtMst_SetupIntrSystem(void)
{
	u32 Status;

	/*
	 * Tx side
	 */
#ifdef Tx
	DpTxSs_SetupIntrSystem();
#endif
	/*
	 * Rx side
	 */
#ifdef Rx
	DpRxSs_SetupIntrSystem();
#endif

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined
	 * above performs the specific interrupt processing for the device.
	 */
#ifdef Rx
	/*
	 * Hook up Rx interrupt service routine
	 */
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
	/*
	 * Connect the device driver handler that will be called when an
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
* @param	None.
*
* @return
*        - XST_SUCCESS if Video Phy configured successfully.
*        - XST_FAILURE, otherwise.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
u32 Dp21RxSs_PtMst_PhyInit(u16 DeviceId)
#else
u32 Dp21RxSs_PtMst_PhyInit(UINTPTR BaseAddress)
#endif
{
	XVphy_Config *ConfigPtr;

	/*
	 * Obtain the device configuration for the DisplayPort RX Subsystem
	 */
#ifndef SDT
	ConfigPtr = XVphy_LookupConfig(DeviceId);
#else
	ConfigPtr = XVphy_LookupConfig(BaseAddress);
#endif
	if (!ConfigPtr) {
		return XST_FAILURE;
	}

	Dp21RxSs_PtMst_PLLRefClkSel (&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_TX_CHANNEL_TYPE);
	Dp21RxSs_PtMst_PLLRefClkSel (&VPhyInst, XDP_TX_LINK_BW_SET_UHBR20, VPHY_RX_CHANNEL_TYPE);

	XVphy_DpInitialize(&VPhyInst, ConfigPtr, 0,
			   VPHY_REFCLK_SEL_400,
			   VPHY_REFCLK_SEL_400,
			   VPHY_TX_PLL_TYPE,
			   VPHY_RX_PLL_TYPE,
			   XDP_TX_LINK_BW_SET_UHBR20);

	/*
	 * Set the default vswing and pe for v0p0
	 */

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
 * This function configures Frame BufferWr for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int Dp21RxSs_PtMst_ConfigWrFb(u32 StrideInBytes,
		    XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr)
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	int Status;

	/*
	 * Stop Frame Buffers
	 */

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

	/*
	 * Enable Interrupt
	 */
	XVFrmbufWr_InterruptEnable(&frmbufwr,
			XVFRMBUFWR_HANDLER_DONE);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);

	/*
	 * Start Frame Buffers
	 */
	XVFrmbufWr_Start(&frmbufwr);

	return(Status);
#endif
}

u8 stopped = 1;
u8 start_rdfb = 0;

/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 * The Frame Buffer is put in Autorestart mode for non-Adaptive Sync mode
 * When Adaptive Sync mode is enabled the FrameBuffer is put in manual mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int Dp21RxSs_PtMst_ConfigRdFb(u32 StrideInBytes,
		    XVidC_ColorFormat Cfmt,
		    XVidC_VideoStream *StreamPtr)
{
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	int Status;

	XVFRMBUFRD_BUFFER_BASEADDR = frame_array[frame_pointer];
	XVFRMBUFRD_BUFFER_BASEADDR_Y = frame_array_y[frame_pointer];

	/*
	 * Configure  Frame Buffers
	 */
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

	/*
	 * Enable Interrupt
	 */
	XVFrmbufRd_InterruptEnable(&frmbufrd,
				   XVFRMBUFRD_HANDLER_DONE);

	/*
	 * When Adaptive mode is 0 or Monitor does not support Adaptive Sync
	 * the FB read is configured in AutoEnableRestart mode
	 */
	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);

	/*
	 * Start Frame Buffers
	 */
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

int Dp21RxSs_PtMst_ConfigRdFbTrunc(u32 offset)
{
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	int Status;

	/*
	 * Stop Frame Buffers
	 */
	Status = XVFrmbufRd_Stop(&frmbufrd);
	if(Status != XST_SUCCESS) {
		xil_printf("Failed to stop XVFrmbufRd\r\n");
	}

	XVFRMBUFRD_BUFFER_BASEADDR = frame_array[frame_pointer_rd] + offset;
	XVFRMBUFRD_BUFFER_BASEADDR_Y = frame_array_y[frame_pointer_rd] + offset;

	offset_rd = offset;
	/*
	 * Configure  Frame Buffers
	 */
	Status = XVFrmbufRd_SetMemFormat(&frmbufrd,
					 XV_frmbufrd_Get_HwReg_stride(&frmbufrd.FrmbufRd),
					 XV_frmbufrd_Get_HwReg_video_format(&frmbufrd.FrmbufRd),
					 XVFrmbufRd_GetVideoStream(&frmbufrd));
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

	/*
	 * Enable Interrupt
	 */
	XVFrmbufRd_InterruptEnable(&frmbufrd, 0);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);

	/*
	 * Start Frame Buffers
	 */
	XVFrmbufRd_Start(&frmbufrd);

	return(Status);
#endif
}

void Dp21RxSs_PtMst_FbStop()
{
	fb_rd_start = 0;
#ifdef Tx
	Dp21RxSs_PtMst_RdFbStop();
#endif
#ifdef Rx
	Dp21RxSs_PtMst_WrFbStop();
#endif
}

#ifdef Tx
void Dp21RxSs_PtMst_RdFbStop()
{
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	u32 Status;
	fb_rd_start = 0;

	Status = XVFrmbufRd_Stop(&frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf ("Failed to stop Frame Buffer Write\r\n");
	}

	Dp21RxSs_PtMst_ResetRdFb();
	Status = XVFrmbufRd_WaitForIdle(&frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
	usleep(1000);
#endif
}
#endif

#ifdef Rx
void Dp21RxSs_PtMst_WrFbStop()
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	u32 Status;

	Status = XVFrmbufWr_Stop(&frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf ("Failed to stop Frame Buffer Write\r\n");
	}

	Dp21RxSs_PtMst_ResetWrFb();
	Status = XVFrmbufWr_WaitForIdle(&frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf ("Frame Buffer is not Idle\r\n");
	}
	usleep (1000);
#endif
}
#endif

void Dp21RxSs_PtMst_WrFbStart(XDpTxSs_MainStreamAttributes Msa[4], u8 Stream)
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	XVidC_ColorFormat Cfmt;
	XVidC_VideoStream VidStream;
	Dp21RxSs_PtMst_ResetWrFb();

	/*
	 * Get video format to test
	 */
	if(Msa[Stream].BitsPerColor <= 8) {
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		}else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	} else if(Msa[Stream].BitsPerColor == 10) {
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[0].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	}else if(Msa[Stream].BitsPerColor == 12) {
		VidStream.ColorDepth = XVIDC_BPC_12;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[18].MemFormat;
			VidStream.ColorFormatId = ColorFormats[18].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[17].MemFormat;
			VidStream.ColorFormatId = ColorFormats[17].StreamFormat;
		} else {
			Cfmt = ColorFormats[15].MemFormat;
			VidStream.ColorFormatId = ColorFormats[15].StreamFormat;
		}
	} else if(Msa[Stream].BitsPerColor == 16) {
		VidStream.ColorDepth = XVIDC_BPC_16;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[20].MemFormat;
			VidStream.ColorFormatId = ColorFormats[20].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[19].MemFormat;
			VidStream.ColorFormatId = ColorFormats[19].StreamFormat;
		} else {
			Cfmt = ColorFormats[16].MemFormat;
			VidStream.ColorFormatId = ColorFormats[16].StreamFormat;
		}
	}

	VidStream.PixPerClk  = 8;
	VidStream.Timing = Msa[Stream].Vtm.Timing;
	VidStream.FrameRate = Msa[Stream].Vtm.FrameRate;

	u32 stride = Dp21RxSs_PtMst_CalcStride(Cfmt, AXIMMWIDTH, &VidStream);
	Dp21RxSs_PtMst_ConfigWrFb(stride, Cfmt, &VidStream);
	stopped = 1;
	fb_wr_count = 0;
#endif
}

void Dp21RxSs_PtMst_RdFbStart(XDpTxSs_MainStreamAttributes Msa[4], u8 Stream)
{
#if XPAR_XV_FRMBUFRD_NUM_INSTANCES
	XVidC_ColorFormat Cfmt;
	XVidC_VideoTiming const *TimingPtr;
	XVidC_VideoStream VidStream;
	(void)TimingPtr;

	/*
	 * Get video format to test
	 */
	if(Msa[Stream].BitsPerColor <= 8) {
		VidStream.ColorDepth = XVIDC_BPC_8;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[2].MemFormat;
			VidStream.ColorFormatId = ColorFormats[2].StreamFormat;
		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[8].MemFormat;
			VidStream.ColorFormatId = ColorFormats[8].StreamFormat;
		} else {
			Cfmt = ColorFormats[7].MemFormat;
			VidStream.ColorFormatId = ColorFormats[7].StreamFormat;
		}
	} else if(Msa[Stream].BitsPerColor == 10) {
		VidStream.ColorDepth = XVIDC_BPC_10;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[9].MemFormat;
			VidStream.ColorFormatId = ColorFormats[9].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[4].MemFormat;
			VidStream.ColorFormatId = ColorFormats[4].StreamFormat;
		} else {
			Cfmt = ColorFormats[3].MemFormat;
			VidStream.ColorFormatId = ColorFormats[3].StreamFormat;
		}
	} else if (Msa[Stream].BitsPerColor == 12) {
		VidStream.ColorDepth = XVIDC_BPC_12;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[18].MemFormat;
			VidStream.ColorFormatId = ColorFormats[18].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[17].MemFormat;
			VidStream.ColorFormatId = ColorFormats[17].StreamFormat;
		} else {
			Cfmt = ColorFormats[15].MemFormat;
			VidStream.ColorFormatId = ColorFormats[15].StreamFormat;
		}
	} else if (Msa[Stream].BitsPerColor == 16) {
		VidStream.ColorDepth = XVIDC_BPC_16;
		if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422) {
			Cfmt = ColorFormats[20].MemFormat;
			VidStream.ColorFormatId = ColorFormats[20].StreamFormat;

		} else if (Msa[Stream].ComponentFormat ==
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444) {
			Cfmt = ColorFormats[19].MemFormat;
			VidStream.ColorFormatId = ColorFormats[19].StreamFormat;
		} else {
			Cfmt = ColorFormats[16].MemFormat;
			VidStream.ColorFormatId = ColorFormats[16].StreamFormat;
		}
	}

	Msa[Stream].UserPixelWidth = DpTxSsInst.DpPtr->TxInstance.MsaConfig[Stream].UserPixelWidth;
	Msa[Stream].Vtm.Timing = DpTxSsInst.DpPtr->TxInstance.MsaConfig[Stream].Vtm.Timing;

	VidStream.PixPerClk  = Msa[Stream].UserPixelWidth;
	VidStream.Timing = Msa[Stream].Vtm.Timing;
	VidStream.FrameRate = Msa[Stream].Vtm.FrameRate;

	u32 stride = Dp21RxSs_PtMst_CalcStride(Cfmt, AXIMMWIDTH, &VidStream);

	Dp21RxSs_PtMst_ConfigRdFb(stride, Cfmt, &VidStream);
	fb_rd_start = 1;
	fb_rd_count = 0;
#endif
}

void Dp21RxSs_PtMst_ResetRdFb()
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x1);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x1);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          /**< Hold reset line */
#endif
}

void Dp21RxSs_PtMst_ResetWrFb()
{
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x2);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x2);
	usleep(10000);          /**< Hold reset line */
	Xil_Out32(XPAR_PROC_HIER_0_HLS_RST_0_BASEADDR, 0x3);
	usleep(10000);          /**< Hold reset line */
#endif
}

#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
void Dp21RxSs_PtMst_WrBufferCallback(void *InstancePtr)
{
	(void)InstancePtr;
	u32 Status;

	if (XVFRMBUFWR_BUFFER_BASEADDR >= (0 + (0x90000000) + (0x10000000 * 2))) {
		XVFRMBUFRD_BUFFER_BASEADDR = (0 + (0x90000000) + (0x10000000 * 1) +
										offset_rd);
		XVFRMBUFRD_BUFFER_BASEADDR_Y = (0 + (0xC0000000) + (0x10000000 * 1) +
										offset_rd);

		XVFRMBUFWR_BUFFER_BASEADDR = 0 + (0x90000000);
		XVFRMBUFWR_BUFFER_BASEADDR_Y = 0 + (0xC0000000);
	} else {
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

	/*
	 * In Non-Adaptive Scenario, the FB Read is in Autorestart mode.
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
void Dp21RxSs_PtMst_RdBufferCallback(void *InstancePtr)
{
	(void)InstancePtr;
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
u32 Dp21RxSs_PtMst_CalcStride(XVidC_ColorFormat Cfmt,
	       u16 AXIMMDataWidth,
	       XVidC_VideoStream *StreamPtr)
{
	u32 stride;
	int width = StreamPtr->Timing.HActive;
	u16 MMWidthBytes = AXIMMDataWidth/8;

	if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
	    || (Cfmt == XVIDC_CSF_MEM_Y10)) {
		/*
		 * 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
		 */
		stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
		   || (Cfmt == XVIDC_CSF_MEM_Y8)) {
		/*
		 * 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
		 */
		stride = ((width + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	} else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)) {
		/*
		 * 3 bytes per pixel (RGB8, YUV8)
		 */
		stride = (((width * 3) + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	} else if (Cfmt == XVIDC_CSF_MEM_YUYV8) {
		stride = (((width * 2) + MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;
	} else {
		/*
		 * 4 bytes per pixel
		 */
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

/*
 * This process takes in all the MSA values and find out resolution, BPC,
 * refresh rate. Further this sets the pixel_width based on the pixel_clock and
 * lane set. This is to ensure that it matches the values in TX driver. Else
 * video cannot be passthrough. Approximation is implemented for refresh rates.
 * Sometimes a refresh rate of 60 is detected as 59
 * and vice-versa. Approximation is done for single digit.
 *
 * This function is a call back to write the MSA values to Tx as they are
 * read from the Rx, instead of reading them from the Video common library
 */
u8 tx_ppc_set = 0;

int Dp21RxSs_PtMst_DetectResolution(void *InstancePtr, u8 Stream)
{
	(void)InstancePtr;
	char *color;
	u8 i;
	i = 0;

	Msa_test = XDpRxss_GetMsa(&DpRxSsInst);
	Dp21RxSs_PtMst_WrFbStop();
	XDp_RxDtgDis(DpRxSsInst.DpPtr);

	/*
	 * Get the actual number of streams configured
	 */
	u8 num_streams = DpRxSsInst.Config.NumMstStreams;
	xil_printf("\r\n========== MSA Information (Configured Streams: %d) ==========\r\n", num_streams);

	for (i = 0; i < num_streams; i++) {
		/*
		 * Only print if stream has valid resolution data
		 */
		if (Msa_test[i].Vtm.Timing.HActive == 0 || Msa_test[i].Vtm.Timing.VActive == 0) {
			xil_printf("****Stream %d: No valid video detected\r\n", i+1);
			continue;
		}

		if (Msa_test[i].ComponentFormat == 0) {
			color="RGB";
		} else if (Msa_test[i].ComponentFormat == 1) {
			color="YUV422";
		} else if (Msa_test[i].ComponentFormat == 2) {
			color="YUV444";
		}

		xil_printf ("****Stream %d************\r\n",i+1);
		if (DpRxSsInst.link_up_trigger == 1) {
			xil_printf("*** Resolution: "
				   "%d x %d @ %dHz, BPC = %d, Color = %s ***\r\n",
				   (int)Msa_test[i].Vtm.Timing.HActive,
				   (int)Msa_test[i].Vtm.Timing.VActive,
				   (int)Msa_test[i].Vtm.FrameRate,
				   (int)Msa_test[i].BitsPerColor,
				   color);

			/*
			 * Print Audio information based on DP version
			 */
			if (Msa_test[i].IsRxDp21) {
				/*
				 * DP 2.1: Print AFREQ (audio frequency)
				 */
				if (Msa_test[i].AudioAFreq != 0) {
					xil_printf("*** Audio AFREQ: %llu Hz ***\r\n",
						   (unsigned long long)Msa_test[i].AudioAFreq);
				}
			} else {
				/*
				 * DP 1.4: Print MAud/NAud
				 */
				if (Msa_test[i].AudioMAud != 0 || Msa_test[i].AudioNAud != 0) {
					xil_printf("*** Audio MSA: MAud = %lu, NAud = %lu ***\r\n",
						   (unsigned long)Msa_test[i].AudioMAud, (unsigned long)Msa_test[i].AudioNAud);
				}
			}

		}
	}
	xil_printf("==============================================================\r\n\r\n");

	if (DpRxSsInst.link_up_trigger == 1) {
#if XPAR_XV_FRMBUFWR_NUM_INSTANCES
		Dp21RxSs_PtMst_WrFbStart(Msa_test, Stream);
#endif
		XDp_RxDtgEn(DpRxSsInst.DpPtr);

	}

	/*
	 * Setup the CRC calculation
	 */
	Dp21RxSs_PtMst_CalculateCRC(Stream);

	return 1;
}

extern u8 tx_after_rx;

int Dp21RxSs_PtMst_DetectColor(void *InstancePtr,
		     XDpTxSs_MainStreamAttributes Msa_test[4])
{
	int x,y = 0;
	(void)InstancePtr;
	u8 color_mode = 0;
	u8 component;

	color_mode = XDpRxss_GetColorComponent(&DpRxSsInst, XDP_TX_STREAM_ID1);
	if (color_mode == 0) {
		component = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_RGB;
	} else if (color_mode == 1) {
		component = XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR444;
	} else if (color_mode == 2) {
		component =
				XDP_MAIN_STREAMX_MISC0_COMPONENT_FORMAT_YCBCR422;
	} else {
		/*
		 * RAW, 420, Y unsupported
		 */
		xil_printf(ANSI_COLOR_RED"Unsupported Color Format !!"ANSI_COLOR_RESET"\r\n");
	}

	if (component != Msa_test[0].ComponentFormat) {
		for (x=0;x<500;x++){
			for (y=0;y<500;y++){
				if (!DpRxSsInst.link_up_trigger) {
					return 0;
				}
			}
			if (!DpRxSsInst.link_up_trigger) {
				return 0;
			}
		}

		for (x = 0; x < 500; x++) {
			for (y = 0; y < 500; y++) {
				if (!DpRxSsInst.link_up_trigger) {
					return 0;
				}
			}

			if (!DpRxSsInst.link_up_trigger) {
				return 0;
			}
		}

		if (DpRxSsInst.link_up_trigger == 1) {
			Dp21RxSs_PtMst_FbStop();
			xil_printf ("Color Format change detected.. restarting video & TX\r\n");
			Dp21RxSs_PtMst_DetectResolution(InstancePtr, 1);	/**< Msa_test */

			if (DpRxSsInst.link_up_trigger == 1) {
				return 1;
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function reads a register value using XDp_ReadReg for MST operations
*
* @param	BaseAddress - The base address of the DP core
* @param	RegOffset - The register offset to read from
*
* @return	The 32-bit value read from the register
*
* @note		This is a wrapper function for XDp_ReadReg specifically for
*			MST-related register reads
*
******************************************************************************/
u32 Dp21RxSs_PtMst_Read(u32 BaseAddress, u32 RegOffset)
{
	return XDp_ReadReg(BaseAddress, RegOffset);
}

/*****************************************************************************/
/**
*
* This function writes a register value using XDp_WriteReg for MST operations
*
* @param	BaseAddress - The base address of the DP core
* @param	RegOffset - The register offset to write to
* @param	Data - The 32-bit value to write to the register
*
* @return	None
*
* @note		This is a wrapper function for XDp_WriteReg specifically for
*			MST-related register writes
*
******************************************************************************/
void Dp21RxSs_PtMst_Write(u32 BaseAddress, u32 RegOffset, u32 Data)
{
	XDp_WriteReg(BaseAddress, RegOffset, Data);
}
#endif
